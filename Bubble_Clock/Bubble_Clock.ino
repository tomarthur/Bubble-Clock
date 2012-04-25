/*
  BubbleClock_0.8
  Tom Arthur
  tom.arthur@mac.com
  http://howtomworks.com
  https://github.com/tomarthur/Bubble-Clock
  
  makes use of 
  LiquidCrystal Library & example code
  Time.h Library & example code
  Blink M Library
  Arduino Cookbook example code

 
 */

#include <Time.h>
#include <LiquidCrystal.h>
#include <Wire.h>
#include <BlinkM_funcs.h>

#define blinkm_addr 0x09

const int alarmStop = 6; // button to stop alarm
const int btnBack = 7; // button to move time back
const int btnForward = 8; // button to move time forward
const int swtchSet = 9; // button to move time back
const int bubbleMotor = 10; // motor control transistor

const int numRows = 2; // number of rows on LCD
const int numCols = 16; // number of colums on LCD
int alarmH = 11; // alarm hour
int alarmM = 0; // alarm minute
int step;   // the number of seconds to move (backwards if negative)
int timeout = 0;
unsigned long prevtime; //when the cock was last displayed
boolean alarmActive = false; //alarm status
boolean alarmStopped = false; // status of stop button

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

void setup() {

 // enable internal pull-up resistors
 digitalWrite(btnForward, HIGH); 
 digitalWrite(btnBack, HIGH); 
 digitalWrite(swtchSet, HIGH); 
 digitalWrite(alarmStop, HIGH);
 
  BlinkM_beginWithPower();  
  BlinkM_stopScript( blinkm_addr );  // turn off startup script 
  BlinkM_setRGB(blinkm_addr, 0x00, 0x00, 0x00 );
 
 //start with the time
 setTime(10,57,0,1,1,12); 
 
 // set up the LCD's number of columns and rows: 
  lcd.begin(numCols, numRows);
  
 // Print a message to the LCD.
  lcd.print("bubble clock 0.6 initialization");
  delay (2000);
  
 // set up formatting
  lcd.clear();
  lcd.print("Now"); 
  lcd.setCursor(0,1); 
  lcd.print("Alarm");
  
  //setup bubble motor control
   pinMode(bubbleMotor, OUTPUT);  
   
  //setup LED magic

 
}

void loop()
{
  prevtime = now();   // note the time
  while( prevtime == now() )    // stay in this loop till the second changes
  {
      // check if the set button pressed while waiting for second to roll over
     if(checkSetTime()) {
         prevtime = now();   //  time changed so reset start time
  }
  }

  digitalClockDisplay(); // displays current time
  digitalAlarmDisplay(); // displays alarm time
  alarmCheck();
}


// functions check to see if the current time or alarm time should 
// be adjusted, returns if time was changed
boolean checkSetTime()
{

boolean isTimeAdjusted = false;  // set to true if the time is adjusted
boolean isAlarmAdjusted = false; // set to true if the alarm is adjusted
if (digitalRead(swtchSet) == LOW)
{
  step = 1;   // ready to step forwards
  while(digitalRead(btnForward)== LOW)
  {
     adjustTime(step);
     isTimeAdjusted = true; // to tell the user that the time has changed
     step = step + 1; // next step will be bigger
     digitalClockDisplay(); // update clock   
     delay(100); 
  }
  step = -1;   // negative numbers step backwards
  while(digitalRead(btnBack)== LOW)
  {
     adjustTime(step);
     isTimeAdjusted = true; // to tell the user that the time has changed
     step = step - 1; // next step will be a bigger negative number
     digitalClockDisplay(); // update clock   
     delay(100); 
  }
}
else {
    step = 1;  
    while(digitalRead(btnForward)== LOW)
    {
      isAlarmAdjusted= true;
      step = step + .5;
      alarmTimeCheck();
      digitalAlarmDisplay();
    }
    step = -1;
    while(digitalRead(btnBack)== LOW)
    {
      isAlarmAdjusted = true;
      step = step - .5;
      alarmTimeCheck();
      digitalAlarmDisplay();
    }
    
}

  return isTimeAdjusted;  // tell the user if the time was adjusted
  return isAlarmAdjusted; // tell the user if the alarm was adjusted
}


// keeps alarm setting in check
void alarmTimeCheck(){
  if (alarmM >= 0 || alarmM <= 59) {
    alarmM= alarmM + step;
  }
  if (alarmM < 0) {
    alarmH= alarmH - 1;
    alarmM = 59;
  }
  if (alarmM > 59) {
    alarmM = 0;
    alarmH = alarmH + 1;
  }
  if (alarmH > 24) {
    alarmH = 0;
    alarmM = 0;
  }
  if (alarmH < 0) {
    alarmH = 23;
    alarmM = 59;
  }
}


void digitalClockDisplay(){   //digital clock display of time via LCD
 
  lcd.setCursor(4,0);
  lcd.print(hour());
  printDigitsLCD(minute());
  printDigitsLCD(second());
  lcd.print(" "); 
}

void digitalAlarmDisplay(){   //digital alarm display on LCD
 
  lcd.setCursor(6,2);
  lcd.print(alarmH);
  printAlmDigitsLCD();
  lcd.print(" ");
}

void printDigitsLCD(int digits){ //digital clock display on LCD
  lcd.print(":");
  if(digits < 10)
  {
    lcd.print('0');
  }
  lcd.print(digits);
}

void printAlmDigitsLCD(){ //digital alarm display on LCD
  lcd.print(":");
  if(alarmM < 10)
  {
    lcd.print('0');
  }
  lcd.print(alarmM);
}

void alarmCheck() {
  if (hour() == alarmH && minute() == alarmM)
  {
    alarmActive = true;
    if (alarmStopped == false)
    {
      MainAlarm ();
    }
  }
  else {
    alarmActive = false;
    if (hour() == alarmH && minute() > (alarmM + 5))
      {
        alarmStopped = false;
      }
  }
  
}

void MainAlarm () {
 while (alarmActive == true) {
   if (timeout > 4000){ // shuts off alarm with button
    alarmStopped = true;
    alarmReset ();
    break;
    }
  else if (digitalRead(alarmStop)== LOW){ // shuts alarm off if we forget to
    alarmStopped = true;
    alarmReset ();
    break;
    } 
   else if (alarmActive == false)
   {
     alarmStopped = true;
     alarmReset ();
     break
   }
   digitalWrite(bubbleMotor, HIGH); // activate bubble maker
   startColors ();
   alarmLCDprint ();
   delay (100);
   timeout ++; 
 } 
}

// prints alarm notification on LCD
void alarmLCDprint (){
  if (timeout == 0) {
  lcd.clear();;
  lcd.print("Wake Up!");
  lcd.setCursor(0,1);
  lcd.print("I'm all wet");
  }
}

// clears display, resets blinkM & shuts bubbles off
void alarmReset () {
  digitalWrite(bubbleMotor, LOW);
  lcd.clear();;
  lcd.print("Sorry 'bout it");
  lcd.setCursor(0,1);
  lcd.print("Good morning!");
  delay(2000);
  lcd.clear();
  lcd.print("Now"); 
  lcd.setCursor(0,1);
  lcd.print("Alarm");
  timeout = 0;
  BlinkM_stopScript( blinkm_addr );
  BlinkM_fadeToRGB(blinkm_addr, 0x00, 0x00, 0x00);
}

void startColors () {
  if (timeout == 0) {
  BlinkM_playScript(blinkm_addr, 0x11, 0x00, 0x00);
  }
}
