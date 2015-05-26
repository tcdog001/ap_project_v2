/*******************************************************************************
Copyright (c) 2012-2015, Autelan Networks. All rights reserved.
*******************************************************************************/
OS_INITER;

static at_env_t __env       = AT_DEFT_ENV;
static at_ops_t __ops[]     = AT_DEFT_OPS;
static at_cache_t __cache[os_count_of(__ops)];
at_control_t at_control     = AT_CONTROL_DEFT(&__env, __ops, __cache);

#ifndef __BUSYBOX__
#define atenv_main  main
#endif

int atenv_main(int argc, char *argv[])
{
    return at_main(argc, argv);
}
/******************************************************************************/
