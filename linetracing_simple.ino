/*
 * 라인 트레이싱 코드 - 단순 임계값 버전
 * 검은선 중앙값이 화면 너비의 30%-70% 범위를 벗어나면 회전
 * HUSKYLENS를 이용한 간단한 라인 추적
 */

#include "HUSKYLENS.h"
#include "SoftwareSerial.h"

// 4륜 모터 제어 클래스 (인라인 구현)
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

  void Direction(bool left, bool right) {
    // 방향 설정 (필요시 구현)
  }

  void Speed(int16_t left_speed, int16_t right_speed) {
    // 모터 보상 적용
    left_speed = applyMotorCompensation(left_speed);
    right_speed = applyMotorCompensation(right_speed);
    
    // 왼쪽 바퀴들
    runMotor(_fl_in1, _fl_in2, _fl_en, left_speed);
    runMotor(_rl_in1, _rl_in2, _rl_en, left_speed);
    
    // 오른쪽 바퀴들
    runMotor(_fr_in1, _fr_in2, _fr_en, right_speed);
    runMotor(_rr_in1, _rr_in2, _rr_en, right_speed);
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
  // 모터 보상 함수 - 마찰력 극복을 위한 초기 부스트
  int16_t applyMotorCompensation(int16_t speed) {
    if (speed == 0) return 0;
    
    int16_t compensated_speed = speed;
    
    // 방법 1: 최소 PWM 보장 (마찰력 극복)
    int16_t min_pwm = 60;  // 최소 PWM 값
    
    if (abs(speed) > 0 && abs(speed) < min_pwm) {
      compensated_speed = (speed > 0) ? min_pwm : -min_pwm;
    }
    
    // 방법 2: 초기 킥 스타트 (처음 시작시 높은 PWM)
    static unsigned long last_speed_change[4] = {0, 0, 0, 0}; 
    static int16_t last_speeds[4] = {0, 0, 0, 0};
    static int motor_index = 0; // 간단하게 처리
    
    if (speed != last_speeds[motor_index] && abs(speed) > 20) {
      // 속도가 변했고 0이 아니면 초기 부스트 적용
      if (millis() - last_speed_change[motor_index] < 1000) { // 200ms 동안
        compensated_speed = speed + ((speed > 0) ? 50 : -50); // 초기 부스트
      }
      last_speed_change[motor_index] = millis();
    }
    
    last_speeds[motor_index] = speed;
    motor_index = (motor_index + 1) % 4;
    
    // 최종 범위 제한
    if (compensated_speed > 255) compensated_speed = 255;
    if (compensated_speed < -255) compensated_speed = -255;
    
    return compensated_speed;
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

// 화면 설정
#define SCREEN_WIDTH 320
#define SCREEN_CENTER 160
#define LEFT_THRESHOLD (SCREEN_WIDTH * 0.30)   // 30% = 96
#define RIGHT_THRESHOLD (SCREEN_WIDTH * 0.70)  // 70% = 224

// 속도 설정
#define FORWARD_SPEED 100
#define TURN_SPEED 190    // 회전 속도 증가 (80 → 150)
#define SEARCH_SPEED 190

// 4륜 모터 초기화
HuskyMobile Robot(2, 3, 5,    // Front Left
                  7, 4, 6,    // Front Right  
                  A0, 8, 9,   // Rear Left
                  A1, A2, 10); // Rear Right

HUSKYLENS huskylens;
int ID1 = 1;

// 상태 변수
enum RobotState {
  FORWARD,
  TURN_LEFT,
  TURN_RIGHT,
  SEARCH_LINE
};

RobotState currentState = SEARCH_LINE;
unsigned long lastLineTime = 0;
#define LINE_TIMEOUT 1000  // 1초 동안 라인을 못 찾으면 탐색 모드

void setup() {
  Serial.begin(115200);
  
  // 모터 방향 초기화
  Robot.Direction(false, false);
  
  // HUSKYLENS 초기화
  Wire.begin();
  while (!huskylens.begin(Wire)) {
    Serial.println(F("HUSKYLENS 연결 실패!"));
    delay(1000);
  }
  
  huskylens.writeAlgorithm(ALGORITHM_LINE_TRACKING);
  Serial.println("단순 임계값 라인 트레이싱 시작!");
  Serial.println(String() + "LEFT_THRESHOLD: " + LEFT_THRESHOLD);
  Serial.println(String() + "RIGHT_THRESHOLD: " + RIGHT_THRESHOLD);
  
  lastLineTime = millis();
}

void loop() {
  bool lineFound = false;
  int lineCenter = SCREEN_CENTER;
  
  if (huskylens.request(ID1) && huskylens.isLearned() && huskylens.available()) {
    HUSKYLENSResult result = huskylens.read();
    
    if (result.command == COMMAND_RETURN_ARROW) {
      lineCenter = result.xTarget;
      lineFound = true;
      lastLineTime = millis();
      
      Serial.println(String() + "라인 발견! 중심: " + lineCenter + 
                     " (목표: " + result.xTarget + ")");
    }
    else if (result.command == COMMAND_RETURN_BLOCK) {
      lineCenter = result.xCenter;
      lineFound = true;
      lastLineTime = millis();
      
      Serial.println(String() + "라인 블록 발견! 중심: " + lineCenter);
    }
  }
  
  // 상태 결정 및 모터 제어
  if (lineFound) {
    // 라인을 찾았을 때의 제어
    if (lineCenter < LEFT_THRESHOLD) {
      // 라인이 왼쪽에 있음 - 좌회전
      currentState = TURN_LEFT;
      Robot.Speed(-TURN_SPEED, TURN_SPEED);
      Serial.println("좌회전 (라인이 너무 왼쪽)");
    }
    else if (lineCenter > RIGHT_THRESHOLD) {
      // 라인이 오른쪽에 있음 - 우회전
      currentState = TURN_RIGHT;
      Robot.Speed(TURN_SPEED, -TURN_SPEED);
      Serial.println("우회전 (라인이 너무 오른쪽)");
    }
    else {
      // 라인이 중앙 범위에 있음 - 직진
      currentState = FORWARD;
      Robot.Speed(FORWARD_SPEED, FORWARD_SPEED);
      Serial.println("직진 (라인이 중앙 범위)");
    }
  }
  else {
    // 라인을 못 찾았을 때
    if (millis() - lastLineTime > LINE_TIMEOUT) {
      // 너무 오래 라인을 못 찾음 - 탐색 모드
      currentState = SEARCH_LINE;
      static bool searchDirection = true;
      
      if (searchDirection) {
        Robot.Speed(-SEARCH_SPEED, SEARCH_SPEED);  // 좌회전하며 탐색
        Serial.println("라인 탐색 중 - 좌회전");
      } else {
        Robot.Speed(SEARCH_SPEED, -SEARCH_SPEED);  // 우회전하며 탐색
        Serial.println("라인 탐색 중 - 우회전");
      }
      
      // 2초마다 탐색 방향 바꾸기
      static unsigned long lastDirectionChange = 0;
      if (millis() - lastDirectionChange > 2000) {
        searchDirection = !searchDirection;
        lastDirectionChange = millis();
      }
    }
    else {
      // 최근에 라인을 봤으니 잠깐 직진
      currentState = FORWARD;
      Robot.Speed(FORWARD_SPEED / 2, FORWARD_SPEED / 2);
      Serial.println("라인 없음 - 천천히 직진");
    }
  }
  
  // 상태 출력
  printStatus(lineFound, lineCenter);
  
  delay(100);  // 제어 주기
}

void printStatus(bool lineFound, int lineCenter) {
  String stateStr;
  switch (currentState) {
    case FORWARD: stateStr = "직진"; break;
    case TURN_LEFT: stateStr = "좌회전"; break;
    case TURN_RIGHT: stateStr = "우회전"; break;
    case SEARCH_LINE: stateStr = "라인탐색"; break;
  }
  
  Serial.println(String() + "상태: " + stateStr + 
                 ", 라인발견: " + (lineFound ? "O" : "X") + 
                 ", 중심: " + lineCenter + 
                 " [" + LEFT_THRESHOLD + "-" + RIGHT_THRESHOLD + "]");
}