/*
 DHCP Chat  Server
 
 A simple server that distributes any incoming messages to all
 connected clients.  To use telnet to  your device's IP address and type.
 You can see the client's input in the serial monitor as well.
 Using an Arduino Wiznet Ethernet shield. 
 
 THis version attempts to get an IP address using DHCP
 
 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 
 Based on ChatServer example by David A. Mellis
 created 21 May 2011
 modified 9 Apr 2012
 by Tom Igoe
 modified 12 Aug 2013
 by Soohwan Kim
 Modified 18 Aug 2015
 by Vassilis Serasidis
 
 =========================================================
 Ported to STM32F103 on 18 Aug 2015 by Vassilis Serasidis

 <---- Pinout ---->
 W5x00 <--> STM32F103
 SS    <-->  PA4 <-->  BOARD_SPI1_NSS_PIN
 SCK   <-->  PA5 <-->  BOARD_SPI1_SCK_PIN
 MISO  <-->  PA6 <-->  BOARD_SPI1_MISO_PIN
 MOSI  <-->  PA7 <-->  BOARD_SPI1_MOSI_PIN
 =========================================================
 
 */

#include <Ethernet_STM32.h>


#define LED_PIN PC13

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network.
// gateway and subnet are optional:
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

IPAddress ip(192,168,100, 1); //(192,168,1, 177);
IPAddress gateway(192,168,100, 1); //(192,168,1, 1);
IPAddress subnet(255, 255, 0, 0);

// telnet defaults to port 23
EthernetServer server(23);
EthernetClient client;

boolean gotAMessage = false; // whether or not you got a message from the client yet
char s[100];
uint8_t i;

//-----------------------------------------------------------------------------
void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  // this check is only needed on the Leonardo:
  while (!Serial) ;

  delay(1000);
  Serial.println("DHCP chat server example - get an IP address using DHCP");

	Ethernet.init(SPI, PA4);
	Serial.print("\nDetected chip: W"); Serial.println(Ethernet.device());

  // start the Ethernet connection:
  Serial.print("Ethernet.begin(mac)...");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("failed!");
    // initialize the ethernet device not using DHCP:
    Ethernet.begin(mac, ip, gateway, subnet);
  }
  // print your local IP address:
  Serial.print("done.\nMy IP address: ");
  Serial.println(Ethernet.localIP());
  // start listening for clients
  Serial.println("\nStart listening for clients...");
  server.begin();
  i = 0;
}

//-----------------------------------------------------------------------------
void loop() {
  // wait for a new client:
  client = server.available();

  // when the client sends the first byte, say hello:
  if (client) {
    if (!gotAMessage) {
      Serial.println("We have a new client");
      client.println("Hello, client!"); 
      gotAMessage = true;
    }

    // read the bytes incoming from the client:
    char thisChar = client.read();
	if (thisChar=='\n') {
		s[i] = 0;
		i = 0;
		server.print("Received from server: ");
		server.print(s);
	} else if (i<sizeof(s)) s[i++] = thisChar;
    // echo the bytes back to the client:
    //server.write(thisChar);
    // echo the bytes to the server as well:
    Serial.print(thisChar);
  }
}

