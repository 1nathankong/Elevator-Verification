'''

Constants stored here for readability. Will be easier to make changes later.

'''

FLOORS = [1 << i for i in range(7)]
DIRECTION_IDLE, DIRECTION_UP, DIRECTION_DOWN = 0, 1, -1
STATE_IDLE, STATE_MOVING, STATE_STOPPING, STATE_SERVICING = "IDLE", "MOVING", "STOPPING", "SERVICING"