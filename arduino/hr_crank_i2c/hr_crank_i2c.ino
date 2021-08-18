#include <Wire.h>
#define USE_ARDUINO_INTERRUPTS true    // Set-up low-level interrupts for most acurate BPM math.
#include <PulseSensorPlayground.h>     // Includes the PulseSensorPlayground Library.   
#define REED_PIN 8
#define MIN_CRANK_TIME 200
#define CRANK_RESET_TIME 4000

int x = 0;

//  Variables
const int PulseWire = 0;       // PulseSensor PURPLE WIRE connected to ANALOG PIN 0
const int LED13 = 13;          // The on-board Arduino LED, close to PIN 13.
int Threshold = 550;           // Determine which Signal to "count as a beat" and which to ignore.
                               // Use the "Gettting Started Project" to fine-tune Threshold Value beyond default setting.
                               // Otherwise leave the default "550" value.

unsigned long prevTime = 0;
float RPM = 0;
float prevRPM = 0;

PulseSensorPlayground pulseSensor;  // Creates an instance of the PulseSensorPlayground object called "pulseSensor"

void setup()
{
  Wire.begin();
//  Serial.begin(9600);

  pinMode(REED_PIN, INPUT);

  // Configure the PulseSensor object, by assigning our variables to it.
  pulseSensor.analogInput(PulseWire);
//  pulseSensor.blinkOnPulse(LED13);       //auto-magically blink Arduino's LED with heartbeat.
  pulseSensor.setThreshold(Threshold);

  // Double-check the "pulseSensor" object was created and "began" seeing a signal.
  if (pulseSensor.begin()) {
//    Serial.println("We created a pulseSensor Object !");  //This prints one time at Arduino power-up,  or on Arduino reset.
  }
}

void loop()
{
  int val = digitalRead(REED_PIN);
  unsigned long currTime = millis();
  prevRPM = RPM;

  if(val == 1 && currTime - prevTime > MIN_CRANK_TIME){
    //calculate the delta time in minutes so we can calculate the rounds per minute
    float deltaTime = ((currTime - prevTime) / 1000.0f) / 60.0f;
    RPM = 1.0f / deltaTime;
    prevTime = currTime;
  }
  
  if(val == 0 && currTime - prevTime > CRANK_RESET_TIME){
    RPM = 0;
  }

  int BPM = pulseSensor.getBeatsPerMinute();  // Calls function on our pulseSensor object that returns BPM as an "int".
                                                // "myBPM" hold this BPM value now.
  if(prevRPM != RPM){
//    Serial.print("RPM: ");                        // Print phrase "BPM: "
//    Serial.print(RPM);
//    Serial.print(" | BPM: ");                        // Print phrase "BPM: "
//    Serial.println(BPM);                        // Print the value inside of myBPM.

    if(RPM > 255.0f) RPM = 255.0f;
    if(BPM > 255.0f) BPM = 255.0f;

    byte buf[2] = {floor(RPM), floor(BPM)};

    Wire.beginTransmission(9);
    Wire.write(buf, 2);
    Wire.endTransmission();
  }

  delay(2);                    // considered best practice in a simple sketch.
}
