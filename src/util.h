#ifndef UTIL_H
#define UTIL_H

#define word(a, b) ( (uint16_t)((a)<<8) | (b) )

#define htons(x) __builtin_bswap16(x)
#define ntohs(x) htons(x)

#define htonl(x) __builtin_bswap32(x)
#define ntohl(x) htonl(x)

#endif
