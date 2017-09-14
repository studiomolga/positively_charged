#!/usr/bin/env python

import sys
import os
import numpy as np
import liblo

from station import Station

STATION_AMNT = 5                   #if it turns out that it cant handle 10, change this to 5
START_LISTEN_PORT = 9990
START_TARGET_PORT = 8880
START_IP = 3
IP_BASE = '192.168.1.%d'
DATAVIZ_IP = '192.168.1.105'
SOUND_IP = '192.168.1.105'
DATAVIZ_PORT = 7000
SOUND_PORT = 7001

used_list = np.zeros(STATION_AMNT)
stations = np.empty(shape=(STATION_AMNT,), dtype=object)

try:
    data_viz_target = liblo.Address(DATAVIZ_IP, DATAVIZ_PORT)
except liblo.AddressError, err:
    print 'error with data viz connection'
    print str(err)
    sys.exit()

try:
    sound_target = liblo.Address(SOUND_IP, SOUND_PORT)
except liblo.AddressError, err:
    print 'error with data viz connection'
    print str(err)
    sys.exit()


def used_callback(sender, used, _sid):
    # print 'callback: %d' % _sid
    if used_list[_sid] != used:
        used_list[_sid] = used
        total_users = np.sum(used_list)
        for st in stations:
            st.server.send_data(total_users)

        bundle = liblo.Bundle(liblo.time(), liblo.Message("/used", total_users))
        liblo.send(data_viz_target, bundle)
        # liblo.send(sound_target, bundle)

        print 'station %d is being used, the total users is %d' % (_sid, total_users)


def data_viz_callback(sender, data, _sid):
    bundle = liblo.Bundle(liblo.time(), liblo.Message("/data", data[0], data[1], data[2], _sid))
    liblo.send(data_viz_target, bundle)
    print 'station %d sends the following data to data viz:\n' \
          'total heartbeats:    %d\n' \
          'total crank time:    %d\n' \
          'total too fast time: %d\n' \
          '--------------------------------------' % (_sid, data[0], data[1], data[2])


def sound_callback(sender, state, _sid):
    if state == 1:
        bundle = liblo.Bundle(liblo.time(), liblo.Message("/circle_on", _sid))
        # liblo.send(data_viz_target, bundle)
        liblo.send(sound_target, bundle)
    elif state == 5:
        bundle = liblo.Bundle(liblo.time(), liblo.Message("/start", _sid))
        # liblo.send(data_viz_target, bundle)
        liblo.send(sound_target, bundle)
    elif state == 3 or state == 4:
        bundle = liblo.Bundle(liblo.time(), liblo.Message("/stop", _sid))
        # liblo.send(data_viz_target, bundle)
        liblo.send(sound_target, bundle)


for ip, listen_port, target_port, sid in zip(range(START_IP, START_IP + STATION_AMNT),
                                             range(START_LISTEN_PORT, START_LISTEN_PORT + STATION_AMNT),
                                             range(START_TARGET_PORT, START_TARGET_PORT + STATION_AMNT),
                                             range(0, STATION_AMNT)):
    # print ip, listen_port, target_port, sid
    try:
        stations[sid] = Station(listen_port, target_port, IP_BASE % ip, sid)
    except liblo.ServerError, err:
        print str(err)
        sys.exit()

    stations[sid].server.add_used_listener(used_callback)
    stations[sid].server.add_data_viz_listener(data_viz_callback)
    stations[sid].server.add_state_listener(sound_callback)
    stations[sid].start()


print 'started'
raw_input('press enter to quit')
print 'quitting...'

for station in stations:
    station.quit()

print 'see you next time!!'
os.system("sudo shutdown -h now")
