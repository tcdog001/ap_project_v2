/*******************************************************************************
Copyright (c) 2012-2015, Autelan Networks. All rights reserved.
*******************************************************************************/
#ifndef __THIS_APP
#define __THIS_APP      atrt
#endif
        
#ifndef __THIS_NAME
#define __THIS_NAME     "atrt"
#endif

#include "utils.h"
#include "atenv/atenv.h"

OS_INITER;

#ifndef __BUSYBOX__
static at_env_t __env;
at_control_t at_control = AT_CONTROL_DEFT(&__env, NULL, NULL);
#endif

#define ATRT_MAXRUN     (12*3600)
#define SCRIPT_REBOOT   AT_FILE("etc/monitor/reboot.sh")

static int runtime = 0;

static int
__rt_load(int idx)
{
    runtime = at_mark_get(__at_mark_runtime);

    return 0;
}

static int
__rt_save(int idx)
{
    at_mark_set(__at_mark_runtime, runtime);

    return 0;
}

static int
rt_load(int noused)
{
    int err;
    
    (void)noused;
    
    err = os_call_1(__at_load, __at_nothing, __rt_load, AT_ENV_MARK);
        debug_trace_error(err, "load atenv");

    return err;
}

static int
rt_save(int noused)
{
    int err;
    
    (void)noused;
    
    err = os_call_1(__at_load, __at_save, __rt_save, AT_ENV_MARK);
        debug_trace_error(err, "save atenv");

    return err;
}

static int 
__reboot(void)
{
    __os_system(SCRIPT_REBOOT "&");

    return 0;
}

static void 
__exit(int sig)
{
    int err;

    debug_trace("recive sig:%d", sig);
    
    err = os_call(at_open, at_close, rt_save, 0);
        debug_trace_error(err, "save atenv");
    
    exit(err);
}

#ifndef __BUSYBOX__
#define atrt_main  main
#endif

static int
__main(int argc, char *argv[])
{
    int i, err;

    os_setup_signal_exit(__exit);
    
    os_call(at_open, at_close, rt_load, 0);
    
    for (i=0; i<ATRT_MAXRUN; i++) {
        sleep(1);

        runtime++;
    }
    
    os_call(at_open, at_close, rt_save, 0);

    return __reboot();
}

int atrt_main(int argc, char *argv[])
{
    return os_main(__main, argc, argv);
}
/******************************************************************************/
