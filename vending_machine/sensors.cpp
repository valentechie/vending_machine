#include <Arduino.h>
#include <DHT.h>
#include "config.h"
#include "sensors.h"

static float distance = NO_ECHO_DISTANCE;

static DHT dht(DHT_PIN, DHT11);
static float temperature = NAN;
static float humidity = NAN;

void sensorsSetup() {
    pinMode(TRIG, OUTPUT);
    pinMode(ECHO, INPUT);
    digitalWrite(TRIG, LOW);

    dht.begin();
}

void updateDistance() {
    digitalWrite(TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG, LOW);

    long duration = pulseIn(ECHO, HIGH, ECHO_TIMEOUT);

    if (duration == 0) {
        distance = NO_ECHO_DISTANCE;
    } else {
        distance = duration * CM_PER_US;
    }
}

float getDistance() {
    return distance;
}

void updateEnvironment() {
    float t = dht.readTemperature();
    float h = dht.readHumidity();

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
