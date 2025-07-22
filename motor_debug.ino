/*
 * 모터 문제 진단 코드
 * 단계별로 테스트하여 문제점 찾기
 */

// 간단한 핀 설정 (단일 모터 테스트했던 설정과 동일)
#define MOTOR1_IN1   2
#define MOTOR1_IN2   3
#define MOTOR1_EN    5

#define MOTOR2_IN1   4
#define MOTOR2_IN2   7
#define MOTOR2_EN    6

#define MOTOR3_IN1   8
#define MOTOR3_IN2   A0
#define MOTOR3_EN    9

#define MOTOR4_IN1   A1
#define MOTOR4_IN2   A2
#define MOTOR4_EN    10

int testSpeed = 150; // 낮은 속도로 시작

void setup() {
  // 모든 핀 초기화
  pinMode(MOTOR1_IN1, OUTPUT); pinMode(MOTOR1_IN2, OUTPUT); pinMode(MOTOR1_EN, OUTPUT);
  pinMode(MOTOR2_IN1, OUTPUT); pinMode(MOTOR2_IN2, OUTPUT); pinMode(MOTOR2_EN, OUTPUT);
  pinMode(MOTOR3_IN1, OUTPUT); pinMode(MOTOR3_IN2, OUTPUT); pinMode(MOTOR3_EN, OUTPUT);
  pinMode(MOTOR4_IN1, OUTPUT); pinMode(MOTOR4_IN2, OUTPUT); pinMode(MOTOR4_EN, OUTPUT);
  
  Serial.begin(9600);
  Serial.println("모터 문제 진단 테스트");
  Serial.println("====================");
  Serial.println("1: 모터1만 | 2: 모터1+2");
  Serial.println("3: 모터1+2+3 | 4: 모든 모터");
  Serial.println("a: 모터1 개별 | b: 모터2 개별");
  Serial.println("c: 모터3 개별 | d: 모터4 개별");
  Serial.println("0: 정지 | +/-: 속도조절");
  
  stopAllMotors();
  Serial.print("시작 속도: "); Serial.println(testSpeed);
}

void loop() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    handleCommand(cmd);
  }
}

void handleCommand(char cmd) {
  stopAllMotors();
  delay(100); // 충분한 딜레이
  
  switch(cmd) {
    case 'a':
      Serial.println(">> 모터1만 테스트");
      runSingleMotor(1);
      break;
      
    case 'b':
      Serial.println(">> 모터2만 테스트");
      runSingleMotor(2);
      break;
      
    case 'c':
      Serial.println(">> 모터3만 테스트");
      runSingleMotor(3);
      break;
      
    case 'd':
      Serial.println(">> 모터4만 테스트");
      runSingleMotor(4);
      break;
      
    case '1':
      Serial.println(">> 모터1만 구동");
      runMotor(MOTOR1_IN1, MOTOR1_IN2, MOTOR1_EN);
      break;
      
    case '2':
      Serial.println(">> 모터1+2 구동");
      runMotor(MOTOR1_IN1, MOTOR1_IN2, MOTOR1_EN);
      delay(50);
      runMotor(MOTOR2_IN1, MOTOR2_IN2, MOTOR2_EN);
      break;
      
    case '3':
      Serial.println(">> 모터1+2+3 구동");
      runMotor(MOTOR1_IN1, MOTOR1_IN2, MOTOR1_EN);
      delay(50);
      runMotor(MOTOR2_IN1, MOTOR2_IN2, MOTOR2_EN);
      delay(50);
      runMotor(MOTOR3_IN1, MOTOR3_IN2, MOTOR3_EN);
      break;
      
    case '4':
      Serial.println(">> 모든 모터 구동");
      Serial.println("전원 상태를 확인하세요!");
      runMotor(MOTOR1_IN1, MOTOR1_IN2, MOTOR1_EN);
      delay(50);
      runMotor(MOTOR2_IN1, MOTOR2_IN2, MOTOR2_EN);
      delay(50);
      runMotor(MOTOR3_IN1, MOTOR3_IN2, MOTOR3_EN);
      delay(50);
      runMotor(MOTOR4_IN1, MOTOR4_IN2, MOTOR4_EN);
      break;
      
    case '0':
      Serial.println(">> 모든 모터 정지");
      stopAllMotors();
      break;
      
    case '+':
      testSpeed += 30;
      if (testSpeed > 255) testSpeed = 255;
      Serial.print("속도: "); Serial.println(testSpeed);
      break;
      
    case '-':
      testSpeed -= 30;
      if (testSpeed < 50) testSpeed = 50;
      Serial.print("속도: "); Serial.println(testSpeed);
      break;
  }
}

void runSingleMotor(int motorNum) {
  switch(motorNum) {
    case 1: runMotor(MOTOR1_IN1, MOTOR1_IN2, MOTOR1_EN); break;
    case 2: runMotor(MOTOR2_IN1, MOTOR2_IN2, MOTOR2_EN); break;
    case 3: runMotor(MOTOR3_IN1, MOTOR3_IN2, MOTOR3_EN); break;
    case 4: runMotor(MOTOR4_IN1, MOTOR4_IN2, MOTOR4_EN); break;
  }
}

void runMotor(int in1, int in2, int en) {
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  analogWrite(en, testSpeed);
}

void stopAllMotors() {
  // PWM을 0으로
  analogWrite(MOTOR1_EN, 0); analogWrite(MOTOR2_EN, 0);
  analogWrite(MOTOR3_EN, 0); analogWrite(MOTOR4_EN, 0);
  
  // 모든 IN핀을 LOW로
  digitalWrite(MOTOR1_IN1, LOW); digitalWrite(MOTOR1_IN2, LOW);
  digitalWrite(MOTOR2_IN1, LOW); digitalWrite(MOTOR2_IN2, LOW);
  digitalWrite(MOTOR3_IN1, LOW); digitalWrite(MOTOR3_IN2, LOW);
  digitalWrite(MOTOR4_IN1, LOW); digitalWrite(MOTOR4_IN2, LOW);
}