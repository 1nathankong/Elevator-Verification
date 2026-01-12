### initalizations ###

f1 = 1 << 0 # floor 1: 0000001
f2 = 1 << 1 # floor 2: 0000010
f3 = 1 << 2 # floor 3: 0000100
f4 = 1 << 3 # floor 4: 0001000
f5 = 1 << 4 # floor 5: 0010000
f6 = 1 << 5 # floor 6: 0100000
f7 = 1 << 6 # floor 7: 1000000

requests = 0b0000000

# Physical State Variables
current_floor_idx = 0 #(0 = F1, 6 = F7)
door_open = False ### door is closed ###

# Direction Variables
direction_IDLE, direction_UP, direction_DOWN = 0, 1, -1
direction = direction_IDLE ### could be "UP", "DOWN", or "IDLE"

# FSM implementation variables 

state_IDLE, state_MOVING, state_STOPPING, state_SERVICING = "IDLE", "MOVING", "STOPPING", "SERVICING"
current_state = state_IDLE

### requests: UP or DOWN ###

def req_above(current_floor_idx, requests):
    mask_above = 0b1111111 << (current_floor_idx + 1)
    return (requests & mask_above) > 0

def req_below(current_floor_idx, requests):
    mask_below = (1 << current_floor_idx) - 1
    return(requests & mask_below) > 0


### case when elevator stops moving ### 

def stop():
    global door_open, current_state
    current_state = state_STOPPING
    door_open = True

### case when elevator needs to be serviced
def service(floor_idx):
    global requests, door_open, current_state, direction
    current_state = state_SERVICING
    door_open = True
    requests &= ~(1 << floor_idx)
    door_open = False

    if direction == direction_UP:

        if req_above(floor_idx, requests):
            current_state = state_MOVING

        elif req_below(floor_idx, requests):
            direction = direction_DOWN
            current_state = state_MOVING

        else:
            current_state = state_IDLE
            idle(floor_idx)
    elif direction == direction_DOWN:

        if req_below(floor_idx, requests):
            current_state = state_MOVING

        elif req_above(floor_idx, requests):
            direction = direction_UP
            current_state = state_MOVING

        else:
            current_state = state_IDLE
            idle(floor_idx)
    # when direction is IDLE
    else: 

        if req_above(floor_idx, requests):
            direction = direction_UP
            current_state = state_MOVING

        elif req_below(floor_idx, requests):
            direction = direction_DOWN
            current_state = state_MOVING

        else:
            current_state = state_IDLE
            idle(floor_idx)

### case when elevator is IDLE
def idle(current_floor_idx):
    global requests, direction, current_state

    if current_floor_idx == 0: ## implies we are on Floor 1
        direction = direction_IDLE
        current_state = state_IDLE
        #print("all requests cleared")
    
    else:
        #print("no more requests")
        requests |= (1 << 0)
        current_state = state_IDLE
    

def elevator_look():
    global requests, current_floor_idx, direction, current_state, door_open
    
    # print statement used to debug when needed and display LOOK traversal
    #print(f"State: {current_state:<9} | Floor: {current_floor_idx + 1} | Dir: {direction:>2} | Reg: {bin(requests):>9}") 

    if current_state == state_IDLE:

        if requests != 0:

            if requests & (1 << current_floor_idx):
                current_state = state_STOPPING

            # check if the elevator is going up based off requests
            elif req_above(current_floor_idx, requests):
                direction = direction_UP
                current_state = state_MOVING

            # check if elevator is going down based off requests
            elif req_below(current_floor_idx, requests):
                direction = direction_DOWN
                current_state = state_MOVING

        # No requests handle return to F1 if needed
        else:
            idle(current_floor_idx)

    
    elif current_state == state_MOVING:
        # get to requested floor: 
        if requests & (1 << current_floor_idx):
            current_state = state_STOPPING
        
        #still looking for floor 
        else:
            current_floor_idx += direction
    
    elif current_state == state_STOPPING:
        stop()
        current_state = state_SERVICING

    elif current_state == state_SERVICING:
        service(current_floor_idx)
