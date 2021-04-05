#include "OneButton.h"    //library for one button interrupt.h
#include<EEPROM.h>        //include the eeprom libabry
#define PIN_INPUT 5       //connect the pushbutton on pin number D1 or GPIO 4
const byte Fan = 4;                 //GPIO 4 (D2/SDA) of Node MCU ESP8266 is assigned to BLDC 12V Fan of AEREM 100 named as "Fan"
const byte Blue_LED = 14;          //GPIO 12(D6/HMISO) of Node MCU ESP8266 is assigned to Blue LED's of AEREM 100 named as "Blue_LED"
const byte Red_LED = 12 ;         //GPIO 14(D5/HSCLK) of Node MCU ESP8266 is assigned to Red LED's of AEREM 100 named as "Red_LED"

float holdTime1 = 0;              //take varibale to store the time
float Cumulative_UV_Ontime = 0;   //take a varibale to store the both the uv time
unsigned long startTime = 0;      //start the millis in long press
int count = 0;                   //take a counter to check the  mode
float uv_timeS1 = 0;            //variable to store the time in eeprom in single click
unsigned long uv_timeS = 0;     //take a varibale to store the millis time

OneButton button(PIN_INPUT, true);  
unsigned long pressStartTime;     //take a variable to stor the millis value
ICACHE_RAM_ATTR void checkTicks()  //attach the ram attribute 
{
  button.tick();                   //check the button ticks
}

void singleClick()           
{
  if(count == 1)
  {
  unsigned long endTimeS = millis();                      //start the millis
  unsigned long uv_timeS = (endTimeS - startTime);
  uv_timeS1 = (uv_timeS * 0.001) / 60;                //convert the millis in minute
  Serial.print("uv_time_in_minute3 = ");
  Serial.println(uv_timeS1);                          //print the uv_time3 and see for how much time our is on at high high speed

  EEPROM.put(0, uv_timeS1);                           //sotre the minute in eeprom
  if(EEPROM.commit())
    Serial.println("EEPROM Successfully Committed");
  else
    Serial.println("ERROR! EEPROM commit failed");
  }
  Serial.println("singleClick() detected.");
  digitalWrite(Blue_LED, LOW);                      //turn on the bvlue led
  digitalWrite(Red_LED, HIGH);                      //turn off the red led
  analogWrite(Fan, 300);                            //fan ata low speed
  count = 0;                                        //assign the counter zero
  startTime = 0;                                    //assign the start time zero
}
void doubleClick()
{
  float uv_timeD = EEPROM.get(0, uv_timeS1);          //read the eeprom
  Serial.print("Previous cycle uv_timeD = ");
  Serial.println(uv_timeD);                           // print the eeprom value

  Cumulative_UV_Ontime = (Cumulative_UV_Ontime + uv_timeD);   //store the previous and current value of eeprom
  Serial.print("Cumulative_UV_Ontime = ");
  Serial.println(Cumulative_UV_Ontime);                       //print the how uch values are store in eeprom
  
  startTime = millis();                                       //start the millis 
  Serial.println("doubleClick() detected.");
  digitalWrite(Blue_LED, HIGH);                               //turn off the blue led
  digitalWrite(Red_LED, LOW);                                 //turn on the red led
  analogWrite(Fan, 1023);                                     // fan at high speed
  count++;                                                    //increase the counter
}

void pressStart()
{
  Serial.println("pressStart()");                             
  pressStartTime = millis() - 1000; // as set in setPressTicks()    //start the time when button is pressed more than 1 sec
}

void pressStop()
{
  Serial.println("pressStop()");
  unsigned long holdTime = (millis() - pressStartTime);             //start the millis when button will release and calculate the difference
  holdTime1 = holdTime * 0.001;                                     //convert the millis second to second
  Serial.println(holdTime1);
}

void Reset()
{
  for (int i = 0 ; i < EEPROM.length() ; i++)
    {
      EEPROM.write(i, 0);                                     //reset the eeprom memory
    }
}
void setup()
{
  Serial.begin(115200);                                      //initialize the baudrate
  Serial.println("One Button Example with interrupts.");

  // enable the led output.
  pinMode(Blue_LED, OUTPUT);                          // sets the blue led as output
  pinMode(Red_LED, OUTPUT);                           //set the red led as output
  pinMode(Fan, OUTPUT);                               //set the fan as the output
  digitalWrite(Blue_LED, HIGH);
  digitalWrite(Red_LED, HIGH);
  analogWrite(Fan, LOW);
  EEPROM.begin(512);                                  //start the eeprom
  attachInterrupt(digitalPinToInterrupt(PIN_INPUT), checkTicks, CHANGE);     //attach trhe interrupt on pin D1

  button.attachClick(singleClick);                                        //call the singleclick fn
  button.attachDoubleClick(doubleClick);                                  //call the double click fn

  button.setPressTicks(1000);
  button.attachLongPressStart(pressStart);                                //call the start press fn
  button.attachLongPressStop(pressStop);                                  //call the stop press fn
}
void loop()
{
  button.tick();
  delay(10);
  if(holdTime1 >= 2 && holdTime1 <= 5)                                //i8f hold time is between the 2 to 5 then consider as long press
  {
    if (count == 1)                                                   //if counter is equal to one the calculate the time difference between double click to long press
  {
    unsigned long endTimeL = millis();                                //start the millis fn
    unsigned long uv_timeL = (endTimeL - startTime);
    float uv_timeL1 = (uv_timeL * 0.001) / 60;                        //calculate the time how much uv lamp has glow in double click
    Serial.print("uv_time_in_minute = ");
    Serial.println(uv_timeL1);
    EEPROM.put(0, uv_timeL1);
    if(EEPROM.commit())
      Serial.println("EEPROM Successfully Committed");
    else
      Serial.println("ERROR! EEPROM commit failed");
  }
    Serial.println("longpress");
    digitalWrite(Blue_LED, HIGH);
    digitalWrite(Red_LED, HIGH);
    analogWrite(Fan, LOW);
    startTime = 0;
    count = 0;
    holdTime1 = 15;
  }
  if (holdTime1 >= 5 && holdTime1 <= 10)
  {
    Serial.println("Eeprom Reset");
    Reset();
    Cumulative_UV_Ontime = 0;
    count = 0;
    startTime = 0;
    holdTime1 = 65;
  }
}
