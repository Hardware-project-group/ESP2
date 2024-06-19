#include <LiquidCrystal_I2C.h>
#include "Display.h"



LiquidCrystal_I2C lcd(0x26, 20, 4);

void Displaysetup() {
  lcd.init();
  lcd.backlight();
}

