#include <Wire.h>
#include "HUSKYLENS.h"

HUSKYLENS huskylens;
int HUSKY_ID = 1;

void setup() {
  Serial.begin(115200);
  Wire.begin();

  Serial.println("HUSKYLENS 디버그 시작...");
  
  // HUSKYLENS 초기화
  while (!huskylens.begin(Wire)) {
    Serial.println("HUSKYLENS 연결 실패 - 재시도 중...");
    delay(1000);
  }
  
  huskylens.writeAlgorithm(ALGORITHM_LINE_TRACKING);
  Serial.println("HUSKYLENS 연결 성공!");
  Serial.println("라인트래킹 모드 설정 완료!");
  Serial.println("");
  Serial.println("=== 디버그 정보 ===");
  Serial.println("1. HUSKYLENS 화면에 검은 라인이 보이는지 확인");
  Serial.println("2. 라인 위에 사각형(BLOCK) 또는 화살표(ARROW)가 표시되는지 확인");
  Serial.println("3. 만약 아무것도 표시 안되면 HUSKYLENS 학습 버튼을 눌러 라인 학습");
  Serial.println("");
}

void loop() {
  delay(500);  // 0.5초마다 체크
  
  Serial.println("--- 체크 시작 ---");
  
  // 1단계: request 체크
  if (!huskylens.request()) {
    Serial.println("❌ Step 1 실패: huskylens.request() - HUSKYLENS 통신 문제");
    return;
  }
  Serial.println("✅ Step 1 성공: huskylens.request() - 통신 OK");
  
  // 2단계: isLearned 체크  
  if (!huskylens.isLearned()) {
    Serial.println("❌ Step 2 실패: huskylens.isLearned() - 학습 안됨");
    Serial.println("   → HUSKYLENS에서 학습 버튼을 눌러 라인을 학습하세요!");
    return;
  }
  Serial.println("✅ Step 2 성공: huskylens.isLearned() - 학습 완료");
  
  // 3단계: available 체크
  if (!huskylens.available()) {
    Serial.println("❌ Step 3 실패: huskylens.available() - 검출된 객체 없음");
    Serial.println("   → 라인이 HUSKYLENS 화면 중앙에 오도록 위치 조정하세요");
    return;
  }
  Serial.println("✅ Step 3 성공: huskylens.available() - 객체 검출됨");
  
  // 4단계: 결과 읽기
  HUSKYLENSResult result = huskylens.read();
  
  Serial.println("=== 검출된 객체 정보 ===");
  Serial.print("Command: ");
  if (result.command == COMMAND_RETURN_BLOCK) {
    Serial.println("BLOCK (사각형)");
    Serial.print("  중심: ("); Serial.print(result.xCenter); 
    Serial.print(", "); Serial.print(result.yCenter); Serial.println(")");
    Serial.print("  크기: "); Serial.print(result.width);
    Serial.print(" x "); Serial.println(result.height);
    
    // 오차 계산
    int error = result.xCenter - 160;
    Serial.print("  오차: "); Serial.print(error);
    if (error > 0) Serial.println(" (오른쪽으로 치우침)");
    else if (error < 0) Serial.println(" (왼쪽으로 치우침)");
    else Serial.println(" (중앙)");
    
  } else if (result.command == COMMAND_RETURN_ARROW) {
    Serial.println("ARROW (화살표)");
    Serial.print("  시작: ("); Serial.print(result.xOrigin);
    Serial.print(", "); Serial.print(result.yOrigin); Serial.println(")");
    Serial.print("  끝점: ("); Serial.print(result.xTarget);
    Serial.print(", "); Serial.print(result.yTarget); Serial.println(")");
    
    // 오차 계산 (화살표 끝점 기준)
    int error = result.xTarget - 160;
    Serial.print("  오차: "); Serial.print(error);
    if (error > 0) Serial.println(" (오른쪽으로 치우침)");
    else if (error < 0) Serial.println(" (왼쪽으로 치우침)");
    else Serial.println(" (중앙)");
    
  } else {
    Serial.println("UNKNOWN - 알 수 없는 객체");
  }
  
  Serial.print("ID: "); Serial.println(result.ID);
  Serial.println("🎉 모든 단계 성공! 라인 검출이 정상 작동 중입니다.");
  Serial.println("");
}