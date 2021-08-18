#include "constants.h"
#include "network_settings.h"

#include <Wire.h>
#include <Ethernet2.h>
#include <EthernetUdp2.h>
#include <SPI.h>
#include <OSCMessage.h>
#include <OSCBundle.h>

#include "elwire.h"

int hrBPM = 0;
int crankRPM = 0;
int prevCrankRPM = 0;
unsigned long stateStartTime = 0;
unsigned long startTime = 0;
unsigned long idleTime = 0;
unsigned long crankTime = 0;
unsigned long tooFastStart = 0;
unsigned long tooFastTime = 0;
boolean hasStopped = false;
int animState[2] = {STATE_NONE, STATE_NONE};
int pulseAmnt[2] = {0, 0};
int cdState[2] = {CD_STATE_NONE, CD_STATE_NONE};
String oscBaseAddress;
long pulseChance = 0;
int pulseTotal = 0;

ElWire elWire;
EthernetUDP Udp;

void setup()
{
//  Serial.begin(9600);
  if (LOG) {
    Serial.begin(9600);
    Serial.print("start positively charged station: ");
    Serial.println(STATION_ID);
  }

  Wire.begin(9);
  Wire.onReceive(receiveEvent);

  Ethernet.begin(mac, inIp);
  Udp.begin(NETWORK_IN_PORT);

  oscBaseAddress = "/station/";
  oscBaseAddress.concat(String(STATION_ID));

  elWire = ElWire();
}

// event for setting the heart rate bpm, when receiving via i2c
void receiveEvent(int bytes)
{
  crankRPM = Wire.read();
  hrBPM = Wire.read();
}

void loop()
{
  unsigned long currTime = millis();

  prevCrankRPM = crankRPM;

  setAnimState(currTime);

  handleAnimState(currTime);

  if (millis() - stateStartTime > STOP_TIME) {
    hasStopped = false;
  }

  if (LOG) {
    if(prevCrankRPM != crankRPM){
      Serial.print("crank RPM: ");
      Serial.println(crankRPM); 
      Serial.print("heartrate BPM: ");
      Serial.println(hrBPM);
    }
    if (animState[0] != animState[1]) {
      Serial.print("animation state: ");
      Serial.println(animState[0]);
    }
  }
}

void handleAnimState(unsigned long currTime) {
  unsigned long deltaTime = 0;

  pulseAmnt[1] = pulseAmnt[0];
  pulseAmnt[0] = elWire.getPulses();

//  Serial.print("anim states: ");
//  Serial.print(animState[0]);
//  Serial.print(" , ");
//  Serial.println(animState[1]);

  if (animState[0] != animState[1]) {
        sendOSCMessage((oscBaseAddress + "/state").c_str(), animState[0]);
  }

  switch(animState[0]){
    case STATE_NONE:
      break;
    case STATE_WAIT:
      
      if(hasStateChanged()){
//        Serial.println("WAITING");
        if(elWire.getIsPulse()){
          elWire.stopPulse();
        }
        
        cdState[1] = cdState[0];
        cdState[0] = CD_STATE_NONE;
        sendOSCMessage((oscBaseAddress + "/cd_state").c_str(), cdState[0]);

        if(animState[1] == STATE_TOO_FAST){
          sendOSCMessage((oscBaseAddress + "/too_fast").c_str(), (int) tooFastTime);
          tooFastTime = 0;
        }
      }
      break;
    case STATE_START:
      
      if(hasStateChanged()){
//        Serial.println("STARTING");
        if(elWire.getIsPulse()){
          elWire.stopPulse();
        }

        cdState[1] = cdState[0];
        cdState[0] = CD_STATE_START;
        sendOSCMessage((oscBaseAddress + "/cd_state").c_str(), cdState[0]);

        if(animState[1] == STATE_TOO_FAST){
          tooFastTime += currTime - tooFastStart;
          cdState[0] = cdState[1];
          sendOSCMessage((oscBaseAddress + "/cd_state").c_str(), cdState[0]);
        }
      }
      break;
    case STATE_IDLE:
    
      deltaTime = currTime - stateStartTime;

      if(hasStateChanged()){
//        Serial.println("IDLING");
        pulseChance = 0;
        stateStartTime = currTime;

        if(elWire.getIsPulse()){
          elWire.stopPulse();
        }

        cdState[1] = cdState[0];
        cdState[0] = CD_STATE_NONE;
        sendOSCMessage((oscBaseAddress + "/cd_state").c_str(), cdState[0]);
      }

      if(animState[1] == STATE_TOO_FAST){
        tooFastTime += currTime - tooFastStart;
        cdState[0] = cdState[1];
        sendOSCMessage((oscBaseAddress + "/cd_state").c_str(), cdState[0]);
      }

//      Serial.println(deltaTime % IDLE_PULSE_CHANCE_INC);
//      Serial.println(pulseChance);
      if(deltaTime % IDLE_PULSE_CHANCE_INC == 0){
//        Serial.println("ADDING PULSE CHANCE");
        pulseChance++;
      }

//      if(LOG){
//        Serial.print("delta time: ");
//        Serial.print(deltaTime);
//        Serial.print(" | mod delta time: ");
//        Serial.print(deltaTime % IDLE_PULSE_CHANCE_INC);
//        Serial.print(" | pulse chance incr: ");
//        Serial.println(IDLE_PULSE_CHANCE_INC);
//      }

//      Serial.println(isRandomPulse(pulseChance, IDLE_PULSE_CHANCE));
      if(!elWire.getIsPulse() && isRandomPulse(pulseChance, IDLE_PULSE_CHANCE)){
//        Serial.println("RANDOM TRIGGER");
        elWire.setFreq(randomSpeed(EL_RANDOM_SPEED_MIN, EL_RANDOM_SPEED_MAX));
        elWire.triggerPulse();
      }
      
      if(elWire.getIsPulse()){
        elWire.pulse(EL_PULSE_ONCE);
      }
      
      break;
    case STATE_HEART:
      
      if(hasStateChanged()){
//        Serial.println("HEART");
        if(elWire.getIsPulse()){
          elWire.stopPulse();
        }
        elWire.triggerPulse();
      }

      if(animState[1] == STATE_TOO_FAST){
        tooFastTime += currTime - tooFastStart;
        cdState[0] = cdState[1];
        sendOSCMessage((oscBaseAddress + "/cd_state").c_str(), cdState[0]);
      }
      
      if(elWire.getIsPulse()){
        elWire.setFreq(hrBPM);
        elWire.pulse(EL_PULSE_INFINITE);
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
      break;
    case STATE_TOO_FAST:
      
      deltaTime = currTime - tooFastStart;
      
      if(hasStateChanged()){
//        Serial.println("TOO FAST");
        pulseChance = 0;
        tooFastStart = currTime;
        pulseTotal += pulseAmnt[0];
        if(elWire.getIsPulse()){
          elWire.stopPulse();
        }
        cdState[1] = cdState[0];
        cdState[0] = CD_STATE_TOO_FAST;
        sendOSCMessage((oscBaseAddress + "/cd_state").c_str(), cdState[0]);
      }

      if(deltaTime % FAST_PULSE_CHANCE_INC < 100){
        pulseChance++;
      }

      if(elWire.getIsPulse()){
        elWire.pulse(EL_PULSE_ONCE);
      }
      break;
    case STATE_STOP:
      
      if(hasStateChanged()){
//        Serial.println("STOP INIT");
        stateStartTime = currTime;
        pulseTotal += pulseAmnt[0];
        
        if(elWire.getIsPulse()){
//          Serial.println("STOP PULSE");
          elWire.stopPulse();
        }

//        Serial.print("too fast time: ");
//        Serial.print(tooFastTime);
//        Serial.print(" | max fast time: ");
//        Serial.print(STOP_MAX_FAST_TIME);
//        Serial.print(" | crank time: ");
//        Serial.print(currTime - crankTime);
//        Serial.print(" | min crank time: ");
//        Serial.println(STOP_MIN_CRANK_TIME);

        if(tooFastTime < STOP_MAX_FAST_TIME && currTime - crankTime > STOP_MIN_CRANK_TIME){
//          Serial.println("GOOD STOP");
          cdState[1] = cdState[0];
          cdState[0] = CD_STATE_GOOD_STOP;
          sendOSCMessage((oscBaseAddress + "/cd_state").c_str(), cdState[0]);
        } else {
//          Serial.println("BAD STOP");
          cdState[1] = cdState[0];
          cdState[0] = CD_STATE_BAD_STOP;
          sendOSCMessage((oscBaseAddress + "/cd_state").c_str(), cdState[0]);
        }

//        sendOSCDataViz((oscBaseAddress + "/data_viz").c_str(), pulseTotal, currTime - crankTime, tooFastTime);
        pulseTotal = 0;
        tooFastTime = 0;
      }
      break;
  }
}

bool hasStateChanged(){
  return animState[0] != animState[1];
}

void setAnimState(unsigned long currTime) {
  animState[1] = animState[0];
//  Serial.print("anim states: ");
//  Serial.print(animState[0]);
//  Serial.print(" , ");
//  Serial.println(animState[1]);

  if (crankRPM == 0 && (animState[0] == STATE_HEART || animState[0] == STATE_TOO_FAST)) {
    hasStopped = true;
    animState[0] = STATE_STOP;
  }

  if (crankRPM > 0 && (animState[0] == STATE_WAIT || animState[0] == STATE_IDLE) && hasStopped == false) {
    crankTime = currTime;
  }

  if (crankRPM != prevCrankRPM && crankRPM == 0 && hasStopped == false) {
    idleTime = currTime;
  }

  if (CRANK_RPM_THRESH > crankRPM > 0 && currTime - crankTime < CRANK_TIME_THRESH && hasStopped == false) {
    animState[0] = STATE_START;
  }

  if (CRANK_RPM_THRESH > crankRPM > 0 && currTime - crankTime > CRANK_TIME_THRESH && hasStopped == false && crankRPM != 0) {
    animState[0] = STATE_HEART;
  }

  if (crankRPM > CRANK_RPM_THRESH && hasStopped == false) {
    animState[0] = STATE_TOO_FAST;
  }

  if (crankRPM == 0 && currTime - idleTime < CRANK_IDLE_TIME && hasStopped == false) {
    animState[0] = STATE_WAIT;
  }

  if (crankRPM == 0 && currTime - idleTime > CRANK_IDLE_TIME && hasStopped == false) {
    animState[0] = STATE_IDLE;
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

void sendOSCMessage(const char* address, int val) {
  //the message wants an OSC address as first argument
  OSCMessage msg(address);
  msg.add(val);

  if(LOG){
    Serial.print("send OSC: ");
    Serial.print(address);
    Serial.print(" | ");
    Serial.println(val);
  }

  Udp.beginPacket(outIp, NETWORK_OUT_PORT);
    msg.send(Udp); // send the bytes to the SLIP stream
  Udp.endPacket(); // mark the end of the OSC Packet
  msg.empty(); // free space occupied by message
}

void sendOSCDataViz(const char* address, int val1, int32_t val2, int32_t val3){
  //the message wants an OSC address as first argument
  OSCMessage msg(address);
  msg.add(val1);
  msg.add(val2);
  msg.add(val3);

  if(LOG){
    Serial.print("send OSC: ");
    Serial.print(address);
    Serial.print(" | ");
    Serial.println(val1);
    Serial.print(", ");
    Serial.println(val2);
    Serial.print(", ");
    Serial.println(val3);
  }
  
  Udp.beginPacket(outIp, NETWORK_OUT_PORT);
    msg.send(Udp); // send the bytes to the SLIP stream
  Udp.endPacket(); // mark the end of the OSC Packet
  msg.empty(); // free space occupied by message
}
