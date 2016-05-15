#include <Wiegand.h>
#define PIN_LENGTH 10
#define MAX_USER_ACTION_DELAY_MS 10000

WIEGAND wg;
bool isPinCodeSet = false;
int pincode[PIN_LENGTH + 1];
int pincodeptr = 0;

bool isKeyCodeSet = false;
long lastKeyCode = 0;

unsigned long lastUserAction = 0;

void setup() {
  Serial.begin(9600);
  resetPinCode();
  wg.begin();
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

void loop() {
  if (wg.available()) {
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

  if (Serial.available()) {
    char c = Serial.read();
    Serial.print(c);
  }

  if (lastUserAction + MAX_USER_ACTION_DELAY_MS < millis()) {
    lastUserAction = millis();
    resetPinCode();
    isKeyCodeSet = false;
  }
}
