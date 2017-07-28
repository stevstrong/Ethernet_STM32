#ifndef ETHERNETSERVER_H
#define ETHERNETSERVER_H


#include "Server.h"


class EthernetClient;

class EthernetServer : 
public Server {
private:
  uint16_t _port;
  void accept();
public:
  EthernetServer(uint16_t);
  EthernetClient available();
  virtual void begin();
  inline size_t write(uint8_t b) { return write(&b, 1); }
  virtual size_t write(const uint8_t *buf, size_t size);
  using Print::write;
};

#endif
