/*******************************************************************************
Copyright (c) 2012-2015, Autelan Networks. All rights reserved.
*******************************************************************************/
#ifndef __THIS_APP
#define __THIS_APP      atenv
#endif
    
#ifndef __THIS_NAME
#define __THIS_NAME     "atenv"
#endif

#include "atenv/atenv.h"
#include "atenv/bcookie.h"

#ifdef __BOOT__
#     if AT_PRODUCT==AT_PRODUCT_LTEFI_MD2
#       include "atenv_boot_md.c"
#   elif AT_PRODUCT==AT_PRODUCT_LTEFI_AP2
#       include "atenv_boot_ap.c"
#   endif
#elif defined(__APP__)
#   include "atenv_app.c"
#endif
/******************************************************************************/
