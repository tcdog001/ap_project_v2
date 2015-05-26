/*******************************************************************************
Copyright (c) 2012-2015, Autelan Networks. All rights reserved.
*******************************************************************************/
#ifndef __THIS_APP
#define __THIS_APP      atss
#endif

#ifndef __THIS_NAME
#define __THIS_NAME     "atss"
#endif

#include "atotp/atotp.h"
#include "atenv/bcookie.h"
#include "atcookie/atcookie.h"

OS_INITER;

#ifndef __BUSYBOX__
static at_env_t __env;
at_control_t at_control = AT_CONTROL_DEFT(&__env, NULL, NULL);
#endif

static int fd_lock = INVALID_FD;

enum { HACKED_CUSTOM = 1 };

#define PLT_USERPWD             "autelanLSS:autelanLSSPWD"
#define PLT_URL                 "https://lms1:9999/"
#define PLT_SERVICE_REGISTER    "service1"
#define PLT_SERVICE_AUTH        "service2"
#define PLT_SERVICE_REPORT      "service3"

#define FILE_LOCK               "/tmp/.atss.lock"

static int
jcheck(jobj_t obj)
{
    int code = 0;
    jobj_t jmac, jmid, jpsn, jcode, jact;
    
    if (NULL==(jmac = jobj_get(obj, "mac"))
        || NULL==(jmid = jobj_get(obj, "mid"))
        || NULL==(jpsn = jobj_get(obj, "psn"))
        || NULL==(jcode= jobj_get(obj, "code"))
        || 0!=os_strcmp(at_info_get(__at_info_product_mac), jobj_string(jmac))
        || at_mark_get(__at_mark_cid_mid)!=jobj_get_int(jmid)
        || at_mark_get(__at_mark_cid_psn)!=jobj_get_int(jpsn)
        || 0!=(code = jobj_get_int(jcode))) {
        return code;
    }
    
    jact = jobj_get(obj, "act");
    if (jact) {
        char *action = jobj_string(jact);
        
        if (is_good_string(action)) {
            __os_system(action);
        }
    }

    return 0;
}

static int
do_hacked(unsigned char custom[], unsigned char private[], int code)
{
    return 0;
}

static int
do_report(unsigned char custom[], unsigned char private[], int hack)
{
    char line[1+OS_LINE_LEN] = {0};
    char *file;
    jobj_t obj = NULL;
    int err, code = 0;

    file = at_secookie_file(AT_CERT_LSS);
    if (NULL==file) {
        return -ENOEXIST;
    }
    
    err = os_v_spgets(line, OS_LINE_LEN, 
            "curl -k -cert %s"
            " -u " PLT_USERPWD
            " -F mac=%s"
            " -F mid=%d"
            " -F psn=%d"
            " -F error=%d"
            " -s " PLT_URL PLT_SERVICE_REPORT,
            file,
            at_info_get(__at_info_product_mac),
            at_mark_get(__at_mark_cid_mid),
            at_mark_get(__at_mark_cid_psn),
            hack);
    unlink(file);
    if (err) {
        return -ECURLFAIL;
    }
    
    obj = jobj(line);
    if (NULL==obj) {
        err = -EFAKESEVER; goto error;
    }

    code = jcheck(obj);
    if (code) {
        err = -EFAKESEVER; goto error;
    }
    
    jobj_t jerr = jobj_get(obj, "error");
    if (NULL==jerr || hack!=jobj_string(jerr)) {
        err = -EFAKESEVER; goto error;
    }

    debug_ok("report ok");
error:
    jobj_put(obj);

    if (code) {
        do_hacked(custom, private, code);
    }
    
    return err;
}

static int
do_register(unsigned char custom[], unsigned char private[])
{
    char line[1+OS_LINE_LEN] = {0};
    char *file;
    jobj_t obj = NULL;
    int err, code = 0;

    file = at_secookie_file(AT_CERT_LSS);
    if (NULL==file) {
        return -ENOEXIST;
    }

    err = os_v_spgets(line, OS_LINE_LEN, 
            "curl -k -cert %s"
            " -u " PLT_USERPWD
            " -F mac=%s"
            " -F mid=%d"
            " -F psn=%d"
            " -s " PLT_URL PLT_SERVICE_REGISTER,
            file,
            at_info_get(__at_info_product_mac),
            at_mark_get(__at_mark_cid_mid),
            at_mark_get(__at_mark_cid_psn));
    unlink(file);
    if (err) {
        return -ECURLFAIL;
    }

    obj = jobj(line);
    if (NULL==obj) {
        err = -EFAKESEVER; goto error;
    }

    code = jcheck(obj);
    if (code) {
        err = -EFAKESEVER; goto error;
    }

    jobj_t jguid= jobj_get(obj, "guid");
    if (NULL==jguid || NULL==__otp_get_bystring(private, jobj_string(jguid))) {
        err = -EFAKESEVER; goto error;
    }

    __otp_call(__otp_private_write, private);
    debug_ok("register ok");
error:
    jobj_put(obj);

    if (code) {
        do_hacked(custom, private, code);
    }

    if (0==err) {
        err = -ENOAUTH; /* not return 0 */
    }
    
    return err; 
}

static int
do_auth(unsigned char custom[], unsigned char private[])
{
    char line[1+OS_LINE_LEN] = {0};
    char *file;
    jobj_t obj = NULL;
    int err, code = 0;

    file = at_secookie_file(AT_CERT_LSS);
    if (NULL==file) {
        return -ENOEXIST;
    }
    
    err = os_v_spgets(line, OS_LINE_LEN, 
            "curl -k -cert %s"
            " -u " PLT_USERPWD
            " -F mac=%s"
            " -F mid=%d"
            " -F psn=%d"
            " -F guid=%s"
            " -s " PLT_URL PLT_SERVICE_AUTH,
            file,
            at_info_get(__at_info_product_mac),
            at_mark_get(__at_mark_cid_mid),
            at_mark_get(__at_mark_cid_psn),
            __otp_string(private));
    unlink(file);
    if (err) {
        return -ECURLFAIL;
    }
    
    obj = jobj(line);
    if (NULL==obj) {
        err = -EFAKESEVER; goto error;
    }

    code = jcheck(obj);
    if (code) {
        err = -EFAKESEVER; goto error;
    }

    jobj_t jguid= jobj_get(obj, "guid");
    if (NULL==jguid || 0!=os_strcmp(__otp_string(private), jobj_string(jguid))) {
        err = -EFAKESEVER; goto error;
    }

    debug_ok("auth ok");
error:
    jobj_put(obj);

    if (code) {
        do_hacked(custom, private, code);
    }

    return err;
}

static bool
load_ok(void)
{
    return is_good_macstring(at_info_get(__at_info_product_mac))
        && at_mark_get(__at_mark_cid_mid)
        && at_mark_get(__at_mark_cid_psn);
}

static void
load_atenv(void)
{
    static bool ok = false;
    
    while(false==ok) {
        at_open();
        at_load_mark();
        at_load_info_named();
        at_close();

        ok = load_ok();
        if (ok) {
            debug_ok("load atenv ok");
            
            return;
        }
        
        sleep(5);
    }
}

static int
do_check(void)
{
    int err;
    unsigned char custom[AT_OTP_SIZE] = {0};
    unsigned char private[AT_OTP_SIZE] = {0};
    
    err = __otp_call(__otp_custom_read, custom);
    if (err) {
        return err;
    }
    
    err = __otp_call(__otp_private_read, private);
    if (err) {
        return err;
    }
    
    if (__otp_eq(custom, AT_OTP_ZERO)) {
        /*
        * custom  is ZERO
        */
        __otp_call(__otp_custom_write, AT_OTP_CUSTOM);

        return -ENOAUTH;
    }

    /*
    * maybe load fail,
    *
    * so, do it at here
    */
    load_atenv();

    if (false==__otp_eq(custom, AT_OTP_CUSTOM)) {
        /*
        * custom is HACKED
        *
        * report it
        */
        debug_trace("custom is HACKED");
        
        return do_report(custom, private, HACKED_CUSTOM);
    }
    else if (__otp_eq(private, AT_OTP_ZERO)) {
        /*
        * custom  is CUSTOM
        * private is ZERO
        *
        * register it
        */
        debug_trace("private is ZERO");
        
        return do_register(custom, private);
    }
    else {
        /*
        * custom  is CUSTOM
        * private is PRIVATE
        *
        * auth it
        */
        debug_trace("private is PRIVATE");
        
        return do_auth(custom, private);
    }
}

static int
lock(void)
{
    fd_lock = os_lock_file(FILE_LOCK);
    if (is_good_fd(fd_lock)) {
        return 0;
    } else {
        return fd_lock;
    }
}

static int
unlock(void)
{
    return os_unlock_file(FILE_LOCK, fd_lock);
}

static int
__fini(void)
{
    unlock();

    os_fini();

    return 0;
}

static int
__init(void)
{
    int err;

    err = os_init();
    if (err) {
        return err;
    }

    err = lock();
    if (err) {
        return err;
    }

    return 0;
}

static void
__exit(int sig)
{
    unlock();
    
    exit(sig);
}

#ifndef __BUSYBOX__
#define atss_main  main
#endif

static int
__main(int argc, char *argv[])
{
    os_setup_signal_exit(__exit);
    
    while(do_check()) {
        sleep(5);
    }

    os_wait_forever();

    return 0;
}

/*
* cmd have enabled when boot
*/
int atss_main(int argc, char *argv[])
{
    return os_call(__init, __fini, __main, argc, argv);
}
/******************************************************************************/
