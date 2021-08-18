//for the network settings see network_settings.h!!

//log to serial port
#define LOG false                                               //turn on logging to serial monitor
#define SEND_RPM false

//pin definitions
#define REED_PIN 8                                            //do not change
#define PULSE_PIN 0                                           //do not change
#define EL_PIN 9                                              //do not change

#define STATION_ID 0                                          //station id number, start at 0                                                               SEMI
#define STATION_AMOUNT 5                                     //the total number of stattions                                                               SEMI

//states
#define STATE_NONE -1                                         //do not change
#define STATE_WAIT 0                                          //do not change
#define STATE_START 1                                         //do not change
#define STATE_IDLE 2                                          //do not change
#define STATE_HEART 3                                         //do not change
#define STATE_TOO_FAST 4                                      //do not change
#define STATE_STOP 5

//city display states
#define CD_STATE_NONE -1                                      //do not change
#define CD_STATE_CIRCLES_HB 0                                 //do not change
#define CD_STATE_CIRCLES_ON 1                                 //do not change
#define CD_STATE_TOO_FAST 2                                   //do not change
#define CD_STATE_GOOD_STOP 3                                  //do not change
#define CD_STATE_BAD_STOP 4                                   //do not change
#define CD_STATE_START 5

#define IDLE_PULSE_CHANCE 1000000                             //the bigger the number the less likely it becomes we have a ranom flash                      KASIA
#define IDLE_PULSE_CHANCE_INC 60000                          //the bigger the number the longer it takes for random flashed to increase in frequency       KASIA

#define FAST_PULSE_CHANCE 10000                               //the bigger the number the less likely it becomes we have a ranom flash                      KASIA
#define FAST_PULSE_CHANCE_INC 500                             //the bigger the number the longer it takes for random flashed to increase in frequency       KASIA

//#define IDLE_PULSE_CHANCE 10000
//#define IDLE_PULSE_CHANCE_INC 3000

//crank
#define MIN_CRANK_TIME 500                                    //the debounce time, in ms                                                                    SEMI
#define CRANK_RESET_TIME 4000                                 //resets rpm when there is no cranking, in ms                                                 SEMI/KASIA
#define CRANK_IDLE_TIME 5000                                  //time after which we are idle, in ms                                                         KASIA
#define CRANK_TIME_THRESH 3000                                //crank this long to start the el wire and actuator, in ms                                    KASIA
#define MAX_CRANK_RPM 200                                     //could be adjusted but only if necesary, just there for a ceiling
#define CRANK_RPM_THRESH 75                                   //the threshold after which we consider the cranking to be too fast                           KASIA

#define STOP_MIN_CRANK_TIME 10000
#define STOP_MAX_FAST_TIME 3000
#define STOP_TIME 20000

//EL
#define EL_FADE_DIR_UP 0                                      //do not change
#define EL_FADE_DIR_DOWN 1                                    //do not change
#define EL_MAX_BRIGHTNESS 255                                 //maximum brightness, 0-255                                                                   KASIA
#define EL_RANDOM_SPEED_MIN 10                                //minimum fade speed when in random mode                                                      KASIA
#define EL_RANDOM_SPEED_MAX 250                               //minimum fade speed when in random mode                                                      KASIA
#define EL_PULSE_ONCE 0                                       //do not change
#define EL_PULSE_INFINITE 1                                   //do not change

#define EL_PULSE_THRESH1 10
#define EL_PULSE_THRESH2 20
