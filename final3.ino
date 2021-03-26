#include <ESP8266WiFi.h>
#include <EEPROM.h>

void ICACHE_RAM_ATTR ISRoutine ();
const int button = 5;      //connect the pushbutton on digital pin D2
int  counter = 0;         //initialize a counter
int previous = 0;        //initialize a variable
byte  state = 0;        //initialize a state

bool buttonState = 0;           // current state of the button
bool defaultButtonState = 1;   // previous state of the button
int startPressed = 0;         // the moment the button was pressed
int endPressed = 0;          // the moment the button was released
int holdTime = 0;           // how long the button was hold in ms
float holdTime1 = 0;       //hold time in second

const byte Fan = 4;                 //GPIO 4 (D2/SDA) of Node MCU ESP8266 is assigned to BLDC 12V Fan of AEREM 100 named as "Fan"
const byte Blue_LED = 14;          //GPIO 12(D6/HMISO) of Node MCU ESP8266 is assigned to Blue LED's of AEREM 100 named as "Blue_LED"
const byte Red_LED = 12 ;         //GPIO 14(D5/HSCLK) of Node MCU ESP8266 is assigned to Red LED's of AEREM 100 named as "Red_LED"
int NoOfPress = 0;               // variable for changes in loop
int i = 0;

unsigned long endTime = 0;
unsigned long endTime1 = 0;
unsigned long startTime = 0;
unsigned long uv_time = 0;
unsigned long uv_time2 = 0;
float uv_time1 = 0;
float uv_time3 = 0;
int count = 0;
float Cumulative_UV_Ontime=0;

void ISRoutine()      //create a function where interrupt check the state of push button
{
  delay(15);
  state = ! state;
}

void updateState()
{
  if (buttonState == HIGH)
  {
    endPressed = millis();
    Serial.print("end time = ");
    Serial.println(endPressed);
    holdTime = (endPressed - startPressed);
    holdTime1 = holdTime * 0.001;
    Serial.print("hold time1 = ");
    Serial.println(holdTime1);
  }

  else
  {
    startPressed = millis();
    Serial.print("start_time = ");
    Serial.println(startPressed);
  }
}
//void EEPROMReset ()
//{
//for (i = 0 ; i < EEPROM.length() ; i++)
//    {
//      EEPROM.write(i, 0);
//    }
//}

void setup()
{
  pinMode(button, INPUT_PULLUP);       // initialize the button pin as a input pull_up
  Serial.begin(115200);               // initialize serial communication
  EEPROM.begin(512);
  pinMode(Blue_LED, OUTPUT);          //Blue LEDs as Output
  pinMode(Red_LED, OUTPUT);           //Red LEDs as Output
  pinMode(Fan, OUTPUT);               //Fan as Output
  attachInterrupt(digitalPinToInterrupt(button), ISRoutine, FALLING); //attach the interrupt
  digitalWrite(Blue_LED, HIGH);
  digitalWrite(Red_LED, HIGH);
  digitalWrite(Fan, LOW);

}

void loop()
{
  if (state != previous)
  {
    previous = state;
    counter++;
    Serial.print("counter = ");
    Serial.println(counter);      //print the counter
  }

  buttonState = digitalRead(button); // read the button input
  if (buttonState != defaultButtonState)  // button state changed
  {
    updateState();
  }
  defaultButtonState = buttonState;        // save state for next loop

  // for one second
  if (holdTime1 > 0.50 && holdTime1 <= 1.5)
  {
    endTime1 = millis();
    uv_time2 = (endTime1 - startTime);
    uv_time3 = (uv_time2 * 0.001)/60;
    Serial.print("uv_time_in_minute3 = ");
    Serial.println(uv_time3);
    
    EEPROM.put(0, uv_time3);
    if (EEPROM.commit())
      Serial.println("EEPROM Successfully Committed");
    else
      Serial.println("ERROR! EEPROM commit failed");
    
    digitalWrite(Blue_LED, LOW);
    digitalWrite(Red_LED, HIGH);
    analogWrite(Fan, 300);
    Serial.println("blue led on fan at low speed");
    NoOfPress = 0;
    holdTime1 = 25;
    count++;
    float z = EEPROM.read(1);
    Serial.print("reading from eeprom = ");
    Serial.println(z);
  }

  //for two second ON
  if (holdTime1 > 1.5 && holdTime1 <= 2.5)
  {
    if(count == 1)
    {
      //notthing
    }
    else
    {
      endTime = millis();
      uv_time = (endTime - startTime);
      uv_time1 = (uv_time * 0.001)/60;
      Serial.print("uv_time_in_second1 = ");
      Serial.println(uv_time1);

      EEPROM.put(0, uv_time1);
    if (EEPROM.commit())
      Serial.println("EEPROM Successfully Committed");
    else
      Serial.println("ERROR! EEPROM commit failed");
    }
    int y = EEPROM.read(1);
    Serial.print("reading from eeprom = ");
    Serial.println(y);
    
    digitalWrite(Blue_LED, HIGH);
    digitalWrite(Red_LED, HIGH);
    digitalWrite(Fan, LOW);
    Serial.println("long press everything off");
    NoOfPress = 0;
    holdTime1 = 14;
    startTime = 0;
    count = 0;
  }

  //for double click
  if (holdTime1 >= 0.01 && holdTime1 < 0.50)
  {
    NoOfPress++;
    Serial.println("NoOfPress1");
    if (NoOfPress < 2)
    {
      NoOfPress++;
      //      startTime = millis();
      Serial.println("NoOfPress2");
      holdTime1 = 10;
    }
    else
    {
      uv_time1 = EEPROM.get(0,uv_time1);          
      Serial.print("Previous cycle uv_time1 = ");       
      Serial.println(uv_time1); 

      uv_time3 = EEPROM.get(0,uv_time3);          
      Serial.print("Previous cycle uv_time3 = ");       
      Serial.println(uv_time3); 

      Cumulative_UV_Ontime = Cumulative_UV_Ontime + uv_time1 + uv_time3;
      Serial.print("Cumulative_UV_Ontime = ");  
      Serial.println(Cumulative_UV_Ontime);                    
      
      startTime = millis();
      analogWrite(Fan, 1023);
      digitalWrite(Red_LED, LOW);
      digitalWrite(Blue_LED, HIGH);
      Serial.println(" double click, fan at high speed, red led off");
      holdTime1 = 18;
    }
  }

  //  if (holdTime1 >= 5 && holdTime1 <= 8)
  //  {
  //    EEPROMReset();
  //    Serial.println("EEPROM Reset");
  //    NoOfPress = 0;
  //  }
  // if (holdTime1 >= 3 && holdTime1 <= 5)
  // {
  //  int y = EEPROM.read(1);
  //  Serial.print("reading from eeprom = ");
  //  Serial.println(y);
  // }

}
