#ifndef FINGER_CHECK_H
#define FINGER_CHECK_H

#include <Adafruit_Fingerprint.h>

extern Adafruit_Fingerprint finger;

uint8_t getFingerprintID();
void getIpOfEsp1();

extern int ItemsScanned;

#endif 
