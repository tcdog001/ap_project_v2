#ifndef __JCMD_H_051CC8DF3E4CE6CDA76BA445D7332481__
#define __JCMD_H_051CC8DF3E4CE6CDA76BA445D7332481__
#ifdef __APP__
/******************************************************************************/
#ifndef JCMD_RESSIZE
#define JCMD_RESSIZE         (32*1024 - 1)
#endif

typedef struct {
    int err;
    int len;
    char json[1+JCMD_RESSIZE];
} jcmd_response_t;

#define jcmd_res_header_size    offsetof(jcmd_response_t, json)

#ifndef INLINE_VAR_JCMD_RES
#define INLINE_VAR_JCMD_RES      ____INLINE_VAR_JCMD_RES
#endif

#define DECLARE_FAKE_JCMD_RES    extern jcmd_response_t INLINE_VAR_JCMD_RES
#define DECLARE_REAL_JCMD_RES    jcmd_response_t INLINE_VAR_JCMD_RES

#ifdef __BUSYBOX__
#define DECLARE_JCMD_RES     DECLARE_FAKE_JCMD_RES
#else
#define DECLARE_JCMD_RES     DECLARE_REAL_JCMD_RES
#endif

DECLARE_FAKE_JCMD_RES;

static inline jcmd_response_t *
__jcmd_res(void)
{
#ifdef USE_INLINE_JCMD_RES
    return &INLINE_VAR_JCMD_RES;
#else
    return NULL;
#endif
}

#define jcmd_res_err     __jcmd_res()->err
#define jcmd_res_json   __jcmd_res()->json
#define jcmd_res_len     __jcmd_res()->len

#define jcmd_res_size    (jcmd_res_header_size + jcmd_res_len)

#define jcmd_res_dump()    \
    debug_trace("jcmd response err:%d, len:%d, json:%s", \
        jcmd_res_err, \
        jcmd_res_len, \
        jcmd_res_json)

#define jcmd_res_zero()  os_objzero(__jcmd_res())

#define jcmd_res_clear() do{ \
    jcmd_res_json[0] = 0;   \
    jcmd_res_len = 0;        \
}while(0)
#define jcmd_res_error(_err)     (jcmd_res_err=(_err))
#define jcmd_res_ok              jcmd_res_error(0)

#define jcmd_res_sprintf(_fmt, _args...) ({ \
    int __len = os_snprintf(                \
            jcmd_res_json + jcmd_res_len,   \
            JCMD_RESSIZE - jcmd_res_len,    \
            _fmt, ##_args);                 \
    jcmd_res_len += __len;                  \
                                            \
    __len;                                  \
})

#define jcmd_res_recv(_fd, _timeout) \
    io_recv(_fd, __jcmd_res(), sizeof(jcmd_response_t), _timeout)

#define jcmd_res_send(_fd) \
    io_send(_fd, __jcmd_res(), jcmd_res_size)

#define jcmd_res_recvfrom(_fd, _timeout, _addr, _paddrlen) \
    io_recvfrom(_fd, __jcmd_res(), sizeof(jcmd_response_t), _timeout, _addr, _paddrlen)

#define jcmd_res_sendto(_fd, _addr, _addrlen) \
    io_sendto(_fd, __jcmd_res(), jcmd_res_size, _addr, _addrlen)

typedef struct {
    int timeout;
    
    struct sockaddr_un server, client;
} jcmd_client;

#define JCMD_CLIENT_INITER(_timeout, _server_path) { \
    .server     = OS_SOCKADDR_UNIX("\0" _server_path),  \
    .client     = OS_SOCKADDR_UNIX(__empty),            \
    .timeout    = _timeout,                             \
}   /* end */
/******************************************************************************/
#endif /* __APP__ */
#endif /* __JCMD_H_051CC8DF3E4CE6CDA76BA445D7332481__ */
