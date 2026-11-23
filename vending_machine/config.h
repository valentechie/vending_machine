#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// PINES
// LEDs (el LED2 va en un pin con PWM para poder cambiar su brillo)
const int LED1 = 11;
const int LED2 = 6;

// pantalla LCD
const int LCD_RS = 5;
const int LCD_E = 3;
const int LCD_D4 = A2;
const int LCD_D5 = A3;
const int LCD_D6 = A4;
const int LCD_D7 = A5;

// botón externo (el pin 2 admite interrupciones)
const int BUTTON = 2;

// joystick (en mi montaje está girado, por eso X e Y están cambiados)
const int JOY_SW = 4;
const int JOY_X = A0;
const int JOY_Y = A1;

// sensores
const int DHT_PIN = 12;
const int TRIG = 8;
const int ECHO = 7;

// MODO ARRANQUE
const unsigned long LED1_INTERVAL = 1000; // el LED1 cambia cada segundo
const int STARTUP_TOGGLES = 6; // 3 parpadeos son 6 cambios (encender y apagar)

// SENSORES
const float CM_PER_US = 0.01715f; // velocidad del sonido dividida entre 2 (ida y vuelta)
const unsigned long MEASURE_INTERVAL = 200; // mido la distancia cada 200 ms
const unsigned long ECHO_TIMEOUT = 30000; // espero el eco como mucho 30 ms
const float NO_ECHO_DISTANCE = 999.0f; // valor que uso cuando no vuelve el eco
const float PRESENCE_DISTANCE = 100.0f; // a menos de 100 cm hay un cliente

const unsigned long DHT_INTERVAL = 2000; // el DHT11 es lento, lo leo cada 2 segundos

// JOYSTICK Y BOTÓN
const int JOY_LOW = 300; // por debajo de este valor el joystick se ha movido
const int JOY_HIGH = 700; // por encima de este valor también
const unsigned long JOY_DEBOUNCE = 50; // antirrebote del botón del joystick

const unsigned long BUTTON_DEBOUNCE = 50; // antirrebote del botón externo
const unsigned long CANCEL_MIN = 2000; // cancelar: entre 2 y 3 segundos
const unsigned long CANCEL_MAX = 3000;
const unsigned long ADMIN_TIME = 5000; // modo admin: 5 segundos o más

// MODO SERVICIO
const unsigned long ENV_DISPLAY_TIME = 5000; // muestro temperatura y humedad 5 segundos
const unsigned long PREP_MIN = 4000; // la preparación dura entre 4
const unsigned long PREP_MAX = 8000; // y 8 segundos
const unsigned long RETIRE_TIME = 3000; // "RETIRE BEBIDA" se ve 3 segundos

// precios en céntimos para no tener errores con los decimales
const int NUM_PRODUCTS = 5;
const int PRICE_STEP = 5; // los precios cambian de 0.05 en 0.05
const int MIN_PRICE = 5;
const int MAX_PRICE = 995;

// MODO ADMIN
const int NUM_ADMIN_OPTIONS = 4;
const unsigned long ADMIN_REFRESH = 500; // las pantallas de admin se actualizan cada medio segundo

#endif
