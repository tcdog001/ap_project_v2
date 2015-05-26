#ifndef __THIS_APP
#define __THIS_APP      umd
#endif

#ifndef __THIS_NAME
#define __THIS_NAME     "umd"
#endif

#ifndef __THIS_FILE
#define __THIS_FILE     "timer"
#endif

#include "umd.h"

static multi_value_t
timer_handle(struct um_user *user, time_t now)
{
    multi_value_u mv;

    mv.value = um_user_aging(user);
    if (mv2_is_break(mv)) {
        return mv.value;
    }

    mv.value = um_user_timeout(user, now);
    if (mv2_is_break(mv)) {
        return mv.value;
    }
    
    return mv2_OK;
}

static int
timer_server_init(simpile_server_t *server)
{
    int fd = os_timerfd(time_sec(UM_TICKS), time_nsec(UM_TICKS));
    if (fd<0) {
        return fd;
    }
    server->fd = fd;

    debug_ok("init timer server ok");
    
    return 0;
}

static int
timer_server_handle(simpile_server_t *server)
{
    uint32_t times = os_timerfd_times(server->fd);
    time_t now = time(NULL);
    int i;
    
    multi_value_t cb(struct um_user *user)
    {
        return timer_handle(user, now);
    }

    for (i=0; i<times; i++) {
        umd.ticks++;
        
        um_user_foreach(cb);
    }
    
    return 0;
}

simpile_server_t um_timer_server = {
    .fd     = INVALID_FD,

    .init   = timer_server_init,
    .handle = timer_server_handle,
};

/******************************************************************************/
