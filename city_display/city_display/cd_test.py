import numpy as np
import time
import el_wire

STATION_AMNT = 10
EL_WIRE_AMNT = 10
PWM_BASE_ADDRESS = 0x40

elwires = np.empty(shape=(EL_WIRE_AMNT,), dtype=object)

for i in range(0, STATION_AMNT):
    for j in range(0, EL_WIRE_AMNT):
        print j, i + int(PWM_BASE_ADDRESS)
        elwires[i] = el_wire.ElWire(i + int(PWM_BASE_ADDRESS), j)


brightness = 0

while True:
    for elwire in elwires:
        elwire.set_brightness(brightness)

    brightness += 1
    brightness %= 4095

    print brightness

    time.sleep(0.001)

    # time.sleep(0.01)
