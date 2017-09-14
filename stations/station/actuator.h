#include "constants.h"

class Actuator
{
  int state;
  int prevState;
  int pulseCnt;
  unsigned long startTime;
  unsigned long moveTime;
  unsigned long pulseStartTime;
  bool isMoving;
  bool firstRun;
  bool secondRun;
  float freq;
  float vibrationTime;

  public:
  Actuator(){
    pinMode(ACTUATOR_ENB_PIN, OUTPUT);
    pinMode(ACTUATOR_IN3_PIN, OUTPUT);
    pinMode(ACTUATOR_IN4_PIN, OUTPUT);

    state = LA_MOVE_NONE;
    prevState = LA_MOVE_NONE;
    startTime = 0;
    moveTime = 0;
    isMoving = false;
    freq = 0;
    vibrationTime = 0;
    firstRun = true;
    secondRun = false;
    pulseCnt = 0;
  }

  void init(){
//    Serial.println("init");
    backward(LA_MAX_MOVE_TIME);
    delay(LA_MAX_MOVE_TIME);
  }

  void stopPulse(){
//    Serial.println("stop");
    backward(LA_INIT_MOVE_TIME);
    delay(LA_INIT_MOVE_TIME);
  }

  void triggerPulse(){
//    Serial.println("starting");
    firstRun = true;
    secondRun = false;
  }

  void pulse(){
    if(firstRun){
      pulseStartTime = millis();
      firstRun = false;
      secondRun = true;
      pulseCnt = 0;
      // we put some pulse count var to 0
    }

    if(secondRun){
      unsigned long deltaTime = millis() - pulseStartTime;

      if(15 > deltaTime > 0 && pulseCnt % LA_PULSE_STEP == 0){   //we only execute this when pulse count % pulse step = 0
        forward(LA_MOVE_TIME);
      }

      if(/*deltaTime > (vibrationTime - (LA_MOVE_TIME + 25))*/ 15 > deltaTime > 0 && pulseCnt % LA_PULSE_STEP == LA_PULSE_STEP / 2){   //we only execute this when pulse count % pulse step = pulse step - 1
        backward(LA_MOVE_TIME + 25);
      }
      
      if(deltaTime >= vibrationTime){
        pulseStartTime = millis();
        pulseCnt++;
        // every time we enter here we do pulse count + 1
      }
    }
  }

  void updateAct(){
    if(state == LA_MOVE_FORWARD || state == LA_MOVE_BACKWARD){
      unsigned long currTime = millis();
      if(currTime - startTime > moveTime && isMoving == true){
        stopAct();
      }
    }
  }

  void forward(unsigned long ttime){
    if(state != LA_MOVE_FORWARD){
      if(LOG){
        Serial.println("forward");
      }
      startTime = millis();
      moveTime = ttime;
      isMoving = true;
      analogWrite(ACTUATOR_ENB_PIN, 0);
      digitalWrite(ACTUATOR_IN3_PIN, LOW);
      digitalWrite(ACTUATOR_IN4_PIN, HIGH);
      analogWrite(ACTUATOR_ENB_PIN, LA_SPEED);
//      prevState = state;
      state = LA_MOVE_FORWARD;
    }
  }

  void backward(unsigned long ttime){
    if(state != LA_MOVE_BACKWARD){
      if(LOG){
        Serial.println("backward");
      }
      startTime = millis();
      moveTime = ttime;
      isMoving = true;
      analogWrite(ACTUATOR_ENB_PIN, 0);
      digitalWrite(ACTUATOR_IN3_PIN, HIGH);
      digitalWrite(ACTUATOR_IN4_PIN, LOW);
      analogWrite(ACTUATOR_ENB_PIN, LA_SPEED);   
//      prevState = state;
      state = LA_MOVE_BACKWARD;    
    }
  }

  void stopAct(){
    if(state != LA_MOVE_STOP){
      if(LOG){
        Serial.println("stop");
      }
      isMoving = false;
      analogWrite(ACTUATOR_ENB_PIN, 0);
      digitalWrite(ACTUATOR_IN3_PIN, LOW);
      digitalWrite(ACTUATOR_IN4_PIN, LOW);
      analogWrite(ACTUATOR_ENB_PIN, LA_SPEED);    
//      prevState = state;
      state = LA_MOVE_STOP; 
    }
  }

  void setFreq(int bpm){
    freq = bpm / 60.0f;
    vibrationTime = (1.0f / (bpm / 60.0f)) * 1000.0f;
    if(LOG){
      Serial.println("vibration time: ");
      Serial.println(vibrationTime);
    }
  }
};

