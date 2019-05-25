#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <Wire.h>
#include "RTClib.h"
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

RTC_DS1307 rtc;

// Initialize Telegram BOT
#define botToken "684752974:AAGGRJKTwDLOXwkR25ApQAeLPzYLL8kTu0c" 

// your Bot Token (Get from Botfather)
WiFiClientSecure client;
UniversalTelegramBot bot(botToken, client);


// The person/group you want the alert messages to go to.
// You must start a chat with a bot first before it can send you messages
String defaultChatId = "-245713034"; //This can be got by using a bot called "myIdBot"


/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "@kadapanatham"
#define WLAN_PASS       "HAHAHA12345"
//#define WLAN_SSID       "G6Home"
//#define WLAN_PASS       "sam@1234"
#define light D3
#define led LED_BUILTIN

int hours;
int minutes;
int api_ontime;
int api_offtime;
String timet;
int timett;
int ontime;
int offtime;
int counter;

HTTPClient http; //Object of class HTTPClient

void setup() {
  Serial.begin(115200);
  pinMode(light, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(light, HIGH);
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
  // Connect to WiFi access point.
  Serial.print("Connecting...");
  Serial.println(WLAN_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  get_time();
}

void get_time()
{
  http.begin("http://36.255.87.81:5003/api/timeset");
  int httpCode = http.GET();
  Serial.println(httpCode);
  if (httpCode > 0) 
  {
    //DynamicJsonDocument doc(1024);
    //deserializeJson(doc, http.getString());
    DynamicJsonBuffer jsonBuffer(1024);
    JsonObject& doc = jsonBuffer.parseObject(http.getString());
    api_ontime= doc["ontime"];
    api_offtime= doc["offtime"];
  }
  else
  {
    Serial.println("No data");
  }
  http.end(); //Close connection
}

void loop() {
  DateTime now = rtc.now();
  hours = now.hour();
  minutes = now.minute();
 
  get_time();

  if (minutes<10)
  {
    timet = String(hours)+"0"+ String(minutes);
  }
  else
  {   
    timet = String(hours)+String(minutes);
  }
  timett= timet.toInt();
  Serial.println(timett);
  Serial.println("ONtime: "+String(api_ontime)); //1830
  Serial.println("Offtime: "+String(api_offtime)); //700
  //if (timett >= 700 && timett <= 1829){
    if (timett > api_ontime && timett < api_offtime){
       Serial.println("ON");
       digitalWrite(light, LOW);
       if (counter ==1){
          Serial.println("Counter One");
          bot.sendMessage(defaultChatId, "Outside light switched ON at "+ String(hours) +":"+ String(minutes));
          counter =0;
      }
    }
    else{
      digitalWrite(light, HIGH);
      Serial.println("OFF");
      if (counter == 0){
        Serial.println("Counter Zero");
        bot.sendMessage(defaultChatId, "Outside light switched OFF at "+ String(hours) +":"+ String(minutes));;
        counter =1;
      }
      
    }
    
  delay(59600);
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED on (HIGH is the voltage level)
  delay(200);                       // wait for a second
  digitalWrite(LED_BUILTIN, HIGH);    // turn the LED off by making the voltage LOW
  delay(200);
}
