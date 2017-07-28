/*
 * Copyright (c) 2010 by WIZnet <support@wiznet.co.kr>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */


#include "w5500.h"


void W5500Class::init(SPIClass & _spi, uint8_t _ss_pin)
{
  initSS(_ss_pin);
  mSPI = _spi;
  mSPI.beginTransaction(SPISettings(42000000));

  writeMR(0x80); // software reset the W5500 chip
  delay(100);    // give time for reset
}

uint16_t W5500Class::getTXFreeSize(SOCKET s)
{
    uint16_t val=0, val1=0;
    do {
        val1 = readSnTX_FSR(s);
        if (val1 != 0)
            val = readSnTX_FSR(s);
    } 
    while (val != val1);
    return val;
}

uint16_t W5500Class::getRXReceivedSize(SOCKET s)
{
    uint16_t val=0,val1=0;
    do {
        val1 = readSnRX_RSR(s);
        if (val1 != 0)
            val = readSnRX_RSR(s);
    } 
    while (val != val1);
    return val;
}

void W5500Class::send_data_processing_offset(SOCKET s, uint16_t data_offset, const uint8_t *data, uint16_t len)
{
    uint16_t ptr = readSnTX_WR(s);
    uint8_t cntl_byte = (0x14+(s<<5));
    ptr += data_offset;
    write(ptr, cntl_byte, data, len);
    ptr += len;
    writeSnTX_WR(s, ptr);
}

void W5500Class::recv_data_processing(SOCKET s, uint8_t *data, uint16_t len, uint8_t peek)
{
    uint16_t ptr = readSnRX_RD(s);
    read_data(s, ptr, data, len);
    if (!peek) {
        ptr += len;
        writeSnRX_RD(s, ptr);
    }
}

void W5500Class::write(uint16_t _addr, uint8_t _cb, uint8_t _data)
{
    select_SS();  
    mSPI.write16(_addr);
    mSPI.write16((uint16_t)(_cb<<8) | _data);
    deselect_SS();
}

void W5500Class::write16(uint16_t _addr, uint8_t _cb, uint16_t _data)
{
    select_SS();  
    mSPI.write16(_addr);
    mSPI.write(_cb);
    mSPI.write16(_data);
    deselect_SS();
}

void W5500Class::write(uint16_t _addr, uint8_t _cb, const uint8_t *_buf, uint16_t _len)
{
    select_SS();
    mSPI.write16(_addr);
    mSPI.write(_cb);
    mSPI.write((uint8_t*)_buf, _len);
    deselect_SS();
}

uint8_t W5500Class::read(uint16_t _addr, uint8_t _cb)
{
    select_SS();
    mSPI.write16(_addr);
    mSPI.write(_cb);
    uint8_t _data = mSPI.transfer(0);
    deselect_SS();
    return _data;
}

uint16_t W5500Class::read16(uint16_t _addr, uint8_t _cb)
{
    select_SS();
    mSPI.write16(_addr);
    mSPI.write(_cb);
	uint16_t _data = mSPI.transfer16(0);
    deselect_SS();
    return _data;
}

uint16_t W5500Class::read(uint16_t _addr, uint8_t _cb, uint8_t *_buf, uint16_t _len)
{
    select_SS();
    mSPI.write16(_addr);
    mSPI.write(_cb);
    mSPI.read(_buf, _len);
    deselect_SS();
    return _len;
}

void W5500Class::execCmdSn(SOCKET s, SockCMD _cmd) {
    // Send command to socket
    writeSnCR(s, _cmd);
    // Wait for command to complete
    while (readSnCR(s))
    ;
}
