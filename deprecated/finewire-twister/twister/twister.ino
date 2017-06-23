#include <AccelStepper.h>

// Microstepper driver connections
#define M0 5
#define M1 6
#define M2 7
#define STEP 10
#define DIR 11

// Button input
#define TRIG 2

// Hardcoded number of steps in the motor I'm using
#define STEPS_PER_REV 200

AccelStepper stepper(1, STEP, DIR);
long num_turns = 100;
long usteps = 0;
volatile bool moving = false;
volatile bool state_change = false;
const int led = LED_BUILTIN; 

void setup(void) {
  Serial.begin(9600);

  // Microsteps (set to 16)
  pinMode(M0, OUTPUT);
  pinMode(M1, OUTPUT);
  pinMode(M2, OUTPUT);

  // Trigger button
  pinMode(TRIG, INPUT);
  attachInterrupt(0, trigPressed, RISING);

  configureParameters(true);
}

void loop(void) {

  if (state_change && moving) {
    // Stop moton
    stepper.setCurrentPosition(0);
    state_change = false;
    moving = false;
    digitalWrite(LED_BUILTIN, LOW);
    Serial.println("Stopping motion");
  } else if (state_change) {
    Serial.println(getNumSteps());
    stepper.move(getNumSteps());
    state_change = false;
    moving = true;
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("Starting motion");
  }

  if (stepper.distanceToGo() != 0) {
    stepper.run();
  } else {
    moving = false;
  }

  if (!moving && Serial.available()) {
    int temp = Serial.parseInt();
    if (temp != 0) {
      num_turns = temp;
      Serial.print("Target turns set to ");
      Serial.println(num_turns);
    }
  }
}

int configureParameters(bool init) {

  if (init) {

      // Set number of microsteps
      usteps = setUStep(usteps);

      // Set max stepper speed in turns/sec
      double max_speed = 500;
      stepper.setMaxSpeed(max_speed);
      stepper.setAcceleration(max_speed/5);
  }
}

int setUStep(int us) {

  switch (us) {
  case 0: {
    digitalWrite(M0, LOW);
    digitalWrite(M1, LOW);
    digitalWrite(M2, LOW);
    Serial.println("Set to no microsteps");
    return 1;
  }
  case 8: {
    digitalWrite(M0, HIGH);
    digitalWrite(M1, HIGH);
    digitalWrite(M2, LOW);
    Serial.println("Set to 8 microsteps");
    return 8;
  }
  case 16: {
    digitalWrite(M0, LOW);
    digitalWrite(M1, LOW);
    digitalWrite(M2, HIGH);
    Serial.println("Set to 16 microsteps");
    return 32;
  }
  case 32: {
    digitalWrite(M0, HIGH);
    digitalWrite(M1, HIGH);
    digitalWrite(M2, HIGH);
    Serial.println("Set to 32 microsteps");
    return 32;
  }
  default: {
    digitalWrite(M0, LOW);
    digitalWrite(M1, LOW);
    digitalWrite(M2, LOW);
    Serial.println("Invalid selection. Set to no microsteps");
    return 1;
  }
  }
}

void trigPressed() { state_change = true; }
long getNumSteps() { return usteps * STEPS_PER_REV * num_turns; }
