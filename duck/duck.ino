#include <Servo.h>
#include <NewPing.h>

#define SONAR_NUM     2 // Number of sensors.
#define MAX_DISTANCE 255 // Maximum distance (in cm) to ping.
#define PING_INTERVAL 100 // Milliseconds between sensor pings (29ms is about the min to avoid cross-sensor echo).

#define CHANNEL_1_PIN   0 //pin 2
#define CHANNEL_2_PIN   1 //pin 3

#define SONAR_1_TRIG    4
#define SONAR_1_ECHO    7
#define SONAR_2_TRIG    8
#define SONAR_2_ECHO   12

#define MOTOR_1A_PIN    5
#define MOTOR_1B_PIN    9
#define MOTOR_2A_PIN    6
#define MOTOR_2B_PIN   10

#define SONAR_SELECT_PIN  11

unsigned long pingTimer[SONAR_NUM]; // Holds the times when the next ping should happen for each sensor.
unsigned int cm[SONAR_NUM];         // Where the ping distances are stored.
uint8_t currentSensor = 0;          // Keeps track of which sensor is active.

NewPing sonar[SONAR_NUM] = {     // Sensor object array.
  NewPing(SONAR_1_TRIG, SONAR_1_ECHO, MAX_DISTANCE), // Each sensor's trigger pin, echo pin, and max distance to ping.
  NewPing(SONAR_2_TRIG, SONAR_2_ECHO, MAX_DISTANCE)
};

//micros when the pin goes HIGH
volatile unsigned long timer_start1;
volatile unsigned long timer_start2;//difference between timer_start and micros() is the length of time that the pin 
//was HIGH - the PWM pulse length. 
volatile int pulse_time1; 
volatile int pulse_time2;

//this is the time that the last interrupt occurred. 
//you can use this to determine if your receiver has a signal or not.
volatile int last_interrupt_time1;
volatile int last_interrupt_time2;

//calcSignal is the interrupt handler
void calcSignal1()
{
//record the interrupt time so that we can tell if the receiver has a signal from the transmitter
 last_interrupt_time1 = micros();

//if the pin has gone HIGH, record the microseconds since the Arduino started up
 if(digitalRead(CHANNEL_1_PIN) == HIGH)
    {
        timer_start1 = micros();
    }
//otherwise, the pin has gone LOW
    else
    {
        //only worry about this if the timer has actually started
        if(timer_start1 > 0)
        {
            //record the pulse time
            pulse_time1 = ((volatile int)micros() - timer_start1);
            //restart the timer
            timer_start1 = 0;
        }
        /* else
        {
          //failsafe
          pulse_time2 = 1500;
        }
        */
    }
}

void calcSignal2()
{
//record the interrupt time so that we can tell if the receiver has a signal from the transmitter
 last_interrupt_time2 = micros();

//if the pin has gone HIGH, record the microseconds since the Arduino started up
 if(digitalRead(CHANNEL_2_PIN) == HIGH)
    {
        timer_start2 = micros();
    }
//otherwise, the pin has gone LOW
    else
    {
        //only worry about this if the timer has actually started
        if(timer_start2 > 0)
        {
            //record the pulse time
            pulse_time2 = ((volatile int)micros() - timer_start2);
            //restart the timer
            timer_start2 = 0;
        }
        /* else
        {
          //failsafe
          pulse_time2 = 1500;
        }
        */
    }
}

void setup() {
  pingTimer[0] = millis() + 75;           // First ping starts at 75ms, gives time for the Arduino to chill before starting.
  for (uint8_t i = 1; i < SONAR_NUM; i++) {// Set the starting time for each sensor.
    pingTimer[i] = pingTimer[i - 1] + PING_INTERVAL;
  }
  
  timer_start1 = 0;
  attachInterrupt(CHANNEL_1_PIN, calcSignal1, CHANGE);
  timer_start2 = 0;
  attachInterrupt(CHANNEL_2_PIN, calcSignal2, CHANGE);
  //Serial.begin(115200);
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
      cm[currentSensor] = 0;                      // Make distance zero in case there's no ping echo for this sensor.
      sonar[currentSensor].ping_timer(echoCheck); // Do the ping (processing continues, interrupt will call echoCheck to look for echo).
    }
  }
  
  //more code for brushed ESC
  if( pulse_time1 <= 1480) {
    analogWrite(MOTOR_1A_PIN, constrain((1480 - pulse_time1) / 2, 0, 255)); //EX 1480-1400= 80/2 = 40/255 %DC = 15 %DC 
    analogWrite(MOTOR_1B_PIN, 0); //never run both directions on the same wheel at once. 
  }
  else if(pulse_time1 >= 1520) {
    analogWrite(MOTOR_1B_PIN, constrain(abs(1520 - pulse_time1) / 2, 0, 255)); //EX abs(1520 - 1600) = 80/2 = 40/255 %DC = 15 %DC 
    analogWrite(MOTOR_1A_PIN, 0); //never run both directions on the same wheel at once. 
  }
  else {
    analogWrite(MOTOR_1A_PIN, 0);  
    analogWrite(MOTOR_1B_PIN, 0);  
  }
  
  if( pulse_time2 <= 1480) {
    analogWrite(MOTOR_2A_PIN, constrain((1480 - pulse_time2) / 2, 0, 255)); //EX 1480-1400= 80/2 = 40/255 %DC = 15 %DC 
    analogWrite(MOTOR_2B_PIN, 0); //never run both directions on the same wheel at once. 
  }
  else if(pulse_time2 >= 1520) {
    analogWrite(MOTOR_2B_PIN, constrain(abs(1520 - pulse_time2) / 2, 0, 255)); //EX abs(1520 - 1600) = 80/2 = 40/255 %DC = 15 %DC 
    analogWrite(MOTOR_2A_PIN, 0); //never run both directions on the same wheel at once. 
  }
  else {
    analogWrite(MOTOR_2A_PIN, 0);  
    analogWrite(MOTOR_2B_PIN, 0);  
  }
   
}

void echoCheck() { // If ping received, set the sensor distance to array.
  if (sonar[currentSensor].check_timer()) {
    cm[currentSensor] = sonar[currentSensor].ping_result / US_ROUNDTRIP_CM;
  }
}

void oneSensorCycle() { // Sensor ping cycle complete, do something with the results.
    analogWrite(SONAR_SELECT_PIN, cm[digitalRead(13) == HIGH]);
}

