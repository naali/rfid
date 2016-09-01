#include <Wiegand.h>
#define PIN_LENGTH 10
#define MAX_USER_ACTION_DELAY_MS 10000
#define SERIAL_INPUT_BUFFER_LENGTH 100
#define PING_DELAY_MS 10000

#define KEY_OK_STRING "ok"
#define PING_STRING "ping"

#define KEY_OK_PIN 4

WIEGAND wg;
bool isPinCodeSet = false;
int pincode[PIN_LENGTH + 1];
int pincodeptr = 0;

bool isKeyCodeSet = false;
long lastKeyCode = 0;

char serialinputbuffer[SERIAL_INPUT_BUFFER_LENGTH];
int serialinputptr;

unsigned long lastUserAction = 0;
unsigned long lastPingSent = 0;

void setup() {
  Serial.begin(9600);

  resetPinCode();
  wg.begin();
  resetSerialInputBuffer();
  
  pinMode(KEY_OK_PIN, OUTPUT);
  digitalWrite(KEY_OK_PIN, HIGH);

  Serial.println("{ \"status\": \"running\" }");
}

void resetSerialInputBuffer() {
  for (int i=0; i<SERIAL_INPUT_BUFFER_LENGTH; i++) {
    serialinputbuffer[i] = 0;
  }
  
  serialinputptr = 0;
}

void resetPinCode() {
  for (int i=0; i<PIN_LENGTH + 1; i++) {
    pincode[i] = -1;
  }

  pincodeptr = 0;
  isPinCodeSet = false;
}

void printAndResetAccessCodes() {
    Serial.print("{ \"pincode\": \"");

    int ptr = 0;
    
    do {
      Serial.print(pincode[ptr]);
    } while (ptr++ < PIN_LENGTH && pincode[ptr] != -1);

    Serial.print("\", ");
    Serial.print("\"keycode\": \"");
    Serial.print(lastKeyCode);
    Serial.println("\" }");
      
    resetPinCode();
    isKeyCodeSet = false;
}

void processWG() {
  lastUserAction = millis();
  
  if (wg.getWiegandType() == 4) {
    int code = wg.getCode();
    if (code == 27) {
      resetPinCode();
    } else if (code == 13) {
      isPinCodeSet = true;
    } else {
      pincode[pincodeptr % PIN_LENGTH] = wg.getCode();
      pincodeptr++;
    }
  } else {
    lastKeyCode = wg.getCode();
    isKeyCodeSet = true;
  }

  if (isPinCodeSet && isKeyCodeSet) {
    printAndResetAccessCodes();
  }
}

void processSerial() {
  char c = Serial.read();
  
  if (c == '\n') {
    if (strncmp(serialinputbuffer, KEY_OK_STRING, strlen(KEY_OK_STRING)) == 0) {
      digitalWrite(KEY_OK_PIN, LOW);
      delay(500);
      digitalWrite(KEY_OK_PIN, HIGH);
    } else if (strncmp(serialinputbuffer, PING_STRING, strlen(PING_STRING)) == 0) {
      Serial.println("was ping");
    }
    
    resetSerialInputBuffer();
  } else {
    if (serialinputptr >= SERIAL_INPUT_BUFFER_LENGTH) {
      resetSerialInputBuffer();
    } else {
      serialinputbuffer[serialinputptr++] = c;
    }
  }
}

void loop() {
  if (wg.available()) {
    processWG();
  }

  if (Serial.available()) {
    processSerial();
  }

  if (lastUserAction + MAX_USER_ACTION_DELAY_MS <= millis()) {
    lastUserAction = millis();
    resetPinCode();
    isKeyCodeSet = false;
  }

  if (lastPingSent + PING_DELAY_MS <= millis()) {
    lastPingSent = millis();
    Serial.print("{ \"ping\": ");
    Serial.print(lastPingSent);
    Serial.println(" }");
  }

}