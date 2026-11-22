const int trig = 7;
const int echo = 8;

// speed of sound in air, in cm per microsecond
const float V_SOUND_AIR = 0.0343f;

// time between measurements (ms)
const unsigned long MEASURE_INTERVAL = 200;

unsigned long lastMeasure = 0;
long duration = 0;
float distance = 0.0f;

void setup() {
  Serial.begin(9600);
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);

  // ensure trigger starts LOW
  digitalWrite(trig, LOW);
  Serial.println("Ultrasonic test started");
}

void loop() {
  // measure only every MEASURE_INTERVAL ms, without blocking
  if (millis() - lastMeasure >= MEASURE_INTERVAL) {
    lastMeasure = millis();

    // send a 10 us pulse on the trigger
    digitalWrite(trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig, LOW);

    // time (us) that echo stays HIGH, waiting at most 30 ms
    duration = pulseIn(echo, HIGH, 30000);

    if (duration == 0) {
      // no echo received in time
      Serial.println("Out of range");
    } else {
      // round trip, so divide by 2
      distance = (duration * V_SOUND_AIR) / 2.0f;
      Serial.print(distance);
      Serial.println(" cm");
    }
  }
}
