/*
 * 라인 트레이싱 코드 - 개선된 버전
 * 정확한 90도 회전과 속도 제어
 * 코너 감지 및 회전 시간 제한
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
#define LEFT_THRESHOLD (SCREEN_WIDTH * 0.35)   // 35% = 112 (좀 더 좁게)
#define RIGHT_THRESHOLD (SCREEN_WIDTH * 0.65)  // 65% = 208 (좀 더 좁게)

// 속도 설정
#define FORWARD_SPEED_FAST 120     // 직선에서 빠른 속도
#define FORWARD_SPEED_SLOW 80      // 코너 근처에서 느린 속도
#define TURN_SPEED 180             // 회전 속도
#define SEARCH_SPEED 100           // 탐색 속도

// 회전 제어
#define TURN_DURATION 500          // 회전 시간 (ms) - 90도 회전 시간
#define CORNER_DETECTION_THRESHOLD 50  // 코너 감지 임계값

// 4륜 모터 초기화
HuskyMobile Robot(2, 3, 5,    // Front Left
                  7, 4, 6,    // Front Right  
                  A0, 8, 9,   // Rear Left
                  A1, A2, 10); // Rear Right

HUSKYLENS huskylens;
int ID1 = 1;

// 상태 변수
enum RobotState {
  FORWARD_FAST,
  FORWARD_SLOW,
  TURNING_LEFT,
  TURNING_RIGHT,
  SEARCH_LINE
};

RobotState currentState = SEARCH_LINE;
unsigned long lastLineTime = 0;
unsigned long turnStartTime = 0;
bool isTurning = false;
int lastLineCenter = SCREEN_CENTER;

#define LINE_TIMEOUT 1500

void setup() {
  Serial.begin(115200);
  
  Robot.Direction(false, false);
  
  Wire.begin();
  while (!huskylens.begin(Wire)) {
    Serial.println(F("HUSKYLENS 연결 실패!"));
    delay(1000);
  }
  
  huskylens.writeAlgorithm(ALGORITHM_LINE_TRACKING);
  Serial.println("개선된 라인 트레이싱 시작!");
  Serial.println(String() + "LEFT_THRESHOLD: " + LEFT_THRESHOLD);
  Serial.println(String() + "RIGHT_THRESHOLD: " + RIGHT_THRESHOLD);
  
  lastLineTime = millis();
}

void loop() {
  bool lineFound = false;
  int lineCenter = SCREEN_CENTER;
  
  // 회전 중이면 시간 체크
  if (isTurning && (millis() - turnStartTime > TURN_DURATION)) {
    isTurning = false;
    Serial.println("회전 완료!");
  }
  
  // 라인 감지
  if (huskylens.request(ID1) && huskylens.isLearned() && huskylens.available()) {
    HUSKYLENSResult result = huskylens.read();
    
    if (result.command == COMMAND_RETURN_ARROW) {
      lineCenter = result.xTarget;
      lineFound = true;
      lastLineTime = millis();
    }
    else if (result.command == COMMAND_RETURN_BLOCK) {
      lineCenter = result.xCenter;
      lineFound = true;
      lastLineTime = millis();
    }
  }
  
  // 모터 제어 로직
  if (isTurning) {
    // 회전 중이면 계속 회전
    if (currentState == TURNING_LEFT) {
      Robot.Speed(-TURN_SPEED, TURN_SPEED);
      Serial.println("좌회전 진행 중...");
    } else if (currentState == TURNING_RIGHT) {
      Robot.Speed(TURN_SPEED, -TURN_SPEED);
      Serial.println("우회전 진행 중...");
    }
  }
  else if (lineFound) {
    // 라인을 찾았을 때
    bool isNearCorner = (lineCenter < LEFT_THRESHOLD + CORNER_DETECTION_THRESHOLD) || 
                       (lineCenter > RIGHT_THRESHOLD - CORNER_DETECTION_THRESHOLD);
    
    if (lineCenter < LEFT_THRESHOLD) {
      // 좌회전 시작
      currentState = TURNING_LEFT;
      isTurning = true;
      turnStartTime = millis();
      Robot.Speed(-TURN_SPEED, TURN_SPEED);
      Serial.println("좌회전 시작!");
    }
    else if (lineCenter > RIGHT_THRESHOLD) {
      // 우회전 시작
      currentState = TURNING_RIGHT;
      isTurning = true;
      turnStartTime = millis();
      Robot.Speed(TURN_SPEED, -TURN_SPEED);
      Serial.println("우회전 시작!");
    }
    else {
      // 직진 - 코너 근처면 속도 감소
      if (isNearCorner) {
        currentState = FORWARD_SLOW;
        Robot.Speed(FORWARD_SPEED_SLOW, FORWARD_SPEED_SLOW);
        Serial.println("직진 (속도 감소 - 코너 근처)");
      } else {
        currentState = FORWARD_FAST;
        Robot.Speed(FORWARD_SPEED_FAST, FORWARD_SPEED_FAST);
        Serial.println("직진 (고속)");
      }
    }
    
    lastLineCenter = lineCenter;
  }
  else {
    // 라인을 못 찾았을 때
    if (millis() - lastLineTime > LINE_TIMEOUT) {
      // 탐색 모드
      currentState = SEARCH_LINE;
      static bool searchDirection = true;
      static unsigned long lastDirectionChange = 0;
      
      if (millis() - lastDirectionChange > 1000) {
        searchDirection = !searchDirection;
        lastDirectionChange = millis();
      }
      
      if (searchDirection) {
        Robot.Speed(-SEARCH_SPEED, SEARCH_SPEED);
        Serial.println("라인 탐색 중 - 좌회전");
      } else {
        Robot.Speed(SEARCH_SPEED, -SEARCH_SPEED);
        Serial.println("라인 탐색 중 - 우회전");
      }
    }
    else {
      // 최근에 라인을 봤으니 천천히 직진
      Robot.Speed(FORWARD_SPEED_SLOW, FORWARD_SPEED_SLOW);
      Serial.println("라인 없음 - 천천히 직진");
    }
  }
  
  // 디버그 정보
  printStatus(lineFound, lineCenter);
  
  delay(50);
}

void printStatus(bool lineFound, int lineCenter) {
  String stateStr;
  switch (currentState) {
    case FORWARD_FAST: stateStr = "고속직진"; break;
    case FORWARD_SLOW: stateStr = "저속직진"; break;
    case TURNING_LEFT: stateStr = "좌회전"; break;
    case TURNING_RIGHT: stateStr = "우회전"; break;
    case SEARCH_LINE: stateStr = "라인탐색"; break;
  }
  
  Serial.println(String() + "상태: " + stateStr + 
                 ", 라인: " + (lineFound ? "O" : "X") + 
                 ", 중심: " + lineCenter + 
                 " [" + LEFT_THRESHOLD + "-" + RIGHT_THRESHOLD + "]" +
                 (isTurning ? " (회전중)" : ""));
}