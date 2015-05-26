#ifndef __THIS_APP
#define __THIS_APP      umc
#endif
    
#ifndef __THIS_NAME
#define __THIS_NAME     "umc"
#endif

#ifndef __THIS_FILE
#define __THIS_FILE     "umc"
#endif

#define USE_INLINE_SIMPILE_RES

#include "um/um.h"

OS_INITER;
DECLARE_SIMPILE_RES;

static char *self;
static simpile_client_t umc = SIMPILE_CLIENT_INITER(UM_TIMEOUT, UMD_UNIX);

static int
usage(int error)
{
#if USE_USAGE
    os_fprintln(stderr, "%s bind   {mac} {ip}", self);
    os_fprintln(stderr, "%s unbind {mac}", self);
    os_fprintln(stderr, "%s auth   {mac} {group} {json}", self);
    os_fprintln(stderr, "%s deauth {mac}", self);
    
    os_fprintln(stderr, "%s show [json]", self);
#endif

    return shell_error(error);
}

#define umc_client(_action, _argc, _argv) \
    simpile_c_client(_action, _argc, _argv, &umc.server, &umc.client, umc.timeout)

/*
* bind {mac} {ip}
*/
static int
cmd_bind(int argc, char *argv[])
{
    char *mac= argv[0];
    char *ip = argv[1];
    
    if (2!=argc) {
        return -EFORMAT;
    }
    else if (false==is_good_macstring(mac)) {
        debug_trace("bad mac %s", mac);

        return -EFORMAT;
    }
    else if (false==is_good_ipstring(ip)) {
        debug_trace("bad ip %s", ip);

        return -EFORMAT;
    }

    return umc_client("bind", argc, argv);
}

/*
* unbind {mac}
*/
static int
cmd_unbind(int argc, char *argv[])
{
    char *mac= argv[0];
    
    if (1!=argc) {
        return -EFORMAT;
    }
    else if (false==is_good_macstring(mac)) {
        debug_trace("bad mac %s", mac);

        return -EFORMAT;
    }

    return umc_client("unbind", argc, argv);
}

/*
* auth {mac} {group} {json}
*/
static int
cmd_auth(int argc, char *argv[])
{
    char *mac   = argv[0];
    char *group = argv[1];
    char *json  = argv[2];
    
    if (3!=argc) {
        return -EFORMAT;
    }
    else if (false==is_good_macstring(mac)) {
        debug_trace("bad mac %s", mac);

        return -EFORMAT;
    }
    else if (os_atoi(group) < 0) {
        debug_trace("bad group %s", group);

        return -EFORMAT;
    }
    else if (false==is_good_json(json)) {
        debug_trace("bad json %s", json);

        return -EFORMAT;
    }
    
    return umc_client("auth", argc, argv);
}

/*
* deauth {mac}
*/
static int
cmd_deauth(int argc, char *argv[])
{
    char *mac = argv[0];
    
    if (1!=argc) {
        return -EFORMAT;
    }
    else if (false==is_good_macstring(mac)) {
        debug_trace("bad mac %s", mac);

        return -EFORMAT;
    }

    return umc_client("deauth", argc, argv);
}

static int
show_by(int argc, char *argv[])
{
    char *json = argv[0];

    if (is_good_json(json) || 0==os_strcmp("stat", json)) {
        return umc_client("show", argc, argv);
    }
    else {
        debug_error("bad json %s", json);
        
        return -EFORMAT;
    }
}

/*
* show {mac} {json}
*/
static int
cmd_show(int argc, char *argv[])
{
    switch(argc) {
        case 0:
            return umc_client("show", argc, argv);
        case 1: {
            char *json = argv[0];

            if (is_good_json(json) || 0==os_strcmp("stat", json)) {
                return umc_client("show", argc, argv);
            }
            else {
                debug_error("bad json %s", json);
                
                return -EFORMAT;
            }
        }
        default:
            return usage(-EHELP);
    }
}

static int
command(int argc, char *argv[])
{
    static cmd_table_t table[] = {
        CMD_ENTRY("bind",   cmd_bind),
        CMD_ENTRY("unbind", cmd_unbind),
        CMD_ENTRY("auth",   cmd_auth),
        CMD_ENTRY("deauth", cmd_deauth),
        CMD_ENTRY("show",   cmd_show),
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
    
    umc.timeout = get_um_timeout_env();

    err = get_umc_path_env(&umc.client);
    if (err) {
        return err;
    }
    
    err = get_umd_path_env(&umc.server);
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
#define umc_main  main
#endif

int umc_main(int argc, char *argv[])
{
    return os_call(os_init, os_fini, __main, argc, argv);
}

/******************************************************************************/
