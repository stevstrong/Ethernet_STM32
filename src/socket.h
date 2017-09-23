 /*
 * - 10 Apr. 2015
 * Added support for Arduino Ethernet Shield 2
 * by Arduino.org team
 */
 
#ifndef	_SOCKET_H_
#define	_SOCKET_H_

#include "w5500.h"


class SOCKETClass
{
private:
	static W5500Class w5500;
public:
	void init(SPIClass & spi, uint8_t sspin) { w5500.init(spi, sspin); }
	void init(uint8_t sspin) { init(SPI, sspin); }
	void init() { init(SPI, PA4); }
	inline uint8_t readChipVersion(void) { return w5500.readVersion(); }
	inline uint8_t status(SOCKET s) { return w5500.readSnSR(s); }
	inline int16_t recvAvailable(SOCKET s) { return w5500.getRXReceivedSize(s); }
	inline void setMACAddress(uint8_t * mac) { w5500.setMACAddress(mac); }
	inline void setIPAddress(uint8_t * ip) { w5500.setIPAddress(ip); }
	inline void getIPAddress(uint8_t * ip) { w5500.getIPAddress(ip); }
	inline void setGatewayIp(uint8_t * ip) { w5500.setGatewayIp(ip); }
	inline void getGatewayIp(uint8_t * ip) { w5500.getGatewayIp(ip); }
	inline void setSubnetMask(uint8_t * sm) { w5500.setSubnetMask(sm); }
	inline void getSubnetMask(uint8_t * sm) { w5500.getSubnetMask(sm); }
	inline void getRemoteIP(SOCKET s, uint8_t * ip) { w5500.readSnDIPR(s, ip); }
	uint8_t open(SOCKET s, uint8_t protocol, uint16_t port, uint8_t flag); // Opens a socket(TCP or UDP or IP_RAW mode)
	void close(SOCKET s); // Close socket
	uint8_t connect(SOCKET s, uint8_t * addr, uint16_t port); // Establish TCP connection (Active connection)
	inline void disconnect(SOCKET s) { w5500.execCmdSn(s, Sock_DISCON); } // disconnect the connection
	uint8_t listen(SOCKET s);	// Establish TCP connection (Passive connection)
	uint16_t send(SOCKET s, const uint8_t * buf, uint16_t len); // Send data (TCP)
	int16_t recv(SOCKET s, uint8_t * buf, int16_t len);	// Receive data (TCP)
	inline void peek(SOCKET s, uint8_t *buf) { w5500.recv_data_processing(s, buf, 1, 1); }
	uint16_t sendto(SOCKET s, const uint8_t * buf, uint16_t len, uint8_t * addr, uint16_t port); // Send data (UDP/IP RAW)
	uint16_t recvfrom(SOCKET s, uint8_t * buf, uint16_t len, uint8_t * addr, uint16_t *port); // Receive data (UDP/IP RAW)
	void flush(SOCKET s); // Wait for transmission to complete
	void set(SOCKET s, uint8_t * mac, uint8_t * addr, uint16_t port); // set multicast UDP

	uint16_t igmpsend(SOCKET s, const uint8_t * buf, uint16_t len);

	// Functions to allow buffered UDP send (i.e. where the UDP datagram is built up over a
	// number of calls before being sent
	/*
	  @brief This function sets up a UDP datagram, the data for which will be provided by one
	  or more calls to bufferData and then finally sent with sendUDP.
	  @return 1 if the datagram was successfully set up, or 0 if there was an error
	*/
	int startUDP(SOCKET s, uint8_t* addr, uint16_t port);
	/*
	  @brief This function copies up to len bytes of data from buf into a UDP datagram to be
	  sent later by sendUDP.  Allows datagrams to be built up from a series of bufferData calls.
	  @return Number of bytes successfully buffered
	*/
	uint16_t bufferData(SOCKET s, uint16_t offset, const uint8_t* buf, uint16_t len);
	/*
	  @brief Send a UDP datagram built up from a sequence of startUDP followed by one or more
	  calls to bufferData.
	  @return 1 if the datagram was successfully sent, or 0 if there was an error
	*/
	int sendUDP(SOCKET s);
};

extern SOCKETClass socket;

#endif /* _SOCKET_H_ */
