#include "elevator.h"

Elevator::Elevator()
    : requests(0), currentFloorIdx(0), doorOpen(false),
      direction(DIR_IDLE), currentState(IDLE),
      previousMillis(0), waitingForTravel(false), waitingForService(false) {
}

void Elevator::requestFloor(uint8_t floor) {
    requests |= (1 << floor);
}

bool Elevator::reqAbove() {
    uint8_t maskAbove = 0b1111111 << (currentFloorIdx + 1);
    return (requests & maskAbove) > 0;
}

bool Elevator::reqBelow() {
    uint8_t maskBelow = (1 << currentFloorIdx) - 1;
    return (requests & maskBelow) > 0;
}

void Elevator::idle() {
    if (currentFloorIdx == 0) {  
        direction = DIR_IDLE;
        currentState = IDLE;
    } else {  
        requests |= (1 << 0);  
        currentState = IDLE;
    }
}

void Elevator::stop() {
    currentState = STOPPING;
    doorOpen = true;
}


void Elevator::service() {
    currentState = SERVICING;
    doorOpen = true;
    requests &= ~(1 << currentFloorIdx);  
    doorOpen = false;

    if (direction == DIR_UP) {
        if (reqAbove()) {
            currentState = MOVING;
        } else if (reqBelow()) {
            direction = DIR_DOWN;
            currentState = MOVING;
        } else {
            currentState = IDLE;
            idle();
        }
    } else if (direction == DIR_DOWN) {
        if (reqBelow()) {
            currentState = MOVING;
        } else if (reqAbove()) {
            direction = DIR_UP;
            currentState = MOVING;
        } else {
            currentState = IDLE;
            idle();
        }
    } else { 
        if (reqAbove()) {
            direction = DIR_UP;
            currentState = MOVING;
        } else if (reqBelow()) {
            direction = DIR_DOWN;
            currentState = MOVING;
        } else {
            currentState = IDLE;
            idle();
        }
    }
}


void Elevator::elevatorLook() {
    switch(currentState) {
        case IDLE:
            if (requests != 0) {
                if (requests & (1 << currentFloorIdx)) {
                    currentState = STOPPING;
                } else if (reqAbove()) {
                    Serial.println("Going up");
                    direction = DIR_UP;
                    currentState = MOVING;
                } else if (reqBelow()) {
                    Serial.println("Going down");
                    direction = DIR_DOWN;
                    currentState = MOVING;
                }
            } else {
                idle();
            }
            break;
        case MOVING:
            if (requests & (1 << currentFloorIdx)) {
                currentState = STOPPING;
                waitingForTravel = false;
            } else if (!waitingForTravel) {
                previousMillis = millis();
                waitingForTravel = true;
            } else if (millis() - previousMillis >= TRAVEL_TIME) {
                currentFloorIdx += direction;
                displayFloor();
                waitingForTravel = false;
            }
            break;
        case STOPPING:
            Serial.println("Stopping");
            stop();
            currentState = SERVICING;
            break;
        case SERVICING:
            if (!waitingForService) {
                previousMillis = millis();
                waitingForService = true;
                Serial.println("Door open, servicing floor...");
            } else if (millis() - previousMillis >= SERVICE_TIME) {
                service();
                waitingForService = false;
            }
            break;
    }
}


void Elevator::displayFloor() {
    digitalWrite(LATCH_PIN, LOW);
    shiftOut(DATA_PIN, CLOCK_PIN, LSBFIRST, SEVEN_SEG_DIGITS[currentFloorIdx + 1]);
    digitalWrite(LATCH_PIN, HIGH);
}