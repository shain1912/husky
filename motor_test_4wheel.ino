/*
 * 4륜 모터 테스트 코드 (방향 수정 버전)
 * 왼쪽앞, 오른쪽뒤: 방향 맞음
 * 오른쪽앞, 왼쪽뒤: 방향 반대 (핀 순서 변경)
 */

// 모터 핀 정의 (단순한 핀 배치)
#define FRONT_LEFT_IN1   2   // 왼쪽앞 - 방향 맞음
#define FRONT_LEFT_IN2   3
#define FRONT_LEFT_EN    5

#define FRONT_RIGHT_IN1  7   // 오른쪽앞 - 방향 반대 (핀 순서 바뀜)
#define FRONT_RIGHT_IN2  4
#define FRONT_RIGHT_EN   6

#define REAR_LEFT_IN1    8   // 왼쪽뒤 - 방향 반대 (핀 순서 바뀜)
#define REAR_LEFT_IN2    A0
#define REAR_LEFT_EN     9

#define REAR_RIGHT_IN1   A1  // 오른쪽뒤 - 방향 맞음
#define REAR_RIGHT_IN2   A2
#define REAR_RIGHT_EN    10

int motorSpeed = 200;

void setup() {
  // 모든 핀을 OUTPUT으로 설정
  pinMode(FRONT_LEFT_IN1, OUTPUT); pinMode(FRONT_LEFT_IN2, OUTPUT); pinMode(FRONT_LEFT_EN, OUTPUT);
  pinMode(FRONT_RIGHT_IN1, OUTPUT); pinMode(FRONT_RIGHT_IN2, OUTPUT); pinMode(FRONT_RIGHT_EN, OUTPUT);
  pinMode(REAR_LEFT_IN1, OUTPUT); pinMode(REAR_LEFT_IN2, OUTPUT); pinMode(REAR_LEFT_EN, OUTPUT);
  pinMode(REAR_RIGHT_IN1, OUTPUT); pinMode(REAR_RIGHT_IN2, OUTPUT); pinMode(REAR_RIGHT_EN, OUTPUT);
  
  Serial.begin(9600);
  Serial.println("4륜 모터 테스트 (방향 수정됨)");
  Serial.println("=============================");
  Serial.println("개별 테스트:");
  Serial.println("1: 왼쪽앞 | 2: 오른쪽앞");
  Serial.println("3: 왼쪽뒤 | 4: 오른쪽뒤");
  Serial.println("그룹 테스트:");
  Serial.println("f: 전진 | b: 후진");
  Serial.println("l: 좌회전 | r: 우회전");
  Serial.println("0: 모든 모터 정지");
  Serial.println("+/-: 속도 조절");
  
  stopAllMotors();
}

void loop() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    handleCommand(cmd);
  }
}

void handleCommand(char cmd) {
  stopAllMotors();
  delay(50);
  
  switch(cmd) {
    case '1':
      Serial.println(">> 왼쪽앞 모터 테스트");
      runMotor(FRONT_LEFT_IN1, FRONT_LEFT_IN2, FRONT_LEFT_EN, motorSpeed);
      break;
      
    case '2':
      Serial.println(">> 오른쪽앞 모터 테스트");
      runMotor(FRONT_RIGHT_IN1, FRONT_RIGHT_IN2, FRONT_RIGHT_EN, motorSpeed);
      break;
      
    case '3':
      Serial.println(">> 왼쪽뒤 모터 테스트");
      runMotor(REAR_LEFT_IN1, REAR_LEFT_IN2, REAR_LEFT_EN, motorSpeed);
      break;
      
    case '4':
      Serial.println(">> 오른쪽뒤 모터 테스트");
      runMotor(REAR_RIGHT_IN1, REAR_RIGHT_IN2, REAR_RIGHT_EN, motorSpeed);
      break;
      
    case 'f':
      Serial.println(">> 전진!");
      goForward();
      break;
      
    case 'b':
      Serial.println(">> 후진!");
      goBackward();
      break;
      
    case 'l':
      Serial.println(">> 좌회전!");
      turnLeft();
      break;
      
    case 'r':
      Serial.println(">> 우회전!");
      turnRight();
      break;
      
    case '0':
      Serial.println(">> 모든 모터 정지");
      stopAllMotors();
      break;
      
    case '+':
      motorSpeed += 30;
      if (motorSpeed > 255) motorSpeed = 255;
      Serial.print("속도: "); Serial.println(motorSpeed);
      break;
      
    case '-':
      motorSpeed -= 30;
      if (motorSpeed < 0) motorSpeed = 0;
      Serial.print("속도: "); Serial.println(motorSpeed);
      break;
  }
}

void runMotor(int in1, int in2, int en, int speed) {
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  analogWrite(en, speed);
}

void goForward() {
  runMotor(FRONT_LEFT_IN1, FRONT_LEFT_IN2, FRONT_LEFT_EN, motorSpeed);
  runMotor(FRONT_RIGHT_IN1, FRONT_RIGHT_IN2, FRONT_RIGHT_EN, motorSpeed);
  runMotor(REAR_LEFT_IN1, REAR_LEFT_IN2, REAR_LEFT_EN, motorSpeed);
  runMotor(REAR_RIGHT_IN1, REAR_RIGHT_IN2, REAR_RIGHT_EN, motorSpeed);
}

void goBackward() {
  // 후진은 모든 모터 반대 방향
  digitalWrite(FRONT_LEFT_IN1, LOW); digitalWrite(FRONT_LEFT_IN2, HIGH); analogWrite(FRONT_LEFT_EN, motorSpeed);
  digitalWrite(FRONT_RIGHT_IN1, LOW); digitalWrite(FRONT_RIGHT_IN2, HIGH); analogWrite(FRONT_RIGHT_EN, motorSpeed);
  digitalWrite(REAR_LEFT_IN1, LOW); digitalWrite(REAR_LEFT_IN2, HIGH); analogWrite(REAR_LEFT_EN, motorSpeed);
  digitalWrite(REAR_RIGHT_IN1, LOW); digitalWrite(REAR_RIGHT_IN2, HIGH); analogWrite(REAR_RIGHT_EN, motorSpeed);
}

void turnLeft() {
  // 왼쪽 바퀴 후진, 오른쪽 바퀴 전진
  digitalWrite(FRONT_LEFT_IN1, LOW); digitalWrite(FRONT_LEFT_IN2, HIGH); analogWrite(FRONT_LEFT_EN, motorSpeed);
  runMotor(FRONT_RIGHT_IN1, FRONT_RIGHT_IN2, FRONT_RIGHT_EN, motorSpeed);
  digitalWrite(REAR_LEFT_IN1, LOW); digitalWrite(REAR_LEFT_IN2, HIGH); analogWrite(REAR_LEFT_EN, motorSpeed);
  runMotor(REAR_RIGHT_IN1, REAR_RIGHT_IN2, REAR_RIGHT_EN, motorSpeed);
}

void turnRight() {
  // 왼쪽 바퀴 전진, 오른쪽 바퀴 후진
  runMotor(FRONT_LEFT_IN1, FRONT_LEFT_IN2, FRONT_LEFT_EN, motorSpeed);
  digitalWrite(FRONT_RIGHT_IN1, LOW); digitalWrite(FRONT_RIGHT_IN2, HIGH); analogWrite(FRONT_RIGHT_EN, motorSpeed);
  runMotor(REAR_LEFT_IN1, REAR_LEFT_IN2, REAR_LEFT_EN, motorSpeed);
  digitalWrite(REAR_RIGHT_IN1, LOW); digitalWrite(REAR_RIGHT_IN2, HIGH); analogWrite(REAR_RIGHT_EN, motorSpeed);
}

void stopAllMotors() {
  analogWrite(FRONT_LEFT_EN, 0); analogWrite(FRONT_RIGHT_EN, 0);
  analogWrite(REAR_LEFT_EN, 0); analogWrite(REAR_RIGHT_EN, 0);
  
  digitalWrite(FRONT_LEFT_IN1, LOW); digitalWrite(FRONT_LEFT_IN2, LOW);
  digitalWrite(FRONT_RIGHT_IN1, LOW); digitalWrite(FRONT_RIGHT_IN2, LOW);
  digitalWrite(REAR_LEFT_IN1, LOW); digitalWrite(REAR_LEFT_IN2, LOW);
  digitalWrite(REAR_RIGHT_IN1, LOW); digitalWrite(REAR_RIGHT_IN2, LOW);
}