#include "constants.h"
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

class Crank
{
  unsigned long prevTime;
  float RPM;
  int brakePos;
  int peltierState;
//  int peltierVal;
  Adafruit_PWMServoDriver pwm;
  
  public:
  Crank(){
    prevTime = 0;
    RPM = 0;
    peltierState = PELTIER_STATE_NONE;
    brakePos = MIN_BRAKE_POS;
//    peltierVal = MIN_PELTIER_VAL;
    pwm = Adafruit_PWMServoDriver();

    pinMode(REED_PIN, INPUT);
  }

  void startCrank(){
    pwm.begin();
    pwm.setPWMFreq(60);
    pwm.setPWM(SERVO_ADRES, 0, brakePos);
    for(int i = 0; i < PELTIER_AMOUNT; i++){
      pwm.setPWM(PELTIER_ADDRESS_START + i, 0, 0);
    }
  }

//  void setPeltierVal(int val){
//    peltierVal = val;
//    pwm.setPWM(PELTIER_ADDRESS_START, 0, peltierVal);
//  }
//
//  int getPeltierVal(){
//    return peltierVal;
//  }

  void setBrakePos(int pos){
    brakePos = pos;
    pwm.setPWM(SERVO_ADRES, 0, brakePos);
  }

  int getBrakePos(){
    return brakePos;
  }
  
  float getRPM(){
    return RPM;
  }

  void updateCrank(){
    int val = digitalRead(REED_PIN);
    int peltierVal = 0;
    unsigned long currTime = millis();

    if(val == 1 && currTime - prevTime > MIN_CRANK_TIME){
      //calculate the delta time in minutes so we can calculate the rounds per minute
      float deltaTime = ((currTime - prevTime) / 1000.0f) / 60.0f;
      RPM = 1.0f / deltaTime;
      prevTime = currTime;
    }
    
    if(val == 0 && currTime - prevTime > CRANK_RESET_TIME){
      RPM = 0;
    }

    if(PELTIER_RPM_THRESH > RPM > 0){
      peltierState = PELTIER_STATE_COLD;
    }

    if(RPM >= PELTIER_RPM_THRESH){
      peltierState = PELTIER_STATE_HOT;
    }

    if(RPM == 0){
      peltierState = PELTIER_STATE_NONE;
    }

    if(peltierState != PELTIER_STATE_NONE){
      float peltierRPM = RPM;
      if(peltierRPM > MAX_CRANK_RPM) peltierRPM == MAX_CRANK_RPM;
      
//      peltierVal = (peltierRPM / MAX_CRANK_RPM) * MAX_PELTIER_VAL;
//      float invPeltierVal = abs(peltierVal - MAX_PELTIER_VAL);

      if(peltierState == PELTIER_STATE_HOT){
        peltierVal = ((peltierRPM - PELTIER_RPM_THRESH) / (MAX_CRANK_RPM - PELTIER_RPM_THRESH)) * MAX_PELTIER_VAL;
        peltierVal *= 3;
        if(peltierVal > 4095){
          peltierVal = 4095;
        }
        float invPeltierVal = abs(peltierVal - MAX_PELTIER_VAL);
//        Serial.println(peltierVal);
        for(int i = 0; i < PELTIER_AMOUNT / 2; i++){
//          Serial.println(peltierVal);
          pwm.setPWM(PELTIER_ADDRESS_START + i, 0, 4095);
//          Serial.println(i);
        }

        for(int i = PELTIER_AMOUNT / 2; i < PELTIER_AMOUNT; i++){
//          pwm.setPWM(PELTIER_ADDRESS_START + i, 0, 0);
          pwm.setPWM(PELTIER_ADDRESS_START + i, 0, 0);
        }
      }

      if(peltierState == PELTIER_STATE_COLD){
        peltierVal = (peltierRPM / PELTIER_RPM_THRESH) * MAX_PELTIER_VAL;
        peltierVal *= 3;
        if(peltierVal > 4095){
          peltierVal = 4095;
        }
//        Serial.println(peltierVal);
        for(int i = 0; i < PELTIER_AMOUNT / 2; i++){
          pwm.setPWM(PELTIER_ADDRESS_START + i, 0, 0);
        }

        for(int i = PELTIER_AMOUNT / 2; i < PELTIER_AMOUNT; i++){
          pwm.setPWM(PELTIER_ADDRESS_START + i, 0, 4095);
        }

//        Serial.println("---------------");
      }
      if(LOG){
        Serial.print("rpm = ");
        Serial.print(RPM);
        Serial.print(" | state = ");
        Serial.print(peltierState);
        Serial.print(" | value =");
        Serial.println(peltierVal);  
      }
    } else {
//      Serial.println("to zero");
      for(int i = 0; i < PELTIER_AMOUNT; i++){
        pwm.setPWM(PELTIER_ADDRESS_START + i, 0, 0);
      }
    }
    
  }
};
