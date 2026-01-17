#ifndef ELEVATOR_CONSTANTS_H
#define ELEVATOR_CONSTANTS_H

#include <Arduino.h>

enum State { IDLE, MOVING, STOPPING, SERVICING };

const int DIR_IDLE = 0;
const int DIR_UP = 1;
const int DIR_DOWN = -1;

const unsigned long TRAVEL_TIME = 1000; 
const unsigned long SERVICE_TIME = 2000;

const int LATCH_PIN = 3; 
const int CLOCK_PIN = 4;
const int DATA_PIN = 2;  

const byte SEVEN_SEG_DIGITS[8] = {
    B00000000,  // = 0
    B01100000,  // = 1
    B11011010,  // = 2
    B11110010,  // = 3
    B01100110,  // = 4
    B10110110,  // = 5
    B10111110,  // = 6
    B11100000   // = 7
};

#endif