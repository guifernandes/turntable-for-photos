// ########## Includes ##########
#include <Arduino.h>

// ########## Pins definition ##########
// DIR PINT PB5
// STEP PIN PB4
// BUTTON PIN PE6
// SHUTTER PIN PD7
// FOCUS PIN PC6
// ENB PIN PD1
// LED PIN PD4

// ########## Defines ##########
const int ShutterDelay = 1500; //define the time by pressing the shutter 
const int FocusDelay = 1500;   //define the time by pressing the shutter 
const int StepPeriod = 1000;   //arbitrary time HAHAHA
const int Steps = 128;         //Stepper config steps 200*16=3200, 
const int PhotoTime = 5000;    //Sets the time the camera takes to take the photo
const int NumPhotos = 24;      //Define the number of photos to take
const int WaitForInit = 2000;  //Waiting time to init process

volatile int state = false;    // state

void ToggleInt0()
{
  state = !state;
}

void LED_ON() // set led on
{
  PORTD &= (0<<4); // Set low to PD4 
}
void LED_OFF() // set led off
{
  PORTD |= (1<<4); // Set high to PB4 
}

void BLINK_LED(int x) //blink led function
{
  for (int i = 0; i <= x; i++)
  {
    LED_ON();   //set led on
    delay(500); //wait 500 ms
    LED_OFF();  //set led off
    delay(500); //wait 500 ms
  }
}

void enable_driver()
{
  DDRD |= (1<<DDD1); //OUTPUT MODE, Set low to PD1, enable stepper driver
  delay (10);         // delay 10 miliseconds
}
void disable_driver()
{
  DDRD &= ~(1<<DDD1); //HIGH Z to PD1, disable stepper driver
  delay (10);        // delay 10 miliseconds
}

void shutter() //T6I shutter open drain PD7
{
  LED_OFF();
  DDRD |= (1<<DDD7); //set PD7 to output mode (sink)
  delay (ShutterDelay);
  DDRD &= ~(1<<DDD7); //set PD7 to high-z mode
  delay(PhotoTime); // wait camera take a photo 
  LED_ON();  
}

void focus() //T6I focus open drain PC6
{
  DDRC |= (1<<DDC6); //set PC6 to output mode (sink)
  delay (FocusDelay);
  DDRC &= ~(1<<DDC6); //set PC6 to high-z mode
}

void rotate_angle()
{
  PORTB |= (1<<5);       // Set high to PB5 DIR PIN (counter clockwise)
  for (int i = 0; i <= Steps; i++)
  {
    PORTB |= (1<<4); // Set high to PB4 STEP PIN 
    delayMicroseconds (StepPeriod);  
    PORTB &= (0<<4); // Set low to PB4 STEP PIN 
    delayMicroseconds (StepPeriod);   
  }
}

void takeXphotos(int x)
{
  enable_driver(); // enable stepper drive
  delay(500); // wait 500 ms to stabilization 
  shutter(); // take a first photo  
  for (int i = 0; i <= x; i++) //take defined number of photos
  {
    if(state == false){ //if button pressed interrupt process
      disable_driver(); // disable stepper driver
      return; // exit
    }
    rotate_angle(); // rotate table to next photo
    shutter(); // take a photo   
  }
  delay(500); //wait 500 ms
  disable_driver(); // disable stepper driver
}

void setup() {
  attachInterrupt(digitalPinToInterrupt(7), ToggleInt0, FALLING); //Attach interrup to pin PE6 (7)
  DDRB |= (1<<DDB4); //SET PB4 TO OUTPUT MODE //pinMode(STP, OUTPUT);
  DDRB |= (1<<DDB5); //SET PB5 TO OUTPUT MODE //pinMode(DIR, OUTPUT);
  DDRD |= (1<<DDD4); //SET PD0 TO OUTPUT MODE //pinMode(LED, OUTPUT);
  LED_OFF();
  state = false;
}

void loop() {  
  if(state != false) // if no false state, run logic
  {
    LED_ON(); 
    delay(WaitForInit); //wait time for init rotate
    takeXphotos(NumPhotos); //call function to take photos
    state = false; // when finished set state to false to stop process
    BLINK_LED(5); //blink led to indicate the end of the process
  }
}
