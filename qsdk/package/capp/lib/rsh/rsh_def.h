#ifndef __RSH_DEF_H_96C0E396473EB754D6B0A32DD2496188__
#define __RSH_DEF_H_96C0E396473EB754D6B0A32DD2496188__
/******************************************************************************/
#include "utils.h"
/******************************************************************************/
#ifndef RSH_CONF
#define RSH_CONF                    "/etc/rsh/rsh.conf"
#endif

#ifndef RSH_FD_COUNT
#define RSH_FD_COUNT                (1024*1024)
#endif

#ifndef RSH_MAC_HASH
#define RSH_MAC_HASH                (1<<16)
#endif

#ifndef RSH_BOARD_HASH
#define RSH_BOARD_HASH              (1<<4)
#endif

#ifndef RSH_BOARD_NAMELEN
#define RSH_BOARD_NAMELEN           31
#endif

#ifndef RSH_TAG_HASH
#define RSH_TAG_HASH                (1<<10)
#endif

#ifndef RSH_TAG_COUNT
#define RSH_TAG_COUNT               8
#endif

#ifndef RSH_TAG_NAMELEN
#define RSH_TAG_NAMELEN             15
#endif

#ifndef RSH_TAG_VALUELEN
#define RSH_TAG_VALUELEN            31
#endif

#ifndef RSH_ROOTFS_HASH
#define RSH_ROOTFS_HASH             (1<<8)
#endif

#ifndef RSH_ROOTFS_COUNT
#define RSH_ROOTFS_COUNT            3
#endif

#ifndef RSH_ROOTFS_NAMELEN
#define RSH_ROOTFS_NAMELEN          31
#endif

#ifndef RSH_WEB_HASH
#define RSH_WEB_HASH                (1<<8)
#endif

#ifndef RSH_WEB_NAMELEN
#define RSH_WEB_NAMELEN             31
#endif

#ifndef RSH_SERVICE_PORT
#define RSH_SERVICE_PORT            874
#endif

#ifndef RSH_COMMAND_PORT
#define RSH_COMMAND_PORT            875
#endif

#ifndef RSH_CONFIG_PORT
#define RSH_CONFIG_PORT             876
#endif

#ifndef RSH_ONLINE_KEEPALIVE
#define RSH_ONLINE_KEEPALIVE        300000 /* ms */
#endif

#ifndef RSH_ALIVE_KEEPALIVE
#define RSH_ALIVE_KEEPALIVE         6000 /* ms */
#endif

#ifndef RSH_KEEPALIVE_INTERVAL
#define RSH_KEEPALIVE_INTERVAL      5   /* times */
#endif

#ifndef RSH_TIMER_MSEC
#define RSH_TIMER_MSEC              1000    /* ms */
#endif


enum {
    RSH_FSM_INIT,
    RSH_FSM_DISCONNECT,
    RSH_FSM_CONNECT,
    RSH_FSM_ALIVE,
    
    RSH_FSM_END
};

#define RSH_FSM_STRINGS \
    [RSH_FSM_INIT]      = "init", \
    [RSH_FSM_DISCONNECT]= "disconnect", \
    [RSH_FSM_CONNECT]   = "connect" \
    [RSH_FSM_ALIVE]     = "alive" \
    /* end */

static inline bool
is_good_rsh_fsm(int state)
{
    return is_good_enum(state)
}

static inline char *
rsh_fsm_string(int state)
{
    static char *array[RSH_FSM_END] = {RSH_FSM_STRINGS};

    return is_good_rsh_fsm(state)?array[state]:__unknow;
}
/******************************************************************************/
#endif /* __RSH_DEF_H_96C0E396473EB754D6B0A32DD2496188__ */
