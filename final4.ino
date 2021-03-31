#include <WiFiManager.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
WiFiClient client;
WiFiServer server(80);
const byte Fan = 4;                 //GPIO 4 (D2/SDA) of Node MCU ESP8266 is assigned to BLDC 12V Fan of AEREM 100 named as "Fan"
const byte Blue_LED = 14;          //GPIO 12(D6/HMISO) of Node MCU ESP8266 is assigned to Blue LED's of AEREM 100 named as "Blue_LED"
const byte Red_LED = 12 ;         //GPIO 14(D5/HSCLK) of Node MCU ESP8266 is assigned to Red LED's of AEREM 100 named as "Red_LED"

unsigned long startTime = 0;    //start the millis in long press
unsigned long endTime1 = 0;     //store the end millis in single click
unsigned long uv_time2 = 0;     //variable to store the time in single click
float uv_time3 = 0;             //variable to store the time in eeprom in single click
int count = 0;
int x = 0;
unsigned long endTime = 0;      // store the end millis in longpress
unsigned long uv_time = 0;      //varable to store the time in long press
float uv_time1 = 0;             // variable to store the time in eeprom in long press


void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  Serial.println(myWiFiManager->getConfigPortalSSID());
}

void setup()
{
  server.begin();
  pinMode(Blue_LED, OUTPUT);          //Blue LEDs as Output
  pinMode(Red_LED, OUTPUT);           //Red LEDs as Output
  pinMode(Fan, OUTPUT);               //Fan as Output
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  Serial.begin(115200);
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
  client = server.available();
  if (client == 1)
  {
    String request = client.readStringUntil('\n');
    Serial.println(request);
    request.trim();
    
    if (request == "GET /singlepress HTTP/1.1")
    {
      endTime1 = millis();
      uv_time2 = (endTime1 - startTime);
      uv_time3 = (uv_time2 * 0.001) / 60;       //convert the millis in minute
      Serial.print("uv_time_in_minute3 = ");
      Serial.println(uv_time3);                  //print the uv_time3 and see for how much time our is on at high high speed 
      
      digitalWrite(Blue_LED, LOW);                  //turn on the blue led
      digitalWrite(Red_LED, HIGH);                  //turn off the red led
      analogWrite(Fan, 500);                        //turn on the fan at low speed
      Serial.println("blue led on fan at low speed");
      request = "GET /doublepresslong HTTP/1.1";
      x++;
      Serial.print("x = ");
      Serial.println(x);
    }
    if (request == "GET /doublepress HTTP/1.1")
    {
      count++;
      Serial.print("count = ");
      Serial.println(count);
      startTime = millis();
      analogWrite(Fan, 1023);
      digitalWrite(Red_LED, LOW);
      digitalWrite(Blue_LED, HIGH);
      Serial.println(" double click, fan at high speed, red led on");
      request = "GET /doublepresslong HTTP/1.1";
    }
   if (request == "GET /longpress HTTP/1.1")
   {
    endTime = millis();
    uv_time = (endTime - startTime);
    uv_time1 = (uv_time * 0.001) / 60;
    Serial.print("uv_time_in_second1 = ");
    Serial.println(uv_time1);
    
    request = "GET /singlepresss HTTP/1.2";
    digitalWrite(Blue_LED, HIGH);
    digitalWrite(Red_LED, HIGH);
    digitalWrite(Fan, LOW);
    Serial.println("long press everything off");
    request = "GET /doublepresslong HTTP/1.1";
   }
  }

}
