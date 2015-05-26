/*******************************************************************************
Copyright (c) 2012-2015, Autelan Networks. All rights reserved.
*******************************************************************************/
#ifndef __THIS_APP
#define __THIS_APP      atdog
#endif
    
#ifndef __THIS_NAME
#define __THIS_NAME     "atdog"
#endif

#include "utils.h"
#include "hi_unf_wdg.h"

OS_INITER;

enum { ATDOG_COUNT = 2 };

#if 0
HI_S32 HI_UNF_WDG_Init(HI_VOID);
HI_S32 HI_UNF_WDG_DeInit(HI_VOID);
HI_S32 HI_UNF_WDG_Enable(HI_U32 u32WdgNum);
HI_S32 HI_UNF_WDG_Disable(HI_U32 u32WdgNum);
HI_S32 HI_UNF_WDG_GetTimeout(HI_U32 u32WdgNum, HI_U32 *pu32Value);
HI_S32 HI_UNF_WDG_SetTimeout(HI_U32 u32WdgNum, HI_U32 u32Value);
HI_S32 HI_UNF_WDG_Clear(HI_U32 u32WdgNum);
HI_S32 HI_UNF_WDG_Reset(HI_U32 u32WdgNum);
#endif

#define method(_method, _dog, _args...)  ({ \
    int err;                                \
                                            \
    err = HI_UNF_WDG_##_method(_dog, ##_args); \
        debug_trace_error(err, "dog%d " #_method, dog); \
                                            \
    err;                                    \
})

static int
enable(int dog)
{
    return method(Enable, dog);
}

static int
disable(int dog)
{
    return method(Disable, dog);
}

static int
clear(int dog)
{
    return method(Clear, dog);
}

static int
reset(int dog)
{
    return method(Reset, dog);
}

static int
get_timeout(int dog, int *timeout)
{
    return method(GetTimeout, dog, timeout);
}

static int
set_timeout(int dog, int timeout)
{
    return method(SetTimeout, dog, timeout);
}

#define foreach(_method, _args...)   ({ \
    int i, err;                         \
                                        \
    for (i=0; i<ATDOG_COUNT; i++) {     \
        err = _method(i, ##_args);      \
        if (err) {                      \
            return err;                 \
        }                               \
    }                                   \
                                        \
    0;                                  \
})

static int
cmd_enable(int argc, char *argv[])
{
    return foreach(enable);
}

static int
cmd_disable(int argc, char *argv[])
{
    foreach(clear);
    
    return foreach(disable);
}

static int
cmd_clear(int argc, char *argv[])
{
    return foreach(clear);
}

static int
cmd_reset(int argc, char *argv[])
{
    return foreach(reset);
}

static int
cmd_get_timeout(int argc, char *argv[])
{
    int i, err, timeout[ATDOG_COUNT];

    for (i=0; i<ATDOG_COUNT; i++) {
        err = get_timeout(i, &timeout[i]);
        if (err) {
            return err;
        }
    }
    
    for (i=1; i<ATDOG_COUNT; i++) {
        if (timeout[i-1] != timeout[i]) {
            debug_error("dog%d's timeout(%d) != dog%d's timeout(%d)",
                i-1,
                timeout[i-1],
                i,
                timeout[i]);
            
            return -1;
        }
    }

    os_println("%d", timeout[0]);
    
    return 0;
}

static int
cmd_set_timeout(int argc, char *argv[])
{
    int timeout;
    char *end = NULL;
    char *string = argv[1];
    
    timeout = os_strtol(string, &end, 0);
    if (false==os_strtonumber_is_good(end) || timeout <= 0) {
        os_println("bad timeout:%s", string);
        
        return -EFORMAT;
    } else {
        return foreach(set_timeout, timeout);
    }
}

static simpile_cmd_t cmd[] = {
    {
        .argc   = 1,
        .argv   = {"enable"},
        .handle = cmd_enable,
    },
    {
        .argc   = 1,
        .argv   = {"disable"},
        .handle = cmd_disable,
    },
    {
        .argc   = 1,
        .argv   = {"clear"},
        .handle = cmd_clear,
    },
    {
        .argc   = 1,
        .argv   = {"reset"},
        .handle = cmd_reset,
    },
    {
        .argc   = 1,
        .argv   = {"timeout"},
        .handle = cmd_get_timeout,
    },
    {
        .argc   = 2,
        .argv   = {"timeout", NULL},
        .handle = cmd_set_timeout,
    },
};

static int
usage(char *self)
{
#if USE_USAGE
    os_fprintln(stderr, "%s enable", self);
    os_fprintln(stderr, "%s disable",self);
    os_fprintln(stderr, "%s clear",  self);
    os_fprintln(stderr, "%s reset",  self);
    os_fprintln(stderr, "%s timeout [timeout(ms)]", self);
#endif

    return shell_error(-EFORMAT);
}

#define __method(_method)           ({  \
    int err = HI_UNF_WDG_##_method();   \
    if (err) {                          \
        debug_error(#_method " error:%d", err); \
    }                                   \
                                        \
    err;                                \
})  /* end */

static int
__fini(void)
{
    __method(DeInit);
    
    os_fini();
    
    return 0;
}

static int
__init(void)
{
    int err;
    
    err = os_init();
    if (err) {
        return shell_error(err);
    }
    
    do {
        err = __method(Init);
        if (err) {
            sleep(1);
        }
    } while(err);
    
    return 0;
}

#ifndef __BUSYBOX__
#define atdog_main  main
#endif

static int
__main(int argc, char *argv[])
{
    return simpile_cmd_handle(cmd, argc, argv, usage);
}

/*
* dog have enabled when boot
*/
int atdog_main(int argc, char *argv[])
{
    return os_call(__init, __fini, __main, argc, argv);
}
/******************************************************************************/
