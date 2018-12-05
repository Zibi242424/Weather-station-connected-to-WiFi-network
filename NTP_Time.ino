/********************
- www.geekstips.com
- Arduino Time Sync from NTP Server using ESP8266 WiFi module 
- Arduino code example
 ********************/

#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <WiFiUdp.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>
#include <ESP8266WiFiMulti.h>   // Include the Wi-Fi-Multi library
#include <ESP8266mDNS.h>        // Include the mDNS library

#define BMP_SCK  (13)
#define BMP_MISO (12)
#define BMP_MOSI (11)
#define BMP_CS   (10)

Adafruit_BMP280 bmp; // I2C
//Adafruit_BMP280 bmp(BMP_CS); // hardware SPI
//Adafruit_BMP280 bmp(BMP_CS, BMP_MOSI, BMP_MISO,  BMP_SCK);


char ssid[] = "dzikieWenze";  //  your network SSID (name)
char pass[] = "fihejook";       // your network password

int SetRead = 0;

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

unsigned int Day;
unsigned int Month;
unsigned int Year;

unsigned int Hour;
unsigned int Minute;
unsigned int Second;

unsigned int Synchronizer = 60;

float Temperature;
unsigned int Pressure;

unsigned int localPort = 2390;      // local port to listen for UDP packets

/* Don't hardwire the IP address or we won't get the benefits of the pool.
 *  Lookup the IP address for the host name instead */
//IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server
IPAddress timeServerIP; // time.nist.gov NTP server address
const char* ntpServerName = "time.coi.pw.edu.pl";

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

String output5State = "off";
String output4State = "off";

int TimeHours = 0;
int TimeMinutes = 0;
int TimeSeconds = 0;

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

bool ConnectedToWiFi = true;
bool ConnectedToBMP280 = true;

ESP8266WiFiMulti wifiMulti;     // Create an instance of the ESP8266WiFiMulti class, called 'wifiMulti'



// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;
Ticker blinker;

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);

  wifiMulti.addAP("dzikieWenze", "fihejook");   // add Wi-Fi networks you want to connect to
  wifiMulti.addAP("AndroidAP", "xkrg7266");
  wifiMulti.addAP("SzymeNetwork", "M@n19sel09");
  
  blinker.attach(20, changeState);
  
  
  Serial.begin(9600);
  Serial.println();
  Serial.println();
  if (!bmp.begin()) {
    bool ConnectedToBMP280 = false;
    Serial.print("n");
    while (1);
  }

  /* If connected to BMP280 sensor send confirmation via UART */
  if (ConnectedToBMP280 != false){
    Serial.print("b");
  }
   /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

  /* Connecting to the WiFi network */
  /*WiFi.begin(ssid, pass);
  
  while (WiFi.status() != WL_CONNECTED) {
    int x = 1;
    delay(500);
    x++;
    if(x == 100){
      ConnectedToWiFi = false;
      Serial.print("n");
      break;
    }
  }*/
  
  /* If connected to WiFi sensor send confirmation via UART */
  /*if (ConnectedToWiFi != false){
    Serial.print("w");
  }*/
  int x = 1;
  while (wifiMulti.run() != WL_CONNECTED) { // Wait for the Wi-Fi to connect: scan for Wi-Fi networks, and connect to the strongest of the networks above
    delay(100);
    x++;
    if(x == 100){
      ConnectedToWiFi = false;
      Serial.print("n");
      break;
    }
    
  }
  if(wifiMulti.run() == WL_CONNECTED){
    ConnectedToWiFi = true;
    Serial.print('w');
  }
  udp.begin(localPort);
  server.begin();
  if (!MDNS.begin("WeatherStation")) {             // Start the mDNS responder for esp8266.local
    Serial.print("j");
  }else{
    Serial.print("i");
  }
  delay(5000);
  Synchronizer = 40;
}

void loop()
{

  if (Synchronizer == 40){
   if(wifiMulti.run() == WL_CONNECTED){
    ConnectedToWiFi = true;
    Serial.print('w');
  }
    digitalWrite(LED_BUILTIN, LOW); 
    //get a random server from the pool
    WiFi.hostByName(ntpServerName, timeServerIP); 

    sendNTPpacket(timeServerIP); // send an NTP packet to a time server
    // wait to see if a reply is available
    delay(1000);
  
    int cb = udp.parsePacket();
    if (!cb) {
      Serial.print("n");
    }
    else {
      /* Packet received, read the data from it */
      udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

      // the timestamp starts at byte 40 of the received packet and is four bytes,
      // or two words, long. First, esxtract the two words:

      unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
      unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
      // combine the four bytes (two words) into a long integer
      // this is NTP time (seconds since Jan 1 1900):
      unsigned long secsSince1900 = highWord << 16 | lowWord;

      // now convert NTP time into everyday time:
      // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
      const unsigned long seventyYears = 2208988800UL;
      // subtract seventy years:
      unsigned long epoch = secsSince1900 - seventyYears;


      if(((epoch  % 86400L) / 3600+1) < 24){
        Hour = (epoch  % 86400L) / 3600 + 1;
      }else{
        Hour = 0;
      }    
      Minute = ((epoch % 3600) / 60);
      Second = (epoch % 60);
      Serial.println();
      Serial.print("h");Serial.print((char)Hour);
      Serial.print((char)Minute);
      Serial.print((char)Second);
      Serial.println();
      digitalWrite(LED_BUILTIN, HIGH);
    }
     
  }
  // wait ten seconds before asking for the time again
  Serial.print("b");
  Temperature = bmp.readTemperature();
  Serial.println(bmp.readPressure());
  Pressure = (bmp.readPressure()/100);  
  int PressureHIGH = Pressure/100;
  int PressureLOW = Pressure%100;
  
  
  

  Serial.print("t");Serial.print((char)((int)Temperature));
  //Serial.print("f");
  Serial.println((char)(((int)(Temperature*100.00))%100));

  Serial.print("p");Serial.print((char)PressureHIGH);
  Serial.print((char)PressureLOW);
  Serial.println(PressureHIGH);
  Serial.println(PressureLOW);
  if(Synchronizer == 40){
    Synchronizer = 0;
  }else if (Synchronizer < 40){
    Synchronizer++;
  }else{
    Synchronizer = 0;
  }
  delay(500);
  //ClientHandler();
 

  
}

void ClientHandler(){
  Serial.print("$");
  WiFiClient client = server.available();   // Listen for incoming clients
  
  if (client) {                             // If a new client connects,
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        //Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
           
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>Zibi WeatherStation 2018</h1>");
            
            // Display current state, and ON/OFF buttons for GPIO 5  
            client.println("<p>Temperatura: ");client.println(Temperature);client.println("C</p>");            
            client.println("<p>Ciśnienie: ");client.println(Pressure);client.println("hPa</p>");
               
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Synchronizer = 40;
  }
 
  Serial.print("^");
   //ESP.restart();
}

void changeState(){
  digitalWrite(LED_BUILTIN, HIGH); 
}



// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
  //Serial.println("sending NTP packet...");
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
