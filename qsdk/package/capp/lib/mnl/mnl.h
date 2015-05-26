#ifndef __MNL_H_2FB8D6EF781CB3C6DF0B024CC5EC7D8A__
#define __MNL_H_2FB8D6EF781CB3C6DF0B024CC5EC7D8A__
/******************************************************************************/
#include "utils.h"
#define MNL_MAGIC       "MNL"
#define MNL_VERSION     1

enum {
    MNL_P_MULTICAST,

    MNL_P_END
};

struct mnlmsghdr {
    char        magic[3];
    uint8_t     protocol;

    union {
        struct {
            uint8_t     version;
            uint8_t     reserved8;
            uint16_t    group; /* dst_group */
            uint32_t    spid;
            uint32_t    reserved32;
        } m;
    } u;
};
/******************************************************************************/
#endif /* __MNL_H_50B50258D31867D43008409C0F99A5B2__ */
