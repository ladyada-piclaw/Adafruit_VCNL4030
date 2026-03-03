/*!
 * @file hw_test_helpers.h
 * @brief Shared helpers for VCNL4030 hardware tests
 */

#ifndef HW_TEST_HELPERS_H
#define HW_TEST_HELPERS_H

#include <Adafruit_VCNL4030.h>

/** Reading type for medianRead() */
enum read_type_t {
  READ_PROX,
  READ_ALS,
  READ_WHITE,
};

/*!
 * @brief Take N readings and return the median value
 * @param vcnl Reference to the sensor object
 * @param type Which reading to take (READ_PROX, READ_ALS, READ_WHITE)
 * @param n Number of samples (must be odd, max 9)
 * @param delayMs Delay between samples in ms
 * @return Median value
 */
uint16_t medianRead(Adafruit_VCNL4030& vcnl, read_type_t type, uint8_t n = 3,
                    uint16_t delayMs = 50) {
  uint16_t readings[9];
  if (n > 9)
    n = 9;
  if (n < 1)
    n = 1;

  for (uint8_t i = 0; i < n; i++) {
    switch (type) {
      case READ_PROX:
        readings[i] = vcnl.readProximity();
        break;
      case READ_ALS:
        readings[i] = vcnl.readALS();
        break;
      case READ_WHITE:
        readings[i] = vcnl.readWhite();
        break;
    }
    if (i < n - 1)
      delay(delayMs);
  }

  // Simple insertion sort
  for (uint8_t i = 1; i < n; i++) {
    uint16_t key = readings[i];
    int8_t j = i - 1;
    while (j >= 0 && readings[j] > key) {
      readings[j + 1] = readings[j];
      j--;
    }
    readings[j + 1] = key;
  }

  return readings[n / 2];
}

#endif // HW_TEST_HELPERS_H
