import sys
import time
import threading
import liblo
from pydispatch import dispatcher
from collections import deque
import numpy as np

import el_wire

BR_BUFFER_SIZE = 2048

PWM_BASE_ADDRESS = 0x40

BRIGHTNESS_SIGNAL = 0
STATE_SIGNAL = 1
SYNC_SIGNAL = 2
USED_SIGNAL = 3
DATA_VIZ_SIGNAL = 4

STATE_NONE = -1
STATE_CIRCLES_HB = 0
STATE_CIRCLES_ON = 1
STATE_TOO_FAST = 2
STATE_GOOD_STOP = 3
STATE_BAD_STOP = 4
STATE_START = 5

START_ROAD = 5

ROAD_AMNT = 5
CIRCLE_AMNT = 5


class Station(threading.Thread):
    def __init__(self, _listen_port, _target_port, _target_ip, _id):
        threading.Thread.__init__(self)

        self.listen_port = _listen_port
        self.target_port = _target_port
        self.target_ip = _target_ip
        self.id = _id
        self.is_running = True
        # self.base_address = '/station/%d' % self.id

        el_address = self.id + int(PWM_BASE_ADDRESS)
        self.circles = np.empty(shape=(CIRCLE_AMNT,), dtype=object)
        self.roads = np.empty(shape=(ROAD_AMNT,), dtype=object)

        for i in range(0, CIRCLE_AMNT):
            self.circles[i] = el_wire.Circle(el_address, i)
            self.circles[i].start()

        for i in range(0, ROAD_AMNT):
            self.roads[i] = el_wire.Road(el_address, i + CIRCLE_AMNT)
            self.roads[i].start()

        self.server = Server(self.listen_port, self.target_port, self.target_ip, self.id)
        self.server.add_brightness_listener(self.brightness_callback)
        self.server.add_state_listener(self.state_callback)
        self.server.add_sync_listener(self.sync_callback)
        # self.server.add_data_viz_listener(self.too_fast_callback)
        self.server.start()

        self.buffer = Buffer(BR_BUFFER_SIZE)
        self.state = STATE_NONE
        # self.el_state = el_wire.CIRCLE_STATE_NONE
        # self.prev_el_state = el_wire.CIRCLE_STATE_NONE
        self.el_state = Buffer(2)
        for i in range(0, 2):
            self.el_state.add_sample(el_wire.CIRCLE_STATE_NONE)
        self.pulseCnt = 0

    def run(self):
        while self.is_running:

            if self.state == STATE_NONE:
                self.el_state.add_sample(el_wire.CIRCLE_STATE_NONE)

                if not self.buffer.is_reset:
                    self.buffer.reset()
                    # print 'reset buffer'

                # if self.el_state.get_sample(0) is not self.el_state.get_sample(1):
                    # print 'set state to none'
                for circle in self.circles:
                    circle.set_state(self.el_state.get_sample(0))

                for road in self.roads:
                    road.pause()
                    road.set_state(el_wire.ROAD_STATE_NONE)

                # print 'state_none'

            elif self.state == STATE_CIRCLES_HB:
                self.el_state.add_sample(el_wire.CIRCLE_STATE_HB)

                if self.el_state.get_sample(0) is not self.el_state.get_sample(1):
                    for circle in self.circles:
                        # print 'setting hartbeat'
                        circle.set_state(self.el_state.get_sample(0))

                    for road in self.roads:
                        road.set_state(el_wire.ROAD_STATE_HB)

                for i in range(0, CIRCLE_AMNT):
                    index = int((BR_BUFFER_SIZE / CIRCLE_AMNT) * i)
                    self.circles[i].set_hb(self.buffer.get_sample(index))

                # print 'state_circles_hb'

            elif self.state == STATE_CIRCLES_ON:
                self.el_state.add_sample(el_wire.CIRCLE_STATE_FADE)

                if self.el_state.get_sample(0) is not self.el_state.get_sample(1):
                    for circle in self.circles:
                        # print 'setting hartbeat'
                        circle.set_state(self.el_state.get_sample(0))

                # print 'state_circles_on'

            elif self.state == STATE_TOO_FAST:
                self.el_state.add_sample(el_wire.CIRCLE_STATE_NONE)

                if self.el_state.get_sample(0) is not self.el_state.get_sample(1):
                    for circle in self.circles:
                        # print 'setting hartbeat'
                        circle.set_state(self.el_state.get_sample(0))

                    for road in self.roads:
                        road.set_state(el_wire.ROAD_STATE_NONE)
                # print 'state_too_fast'

            elif self.state == STATE_BAD_STOP:
                self.el_state.add_sample(el_wire.CIRCLE_STATE_BAD_STOP)

                if self.el_state.get_sample(0) is not self.el_state.get_sample(1):
                    for circle in self.circles:
                        circle.set_state(self.el_state.get_sample(0))

                        for road in self.roads:
                            road.set_state(el_wire.ROAD_STATE_BAD_STOP)

                # print 'state_bad_stop'

            elif self.state == STATE_GOOD_STOP:
                self.el_state.add_sample(el_wire.CIRCLE_STATE_GOOD_STOP)

                if self.el_state.get_sample(0) is not self.el_state.get_sample(1):
                    for circle in self.circles:
                        circle.set_state(self.el_state.get_sample(0))

                    for road in self.roads:
                        road.set_state(el_wire.ROAD_STATE_GOOD_STOP)
                # print 'state_good_stop'

            elif self.state == STATE_START:
                self.el_state.add_sample(el_wire.CIRCLE_STATE_NONE)
                # print 'state_start'

            # print 'el state: %d' % self.el_state.get_sample(0)

            time.sleep(0.01)

    def quit(self):
        self.server.quit()

        for circle in self.circles:
            circle.quit()

        for road in self.roads:
            road.quit()

        self.is_running = False

    def brightness_callback(self, sender, brightness):
        brightness /= 255
        brightness *= 4096
        # print brightness
        self.buffer.add_sample(brightness)
        # self.buffer.print_buffer()

    def state_callback(self, sender, state, _sid):
        self.state = state

    def sync_callback(self, sender, sync):
        self.pulseCnt = sync

        if (START_ROAD + ROAD_AMNT) > self.pulseCnt >= START_ROAD:
            print 'station %d: START_ROAD : %d' % (self.id, self.pulseCnt - START_ROAD)
            self.roads[self.pulseCnt - START_ROAD].resume()

    # def too_fast_callback(self, sender, time):
    #     print 'this user spent %d on cranking too fast' % time


class Server(threading.Thread):
    def __init__(self, _listen_port, _target_port, _target_ip, _id):
        threading.Thread.__init__(self)
        self.is_running = True
        self.target_port = _target_port
        self.target_ip = _target_ip
        self.id = _id
        self.base_address = '/station/%d' % self.id

        try:
            self.server = liblo.Server(_listen_port)
        except liblo.ServerError, err:
            print str(err)
            sys.exit()

        try:
            self.target = liblo.Address(self.target_ip, self.target_port)
        except liblo.AddressError, err:
            print str(err)
            sys.exit()

        self.server.add_method(self.base_address + '/brightness', 'f', self.set_brightness)
        self.server.add_method(self.base_address + '/cd_state', 'i', self.set_state)
        self.server.add_method(self.base_address + '/pulse_sync', 'i', self.set_sync)
        self.server.add_method(self.base_address + '/used', 'i', self.set_used)
        self.server.add_method(self.base_address + '/data_viz', 'ihh', self.set_data_viz)
        self.server.add_method(self.base_address + '/rpm', 'i', self.set_rpm)

    def run(self):
        while self.is_running:
            self.server.recv(100)

    def quit(self):
        self.is_running = False

    def send_data(self, tot_used):
        # print 'sending data to %s:%d | tot_users: %d' % (self.target_ip, self.target_port, tot_used)
        bundle = liblo.Bundle(liblo.time(), liblo.Message("/used", int(tot_used)))
        liblo.send(self.target, bundle)

    def set_brightness(self, path, args):
        dispatcher.send(signal=BRIGHTNESS_SIGNAL, sender=self, brightness=args[0])

    def set_state(self, path, args):
        print 'station %d: received state: %d' % (self.id, args[0])
        dispatcher.send(signal=STATE_SIGNAL, sender=self, state=args[0], _sid=self.id)

    def set_sync(self, path, args):
        print 'station %d: received sync pulse# : %d' % (self.id, args[0])
        dispatcher.send(signal=SYNC_SIGNAL, sender=self, sync=args[0])

    def set_used(self, path, args):
        # print 'being used'
        # print self.id
        dispatcher.send(signal=USED_SIGNAL, sender=self, used=args[0], _sid=self.id)

    def set_data_viz(self, path, args):
        dispatcher.send(signal=DATA_VIZ_SIGNAL, sender=self, data=args, _sid=self.id)

    def set_rpm(self, path, args):
        print 'station %d: being cranked at %d rpm' % (self.id, args[0])
        time.sleep(0.01)

    def add_used_listener(self, callback):
        dispatcher.connect(callback, signal=USED_SIGNAL, sender=self)

    def add_state_listener(self, callback):
        dispatcher.connect(callback, signal=STATE_SIGNAL, sender=self)

    def add_sync_listener(self, callback):
        dispatcher.connect(callback, signal=SYNC_SIGNAL, sender=self)

    def add_brightness_listener(self, callback):
        dispatcher.connect(callback, signal=BRIGHTNESS_SIGNAL, sender=self)

    def add_data_viz_listener(self, callback):
        dispatcher.connect(callback, signal=DATA_VIZ_SIGNAL, sender=self)


class Buffer(object):
    def __init__(self, _length):
        self.buf_len = _length
        self.buffer = deque(maxlen=self.buf_len)

        self.is_reset = False
        # init buffer
        self.reset()

    def add_sample(self, sample):
        self.buffer.appendleft(int(sample))
        self.is_reset = False

    def get_sample(self, index):
        return self.buffer[index]

    def reset(self):
        for i in range(self.buf_len):
            self.buffer.appendleft(0)
        self.is_reset = True

    def print_buffer(self):
        print self.buffer
