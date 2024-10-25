/*
* Name: clock and temp project
* Author: Hugo Romeborn
* Date: 2024-10-25
* Description: This project uses a ds3231 to measure time and displays the time to an 1306 oled display,
* Further, it measures temprature with a analog temprature module and displays a mapped value to a 9g-servo-motor and on the oled display.
* It has a menu from which you can set a timer and alarm, when finished it makes a sound from a piezo. 
*/

// Include Libraries
#include <RTClib.h>
#include <Wire.h>
#include "U8glib.h"
#include <Servo.h>


// Init constants
const int tmpPin = A1;
const int btn1 = 11;
const int btn2 = 12;
const int btn3 = 13;
const int piezoPin = 8;


// Init global variables
int timer;
long int pastSeconds;
bool hasTimer = 0;
int alarmHour;
int alarmMinute;
bool alarmSounds = 0;

// construct objects
RTC_DS3231 rtc;
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NO_ACK);  // Display which does not send AC
Servo myservo;

void setup() {
  // init communication
  Serial.begin(9600);

  // Init Hardware
  u8g.setFont(u8g_font_unifont);  // sets a font for oled-screen
  Wire.begin();
  rtc.begin();
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  pinMode(tmpPin, INPUT);
  myservo.attach(9);
  pinMode(btn1, INPUT);
  pinMode(btn2, INPUT);
  pinMode(btn3, INPUT);
  pinMode(piezoPin, OUTPUT);
}

void loop() {
  oledWrite2(getTime(), String(getTemp()));  // Writes time and temperature on the oled screen
  servoWrite(getTemp());
  checkTimer();
  checkAlarm();
  stopAlarmSound();
  delay(50);
  if (digitalRead(btn2) == HIGH || digitalRead(btn3) == HIGH) {  // If a button is pressed, opens the menu to set timer and alarm
    mainMenu();
    delay(100);
  }
}

/*use a ds3231 module and package the time as a String
*Parameters: Void
*Returns: time in hh:mm:ss as String
*/
String getTime() {
  DateTime now = rtc.now();
  if(now.minute()<10 && now.second()<10){
    return String(now.hour()) + ":" + "0" + String(now.minute()) + ":" + "0" + String(now.second());
  }
  else if(now.minute()<10){
    return String(now.hour()) + ":" + "0" + String(now.minute()) + ":" + String(now.second());
  }
  else if(now.second()<10){
    return String(now.hour()) + ":" + String(now.minute()) + ":" + "0" + String(now.second());
  }
  else{
    return String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second());
  }
}

/*
* This function reads an analog pin connected to an analog temprature sensor and calculates the corresponding temp
*Parameters: Void
*Returns: temprature as float
*/
float getTemp() {
  int Vo;
  float R1 = 10000;  // value of R1 on board
  float logR2, R2, T;
  float c1 = 0.001129148, c2 = 0.000234125, c3 = 0.0000000876741;  //steinhart-hart coeficients for thermistor

  Vo = analogRead(tmpPin);
  R2 = R1 * (1023.0 / (float)Vo - 1.0);  //calculate resistance on thermistor
  logR2 = log(R2);
  T = (1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2));  // temperature in Kelvin
  T = T - 273.15;                                              //convert Kelvin to Celcius

  return T;
}

/*
* This function takes a string and draws it to an oled display
*Parameters: - text: String to write to display
*Returns: void
*/
void oledWrite(String text) {
  u8g.firstPage();
  do {
    u8g.drawStr(30, 10, text.c_str());
  } while (u8g.nextPage());
}

/*
* This function takes two strings and draws it to an oled display
*Parameters: - text: two Strings to write to display
*Returns: void
*/
void oledWrite2(String text, String text2) {
  u8g.firstPage();
  do {
    u8g.drawStr(30, 10, text.c_str());
    u8g.drawStr(30, 40, text2.c_str());
  } while (u8g.nextPage());
}

/*
* This function takes three strings and draws it to an oled display
*Parameters: - text: three Strings to write to display
*Returns: void
*/
void oledWrite3(String text, String text2, String text3) {
  u8g.firstPage();
  do {
    u8g.drawStr(30, 22, text.c_str());
    u8g.drawStr(30, 42, text2.c_str());
    u8g.drawStr(30, 62, text3.c_str());
  } while (u8g.nextPage());
}


/*
* takes a temprature value and maps it to corresppnding degree on a servo
*Parameters: - value: temprature
*Returns: void
*/
void servoWrite(float value) {
  int angle = map(value, 10, 30, 0, 180);

  myservo.write(angle);
}

/* 
* the main menu from where the user can set a timer an alarm
* parameters  void
* Return   void
*/
void mainMenu() {
  oledWrite3("TIMER", "ALARM", "HOME");
  delay(500);
  while (digitalRead(btn3) == LOW) {
    oledWrite3("TIMER", "ALARM", "HOME");
    delay(50);
    if (digitalRead(btn2) == HIGH) {
      alarm();
    } else if (digitalRead(btn1) == HIGH) {
      setTimer();
    }
  }
}

/* 
* This is the function that sets an alarm
* parameters: Void
* Return: Void
*/
void alarm() {
  oledWrite2("set hour", "00");
  alarmHour = 0;
  while (digitalRead(btn3) == LOW) {
    oledWrite2("set hour", String(alarmHour));
    if (digitalRead(btn1) == HIGH) {
      alarmHour++;
    }
    if (digitalRead(btn2) == HIGH) {
      alarmHour--;
    }
  }
  alarmMinute = 0;
  oledWrite2("set minute", String(alarmMinute));
  delay(200);
  while (digitalRead(btn3) == LOW) {
    oledWrite2("set minute", String(alarmMinute));
    if (digitalRead(btn1) == HIGH) {
      alarmMinute++;
    }
    if (digitalRead(btn2) == HIGH) {
      alarmMinute--;
    }
  }
}

/* 
* This is the function that sets a timer
* parameters: Void
* Return: Void
*/
void setTimer() {
  while (digitalRead(btn3) == LOW) {
    oledWrite(String(timer));
    if (digitalRead(btn1) == HIGH) {
      timer++;
    } else if (digitalRead(btn2) == HIGH) {
      timer--;
    }
    delay(100);
  }
  pastSeconds = millis()/1000;
  hasTimer = 1;
  Serial.println("Timer set");
}

/* This functionis called upon when the timer or alarm ends, makes a sound for 5 seconds with piezo
* parameters: Void
* Return: Void
*/
void soundAlarm() {
  // for (int i = 4; i <= 7; i++) {
  //   digitalWrite(i, HIGH);
  //   delay(50);
  //   digitalWrite(i, LOW);
  // }
  tone(piezoPin, 500);
  alarmSounds = 1;
}

/* This function checks if the timer is finished
* parameters: Void
* Return: Void
*/
void checkTimer() {
  if (hasTimer) {
    if (millis()/1000 >= (pastSeconds + (60 * timer))) {
      soundAlarm();
      hasTimer = 0;
      Serial.println("timer is out");
    }
  }
}

/* This function checks if the alarm is finished
* parameters: Void
* Return: Void
*/
void checkAlarm() {
  DateTime now = rtc.now();
  if (alarmHour == now.hour()) {
    if (alarmMinute == now.minute()) {
      soundAlarm();
    }
  }
}

/* This function scilence the piezo if button 1 is clicked
* parameters: void
* return: void
*/
void stopAlarmSound() {
  if (alarmSounds) {
    if (digitalRead(btn1)) {
      noTone(piezoPin);
      alarmSounds = 0;
    }
  }
}