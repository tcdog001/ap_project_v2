/*******************************************************************************
Copyright (c) 2012-2015, Autelan Networks. All rights reserved.
*******************************************************************************/
#ifdef AUTELAN
#ifndef __THIS_APP
#define __THIS_APP      cmd_atenv
#endif

#ifndef __THIS_NAME
#define __THIS_NAME     "cmd_atenv"
#endif

#include "atenv/atenv.h"

static int 
do_atenv (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    return at_main(argc, argv);
}

U_BOOT_CMD(
    atenv,  CONFIG_SYS_MAXARGS, 0,  do_atenv,
    "get/set atenv",
    "\n"
    "    atenv name ==> show env by name\n"
    "    atenv name1=value1 name2=value2 ... ==> set env by name and value"
);

static inline int
__at_version(int argc, char *argv[])
{
    int i, err;
    at_version_t version;

    err = at_version_atoi(&version, argv[1]);
    if (err) {
        return err;
    }
    
    for (i=0; i<AT_OS_COUNT; i++) {
        os_objcpy(at_rootfs_version(i), &version);
        os_objcpy(at_kernel_version(i), &version);
    }

    __at_set_dirty(AT_ENV_OS);
}

static int 
do_atversion (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    return os_call(__at_init, __at_fini, __at_version, argc, argv);
}

U_BOOT_CMD(
    atversion,  CONFIG_SYS_MAXARGS, 0,  do_atversion,
    "set atversion",
    "\n"
    "    atversion version\n"
    "    atversion version\n"
);

static int 
do_endian (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    unsigned int v = 0xaabbccdd;  
    unsigned char c = *(unsigned char *)(&v);  

    switch(c) {
        case 0xaa:
            os_println("big"); 
            break;
        case 0xdd:
            os_println("little"); 
            break;
        default:
            os_println("unknow"); 
            break;
    }

    return 0;
}

U_BOOT_CMD(
    endian,  CONFIG_SYS_MAXARGS, 0,  do_endian,
    "endian test",
    "\n"
    "    endian test\n"
    "    endian test\n"
);

#endif
/******************************************************************************/
