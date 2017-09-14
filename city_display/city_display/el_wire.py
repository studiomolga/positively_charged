import time
import random
import Adafruit_PCA9685
import threading

import numpy as np

CIRCLE_STATE_NONE = 0
CIRCLE_STATE_HB = 1
CIRCLE_STATE_FADE = 2
CIRCLE_STATE_GOOD_STOP = 3
CIRCLE_STATE_BAD_STOP = 4

CIRCLE_RAND_SPEED_MIN = 10                      #with kasia its called circle_good_speed_min, max and step_size
CIRCLE_RAND_SPEED_MAX = 100
CIRCLE_RAND_STEP_SIZE = 1024
CIRCLE_GOOD_FADE_DECR = 0.0009

CIRCLE_BAD_SPEED_MIN = 250
CIRCLE_BAD_SPEED_MAX = 400
CIRCLE_BAD_FADE_DECR = 0.005
CIRCLE_BAD_MAX_SLEEP = 4000.0

ROAD_STATE_NONE = 0
ROAD_STATE_HB = 1
ROAD_STATE_GOOD_STOP = 2
ROAD_STATE_BAD_STOP = 3

ROAD_RAND_SPEED_MIN = 10
ROAD_RAND_SPEED_MAX = 100
ROAD_RAND_STEP_SIZE = 1024
ROAD_GOOD_FADE_DECR = 0.0009

ROAD_BAD_SPEED_MIN = 250
ROAD_BAD_SPEED_MAX = 400
ROAD_BAD_FADE_DECR = 0.005
ROAD_BAD_MAX_SLEEP = 4000.0

# GOOD_STOP_FADE_DECR = 0.0009


class ElWire:
    def __init__(self, address, channel):
        # threading.Thread.__init__(self)
        self.has_pwm = True
        try:
            self.pwm = Adafruit_PCA9685.PCA9685(address=address)
            self.pwm.set_pwm_freq(60)
        except IOError as e:
            print e
            self.has_pwm = False

        self.channel = channel
        self.brightness = 0

    def set_brightness(self, br):
        self.brightness = br
        if self.has_pwm:
            # print 'setting brightness at: %d' % self.brightness
            self.pwm.set_pwm(self.channel, 0, self.brightness)

    def get_brightness(self):
        return self.brightness


class Circle(threading.Thread):
    def __init__(self, address, channel):
        threading.Thread.__init__(self)
        self.el_wire = ElWire(address, channel)
        # self.set_brightness(0)

        self.is_running = True
        # self.daemon = True

        self.state = CIRCLE_STATE_NONE
        self.hb = 0
        self.is_max_br = False
        self.brightness = 0

        self.is_dest = True
        self.speed = 0
        self.dest = 0

        self.fade = 1.0

    def run(self):
        while self.is_running:
            # print self.state
            if self.state == CIRCLE_STATE_NONE:
                # print 'circle off'
                self.el_wire.set_brightness(0)
                self.is_max_br = False
                self.fade = 1.0
            elif self.state == CIRCLE_STATE_HB:
                # print 'circle hb'
                self.el_wire.set_brightness(self.hb)
                self.is_max_br = False
                self.fade = 1.0
            elif self.state == CIRCLE_STATE_FADE or self.state == CIRCLE_STATE_GOOD_STOP:
                # print self.hb
                if not self.is_max_br:
                    self.brightness = self.hb
                    self.is_max_br = True

                if self.is_dest:
                    if self.brightness < CIRCLE_RAND_STEP_SIZE:
                        self.dest = random.randint(3000, 4096)
                        self.speed = random.randint(100, 150)

                    else:
                        # min = self.brightness - 512
                        min_dest = self.brightness - CIRCLE_RAND_STEP_SIZE if self.brightness - CIRCLE_RAND_STEP_SIZE >= 0 else 0
                        max_dest = self.brightness + CIRCLE_RAND_STEP_SIZE if self.brightness + CIRCLE_RAND_STEP_SIZE <= 4095 else 4095
                        self.dest = random.randint(min_dest, max_dest)
                        self.speed = random.randint(CIRCLE_RAND_SPEED_MIN, CIRCLE_RAND_SPEED_MAX)
                        self.speed = self.speed if self.dest >= self.brightness else -self.speed
                    self.is_dest = False
                    # if self.channel == 5:
                    #     print 'dest = %d and speed = %d' % (self.dest, self.speed)
                else:
                    self.brightness += self.speed
                    self.brightness = 4095 if self.brightness > 4095 else self.brightness
                    self.brightness = 0 if self.brightness < 0 else self.brightness
                    # print int(self.brightness * self.fade)

                    if self.fade <= 0.0:
                        self.fade = 0.0

                    # self.brightness = int(float(self.brightness) * float(self.fade))
                    # print 'brightness: %d | fade: %f' % (int(float(self.brightness) * float(self.fade)), self.fade)
                    self.el_wire.set_brightness(int(float(self.brightness) * float(self.fade)))
                    if self.speed < 0:
                        self.is_dest = True if self.brightness <= self.dest else False
                    else:
                        self.is_dest = True if self.brightness >= self.dest else False

                if self.state == CIRCLE_STATE_GOOD_STOP:
                    self.fade -= CIRCLE_GOOD_FADE_DECR
                    # print self.fade
                else:
                    self.fade = 1.0
            elif self.state == CIRCLE_STATE_BAD_STOP:
                # print self.fade
                if self.brightness <= 0:
                    max_sleep = CIRCLE_BAD_MAX_SLEEP * (abs(self.fade - 1.0))
                    # print max_sleep
                    time.sleep(float(random.randint(0, int(max_sleep))) / 1000.0)
                    # self.dest = random.randint(3000, 4096)
                    self.speed = random.randint(CIRCLE_BAD_SPEED_MIN, CIRCLE_BAD_SPEED_MAX)

                if self.brightness >= 4095:
                    # self.dest = random.randint(0, 1024)
                    self.speed = -(random.randint(CIRCLE_BAD_SPEED_MIN, CIRCLE_BAD_SPEED_MAX))

                self.brightness += self.speed
                self.brightness = 4095 if self.brightness > 4095 else self.brightness
                self.brightness = 0 if self.brightness < 0 else self.brightness

                if self.fade <= 0.0:
                    self.fade = 0.0

                self.el_wire.set_brightness(int(float(self.brightness) * float(self.fade)))

                self.fade -= CIRCLE_BAD_FADE_DECR

                # self.el_wire.set_brightness(self.brightness)

            time.sleep(0.001)

    def quit(self):
        self.el_wire.set_brightness(0)
        time.sleep(0.1)
        self.is_running = False

    def set_state(self, state):
        self.state = state

    # def get_state(self):
    #     return self.state

    def set_hb(self, hb):
        # print 'setting hb: %d' % hb
        self.hb = hb


class Road(threading.Thread):
    def __init__(self, address, channel):
        threading.Thread.__init__(self)
        self.el_wire = ElWire(address, channel)
        self.el_wire.set_brightness(0)

        self.channel = channel

        self.is_running = True
        # self.daemon = True

        self.paused = True  # start out paused
        self.thread_state = threading.Condition()

        self.is_dest = True
        self.brightness = 0
        self.speed = 0
        self.dest = 0
        self.state = ROAD_STATE_NONE
        self.fade = 1.0

        # self.state = CIRCLE_STATE_NONE
        # self.hb = 0

    def run(self):
        while self.is_running:
            with self.thread_state:
                if self.paused:
                    # if self.brightness > 0:
                    #     self.brightness -= 48
                    #     self.el_wire.set_brightness(self.brightness)
                    # else:
                    self.brightness = 0
                    #     self.el_wire.set_brightness(self.brightness)
                    self.el_wire.set_brightness(self.brightness)
                    self.thread_state.wait()  # block until notified

            # if not self.paused:
            if self.state == ROAD_STATE_NONE:
                self.fade = 1.0
                self.brightness = 0
                self.el_wire.set_brightness(self.brightness)
            elif self.state == ROAD_STATE_HB or self.state == ROAD_STATE_GOOD_STOP:
                if self.is_dest:
                    if self.brightness < ROAD_RAND_STEP_SIZE:
                        self.dest = random.randint(3000, 4096)
                        self.speed = random.randint(100, 150)

                    else:
                        # min = self.brightness - 512
                        min_dest = self.brightness - ROAD_RAND_STEP_SIZE if self.brightness - ROAD_RAND_STEP_SIZE >= 0 else 0
                        max_dest = self.brightness + ROAD_RAND_STEP_SIZE if self.brightness + ROAD_RAND_STEP_SIZE <= 4095 else 4095
                        self.dest = random.randint(min_dest, max_dest)
                        self.speed = random.randint(ROAD_RAND_SPEED_MIN, ROAD_RAND_SPEED_MAX)
                        self.speed = self.speed if self.dest >= self.brightness else -self.speed
                    self.is_dest = False
                    # if self.channel == 5:
                    #     print 'dest = %d and speed = %d' % (self.dest, self.speed)
                else:
                    self.brightness += self.speed
                    self.brightness = 4095 if self.brightness > 4095 else self.brightness
                    self.brightness = 0 if self.brightness < 0 else self.brightness

                    if self.fade <= 0.0:
                        self.fade = 0.0

                    self.el_wire.set_brightness(int(float(self.brightness) * float(self.fade)))
                    if self.speed < 0:
                        self.is_dest = True if self.brightness <= self.dest else False
                    else:
                        self.is_dest = True if self.brightness >= self.dest else False

                if self.state == ROAD_STATE_GOOD_STOP:
                    self.fade -= ROAD_GOOD_FADE_DECR
                    # print self.fade
                else:
                    self.fade = 1.0
            elif self.state == ROAD_STATE_BAD_STOP:
                # print self.fade
                if self.brightness <= 0:
                    max_sleep = ROAD_BAD_MAX_SLEEP * (abs(self.fade - 1.0))
                    # print max_sleep
                    time.sleep(float(random.randint(0, int(max_sleep))) / 1000.0)
                    # self.dest = random.randint(3000, 4096)
                    self.speed = random.randint(ROAD_BAD_SPEED_MIN, ROAD_BAD_SPEED_MAX)

                if self.brightness >= 4095:
                    # self.dest = random.randint(0, 1024)
                    self.speed = -(random.randint(ROAD_BAD_SPEED_MIN, ROAD_BAD_SPEED_MAX))

                self.brightness += self.speed
                self.brightness = 4095 if self.brightness > 4095 else self.brightness
                self.brightness = 0 if self.brightness < 0 else self.brightness

                if self.fade <= 0.0:
                    self.fade = 0.0

                self.el_wire.set_brightness(int(float(self.brightness) * float(self.fade)))

                self.fade -= ROAD_BAD_FADE_DECR

            # self.el_wire.set_brightness(4095)
            time.sleep(0.001)

    def resume(self):
        with self.thread_state:
            self.paused = False
            self.thread_state.notify()  # unblock self if waiting

    def pause(self):
        with self.thread_state:
            self.paused = True  # make self block and wait

    def quit(self):
        # self.el_wire.set_brightness(0)
        # time.sleep(0.1)
        if self.paused:
            self.resume()
        self.is_running = False

    def set_state(self, state):
        self.state = state


if __name__ == '__main__':
    test_circles = np.empty(shape=(10,), dtype=object)
    for i in range(0, 10):
        test_circles[i] = Circle(0 + int(0x40), i)
        test_circles[i].start()
        test_circles[i].set_state(CIRCLE_STATE_HB)
    # circle = Circle(0 + int(0x40), 6)
    # circle.start()
    # circle.set_state(CIRCLE_STATE_HB)
    bright = 0

    while True:
        bright += 48
        bright %= 4095

        for test_circle in test_circles:
            test_circle.set_hb(bright)
        # circle.set_hb(bright)
        time.sleep(0.01)
