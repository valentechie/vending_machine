#ifndef JOYSTICK_H
#define JOYSTICK_H

enum JoyEvent {
    JOY_NONE,
    JOY_UP,
    JOY_DOWN,
    JOY_LEFT,
    JOY_RIGHT,
    JOY_PRESS
};

void joystickSetup();
JoyEvent readJoystick();

#endif
