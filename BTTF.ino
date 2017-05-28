#include <OBD2UART.h>
#include "LowPower.h"

// Pin connected to ST_CP of 74HC595
#define LATCH_PIN 7
// Pin connected to SH_CP of 74HC595
#define CLOCK_PIN 8
// Pin connected to DS of 74HC595
#define DATA_PIN 6

// Number of digits attached
#define SHIFT_REGISTERS 2

COBD obd;

// Numbers mapped on our seven segment display
byte segmentNumbers[] = {0b11101101, 0b10000001, 0b11011100, 0b11010101, 0b10110001, 0b01110101, 0b01111101, 0b11000001, 0b11111101, 0b11110001};

// Internal Vars
byte regDataBuffer[SHIFT_REGISTERS];
int lastSpeed = 0;

void setup() {
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);

  // Init display, then clear
  showDigits(88);

  delay(1000);
  
  clearDisplay();

  // Wait for OBDII signal
  obd.begin();
  while (!obd.init());

  // Test display
  for (int i = 0; i <= 99; i++) {
    showDigits(i);
    delay(10);
  }
}

void loop() {
  // Try to read speed, otherwise just show a single dot. Deep sleep Arduino for 8 seconds to reduce power usage
  if (obd.readPID(PID_SPEED, lastSpeed)) {
    lastSpeed *= 0.621371;

    if (lastSpeed == 88) {
      clearDisplay();
      delay(300);
      showDigits(lastSpeed);
      delay(300);
      clearDisplay();
      delay(300);
    }
    
    showDigits(lastSpeed);
  } else {
    clearDisplay();
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  }

  delay(200);
}

// Show specified digits on display
void showDigits(int digit) {
  memset(regDataBuffer, 0, sizeof(regDataBuffer));

  int c = 0;

  if (digit == 0) {
    regDataBuffer[0] = segmentNumbers[0];
    regDataBuffer[1] = 0b00000000;
  }

  while ( digit > 0 ) {
    // Serial.println(digit % 10);
    byte b = segmentNumbers[digit % 10]; // modulus 10 of our input

    regDataBuffer[c] = b;

    digit /= 10;
    c++;
  }

  // Put a decimal after the second number
  regDataBuffer[0] |= 0b00000010;
  writeBuffer();
}

void writeBuffer() {
  digitalWrite(LATCH_PIN, LOW);
  
  for (int a = sizeof(regDataBuffer) - 1; a >= 0  ; a--) {
    shiftOut(regDataBuffer[a]);
  }

  digitalWrite(LATCH_PIN, HIGH);
}

// Show single dot on display
void clearDisplay() {
  digitalWrite(LATCH_PIN, LOW);
  shiftOut(0b00000000);
  shiftOut(0b00000010);
  digitalWrite(LATCH_PIN, HIGH);
}

// Shift bits to register
void shiftOut(byte myDataOut) {
  int i = 0;
  int pinState;

  //clear everything out just in case to
  //prepare shift register for bit shifting
  digitalWrite(DATA_PIN, LOW);
  digitalWrite(CLOCK_PIN, LOW);

  // Serial.println(myDataOut, BIN);

  for (i = 7; i >= 0; i--)  {
    digitalWrite(CLOCK_PIN, LOW);

    if (myDataOut & (1 << i) ) {
      pinState = HIGH;
    }
    else {
      pinState = LOW;
    }

    // Serial.print(pinState, BIN);
    digitalWrite(DATA_PIN, pinState);
    digitalWrite(CLOCK_PIN, HIGH);
    digitalWrite(DATA_PIN, LOW);
  }

  digitalWrite(CLOCK_PIN, LOW);
}
