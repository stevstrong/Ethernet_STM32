/*

 Udp NTP Client
 
 Get the time from a Network Time Protocol (NTP) time server
 Demonstrates use of UDP sendPacket and ReceivePacket 
 For more on NTP time servers and the messages needed to communicate with them, 
 see http://en.wikipedia.org/wiki/Network_Time_Protocol
 
 Warning: NTP Servers are subject to temporary failure or IP address change.
 Plese check 

    http://tf.nist.gov/tf-cgi/servers.cgi

 if the time server used in the example didn't work.

 created 4 Sep 2010 by Michael Margolis
 modified 9 Apr 2012 by Tom Igoe
 modified 12 Aug 2013 by Soohwan Kim
 modified 18 Aug 2015 by Vassilis Serasidis
 modified 16 July 2017 by stevestrong
 
 =========================================================
 Ported to STM32F103 on 18 Aug 2015 by Vassilis Serasidis

 <---- Pinout ---->
 W5x00 <--> STM32F103
 SS    <-->  PA4 <-->  BOARD_SPI1_NSS_PIN
 SCK   <-->  PA5 <-->  BOARD_SPI1_SCK_PIN
 MISO  <-->  PA6 <-->  BOARD_SPI1_MISO_PIN
 MOSI  <-->  PA7 <-->  BOARD_SPI1_MOSI_PIN
 =========================================================
 
 
 This code is in the public domain.

 */

#include <SPI.h>         
#include <Ethernet_STM32.h>

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
#if defined(WIZ550io_WITH_MACADDRESS) // Use assigned MAC address of WIZ550io
;
#else
//byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
byte mac[] = { 0x10,0x69,0x69,0x2D,0x30,0x41 };
#endif  

#define LED_PIN PC13

#define ETHERNET_SPI_CS_PIN PA4
#define ETHERNET_RESET_PIN	PA3

unsigned int localPort = 8888;      // local port to listen for UDP packets

IPAddress timeServer(192, 168, 100, 1); // local modem
//IPAddress timeServer(132, 163, 4, 101); // time-a.timefreq.bldrdoc.gov NTP server
// IPAddress timeServer(132, 163, 4, 102); // time-b.timefreq.bldrdoc.gov NTP server
// IPAddress timeServer(132, 163, 4, 103); // time-c.timefreq.bldrdoc.gov NTP server

const int NTP_PACKET_SIZE= 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets 

// A UDP instance to let us send and receive packets over UDP
EthernetUDP Udp;
uint8_t time;
//-----------------------------------------------------------------------------
void yield(const char * str)
{
	Serial.println(str);
	delay(10);
}
//-----------------------------------------------------------------------------
void setup() 
{
 // Open serial communications and wait for port to open:
  Serial.begin(115200);

  while ( !Serial ) ;

	delay(1000);
    Serial.println("UdpNtpClient example - configure Ethernet using DHCP");

	Ethernet.init(ETHERNET_SPI_CS_PIN);
	//uint8_t ver = Ethernet.init(&SPI, ETHERNET_SPI_CS_PIN, ETHERNET_RESET_PIN, yield);
	//Serial.print("\nChip version: "); Serial.println(ver);

	// start Ethernet and UDP
#if defined(WIZ550io_WITH_MACADDRESS)
  if (Ethernet.begin() == 0) {
#else
    Serial.print("Ethernet.begin(mac)...");
  if (Ethernet.begin(mac) == 0) {
#endif  
    Serial.println("failed!");
    // no point in carrying on, so do nothing forevermore:
	pinMode(LED_PIN, OUTPUT);
    while(1) {
		digitalWrite(LED_PIN, LOW);
		delay(125);
		digitalWrite(LED_PIN, HIGH);
		delay(125);
	}
  }
  Serial.println("done.\n");
  Udp.begin(localPort);
  time = ((millis()/1000)%10); // take the second
}

//-----------------------------------------------------------------------------
void loop()
{
	if ( time != ((millis()/1000)%10) ) {
		time = ((millis()/1000)%10);
		Serial.print(time); Serial.write('.');
		if (time) return; // do it only each 10 seconds
	} else
		return;
    Serial.println("\nSending NTP packet...");

	sendNTPpacket(timeServer); // send an NTP packet to a time server

    // wait to see if a reply is available
	Serial.print("Waiting for response");
	int ret;
	do {
		Serial.write('.'); delay(100);
	} while ( !Udp.parsePacket() );

	Serial.write('\n');

    // We've received a packet, read the data from it
    Udp.read(packetBuffer,NTP_PACKET_SIZE);  // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    uint16_t highWord = (uint16_t)(packetBuffer[40]<<8) | packetBuffer[41];
    uint16_t lowWord = (uint16_t)(packetBuffer[42]<<8) | packetBuffer[43];  
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1970):
    unsigned long secsSince1900 = highWord << 16 | lowWord;  
    Serial.print("Seconds since Jan 1 1900 = " );
    Serial.println(secsSince1900);               

    // now convert NTP time into everyday time:
    Serial.print("Unix time = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;     
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;  
    // print Unix time:
    Serial.println(epoch);                               

	// adjust to local time: add 2 more hours
	epoch += 2*60*60;

    // print the hour, minute and second:
    Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
    Serial.print((epoch  % 86400L) / 3600); // print the hour (86400 equals secs per day)
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
    Serial.println(epoch %60); // print the second
}

// send an NTP request to the time server at the given address 
unsigned long sendNTPpacket(IPAddress& address)
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










