const int button = 2;

volatile unsigned long pressStart = 0;
volatile unsigned long pressDuration = 0;

void setup() {
  // start serial connection
  Serial.begin(9600);

  // configure pin 2 as an input and enable the internal pull-up resistor
  pinMode(button, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(button), onButtonChange, CHANGE);
}

void loop() {
  // read the pushbutton value into a variable
  // int sensorVal = digitalRead(2);

  // print out the value of the pushbutton
  // Serial.println(sensorVal);

  // if the ISR saved a new press duration, print it once and reset it
  if (pressDuration != 0) {
    Serial.println(pressDuration);
    pressDuration = 0;
  }
}

// ISR, runs every time the interrupt is triggered
void onButtonChange() {
  if (digitalRead(button) == LOW) {
    // pressed: save the start time
    pressStart = millis();
  } else {
    // released: end time - start time
    // debounce filter
    unsigned long d = millis() - pressStart;
    if (d > 50) {
      pressDuration = d;
    }
  }
}