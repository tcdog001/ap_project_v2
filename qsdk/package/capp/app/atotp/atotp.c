/*******************************************************************************
Copyright (c) 2012-2015, Autelan Networks. All rights reserved.
*******************************************************************************/
#ifndef __THIS_APP
#define __THIS_APP      atotp
#endif
    
#ifndef __THIS_NAME
#define __THIS_NAME     "atotp"
#endif

#include "atotp/atotp.h"
#include "atenv/bcookie.h"

OS_INITER;

static int
usage(char *self)
{
#if USE_USAGE
    os_fprintln(stderr, "%s custom  show",  self);
    os_fprintln(stderr, "%s private show",  self);
#endif

    return -EFORMAT;
}

static int
cmd_custom_show(int argc, char *argv[])
{
    return at_otp_custom_show();
}

static int
cmd_custom_write(int argc, char *argv[])
{
    return at_otp_custom_write(argv[2]);
}

static int
cmd_private_show(int argc, char *argv[])
{
    return at_otp_private_show();
}

static int
cmd_private_write(int argc, char *argv[])
{
    unsigned char otp[AT_OTP_SIZE];
    int count;
    
    count = os_hexstring2buf(argv[2], otp, AT_OTP_SIZE);
    if (AT_OTP_SIZE!=count) {
        return -EFORMAT;
    }
    
    return at_otp_private_write(otp);
}

#if AT_PRODUCT==AT_PRODUCT_LTEFI_MD2
static int
cmd_bcookie_show(int argc, char *argv[])
{
    int err;
    struct bcookie_otp botp = BCOOKIE_OBJ(BCOOKIE_OTP);
    
    err = bcookie_load((struct bcookie *)&botp, sizeof(botp));
    if (err) {
        return err;
    }

    os_println("%s", __otp_string(botp.custom));
    os_println("%s", __otp_string(botp.private));

    return 0;
}

static int
cmd_bcookie_write(int argc, char *argv[])
{
    int count;
    struct bcookie_otp botp = BCOOKIE_OBJ(BCOOKIE_OTP);
    
    count = os_hexstring2buf(argv[2], botp.custom, AT_OTP_SIZE);
    if (count<0) {
        return count;
    }
    
    count = os_hexstring2buf(argv[3], botp.private, AT_OTP_SIZE);
    if (count<0) {
        return count;
    }
    
    return bcookie_save((struct bcookie *)&botp, sizeof(botp));
}
#endif

static simpile_cmd_t cmd[] = {
    {
        .argc   = 2,
        .argv   = {"custom", "show"},
        .handle = cmd_custom_show,
    },
    {
        .argc   = 3,
        .argv   = {"custom", "write", NULL},
        .handle = cmd_custom_write,
    },
    {
        .argc   = 2,
        .argv   = {"private", "show"},
        .handle = cmd_private_show,
    },
    {
        .argc   = 3,
        .argv   = {"private", "write", NULL},
        .handle = cmd_private_write,
    },
#if AT_PRODUCT==AT_PRODUCT_LTEFI_MD2
    {
        .argc   = 2,
        .argv   = {"cookie", "show"},
        .handle = cmd_bcookie_show,
    },
    {
        .argc   = 4,
        .argv   = {"cookie", "write", NULL, NULL},
        .handle = cmd_bcookie_write,
    },
#endif
};

#ifndef __BUSYBOX__
#define atotp_main  main
#endif

/*
* cmd have enabled when boot
*/
static int
__main(int argc, char *argv[])
{
    return simpile_cmd_handle(cmd, argc, argv, usage);
}

/*
* cmd have enabled when boot
*/
int atotp_main(int argc, char *argv[])
{
    return os_main(__main, argc, argv);
}
/******************************************************************************/
