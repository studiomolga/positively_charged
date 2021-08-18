#include "constants.h"

class ElWire
{
  float brightness;
  float prevBrightness;
  int state;
  float pSpeed;
  float freq;
  float vibrationTime;
  unsigned long prevTime;
  bool isPulse;
  bool firstRun;
  bool secondRun;
  bool dir;
  int zeroCount;
  int prevZeroCount;
  int pulses;
  
  public:
  ElWire(){
    brightness = 0;
    prevBrightness = 0;
//    bpm = 1;
    state = STATE_WAIT;
    isPulse = false;
    firstRun = false;
    secondRun = false;
    freq = 0;
    pulses = 0;
    vibrationTime = 0;
    prevTime = 0;
    pSpeed = 0;
    dir = EL_FADE_DIR_UP;
    zeroCount = 0;
    prevZeroCount = -1;
    analogWrite(EL_PIN, brightness);
  }

  void stopPulse(){
    isPulse = false;
    unsigned long stopTime = millis();
    pSpeed = EL_MAX_BRIGHTNESS / (vibrationTime / 2.0f);
    
    while(brightness > 0.0){
      unsigned long deltaTime = millis() - stopTime;
      brightness -= (deltaTime * pSpeed);
      if(brightness < 0.0){
        brightness = 0.0;
      }
//      if(LOG){
//        Serial.print("EL brightness (stoppping): ");
//        Serial.println(brightness);
//      }
      analogWrite(EL_PIN, brightness);
//      delay(5);
    }
  }

  void triggerPulse(){
    isPulse = true;
    firstRun = true;
    secondRun = false;
    brightness = 0;
    zeroCount = 0;
    prevZeroCount = -1;
    pulses = 0;
    dir = EL_FADE_DIR_UP;
  }

  bool getIsPulse(){
    return isPulse;
  }

  void setState(int _state){
    state = _state;
  }

  void setFreq(int bpm){
    freq = bpm / 60.0f;
    vibrationTime = (1.0f / (bpm / 60.0f)) * 1000.0f;
  }

  float getBrightness(){
    return brightness;
  }

  int getPulses(){
    return pulses;
  }

  void pulse(int state){
    if(isPulse){
      unsigned long currTime = millis();
      
      if(firstRun){
        prevTime = currTime;
        firstRun = false;
        secondRun = true;
      }
  
      if(secondRun){
        unsigned long deltaTime = currTime - prevTime;
//        Serial.println(deltaTime);
        prevTime = currTime;

        pSpeed = EL_MAX_BRIGHTNESS / (vibrationTime / 2.0f);

        prevBrightness = brightness;
        
        if(dir == EL_FADE_DIR_UP){
          brightness += (deltaTime * pSpeed);
        }
  
        if(dir == EL_FADE_DIR_DOWN){
          brightness -= (deltaTime * pSpeed);
        }
  
        if(brightness >= EL_MAX_BRIGHTNESS){
          dir = EL_FADE_DIR_DOWN;
          brightness = EL_MAX_BRIGHTNESS;
        } else if(brightness <= 0.0){
          dir = EL_FADE_DIR_UP;
          brightness = 0.0;
        }

        if(brightness == 0.0 && brightness != prevBrightness){
          prevZeroCount = zeroCount;
          zeroCount++;

          if(zeroCount != prevZeroCount){
            pulses++;
          }
        }

        if(zeroCount == 1 && state == EL_PULSE_ONCE){
          isPulse = false;
          brightness = 0;
          prevBrightness = 0;
          firstRun = true;
          secondRun = false;
          zeroCount = 0;
          prevZeroCount = -1;
          pulses = 0;
        }
      }
      if(LOG){
        Serial.print("EL brightness: ");
        Serial.println(brightness);
      }
      
      analogWrite(EL_PIN, brightness);
    }
  }
};
