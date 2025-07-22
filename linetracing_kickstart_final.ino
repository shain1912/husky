#include "HUSKYLENS.h"

enum MotionState { STOP, FORWARD, TURN_LEFT, TURN_RIGHT };

class HuskyMobile {
private:
  uint8_t _fl_in1, _fl_in2, _fl_en;
  uint8_t _fr_in1, _fr_in2, _fr_en;
  uint8_t _rl_in1, _rl_in2, _rl_en;
  uint8_t _rr_in1, _rr_in2, _rr_en;

  unsigned long _lastMotorChange;
  unsigned long _bootstrapDuration;
  bool _isBootstrapping;
  int16_t _targetSpeed[4];
  int16_t _currentSpeed[4];
  

  MotionState _currentState = STOP;
  int16_t _lastSpeed = 0;  // 마지막 명령 속도
  unsigned long _lastCommandTime = 0;     // 마지막 명령 수행 시간

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

 void Forward(int16_t speed) {
  unsigned long now = millis();

  if (_currentState == FORWARD && _lastSpeed == speed) {
    if (!_isBootstrapping && (now - _lastCommandTime >= 2000)) {
      Serial.println("💡 전진 재부트스트랩 (2초 경과)");
      _lastCommandTime = now;
      int16_t speeds[4] = {speed, speed, speed, speed};
      setMotorSpeeds(speeds, 200, speed, 500);
    }
    return;
  }

  _lastSpeed = speed;
  _currentState = FORWARD;
  _lastCommandTime = now;

  int16_t speeds[4] = {speed, speed, speed, speed};
  setMotorSpeeds(speeds, 150, speed, 500);
}

void TurnLeft(int16_t speed) {
  unsigned long now = millis();

  if (_currentState == TURN_LEFT && _lastSpeed == speed) {
    if (!_isBootstrapping && (now - _lastCommandTime >= 2000)) {
      Serial.println("💡 좌회전 재부트스트랩 (2초 경과)");
      _lastCommandTime = now;
      int16_t speeds[4] = {-speed, speed, -speed, speed};
      setMotorSpeeds(speeds, 250, speed, 600);
    }
    return;
  }

  _lastSpeed = speed;
  _currentState = TURN_LEFT;
  _lastCommandTime = now;

  int16_t speeds[4] = {-speed, speed, -speed, speed};
  setMotorSpeeds(speeds, 250, speed, 600);
}

void TurnRight(int16_t speed) {
  unsigned long now = millis();

  if (_currentState == TURN_RIGHT && _lastSpeed == speed) {
    if (!_isBootstrapping && (now - _lastCommandTime >= 2000)) {
      Serial.println("💡 우회전 재부트스트랩 (2초 경과)");
      _lastCommandTime = now;
      int16_t speeds[4] = {speed, -speed, speed, -speed};
      setMotorSpeeds(speeds, 250, speed, 600);
    }
    return;
  }

  _lastSpeed = speed;
  _currentState = TURN_RIGHT;
  _lastCommandTime = now;

  int16_t speeds[4] = {speed, -speed, speed, -speed};
  setMotorSpeeds(speeds, 250, speed, 600);
}

void Stop() {
  if (_currentState == STOP) return;
  _currentState = STOP;
  _lastSpeed = 0;
  _lastCommandTime = millis();  // ← 추가!

  analogWrite(_fl_en, 0); analogWrite(_fr_en, 0);
  analogWrite(_rl_en, 0); analogWrite(_rr_en, 0);

  digitalWrite(_fl_in1, LOW); digitalWrite(_fl_in2, LOW);
  digitalWrite(_fr_in1, LOW); digitalWrite(_fr_in2, LOW);
  digitalWrite(_rl_in1, LOW); digitalWrite(_rl_in2, LOW);
  digitalWrite(_rr_in1, LOW); digitalWrite(_rr_in2, LOW);

  _isBootstrapping = false;
  for (int i = 0; i < 4; i++) {
    _targetSpeed[i] = 0;
    _currentSpeed[i] = 0;
  }
}

  void update() {
    if (!_isBootstrapping) return;
    unsigned long elapsed = millis() - _lastMotorChange;
    if (elapsed >= _bootstrapDuration) {
      for (int i = 0; i < 4; i++) _currentSpeed[i] = _targetSpeed[i];
      applySpeeds();
      _isBootstrapping = false;
      Serial.println("부트스트랩 완료 - 목표 속도로 전환");
    }
  }

private:
  void setMotorSpeeds(int16_t speeds[4], int16_t bootstrapHigh, int16_t targetLow, unsigned long duration) {
    _bootstrapDuration = duration;
    for (int i = 0; i < 4; i++) {
      _targetSpeed[i] = (speeds[i] > 0) ? targetLow : (speeds[i] < 0 ? -targetLow : 0);
    }

    int16_t bootstrapSpeeds[4];
    for (int i = 0; i < 4; i++) {
      if (_targetSpeed[i] == 0) bootstrapSpeeds[i] = 0;
      else if (_targetSpeed[i] > 0) bootstrapSpeeds[i] = bootstrapHigh;
      else bootstrapSpeeds[i] = -bootstrapHigh;
    }

    for (int i = 0; i < 4; i++) _currentSpeed[i] = bootstrapSpeeds[i];
    applySpeeds();

    _lastMotorChange = millis();
    _isBootstrapping = true;

    Serial.println(String() + "부트스트랩 시작! 초기속도: " + bootstrapHigh + " -> 목표속도: " + targetLow);
  }

  void applySpeeds() {
    runMotor(_fl_in1, _fl_in2, _fl_en, _currentSpeed[0]);
    runMotor(_fr_in1, _fr_in2, _fr_en, _currentSpeed[1]);
    runMotor(_rl_in1, _rl_in2, _rl_en, _currentSpeed[2]);
    runMotor(_rr_in1, _rr_in2, _rr_en, _currentSpeed[3]);
  }

  void runMotor(uint8_t in1, uint8_t in2, uint8_t en, int16_t speed) {
    speed = constrain(speed, -255, 255);
    if (speed >= 0) {
      digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
      analogWrite(en, speed);
    } else {
      digitalWrite(in1, LOW); digitalWrite(in2, HIGH);
      analogWrite(en, -speed);
    }
  }
};

// === 설정 ===
#define FORWARD_SPEED 90
#define TURN_SPEED 120
#define ERROR_THRESHOLD 40

HuskyMobile Robot(2, 3, 5,
                  7, 4, 6,
                  A0, 8, 9,
                  A1, A2, 10);

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
  Serial.println("라인 트레이싱 시작 (PWM 부트스트랩 적용)");
}

void loop() {
  Robot.update();

  int32_t error = 0;
  bool lineFound = false;

  if (huskylens.request(ID1) && huskylens.isLearned() && huskylens.available()) {
    HUSKYLENSResult result = huskylens.read();
    int lineCenter = (result.command == COMMAND_RETURN_ARROW) ? result.xTarget : result.xCenter;
    error = lineCenter - 160;
    lineFound = true;
    Serial.println(String() + "라인 중심: " + lineCenter + ", 오차: " + error);
  }

  if (!lineFound) {
    Serial.println("라인 없음 → 정지");
    Robot.Stop();
  } else {
    if (abs(error) < ERROR_THRESHOLD) {
      Serial.println("전진 (부트스트랩)");
      Robot.Forward(FORWARD_SPEED);
    } else if (error < 0) {
      Serial.println("좌회전 (부트스트랩)");
      Robot.TurnLeft(TURN_SPEED);
    } else {
      Serial.println("우회전 (부트스트랩)");
      Robot.TurnRight(TURN_SPEED);
    }
  }

  delay(50);
}
