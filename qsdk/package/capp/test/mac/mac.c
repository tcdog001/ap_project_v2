/******************************************************************************/
#ifndef __THIS_APP
#define __THIS_APP      testmac
#endif
    
#ifndef __THIS_NAME
#define __THIS_NAME     "testmac"
#endif

#include "utils.h"

OS_INITER;

#define MAC0        ((unsigned char *)"\x00\x11\x22\xaa\xbb\xcc")
#define MAC1        ((unsigned char *)"\xff\xee\xdd\xcc\xbb\xaa")

#define STRING0     "001122aabbcc"
#define STRING1     "ffeeddccbbaa"

#define SEP0        "00:11:22:aa:bb:cc"
#define SEP1        "ff:ee:dd:cc:bb:aa"

static char g_macstring[1+MACSTRINGLEN_L] = {0};

int main(int argc, char *argv[])
{   
    __os_dump_buffer(MAC0, 6, NULL);
    os_println("mac==>string(%s)", os_macstring(MAC0));
    os_println("==========================================");
    
    __os_dump_buffer(MAC1, 6, NULL);
    os_println("mac==>string(%s)", os_macstring(MAC1));
    os_println("==========================================");
    
    __os_dump_buffer(os_mac(STRING0), 6, NULL);
    os_println("string(%s)==>mac", STRING0);
    os_println("==========================================");
    
    __os_dump_buffer(os_mac(STRING1), 6, NULL);
    os_println("string(%s)==>mac", STRING1);
    os_println("==========================================");
    
    __os_dump_buffer(os_mac(SEP0), 6, NULL);
    os_println("string(%s)==>mac", SEP0);
    os_println("==========================================");
    
    __os_dump_buffer(os_mac(SEP1), 6, NULL);
    os_println("string(%s)==>mac", SEP1);
    os_println("==========================================");
    
    return 0;
}
/******************************************************************************/
