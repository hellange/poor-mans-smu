/*
 DMM6500 API read
 This sketch connects to a DMM6500
 using an a W5500 based ethernet board connnected to a Teensy 3.2.
 Based on the DMM6500 ajax_proc (undocumented?) API.
 Just a first hack to see if it`s possible to read data from DMM6500 though its ethernet port.
 
 created 25 Mar 2020 by Helge Langehaug.
 
 Crude adaption of original WebClient (ref Ethernet library) work
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe, based on work by Adrian McEwen
 */

#include <SPI.h>
#include <NativeEthernet.h>
#include "ethernet.h"

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
IPAddress server(192,168,0,100);  // numeric IP for your DMM6500  (original file was Google (no DNS)
//char server[] = "www.google.com";    // NOT RELEVANT FOR DMM6500 test. Original comment: name address for Google (using DNS)

// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 0, 177);
IPAddress myDns(192, 168, 0, 1);

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;

// Variables to measure the speed
unsigned long beginMicros, endMicros;
unsigned long byteCount = 0;
bool printWebData = true;  // set to false for better speed measurement


void EtnernetClass::httpRequest() {
byteCount = 0;
beginMicros = micros();
   // if you get a connection, report back via serial:
  if (client.connect(server, 80)) {
      return;
    Serial.print("connected to ");
    Serial.println(client.remoteIP());
    // Make a HTTP request:
    //client.println("GET /search?q=arduino HTTP/1.1");
    client.println("POST /ajax_proc");
    
    client.println("Host: 192.168.0.100");
    client.println("Content-Type: text/plain");
    client.println("Content-length: 28");
    client.println("Connection: close");
    client.println();
    client.println("function=1&command=:MEASure?");
    //client.flush();
    //client.stop();
  
  } else {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
  }

  
   
}
void EtnernetClass::setup() {
  // You can use Ethernet.init(pin) to configure the CS pin
  //Ethernet.init(10);  // Most Arduino shields
  //Ethernet.init(5);   // MKR ETH shield
  //Ethernet.init(0);   // Teensy 2.0
  //Ethernet.init(20);  // Teensy++ 2.0
  //Ethernet.init(15);  // ESP8266 with Adafruit Featherwing Ethernet
  //Ethernet.init(33);  // ESP32 with Adafruit Featherwing Ethernet

  // Open serial communications and wait for port to open:
  
  
  //Serial.begin(9600);
  //while (!Serial) {
  //  ; // wait for serial port to connect. Needed for native USB port only
  //}

  // start the Ethernet connection:
  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    return;
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
      while (true) {
        delay(1); // do nothing, no point running without Ethernet hardware
      }
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip, myDns);
  } else {
      return;
    Serial.print("  DHCP assigned IP ");
    Serial.println(Ethernet.localIP());
  }
  
  // give the Ethernet shield a second to initialize:
  delay(1000);
  Serial.print("connecting to ");
  Serial.print(server);
  Serial.println("...");

  
}

void EtnernetClass::loop() {
  //delay(1000);
  // if there are incoming bytes available
  // from the server, read them and print them:
  int len = client.available();
  if (len > 0) {
    byte buffer[80];
    if (len > 80) len = 80;
    client.read(buffer, len);
    if (printWebData) {
      Serial.write(buffer, len); // show in the serial monitor (slows some boards)
    }
    byteCount = byteCount + len;
  }

  

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    endMicros = micros();
    Serial.println();
    //Serial.println("disconnecting.");
    //client.stop();
    Serial.print("Received ");
    Serial.print(byteCount);
    Serial.print(" bytes in ");
    float seconds = (float)(endMicros - beginMicros) / 1000000.0;
    Serial.print(seconds, 4);
    float rate = (float)byteCount / seconds / 1000.0;
    Serial.print(", rate = ");
    Serial.print(rate);
    Serial.print(" kbytes/second");
    Serial.println();
    httpRequest();
     
     
    
    
    // do nothing forevermore:
    //while (true) {
    //  delay(1);
   // }
  } 
  
}


EtnernetClass ETHERNET_UTIL;
