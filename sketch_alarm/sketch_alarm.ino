#include <DS3231.h>
#include <Wire.h>
#include <LiquidCrystal.h>
#include "pitches.h"

// headers 
int print_time();
void set_mode();
void update_time();
int check_time();
void alarm_play();
void cancelAlarmISR();

// Flags
bool play_alarm = false;

// Global variables
byte current_time[7]; // MM.YY.DD.w.HH.MM.SS
byte set_alarm[4] = {21, 29, 0, 1}; // HH.MM.SS.W?
DS3231 myRTC;

// Pins
const byte ledPin = 13;
const byte buttonPin = 18; // interrupt pin
const byte piezoPin = 7;
const int rs = 12, en = 11, d4 = 2, d5 = 3, d6 = 4, d7 = 5;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Constants for DS3231
bool century; // needed for overflowing
bool H12; // needs to be 0
bool PM_TIME; // needs to be 0



void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Wire.begin();
  lcd.begin(16,2);

  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(buttonPin), cancelAlarmISR, FALLING); 
}



void loop() {
  // put your main code here, to run repeatedly:
  update_time();
  print_time();
  if(check_time()) alarm_play();
  delay(1000);
}


void set_mode() {
  myRTC.setClockMode(false);  // set to 24h
  /*myRTC.setYear(year);
  myRTC.setMonth(month);
  myRTC.setDate(date);
  myRTC.setDoW(dOW);
  myRTC.setHour(hour);
  myRTC.setMinute(minute);
  myRTC.setSecond(second);
  */
  // come back
}

void update_time() {
  current_time[0] = myRTC.getYear();
  current_time[1] = myRTC.getMonth(century);
  current_time[2] = myRTC.getDate();
  current_time[3] = myRTC.getDoW();
  current_time[4] = myRTC.getHour(H12, PM_TIME);
  current_time[5] = myRTC.getMinute();
  current_time[6] = myRTC.getSecond();
}

int check_time() {
  // return 1 if alarm matches
  for (int i = 0; i < 3; i++) {
    if (current_time[i + 4] != set_alarm[i] ) return 0; 
  }

  return 1;
}

void alarm_play() {
  Serial.println(
    "Alarm!!!");
  play_alarm = true;
  while (play_alarm) {
    for (int thisNote = 0; thisNote < 8; thisNote++) {
      // to calculate the note duration, take one second divided by the note type.
      //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
      int noteDuration = 1000 / noteDurations[thisNote];
      tone(piezoPin, melody[thisNote], noteDuration);

      // to distinguish the notes, set a minimum time between them.
      // the note's duration + 30% seems to work well:

      int pauseBetweenNotes = noteDuration * 1.30;
      delay(pauseBetweenNotes);
      // stop the tone playing:
      noTone(piezoPin);

      update_time();
      print_time();
    }
  }
  
}

int print_time() {
  char buff[3];
  lcd.setCursor(0, 0);
  sprintf(buff, "%02d", current_time[2]);
  lcd.print(buff);
  Serial.print(buff);
  
  lcd.setCursor(2, 0);

  lcd.print(".");
  Serial.print(".");

  lcd.setCursor(3, 0);
  sprintf(buff, "%02d", current_time[1]);
  lcd.print(buff);
  Serial.print(buff);

  lcd.setCursor(5, 0);
  lcd.print(".");
  Serial.print(".");
  
  lcd.setCursor(6, 0);
  sprintf(buff, "%02d", current_time[0]);
  lcd.print(buff);
  Serial.print(buff);

  lcd.setCursor(8,0);
  lcd.print(" ");
  Serial.print(" ");

  lcd.setCursor(11,0);
  char* days[7] = {"MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN"};
  sprintf(buff, "%02d", current_time[3]);
  lcd.print(days[current_time[3]]);
  Serial.print(buff);

  Serial.print(" ");

  lcd.setCursor(0,1);
  sprintf(buff, "%02d", current_time[4]);
  lcd.print(buff);
  Serial.print(buff);

  lcd.setCursor(2,1);
  lcd.print(":");
  Serial.print(":");

  lcd.setCursor(3,1);
  sprintf(buff, "%02d", current_time[5]);
  lcd.print(buff);
  Serial.print(buff);

  lcd.setCursor(5,1);
  lcd.print(":");
  Serial.print(":");

  lcd.setCursor(6,1);
  sprintf(buff, "%02d", current_time[6]);
  lcd.print(buff);
  Serial.println(buff);

  lcd.setCursor(9,1);
  lcd.print("A");

  lcd.setCursor(11,1);
  sprintf(buff, "%02d", set_alarm[0]);
  lcd.print(buff);

  lcd.setCursor(13,1);
  lcd.print(":");

  lcd.setCursor(14,1);
  sprintf(buff, "%02d", set_alarm[1]);
  lcd.print(buff);
}

void cancelAlarmISR(){
  Serial.println("Interrupt");
  play_alarm = false;
}

