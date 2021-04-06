#include "OneButton.h"                 //library for one button interrupt.h
#include<EEPROM.h>                     //include the eeprom libabry
#define PIN_INPUT 5                    //connect the pushbutton on pin number D1 or GPIO 4
const byte Fan = 4;                    //GPIO 4 (D2/SDA) of Node MCU ESP8266 is assigned to BLDC 12V Fan of AEREM 100 named as "Fan"
const byte Blue_LED = 14;              //GPIO 12(D6/HMISO) of Node MCU ESP8266 is assigned to Blue LED's of AEREM 100 named as "Blue_LED"
const byte Red_LED = 12 ;              //GPIO 14(D5/HSCLK) of Node MCU ESP8266 is assigned to Red LED's of AEREM 100 named as "Red_LED"               
float Cumulative_UV_Ontime = 0;        //take a varibale to store the both the uv time
unsigned long startTime = 0;           //start the millis in long press
float uv_timeS1 = 0;                   //variable to store the time in eeprom in single click
unsigned long uv_timeS = 0;            //take a varibale to store the millis time
int singleclick = 0;                    //take a variable to check weather it is on of off
int doubleclick = 0;                    //take a variable to check weather the time is store in eeprom or not

OneButton button(PIN_INPUT, true);
unsigned long pressStartTime;          //take a variable to stor the millis value
ICACHE_RAM_ATTR void checkTicks()      //attach the ram attribute
{
  button.tick();                       //check the button ticks
}


void singleClick()
{
  if(doubleclick == 1)                                    //if doubleclick is equal to one then store the uv lamp time in eeprom
  {
  unsigned long endTimeS = millis();                      //start the millis
  unsigned long uv_timeS = (endTimeS - startTime);
  uv_timeS1 = (uv_timeS * 0.001) / 60;                    //convert the millis in minute
  Serial.print("uv_time_in_minuteS = ");
  Serial.println(uv_timeS1);                              //print the uv_time3 and see for how much time our is on at high high speed

  EEPROM.put(0, uv_timeS1);                                 //sotre the minute in eeprom
  if (EEPROM.commit()){                                     //check weather the data is save in eeprom or not
    Serial.println("EEPROM Successfully Committed");}       
  else{
    Serial.println("ERROR! EEPROM commit failed");}
    doubleclick = 0;
  }
  if(singleclick == 0)
  {
  Serial.println("Ioniser turn on");
  digitalWrite(Blue_LED, LOW);                      //turn on the blue led
  digitalWrite(Red_LED, HIGH);                      //turn off the red led
  analogWrite(Fan, 300);
  singleclick = 1;
  }
  else{
  Serial.println("ioniser turn off");
  digitalWrite(Blue_LED, HIGH);                     //turn off the blue led
  digitalWrite(Red_LED, HIGH);                      //turn off the red led
  analogWrite(Fan, LOW);                            //turn off the fan
  singleclick = 0;
  }
  }

  
void doubleClick()
{
  float uv_timeD = EEPROM.get(0, uv_timeS1);                //read the eeprom
  Serial.print("Previous cycle uv_timeD = ");
  Serial.println(uv_timeD);                                // print the eeprom value

  Cumulative_UV_Ontime = (Cumulative_UV_Ontime + uv_timeD);   //store the previous and current value of eeprom
  Serial.print("Cumulative_UV_Ontime = ");
  Serial.println(Cumulative_UV_Ontime);                       //print the how uch values are store in eeprom

  startTime = millis();                                       //start the millis
  Serial.println("doubleClick() detected.");
  digitalWrite(Blue_LED, HIGH);                               //turn off the blue led
  digitalWrite(Red_LED, LOW);                                 //turn on the red led
  analogWrite(Fan, 1023);                                     // fan at high speed
  doubleclick = 1;                                               
}


void long_longpress()
{
  for (int i = 0 ; i < EEPROM.length() ; i++)
    {
      EEPROM.write(i, 0);                                     //reset the eeprom memory
    }
    Serial.println("Eeprom Reset");
    Cumulative_UV_Ontime = 0;                                 //reset the Cummulative time
}

void pressStop()
{
  //nothing
}


void setup()
{
  Serial.begin(115200);                                //initialize the baudrate
  pinMode(Blue_LED, OUTPUT);                          // sets the blue led as output
  pinMode(Red_LED, OUTPUT);                           //set the red led as output
  pinMode(Fan, OUTPUT);                               //set the fan as the output
  digitalWrite(Blue_LED, HIGH);                       //turn off the blue led
  digitalWrite(Red_LED, HIGH);                        //turn off the red led                  
  analogWrite(Fan, LOW);                               //turn of the fan
  EEPROM.begin(512);                                  //start the eeprom
  attachInterrupt(digitalPinToInterrupt(PIN_INPUT), checkTicks, CHANGE);     //attach trhe interrupt on pin D1

  button.attachClick(singleClick);                                        //call the singleclick fn
  button.attachDoubleClick(doubleClick);                                  //call the double click fn

  button.setPressTicks(5000);
  button.attachLongPressStart(long_longpress);                            //call the start press fn
  button.attachLongPressStop(pressStop);                                  //call the stop press fn
}
void loop()
{
  button.tick();
  delay(10);
}
