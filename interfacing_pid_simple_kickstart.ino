/*
 * 4륜 라인 트레이싱 - PWM 부트스트랩 적용 버전
 * 전진 또는 제자리 회전만 수행
 * PWM 부트스트랩으로 모터 시작 토크 향상
 */

#include "HUSKYLENS.h"

// 4륜 모터 제어 클래스 (PWM 부트스트랩 적용)
class HuskyMobile {
private:
  uint8_t _fl_in1, _fl_in2, _fl_en;  // Front Left
  uint8_t _fr_in1, _fr_in2, _fr_en;  // Front Right
  uint8_t _rl_in1, _rl_in2, _rl_en;  // Rear Left
  uint8_t _rr_in1, _rr_in2, _rr_en;  // Rear Right
  
  unsigned long _lastMotorChange;     // 마지막 모터 변경 시간
  bool _isBootstrapping;              // 부트스트랩 중인지 확인
  int16_t _targetSpeed[4];            // 각 모터의 목표 속도
  int16_t _currentSpeed[4];           // 각 모터의 현재 속도
  
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
    
    _lastMotorChange = 0;
    _isBootstrapping = false;
    for(int i = 0; i < 4; i++) {
      _targetSpeed[i] = 0;
      _currentSpeed[i] = 0;
    }
    
    Stop();
  }

  void Direction(bool left, bool right) {}

  // 전진 (150 -> 110으로 500ms 부트스트랩)
  void Forward(int16_t speed) {
    int16_t speeds[4] = {speed, speed, speed, speed};
    setMotorSpeeds(speeds, 150, 110, 500);
  }

  // 제자리 좌회전 (200 -> 110으로 500ms 부트스트랩)
  void TurnLeft(int16_t speed) {
    int16_t speeds[4] = {-speed, speed, -speed, speed};
    setMotorSpeeds(speeds, 200, 110, 500);
  }

  // 제자리 우회전 (200 -> 110으로 500ms 부트스트랩)
  void TurnRight(int16_t speed) {
    int16_t speeds[4] = {speed, -speed, speed, -speed};
    setMotorSpeeds(speeds, 200, 110, 500);
  }

  void Stop() {
    analogWrite(_fl_en, 0); analogWrite(_fr_en, 0);
    analogWrite(_rl_en, 0); analogWrite(_rr_en, 0);
    
    digitalWrite(_fl_in1, LOW); digitalWrite(_fl_in2, LOW);
    digitalWrite(_fr_in1, LOW); digitalWrite(_fr_in2, LOW);
    digitalWrite(_rl_in1, LOW); digitalWrite(_rl_in2, LOW);
    digitalWrite(_rr_in1, LOW); digitalWrite(_rr_in2, LOW);
    
    _isBootstrapping = false;
    for(int i = 0; i < 4; i++) {
      _targetSpeed[i] = 0;
      _currentSpeed[i] = 0;
    }
  }

  // PWM 부트스트랩 업데이트 (loop에서 호출 필요)
  void update() {
    if (!_isBootstrapping) return;
    
    unsigned long currentTime = millis();
    unsigned long elapsed = currentTime - _lastMotorChange;
    
    if (elapsed >= 500) {  // 500ms 후 목표 속도로 변경
      _currentSpeed[0] = _targetSpeed[0];  // FL
      _currentSpeed[1] = _targetSpeed[1];  // FR
      _currentSpeed[2] = _targetSpeed[2];  // RL
      _currentSpeed[3] = _targetSpeed[3];  // RR
      
      runMotor(_fl_in1, _fl_in2, _fl_en, _currentSpeed[0]);
      runMotor(_fr_in1, _fr_in2, _fr_en, _currentSpeed[1]);
      runMotor(_rl_in1, _rl_in2, _rl_en, _currentSpeed[2]);
      runMotor(_rr_in1, _rr_in2, _rr_en, _currentSpeed[3]);
      
      _isBootstrapping = false;
      Serial.println("부트스트랩 완료 - 목표 속도로 전환");
    }
  }

private:
  void setMotorSpeeds(int16_t speeds[4], int16_t bootstrapHigh, int16_t targetLow, unsigned long duration) {
    // 새로운 명령이 들어오면 부트스트랩 시작
    _targetSpeed[0] = speeds[0];  // FL
    _targetSpeed[1] = speeds[1];  // FR  
    _targetSpeed[2] = speeds[2];  // RL
    _targetSpeed[3] = speeds[3];  // RR
    
    // 부트스트랩 속도 계산 (방향 고려)
    int16_t bootstrapSpeeds[4];
    for(int i = 0; i < 4; i++) {
      if(_targetSpeed[i] == 0) {
        bootstrapSpeeds[i] = 0;
      } else if(_targetSpeed[i] > 0) {
        bootstrapSpeeds[i] = bootstrapHigh;
      } else {
        bootstrapSpeeds[i] = -bootstrapHigh;
      }
    }
    
    // 부트스트랩 속도로 모터 시작
    runMotor(_fl_in1, _fl_in2, _fl_en, bootstrapSpeeds[0]);
    runMotor(_fr_in1, _fr_in2, _fr_en, bootstrapSpeeds[1]);
    runMotor(_rl_in1, _rl_in2, _rl_en, bootstrapSpeeds[2]);
    runMotor(_rr_in1, _rr_in2, _rr_en, bootstrapSpeeds[3]);
    
    _currentSpeed[0] = bootstrapSpeeds[0];
    _currentSpeed[1] = bootstrapSpeeds[1];
    _currentSpeed[2] = bootstrapSpeeds[2];
    _currentSpeed[3] = bootstrapSpeeds[3];
    
    _lastMotorChange = millis();
    _isBootstrapping = true;
    
    Serial.println(String() + "부트스트랩 시작! 초기속도: " + bootstrapHigh + " -> 목표속도: " + targetLow);
  }

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
#define FORWARD_SPEED 110        // 전진 목표 속도 (부트스트랩 후)
#define TURN_SPEED 110          // 회전 목표 속도 (부트스트랩 후)
#define ERROR_THRESHOLD 40      // 오차 임계값

// 4륜 모터 초기화
HuskyMobile Robot(2, 3, 5,    // Front Left
                  7, 4, 6,    // Front Right  
                  A0, 8, 9,   // Rear Left
                  A1, A2, 10); // Rear Right

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
  Serial.println("4륜 PWM 부트스트랩 라인 트레이싱 시작!");
  Serial.println("전진: 150->110 (500ms), 회전: 200->110 (500ms)");
}

void loop() {
  // PWM 부트스트랩 업데이트
  Robot.update();
  
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
    // 단순한 임계값 제어
    if (abs(error) < ERROR_THRESHOLD) {
      // 오차가 작으면 전진 (150->110 부트스트랩)
      Serial.println("전진! (PWM 부트스트랩)");
      Robot.Forward(FORWARD_SPEED);
    }
    else if (error < 0) {
      // 라인이 왼쪽에 있으면 좌회전 (200->110 부트스트랩)
      Serial.println("좌회전! (PWM 부트스트랩)");
      Robot.TurnLeft(TURN_SPEED);
    }
    else {
      // 라인이 오른쪽에 있으면 우회전 (200->110 부트스트랩)
      Serial.println("우회전! (PWM 부트스트랩)");
      Robot.TurnRight(TURN_SPEED);
    }
  }
  
  delay(50);  // 제어 주기 (부트스트랩 업데이트를 위해 짧게)
}

void printResult(HUSKYLENSResult result) {
  if (result.command == COMMAND_RETURN_ARROW) {
    Serial.println(String() + F("Arrow: xTarget=") + result.xTarget);
  }
  else if (result.command == COMMAND_RETURN_BLOCK) {
    Serial.println(String() + F("Block: xCenter=") + result.xCenter);
  }
}