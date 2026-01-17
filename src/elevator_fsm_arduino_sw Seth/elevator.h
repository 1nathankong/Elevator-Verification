#ifndef ELEVATOR_H
#define ELEVATOR_H

#include "elevator_constants.h"
#include <Arduino.h>

class Elevator {
public:
    Elevator();
    void elevatorLook();         
    void requestFloor(uint8_t floor);  
    void displayFloor();    

private:
    // State variables
    uint8_t requests;
    int currentFloorIdx;
    bool doorOpen;
    int direction;
    State currentState;

    // Timing variables
    unsigned long previousMillis;
    bool waitingForTravel;
    bool waitingForService;

    // Private helper methods
    void idle();
    void stop();
    void service();
    bool reqAbove();
    bool reqBelow();
};

#endif