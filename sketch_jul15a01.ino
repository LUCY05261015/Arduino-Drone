#include <MPU6050.h>

#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

const int riseButtonPin = 2;  // 상승버튼의 연결 핀 번호
const int signalPin = 7;      // MPU6050의 신호 핀으로 사용할 핀 번호

void setup() {
  Serial.begin(9600);
  
  // MPU6050 초기화
  Wire.begin();
  mpu.initialize();

  // 상승버튼의 핀 설정
  pinMode(riseButtonPin, INPUT);
  digitalWrite(riseButtonPin, HIGH); // 내부 풀업 저항 사용

  // 신호 핀 설정
  pinMode(signalPin, OUTPUT);
  digitalWrite(signalPin, LOW);  // 초기에는 LOW로 설정
}

void loop() {
  // MPU6050에서 가속도 데이터 읽기
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);
  
  // 가속도 데이터를 기반으로 신호 설정
  int signal = 0;

  // 앞, 뒤, 좌, 우 방향 기울기 판별
  if (ay < -5000) {   // 앞으로 많이 기울어짐
    signal = 4;
  } else if (ay > 5000) {  // 뒤로 많이 기울어짐
    signal = 5;
  } else if (ax < -5000) {  // 좌측으로 많이 기울어짐
    signal = 6;
  } else if (ax > 5000) {   // 우측으로 많이 기울어짐
    signal = 7;
  } else if (ay < -1500) {  // 앞으로 조금 기울어짐
    signal = 0;
  } else if (ay > 1500) {   // 뒤로 조금 기울어짐
    signal = 1;
  } else if (ax < -1500) {  // 좌측으로 조금 기울어짐
    signal = 2;
  } else if (ax > 1500) {   // 우측으로 조금 기울어짐
    signal = 3;
  }

  // 상승버튼 감지
  if (digitalRead(riseButtonPin) == LOW) {
    signal = 8;
  }

  // 신호 출력
  Serial.println(signal);
  digitalWrite(signalPin, signal);

  delay(100);  // 필요에 따라 딜레이 조정
}
