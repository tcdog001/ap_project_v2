/******************************************************************************/
#ifndef __THIS_APP
#define __THIS_APP      testco
#endif
    
#ifndef __THIS_NAME
#define __THIS_NAME     "testco"
#endif

#define USE_INLINE_TIMER
#define USE_INLINE_COROUTINE

#include "utils.h"

OS_INITER;
DECLARE_TIMER;
DECLARE_COROUTINE;

#define DELAY   (1000*100)
#define COUNT   1000
#define TIMES   10

int something(void *data)
{
    char name[1+CO_NAME_LEN];
    int i;

    for (i=0; i<TIMES; i++) {
        os_saprintf(name, "%s.%d", co_self_name(), i);
        co_new(name, something, NULL, -1, 128, 0, false);
    }
    
    co_yield();

    return 0;
}

static int 
fini(void) 
{
    co_fini();

    return 0;
}

static int 
init(void) 
{
    co_init();

    return 0;
}

static int 
__main(int argc, char *argv[])
{
    co_id_t one;
    int i = 0;
    
    one = co_new("1", something, NULL, -1, 128, 0, false);

    debug_ok("MAIN start idle");
    co_idle();
    debug_ok("MAIN end idle");

    return 0;
}

int main(int argc, char *argv[])
{
    return os_call(init, fini, __main, argc, argv);
}

/******************************************************************************/
