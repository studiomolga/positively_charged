#include "constants.h"

#include <Ethernet2.h>
#include <EthernetUdp2.h>
#include <SPI.h>    
#include <OSCMessage.h>
#include <OSCBundle.h>

#include "crank.h"
#include "pulsesensor.h"
#include "elwire.h"
#include "actuator.h"
#include "network_settings.h"

int crankRPM, prevCrankRPM;
int pulseBPM;
int animState[2];
int cdState[2];
int pulseAmnt[2];
int pulseTotal;
unsigned long stateStartTime;
unsigned long startTime;
unsigned long idleTime;
unsigned long crankTime;
unsigned long tooFastStart;
unsigned long tooFastTime;
long pulseChance;
String oscBaseAddress;
boolean hasStopped;

Crank crank;
PulseSensor pulseSensor;
ElWire elWire;
Actuator actuator;
EthernetUDP Udp;

void setup() {
  
  if(LOG){
    Serial.begin(9600);
    Serial.println("starting Positively Charged Module...");
  }
  
  oscBaseAddress = "/station/";
  oscBaseAddress.concat(String(STATION_ID));

  Ethernet.begin(mac, inIp);
  Udp.begin(NETWORK_IN_PORT);
  actuator = Actuator();
  actuator.init();
  crank = Crank();
  crank.startCrank();
  pulseSensor = PulseSensor();
  elWire = ElWire();
  crankRPM = 0;
  prevCrankRPM = 0;
  stateStartTime = 0;
  idleTime = 0;
  crankTime = 0;
  tooFastTime = 0;
  tooFastStart = 0;
  animState[0] = STATE_NONE;
  animState[1] = STATE_NONE;
  cdState[0] = CD_STATE_NONE;
  cdState[1] = CD_STATE_NONE;
  pulseAmnt[0] = 0;
  pulseAmnt[1] = 0;
  pulseChance = 0;
  pulseTotal = 0;
  hasStopped = false;
}

void loop() {
  recvOSCMsgs();
  crank.updateCrank();
  pulseSensor.updatePulse();
  actuator.updateAct();

  prevCrankRPM = crankRPM;
  if(animState[0] == STATE_STOP){
    crankRPM = 0;
  } else {
    crankRPM = crank.getRPM();
  }
  if(SEND_RPM){
    sendOSCMessage((oscBaseAddress + "/rpm").c_str(), crankRPM);
//    delay(100);
  }

  setState();
  elWire.setState(animState[0]);

//  prevCrankRPM = crankRPM;
//  if(animState[0] = 
//  crankRPM = crank.getRPM();

  bool isPulse = elWire.getIsPulse();

  pulseAmnt[1] = pulseAmnt[0];
  pulseAmnt[0] = elWire.getPulses();

  if(animState[0] != animState[1]){
    sendOSCMessage((oscBaseAddress + "/state").c_str(), animState[0]);
  }

  if(animState[0] == STATE_WAIT || animState[0] == STATE_START){
    if(animState[0] != animState[1]){
      if(isPulse){
        elWire.stopPulse();
      }

      if(animState[1] == STATE_HEART){
        actuator.stopPulse();
      }

      if(animState[1] == STATE_TOO_FAST && animState[0] == STATE_START){
        tooFastTime += millis() - tooFastStart;
        actuator.stopPulse();
        cdState[0] = cdState[1];
        sendOSCMessage((oscBaseAddress + "/cd_state").c_str(), cdState[0]);
      }

      if(animState[0] == STATE_START){
        sendOSCMessage((oscBaseAddress + "/used").c_str(), 1);
        cdState[1] = cdState[0];
        cdState[0] = CD_STATE_START;
        sendOSCMessage((oscBaseAddress + "/cd_state").c_str(), cdState[0]);
      }

      if(animState[0] == STATE_WAIT){
        sendOSCMessage((oscBaseAddress + "/too_fast").c_str(), (int) tooFastTime);
        tooFastTime = 0;
        sendOSCMessage((oscBaseAddress + "/used").c_str(), 0);
        cdState[1] = cdState[0];
        cdState[0] = CD_STATE_NONE;
        sendOSCMessage((oscBaseAddress + "/cd_state").c_str(), cdState[0]);
      }
    }
  }

  if(animState[0] == STATE_IDLE){   
    if(animState[0] != animState[1]){
      pulseChance = 0;
      stateStartTime = millis();
      if(isPulse){
        elWire.stopPulse();
      }
      sendOSCMessage((oscBaseAddress + "/used").c_str(), 0);
      cdState[1] = cdState[0];
      cdState[0] = CD_STATE_NONE;
      sendOSCMessage((oscBaseAddress + "/cd_state").c_str(), cdState[0]);
    }

    if(animState[1] == STATE_TOO_FAST){
      tooFastTime += millis() - tooFastStart;
      actuator.stopPulse();
      cdState[0] = cdState[1];
      sendOSCMessage((oscBaseAddress + "/cd_state").c_str(), cdState[0]);
    }

    unsigned long deltaTime = millis() - stateStartTime;
    
    if(deltaTime % IDLE_PULSE_CHANCE_INC == 0){
      pulseChance++;
    }
    
    if(!isPulse && isRandomPulse(pulseChance, IDLE_PULSE_CHANCE)){
      elWire.setFreq(randomSpeed(EL_RANDOM_SPEED_MIN, EL_RANDOM_SPEED_MAX));
      elWire.triggerPulse();
    }
    
    if(isPulse){
      elWire.pulse(EL_PULSE_ONCE);
    }
  }

  if(animState[0] == STATE_HEART){
    if(animState[0] != animState[1]){
      if(isPulse){
        elWire.stopPulse();
      }
      elWire.triggerPulse();
      actuator.triggerPulse();
      
      sendOSCMessage((oscBaseAddress + "/used").c_str(), 1);
    }    

    if(animState[1] == STATE_TOO_FAST){
      tooFastTime += millis() - tooFastStart;
      actuator.stopPulse();
      cdState[0] = cdState[1];
      sendOSCMessage((oscBaseAddress + "/cd_state").c_str(), cdState[0]);
    }
    
    if(isPulse){
      pulseBPM = pulseSensor.getBPM();
      elWire.setFreq(pulseBPM);
//      elWire.setFreq(70);
      elWire.pulse(EL_PULSE_INFINITE);
      actuator.setFreq(pulseBPM);
//      actuator.setFreq(70);
      actuator.pulse();
      sendOSCMessage((oscBaseAddress + "/brightness").c_str(), elWire.getBrightness());

      if(pulseAmnt[0] != pulseAmnt[1] && pulseAmnt[0] == EL_PULSE_THRESH1){
        cdState[1] = cdState[0];
        cdState[0] = CD_STATE_CIRCLES_HB;
        sendOSCMessage((oscBaseAddress + "/cd_state").c_str(), cdState[0]);
      }
      
      if(pulseAmnt[0] != pulseAmnt[1] && pulseAmnt[0] >= EL_PULSE_THRESH1 && pulseAmnt[0] < EL_PULSE_THRESH2){
        sendOSCMessage((oscBaseAddress + "/pulse_sync").c_str(), pulseAmnt[0] - EL_PULSE_THRESH1);
      }

      if(pulseAmnt[0] != pulseAmnt[1] && pulseAmnt[0] == EL_PULSE_THRESH2){
        cdState[1] = cdState[0];
        cdState[0] = CD_STATE_CIRCLES_ON;
        sendOSCMessage((oscBaseAddress + "/cd_state").c_str(), cdState[0]);
      }
    }  
  }

  if(animState[0] == STATE_TOO_FAST){   
    if(animState[0] != animState[1]){
      pulseChance = 0;
      tooFastStart = millis();
      pulseTotal += pulseAmnt[0];
      if(isPulse){
        elWire.stopPulse();
        actuator.stopPulse();
      }
      cdState[1] = cdState[0];
      cdState[0] = CD_STATE_TOO_FAST;
      sendOSCMessage((oscBaseAddress + "/cd_state").c_str(), cdState[0]);
    }

    unsigned long deltaTime = millis() - tooFastStart;
    
    if(deltaTime % FAST_PULSE_CHANCE_INC == 0){
      pulseChance++;
    }
    
    if(!isPulse && isRandomPulse(pulseChance, FAST_PULSE_CHANCE)){
      elWire.setFreq(randomSpeed(150, 250));
      elWire.triggerPulse();
    }
    
    if(isPulse){
      elWire.pulse(EL_PULSE_ONCE);
    }
  }

  if(animState[0] == STATE_STOP){
    if(animState[0] != animState[1]){
      stateStartTime = millis();
      pulseTotal += pulseAmnt[0];
      if(isPulse){
        elWire.stopPulse();
      }
      actuator.stopPulse();
      
      if(tooFastTime < STOP_MAX_FAST_TIME && millis() - crankTime > STOP_MIN_CRANK_TIME){
        cdState[1] = cdState[0];
        cdState[0] = CD_STATE_GOOD_STOP;
        sendOSCMessage((oscBaseAddress + "/cd_state").c_str(), cdState[0]);
      } else {
        cdState[1] = cdState[0];
        cdState[0] = CD_STATE_BAD_STOP;
        sendOSCMessage((oscBaseAddress + "/cd_state").c_str(), cdState[0]);
      }

//      Serial.println(tooFastTime);
//      Serial.println(millis() - crankTime);
//      Serial.println("____________");

      sendOSCDataViz((oscBaseAddress + "/data_viz").c_str(), pulseTotal, millis() - crankTime, tooFastTime);
      pulseTotal = 0;
      tooFastTime = 0;
//      crankTime = 0;
      
    }

    if(millis() - stateStartTime > STOP_TIME){
      hasStopped = false;
    }
    
  }
  
  if(LOG){
    Serial.print("crank rotation speed: ");
    Serial.print(crankRPM);
    Serial.print(" | hartrate: ");
    Serial.print(pulseBPM);
    Serial.print(" | state: ");
    Serial.println(animState[0]);
  }
}

bool isRandomPulse(long chance, long fromVal){
  long randVal = random(0, fromVal);
  if(LOG){
    Serial.print("chance: ");
    Serial.print(chance);
    Serial.print(" | randVal: ");
    Serial.print(randVal);
    Serial.print(" | trigger pulse: ");
    Serial.println(randVal < chance);
  }
  if(randVal < chance){
    return true;
  } else {
    return false;
  }
}

int randomSpeed(int minS, int maxS){
  int rs = (int)random(minS, maxS);
  if(LOG){
    Serial.print("random speed: ");
    Serial.println(rs);
  }
  return rs;
}

void setState(){
  unsigned long currTime = millis();

  animState[1] = animState[0];

  //if crankrpm == 0 and animstate[0] is either too_fast or heart
  //  animstate[0] = state_stop
  //  boolean has_stopped = true  --> make has_stopped false somewhere
  if(crankRPM == 0 && (animState[0] == STATE_HEART || animState[0] == STATE_TOO_FAST)){
    hasStopped = true;
    animState[0] = STATE_STOP;
  }

//  if(crankRPM > 0 && (animState[0] == STATE_WAIT || animState[0] == STATE_IDLE || animState[0] == STATE_TOO_FAST)){
  if(crankRPM > 0 && (animState[0] == STATE_WAIT || animState[0] == STATE_IDLE && hasStopped == false)){
    crankTime = millis();
  }

  if(crankRPM != prevCrankRPM && crankRPM == 0 && hasStopped == false){        //and has_stopped = false
    idleTime = millis();
  }

  if(CRANK_RPM_THRESH > crankRPM > 0 && currTime - crankTime < CRANK_TIME_THRESH && hasStopped == false){      //and has_stopped = false
//  if(CRANK_RPM_THRESH > crankRPM > 0 && CRANK_TIME_THRESH > currTime - crankTime > 0 && hasStopped == false){      //and has_stopped = false
    animState[0] = STATE_START;
  }

  if(CRANK_RPM_THRESH > crankRPM > 0 && currTime - crankTime > CRANK_TIME_THRESH && hasStopped == false){    //and has_stopped = false
    animState[0] = STATE_HEART;
  }

  if(crankRPM > CRANK_RPM_THRESH && hasStopped == false){      //and has_stopped = false
    animState[0] = STATE_TOO_FAST;
  }

  if(crankRPM == 0 && currTime - idleTime < CRANK_IDLE_TIME && hasStopped == false){   //and has_stopped = false
    animState[0] = STATE_WAIT;
  }

  if(crankRPM == 0 && currTime - idleTime > CRANK_IDLE_TIME && hasStopped == false){   //and has_stopped = false
    animState[0] = STATE_IDLE;
  }
}

void routeUsed(OSCMessage &msg, int addrOffset){
  if(msg.getInt(0) >= 0 && msg.getInt(0) < STATION_AMOUNT){
//    int brakePos = ((MAX_BRAKE_POS - MIN_BRAKE_POS) / STATION_AMOUNT) * msg.getInt(0);
//    int brakePos = abs(((MAX_BRAKE_POS - MIN_BRAKE_POS) / STATION_AMOUNT) - STATION_AMOUNT) * msg.getInt(0);
    int brakePos = ((MAX_BRAKE_POS - MIN_BRAKE_POS) / STATION_AMOUNT) * abs(msg.getInt(0) - STATION_AMOUNT);
    brakePos += MIN_BRAKE_POS;
//    Serial.println(msg.getInt(0));
//    Serial.println(abs(msg.getInt(0) - STATION_AMOUNT));
//    brakePos = abs(brakePos - MAX_BRAKE_POS);
//    Serial.print("setting brake to: ");
//    Serial.println(brakePos);
    crank.setBrakePos(brakePos);
  } else {
    crank.setBrakePos(MIN_BRAKE_POS);
  }
}

void recvOSCMsgs(){
  OSCBundle bundleIN;
  int size;
  
  if( (size = Udp.parsePacket())>0)
  Serial.println("received message");
  {
   while(size--)
     bundleIN.fill(Udp.read());
  
    if(!bundleIN.hasError())
      bundleIN.route("/used", routeUsed);
  }
}

void sendOSCDataViz(const char* address, int val1, int32_t val2, int32_t val3){
  //the message wants an OSC address as first argument
  OSCMessage msg(address);
  msg.add(val1);
  msg.add(val2);
  msg.add(val3);
  
  Udp.beginPacket(outIp, NETWORK_OUT_PORT);
    msg.send(Udp); // send the bytes to the SLIP stream
  Udp.endPacket(); // mark the end of the OSC Packet
  msg.empty(); // free space occupied by message
}

void sendOSCMessage(const char* address, int val){
  //the message wants an OSC address as first argument
  OSCMessage msg(address);
  msg.add(val);
  
  Udp.beginPacket(outIp, NETWORK_OUT_PORT);
    msg.send(Udp); // send the bytes to the SLIP stream
  Udp.endPacket(); // mark the end of the OSC Packet
  msg.empty(); // free space occupied by message
}

