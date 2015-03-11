#include <NewPing.h>

#define SONAR_NUM     2 // Number of sensors.
#define MAX_DISTANCE 254 // Maximum distance (in cm) to ping.
#define PING_INTERVAL 80 // Milliseconds between sensor pings (29ms is about the min to avoid cross-sensor echo).

#define SONAR_1_TRIG    4
#define SONAR_1_ECHO    7
#define SONAR_2_TRIG    8
#define SONAR_2_ECHO   12

#define SONAR_OUT_PIN    10
#define SONAR_SELECT_PIN  5 

unsigned long pingTimer[SONAR_NUM]; // Holds the times when the next ping should happen for each sensor.
unsigned int cm[SONAR_NUM];         // Where the ping distances are stored.
uint8_t currentSensor = 0;          // Keeps track of which sensor is active.

NewPing sonar[SONAR_NUM] = {     // Sensor object array.
  NewPing(SONAR_1_TRIG, SONAR_1_ECHO, MAX_DISTANCE), // Each sensor's trigger pin, echo pin, and max distance to ping.
  NewPing(SONAR_2_TRIG, SONAR_2_ECHO, MAX_DISTANCE)
};

void setup() {
  pingTimer[0] = millis() + 75;           // First ping starts at 75ms, gives time for the Arduino to chill before starting.
  for (uint8_t i = 1; i < SONAR_NUM; i++) {// Set the starting time for each sensor.
    pingTimer[i] = pingTimer[i - 1] + PING_INTERVAL;
  }
  
  pinMode(SONAR_OUT_PIN, OUTPUT);  
  
  //Serial.begin(57600);
}

void loop() {  
  for (uint8_t i = 0; i < SONAR_NUM; i++) { // Loop through all the sensors.
    if (millis() >= pingTimer[i]) {         // Is it this sensor's time to ping?
      pingTimer[i] += PING_INTERVAL * SONAR_NUM;  // Set next time this sensor will be pinged.
      if (i == 0 && currentSensor == SONAR_NUM - 1) {
        oneSensorCycle(); // Sensor ping cycle complete, do something with the results.
      }
      sonar[currentSensor].timer_stop();          // Make sure previous timer is canceled before starting a new ping (insurance).
      currentSensor = i;                          // Sensor being accessed.
      cm[currentSensor] = 255;                      // Make distance zero in case there's no ping echo for this sensor.
      sonar[currentSensor].ping_timer(echoCheck); // Do the ping (processing continues, interrupt will call echoCheck to look for echo).
    }
  }
}

void echoCheck() { // If ping received, set the sensor distance to array.
  if (sonar[currentSensor].check_timer()) {
    cm[currentSensor] = sonar[currentSensor].ping_result / US_ROUNDTRIP_CM;
  }
}

void oneSensorCycle() { // Sensor ping cycle complete, do something with the results.
    analogWrite(SONAR_OUT_PIN, 255 - cm[digitalRead(SONAR_SELECT_PIN) == HIGH]);
    //Serial.println(cm[digitalRead(SONAR_SELECT_PIN) == HIGH]);
}

