/*
  Web client
 This sketch sends an event to Initial State (http://insecure-groker.initialstate.com)
 using an Arduino Wiznet Ethernet shield.
 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 */

#include <SPI.h>
#include <Ethernet.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 7
#define LED 5

float temperature = 0;

String httpString = "GET /api/events?accessKey=V4iVTL0C9Mh5m5gMPP7XLwwF4y3mouYI&bucketKey=EF67ENPARL5F";
String httpEnd = " HTTP/1.1";

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// if you don't want to use DNS (and reduce your sketch size) use the numeric IP instead of the name for the server:
//IPAddress server(74,125,232,128);  // numeric IP for Google (no DNS)
char server[] = "insecure-groker.initialstate.com";    // name address for Google (using DNS)

// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 1, 149);
byte gateway[] = { 192, 168,1,254};
byte subnet[] = { 255, 255, 255, 0 };

// Initialize the Ethernet client library with the IP address and port of the server that you want to connect to (port 80 is default for HTTP):
EthernetClient client;

OneWire ds(ONE_WIRE_BUS);
DallasTemperature sensors(&ds); // Pass our oneWire reference to Dallas Temperature.

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
//  while (!Serial) {
//    ; // wait for serial port to connect. Needed for native USB port only
//  }

  sensors.begin();   // Start up the ds library 
  pinMode(LED, OUTPUT);
}

void loop() {
 digitalWrite(LED,1);
 String readingHttp ="";

  sensors.requestTemperatures(); // Send the command to get temperatures
  delay(500);
  temperature = sensors.getTempCByIndex(0);

  readingHttp += "&Temperature=" + String(temperature);

  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip, gateway, subnet);
  }

  digitalWrite(LED,0);

  delay(1000);    // give the Ethernet shield a second to initialize:
  Serial.println("connecting...");
  if (client.connect(server, 80))
  {
    Serial.println("connected");
    //client.println("GET /api/events?accessKey=V4iVTL0C9Mh5m5gMPP7XLwwF4y3mouYI&bucketKey=EF67ENPARL5F&rasw=99.9 HTTP/1.1");
    client.println(httpString + readingHttp + httpEnd);
    client.println("Host: insecure-groker.initialstate.com");
    //client.println("Connection: close");
    client.println();
   }
   else
   {
      Serial.println("connection failed"); // if you didn't get a connection to the server:
   }

  // if there are incoming bytes available from the server, read them and print them:
  if (client.available()) {
    char c = client.read();
    Serial.println(c);
  }

  // if the server's disconnected, stop the client:
  if (!client.connected())
  {
    Serial.println("disconnecting.");
    client.stop();
  }

    //Serial.println(httpString + readingHttp + httpEnd);
    client.stop();

    delay(8000);   // wait
}

float GetTemperatures()
{
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius;
  float fahrenheit;

  if ( !ds.search(addr)) {
    //Serial.println("No more addresses.");
    //Serial.println();
    ds.reset_search();
    delay(250);
    return -100.00;
  }

//  Serial.print("ROM =");
//  for( i = 0; i < 8; i++) {
//    Serial.write(' ');
//    Serial.print(addr[i], HEX);
//  }

  if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return 888;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 0);        // start conversion, with parasite power off at the end (1 for ON)

  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.

  present = ds.reset();
  ds.select(addr);   
  ds.write(0xBE);         // Read Scratchpad

  //Serial.print("  Data = ");
  //Serial.print(present, HEX);
  //Serial.print(" ");
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
    //Serial.print(data[i], HEX);
    //Serial.print(" ");
  }
  //Serial.print(" CRC=");
  //Serial.print(OneWire::crc8(data, 8), HEX);
  //Serial.println();

  // Convert the data to actual temperature because the result is a 16 bit signed integer,
  // it should be stored to an "int16_t" type, which is always 16 bits even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
  //fahrenheit = celsius * 1.8 + 32.0;
   //Serial.println(celsius);
   return celsius;
}

