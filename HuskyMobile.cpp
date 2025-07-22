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
  // 4륜 차량용 스무스 회전 알고리즘
  // 회전시 안쪽 바퀴는 느리게, 바깥쪽 바퀴는 빠르게
  
  int16_t fl_speed, fr_speed, rl_speed, rr_speed;
  
  if (left_speed == right_speed) {
    // 직진: 모든 바퀴 같은 속도
    fl_speed = fr_speed = rl_speed = rr_speed = left_speed;
  } else {
    // 회전: 차동 제어 적용
    int16_t speed_diff = right_speed - left_speed;
    int16_t base_speed = (left_speed + right_speed) / 2;
    
    // 회전 반경을 고려한 속도 분배
    // 앞바퀴와 뒷바퀴 간격으로 인한 속도 차이 보정
    float turn_factor = 0.7;  // 회전 시 속도 차이 조정 (0.5~1.0)
    
    if (speed_diff > 0) {
      // 우회전: 왼쪽 바퀴들 느리게, 오른쪽 바퀴들 빠르게
      fl_speed = left_speed;
      rl_speed = left_speed * turn_factor;  // 뒷바퀴는 더 느리게
      fr_speed = right_speed;
      rr_speed = right_speed * turn_factor;
    } else {
      // 좌회전: 오른쪽 바퀴들 느리게, 왼쪽 바퀴들 빠르게  
      fr_speed = right_speed;
      rr_speed = right_speed * turn_factor;  // 뒷바퀴는 더 느리게
      fl_speed = left_speed;
      rl_speed = left_speed * turn_factor;
    }
  }
  
  // 각 바퀴 독립 제어
  runMotor(_fl_in1, _fl_in2, _fl_en, abs(fl_speed), fl_speed < 0);
  runMotor(_fr_in1, _fr_in2, _fr_en, abs(fr_speed), fr_speed < 0);
  runMotor(_rl_in1, _rl_in2, _rl_en, abs(rl_speed), rl_speed < 0);
  runMotor(_rr_in1, _rr_in2, _rr_en, abs(rr_speed), rr_speed < 0);
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

void HuskyMobile::SmoothTurn(int16_t forward_speed, float turn_ratio) {
  // turn_ratio: -1.0(완전좌회전) ~ 0(직진) ~ 1.0(완전우회전)
  
  int16_t left_speed = forward_speed * (1.0 - turn_ratio);
  int16_t right_speed = forward_speed * (1.0 + turn_ratio);
  
  // 4륜 차동 제어로 스무스한 회전
  Speed(left_speed, right_speed);
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