#ifndef __SMONITOR_H_61C5E8485DEB03CFF49ECA4D8DC2BCE9__
#define __SMONITOR_H_61C5E8485DEB03CFF49ECA4D8DC2BCE9__
/******************************************************************************/
#include "utils.h"
/******************************************************************************/
#ifndef ENV_SMONITORD_UNIX
#define ENV_SMONITORD_UNIX          "__SMONITORD_UNIX__"
#endif

#ifndef ENV_SMONITORC_UNIX
#define ENV_SMONITORC_UNIX          "__SMONITORC_UNIX__"
#endif

#ifndef ENV_SMONITOR_TIMEOUT
#define ENV_SMONITOR_TIMEOUT        "__SMONITOR_TIMEOUT__"
#endif

#ifndef SMONITORD_UNIX
#define SMONITORD_UNIX              "/tmp/.smonitord.unix"
#endif

#ifndef SMONITORC_UNIX
#define SMONITORC_UNIX              "/tmp/.smonitorc.%d.unix"
#endif

#ifndef SMONITOR_TIMEOUT
#define SMONITOR_TIMEOUT            3000
#endif

#ifndef SMONITOR_NAMESIZE
#define SMONITOR_NAMESIZE           31
#endif

#ifndef SMONITOR_CMDSIZE
#define SMONITOR_CMDSIZE            127
#endif

static inline int
get_smonitord_path_env(struct sockaddr_un *addr) 
{
    return __copy_string_env(ENV_SMONITORD_UNIX, SMONITORD_UNIX, 
                get_abstract_path(addr),
                abstract_path_size);
}

static inline int
get_smonitorc_path_env(struct sockaddr_un *addr) 
{
    char path[1+OS_LINE_LEN] = {0};

    os_saprintf(path, SMONITORC_UNIX, getpid());
    
    return __copy_string_env(ENV_SMONITORC_UNIX, path, 
                get_abstract_path(addr),
                abstract_path_size);
}

static inline int
get_smonitor_timeout_env(void) 
{
    return get_int_env(ENV_SMONITOR_TIMEOUT, SMONITOR_TIMEOUT);
}
/******************************************************************************/
#endif /* __SMONITOR_H_61C5E8485DEB03CFF49ECA4D8DC2BCE9__ */
