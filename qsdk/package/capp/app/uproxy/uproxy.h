#ifndef __UPROXY_H_755D96B153E0AE33BDB7D6FF92ABCD97__
#define __UPROXY_H_755D96B153E0AE33BDB7D6FF92ABCD97__
/******************************************************************************/
#include <libubox/uloop.h>
#include <libubox/utils.h>
#include <uci_blob.h>
#include <libubus.h>


struct uproxy_control {
    struct ubus_context *ctx;
    char *path;
    struct ubus_event_handler listener;
    struct ubus_subscriber subscriber;
    
    struct {
        
    } policy;

    struct {
        appkey_t uci;
        appkey_t ubus;
        appkey_t inet;
    } debug;
};

extern struct uproxy_control uproxy;
extern struct blob_buf b;


/******************************************************************************/
#define uproxy_debug(_var, _fmt, _args...)  do{ \
    if (appkey_get(uproxy.debug._var, 0)) { \
        __debug_with_prefix(_fmt, ##_args); \
    }                                       \
}while(0)

#define uproxy_ok(_var, _fmt, _args...) do{ \
    if (appkey_get(uproxy.debug._var, 0)) { \
        debug_ok(_fmt, ##_args);            \
    }                                       \
}while(0)

#define uproxy_error(_var, _fmt, _args...)  do{ \
    if (appkey_get(uproxy.debug._var, 0)) { \
        debug_error(_fmt, ##_args);         \
    }                                       \
}while(0)

#define uproxy_trace(_var, _fmt, _args...)  do{ \
    if (appkey_get(uproxy.debug._var, 0)) { \
        debug_trace(_fmt, ##_args);         \
    }                                       \
}while(0)

#define uproxy_test(_var, _fmt, _args...)   do{ \
    if (appkey_get(uproxy.debug._var, 0)) { \
        debug_test(_fmt, ##_args);          \
    }                                       \
}while(0)
/******************************************************************************/
#endif /* __UPROXY_H_755D96B153E0AE33BDB7D6FF92ABCD97__ */
