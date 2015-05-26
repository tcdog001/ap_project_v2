#ifndef __THIS_APP
#define __THIS_APP      appkey
#endif

#ifndef __THIS_NAME
#define __THIS_NAME     "appkey"
#endif

#include "utils.h"

OS_INITER;

static int 
cmd_reload(int argc, char *argv[])
{
    return appkey_reload();
}

static int
usage(char *self)
{
#if USE_USAGE
    os_fprintln(stderr, "%s reload", self);
#endif

    return shell_error(-EFORMAT);
}

static simpile_cmd_t cmd[] = {
    {
        .argc   = 1,
        .argv   = {"reload"},
        .handle = cmd_reload,
    },
};

static int
__main(int argc, char *argv[])
{
    os_setup_signal_default(NULL);
    
    return simpile_cmd_handle(cmd, argc, argv, usage);
}

#ifndef __BUSYBOX__
#define appkey_main  main
#endif

int appkey_main(int argc, char *argv[])
{
    return os_main(__main, argc, argv);
}

/******************************************************************************/ 
