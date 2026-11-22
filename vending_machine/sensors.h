#ifndef SENSORS_H
#define SENSORS_H

void sensorsSetup();

void updateDistance();
float getDistance();

void updateEnvironment();
float getTemperature();
float getHumidity();

#endif
