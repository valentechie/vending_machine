#ifndef INPUTS_H
#define INPUTS_H

// lo que puede hacer el joystick
enum JoyEvent {
    JOY_NONE,
    JOY_UP,
    JOY_DOWN,
    JOY_LEFT,
    JOY_RIGHT,
    JOY_PRESS
};

// lo que puede hacer el botón externo
enum ButtonEvent {
    BTN_NONE,
    BTN_CANCEL,
    BTN_ADMIN
};

// prepara los pines de todas las entradas
void inputsSetup();

// sensor de ultrasonidos
void updateDistance();
float getDistance();

// sensor DHT11
void updateEnvironment();
float getTemperature();
float getHumidity();

// joystick
JoyEvent readJoystick();

// botón externo
ButtonEvent readButton();

#endif
