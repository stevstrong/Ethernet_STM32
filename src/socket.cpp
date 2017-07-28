 /*
 * - 10 Apr. 2015
 * Added support for Arduino Ethernet Shield 2
 * by Arduino.org team
 */
 
#include "w5500.h"
#include "socket.h"
#include "IPAddress.h"

static uint16_t local_port;
W5500Class SOCKETClass::w5500;

/**
 * @brief	This Socket function initialize the channel in perticular mode, and set the port and wait for w5500 done it.
 * @return 	1 for success else 0.
 */
uint8_t SOCKETClass::open(SOCKET s, uint8_t protocol, uint16_t port, uint8_t flag)
{
  if ((protocol == SnMR::TCP) || (protocol == SnMR::UDP) || (protocol == SnMR::IPRAW) || (protocol == SnMR::MACRAW) || (protocol == SnMR::PPPOE))
  {
    close(s);
    w5500.writeSnMR(s, protocol | flag);
    if (port != 0) {
      w5500.writeSnPORT(s, port);
    } 
    else {
      local_port++; // if don't set the source port, set local_port number.
      w5500.writeSnPORT(s, local_port);
    }

    w5500.execCmdSn(s, Sock_OPEN);
    
    return 1;
  }

  return 0;
}


/**
 * @brief	This function close the socket and parameter is "s" which represent the socket number
 */
void SOCKETClass::close(SOCKET s)
{
  w5500.execCmdSn(s, Sock_CLOSE);
  w5500.writeSnIR(s, 0xFF);
}


/**
 * @brief	This function established  the connection for the channel in passive (server) mode. This function waits for the request from the peer.
 * @return	1 for success else 0.
 */
uint8_t SOCKETClass::listen(SOCKET s)
{
  if (w5500.readSnSR(s) != SnSR::INIT)
    return 0;
  w5500.execCmdSn(s, Sock_LISTEN);
  return 1;
}


/**
 * @brief	This function established  the connection for the channel in Active (client) mode. 
 * 		This function waits for the untill the connection is established.
 * 		
 * @return	1 for success else 0.
 */
uint8_t SOCKETClass::connect(SOCKET s, uint8_t * addr, uint16_t port)
{
  if ( ( IPAddress(addr) == IPAddress((uint32_t)0xFFFFFFFF ) ) ||
       ( IPAddress(addr) == IPAddress((uint32_t)0 ) ) ||
	   (port == 0x00) )
    return 0;

  // set destination IP
  w5500.writeSnDIPR(s, addr);
  w5500.writeSnDPORT(s, port);
  w5500.execCmdSn(s, Sock_CONNECT);

  return 1;
}

void SOCKETClass::set(SOCKET s, uint8_t * mac, uint8_t * addr, uint16_t port)
{
  w5500.writeSnDHAR(s, mac);
  w5500.writeSnDIPR(s, addr);   //239.255.0.1
  w5500.writeSnDPORT(s, port);
}


/**
 * @brief	This function used to send the data in TCP mode
 * @return	1 for success else 0.
 */
uint16_t SOCKETClass::send(SOCKET s, const uint8_t * buf, uint16_t len)
{
  uint8_t status=0;
  uint16_t ret=0;
  uint16_t freesize=0;

  if (len > w5500.SSIZE) 
    ret = w5500.SSIZE; // check size not to exceed MAX size.
  else 
    ret = len;

  // if freebuf is available, start.
  do 
  {
    freesize = w5500.getTXFreeSize(s);
    status = w5500.readSnSR(s);
    if ((status != SnSR::ESTABLISHED) && (status != SnSR::CLOSE_WAIT))
    {
      ret = 0; 
      break;
    }
  } 
  while (freesize < ret);

  // copy data
  w5500.send_data_processing(s, (uint8_t *)buf, ret);
  w5500.execCmdSn(s, Sock_SEND);

  /* +2008.01 bj */
  while ( (w5500.readSnIR(s) & SnIR::SEND_OK) != SnIR::SEND_OK ) 
  {
    /* m2008.01 [bj] : reduce code */
    if ( w5500.readSnSR(s) == SnSR::CLOSED )
    {
      close(s);
      return 0;
    }
  }
  /* +2008.01 bj */
  w5500.writeSnIR(s, SnIR::SEND_OK);
  return ret;
}


/**
 * @brief	This function is an application I/F function which is used to receive the data in TCP mode.
 * 		It continues to wait for data as much as the application wants to receive.
 * 		
 * @return	received data size for success else -1.
 */
int16_t SOCKETClass::recv(SOCKET s, uint8_t *buf, int16_t len)
{
  // Check how much data is available
  int16_t ret = w5500.getRXReceivedSize(s);
  if ( ret == 0 )
  {
    // No data available.
    uint8_t status = w5500.readSnSR(s);
    if ( status == SnSR::LISTEN || status == SnSR::CLOSED || status == SnSR::CLOSE_WAIT )
    {
      // The remote end has closed its side of the connection, so this is the eof state
      ret = 0;
    }
    else
    {
      // The connection is still up, but there's no data waiting to be read
      ret = -1;
    }
  }
  else if (ret > len)
  {
    ret = len;
  }

  if ( ret > 0 )
  {
    w5500.recv_data_processing(s, buf, ret);
    w5500.execCmdSn(s, Sock_RECV);
  }
  return ret;
}


/**
 * @brief	This function is an application I/F function which is used to send the data for other then TCP mode. 
 * 		Unlike TCP transmission, The peer's destination address and the port is needed.
 * 		
 * @return	This function return send data size for success else -1.
 */
uint16_t SOCKETClass::sendto(SOCKET s, const uint8_t *buf, uint16_t len, uint8_t *addr, uint16_t port)
{
  if (len > w5500.SSIZE) len = w5500.SSIZE; // check size not to exceed MAX size.

  if ( ( IPAddress(addr) == IPAddress((uint32_t)0) ) || (port == 0x00) || (len == 0) ) {
    /* +2008.01 [bj] : added return value */
    return 0;
  }
  w5500.writeSnDIPR(s, addr);
  w5500.writeSnDPORT(s, port);

  // copy data
  w5500.send_data_processing(s, (uint8_t *)buf, len);
  w5500.execCmdSn(s, Sock_SEND);

  /* +2008.01 bj */
  while ( !(w5500.readSnIR(s) & SnIR::SEND_OK) ) 
  {
    if (w5500.readSnIR(s) & SnIR::TIMEOUT)
    {
      /* +2008.01 [bj]: clear interrupt */
      w5500.writeSnIR(s, (SnIR::SEND_OK | SnIR::TIMEOUT)); /* clear SEND_OK & TIMEOUT */
      return 0;
    }
  }

  /* +2008.01 bj */
  w5500.writeSnIR(s, SnIR::SEND_OK);

  return len;
}


/**
 * @brief	This function is an application I/F function which is used to receive the data in other then
 * 	TCP mode. This function is used to receive UDP, IP_RAW and MAC_RAW mode, and handle the header as well. 
 * 	
 * @return	This function return received data size for success else -1.
 */
uint16_t SOCKETClass::recvfrom(SOCKET s, uint8_t *buf, uint16_t len, uint8_t *addr, uint16_t *port)
{
  uint8_t head[8];
  uint16_t data_len=0;

  if ( len > 0 )
  {
    uint16_t ptr = w5500.readSnRX_RD(s);
    switch (w5500.readSnMR(s) & 0x07)
    {
    case SnMR::UDP :
      w5500.read_data(s, ptr, head, 0x08);
      ptr += 8;
      // read peer's IP address, port number.
      *(uint32_t*)addr = *(uint32_t*)head;
      *port = word(head[4], head[5]);
      data_len = word(head[6], head[7]);

      w5500.read_data(s, ptr, buf, data_len); // data copy.
      ptr += data_len;

      w5500.writeSnRX_RD(s, ptr);
      break;

    case SnMR::IPRAW :
      w5500.read_data(s, ptr, head, 0x06);
      ptr += 6;

      *(uint32_t*)addr = *(uint32_t*)head;
      //addr[0] = head[0];
      //addr[1] = head[1];
      //addr[2] = head[2];
      //addr[3] = head[3];
      data_len = word(head[4], head[5]);

      w5500.read_data(s, ptr, buf, data_len); // data copy.
      ptr += data_len;

      w5500.writeSnRX_RD(s, ptr);
      break;

    case SnMR::MACRAW:
      w5500.read_data(s, ptr, head, 2);
      ptr+=2;
      data_len = word(head[0], head[1]) - 2;

      w5500.read_data(s, ptr, buf, data_len);
      ptr += data_len;
      w5500.writeSnRX_RD(s, ptr);
      break;

    default :
      break;
    }
    w5500.execCmdSn(s, Sock_RECV);
  }
  return data_len;
}

/**
 * @brief	Wait for buffered transmission to complete.
 */
void SOCKETClass::flush(SOCKET s) {
  // TODO
}

uint16_t SOCKETClass::igmpsend(SOCKET s, const uint8_t * buf, uint16_t len)
{
  uint16_t ret=0;

  if (len > w5500.SSIZE) 
    ret = w5500.SSIZE; // check size not to exceed MAX size.
  else 
    ret = len;

  if (ret == 0)
    return 0;

  w5500.send_data_processing(s, (uint8_t *)buf, ret);
  w5500.execCmdSn(s, Sock_SEND);

  while ( !(w5500.readSnIR(s) & SnIR::SEND_OK) ) 
  {
    if (w5500.readSnIR(s) & SnIR::TIMEOUT)
    {
      /* in case of igmp, if send fails, then socket closed */
      /* if you want change, remove this code. */
      close(s);
      return 0;
    }
  }

  w5500.writeSnIR(s, SnIR::SEND_OK);
  return ret;
}

uint16_t SOCKETClass::bufferData(SOCKET s, uint16_t offset, const uint8_t* buf, uint16_t len)
{
  uint16_t ret =0;
  if (len > w5500.getTXFreeSize(s))
  {
    ret = w5500.getTXFreeSize(s); // check size not to exceed MAX size.
  }
  else
  {
    ret = len;
  }
  w5500.send_data_processing_offset(s, offset, buf, ret);
  return ret;
}

int SOCKETClass::startUDP(SOCKET s, uint8_t* addr, uint16_t port)
{
  if ( (IPAddress(addr) == IPAddress((uint32_t)0)) || (port == 0) ) {
    return 0;
  }
  else
  {
    w5500.writeSnDIPR(s, addr);
    w5500.writeSnDPORT(s, port);
    return 1;
  }
}

int SOCKETClass::sendUDP(SOCKET s)
{
  w5500.execCmdSn(s, Sock_SEND);

  /* +2008.01 bj */
  while ( !(w5500.readSnIR(s) & SnIR::SEND_OK) ) 
  {
    if (w5500.readSnIR(s) & SnIR::TIMEOUT)
    {
      /* +2008.01 [bj]: clear interrupt */
      w5500.writeSnIR(s, (SnIR::SEND_OK|SnIR::TIMEOUT));
      return 0;
    }
  }

  /* +2008.01 bj */	
  w5500.writeSnIR(s, SnIR::SEND_OK);

  /* Sent ok */
  return 1;
}

SOCKETClass socket;

