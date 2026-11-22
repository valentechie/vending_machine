#include <LiquidCrystal.h>

int rs = 12;
int e = 11;
int d4 = 5;
int d5 = 4;
int d6 = 3;
int d7 = 2;

int xPin = A0;
int xVal;

LiquidCrystal lcd(rs, e, d4, d5, d6, d7);

void setup() {
  // put your setup code here, to run once:
  lcd.begin(16,2);
}

void loop() {
  // put your main code here, to run repeatedly:
  lcd.setCursor(0,0);
  lcd.print("Valen es...");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("1. bonita");
  lcd.setCursor(0,1);
  lcd.print("2. inteligente");
  delay(8000);
  lcd.clear();
}
