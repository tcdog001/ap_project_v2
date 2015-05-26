/*******************************************************************************
Copyright (c) 2012-2015, Autelan Networks. All rights reserved.
*******************************************************************************/
#ifndef __THIS_APP
#define __THIS_APP      smonitorc
#endif
        
#ifndef __THIS_NAME
#define __THIS_NAME     "smonitorc"
#endif

#define USE_INLINE_SIMPILE_RES

#include "smonitor/smonitor.h"

OS_INITER;
DECLARE_SIMPILE_RES;

static char *self;
static simpile_client_t smonitorc = SIMPILE_CLIENT_INITER(SMONITOR_TIMEOUT, SMONITORD_UNIX);

static int
usage(int error)
{
#if USE_USAGE
    os_fprintln(stderr, "%s insert name command", self);
    os_fprintln(stderr, "%s remove name", self);
    os_fprintln(stderr, "%s show", self);
#endif

    return shell_error(error);
}

#define smonitorc_client(_action, _argc, _argv) \
    simpile_c_client(_action, _argc, _argv, &smonitorc.server, &smonitorc.client, smonitorc.timeout)

static int
cmd_insert(int argc, char *argv[])
{
    char *name      = argv[0];
    char *command   = argv[1];
    
    if (2!=argc) {
        return usage(-EINVAL);
    }
    else if (os_strlen(name) > SMONITOR_NAMESIZE) {
        return usage(-ETOOBIG);
    }
    else if (os_strlen(command) > SMONITOR_CMDSIZE) {
        return usage(-ETOOBIG);
    }
    
    return smonitorc_client("insert", argc, argv);
}

static int
cmd_remove(int argc, char *argv[])
{
    char *name = argv[0];
    
    if (1!=argc) {
        return usage(-EINVAL);
    }
    else if (os_strlen(name) > SMONITOR_NAMESIZE) {
        return usage(-ETOOBIG);
    }
    else {
        return smonitorc_client("remove", argc, argv);
    }
}

static int
cmd_show(int argc, char *argv[])
{
    if (0!=argc) {
        return usage(-EINVAL);
    }
    else {
        return smonitorc_client("show", argc, argv);
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
    
    smonitorc.timeout = get_smonitor_timeout_env();

    err = get_smonitorc_path_env(&smonitorc.client);
    if (err) {
        return err;
    }
    
    err = get_smonitord_path_env(&smonitorc.server);
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
#define smonitorc_main  main
#endif

int smonitorc_main(int argc, char *argv[])
{
    return os_main(__main, argc, argv);
}
/******************************************************************************/
