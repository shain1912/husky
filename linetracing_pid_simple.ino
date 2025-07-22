/*
 * 4륜 라인 트레이싱 - 단순 PID 버전
 * 전진 또는 제자리 회전만 수행
 * 스무스 회전 없이 확실한 동작
 */

#include "HUSKYLENS.h"
//#include "SoftwareSerial.h"

// 4륜 모터 제어 클래스 (단순 버전)
class HuskyMobile {
private:
  uint8_t _fl_in1, _fl_in2, _fl_en;  // Front Left
  uint8_t _fr_in1, _fr_in2, _fr_en;  // Front Right
  uint8_t _rl_in1, _rl_in2, _rl_en;  // Rear Left
  uint8_t _rr_in1, _rr_in2, _rr_en;  // Rear Right

public:
  HuskyMobile(uint8_t fl_in1, uint8_t fl_in2, uint8_t fl_en,
              uint8_t fr_in1, uint8_t fr_in2, uint8_t fr_en,
              uint8_t rl_in1, uint8_t rl_in2, uint8_t rl_en,
              uint8_t rr_in1, uint8_t rr_in2, uint8_t rr_en) {
    _fl_in1 = fl_in1; _fl_in2 = fl_in2; _fl_en = fl_en;
    _fr_in1 = fr_in1; _fr_in2 = fr_in2; _fr_en = fr_en;
    _rl_in1 = rl_in1; _rl_in2 = rl_in2; _rl_en = rl_en;
    _rr_in1 = rr_in1; _rr_in2 = rr_in2; _rr_en = rr_en;

    pinMode(_fl_in1, OUTPUT); pinMode(_fl_in2, OUTPUT); pinMode(_fl_en, OUTPUT);
    pinMode(_fr_in1, OUTPUT); pinMode(_fr_in2, OUTPUT); pinMode(_fr_en, OUTPUT);
    pinMode(_rl_in1, OUTPUT); pinMode(_rl_in2, OUTPUT); pinMode(_rl_en, OUTPUT);
    pinMode(_rr_in1, OUTPUT); pinMode(_rr_in2, OUTPUT); pinMode(_rr_en, OUTPUT);
    
    Stop();
  }

  void Direction(bool left, bool right) {}

  // 전진만
  void Forward(int16_t speed) {
    runMotor(_fl_in1, _fl_in2, _fl_en, speed);
    runMotor(_fr_in1, _fr_in2, _fr_en, speed);
    runMotor(_rl_in1, _rl_in2, _rl_en, speed);
    runMotor(_rr_in1, _rr_in2, _rr_en, speed);
  }

  // 제자리 좌회전 (왼쪽 바퀴 뒤로, 오른쪽 바퀴 앞으로)
  void TurnLeft(int16_t speed) {
    runMotor(_fl_in1, _fl_in2, _fl_en, -speed);  // 왼쪽앞 뒤로
    runMotor(_rl_in1, _rl_in2, _rl_en, -speed);  // 왼쪽뒤 뒤로
    runMotor(_fr_in1, _fr_in2, _fr_en, speed);   // 오른쪽앞 앞으로
    runMotor(_rr_in1, _rr_in2, _rr_en, speed);   // 오른쪽뒤 앞으로
  }

  // 제자리 우회전 (왼쪽 바퀴 앞으로, 오른쪽 바퀴 뒤로)
  void TurnRight(int16_t speed) {
    runMotor(_fl_in1, _fl_in2, _fl_en, speed);   // 왼쪽앞 앞으로
    runMotor(_rl_in1, _rl_in2, _rl_en, speed);   // 왼쪽뒤 앞으로
    runMotor(_fr_in1, _fr_in2, _fr_en, -speed);  // 오른쪽앞 뒤로
    runMotor(_rr_in1, _rr_in2, _rr_en, -speed);  // 오른쪽뒤 뒤로
  }

  void Stop() {
    analogWrite(_fl_en, 0); analogWrite(_fr_en, 0);
    analogWrite(_rl_en, 0); analogWrite(_rr_en, 0);
    
    digitalWrite(_fl_in1, LOW); digitalWrite(_fl_in2, LOW);
    digitalWrite(_fr_in1, LOW); digitalWrite(_fr_in2, LOW);
    digitalWrite(_rl_in1, LOW); digitalWrite(_rl_in2, LOW);
    digitalWrite(_rr_in1, LOW); digitalWrite(_rr_in2, LOW);
  }

private:
  void runMotor(uint8_t in1, uint8_t in2, uint8_t en, int16_t speed) {
    if (speed > 255) speed = 255;
    if (speed < -255) speed = -255;
    
    if (speed >= 0) {
      digitalWrite(in1, HIGH);
      digitalWrite(in2, LOW);
      analogWrite(en, speed);
    } else {
      digitalWrite(in1, LOW);
      digitalWrite(in2, HIGH);
      analogWrite(en, -speed);
    }
  }
};

// 설정값
#define FORWARD_SPEED 120        // 전진 속도
#define TURN_SPEED 170          // 회전 속도 (150 이상)
#define ERROR_THRESHOLD 40      // 오차 임계값

// 4륜 모터 초기화
HuskyMobile Robot(2, 3, 5,    // Front Left
                  7, 4, 6,    // Front Right  
                  A0, 8, 9,   // Rear Left
                  A1, A2, 10); // Rear Right

// PID 제어기 없이 단순한 제어만 사용

HUSKYLENS huskylens;
int ID1 = 1;

void setup() {
  Serial.begin(115200);
  
  Wire.begin();
  while (!huskylens.begin(Wire)) {
    Serial.println(F("HUSKYLENS 연결 실패!"));
    delay(1000);
  }
  
  huskylens.writeAlgorithm(ALGORITHM_LINE_TRACKING);
  Serial.println("4륜 단순 라인 트레이싱 시작!");
  Serial.println("전진 또는 제자리 회전만 수행");
}

void loop() {
  int32_t error = 0;
  bool lineFound = false;
  
  if (huskylens.request(ID1) && huskylens.isLearned() && huskylens.available()) {
    HUSKYLENSResult result = huskylens.read();
    
    if (result.command == COMMAND_RETURN_ARROW || result.command == COMMAND_RETURN_BLOCK) {
      int lineCenter = (result.command == COMMAND_RETURN_ARROW) ? result.xTarget : result.xCenter;
      error = lineCenter - 160;  // 화면 중앙에서의 오차
      lineFound = true;
      
      Serial.println(String() + "라인 발견! 중심: " + lineCenter + ", 오차: " + error);
    }
  }
  
  if (!lineFound) {
    Serial.println("라인 없음 - 천천히 전진");
    Robot.Forward(FORWARD_SPEED / 2);
  }
  else {
    // PID 제어 대신 단순한 임계값 제어
    if (abs(error) < ERROR_THRESHOLD) {
      // 오차가 작으면 전진
      Serial.println("전진!");
      Robot.Forward(FORWARD_SPEED);
    }
    else if (error < 0) {
      // 라인이 왼쪽에 있으면 좌회전
      Serial.println("좌회전!");
      Robot.TurnLeft(TURN_SPEED);
    }
    else {
      // 라인이 오른쪽에 있으면 우회전  
      Serial.println("우회전!");
      Robot.TurnRight(TURN_SPEED);
    }
  }
  
  delay(100);  // 제어 주기
}

void printResult(HUSKYLENSResult result) {
  if (result.command == COMMAND_RETURN_ARROW) {
    Serial.println(String() + F("Arrow: xTarget=") + result.xTarget);
  }
  else if (result.command == COMMAND_RETURN_BLOCK) {
    Serial.println(String() + F("Block: xCenter=") + result.xCenter);
  }
}