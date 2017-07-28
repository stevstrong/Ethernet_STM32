W5500 Ethernet library for STM32F1 and STMF4 micro-controllers
----

That library has been ported from the official Arduino Ethernet library.
As reference was also used: 

This library runs **only on the STM32F1 and F4** family micro-controllers!

The library supports **only W5500** Ethernet controllers, and uses 8 sockets.
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
and to define the used chip select pin

`SPIClass mSpi(1); // you can use 1..2 for F1, 1..3 for F4)`

`const uint8_t ETH_CS_PIN = PA4; // set here the chip select port pin`

3. Then you need to call the following function in setup() before Ethernet.begin():

`  Ethernet.init(mSpi, ETH_CS_PIN); // SPI class object, chip select pin`

This will start the SPI transaction at maximum supported speed (36MHz on SPI1 for F3, 42MHz for F4).

4. The rest of functions / commands have the same syntax with the stock Arduino Ethernet library.

Examples how to use the Ethernet_STM32 library
----

