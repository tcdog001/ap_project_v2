/*******************************************************************************
Copyright (c) 2012-2015, Autelan Networks. All rights reserved.
*******************************************************************************/
#ifndef __THIS_APP
#define __THIS_APP      atcheck
#endif
    
#ifndef __THIS_NAME
#define __THIS_NAME     "atcheck"
#endif

#include "atenv/atenv.h"
#include "atdog/atdog.h"

OS_INITER;

static char *bootbuf;

static int fd_boot      = INVALID_FD;
static int fd_bootenv   = INVALID_FD;
static int fd_cmdline   = INVALID_FD;

#define AT_CHECK_DEBUG  0
#if AT_CHECK_DEBUG
#define dprintln(_fmt, _args...)  os_println(_fmt, ##_args)
#else
#define dprintln(_fmt, _args...)  os_do_nothing()
#endif

static int
__fini(void)
{
    os_close(fd_cmdline);
    os_close(fd_bootenv);
    os_close(fd_boot);
    os_free(bootbuf);

    os_fini();

    return 0;
}

static int
__init(void)
{
    int fd, err;
    int permit = S_IRUSR | S_IRGRP;
        
    err = os_init();
    if (err) {
        return shell_error(err);
    }

    bootbuf = (char *)os_malloc(AT_BOOT_SIZE);
    if (NULL==bootbuf) {
        return shell_error(-ENOMEM);
    }

    do {
        fd = open(DEV_BOOT, O_RDONLY, permit);
        if(false==is_good_fd(fd)) {
            trace_error(errno, "open " DEV_BOOT);
            
            sleep(1);
        }
    }while(false==is_good_fd(fd));
    fd_boot = fd;
    
    fd = open(DEV_BOOTENV, O_RDONLY, permit);
    if (false==is_good_fd(fd)) {
        trace_error(errno, "open " AT_DEV_BOOTENV);
        
        return shell_error(-errno);
    }
    fd_bootenv = fd;

    fd = open(AT_PROC_CMDLINE, O_RDONLY, permit);
    if (false==is_good_fd(fd)) {
        trace_error(errno, "open " AT_PROC_CMDLINE);
        
        return shell_error(-errno);
    }
    fd_cmdline = fd;
    
    return 0;
}

static inline int
hacked(int obj, int reason)
{
    os_println("system is hacked(%d.%d), reboot...", obj, reason);
    
    __os_system("reboot &");
    
    return -EHACKED;
}

static int
check_boot(void)
{
    static at_reg_t dog_enable[] = AT_REG_DOG_ENABLE;
    static struct {
        char *addr;
        int size;
    } cookie[] = {
        {
            .addr = (char *)dog_enable,
            .size = sizeof(dog_enable),
        },
        {
            .addr = CONFIG_BOOTARGS,
            .size = sizeof(CONFIG_BOOTARGS),
        },
    };

    char *p     = bootbuf;
    char *end   = bootbuf + AT_BOOT_SIZE;
    int i;

    dprintln("before check boot");
    
    if (AT_BOOT_SIZE != read(fd_boot, bootbuf, AT_BOOT_SIZE)) {
        trace_error(errno, "read boot");

        return -errno;
    }
    
    for (i=0; i<os_count_of(cookie); i++) {
        
        p = os_memmem(p, end - p, cookie[i].addr, cookie[i].size);
        if (NULL==p) {
            return hacked(1, i);
        }
    }
    dprintln("after  check boot");
    
    return 0;
}

static int
check_bootenv(void)
{
    at_cookie_t deft = AT_DEFT_COOKIE;
    at_cookie_t cookie;

    dprintln("before check bootenv");
    
    if (sizeof(cookie) != read(fd_bootenv, &cookie, sizeof(cookie))) {
        trace_error(errno, "read atenv cookie");

        return -errno;
    }

    if (false==os_objeq(&cookie, &deft)) {
        return hacked(2, 0);
    }

    dprintln("after  check bootenv");
    
    return 0;
}

static int
check_partition(void)
{
    char line[1+OS_LINE_LEN] = {0};

    dprintln("before check partition");
    
    read(fd_cmdline, line, OS_LINE_LEN);
    
    if (NULL==os_strstr(line, CONFIG_BOOTARGS_BODY)) {
        return hacked(3, 0);
    }

    dprintln("after  check partition");
    
    return 0;
}

#ifndef __BUSYBOX__
#define atcheck_main  main
#endif


/*
* cmd have enabled when boot
*/
static int
__main(int argc, char *argv[])
{
    static int (*check[])(void) = {
        check_boot,
        check_bootenv,
        check_partition,
    };
    
    int i, err;

    for (i=0; i<os_count_of(check); i++) {
        err = (*check[i])();
        if (err) {
            return shell_error(err);
        }
    }
    
    return 0;
}

/*
* cmd have enabled when boot
*/
int atcheck_main(int argc, char *argv[])
{
    return os_call(__init, __fini, __main, argc, argv);
}
/******************************************************************************/
