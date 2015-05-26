/*******************************************************************************
Copyright (c) 2012-2015, Autelan Networks. All rights reserved.
*******************************************************************************/
#define _LINUX_LIST_H
#include <environment.h>

static at_ops_t __ops[]     = AT_DEFT_OPS;
static at_cache_t __cache[os_count_of(__ops)];
at_control_t at_control     = AT_CONTROL_DEFT(NULL, __ops, __cache);

extern env_t *env_ptr;
unsigned int *__AKID_DEBUG;

/*
* call it in fastboot
*/
static void
at_boot_init(void)
{
    __at_control->env = (at_env_t *)(env_ptr->env);
    
    __AKID_DEBUG = at_mark_pointer(__at_mark_debug);
}

/******************************************************************************/
