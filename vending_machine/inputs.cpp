#include <Arduino.h>
#include <DHT.h>
#include "config.h"
#include "inputs.h"

// últimos valores leídos de los sensores
static float distance = NO_ECHO_DISTANCE;

static DHT dht(DHT_PIN, DHT11);
static float temperature = NAN; // NAN significa que todavía no hay lectura
static float humidity = NAN;

// estado del joystick
static JoyEvent lastDirection = JOY_NONE;
static int lastSwState = HIGH;
static unsigned long lastSwChange = 0;

// estado del botón externo
static volatile bool buttonChanged = false; // volatile porque lo cambia la interrupción

static int lastLevel = HIGH;
static unsigned long lastChange = 0;
static int buttonState = HIGH;
static unsigned long pressStart = 0;
static bool adminDone = false;

// interrupción del botón, solo avisa de que ha cambiado, para que sea muy corta
static void onButtonChange() {
    buttonChanged = true;
}

void inputsSetup() {
    pinMode(TRIG, OUTPUT);
    pinMode(ECHO, INPUT);
    digitalWrite(TRIG, LOW);

    dht.begin();

    // INPUT_PULLUP, sin pulsar se lee HIGH y pulsado se lee LOW
    pinMode(JOY_SW, INPUT_PULLUP);

    pinMode(BUTTON, INPUT_PULLUP);
    // la interrupción salta al pulsar y al soltar (CHANGE)
    attachInterrupt(digitalPinToInterrupt(BUTTON), onButtonChange, CHANGE);
}

// SENSOR DE ULTRASONIDOS
void updateDistance() {
    // pulso de 10 microsegundos para que el sensor mande el sonido
    digitalWrite(TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG, LOW);

    // mido cuánto tarda en volver el eco, esperando como mucho 30 ms
    long duration = pulseIn(ECHO, HIGH, ECHO_TIMEOUT);

    if (duration == 0) {
        distance = NO_ECHO_DISTANCE; // no ha vuelto el eco, no hay nadie cerca
    } else {
        distance = duration * CM_PER_US; // paso el tiempo a centímetros
    }
}

float getDistance() {
    return distance;
}

// SENSOR DHT11
void updateEnvironment() {
    float t = dht.readTemperature();
    float h = dht.readHumidity();

    // solo guardo la lectura si es buena
    if (!isnan(t) && !isnan(h)) {
        temperature = t;
        humidity = h;
    }
}

float getTemperature() {
    return temperature;
}

float getHumidity() {
    return humidity;
}

// JOYSTICK
JoyEvent readJoystick() {
    // botón del joystick con antirrebote
    int sw = digitalRead(JOY_SW);
    if (sw != lastSwState && millis() - lastSwChange >= JOY_DEBOUNCE) {
        lastSwChange = millis();
        lastSwState = sw;
        if (sw == LOW) {
            return JOY_PRESS;
        }
    }

    // los ejes dan valores de 0 a 1023, con el centro en unos 512
    int x = analogRead(JOY_X);
    int y = analogRead(JOY_Y);

    JoyEvent direction = JOY_NONE;
    if (y < JOY_LOW) {
        direction = JOY_UP;
    } else if (y > JOY_HIGH) {
        direction = JOY_DOWN;
    } else if (x < JOY_LOW) {
        direction = JOY_LEFT;
    } else if (x > JOY_HIGH) {
        direction = JOY_RIGHT;
    }

    // solo aviso del movimiento cuando empieza, no mientras se mantiene
    if (direction != lastDirection) {
        lastDirection = direction;
        return direction;
    }

    return JOY_NONE;
}

// BOTÓN EXTERNO
ButtonEvent readButton() {
    // copio el aviso de la interrupción con las interrupciones paradas un instante
    noInterrupts();
    bool changed = buttonChanged;
    buttonChanged = false;
    interrupts();

    // si el botón está suelto y no ha habido aviso, no hay nada que hacer
    if (!changed && buttonState == HIGH && lastLevel == HIGH) {
        return BTN_NONE;
    }

    int level = digitalRead(BUTTON);

    // apunto cuándo ha cambiado el nivel del pin
    if (level != lastLevel) {
        lastLevel = level;
        lastChange = millis();
    }

    // antirrebote, el cambio solo cuenta si el pin lleva 50 ms igual
    if (millis() - lastChange >= BUTTON_DEBOUNCE && level != buttonState) {
        buttonState = level;

        if (level == LOW) {
            // se ha pulsado, empieza a contar el tiempo
            pressStart = lastChange;
            adminDone = false;
        } else {
            // se ha soltado, si ha durado entre 2 y 3 segundos, es cancelar
            unsigned long d = lastChange - pressStart;
            if (!adminDone && d >= CANCEL_MIN && d <= CANCEL_MAX) {
                return BTN_CANCEL;
            }
        }
    }

    // si sigue pulsado y llega a 5 segundos, es el modo admin (sin tener que soltar)
    if (buttonState == LOW && lastLevel == LOW && !adminDone && millis() - pressStart >= ADMIN_TIME) {
        adminDone = true;
        return BTN_ADMIN;
    }

    return BTN_NONE;
}
