W5500 Ethernet library for STM32F1 and STMF4 micro-controllers
----

That library has been ported from the official Arduino Ethernet library.
As reference was also used: 

This library runs **only on the STM32F1 and F4** family micro-controllers!

The library supports **only W5500** Ethernet controllers, and uses 8 sockets.

A possible W5500-based module:
https://www.aliexpress.com/item/Free-shipping-W5500-Ethernet-network-module-hardware-TCP-IP-51-STM32-microcontroller-program-over-W5100/32505484781.html?spm=a2g0s.9042311.0.0.pWrBlQ

https://ae01.alicdn.com/kf/HTB1_UFSKXXXXXXIXFXXq6xXFXXXd/W5500-Ethernet-network-module-hardware-TCP-IP-51-STM32-microcontroller-program-over-W5100.jpg

Library installation
----

* Unzip the file **Ethernet_STM32-master.zip** into your Arduino IDE libraries directory 

> arduino/libraries

and rename the folder **Ethernet_STM32-master** to **Ethernet_STM32**


Using the Ethernet_STM32 library
----
1. Include this library in your sketch  

`#include <Ethernet_STM32.h>`

2. The library needs to know the SPI port on which you connected the W5500 chip, and the corresponding chip select pin.
Therefore you have to declare an SPI class object with appropriate SPI port number (1..2 for F1, 1..3 for F4)

`SPIClass mSpi(1); // you can use 1..2 for F1, 1..3 for F4)`


3. Then you need to call the following function in setup() before Ethernet.begin():

`  Ethernet.init(mSpi, PA4); // SPI class object, chip select pin on your choice`

This will start the SPI transaction at maximum supported speed (36MHz on SPI1 for F3, 42MHz for F4).

4. The rest of functions / commands have the same syntax with the stock Arduino Ethernet library.

5. A typical application looks like following example:
```
#include <Ethernet_STM32.h>
...
SPIClass mSpi(1); // you can use 1..2 for STM32F1, 1..3 for STM32F4)
...
void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(112500);
  while (!Serial) ; // wait for serial port to connect. Needed for native USB port only
  delay(1000);

  // init interface and hardware using SPI class object and chip select pin on your choice
  Ethernet.init(SPI, PA4);

  // start the Ethernet connection:
  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip, myDns);
  } else {
    Serial.print("  DHCP assigned IP ");
    Serial.println(Ethernet.localIP());
  }
}
...
void loop()
{
  // do something
  ...
}
```
