#ifndef __MAC_H_F836BDB5B61CB538446562E3E6DD918C__
#define __MAC_H_F836BDB5B61CB538446562E3E6DD918C__
/******************************************************************************/
#include "number.h"

enum {
    OS_MACSIZE      = 6,
    OS_OUISIZE      = 3,

    /*
    * macstring is "XX:XX:XX:XX:XX:XX" or "XX-XX-XX-XX-XX-XX"
    */
    MACSTRINGLEN_L  = (3*OS_MACSIZE-1),

    /*
    * macstring is "XXXXXXXXXXXX"
    */
    MACSTRINGLEN_S  = (2*OS_MACSIZE),
};

#define OS_ZEROMAC                  ((unsigned char *)"\x00\x00\x00\x00\x00\x00")
#define OS_FULLMAC                  ((unsigned char *)"\xff\xff\xff\xff\xff\xff")
#define OS_2BMAC                    ((unsigned char *)"\x2b\x2b\x2b\x2b\x2b\x2b")

static inline unsigned char *
os_maccpy(unsigned char dst[], unsigned char src[])
{
    return os_memcpy(dst, src, OS_MACSIZE);
}

static inline unsigned char *
os_maczero(unsigned char mac[])
{
    return os_memzero(mac, OS_MACSIZE);
}

static inline unsigned char *
os_macfull(unsigned char mac[])
{
    return os_maccpy(mac, OS_FULLMAC);
}

static inline int
os_maccmp(unsigned char a[], unsigned char b[])
{
    return os_memcmp(a, b, OS_MACSIZE);
}

static inline bool
os_maceq(unsigned char a[], unsigned char b[])
{
    return 0==os_maccmp(a, b);
}

static inline bool
os_macmaskmach(unsigned char a[], unsigned char b[], unsigned char mask[])
{
    return os_bufmaskmatch(a, b, mask, OS_MACSIZE);
}

static inline bool
is_zero_mac(unsigned char mac[])
{
    return os_maceq(mac, OS_ZEROMAC);
}

static inline bool
is_full_mac(unsigned char mac[])
{
    return os_maceq(mac, OS_FULLMAC);
}

static inline bool
is_good_mac(unsigned char mac[])
{
    return false==is_zero_mac(mac) 
        && false==is_full_mac(mac);
}

static inline unsigned char *
os_getmac_bystring(unsigned char mac[], char macstring[])
{
    int len = os_strlen(macstring);
    int width = 3;
    int i;

    /*
    * macstring is "XX:XX:XX:XX:XX:XX" / "XX-XX-XX-XX-XX-XX"
    */
    if (MACSTRINGLEN_L==len) {
        width = 3;
    }
    /*
    * macstring is "XXXXXXXXXXXX"
    */
    else if (MACSTRINGLEN_S==len) {
        width = 2;
    }
    else {
        return OS_2BMAC;
    }
    
    for (i=0; i<OS_MACSIZE; i++) {
        mac[i] = os_digitstring2number(macstring + width*i, 2, 16, int);
    }

    return mac;
}

/*
*  multi-thread unsafe
*/
static inline unsigned char *
os_mac(char *macstring)
{
    static unsigned char mac[OS_MACSIZE] = {0};
    
    return is_good_string(macstring)?os_getmac_bystring(mac, macstring):OS_ZEROMAC;
}

static inline int
os_macsnprintf(unsigned char mac[], char macstring[], int len, int sep)
{
    if (0==sep) {
        return os_snprintf(macstring, len,
            "%.2x"
            "%.2x"
            "%.2x"
            "%.2x"
            "%.2x"
            "%.2x",
            mac[0], 
            mac[1], 
            mac[2], 
            mac[3], 
            mac[4], 
            mac[5]);
    } else {
        return os_snprintf(macstring, len,
            "%.2x"  "%c"
            "%.2x"  "%c"
            "%.2x"  "%c"
            "%.2x"  "%c"
            "%.2x"  "%c"
            "%.2x",
            mac[0], sep,
            mac[1], sep,
            mac[2], sep,
            mac[3], sep,
            mac[4], sep,
            mac[5]);
    }
}

#define os_macsaprintf(_mac, _macstring, _sep) \
        os_macsnprintf(_mac, _macstring, sizeof(_macstring), _sep)

/*
*  multi-thread unsafe
*/
static inline char *
os_getmacstring(unsigned char mac[], int sep)
{
    static char macstring[1+MACSTRINGLEN_L] = {0};

    os_macsaprintf(mac, macstring, sep);

    return macstring;
}

/*
*  multi-thread unsafe
*/
static inline char *
os_macstring(unsigned char mac[])
{
    return os_getmacstring(mac, ':');
}

static inline bool
is_good_macstring(char *macstring)
{
    return is_good_string(macstring) && is_good_mac(os_mac(macstring));
}
/******************************************************************************/
#endif /* __MAC_H_F836BDB5B61CB538446562E3E6DD918C__ */
