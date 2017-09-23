/*
 modified 12 Aug 2013
 by Soohwan Kim (suhwan@wiznet.co.kr)
*/
#ifndef ETHERNET_STM32_H
#define ETHERNET_STM32_H


#include <IPAddress.h>
#include "EthernetClient.h"
#include "EthernetServer.h"
#include "EthernetUdp.h"


class EthernetClass {
private:
  IPAddress _dnsServerAddress;
public:
  static uint8_t _state[MAX_SOCK_NUM];
  static uint16_t server_port[MAX_SOCK_NUM];
  void init(SPIClass & spi, uint8_t cs_pin) { socket.init(spi, cs_pin); }
  void init(uint8_t cs_pin) { init(SPI, cs_pin); }
  // Initialize the Ethernet shield to use the provided MAC address and gain the rest of the
  // configuration through DHCP.
  // Returns 0 if the DHCP configuration failed, and 1 if it succeeded
  int begin(uint8_t *mac_address, unsigned long timeout = 60000, unsigned long responseTimeout = 4000);
  void begin(uint8_t *mac_address, IPAddress local_ip);
  void begin(uint8_t *mac_address, IPAddress local_ip, IPAddress dns_server);
  void begin(uint8_t *mac_address, IPAddress local_ip, IPAddress dns_server, IPAddress gateway);
  void begin(uint8_t *mac_address, IPAddress local_ip, IPAddress dns_server, IPAddress gateway, IPAddress subnet);
  int maintain();

  IPAddress localIP();
  IPAddress subnetMask();
  IPAddress gatewayIP();
  IPAddress dnsServerIP();

  friend class EthernetClient;
  friend class EthernetServer;
};

extern EthernetClass Ethernet;

#endif
