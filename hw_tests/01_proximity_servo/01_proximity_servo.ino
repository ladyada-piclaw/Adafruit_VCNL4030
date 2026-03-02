/*!
 * @file 01_proximity_servo.ino
 * @brief HW test: Verify proximity reads with servo-positioned reflector
 *
 * Hardware setup:
 * - VCNL4030 sensor connected via I2C
 * - Servo on pin D4 with a reflective surface attached
 * - Servo parks at a fixed position to place target near sensor
 */

#include <Adafruit_VCNL4030.h>
#include <Servo.h>
#include <Wire.h>

#define SERVO_PIN 4
#define TARGET_POS 130

Adafruit_VCNL4030 vcnl;
Servo servo;

uint16_t medianProximity() {
  uint16_t readings[3];
  for (uint8_t i = 0; i < 3; i++) {
    readings[i] = vcnl.readProximity();
    delay(50);
  }
  if (readings[0] > readings[1]) {
    uint16_t t = readings[0];
    readings[0] = readings[1];
    readings[1] = t;
  }
  if (readings[1] > readings[2]) {
    uint16_t t = readings[1];
    readings[1] = readings[2];
    readings[2] = t;
  }
  if (readings[0] > readings[1]) {
    uint16_t t = readings[0];
    readings[0] = readings[1];
    readings[1] = t;
  }
  return readings[1];
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println(F("=== 01_proximity_servo ==="));
  Serial.println(F("Verify proximity with servo-parked reflector"));
  Serial.println();

  // Park servo and let it settle
  servo.attach(SERVO_PIN);
  servo.write(TARGET_POS);
  delay(1500);

  if (!vcnl.begin()) {
    Serial.println(F("ERROR: VCNL4030 not found!"));
    while (1)
      delay(10);
  }
  Serial.println(F("VCNL4030 initialized"));

  vcnl.enablePS(true);
  vcnl.setLEDCurrent(VCNL4030_LED_I_100MA);
  delay(200);

  // Take 5 readings to show stability
  Serial.println(F("--- Proximity readings (5x) ---"));
  uint16_t total = 0;
  for (uint8_t i = 0; i < 5; i++) {
    uint16_t ps = medianProximity();
    Serial.print(F("  Reading "));
    Serial.print(i + 1);
    Serial.print(F(": "));
    Serial.println(ps);
    total += ps;
    delay(100);
  }
  uint16_t avg = total / 5;
  Serial.print(F("  Average: "));
  Serial.println(avg);

  // Detach servo to stop jitter
  servo.detach();

  // Take 5 more readings with servo detached
  Serial.println(F("--- After servo detach (5x) ---"));
  delay(500);
  total = 0;
  for (uint8_t i = 0; i < 5; i++) {
    uint16_t ps = medianProximity();
    Serial.print(F("  Reading "));
    Serial.print(i + 1);
    Serial.print(F(": "));
    Serial.println(ps);
    total += ps;
    delay(100);
  }
  uint16_t avgDetached = total / 5;
  Serial.print(F("  Average: "));
  Serial.println(avgDetached);

  Serial.println();
  Serial.println(F("========================="));
  if (avgDetached > 30) {
    Serial.println(F("PASS: Proximity detecting target"));
  } else {
    Serial.println(F("FAIL: Proximity too low, target not detected"));
  }
}

void loop() {}
