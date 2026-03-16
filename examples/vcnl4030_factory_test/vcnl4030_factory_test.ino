/*!
 * @file vcnl4030_factory_test.ino
 *
 * Factory test for the Adafruit VCNL4030 proximity and ambient light sensor
 * breakout. Uses Adafruit_TestBed for pass/fail indication.
 *
 * Test jig wiring:
 *   3.3V output from breakout connected to A0 (voltage check)
 *   INT pin from breakout connected to D2
 *   I2C: SDA=A4, SCL=A5 (standard Arduino)
 *
 * Written by Limor 'ladyada' Fried with assistance from Claude Code
 * for Adafruit Industries. MIT license.
 */

#include <Adafruit_TestBed.h>
#include <Adafruit_VCNL4030.h>

extern Adafruit_TestBed TB;
Adafruit_VCNL4030 vcnl;

#define VCNL4030_ADDR 0x60
#define INT_PIN 2
#define POWER_PIN A0

bool failed = false;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println(F("VCNL4030 Factory Test"));
  Serial.println(F("====================="));

  TB.ledPin = 7;
  TB.piezoPin = 12;
  TB.begin();

  // AVR ADC reference is 5V
  TB.analogRef = 5.0;

  // A0 reads the 3.3V output from the breakout
  pinMode(POWER_PIN, INPUT);
}

void loop() {
  failed = false;

  Serial.println(F("\n--- Testing... ---"));

  // 1. 3.3V output check
  Serial.print(F("3.3V output... "));
  if (!TB.testAnalogVoltage(POWER_PIN, "3V3", 1.0, 3.3, 10)) {
    fail(F("3.3V out of range"));
    return;
  }

  // 2. I2C pullup check
  Serial.print(F("I2C pullups... "));
  TB.disableI2C();
  if (!TB.testPullup(A4) || !TB.testPullup(A5)) {
    fail(F("I2C pullups not detected"));
    return;
  }
  Serial.println(F("OK"));

  // 3. I2C scan
  Serial.print(F("I2C scan 0x60... "));
  if (!TB.scanI2CBus(VCNL4030_ADDR)) {
    fail(F("VCNL4030 not found at 0x60"));
    return;
  }
  Serial.println(F("OK"));

  // 4. Sensor init (checks ID register = 0x80)
  Serial.print(F("Sensor init... "));
  if (!vcnl.begin(VCNL4030_ADDR)) {
    fail(F("VCNL4030 init failed"));
    return;
  }
  Serial.println(F("OK"));

  // 5. ALS reading
  Serial.print(F("ALS reading... "));
  vcnl.enableALS(true);
  delay(150); // Wait for first reading with default 100ms IT
  uint16_t als = vcnl.readALS();
  Serial.print(als);
  Serial.print(F(" "));
  if (als == 0) {
    fail(F("ALS is zero"));
    return;
  }
  Serial.println(F("OK"));

  // 6. White channel reading
  Serial.print(F("White channel... "));
  vcnl.enableWhiteChannel(true);
  delay(150);
  uint16_t white = vcnl.readWhite();
  Serial.print(white);
  Serial.print(F(" "));
  if (white == 0) {
    fail(F("White is zero"));
    return;
  }
  Serial.println(F("OK"));

  // 7. Lux reading
  Serial.print(F("Lux reading... "));
  float lux = vcnl.readLux();
  Serial.print(lux, 2);
  Serial.print(F(" "));
  if (lux <= 0.0) {
    fail(F("Lux is zero or negative"));
    return;
  }
  Serial.println(F("OK"));

  // 8. Proximity reading
  Serial.print(F("Proximity... "));
  vcnl.enableProx(true);
  vcnl.setProxResolution16Bit(true);
  delay(50);
  uint16_t prox = vcnl.readProximity();
  Serial.print(prox);
  Serial.print(F(" "));
  if (prox == 0) {
    fail(F("Prox is zero"));
    return;
  }
  Serial.println(F("OK"));

  // 9. INT pin test
  Serial.print(F("INT pin... "));
  pinMode(INT_PIN, INPUT_PULLUP);

  // First verify INT is HIGH (idle, no interrupt pending)
  // Clear any stale flags
  vcnl.readInterruptFlags();
  vcnl.setProxInterruptMode(VCNL4030_PROX_INT_DISABLE);
  delay(10);

  if (digitalRead(INT_PIN) != HIGH) {
    fail(F("INT not HIGH at idle"));
    return;
  }

  // Set proximity interrupt with threshold of 1 (any reading triggers)
  vcnl.setProxHighThreshold(1);
  vcnl.setProxPersistence(VCNL4030_PROX_PERS_1);
  vcnl.setProxInterruptMode(VCNL4030_PROX_INT_CLOSE);

  // Wait for sensor to complete a conversion and fire interrupt
  // Prox measurement period depends on duty/IT, give it plenty of time
  unsigned long start = millis();
  bool intFired = false;
  while (millis() - start < 500) {
    if (digitalRead(INT_PIN) == LOW) {
      intFired = true;
      break;
    }
    delay(5);
  }

  if (!intFired) {
    fail(F("INT did not go LOW"));
    return;
  }

  // Clear interrupt by reading flags
  vcnl.readInterruptFlags();
  delay(10);

  // Disable interrupt so it doesn't re-fire
  vcnl.setProxInterruptMode(VCNL4030_PROX_INT_DISABLE);
  delay(10);

  // INT should return HIGH after clearing + disabling
  if (digitalRead(INT_PIN) != HIGH) {
    fail(F("INT did not go HIGH after clear"));
    return;
  }
  Serial.println(F("OK"));


  // ALL PASS
  Serial.println(F("\n*** ALL TESTS PASSED ***"));
  TB.beepNblink();
  delay(1000);
}

void fail(const __FlashStringHelper *msg) {
  Serial.print(F("FAIL: "));
  Serial.println(msg);
  failed = true;
}
