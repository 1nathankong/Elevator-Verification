#include "IRremote.h"

/* Variables and Bitmasks */

uint8_t requests = 0b0000000;

int currentFloorIdx = 0; // 0 = Floor 1 6 = Floor 7

bool doorOpen = false; // Initialized to false which means door is closed. 

int receiver = 13; // signal pin of IR receiver to Arduino digital pin 13

enum State { IDLE, MOVING, STOPPING, SERVICING }; // state machine for the 4 states. 
State currentState = IDLE; // intialized to idle 

const int DIR_IDLE = 0, DIR_UP = 1, DIR_DOWN = -1; //direction constants

int direction = DIR_IDLE; // intialization

IRrecv irrecv(receiver); //instance of 'irrecv'
decode_results results; //instance of 'decode_results'


byte seven_seg_digits[8] = {
                              B00000000,  // = 0 IGNORE THIS
                              B01100000,  // = 1
                              B11011010,  // = 2
                              B11110010,  // = 3
                              B01100110,  // = 4
                              B10110110,  // = 5
                              B10111110,  // = 6
                              B11100000   // = 7
                             };

// connect to the ST_CP of 74HC595 (pin 3,latch pin)
int latchPin = 3;
// connect to the SH_CP of 74HC595 (pin 4, clock pin)
int clockPin = 4;
// connect to the DS of 74HC595 (pin 2)
int dataPin = 2;

/* Timing and Latency Setup */
unsigned long previousMillis = 0;
const unsigned long TRAVEL_TIME = 1000; // 1 second per floor
const unsigned long SERVICE_TIME = 2000; // 2 seconds per door

/* Flags to determine when to move or service */
bool waitingForTravel = false;
bool waitingForService = false;


/* Helper functions to manage requests going UP or DOWN */

bool reqAbove()
{
  uint8_t maskAbove = 0b1111111 << (currentFloorIdx + 1);
  return (requests & maskAbove) > 0;
}

bool reqBelow()
{
  uint8_t maskBelow = (1 << currentFloorIdx) - 1;
  return (requests & maskBelow) > 0;
}

/* Main functions */

void translateIR()
{
  switch (results.value)
  {
    case 0xFF30CF: Serial.println("Floor 1 Requested"); requests |= (1 << 0); break;
    case 0xFF18E7: Serial.println("Floor 2 Requested"); requests |= (1 << 1); break;
    case 0xFF7A85: Serial.println("Floor 3 Requested"); requests |= (1 << 2); break;
    case 0xFF10EF: Serial.println("Floor 4 Requested"); requests |= (1 << 3); break;
    case 0xFF38C7: Serial.println("Floor 5 Requested"); requests |= (1 << 4); break;
    case 0xFF5AA5: Serial.println("Floor 6 Requested"); requests |= (1 << 5); break;
    case 0xFF42BD: Serial.println("Floor 7 Requested"); requests |= (1 << 6); break;

    case 0xFFFFFF: break;

    //when you click a button that is not #1-#7:
    //default:
      //Serial.println("Button does not have any input, please click #1-#7 for Floor Request");
    break;
  }
}

void setup()
{

  // Setup for IR
  Serial.begin(9600);
  irrecv.enableIRIn(); //Receiver Turned on
  Serial.println("Elevator Ready. Please select a floor (1-7):");

  // Setup for LED digit 
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
}



// display a number on the digital segment display
void sevenSegWrite(byte digit) {
  // set the latchPin to low potential, before sending data
  digitalWrite(latchPin, LOW);
     
  // the original data (bit pattern)
  shiftOut(dataPin, clockPin, LSBFIRST, seven_seg_digits[digit]);  
 
  // set the latchPin to high potential, after sending data
  digitalWrite(latchPin, HIGH);
}

void loop()
{
  if (irrecv.decode(&results))
  {
    translateIR();
    irrecv.resume();
  }
  elevatorLook();  // Run FSM every loop iteration
}

/* Idle function - returns to Floor 1 when no requests */
void idle()
{
  
  if (currentFloorIdx == 0)  // Already at Floor 1
  {
    direction = DIR_IDLE;
    currentState = IDLE;
  }
  
  else  // Not at Floor 1, return to it
  {
    requests |= (1 << 0);  // Request Floor 1
    currentState = IDLE;
  }
}

/* Stop function - sets state to STOPPING and opens door */
void stop()
{
  currentState = STOPPING;
  doorOpen = true;
}

void service()
{
  currentState = SERVICING;
  doorOpen = true;
  requests &= ~(1 << currentFloorIdx);  // Clear current floor request
  doorOpen = false;

  // Maintain direction priority (matches Python logic)
  if (direction == DIR_UP)
  {
    
    if (reqAbove())
    {
      currentState = MOVING;
    }
    
    else if (reqBelow())
    {
      direction = DIR_DOWN;
      currentState = MOVING;
    }
    
    else
    {
      currentState = IDLE;
      idle();
    }
  }
  
  else if (direction == DIR_DOWN)
  {
    
    if (reqBelow())
    {
      currentState = MOVING;
    }
    
    else if (reqAbove())
    {
      direction = DIR_UP;
      currentState = MOVING;
    }
    
    else
    {
      currentState = IDLE;
      idle();
    }
  }
  
  else  // direction == DIR_IDLE
  {
    
    if (reqAbove())
    {
      direction = DIR_UP;
      currentState = MOVING;
    }
    
    else if (reqBelow())
    {
      direction = DIR_DOWN;
      currentState = MOVING;
    }
    
    else
    {
      currentState = IDLE;
      idle();
    }
  }
}

void elevatorLook()
{
  switch(currentState)
  {
    case IDLE:
      
      if (requests != 0)
      {
        
        if (requests & (1 << currentFloorIdx))
        {
          currentState = STOPPING;
        }
        
        else if (reqAbove())
        {
          Serial.println("Going up");
          direction = DIR_UP;
          currentState = MOVING;
        }
        
        else if (reqBelow())
        {
          Serial.println("Going down");
          direction = DIR_DOWN;
          currentState = MOVING;
        }
      }
      
      else  // No requests, handle return to F1 if needed
      {
        idle();
      }
      break;

    case MOVING:
      
      // Check if we reached a requested floor
      if (requests & (1 << currentFloorIdx))
      {
        currentState = STOPPING;
        waitingForTravel = false;
      }
      
      else if(!waitingForTravel)
      {
        previousMillis = millis();
        waitingForTravel = true;
      }
      
      else if(millis() - previousMillis >= TRAVEL_TIME)  // Still moving
      {
        currentFloorIdx += direction;
        sevenSegWrite(currentFloorIdx + 1);
        waitingForTravel = false;
      }
      break;

    case STOPPING:
      Serial.println("Stopping");
      stop();
      currentState = SERVICING;
      break;

    case SERVICING:
      
      if(!waitingForService)
      {
        previousMillis = millis();
        waitingForService = true;
        Serial.println("Door open, servicing floor...");
      }

      else if(millis() - previousMillis >= SERVICE_TIME)
      {
        service();
        waitingForService = false;
      }
      break;
  }
}