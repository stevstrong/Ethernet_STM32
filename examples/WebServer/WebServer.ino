/*
  Web Server
 
 A simple web server that shows the value of the analog input pins.
 using an Arduino Wiznet Ethernet shield. 
 
 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * Analog inputs attached to pins A0 through A5 (optional)
 
 created 18 Dec 2009
 by David A. Mellis
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

#include <SPI.h>
#include <Ethernet_STM32.h>
#include <Streaming.h>

#define LED_PIN PC13

const uint8_t ETHERNET_SPI_CS_PIN = PB12;

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network.
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

IPAddress ip(192,168,100, 10); //(192,168,1, 177);


// Initialize the Ethernet server library
// with the IP address and port you want to use 
// (port 80 is default for HTTP):
EthernetServer server(80);
EthernetClient client;

// A UDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

/*****************************************************************************/
void setup()
{
	// Open serial communications and wait for port to open:
	Serial.begin(115200);
	while ( !Serial ); delay(100);

	Serial.println(("*************************************************************"));
	Serial.println(("***** Web server example combined with getting NTP time *****"));
	Serial.println(("*************************************************************"));

	SPI.setModule(2);
	// init Ethernet interface
	Ethernet.init(SPI, ETHERNET_SPI_CS_PIN);

	Serial.print("\nGetting IP address using DHCP...");
	if ( Ethernet.begin(mac)==0 ) {
		Serial.println(("failed."));
		Serial.print(" Setting static IP address...");
		Ethernet.begin(mac, ip);
	}
	Serial.println("done.");

	server.begin();
	Serial << ("Server is at ") << Ethernet.localIP() << endl << endl;
}

/*****************************************************************************/
void loop()
{
	// listen for incoming clients
	ListenForClient();
	// give the web browser time to receive the data
	delay(1);
}

uint32_t time;
uint8_t hour, minute, second;

/*****************************************************************************/
void ListenForClient(void)
{
	client = server.available();
	if ( !client ) return;
  
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;

    if (client.connected()) {

		GetNTPTime();

		Serial.println("\n--> Client connected:");
		while (client.available()) {
			char c = client.read();
			//Serial.write(c);
			// if you've gotten to the end of the line (received a newline
			// character) and the line is blank, the http request has ended,
			// so you can send a reply
			if (c == '\n' && currentLineIsBlank) {
				Serial.print("sending standard response to the client in ");
				time = millis();
				// send a standard http response header
				client.println("HTTP/1.1 200 OK");
				client.println("Content-Type: text/html");
				client.println("Connection: close");  // the connection will be closed after completion of the response
				client.println("Refresh: 5");  // refresh the page automatically every 5 sec
				client.println();
				client << ("<!DOCTYPE HTML>\n<html>\n");
				client << ("The UTC time is ") << _TIME(hour, minute, second) << ("<br />\n<br />\n");
				// output the value of each analog input pin
				for (int analogChannel = 0; analogChannel < 6; analogChannel++) {
					int sensorReading = analogRead(analogChannel);
					client << ("analog input ") << (analogChannel) << (" is ")<< (sensorReading) << ("<br />\n");
				}
				client.println("</html>");
				Serial.println(millis()-time);
				break;
			}
			if (c == '\n') {
				// you're starting a new line
				currentLineIsBlank = true;
			}
			else if (c != '\r') {
				// you've gotten a character on the current line
				currentLineIsBlank = false;
			}
		}
		// close the connection:
		client.stop();
		Serial.println("<-- client disconnected.\n");
   }
}

/*****************************************************************************/
const int NTP_PACKET_SIZE= 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets 
//IPAddress timeServer(132, 163, 4, 101); // time-a.timefreq.bldrdoc.gov NTP server
IPAddress timeServer(192, 168, 100, 1); // local router/modem
/*****************************************************************************/
void GetNTPTime(void)
{
	unsigned int localPort = 8888;      // local port to listen for UDP packets
	Udp.begin(localPort);

    Serial.println("Sending NTP packet...");
	sendNTPpacket(timeServer); // send an NTP packet to a time server

	// wait to see if a reply is available
	while ( !Udp.parsePacket() );
	// We've received a packet, read the data from it
	Udp.read(packetBuffer,NTP_PACKET_SIZE);  // read the packet into the buffer
	Udp.stop();

	//the timestamp starts at byte 40 of the received packet and is four bytes,
	// or two words, long. First, esxtract the two words:

	unsigned long highWord = (packetBuffer[40]<<8) | packetBuffer[41];
	unsigned long lowWord = (packetBuffer[42]<<8) | packetBuffer[43];  
	// combine the four bytes (two words) into a long integer
	// this is NTP time (seconds since Jan 1 1900):
	unsigned long secsSince1900 = (highWord << 16) | lowWord;  
	Serial.print("Seconds since Jan 1 1900 = " );
	Serial.println(secsSince1900);               

	// now convert NTP time into everyday time:
	// Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
	const unsigned long seventyYears = 2208988800UL;     
	// subtract seventy years:
	unsigned long epoch = secsSince1900 - seventyYears;  
	// print Unix time:
	Serial << ("Unix time = ") << (epoch) << endl;
	// print the hour, minute and second:
    // UTC is the time at Greenwich Meridian (GMT)
	hour = (epoch  % 86400L) / 3600;
	minute = (epoch % 3600) / 60;
	second = (epoch % 60);
	Serial << ("The UTC time is ") << _TIME(hour, minute, second) << endl;
}

/*****************************************************************************/
// send an NTP request to the time server at the given address 
/*****************************************************************************/
void sendNTPpacket(IPAddress& address)
{
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
	Udp.beginPacket(address, 123); //NTP requests are to port 123
	Udp.write(packetBuffer,NTP_PACKET_SIZE);
	Udp.endPacket(); 
}

