#include <TimerOne.h>

// Defines for traffic light Pins

// Car traffic lights
#define CRPin 3
#define CYPin 5
#define CGPin 6
//Pedestrian traffic lights
#define PRPin 9
#define PGPin 10


// Defines for timings (In ms)
#define DURATION_RESTART 5000
#define DURATION_PREPARE_DRIVE 1000
#define DURATION_DRIVE 50000 // Don't exceed one minute
#define DURATION_PREPARE_HALT 2000
#define DURATION_WALK 15000

#define DURATION_DRIVE_AFTER_REQUEST 10000

//Misc Defines
#define BTNPin 4
#define UseRY true 
// Red and yellow at the same time after red
#define AutomaticPedestrianPhase false 
// Let the pedestrians go after the Duration of DURATION_DRIVE

// Variable definitions:
unsigned int mainState = 0;
unsigned int mainTimer = 0;

bool requested = false;

void setLamps(bool cr, bool cy, bool cg, bool pr, bool pg) {
  // Car traffic lights
  digitalWrite(CRPin, cr);
  digitalWrite(CYPin, cy);
  digitalWrite(CGPin, cg);
  // Pedestrian traffic lights
  digitalWrite(PRPin, pr);
  digitalWrite(PGPin, pg);
}


unsigned int mainStateMachine(unsigned int state) {
    int returnState = state;
    switch (state) {
      case 0: // Restarted state
        setLamps(1, 0, 0, 1, 0); // Both Lights red, nobody should go.
        if(mainTimer >= DURATION_RESTART) {
          mainTimer = 0;
          if(UseRY) {
            returnState = 1; // Let the cars prepare to drive
          }
          else {
            returnState = 2; // Let the cars drive
          }
        }
        break;
      case 1: // Cars preparing to drive
        setLamps(1, 1, 0, 1, 0);
        if(mainTimer >= DURATION_PREPARE_DRIVE) {
          mainTimer = 0;
          returnState = 2; // Let the cars drive
        }
        break;
      case 2: // Cars driving
        setLamps(0, 0, 1, 1, 0);
        if(mainTimer >= DURATION_DRIVE && AutomaticPedestrianPhase) {
          mainTimer = 0;
          returnState = 3; // Let the cars prepare to halt
        }
        else if(requested && mainTimer >= DURATION_DRIVE_AFTER_REQUEST) {
          requested = false;
          mainTimer = 0;
          returnState = 3; // Let the cars prepare to halt
        }
        break;
      case 3: // Cars preparing to halt
        setLamps(0, 1, 0, 1, 0);
        if(mainTimer >= DURATION_PREPARE_HALT) {
          mainTimer = 0;
          returnState = 4; // Let the cars halt
        }
        break;
      case 4: // Cars halting
        setLamps(1, 0, 0, 1, 0);
        if(mainTimer >= DURATION_WALK) {
          mainTimer = 0;
          returnState = 5; // Let the cars pedestrians walk
        }
        break;
      case 5: // Pedestrians walking
        setLamps(0, 1, 0, 1, 0);
        if(mainTimer >= DURATION_WALK) {
          mainTimer = 0;
          returnState = 0; // Reset to let everyone halt
        }
        break;
      default:
        returnState = 0; 
        // Something went wrong, better restart and let nobody go.
  }
  return returnState;
}

void incrementTimers() {
  /*
   * Unsigned ints overflow about every 65 seconds. 
   * Don't set timings larger than a minute.
   * Use multiple states instead
   */
  ++mainTimer;
}

void setup() {
  Timer1.initialize(1000); //Interrupt every millisecond
  Timer1.attachInterrupt(incrementTimers);

  // Set pin modes
  pinMode(CRPin, OUTPUT);
  pinMode(CYPin, OUTPUT);
  pinMode(CGPin, OUTPUT);
  pinMode(PRPin, OUTPUT);
  pinMode(PGPin, OUTPUT);
  
  pinMode(BTNPin, INPUT);
}

void loop() {
  mainState = mainStateMachine(mainState);
  if(digitalRead(BTNPin) && mainState == 2) { 
    //Just allow request while in phase 2
    requested = true;
  }
}
