#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <WiFiUdp.h>
#define light D1
#define statusled D4

// Initialize Telegram BOT
#define botToken "513037377:AAGkmqHkoobVIXqYjv5x47V9Ev3fgY_SZGc"  // your Bot Token (Get from Botfather)
WiFiClientSecure client;
UniversalTelegramBot bot(botToken, client);

// The person/group you want the alert messages to go to.
// You must start a chat with a bot first before it can send you messages
//String defaultChatId = "395386694"; //This can be got by using a bot called "myIdBot"
String defaultChatId = "-260318820"; //This can be got by using a bot called "myIdBot" - Group ChatID

char ssid[] = "Asianet broadband";       // your network SSID (name)
char password[] = "9846072300";  // your network key

unsigned int localPort = 2390;      // local port to listen for UDP packets

/* Don't hardwire the IP address or we won't get the benefits of the pool.
 *  Lookup the IP address for the host name instead */
//IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server
IPAddress timeServerIP; // time.nist.gov NTP server address
//const char* ntpServerName = "time.nist.gov";
const char* ntpServerName = "time.nplindia.org";

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
int counter;
int hours;
int minutes;
int seconds;
String timet;
int timett;
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;

unsigned long sendNTPpacket(IPAddress& address);

void setup()
{
  pinMode(light, OUTPUT);
  pinMode(statusled, OUTPUT);
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  counter=0;
  // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Starting UDP");
  udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());
  bot.sendMessage(defaultChatId, "ESP8266 rebooted!!");
}

void loop()
{
  //get a random server from the pool
  WiFi.hostByName(ntpServerName, timeServerIP); 

  sendNTPpacket(timeServerIP); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(1000);
  
  int cb = udp.parsePacket();
  if (!cb) {
    Serial.println("no packet yet");
  }
  else {
    Serial.print("packet received, length=");
    Serial.println(cb);
    digitalWrite(statusled, LOW);
    delay(500);
    digitalWrite(statusled, HIGH);
    // We've received a packet, read the data from it
    udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    //Serial.print("Seconds since Jan 1 1900 = " );
    //Serial.println(secsSince1900);

    // now convert NTP time into everyday time:
    //Serial.print("Unix time = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;
    // print Unix time:
    //Serial.println(epoch);

    epoch=epoch+19800;
    // print the hour, minute and second:
    Serial.print("The IST time is ");       // UTC is the time at Greenwich Meridian (GMT)
    Serial.print(((epoch  % 86400L) / 3600)); // print the hour (86400 equals secs per day)
    Serial.print(':');
    if ( ((epoch % 3600) / 60) < 10 ) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
    Serial.print(':');
    if ( (epoch % 60) < 10 ) {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.println(epoch % 60); // print the second
    hours = (epoch  % 86400L) / 3600;
    minutes = (epoch  % 3600) / 60;
    seconds = epoch % 60;
    if (minutes<10){
      timet = String(hours)+"0"+ String(minutes);
    }
    else{
      timet = String(hours)+String(minutes); 
    }
    //Serial.println(hours);
    //Serial.println(minutes);
    timett= timet.toInt();
    Serial.println(timett);
    if (timett >= 700 && timett <= 1829){
       Serial.println("OFF");
        digitalWrite(light, HIGH);
        if (counter ==1){
          Serial.println("Counter One");
          bot.sendMessage(defaultChatId, "Porch light switch OFF at "+ String(hours) +":"+ String(minutes) +":"+ String(seconds));
          counter =0;
      }
    }
    if ((timett >= 1830 && timett <= 2359) || (timett >= 0000 && timett <= 659)){
       digitalWrite(light, LOW);
       Serial.println("ON");
       if (counter == 0){
         Serial.println("Counter Zero");
         bot.sendMessage(defaultChatId, "Porch light switch ON at "+ String(hours) +":"+ String(minutes) +":"+ String(seconds));
         counter =1;
       }
      
    }
    /*
    if ((18 <= hours && hours <= 23 or 00 <= hours && hours <6) and (00 <= minutes && minutes <= 59)){
      //if (hours==18 and minutes > 30){
        digitalWrite(light, LOW);
        Serial.println("ON");
        if (counter == 0){
         Serial.println("Counter Zero");
         bot.sendMessage(defaultChatId, "Porch light switch ON at "+ String(hours) +":"+ String(minutes) +":"+ String(seconds));
         counter =1;
       }
     }
   }
    if ((6 <= hours && hours <= 17) and (00 <= minutes && minutes <= 59)){
      //if (hours==6 and minutes>30){
        Serial.println("OFF");
        digitalWrite(light, HIGH);
        if (counter ==1){
          Serial.println("Counter One")
          bot.sendMessage(defaultChatId, "Porch light switch OFF at "+ String(hours) +":"+ String(minutes) +":"+ String(seconds));
          counter =0;
      //}
    }
  }*/
 }
  //digitalWrite(statusled, HIGH);
  //delay(500);
  //digitalWrite(statusled, LOW);
  // wait ten seconds before asking for the time again
  delay(59500);
}

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}


