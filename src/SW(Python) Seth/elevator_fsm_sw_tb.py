from elevator_fsm_sw import ElevatorFSM
from elevator_constants import *
import sys

### IMPORTANT INFO
### 32K bytes of flash program memory
### 2k bytes of internal SRAM

### EACH variable uses 1 byte in FSM. 
SRAM_LIMIT = 2048 # 2 * 1024 bytes
FLASH_LIMIT = 32768 # 32 * 1024 bytes

def check_memory(variables):
    global SRAM_LIMIT
    total_bytes = 0
    for name, value in variables.items():
        try:
            size = len(value)
        except TypeError:
            size = 1
        print(f"{name}: {size} bytes")
        total_bytes += size

    if total_bytes > SRAM_LIMIT:
        print("Exceeding 2KB")
        return False
    else:
        print("Within 2kb SRAM")
        return True



def test(test_name, initial_requests, start_floor = 0):
    dev = ElevatorFSM()
    dev.requests = initial_requests
    dev.current_floor_idx = start_floor
    dev.current_state = STATE_IDLE
    dev.direction = DIRECTION_IDLE
    print(f"\n[TEST] {test_name} | Initial: {bin(initial_requests)}")

    for tick in range(100):
        dev.elevator_look()

        if dev.current_state == STATE_IDLE and dev.requests == 0 and dev.current_floor_idx == 0:
            print(f"RESULT: PASS ({tick} ticks)")
            return True

    print(f"RESULT: FAIL. Stuck at F{dev.current_floor_idx+1} in state {dev.current_state}")
    return False

if __name__ == "__main__":
    check_memory({'req':1, 'floor':1, 'state':1, 'dir':1, 'door':1})
    
    print("\nBOUNDARY TESTS")
    test("Top Floor Only", 0b1000000, start_floor=0)
    test("Already At Request", 0b0000001, start_floor=0)
    test("At Top Request Top", 0b1000000, start_floor=6)

    print("\nDIRECTION REVERSAL")
    test("Zig-Zag", 0b1010101, start_floor=0)
    test("Bottom-Top-Middle", 0b1001001, start_floor=0)
    test("Start Middle Both Ways", 0b1010101, start_floor=3)

    print("\nIDLE FUNCTION TESTS")
    test("Finish At F7", 0b1000000, start_floor=0)
    test("Finish At F4", 0b0001000, start_floor=0)
    test("No Requests At F5", 0b0000000, start_floor=4)

    

    print("\nADJACENT FLOORS")
    test("All Adjacent", 0b0111111, start_floor=0)
    test("Two Adjacent", 0b0000011, start_floor=0)

    print("\nEDGE CASES")
    test("No Requests", 0b0000000, start_floor=0)
    test("Skip Current", 0b1111110, start_floor=0)
    test("Even Floors Only", 0b0101010, start_floor=0)
    test("Odd Floors Only", 0b1010101, start_floor=0)

    print("\nRANDOM CASES")
    test("Random 1", 0b000011, start_floor=0)
    test("Random 2", 0b0000111, start_floor=0)
    test("Random 3", 0b1000001, start_floor=3)