#ifndef __STIMER_H_03B9C4BFCDC8EF03E5E39A08D5201373__
#define __STIMER_H_03B9C4BFCDC8EF03E5E39A08D5201373__
/******************************************************************************/
#include "utils.h"
/******************************************************************************/
#ifndef ENV_STIMERD_UNIX
#define ENV_STIMERD_UNIX            "__STIMERD_UNIX__"
#endif

#ifndef ENV_STIMERC_UNIX
#define ENV_STIMERC_UNIX            "__STIMERC_UNIX__"
#endif

#ifndef ENV_STIMER_TIMEOUT
#define ENV_STIMER_TIMEOUT          "__STIMER_TIMEOUT__"
#endif

#ifndef STIMERD_UNIX
#define STIMERD_UNIX                "/tmp/.stimerd.unix"
#endif

#ifndef STIMERC_UNIX
#define STIMERC_UNIX                "/tmp/.stimerc.%d.unix"
#endif

#ifndef STIMER_TICKS
#define STIMER_TICKS                1000 /* ms */
#endif

#ifndef STIMER_TIMEOUT
#define STIMER_TIMEOUT              3000 /* ms */
#endif

#ifndef STIMER_NAMESIZE
#define STIMER_NAMESIZE             31
#endif

#ifndef STIMER_CMDSIZE
#define STIMER_CMDSIZE              127
#endif

static bool
is_good_stimer_args(int delay, int interval, int limit)
{
    if (delay<0 || interval<0 || limit<0) {
        return false;
    } else {
        return interval || (delay && 1==limit);
    }
}

static inline int
get_stimerd_path_env(struct sockaddr_un *addr) 
{
    return __copy_string_env(ENV_STIMERD_UNIX, STIMERD_UNIX, 
                get_abstract_path(addr),
                abstract_path_size);
}

static inline int
get_stimerc_path_env(struct sockaddr_un *addr) 
{
    char path[1+OS_LINE_LEN] = {0};

    os_saprintf(path, STIMERC_UNIX, getpid());
    
    return __copy_string_env(ENV_STIMERC_UNIX, path, 
                get_abstract_path(addr),
                abstract_path_size);
}

static inline int
get_stimer_timeout_env(void) 
{
    return get_int_env(ENV_STIMER_TIMEOUT, STIMER_TIMEOUT);
}
/******************************************************************************/
#endif /* __STIMER_H_03B9C4BFCDC8EF03E5E39A08D5201373__ */
