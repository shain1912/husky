/*
 * 간단한 모터 테스트 코드
 * 하나의 모터만으로 연결 상태를 확인합니다.
 */

// 첫 번째 모터만 테스트
#define MOTOR_IN1  2
#define MOTOR_IN2  3
#define MOTOR_EN   5

void setup() {
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  pinMode(MOTOR_EN, OUTPUT);
  
  Serial.begin(9600);
  Serial.println("단일 모터 테스트 시작");
  Serial.println("1: 정방향, 2: 역방향, 0: 정지");
  
  // 모터 정지
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);
  analogWrite(MOTOR_EN, 0);
}

void loop() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    
    switch(cmd) {
      case '1':
        Serial.println("모터 정방향 회전 (속도 200)");
        digitalWrite(MOTOR_IN1, HIGH);
        digitalWrite(MOTOR_IN2, LOW);
        analogWrite(MOTOR_EN, 200);
        break;
        
      case '2':
        Serial.println("모터 역방향 회전 (속도 200)");
        digitalWrite(MOTOR_IN1, LOW);
        digitalWrite(MOTOR_IN2, HIGH);
        analogWrite(MOTOR_EN, 200);
        break;
        
      case '0':
        Serial.println("모터 정지");
        digitalWrite(MOTOR_IN1, LOW);
        digitalWrite(MOTOR_IN2, LOW);
        analogWrite(MOTOR_EN, 0);
        break;
        
      case '?':
        Serial.println("1: 정방향, 2: 역방향, 0: 정지");
        break;
    }
  }
}