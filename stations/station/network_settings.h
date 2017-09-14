//network
const byte inIp[] {192,168, 1, 3};                                          //ip address of the this arduino, (station 0 = 192.168.1.2, station 1 = 192.168.1.3, etc)         SEMI
const byte outIp[] {192,168, 1, 1};                                         //do not change, ip address of rpi
const byte mac[] {0x90, 0xA2, 0xDA, 0x11, 0x11, 0x4C};                      //mac address of ethernet shield, there is a sticker on the shield with the mac address           SEMI
#define NETWORK_IN_PORT 8880                                                //in port, station 0 = 8880, station 1 = 8881, etc                                                SEMI
#define NETWORK_OUT_PORT 9990                                               //out port, station 0 = 9990, station 1 = 9991, etc                                               SEMI
