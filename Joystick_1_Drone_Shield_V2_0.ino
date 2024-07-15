#include <SoftwareSerial.h>

#define OPERATION_STEP_0  0
#define OPERATION_STEP_1  1
#define OPERATION_STEP_2  2
#define OPERATION_STEP_3  3
#define OPERATION_STEP_4  4
#define OPERATION_STEP_5  5
#define OPERATION_STEP_6  6
#define OPERATION_STEP_7  7
#define OPERATION_STEP_8  8
#define OPERATION_STEP_9  9
#define OPERATION_STEP_10  10

#define OPERATION_STEP_ERROR  100

String uartString = "";
unsigned int currentStep;
unsigned int oldStep;

SoftwareSerial bleSerial(A0, A1); // RX, TX

// for Drone
unsigned char startBit = 0xf0;
unsigned char commandBit = 0xa1;
unsigned char roll = 100;
unsigned char pitch = 70;
unsigned char yaw = 100;
unsigned char throttle = 0;
unsigned char operationBit = 0x05;
unsigned char checkSum = 0;

unsigned int firstRoll;
unsigned int firstPitch;

void initUart() {
  uartString = "";
}

void initFlag() {
  currentStep = OPERATION_STEP_1;
  oldStep = OPERATION_STEP_1;
}

void checkNextStep() {
  oldStep = currentStep;
  currentStep = OPERATION_STEP_0;
}

void returnOldStep() {
  currentStep = oldStep;
  currentStep++;
}

void checkCrLfProcess() {
  while (bleSerial.available()) {
    char inChar = bleSerial.read();
    uartString += inChar;
    if (uartString.length() > 4 && uartString.startsWith("\r\n") && uartString.endsWith("\r\n")) {
      returnOldStep();
      break;
    }
  }
}

void sendDroneCommand() {
  if (throttle == 0) {
    roll = 100;
    pitch = 100;
    yaw = 100;
  }
  
  bleSerial.print("at+writeh000d");
  bleSerial.print(String(startBit, HEX));
  bleSerial.print(String(commandBit, HEX));
  
  if (roll < 0x10)
    bleSerial.print("0" + String(roll, HEX));
  else 
    bleSerial.print(String(roll, HEX));
  
  if (pitch < 0x10)
    bleSerial.print("0" + String(pitch, HEX));
  else 
    bleSerial.print(String(pitch, HEX));
  
  if (yaw < 0x10)
    bleSerial.print("0" + String(yaw, HEX));
  else 
    bleSerial.print(String(yaw, HEX));
  
  if (throttle < 0x10)
    bleSerial.print("0" + String(throttle, HEX));
  else 
    bleSerial.print(String(throttle, HEX));
  
  bleSerial.print("0" + String(operationBit, HEX));
  
  checkSum = commandBit + roll + pitch + yaw + throttle + operationBit;
  checkSum = checkSum & 0x00ff;
  
  if (checkSum < 0x10)
    bleSerial.print("0" + String(checkSum, HEX));
  else 
    bleSerial.print(String(checkSum, HEX));
  
  bleSerial.print("\r");
}

void setup() {
  Serial.begin(9600);
  Serial.println("Test Started!");
  bleSerial.begin(9600);
  initFlag();
  initUart();

  for (int i = 5; i < 11; i++) {
    pinMode(i, INPUT);
    digitalWrite(i, HIGH);
  }
}

void loop() {
  switch (currentStep) {
    case OPERATION_STEP_0:
      checkCrLfProcess();
      break;
      
    case OPERATION_STEP_1:
      delay(2000);
      bleSerial.flush();
      initFlag();
      initUart();
      while (bleSerial.available()) {
        bleSerial.read();
      }
      currentStep++;
      break;
      
    case OPERATION_STEP_2:
      // Wait until press start button
      if (!digitalRead(9)) {
        firstRoll = analogRead(4);
        firstPitch = analogRead(5);
        currentStep++;
      }
      break;
      
    case OPERATION_STEP_3:
      bleSerial.print("atd");
      bleSerial.print("083a5c1f5072");
      bleSerial.print("\r");
      checkNextStep();
      break;
      
    case OPERATION_STEP_4:
      if (uartString.equals("\r\nOK\r\n")) {
        Serial.println("Wait Connect");
        delay(300);
        initUart();
        checkNextStep();
      } else {
        Serial.println("CONNECT 1 ERROR");
        initUart();
        currentStep = OPERATION_STEP_ERROR;
      }
      break;
      
    case OPERATION_STEP_5:
      if (uartString.startsWith("\r\nCONNECT ")) {
        Serial.println("CONNECT OK");
        delay(300);
        initUart();
        currentStep++;
      } else {
        Serial.println("CONNECT 2 ERROR");
        initUart();
        currentStep = OPERATION_STEP_ERROR;
      }
      break;
      
    case OPERATION_STEP_6: {
      // Receive signal from Arduino A
      int signal = atoi(uartString.c_str());
      
      // Interpret signal and set drone commands
      switch (signal) {
        case 0:
          pitch = 70;  // 앞으로 적당한 속도로 이동
          break;
        case 1:
          pitch = 130; // 뒤로 적당한 속도로 이동
          break;
        case 2:
          roll = 85;   // 좌측으로 적당한 속도로 이동
          break;
        case 3:
          roll = 115;  // 우측으로 적당한 속도로 이동
          break;
        case 4:
          pitch = 50;  // 앞으로 조금 더 빠른 속도로 이동
          break;
        case 5:
          pitch = 150; // 뒤로 조금 더 빠른 속도로 이동
          break;
        case 6:
          roll = 75;   // 좌측으로 조금 더 빠른 속도로 이동
          break;
        case 7:
          roll = 125;  // 우측으로 조금 더 빠른 속도로 이동
          break;
        case 8:
          throttle = 120; // 드론 상승
          break;
        default:
          // Handle unexpected signals or errors
          break;
      }
      
      // Clear UART buffer and move to next step
      uartString = "";
      checkNextStep();
      break;
    }
      
    case OPERATION_STEP_7:
      // Send drone commands
      sendDroneCommand();
      delay(10);
      
      // Request Disconnect
      if (!digitalRead(10)) {
        Serial.println("REQUEST DISCONNECT");
        delay(300);
        initUart();
        currentStep++;
      }
      break;
      
    case OPERATION_STEP_8:
      delay(1000);
      bleSerial.flush();
      initUart();
      while (bleSerial.available()) {
        bleSerial.read();
      }
      initUart();
      bleSerial.print("ath\r");
      checkNextStep();
      break;
      
    case OPERATION_STEP_9:
      if (uartString.equals("\r\nOK\r\n")) {
        Serial.println("Wait Disconnect");
        delay(300);
        initUart();
        checkNextStep();
      } else {
        Serial.println("DISCONNECT 1 ERROR");
        initUart();
        currentStep = OPERATION_STEP_ERROR;
      }
      break;
      
    case OPERATION_STEP_10:
      if (uartString.startsWith("\r\nDISCONNECT")) {
        Serial.println("DISCONNECT 1 OK");
        delay(300);
        initUart();
        checkNextStep();
      } else {
        Serial.println("DISCONNECT 2 ERROR");
        initUart();
        currentStep = OPERATION_STEP_ERROR;
      }
      break;
      
    default:
      if (bleSerial.available())
        Serial.write(bleSerial.read());
      if (Serial.available())
        bleSerial.write(Serial.read());
      break;
  }
}
