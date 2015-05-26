#ifndef __AT_OTP_H_739039F132EB8833CE84495D183364C7__
#define __AT_OTP_H_739039F132EB8833CE84495D183364C7__
#include "utils.h"
#ifdef __APP__
/******************************************************************************/
#include "hi_unf_otp.h"
#include "hi_unf_advca.h"

#define AT_OTP_SIZE         16
#define AT_OTP_ZERO         "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
#define AT_OTP_PRIVATE      "\x73\x90\x39\xF1\x32\xEB\x88\x33\xCE\x84\x49\x5D\x18\x33\x64\xC7"
#define AT_OTP_CUSTOM       "www.autelan.com"

enum {
    AT_OTP_ERR_HACKED   = 1,
    AT_OTP_ERR_READ     = 2,
    AT_OTP_ERR_WRITE    = 3,
};

#define AT_OTP_DEBUG    0

#if AT_OTP_DEBUG
#define dprintln(_fmt, _args...)  os_println(_fmt, ##_args)
#else
#define dprintln(_fmt, _args...)  os_do_nothing()
#endif

#define __otp_eq(_a, _b)    os_memeq(_a, _b, AT_OTP_SIZE)

static inline char *
__otp_string(unsigned char otp[AT_OTP_SIZE])
{
    static char string[2*AT_OTP_SIZE+1];

    os_sprintf(string,
        "%.2X%.2X%.2X%.2X"
        "%.2X%.2X%.2X%.2X"
        "%.2X%.2X%.2X%.2X"
        "%.2X%.2X%.2X%.2X"
        ,
        otp[0],  otp[1],  otp[2],  otp[3],
        otp[4],  otp[5],  otp[6],  otp[7],
        otp[8],  otp[9],  otp[10], otp[11],
        otp[12], otp[13], otp[14], otp[15]);

    return string;
}

static inline unsigned char *
__otp_get_bystring(unsigned char otp[AT_OTP_SIZE], char *string)
{
    int len;

    len = os_strlen(string);
    if (2*AT_OTP_SIZE != len) {
        return NULL;
    }
    
    len = os_sscanf(string,
        "%.2X%.2X%.2X%.2X"
        "%.2X%.2X%.2X%.2X"
        "%.2X%.2X%.2X%.2X"
        "%.2X%.2X%.2X%.2X"
        ,
        &otp[0],  &otp[1],  &otp[2],  &otp[3],
        &otp[4],  &otp[5],  &otp[6],  &otp[7],
        &otp[8],  &otp[9],  &otp[10], &otp[11],
        &otp[12], &otp[13], &otp[14], &otp[15]);
    if (AT_OTP_SIZE!=len) {
        return NULL;
    }
    
    return otp;
}

static inline void
__otp_dump(unsigned char otp[AT_OTP_SIZE])
{
    os_println("%s", __otp_string(otp));
}

static inline int
__otp_init(void)
{
    int err;

    do{
        err = HI_UNF_OTP_Init();
            debug_trace_error(err, "otp init");
        if (err) {
            
            sleep(1);
        }
    } while(err);
    
    return 0;
}

static inline int
__otp_fini(void)
{
    int err;
    
    err = HI_UNF_OTP_DeInit();
        debug_trace_error(err, "otp fini");
    if (err) {
    }

    return err;
}

static inline int
__otp_custom_read(unsigned char otp[AT_OTP_SIZE])
{
    int err;

    err = HI_UNF_OTP_GetCustomerKey(otp, AT_OTP_SIZE);
        debug_trace_error(err, "read custom");
    if (err) {
        
        err = AT_OTP_ERR_READ;
    }
    
    return err;
}

static inline int
__otp_custom_write(unsigned char otp[AT_OTP_SIZE])
{
    int err;

    err = HI_UNF_OTP_SetCustomerKey(otp, AT_OTP_SIZE);
        debug_trace_error(err, "write custom");
    if (err) {
        err = AT_OTP_ERR_WRITE;
    } else {
        os_println("custom inited.");
    }

    return err;
}

static inline int
__otp_private_read(unsigned char otp[AT_OTP_SIZE])
{
    int i, err, errs = 0;

    for (i=0; i<AT_OTP_SIZE; i++) {
        err = HI_UNF_OTP_GetStbPrivData(i, &otp[i]);
        if (err) {
            errs = err;
        }
    }
    
    debug_trace_error(errs, "read private");
    if (errs) {
        
        errs = AT_OTP_ERR_READ;
    }

    return errs;
}

static inline int
__otp_private_write(unsigned char otp[AT_OTP_SIZE])
{
    int i, err, errs = 0;

    for (i=0; i<AT_OTP_SIZE; i++) {
        err = HI_UNF_OTP_SetStbPrivData(i, otp[i]);
        if (err) {
            errs = err;
        }
    }
    
    debug_trace_error(errs, "write private");
    if (errs) {
        errs = AT_OTP_ERR_WRITE;
    } else {
        os_println("private inited.");
    }

    return errs;
}

static inline int
__otp_show(unsigned char otp[AT_OTP_SIZE], int (*read)(unsigned char otp[]), bool string)
{
    int err = (*read)(otp);
    if (0==err) {
        os_println("%s", string?(char *)otp:__otp_string(otp));
    }

    return err;
}

static inline int
__otp_check(
    unsigned char val[AT_OTP_SIZE], 
    int (*read)(unsigned char otp[]), 
    int (*write)(unsigned char otp[])
)
{
    unsigned char otp[AT_OTP_SIZE] = {0};
    int err;

    err = (*read)(otp);
    if (err) {
        return err;
    }

    if (__otp_eq(otp, AT_OTP_ZERO)) {
        dprintln("otp is zero");

        if (__otp_eq(val, AT_OTP_ZERO)) {
            dprintln("try set otp zero");

            return 0;
        }

        err = (*write)(val);
        if (err) {
            return err;
        }

        return 0;
    }
    else if (__otp_eq(otp, val)) {
        dprintln("otp set==get");

        return 0;
    }
    else {
        dprintln("otp is hacked");

        return AT_OTP_ERR_HACKED;
    }
}

#define __otp_call(_call, _otp, _args...)   os_call(__otp_init, __otp_fini, _call, _otp, ##_args)

static inline int
at_otp_custom_show(void)
{
    unsigned char otp[AT_OTP_SIZE] = {0};
    
    return __otp_call(__otp_show, otp, __otp_custom_read, true);
}

static inline int
at_otp_custom_check(void)
{
    return __otp_call(__otp_check, AT_OTP_CUSTOM, __otp_custom_read, __otp_custom_write);
}

static inline int
at_otp_custom_write(unsigned char val[AT_OTP_SIZE])
{
    return __otp_call(__otp_custom_write, val);
}

static inline int
at_otp_private_show(void)
{
    unsigned char otp[AT_OTP_SIZE] = {0};
 
    return __otp_call(__otp_show, otp, __otp_private_read, false);
}

static inline int
at_otp_private_check(unsigned char val[AT_OTP_SIZE])
{
    return __otp_call(__otp_check, val, __otp_private_read, __otp_private_write);
}

static inline int
at_otp_private_write(unsigned char val[AT_OTP_SIZE])
{
    return __otp_call(__otp_private_write, val);
}
/******************************************************************************/
#endif /* __APP__ */
#endif /* __AT_OTP_H_739039F132EB8833CE84495D183364C7__ */
