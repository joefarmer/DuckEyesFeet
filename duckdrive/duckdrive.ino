#define CHANNEL_1_PIN   0 //pin 2
#define CHANNEL_2_PIN   1 //pin 3

#define MOTOR_1A_PIN    5
#define MOTOR_1B_PIN    9
#define MOTOR_2A_PIN    6
#define MOTOR_2B_PIN   10

//micros when the pin goes HIGH
volatile unsigned long timer_start1;
volatile unsigned long timer_start2;//difference between timer_start and micros() is the length of time that the pin 
//was HIGH - the PWM pulse length. 
volatile int pulse_time1 = 1500; 
volatile int pulse_time2 = 1500;

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
 if(digitalRead(2) == HIGH)
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
  }
}

void calcSignal2()
{
//record the interrupt time so that we can tell if the receiver has a signal from the transmitter
 last_interrupt_time2 = micros();

//if the pin has gone HIGH, record the microseconds since the Arduino started up
 if(digitalRead(3) == HIGH)
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
    }
}

void setup() {
  pinMode(MOTOR_1A_PIN, OUTPUT);
  pinMode(MOTOR_1B_PIN, OUTPUT);
  pinMode(MOTOR_2A_PIN, OUTPUT);
  pinMode(MOTOR_2B_PIN, OUTPUT);
  
  timer_start1 = 0;
  attachInterrupt(CHANNEL_1_PIN, calcSignal1, CHANGE);
  timer_start2 = 0;
  attachInterrupt(CHANNEL_2_PIN, calcSignal2, CHANGE);
}

void loop() {
  //code for brushed ESC
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
