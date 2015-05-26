/*******************************************************************************
Copyright (c) 2012-2015, Autelan Networks. All rights reserved.
*******************************************************************************/
#ifndef __THIS_APP
#define __THIS_APP      stimerc
#endif

#ifndef __THIS_NAME
#define __THIS_NAME     "stimerc"
#endif

#define USE_INLINE_SIMPILE_RES

#include "stimer/stimer.h"

OS_INITER;
DECLARE_SIMPILE_RES;

static char *self;
static simpile_client_t stimerc = SIMPILE_CLIENT_INITER(STIMER_TIMEOUT, STIMERD_UNIX);

static int
usage(int error)
{
#if USE_USAGE
    os_fprintln(stderr, "%s insert name delay(second) interval(second) limit command", self);
    os_fprintln(stderr, "%s remove name", self);
    os_fprintln(stderr, "%s show [name]", self);
#endif

    return shell_error(error);
}

#define stimerc_client(_action, _argc, _argv) \
    simpile_c_client(_action, _argc, _argv, &stimerc.server, &stimerc.client, stimerc.timeout)

static int
cmd_insert(int argc, char *argv[])
{
    char *name      = argv[0];
    char *delay     = argv[1];
    char *interval  = argv[2];
    char *limit     = argv[3];
    char *command   = argv[4];
    
    if (5!=argc) {
        return usage(-EINVAL0);
    }
    else if (os_strlen(name) > STIMER_NAMESIZE) {
        return usage(-ETOOBIG);
    }
    else if (os_strlen(command) > STIMER_CMDSIZE) {
        return usage(-ETOOBIG);
    }
    
    int i_delay     = os_atoi(delay);
    int i_interval  = os_atoi(interval);
    int i_limit     = os_atoi(limit);
    
    if (false==is_good_stimer_args(i_delay, i_interval, i_limit)) {
        return usage(-EINVAL);
    }
    
    return stimerc_client("insert", argc, argv);
}

static int
cmd_remove(int argc, char *argv[])
{
    char *name = argv[0];
    
    if (1!=argc) {
        return usage(-EINVAL);
    }
    else if (os_strlen(name) > STIMER_NAMESIZE) {
        return usage(-ETOOBIG);
    }
    else {
        return stimerc_client("remove", argc, argv);
    }
}

static int
cmd_show(int argc, char *argv[])
{
    char *name = argv[0];
    
    if (0!=argc && 1!=argc) {
        return usage(-EINVAL);
    }
    else if (name && os_strlen(name) > STIMER_NAMESIZE) {
        return usage(-ETOOBIG);
    }
    else {
        return stimerc_client("show", argc, argv);
    }
}

static int
command(int argc, char *argv[])
{
    static cmd_table_t table[] = {
        CMD_ENTRY("insert",  cmd_insert),
        CMD_ENTRY("remove",  cmd_remove),
        CMD_ENTRY("show",    cmd_show),
    };
    int err;

    err = cmd_argv_handle(table, os_count_of(table), argc, argv);
    if (err) {
        debug_error("%s error:%d", argv[0], err);

        return err;
    }
    
    return 0;
}

static int
init_env(void) 
{
    int err;
    
    stimerc.timeout = get_stimer_timeout_env();

    err = get_stimerc_path_env(&stimerc.client);
    if (err) {
        return err;
    }
    
    err = get_stimerd_path_env(&stimerc.server);
    if (err) {
        return err;
    }

    return 0;
}

static int
__main(int argc, char *argv[])
{
    int err;
    
    self = os_cmd_trace_argv(argc, argv);

    if (1==argc) {
        return usage(-EHELP);
    }
    
    err = init_env();
        debug_trace_error(err, "init env");
    if (err) {
        return err;
    }
    
    err = command(argc-1, argv+1);
    if (err) {
        /* just log, NOT return */
    }

    return shell_error(err);
}

#ifndef __BUSYBOX__
#define stimerc_main  main
#endif

int stimerc_main(int argc, char *argv[])
{
    return os_main(__main, argc, argv);
}
/******************************************************************************/
