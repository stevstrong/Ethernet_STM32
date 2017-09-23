#ifndef UTIL_H
#define UTIL_H


#ifndef word
 #define word(a, b) ( (uint16_t)((a)<<8) | (b) )
#endif

#ifndef htons
 #define htons(x) __builtin_bswap16(x)
 #define ntohs(x) htons(x)
#endif

#ifndef htonl
 #define htonl(x) __builtin_bswap32(x)
 #define ntohl(x) htonl(x)
#endif


#endif
