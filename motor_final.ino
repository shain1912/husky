/*
 * 4륜 모터 최종 테스트 코드
 * 올바른 핀 배치로 수정됨
 */

// 모터 핀 정의 (사용자 테스트로 확인된 올바른 배치)
#define MOTOR1_IN1   2   // 바퀴1 (왼쪽앞)
#define MOTOR1_IN2   3
#define MOTOR1_EN    5

#define MOTOR2_IN1   7   // 바퀴2 (오른쪽앞)  
#define MOTOR2_IN2   4
#define MOTOR2_EN    6

#define MOTOR3_IN1   A0  // 바퀴3 (왼쪽뒤)
#define MOTOR3_IN2   8
#define MOTOR3_EN    9

#define MOTOR4_IN1   A1  // 바퀴4 (오른쪽뒤)
#define MOTOR4_IN2   A2
#define MOTOR4_EN    10

int motorSpeed = 100;

void setup() {
  // 모든 핀 초기화
  pinMode(2, OUTPUT); pinMode(3, OUTPUT); pinMode(5, OUTPUT);
  pinMode(4, OUTPUT); pinMode(7, OUTPUT); pinMode(6, OUTPUT);
  pinMode(8, OUTPUT); pinMode(A0, OUTPUT); pinMode(9, OUTPUT);
  pinMode(A1, OUTPUT); pinMode(A2, OUTPUT); pinMode(10, OUTPUT);
  
  Serial.begin(9600);
  Serial.println("4륜 모터 최종 테스트");
  Serial.println("===================");
  Serial.println("개별 테스트:");
  Serial.println("1: 바퀴1(왼쪽앞) | 2: 바퀴2(오른쪽앞)");
  Serial.println("3: 바퀴3(왼쪽뒤) | 4: 바퀴4(오른쪽뒤)");
  Serial.println("그룹 테스트:");
  Serial.println("f: 전진 | b: 후진");
  Serial.println("l: 좌회전 | r: 우회전");
  Serial.println("a: 모든 바퀴 정방향 | 0: 정지");
  Serial.println("+/-: 속도 조절");
  
  stopAllMotors();
  Serial.print("현재 속도: "); Serial.println(motorSpeed);
}

void loop() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    handleCommand(cmd);
  }
}

void handleCommand(char cmd) {
  switch(cmd) {
    case '1':
      Serial.println(">> 바퀴1 (왼쪽앞) 테스트");
      stopAllMotors();
      runMotor(MOTOR1_IN1, MOTOR1_IN2, MOTOR1_EN);
      break;
      
    case '2':
      Serial.println(">> 바퀴2 (오른쪽앞) 테스트");
      stopAllMotors();
      runMotor(MOTOR2_IN1, MOTOR2_IN2, MOTOR2_EN);
      break;
      
    case '3':
      Serial.println(">> 바퀴3 (왼쪽뒤) 테스트");
      stopAllMotors();
      runMotor(MOTOR3_IN1, MOTOR3_IN2, MOTOR3_EN);
      break;
      
    case '4':
      Serial.println(">> 바퀴4 (오른쪽뒤) 테스트");
      stopAllMotors();
      runMotor(MOTOR4_IN1, MOTOR4_IN2, MOTOR4_EN);
      break;
      
    case 'a':
      Serial.println(">> 모든 바퀴 정방향 회전");
      runAllMotorsForward();
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
      Serial.println(">> 정지");
      stopAllMotors();
      break;
      
    case '+':
      motorSpeed += 30;
      if (motorSpeed > 255) motorSpeed = 255;
      Serial.print("속도: "); Serial.println(motorSpeed);
      break;
      
    case '-':
      motorSpeed -= 30;
      if (motorSpeed < 50) motorSpeed = 50;
      Serial.print("속도: "); Serial.println(motorSpeed);
      break;
  }
}

void runMotor(int in1, int in2, int en) {
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  analogWrite(en, motorSpeed);
}

void runMotorReverse(int in1, int in2, int en) {
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  analogWrite(en, motorSpeed);
}

void runAllMotorsForward() {
  // 사용자가 확인한 대로 모든 바퀴를 정방향으로
  digitalWrite(2, HIGH); digitalWrite(3, LOW); analogWrite(5, motorSpeed);
  digitalWrite(7, HIGH); digitalWrite(4, LOW); analogWrite(6, motorSpeed);
  digitalWrite(A0, HIGH); digitalWrite(8, LOW); analogWrite(9, motorSpeed);
  digitalWrite(A1, HIGH); digitalWrite(A2, LOW); analogWrite(10, motorSpeed);
}

void goForward() {
  // 전진: 모든 바퀴 정방향
  runAllMotorsForward();
}

void goBackward() {
  // 후진: 모든 바퀴 역방향
  digitalWrite(2, LOW); digitalWrite(3, HIGH); analogWrite(5, motorSpeed);
  digitalWrite(7, LOW); digitalWrite(4, HIGH); analogWrite(6, motorSpeed);
  digitalWrite(A0, LOW); digitalWrite(8, HIGH); analogWrite(9, motorSpeed);
  digitalWrite(A1, LOW); digitalWrite(A2, HIGH); analogWrite(10, motorSpeed);
}

void turnLeft() {
  // 좌회전: 왼쪽 바퀴 역방향, 오른쪽 바퀴 정방향
  digitalWrite(2, LOW); digitalWrite(3, HIGH); analogWrite(5, motorSpeed);    // 왼쪽앞 역방향
  digitalWrite(7, HIGH); digitalWrite(4, LOW); analogWrite(6, motorSpeed);    // 오른쪽앞 정방향
  digitalWrite(A0, LOW); digitalWrite(8, HIGH); analogWrite(9, motorSpeed);   // 왼쪽뒤 역방향
  digitalWrite(A1, HIGH); digitalWrite(A2, LOW); analogWrite(10, motorSpeed); // 오른쪽뒤 정방향
}

void turnRight() {
  // 우회전: 왼쪽 바퀴 정방향, 오른쪽 바퀴 역방향
  digitalWrite(2, HIGH); digitalWrite(3, LOW); analogWrite(5, motorSpeed);    // 왼쪽앞 정방향
  digitalWrite(7, LOW); digitalWrite(4, HIGH); analogWrite(6, motorSpeed);    // 오른쪽앞 역방향
  digitalWrite(A0, HIGH); digitalWrite(8, LOW); analogWrite(9, motorSpeed);   // 왼쪽뒤 정방향
  digitalWrite(A1, LOW); digitalWrite(A2, HIGH); analogWrite(10, motorSpeed); // 오른쪽뒤 역방향
}

void stopAllMotors() {
  analogWrite(5, 0); analogWrite(6, 0); analogWrite(9, 0); analogWrite(10, 0);
  digitalWrite(2, LOW); digitalWrite(3, LOW);
  digitalWrite(4, LOW); digitalWrite(7, LOW);
  digitalWrite(8, LOW); digitalWrite(A0, LOW);
  digitalWrite(A1, LOW); digitalWrite(A2, LOW);
}