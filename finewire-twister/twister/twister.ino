#include <AccelStepper.h>

// Microstepper Settings
#define M0 5
#define M1 6
#define M2 7

#define STEP 10
#define DIR 11

#define TRIG 2
#define STEPS_PER_REV 24

AccelStepper stepper(1, STEP, DIR);
int num_turns = 100;
int usteps = 8;
volatile bool moving = false;
volatile bool state_change = false;

// This example uses the timer interrupt to blink an LED
// and also demonstrates how to share a variable between
// the interrupt and the main program.

const int led = LED_BUILTIN;  // the pin with a LED
// Define a stepper and the pins it will use

void setup(void)
{
  Serial.begin(9600);
  
  // Microsteps (set to 16)
  pinMode(M0, OUTPUT);
  pinMode(M1, OUTPUT);
  pinMode(M2, OUTPUT);
  usteps = setUStep(usteps);

  // Trigger button
  pinMode(TRIG, INPUT);
  digitalWrite(TRIG, LOW); // Pulldown
  attachInterrupt(0, trigPressed, RISING);

  // Stepper setup (Empirical...)
  stepper.setMaxSpeed(10000);
  stepper.setAcceleration(200);
}

// The main program will print the blink count
// to the Arduino Serial Monitor
void loop(void)
{
  if (state_change && moving) {
    // Stop moton
    stepper.setCurrentPosition(0);
    state_change = false;
    moving = false;
    Serial.println("Stopping motion");
  } else if (state_change) {
    stepper.move(getNumSteps());
    state_change = false;
    moving = true;
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

void trigPressed() {
  state_change = true;
}

int setUStep(int us) {

  switch (us) {
    case 8: {
      digitalWrite(M0, HIGH);
      digitalWrite(M1, HIGH);
      digitalWrite(M2, LOW);
      Serial.println("Set to 8 microsteps");
      return 8;
    }
    default : {
      digitalWrite(M0, LOW);
      digitalWrite(M1, LOW);
      digitalWrite(M2, LOW);
      Serial.println("Set to no microsteps");
      return 1;
    }
  }
}

int getNumSteps() {
  return usteps * STEPS_PER_REV * num_turns;
}

