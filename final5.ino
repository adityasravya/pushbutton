#include <WiFiManager.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
WiFiClient client;
WiFiServer server(80);
void ICACHE_RAM_ATTR ISRoutine ();

const byte Fan = 4;                 //GPIO 4 (D2/SDA) of Node MCU ESP8266 is assigned to BLDC 12V Fan of AEREM 100 named as "Fan"
const byte Blue_LED = 14;          //GPIO 12(D6/HMISO) of Node MCU ESP8266 is assigned to Blue LED's of AEREM 100 named as "Blue_LED"
const byte Red_LED = 12 ;         //GPIO 14(D5/HSCLK) of Node MCU ESP8266 is assigned to Red LED's of AEREM 100 named as "Red_LED"

//for button and interrupt
const int button = 5;      //connect the pushbutton on digital pin D2
int  counter = 0;         //initialize a counter
int previous = 0;        //initialize a variable
byte  state = 0;        //initialize a state
String request;

bool buttonState = 0;           // current state of the button
bool defaultButtonState = 1;   // previous state of the button
int startPressed = 0;         // the moment the button was pressed
int endPressed = 0;          // the moment the button was released
int holdTime = 0;           // how long the button was hold in ms
float holdTime1 = 0;       //hold time in second
int i;

//for double press
int NoOfPress = 0;               // variable for changes in loop
unsigned long startTime = 0;    //start the millis in long press
float Cumulative_UV_Ontime = 0; // store the both eeprom value like uv_time1 and uv_time3

//for long press
int count = 0;                  //take a counter to check the  mode
unsigned long endTime = 0;      // store the end millis in longpress
unsigned long uv_time = 0;      //varable to store the time in long press//for single press
float uv_timeL = 0;             // variable to store the time in eeprom in long press


//for single press
unsigned long endTimeS = 0;     //store the end millis in single click
unsigned long uv_time2 = 0;     //variable to store the time in single click
float uv_timeS = 0;             //variable to store the time in eeprom in single click


void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  Serial.println(myWiFiManager->getConfigPortalSSID());
}

void ISRoutine()              //create a function where interrupt check the state of push button
{
  delay(15);                 // delay of 25ms for debouncing
  state = ! state;
}

void updateState()          //make a function here we are calculating the time how long we press the button
{
  if (buttonState == HIGH)
  {
    endPressed = millis();                            //start the millis function when button will be release
    Serial.print("end time = ");                      //print the end time
    Serial.println(endPressed);                       //print the millis function
    holdTime = (endPressed - startPressed);           //subtract the time from endpress to startpress
    holdTime1 = holdTime * 0.001;                     //convert the milli second in second
    Serial.print("hold time1 = ");                    //print the hold time
    Serial.println(holdTime1);                        //print the hold time value
  }

  else
  {
    startPressed = millis();                          //start the millis when button is pressed
    Serial.print("start_time = ");                    //print the start time
    Serial.println(startPressed);                     //print the millis
  }
}

void EEPROMReset ()                                   //make a function here we are restting the eeprom
{
  for (i = 0 ; i < EEPROM.length() ; i++)
  {
    EEPROM.write(i, 0);
  }
}

void setup()
{
  pinMode(button, INPUT_PULLUP);       // initialize the button pin as a input pull_up
  EEPROM.begin(512);
  pinMode(Blue_LED, OUTPUT);          //Blue LEDs as Output
  pinMode(Red_LED, OUTPUT);           //Red LEDs as Output
  pinMode(Fan, OUTPUT);               //Fan as Output
  Serial.begin(115200);
  attachInterrupt(digitalPinToInterrupt(button), ISRoutine, FALLING); //attach the interrupt

  server.begin();
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  WiFiManager wm;     //Local intialization. Once its business is done, there is no need to keep it around
  wm.setAPCallback(configModeCallback);
  if (!wm.autoConnect()) {
    Serial.println("failed to connect and hit timeout");
    ESP.restart();
    delay(1000);
  }
  Serial.println("connected...yeey :)");
  digitalWrite(Blue_LED, HIGH);       //blue led off
  digitalWrite(Red_LED, HIGH);        //red led off
  digitalWrite(Fan, LOW);             //fan of
}

void loop()
{
  if (state != previous)            //check the state is not eqaul to previous
  {
    previous = state;               //store the state in previous for next iteration
    counter++;
    Serial.print("counter = ");
    Serial.println(counter);      //print the counter
  }

  buttonState = digitalRead(button); // read the button input
  if (buttonState != defaultButtonState)  // button state changed
  {
    updateState();                        // call the update state function
  }
  defaultButtonState = buttonState;        // save state for next loop

  client = server.available();
  if (client == 1)
  {
    request = client.readStringUntil('\n');
    Serial.println(request);
    request.trim();
  }

  // for single click or single press
  if (request == "GET /singlepress HTTP/1.1" || (holdTime1 > 0.50 && holdTime1 <= 1.5))
  {
    endTimeS = millis();                      //start the millis
    uv_time2 = (endTimeS - startTime);
    uv_timeS = (uv_time2 * 0.001) / 60;       //convert the millis in minute
    Serial.print("uv_time_in_minuteS = ");
    Serial.println(uv_timeS);                  //print the uv_time3 and see for how much time our is on at high high speed

    EEPROM.put(0, uv_timeS);                  //sotr the minute in eeprom
    if (EEPROM.commit())
      Serial.println("EEPROM Successfully Committed");
    else
      Serial.println("ERROR! EEPROM commit failed");

    digitalWrite(Blue_LED, LOW);                  //turn on the blue led
    digitalWrite(Red_LED, HIGH);                  //turn off the red led
    analogWrite(Fan, 300);                        //turn on the fan at low speed
    Serial.println("blue led on fan at low speed");
    NoOfPress = 0;                                 //no of press is equal to zero
    holdTime1 = 25;                                 //change the state
    count++;
  }

  // for double click or double press
  if (request == "GET /doublepress HTTP/1.1" || (0.01 && holdTime1 < 0.50))
  {
    NoOfPress++;
    Serial.println("NoOfPress1");
    if (NoOfPress < 2)
    {
      NoOfPress++;
      Serial.println("NoOfPress2");
      holdTime1 = 10;
    }
    else
    {
      uv_timeL = EEPROM.get(0, uv_timeL);
      Serial.print("Previous cycle uv_timeL = ");
      Serial.println(uv_timeL);

      uv_timeS = EEPROM.get(0, uv_timeS);
      Serial.print("Previous cycle uv_timeS = ");
      Serial.println(uv_timeS);

      Cumulative_UV_Ontime = (Cumulative_UV_Ontime + uv_timeL + uv_timeS) / 2;
      Serial.print("Cumulative_UV_Ontime = ");
      Serial.println(Cumulative_UV_Ontime);

      startTime = millis();
      analogWrite(Fan, 1023);
      digitalWrite(Red_LED, LOW);
      digitalWrite(Blue_LED, HIGH);
      Serial.println(" double click, fan at high speed, red led off");
      holdTime1 = 18;
      client == 0;
    }
  }

  //for 2second or long press
  if (request == "GET /longpress HTTP/1.1" || (holdTime1 > 1.5 && holdTime1 <= 2.5))
  {
    if (count == 1)
    {
      //notthing
    }
    else
    {
      endTime = millis();
      uv_time = (endTime - startTime);
      uv_timeL = (uv_time * 0.001) / 60;
      Serial.print("uv_time_in_second1 = ");
      Serial.println(uv_timeL);

      EEPROM.put(0, uv_timeL);
      if (EEPROM.commit())
        Serial.println("EEPROM Successfully Committed");
      else
        Serial.println("ERROR! EEPROM commit failed");
    }
    digitalWrite(Blue_LED, HIGH);
    digitalWrite(Red_LED, HIGH);
    digitalWrite(Fan, LOW);
    Serial.println("long press everything off");
    NoOfPress = 0;
    holdTime1 = 14;
    startTime = 0;
    count = 0;
  }

  if (holdTime1 >= 5 && holdTime1 <= 8)
  {
    EEPROMReset();
    Serial.println("EEPROM Reset");
    NoOfPress = 0;
    Cumulative_UV_Ontime = 0;
    holdTime1 = 32;
  }
}
