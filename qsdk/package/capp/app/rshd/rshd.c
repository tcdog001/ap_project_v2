/*******************************************************************************
Copyright (c) 2012-2015, Autelan Networks. All rights reserved.
*******************************************************************************/
#ifndef __THIS_APP
#define __THIS_APP      rshd
#endif
        
#ifndef __THIS_NAME
#define __THIS_NAME     "rshd"
#endif

#include "rshd.h"

OS_INITER;
/*
* rsha: reverse shell agent, on work net
* rshd: reverse shell server, on internet
* rshc: reverse shell client, on person pc
*/
static struct rsh_control rshd = RSH_CONTROL_INITER;

static int
run(void)
{
    return 0;
}

static int
init_timer(void)
{
    int fd = os_timerfd(time_sec(RSH_TIMER_MSEC), time_nsec(RSH_TIMER_MSEC));
    if (fd<0) {
        return fd;
    }
    
    rshd.tm.fd = fd;

    return 0;
}

static int
init_epoll(void)
{
    int fd = epoll_create(RSH_FD_COUNT);
    if (fd<0) {
        return fd;
    }

    rshd.ep.fd = fd;

    return 0;
}

static int
init_cmd(void)
{
    return 0;
}

static int
init_svr(void)
{
    return 0;
}

static int
load(char *config)
{
    return 0;
}

static int
usage(int argc, char *argv[])
{
#if USE_USAGE
    os_fprintln(stderr, "%s [config]", argv[0]);
#endif

    return -EINVAL;
}

static int
check(int argc, char *argv[])
{
    switch(argc) {
    case 1:
        return load(RSH_CONF);
    case 2:
        return load(argv[1]);
    default:
        return usage(argc, argv);
    }
}

static int
init(void)
{
    int err;
    
    err = init_epoll();
    if (err) {
        return err;
    }

    err = init_timer();
    if (err) {
        return err;
    }

    err = init_cmd();
    if (err) {
        return err;
    }

    err = init_svr();
    if (err) {
        return err;
    }
    
    return 0;
}

static int
__main(int argc, char *argv[])
{
    int err;
    
    err = rsh_sym_init(&rshd);
    if (err) {
        debug_error("__init sym error:%d", err);

        return shell_error(err);
    }
    
    err = rsh_ep_init(&rshd);
    if (err) {
        debug_error("__init ep error:%d", err);

        return shell_error(err);
    }

    err = check(argc, argv);
    if (err) {
        return shell_error(err);
    }

    err = init();
    if (err) {
        return shell_error(err);
    }

    err = run();
    if (err) {
        return shell_error(err);
    }
    
    return 0;
}

int main(int argc, char *argv[])
{
    return os_main(__main, argc, argv);
}
/******************************************************************************/
