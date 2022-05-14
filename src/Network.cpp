// Based on examples at https://github.com/vjmuzik/NativeEthernet

#include <SPI.h>
#include "Network.h"
#include "Debug.h"
#include "filters.h"


// Enter a MAC address for your controller below.
byte mac[6]; // = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 1, 123);
IPAddress myDns(192, 168, 1, 1);

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;

// Variables to measure the speed
unsigned long byteCount = 0;
bool printWebData = true;  // set to false for better speed measurement

void EtnernetClass::setup() {
status = 0;
  teensyMAC(mac);

  // start the Ethernet connection:
  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    status = 99;
    Serial.println("Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      status = 98;
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
     // while (true) {
     //   delay(1); // do nothing, no point running without Ethernet hardware
     // }
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
      status = 97;

    }
    // try to congifure using IP address instead of DHCP:
 //   Ethernet.begin(mac, ip, myDns);
  } else {
    Serial.print("  DHCP assigned IP ");
    Serial.println(Ethernet.localIP());
  }
}

String EtnernetClass::localIp() {
  IPAddress address = Ethernet.localIP();
  return String(address[0]) + "." + 
        String(address[1]) + "." + 
        String(address[2]) + "." + 
        String(address[3]);Ethernet.localIP();
}


EthernetServer serverx(80);

bool EtnernetClass::loop() {
  if (status != 0) {
    return false;
  }
  EthernetClient client = serverx.available();
 if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected() && client.available()) {
     // if (client.available()) {
        char c = client.read();
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {

          float milliVolt = V_FILTERS.mean;
          client.println(milliVolt,4);

          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
     // }
    }
     delay(1);
    // close the connection:
    client.stop();
    //Serial.println("client disconnected");
    }
}

void EtnernetClass::teensyMAC(uint8_t *mac)
{
for(uint8_t by=0; by<2; by++) mac[by]=(HW_OCOTP_MAC1 >> ((1-by)*8)) & 0xFF;
for(uint8_t by=0; by<4; by++) mac[by+2]=(HW_OCOTP_MAC0 >> ((3-by)*8)) & 0xFF;
Serial.printf("MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}
 


EtnernetClass ETHERNET_UTIL;
