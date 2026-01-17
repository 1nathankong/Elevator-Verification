#include "elevator.h"
#include "IRremote.h"

int receiver = 13; 
IRrecv irrecv(receiver);
decode_results results; 

Elevator elevator;


void translateIR()
{
  switch (results.value)
  {
    case 0xFF30CF: Serial.println("Floor 1 Requested"); elevator.requestFloor(0); break;
    case 0xFF18E7: Serial.println("Floor 2 Requested"); elevator.requestFloor(1); break;
    case 0xFF7A85: Serial.println("Floor 3 Requested"); elevator.requestFloor(2); break;
    case 0xFF10EF: Serial.println("Floor 4 Requested"); elevator.requestFloor(3); break;
    case 0xFF38C7: Serial.println("Floor 5 Requested"); elevator.requestFloor(4); break;
    case 0xFF5AA5: Serial.println("Floor 6 Requested"); elevator.requestFloor(5); break;
    case 0xFF42BD: Serial.println("Floor 7 Requested"); elevator.requestFloor(6); break;

    case 0xFFFFFF: break;
    break;
  }
}

void setup()
{
  Serial.begin(9600);
  irrecv.enableIRIn();
  Serial.println("Elevator Ready. Please select a floor (1-7):");

  pinMode(LATCH_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);
}

void loop()
{
  if (irrecv.decode(&results))
  {
    translateIR();
    irrecv.resume();
  }
  elevator.elevatorLook();  
}
