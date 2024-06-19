#ifndef ENROLL_H
#define ENROLL_H

#include <Adafruit_Fingerprint.h>

extern Adafruit_Fingerprint finger;

uint8_t getFingerprintEnroll(uint8_t fingerId);

#endif // FINGERPRINT_H
