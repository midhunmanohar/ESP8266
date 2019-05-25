#include <ESP8266WiFi.h>
#include <stdlib.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <WiFiUdp.h>


#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// Initialize Telegram BOT
#define botToken "684752974:AAGGRJKTwDLOXwkR25ApQAeLPzYLL8kTu0c" 

// your Bot Token (Get from Botfather)
//WiFiClientSecure client;


// The person/group you want the alert messages to go to.
// You must start a chat with a bot first before it can send you messages
String defaultChatId = "-245713034"; //This can be got by using a bot called "myIdBot"


/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "@kadapanatham"
#define WLAN_PASS       "HAHAHA12345"
#define pin D6
#define red D1
#define red D2
#define lock D3

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "midhunmanohar"
#define AIO_KEY         "1843a8e537fe47a98d6e0907e8001d6f"

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
WiFiClientSecure client1;
// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
UniversalTelegramBot bot(botToken, client1);

/****************************** Feeds ***************************************/

// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
//Adafruit_MQTT_Publish photocell = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/feedback");

// Setup a feed called 'onoff' for subscribing to changes.
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/outside-light");


/*************************** Sketch Code ************************************/

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();
int counter;


void setup() {
  Serial.begin(115200);
  delay(10);
  pinMode(5, OUTPUT);
  pinMode(4, OUTPUT);
  counter=0;
  Serial.println(F("Adafruit MQTT"));

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  // Setup MQTT subscription for onoff feed.
  mqtt.subscribe(&onoffbutton);
  
}


void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &onoffbutton) {
      int val = atoi((char *)onoffbutton.lastread);
      Serial.println(val);
      if (val==0)
      {
        //Serial.println(val);
        digitalWrite(5, HIGH);
        if (counter ==1)
        {
          bot.sendMessage(defaultChatId, "Outside light is OFF");
          counter =0;
        }
      }
      if (val==1)
      {
        digitalWrite(5, LOW);
        if (counter == 0)
        {
          bot.sendMessage(defaultChatId, "Outside light is ON");
          counter =1;
        }

        //delay(4000);
        //Serial.print(F("\nSending door val "));
        //x=0;
        //Serial.print(x);
        //Serial.print("...");
        //if (! photocell.publish(x)) {
        //   Serial.println(F("Failed"));
        //} else {
           //Serial.println(F("OK!"));
        
        //digitalWrite(5, HIGH);
        //digitalWrite(4, LOW);
        //}
      }
    
    }
  }

  // Now we can publish stuff!
//  Serial.print(F("\nSending door val "));
//  if(x==2)
//  {
//    Serial.print(x);
//    Serial.print("...");
//    if (! photocell.publish(x)) {
//      Serial.println(F("Failed"));
//    } else {
//      Serial.println(F("OK!"));
//      delay(2000);
//      x=3;
//    }
//  }
  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
  
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
  
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
}

  Serial.print("Connecting to MQTT... ");

  int retries = 10000;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       for (int i=5;i>0;i--)
       {
        digitalWrite(5, LOW);
        delay(500);
        digitalWrite(5, LOW);
        delay(500);
       }
         // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1){
           digitalWrite(5, LOW);
           delay(1000);
           digitalWrite(5, HIGH);;
           delay(1000);
         }
       }
       
  }
  Serial.println("MQTT Connected!");

}
