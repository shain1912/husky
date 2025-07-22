/*
 * 라인 트레이싱 코드 - PID 제어 버전
 * DFMobile 스타일 인터페이스 사용
 * HUSKYLENS를 이용한 라인 추적
 */

#include "HUSKYLENS.h"
#include "SoftwareSerial.h"
#include "PIDLoop.h"
#include "HuskyMobile.h"

#define HUSKY_FAST 120  // 기본 속도 (낮춤)

// 4륜 모터 초기화 (사용자가 확인한 핀 배치)
HuskyMobile Robot(3, 2, 5,    // Front Left: IN1, IN2, EN
                  4, 7, 6,    // Front Right: IN1, IN2, EN  
                  8, A0, 9,   // Rear Left: IN1, IN2, EN
                  A2, A1, 10); // Rear Right: IN1, IN2, EN

// PID 제어기 (P=2000, I=0, D=0, velocity mode)
PIDLoop headingLoop(2000, 0, 0, false);

HUSKYLENS huskylens;
int ID1 = 1;

void setup() {
  Serial.begin(115200);
  
  // 모터 방향 초기화 (필요시 조정)
  Robot.Direction(false, false);  // 기본 방향 설정
  
  // HUSKYLENS 초기화
  Wire.begin();
  while (!huskylens.begin(Wire)) {
    Serial.println(F("HUSKYLENS 연결 실패!"));
    Serial.println(F("1. 프로토콜 타입을 I2C로 설정하세요"));
    Serial.println(F("2. 연결을 다시 확인하세요"));
    delay(1000);
  }
  
  // 라인 트래킹 알고리즘으로 전환
  huskylens.writeAlgorithm(ALGORITHM_LINE_TRACKING);
  Serial.println("4륜 PID 라인 트레이싱 시작!");
  
  headingLoop.reset();
}

int left = 0, right = 0;

void loop() {
  int32_t error;
  
  if (!huskylens.request(ID1)) {
    Serial.println(F("HUSKYLENS 데이터 요청 실패!"));
    left = 0; 
    right = 0;
  }
  else if (!huskylens.isLearned()) {
    Serial.println(F("학습된 라인이 없습니다. 학습 버튼을 누르세요!"));
    left = 0; 
    right = 0;
  }
  else if (!huskylens.available()) {
    Serial.println(F("화면에 라인이 보이지 않습니다!"));
    left = HUSKY_FAST / 2;  // 천천히 전진하며 라인 찾기
    right = HUSKY_FAST / 2;
  }
  else {
    HUSKYLENSResult result = huskylens.read();
    printResult(result);
    
    // 에러 계산: 화면 중앙(160)에서 라인 중심까지의 거리
    error = (int32_t)result.xTarget - (int32_t)160;
    
    // PID 알고리즘 적용
    headingLoop.update(error);
    
    // 좌우 바퀴 속도 계산
    left = headingLoop.m_command;
    right = -headingLoop.m_command;
    
    // 기본 속도 추가
    left += HUSKY_FAST;
    right += HUSKY_FAST;
    
    // 속도 제한
    left = constrain(left, -255, 255);
    right = constrain(right, -255, 255);
  }
  
  // 디버그 출력
  Serial.println(String() + "Left: " + left + ", Right: " + right + ", Error: " + error);
  
  // 모터 제어
  Robot.Speed(left, right);
  
  delay(50);  // 제어 주기
}

void printResult(HUSKYLENSResult result) {
  if (result.command == COMMAND_RETURN_BLOCK) {
    Serial.println(String() + F("Block: x=") + result.xCenter + 
                   F(", y=") + result.yCenter + 
                   F(", w=") + result.width + 
                   F(", h=") + result.height);
  }
  else if (result.command == COMMAND_RETURN_ARROW) {
    Serial.println(String() + F("Arrow: xOrigin=") + result.xOrigin + 
                   F(", yOrigin=") + result.yOrigin + 
                   F(", xTarget=") + result.xTarget + 
                   F(", yTarget=") + result.yTarget);
  }
  else {
    Serial.println("Unknown object!");
  }
}