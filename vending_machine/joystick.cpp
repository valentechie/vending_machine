#include <Arduino.h>
#include "config.h"
#include "joystick.h"

static JoyEvent lastDirection = JOY_NONE;
static int lastSwState = HIGH;
static unsigned long lastSwChange = 0;

void joystickSetup() {
    pinMode(JOY_SW, INPUT_PULLUP);
}

JoyEvent readJoystick() {
    int sw = digitalRead(JOY_SW);
    if (sw != lastSwState && millis() - lastSwChange >= JOY_DEBOUNCE) {
        lastSwChange = millis();
        lastSwState = sw;
        if (sw == LOW) {
            return JOY_PRESS;
        }
    }

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

    if (direction != lastDirection) {
        lastDirection = direction;
        return direction;
    }

    return JOY_NONE;
}
