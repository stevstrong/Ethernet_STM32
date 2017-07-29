/*
* Copyright (c) 2010 by WIZnet <support@wiznet.co.kr>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#ifndef	W5500_H_INCLUDED
#define	W5500_H_INCLUDED

#include <SPI.h>
#include "util.h"
#include "variant.h"

/** Total RAM buffer is 16 kBytes for Transmitter and 16 kBytes for receiver for 1 Socket.
 *  The Total W5500 RAM buffer is 32 kBytes (16 + 16).
 *  If you use more Sockets then the RAM buffer must be split.
 *  For example: if you use 2 Sockets then all socket must use upto 16 kBytes in total RAM.
 *  So, we have: 
 *
 *  #define MAX_SOCK_NUM 2   // Select two Sockets.
 *  #define RXBUF_SIZE   8   // The Receiver buffer size will be 8 kBytes
 *  #define TXBUF_SIZE   8   // The Transmitter buffer size will be 8 kBytes
 *
 *  In total we use (2 Sockets)*(8 kBytes) for transmitter + (2 Sockets)*(8 kBytes) for receiver = 32 kBytes.
 * 
 *  I would prefer to use only 1 Socket with 16 kBytes RAM buffer for transmitter and 16 kByte for receiver buffer.
 *
 *  #define MAX_SOCK_NUM 1   // Select only one Socket
 *  #define RXBUF_SIZE   16  // Select 16 kBytes Receiver RAM buffer
 *  #define TXBUF_SIZE   16  // Select 16 kBytes Transmitter RAM buffer
 * 
 *  (c) 02 September 2015 By Vassilis Serasidis 
 *   cleaned up and extended by stevestrong - 2017.07
 */

typedef uint8_t SOCKET;

#define MAX_SOCK_NUM 8   // Select the number of Sockets (1-8)


class SnMR {
public:
  static const uint8_t CLOSE  = 0x00;
  static const uint8_t TCP    = 0x01;
  static const uint8_t UDP    = 0x02;
  static const uint8_t IPRAW  = 0x03;
  static const uint8_t MACRAW = 0x04;
  static const uint8_t PPPOE  = 0x05;
  static const uint8_t ND     = 0x20;
  static const uint8_t MULTI  = 0x80;
};

enum SockCMD {
  Sock_OPEN      = 0x01,
  Sock_LISTEN    = 0x02,
  Sock_CONNECT   = 0x04,
  Sock_DISCON    = 0x08,
  Sock_CLOSE     = 0x10,
  Sock_SEND      = 0x20,
  Sock_SEND_MAC  = 0x21,
  Sock_SEND_KEEP = 0x22,
  Sock_RECV      = 0x40
};

class SnIR {
public:
  static const uint8_t SEND_OK = 0x10;
  static const uint8_t TIMEOUT = 0x08;
  static const uint8_t RECV    = 0x04;
  static const uint8_t DISCON  = 0x02;
  static const uint8_t CON     = 0x01;
};

class SnSR {
public:
  static const uint8_t CLOSED      = 0x00;
  static const uint8_t INIT        = 0x13;
  static const uint8_t LISTEN      = 0x14;
  static const uint8_t SYNSENT     = 0x15;
  static const uint8_t SYNRECV     = 0x16;
  static const uint8_t ESTABLISHED = 0x17;
  static const uint8_t FIN_WAIT    = 0x18;
  static const uint8_t CLOSING     = 0x1A;
  static const uint8_t TIME_WAIT   = 0x1B;
  static const uint8_t CLOSE_WAIT  = 0x1C;
  static const uint8_t LAST_ACK    = 0x1D;
  static const uint8_t UDP         = 0x22;
  static const uint8_t IPRAW       = 0x32;
  static const uint8_t MACRAW      = 0x42;
  static const uint8_t PPPOE       = 0x5F;
};

class IPPROTO {
public:
  static const uint8_t IP   = 0;
  static const uint8_t ICMP = 1;
  static const uint8_t IGMP = 2;
  static const uint8_t GGP  = 3;
  static const uint8_t TCP  = 6;
  static const uint8_t PUP  = 12;
  static const uint8_t UDP  = 17;
  static const uint8_t IDP  = 22;
  static const uint8_t ND   = 77;
  static const uint8_t RAW  = 255;
};

class W5500Class
{
public:

  void init(SPIClass & _spi, uint8_t _ss_pin);
  inline uint8_t readVersion(void) { return readVERSIONR(); };

  /**
   * @brief	This function is being used for copy the data form Receive buffer of the chip to application buffer.
   * 
   * It calculate the actual physical address where one has to read
   * the data from Receive buffer. Here also take care of the condition while it exceed
   * the Rx memory uper-bound of socket.
   */
  inline void read_data(SOCKET s, volatile uint16_t  src, volatile uint8_t * dst, uint16_t len)
    { read((uint16_t)src , (0x18+(s<<5)), (uint8_t *)dst, len); }

  /**
   * @brief	 This function is being called by send() and sendto() function also. 
   * 
   * This function read the Tx write pointer register and after copy the data in buffer update the Tx write pointer
   * register. User should read upper byte first and lower byte later to get proper value.
   */
  inline void send_data_processing(SOCKET s, const uint8_t *data, uint16_t len)
    { send_data_processing_offset(s, 0, data, len); }

  /**
   * @brief A copy of send_data_processing that uses the provided ptr for the
   *        write offset.  Only needed for the "streaming" UDP API, where
   *        a single UDP packet is built up over a number of calls to
   *        send_data_processing_ptr, because TX_WR doesn't seem to get updated
   *        correctly in those scenarios
   * @param ptr value to use in place of TX_WR.  If 0, then the value is read
   *        in from TX_WR
   * @return New value for ptr, to be used in the next call
   */
  // FIXME Update documentation
  void send_data_processing_offset(SOCKET s, uint16_t data_offset, const uint8_t *data, uint16_t len);

  /**
   * @brief	This function is being called by recv() also.
   * 
   * This function read the Rx read pointer register
   * and after copy the data from receive buffer update the Rx write pointer register.
   * User should read upper byte first and lower byte later to get proper value.
   */
  void recv_data_processing(SOCKET s, uint8_t *data, uint16_t len, uint8_t peek = 0);

  inline void setGatewayIp(uint8_t * addr)  { writeGAR(addr); }
  inline void getGatewayIp(uint8_t * addr)  { readGAR(addr); }

  inline void setSubnetMask(uint8_t * addr) { writeSUBR(addr); }
  inline void getSubnetMask(uint8_t * addr) { readSUBR(addr); }

  inline void setMACAddress(uint8_t * addr) { writeSHAR(addr); }
  inline void getMACAddress(uint8_t * addr) { readSHAR(addr); }

  inline void setIPAddress(uint8_t * addr)  { writeSIPR(addr); }
  inline void getIPAddress(uint8_t * addr)  { readSIPR(addr); }

  inline void setRetransmissionTime(uint16_t timeout) { writeRTR(timeout); }
  inline void setRetransmissionCount(uint8_t retry)  { writeRCR(retry); }

  inline void setPHYCFGR(uint8_t val) { writePHYCFGR(val); }
  inline uint8_t getPHYCFGR() { return read(0x002E, 0x00); } // { return readPHYCFGR(); }

  void execCmdSn(SOCKET s, SockCMD _cmd);
  
  uint16_t getTXFreeSize(SOCKET s);
  uint16_t getRXReceivedSize(SOCKET s);
  

  // W5500 Registers
  // ---------------
private:
  void write(uint16_t _addr, uint8_t _cb, uint8_t _data);
  void write16(uint16_t _addr, uint8_t _cb, uint16_t _data);
  void write(uint16_t _addr, uint8_t _cb, const uint8_t *buf, uint16_t len);
  uint8_t  read(uint16_t _addr, uint8_t _cb );
  uint16_t read16(uint16_t _addr, uint8_t _cb );
  uint16_t read(uint16_t _addr, uint8_t _cb, uint8_t *buf, uint16_t len);
  
#define __GP_REGISTER8(name, address)          \
  inline void write##name(uint8_t _data) {     \
    write(address, 0x04, _data);               \
  }                                            \
  inline uint8_t read##name() {                \
    return read(address, 0x00);                \
  }
#define __GP_REGISTER16(name, address)         \
  inline void write##name(uint16_t _data) {    \
    write16(address, 0x04, _data);             \
  }                                            \
  inline uint16_t read##name() {               \
    return read16(address, 0x00);              \
  }
#define __GP_REGISTER_N(name, address, size)   \
  inline void write##name(uint8_t *_buff) {    \
    write(address, 0x04, _buff, size);         \
  }                                            \
  inline uint16_t read##name(uint8_t *_buff) { \
    return read(address, 0x00, _buff, size);   \
  }

public:
  __GP_REGISTER8 (MR,      0x0000);    // Mode
  __GP_REGISTER_N(GAR,     0x0001, 4); // Gateway IP address
  __GP_REGISTER_N(SUBR,    0x0005, 4); // Subnet mask address
  __GP_REGISTER_N(SHAR,    0x0009, 6); // Source MAC address
  __GP_REGISTER_N(SIPR,    0x000F, 4); // Source IP address
  __GP_REGISTER8 (IR,      0x0015);    // Interrupt
  __GP_REGISTER8 (IMR,     0x0016);    // Interrupt Mask
  __GP_REGISTER16(RTR,     0x0019);    // Timeout address
  __GP_REGISTER8 (RCR,     0x001B);    // Retry count
  __GP_REGISTER8 (PTIMER,  0x001C);    // PPP LCP Request Timer
  __GP_REGISTER8 (PMAGIC,  0x001D);    // PPP LCP Magic Number
  __GP_REGISTER_N(UIPR,    0x0028, 4); // Unreachable IP address in UDP mode
  __GP_REGISTER16(UPORT,   0x002C);    // Unreachable Port address in UDP mode
  __GP_REGISTER8 (PHYCFGR, 0x002E);   // PHY Configuration
  __GP_REGISTER8 (VERSIONR,0x0039);   // Chip version


#undef __GP_REGISTER8
#undef __GP_REGISTER16
#undef __GP_REGISTER_N

  // W5500 Socket registers
  // ----------------------
private:
  inline uint8_t readSn(SOCKET s, uint16_t addr)    { return read(addr, (s<<5)+0x08); }
  inline uint16_t readSn16(SOCKET s, uint16_t addr) { return read16(addr, (s<<5)+0x08); }
  inline uint16_t readSn(SOCKET s, uint16_t addr, uint8_t *buf, uint16_t len) { return read(addr, (s<<5)+0x08, buf, len ); }
  inline void writeSn(SOCKET s, uint16_t addr, uint8_t data) { write(addr, (s<<5)+0x0C, data); }
  inline void writeSn16(SOCKET s, uint16_t addr, uint16_t data) { write16(addr, (s<<5)+0x0C, data); }
  inline void writeSn(SOCKET s, uint16_t addr, uint8_t *buf, uint16_t len) { write(addr, (s<<5)+0x0C, buf, len); }

#define __SOCKET_REGISTER8(name, address)                 \
  inline void write##name(SOCKET _s, uint8_t _data) {     \
    writeSn(_s, address, _data);                          \
  }                                                       \
  inline uint8_t read##name(SOCKET _s) {                  \
    return readSn(_s, address);                           \
  }
#define __SOCKET_REGISTER16(name, address)                \
  inline void write##name(SOCKET _s, uint16_t _data) {    \
    writeSn16(_s, address, _data);                        \
  }                                                       \
  inline uint16_t read##name(SOCKET _s) {                 \
    return readSn16(_s, address);                         \
  }
#define __SOCKET_REGISTER_N(name, address, size)          \
  inline void write##name(SOCKET _s, uint8_t *_buff) {    \
    writeSn(_s, address, _buff, size);                    \
  }                                                       \
  inline uint16_t read##name(SOCKET _s, uint8_t *_buff) { \
    return readSn(_s, address, _buff, size);              \
  }

public:
  __SOCKET_REGISTER8(SnMR,        0x0000)        // Mode
  __SOCKET_REGISTER8(SnCR,        0x0001)        // Command
  __SOCKET_REGISTER8(SnIR,        0x0002)        // Interrupt
  __SOCKET_REGISTER8(SnSR,        0x0003)        // Status
  __SOCKET_REGISTER16(SnPORT,     0x0004)        // Source Port
  __SOCKET_REGISTER_N(SnDHAR,     0x0006, 6)     // Destination Hardw Addr
  __SOCKET_REGISTER_N(SnDIPR,     0x000C, 4)     // Destination IP Addr
  __SOCKET_REGISTER16(SnDPORT,    0x0010)        // Destination Port
  __SOCKET_REGISTER16(SnMSSR,     0x0012)        // Max Segment Size
  __SOCKET_REGISTER8(SnPROTO,     0x0014)        // Protocol in IP RAW Mode
  __SOCKET_REGISTER8(SnTOS,       0x0015)        // IP TOS
  __SOCKET_REGISTER8(SnTTL,       0x0016)        // IP TTL
  __SOCKET_REGISTER16(SnTX_FSR,   0x0020)        // TX Free Size
  __SOCKET_REGISTER16(SnTX_RD,    0x0022)        // TX Read Pointer
  __SOCKET_REGISTER16(SnTX_WR,    0x0024)        // TX Write Pointer
  __SOCKET_REGISTER16(SnRX_RSR,   0x0026)        // RX Free Size
  __SOCKET_REGISTER16(SnRX_RD,    0x0028)        // RX Read Pointer
  __SOCKET_REGISTER16(SnRX_WR,    0x002A)        // RX Write Pointer (supported?)
  
#undef __SOCKET_REGISTER8
#undef __SOCKET_REGISTER16
#undef __SOCKET_REGISTER_N

public:
  static const uint16_t SSIZE = 2048; // Max Tx buffer size

private:
  SPIClass & mSPI = SPI;
  uint8_t SS;
  volatile uint32_t * ssPortReg;
  uint16_t ssBitMask;
  inline void initSS(uint8_t ss_pin) {
    SS = ss_pin;
    pinMode(SS, OUTPUT);
    ssPortReg = portSetRegister(SS);
    ssBitMask = digitalPinToBitMask(SS);
    deselect_SS();
  };
  inline void select_SS()     { *ssPortReg = (ssBitMask<<16); } //{ digitalWrite(SS, LOW); };
  inline void deselect_SS()   { *ssPortReg = ssBitMask; } //{ digitalWrite(SS, HIGH); };
};


#endif
