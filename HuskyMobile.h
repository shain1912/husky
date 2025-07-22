/*
 * HuskyMobile.h - 4륜 모터 제어 클래스
 * DFMobile과 유사한 인터페이스로 4륜 모터 제어
 */

#ifndef HUSKYMOBILE_H
#define HUSKYMOBILE_H

#include <Arduino.h>

class HuskyMobile {
private:
  // 모터 핀 정의
  uint8_t _fl_in1, _fl_in2, _fl_en;  // Front Left
  uint8_t _fr_in1, _fr_in2, _fr_en;  // Front Right
  uint8_t _rl_in1, _rl_in2, _rl_en;  // Rear Left
  uint8_t _rr_in1, _rr_in2, _rr_en;  // Rear Right

  // 방향 설정
  bool _left_advance, _left_back;
  bool _right_advance, _right_back;

public:
  // 생성자: 모든 모터 핀을 초기화
  HuskyMobile(uint8_t fl_in1, uint8_t fl_in2, uint8_t fl_en,
              uint8_t fr_in1, uint8_t fr_in2, uint8_t fr_en,
              uint8_t rl_in1, uint8_t rl_in2, uint8_t rl_en,
              uint8_t rr_in1, uint8_t rr_in2, uint8_t rr_en);

  // DFMobile과 동일한 인터페이스
  void Direction(bool left, bool right);
  void Speed(int16_t left_speed, int16_t right_speed);

  // 추가 4륜 제어 함수
  void Speed4(int16_t fl, int16_t fr, int16_t rl, int16_t rr);
  void Forward(int16_t speed = 100);
  void Backward(int16_t speed = 100);
  void TurnLeft(int16_t speed = 100);
  void TurnRight(int16_t speed = 100);
  void Stop();

private:
  void runMotor(uint8_t in1, uint8_t in2, uint8_t en, int16_t speed, bool reverse = false);
};

#endif