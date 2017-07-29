/*
  Web client

 This sketch connects to a website (http://www.google.com)
 using an W5500 Wiznet Ethernet module.
 */

#include <SPI.h>
#include <Ethernet_STM32.h>

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(74,125,232,128);  // numeric IP for Google (no DNS)
//char server[] = "www.google.com";    // name address for Google (using DNS)
char server[] = "www.serasidis.gr";    // name address for Serasidis.gr (using DNS)

// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 0, 177);
IPAddress myDns(192, 168, 0, 1);

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;

uint32_t beginMicros, endMicros;
int32_t byteCount;

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) ; // wait for serial port to connect. Needed for native USB port only
  delay(1000);

  // initialise interface and hardware
  Ethernet.init(SPI, PA4); // SPI object, chip select pin

  // start the Ethernet connection:
  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    // try to configure using IP address instead of DHCP:
    Ethernet.begin(mac, ip, myDns);
  } else {
    Serial.print("  DHCP assigned IP ");
    Serial.println(Ethernet.localIP());
  }
  byteCount = -1;
}

void connect_to_server()
{
	if ( byteCount!=-1 ) return; // only send request after finished the reception
  Serial.print("********************************************************\n");
  Serial.print("press any key to send request to the server\n");
	while ( !Serial.available() ) ; // wait for any input from serial
	while ( Serial.available() ) Serial.read(); // read all Rx bytes
  Serial.print("connecting to ");
  Serial.print(server);
  Serial.println("...");

  // if you get a connection, report back via serial:
  if (client.connect(server, 80)) {
    Serial.println("connected. Sending request...");
    // Make a HTTP request:
    //client.println("GET /search?q=arduino HTTP/1.1"); // for Google
    client.println("GET / HTTP/1.1");
    client.print("Host: "); client.println(server);
    client.println("Connection: close");
    client.println();
    beginMicros = micros();
    byteCount = 0;
  } else {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
  }
}

#define BUFF_SIZE 1024
static byte buffer[BUFF_SIZE];

void check_client()
{
  if (byteCount<0) return;
  // if there are incoming bytes available
  // from the server, read them and print them:
  uint16_t len;
  while ( (len=client.available())>0 ) {
    if (len > BUFF_SIZE) len = BUFF_SIZE;
	//Serial.print("-> received "); Serial.println(len); //Serial.print(" bytes in "); Serial.println((float)(endMicros-beginMicros) / 1000000.0);
    client.read(buffer, len);
    Serial.write(buffer, len); // show in the serial monitor (slows some boards)
    byteCount += len;
  }
  // if the server's disconnected, stop the client:
  if (byteCount>0 && !client.connected()) {
    endMicros = micros();
    Serial.println("\n disconnecting.");
    client.stop();
    Serial.print("\nReceived ");
    Serial.print(byteCount);
    Serial.print(" bytes in ");
    float seconds = (float)(endMicros - beginMicros) / 1000000.0;
    Serial.print(seconds, 4);
    float rate = (float)byteCount / seconds / 1000.0;
    Serial.print(", rate = ");
    Serial.print(rate);
    Serial.print(" kbytes/second");
    Serial.println();
    byteCount = -1;
  }
}

void loop()
{
  connect_to_server(); // send request
  check_client(); // receive reply
}
