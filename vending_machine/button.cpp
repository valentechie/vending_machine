#include <Arduino.h>
#include "config.h"
#include "button.h"

static volatile unsigned long pressStart = 0;
static volatile unsigned long pressDuration = 0;

static void onButtonChange() {
    if (digitalRead(BUTTON) == LOW) {
        pressStart = millis();
    } else {
        unsigned long d = millis() - pressStart;
        if (d > BUTTON_DEBOUNCE) {
            pressDuration = d;
        }
    }
}

void buttonSetup() {
    pinMode(BUTTON, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BUTTON), onButtonChange, CHANGE);
}

unsigned long getPressDuration() {
    noInterrupts();
    unsigned long d = pressDuration;
    pressDuration = 0;
    interrupts();

    return d;
}
