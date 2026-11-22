#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

const int LED1 = 11;
const int LED2 = 10;

const int LCD_RS = 5;
const int LCD_E = 3;
const int LCD_D4 = A2;
const int LCD_D5 = A3;
const int LCD_D6 = A4;
const int LCD_D7 = A5;

const int BUTTON = 2;

const int JOY_SW = 4;
const int JOY_X = A1;
const int JOY_Y = A0;

const int DHT_PIN = 6;
const int TRIG = 8;
const int ECHO = 7;

const unsigned long LED1_INTERVAL = 1000;
const int STARTUP_TOGGLES = 6;

const float CM_PER_US = 0.01715f;
const unsigned long MEASURE_INTERVAL = 200;
const unsigned long ECHO_TIMEOUT = 30000;
const float NO_ECHO_DISTANCE = 999.0f;
const float PRESENCE_DISTANCE = 100.0f;

const unsigned long DHT_INTERVAL = 2000;

const int JOY_LOW = 300;
const int JOY_HIGH = 700;
const unsigned long JOY_DEBOUNCE = 50;

const unsigned long BUTTON_DEBOUNCE = 50;
const unsigned long CANCEL_MIN = 2000;
const unsigned long CANCEL_MAX = 3000;
const unsigned long ADMIN_TIME = 5000;

const unsigned long ENV_DISPLAY_TIME = 5000;
const unsigned long PREP_MIN = 4000;
const unsigned long PREP_MAX = 8000;
const unsigned long RETIRE_TIME = 3000;

const int NUM_PRODUCTS = 5;
const int PRICE_STEP = 5;
const int MIN_PRICE = 5;
const int MAX_PRICE = 995;

const int NUM_ADMIN_OPTIONS = 4;
const unsigned long ADMIN_REFRESH = 500;

#endif
