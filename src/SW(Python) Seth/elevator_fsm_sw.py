from elevator_constants import *

class ElevatorFSM:
    '''
    

    I refactored your elevator FSM script and functions into a object oriented pattern.
    I think it makes more sense to design your testbench source this way since the FSM
    uses states to control the machine, and your class variables will act like states.
    And by having class variables dictate state you do not need to manually pass around
    global variables between functions.

    Let me know what you think! I think any time you need to design a system with constant 
    values and state variables, OOP is a good approach and easier for another developer to 
    understand. 
    '''



    def __init__(self):
        self.requests = 0
        self.current_floor_idx = 0
        self.door_open = False
        self.direction = DIRECTION_IDLE
        self.current_state = STATE_IDLE

        
    def _req_above(self):
        mask_above = 0b1111111 << (self.current_floor_idx + 1)
        return (self.requests & mask_above) > 0

    def _req_below(self):
        mask_below = (1 << self.current_floor_idx) - 1
        return(self.requests & mask_below) > 0


    ### case when elevator stops moving ### 

    def _stop(self):
        self.current_state = STATE_STOPPING
        self.door_open = True


    ### case when elevator needs to be serviced

    def _service(self):
        self.current_state = STATE_SERVICING
        self.door_open = True
        self.requests &= ~(1 << self.current_floor_idx)
        self.door_open = False
        if self.direction == DIRECTION_UP:

            if self._req_above():
                self.current_state = STATE_MOVING

            elif self._req_below():
                self.direction = DIRECTION_DOWN
                self.current_state = STATE_MOVING

            else:
                self.current_state = STATE_IDLE
                self._idle()
        elif self.direction == DIRECTION_DOWN:

            if self._req_below():
                self.current_state = STATE_MOVING

            elif self._req_above():
                self.direction = DIRECTION_UP
                self.current_state = STATE_MOVING
            else:
                self.current_state = STATE_IDLE
                self._idle()

        # when direction is IDLE
        else: 

            if self._req_above():
                self.direction = DIRECTION_UP
                self.current_state = STATE_MOVING

            elif self._req_below():
                self.direction = DIRECTION_DOWN
                self.current_state = STATE_MOVING
            else:
                self.current_state = STATE_IDLE
                self._idle()


    ### case when elevator is IDLE

    def _idle(self):
        if self.current_floor_idx == 0: ## implies we are on Floor 1
            self.direction = DIRECTION_IDLE
            self.current_state = STATE_IDLE
            #print("all requests cleared")
        
        else:
            #print("no more requests")
            self.requests |= (1 << 0)
            self.current_state = STATE_IDLE
        

    def elevator_look(self):
        # print statement used to debug when needed and display LOOK traversal
        #print(f"State: {self.current_state:<9} | Floor: {self.current_floor_idx + 1} | Dir: {self.direction:>2} | Reg: {bin(self.requests):>9}") 

        if self.current_state == STATE_IDLE:
            if self.requests != 0:

                if self.requests & (1 << self.current_floor_idx):
                    self.current_state = STATE_STOPPING

                # check if the elevator is going up based off requests
                elif self._req_above():
                    self.direction = DIRECTION_UP
                    self.current_state = STATE_MOVING

                # check if elevator is going down based off requests
                elif self._req_below():
                    self.direction = DIRECTION_DOWN
                    self.current_state = STATE_MOVING

            # No requests handle return to F1 if needed
            else:
                self._idle()

        
        elif self.current_state == STATE_MOVING:
            # get to requested floor: 
            if self.requests & (1 << self.current_floor_idx):
                self.current_state = STATE_STOPPING
            
            #still looking for floor 
            else:
                self.current_floor_idx += self.direction
        
        elif self.current_state == STATE_STOPPING:
            self._stop()
            self.current_state = STATE_SERVICING

        elif self.current_state == STATE_SERVICING:
            self._service()