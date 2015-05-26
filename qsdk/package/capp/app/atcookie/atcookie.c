/*******************************************************************************
Copyright (c) 2012-2015, Autelan Networks. All rights reserved.
*******************************************************************************/
#ifndef __THIS_APP
#define __THIS_APP      atcookie
#endif
    
#ifndef __THIS_NAME
#define __THIS_NAME     "atcookie"
#endif

#include "atcookie/atcookie.h"

OS_INITER;

#ifndef __BUSYBOX__
#define atcookie_main  main
#endif

/*
* cmd have enabled when boot
*/
static int
__main(int argc, char *argv[])
{
    char *file, *name;

    switch(argc) {
        case 1:
            name = AT_CERT;
            break;
        case 2:
            name = argv[1];
            break;
        default:
            return shell_error(EFORMAT);
    }
    
    file = at_secookie_file(name);
    if (NULL==file) {
        return shell_error(ENOEXIST);
    }

    os_println("%s", file);
    
    return 0;
}

/*
* cmd have enabled when boot
*/
int atcookie_main(int argc, char *argv[])
{
    return os_main(__main, argc, argv);
}
/******************************************************************************/
