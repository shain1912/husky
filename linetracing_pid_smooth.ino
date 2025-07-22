/*
 * 라인 트레이싱 코드 - PID 제어 + 스무스 회전 버전
 * 4륜 차량용 개선된 회전 알고리즘 적용
 */

#include "HUSKYLENS.h"
#include "SoftwareSerial.h"
#include "PIDLoop.h"

// 4륜 모터 제어 클래스 (개선된 버전 - 인라인)
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

  // 4륜 스무스 회전을 위한 개선된 Speed 함수
  void Speed(int16_t left_speed, int16_t right_speed) {
    int16_t fl_speed, fr_speed, rl_speed, rr_speed;
    
    if (abs(left_speed - right_speed) < 20) {
      // 거의 직진: 모든 바퀴 같은 속도
      fl_speed = fr_speed = rl_speed = rr_speed = (left_speed + right_speed) / 2;
    } else {
      // 회전: 4륜 차동 제어
      float turn_factor = 0.8;  // 뒷바퀴 속도 조정 (0.6~1.0)
      
      // 회전 중심을 고려한 속도 배분
      fl_speed = left_speed;
      fr_speed = right_speed;
      
      // 뒷바퀴는 앞바퀴보다 약간 느리게 (끌림 방지)
      rl_speed = left_speed * turn_factor;
      rr_speed = right_speed * turn_factor;
      
      // 급한 회전시 더 큰 차이
      if (abs(right_speed - left_speed) > 100) {
        turn_factor = 0.6;
        rl_speed = left_speed * turn_factor;
        rr_speed = right_speed * turn_factor;
      }
    }
    
    // 각 바퀴 독립 제어
    runMotor(_fl_in1, _fl_in2, _fl_en, fl_speed);
    runMotor(_fr_in1, _fr_in2, _fr_en, fr_speed);
    runMotor(_rl_in1, _rl_in2, _rl_en, rl_speed);
    runMotor(_rr_in1, _rr_in2, _rr_en, rr_speed);
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

#define HUSKY_FAST 100  // 기본 속도 (낮춤)

// 4륜 모터 초기화 (사용자가 확인한 핀 배치)
HuskyMobile Robot(3, 2, 5,    // Front Left: IN1, IN2, EN
                  4, 7, 6,    // Front Right: IN1, IN2, EN  
                  8, A0, 9,   // Rear Left: IN1, IN2, EN
                  A2, A1, 10); // Rear Right: IN1, IN2, EN

// PID 제어기 (P=1500으로 낮춤, 부드러운 제어)
PIDLoop headingLoop(1500, 0, 200, false);  // D항 추가

HUSKYLENS huskylens;
int ID1 = 1;

void setup() {
  Serial.begin(115200);
  
  Robot.Direction(false, false);
  
  Wire.begin();
  while (!huskylens.begin(Wire)) {
    Serial.println(F("HUSKYLENS 연결 실패!"));
    delay(1000);
  }
  
  huskylens.writeAlgorithm(ALGORITHM_LINE_TRACKING);
  Serial.println("4륜 스무스 PID 라인 트레이싱 시작!");
  
  headingLoop.reset();
}

int left = 0, right = 0;

void loop() {
  int32_t error;
  
  if (!huskylens.request(ID1)) {
    Serial.println(F("HUSKYLENS 데이터 요청 실패!"));
    left = 0; right = 0;
  }
  else if (!huskylens.isLearned()) {
    Serial.println(F("학습된 라인이 없습니다."));
    left = 0; right = 0;
  }
  else if (!huskylens.available()) {
    // 라인이 안 보이면 천천히 직진
    left = HUSKY_FAST / 3;
    right = HUSKY_FAST / 3;
  }
  else {
    HUSKYLENSResult result = huskylens.read();
    
    // 에러 계산
    error = (int32_t)result.xTarget - (int32_t)160;
    
    // PID 적용
    headingLoop.update(error);
    
    // 부드러운 제어를 위해 PID 출력 제한
    int32_t pid_output = headingLoop.m_command;
    pid_output = constrain(pid_output, -80, 80);  // PID 출력 제한
    
    left = HUSKY_FAST - pid_output;
    right = HUSKY_FAST + pid_output;
    
    // 최소 속도 보장 (한쪽이 너무 느려지지 않게)
    if (abs(left) < 30 && abs(right) < 30) {
      left = (left > 0) ? 30 : -30;
      right = (right > 0) ? 30 : -30;
    }
    
    // 최대 속도 제한
    left = constrain(left, -200, 200);
    right = constrain(right, -200, 200);
    
    Serial.println(String() + "Error: " + error + 
                   ", PID: " + pid_output + 
                   ", L: " + left + ", R: " + right);
  }
  
  Robot.Speed(left, right);
  delay(30);  // 제어 주기를 조금 짧게
}

void printResult(HUSKYLENSResult result) {
  if (result.command == COMMAND_RETURN_ARROW) {
    Serial.println(String() + F("Arrow: xTarget=") + result.xTarget);
  }
}