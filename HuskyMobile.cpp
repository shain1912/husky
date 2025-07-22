/*
 * HuskyMobile.cpp - 4륜 모터 제어 클래스 구현
 */

#include "HuskyMobile.h"

HuskyMobile::HuskyMobile(uint8_t fl_in1, uint8_t fl_in2, uint8_t fl_en,
                         uint8_t fr_in1, uint8_t fr_in2, uint8_t fr_en,
                         uint8_t rl_in1, uint8_t rl_in2, uint8_t rl_en,
                         uint8_t rr_in1, uint8_t rr_in2, uint8_t rr_en) {
  // 핀 할당
  _fl_in1 = fl_in1; _fl_in2 = fl_in2; _fl_en = fl_en;
  _fr_in1 = fr_in1; _fr_in2 = fr_in2; _fr_en = fr_en;
  _rl_in1 = rl_in1; _rl_in2 = rl_in2; _rl_en = rl_en;
  _rr_in1 = rr_in1; _rr_in2 = rr_in2; _rr_en = rr_en;

  // 모든 핀을 OUTPUT으로 설정
  pinMode(_fl_in1, OUTPUT); pinMode(_fl_in2, OUTPUT); pinMode(_fl_en, OUTPUT);
  pinMode(_fr_in1, OUTPUT); pinMode(_fr_in2, OUTPUT); pinMode(_fr_en, OUTPUT);
  pinMode(_rl_in1, OUTPUT); pinMode(_rl_in2, OUTPUT); pinMode(_rl_en, OUTPUT);
  pinMode(_rr_in1, OUTPUT); pinMode(_rr_in2, OUTPUT); pinMode(_rr_en, OUTPUT);

  // 방향 초기화
  _left_advance = true;
  _left_back = false;
  _right_advance = true;
  _right_back = false;

  Stop();
}

void HuskyMobile::Direction(bool left, bool right) {
  if (left) {
    _left_advance = !_left_advance;
    _left_back = !_left_back;
  }
  if (right) {
    _right_advance = !_right_advance;
    _right_back = !_right_back;
  }
}

void HuskyMobile::Speed(int16_t left_speed, int16_t right_speed) {
  // 왼쪽 바퀴들 (Front Left + Rear Left)
  bool left_reverse = left_speed < 0;
  if (left_reverse) left_speed = -left_speed;
  if (left_speed > 255) left_speed = 255;

  runMotor(_fl_in1, _fl_in2, _fl_en, left_speed, left_reverse != _left_advance);
  runMotor(_rl_in1, _rl_in2, _rl_en, left_speed, left_reverse != _left_advance);

  // 오른쪽 바퀴들 (Front Right + Rear Right)
  bool right_reverse = right_speed < 0;
  if (right_reverse) right_speed = -right_speed;
  if (right_speed > 255) right_speed = 255;

  runMotor(_fr_in1, _fr_in2, _fr_en, right_speed, right_reverse != _right_advance);
  runMotor(_rr_in1, _rr_in2, _rr_en, right_speed, right_reverse != _right_advance);
}

void HuskyMobile::Speed4(int16_t fl, int16_t fr, int16_t rl, int16_t rr) {
  runMotor(_fl_in1, _fl_in2, _fl_en, abs(fl), fl < 0);
  runMotor(_fr_in1, _fr_in2, _fr_en, abs(fr), fr < 0);
  runMotor(_rl_in1, _rl_in2, _rl_en, abs(rl), rl < 0);
  runMotor(_rr_in1, _rr_in2, _rr_en, abs(rr), rr < 0);
}

void HuskyMobile::Forward(int16_t speed) {
  Speed(speed, speed);
}

void HuskyMobile::Backward(int16_t speed) {
  Speed(-speed, -speed);
}

void HuskyMobile::TurnLeft(int16_t speed) {
  Speed(-speed, speed);
}

void HuskyMobile::TurnRight(int16_t speed) {
  Speed(speed, -speed);
}

void HuskyMobile::Stop() {
  analogWrite(_fl_en, 0); analogWrite(_fr_en, 0);
  analogWrite(_rl_en, 0); analogWrite(_rr_en, 0);
  
  digitalWrite(_fl_in1, LOW); digitalWrite(_fl_in2, LOW);
  digitalWrite(_fr_in1, LOW); digitalWrite(_fr_in2, LOW);
  digitalWrite(_rl_in1, LOW); digitalWrite(_rl_in2, LOW);
  digitalWrite(_rr_in1, LOW); digitalWrite(_rr_in2, LOW);
}

void HuskyMobile::runMotor(uint8_t in1, uint8_t in2, uint8_t en, int16_t speed, bool reverse) {
  if (speed > 255) speed = 255;
  if (speed < 0) speed = 0;

  if (reverse) {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
  } else {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
  }
  analogWrite(en, speed);
}