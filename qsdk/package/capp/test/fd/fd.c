/******************************************************************************/
#ifndef __THIS_APP
#define __THIS_APP      testfd
#endif
    
#ifndef __THIS_NAME
#define __THIS_NAME     "testfd"
#endif

#define USE_INLINE_TIMER
#define USE_INLINE_COROUTINE
#define USE_INLINE_FD

#include "utils.h"

OS_INITER;
DECLARE_TIMER;
DECLARE_COROUTINE;
DECLARE_FD;

#define SERVER "server"
#define CLIENT "client"

static int
server(void *no_used)
{
    return 0;
}

static int
client(void *no_used)
{
    return 0;
}

static int 
as_server(int argc, char *argv[])
{
    fd_looper();
    co_new("server", server, NULL, -1, CO_PRI_DEFAULT, 0, false);
    co_idle();

    return 0;
}

static int
as_client(int argc, char *argv[])
{
    
    return 0;
}

static int
help(void)
{
    return 0;
}

static int 
__fini(void) 
{
    fd_fini();

    return 0;
}

static int 
__init(void) 
{
    fd_init();

    return 0;
}

static int
__main(int argc, char *argv[])
{
    int err;
    
    if (argc < 2) {
        return shell_error(help());
    }
    
    argc--;
    argv++;
    
    if (0==os_strcmp(SERVER, argv[0])) {
        err = as_server(argc, argv);
    }
    else if (0==os_strcmp(CLIENT, argv[0])) {
        err = as_client(argc, argv);
    }
    else {
        err = help();
    }

    return shell_error(err);
}

int main(int argc, char *argv[])
{
    return os_call(__init, __fini, __main, argc, argv);
}

/******************************************************************************/
