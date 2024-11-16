#define BLYNK_TEMPLATE_ID "TMPL3PRwEd-Vy"
#define BLYNK_TEMPLATE_NAME "Door Lock"
#define BLYNK_AUTH_TOKEN "z6S_kmx30rC9cx2t_AOYH5K0rCB_obWr"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>

// Blynk settings
char auth[] = BLYNK_AUTH_TOKEN;

// Define keypad layout and pins
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {15, 2, 4, 5};
byte colPins[COLS] = {18, 19, 27, 23};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Define potentiometer pin
const int potPin = A0;

// Define servo settings
Servo myServo;
const int servoPin = 14;
const int unlockPosition = -90; // Set servo position to -90 degrees
const int lockPosition = 90;    // Set servo position to +90 degrees

// Define LCD settings
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Variables
bool locked = true;
char enteredCode[5]; // To store the entered code
int codeIndex = 0;

// Blynk setup
BlynkTimer timer;
BLYNK_WRITE(V2) {
  int switchState = param.asInt();
  if (switchState == HIGH) {
    unlockDoor();
  } else {
    lockDoor();
  }
}

void setup() {
  Serial.begin(9600);

  Blynk.begin(auth, "Wokwi-GUEST", "");

  myServo.attach(servoPin);

  lcd.init();
  lcd.backlight();

  updateLockStatus();

  timer.setInterval(1000L, updateBlynk);
}

void updateBlynk() {
  Blynk.virtualWrite(V2, locked ? 0 : 1); // Update switch widget state
}

void loop() {
  Blynk.run();
  timer.run();

  char key = keypad.getKey();
  if (key) {
    handleKeypadInput(key);
  }
}

void updateLockStatus() {
  if (locked) {
    lcd.clear();
    lcd.print("Door Locked");
    Blynk.virtualWrite(V0, 0); // LED off for locked state
    Blynk.virtualWrite(V1, "Door Locked");
  } else {
    lcd.clear();
    lcd.print("Door Unlocked");
    Blynk.virtualWrite(V0, 255); // LED on for unlocked state
    Blynk.virtualWrite(V1, "Door Unlocked");
  }
}

void handleKeypadInput(char key) {
  if (locked) {
    if (key == '#' && codeIndex > 0) { // Check for Enter key
      enteredCode[codeIndex] = '\0'; // Null-terminate the entered code
      codeIndex = 0;

      if (strcmp(enteredCode, "1234") == 0) { // Check if the entered code is correct
        unlockDoor();
        lcd.clear();
        lcd.print("Door Unlocked");
        Blynk.virtualWrite(V1, "Door Unlocked");
        Blynk.virtualWrite(V2, 1); // Set switch widget ON
      } else {
        lcd.clear();
        lcd.print("Incorrect Pin!");
        delay(2000); // Display the message for 2 seconds
        lcd.clear();
        lcd.print("Door Locked");
        Blynk.virtualWrite(V1, "Incorrect Pin!");
      }

      // Clear entered code
      memset(enteredCode, 0, sizeof(enteredCode));
    } else if (key == 'C' && codeIndex > 0) { // Check for 'C' key to delete
      lcd.setCursor(codeIndex - 1, 1);
      lcd.print(' ');
      codeIndex--;
      enteredCode[codeIndex] = '\0';
    } else if (key != '#' && key != 'C' && codeIndex < sizeof(enteredCode) - 1) {
      enteredCode[codeIndex] = key;
      lcd.setCursor(codeIndex, 1);
      lcd.print('*');
      codeIndex++;
    }
  } else {
    if (key == '*') {
      lockDoor();
      lcd.clear();
      lcd.print("Door Locked");
      Blynk.virtualWrite(V1, "Door Locked");
      Blynk.virtualWrite(V2, 0); // Set switch widget OFF
    }
  }
}

void unlockDoor() {
  locked = false;
  myServo.write(unlockPosition);
  updateLockStatus();
}

void lockDoor() {
  locked = true;
  myServo.write(lockPosition);
  updateLockStatus();
}
