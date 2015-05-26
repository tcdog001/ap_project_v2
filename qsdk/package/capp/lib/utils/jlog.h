#ifndef __JLOG_H_1E53C7301905156FEF0817624CC8964D__
#define __JLOG_H_1E53C7301905156FEF0817624CC8964D__
#include "utils/jsonapi.h"
/******************************************************************************/
#ifndef JLOG_UNIX
#define JLOG_UNIX               "/tmp/.jlog.unix"
#endif

#ifndef JLOG_BUFSIZE
#define JLOG_BUFSIZE            (1*1024*1024-1)
#endif

#ifndef JLOG_PRI
#define JLOG_PRI                LOG_CRIT
#endif

#ifndef JLOG_TIMEOUT
#define JLOG_TIMEOUT            500
#endif

#ifndef JLOG_CUTCOUNT
#ifdef __PC__
#define JLOG_CUTCOUNT           10
#else
#define JLOG_CUTCOUNT           1000
#endif
#endif

#ifndef JLOG_PATH
#ifdef __PC__
#define JLOG_PATH               "/tmp/jlog"
#else
#define JLOG_PATH               "/tmp/log"
#endif
#endif

#ifndef JLOG_FILE_LOCAL
#define JLOG_FILE_LOCAL         JLOG_PATH "/md.log"
#endif

#ifndef JLOG_CACHE_LOCAL
#define JLOG_CACHE_LOCAL        JLOG_PATH "/.md.log"
#endif

#ifndef JLOG_FILE_REMOTE
#define JLOG_FILE_REMOTE        JLOG_PATH "/ap.log"
#endif

#ifndef JLOG_CACHE_REMOTE
#define JLOG_CACHE_REMOTE       JLOG_PATH "/.ap.log"
#endif

#ifndef JLOG_IP
#ifdef __PC__
#define JLOG_IP                 "0.0.0.0"
#else
#define JLOG_IP                 "1.0.0.6"
#endif
#endif

#ifndef JLOG_PORT
#define JLOG_PORT               4110
#endif

#ifndef JLOG_FAMILY
#define JLOG_FAMILY             AF_UNIX
#endif

#ifndef ENV_JLOG_UNIX
#define ENV_JLOG_UNIX           "__JLOG_UNIX__"
#endif

#ifndef ENV_JLOG_IP
#define ENV_JLOG_IP             "__JLOG_IP__"
#endif

#ifndef ENV_JLOG_PORT
#define ENV_JLOG_PORT           "__JLOG_PORT__"
#endif

#ifndef ENV_JLOG_FAMILY
#define ENV_JLOG_FAMILY         "__JLOG_FAMILY__"
#endif

#ifndef ENV_JLOG_PRI
#define ENV_JLOG_PRI            "__JLOG_PRI__"
#endif

#ifndef ENV_JLOG_CUTCOUNT
#define ENV_JLOG_CUTCOUNT       "__JLOG_CUTCOUNT__"
#endif

#ifndef ENV_JLOG_TIMEOUT
#define ENV_JLOG_TIMEOUT        "__JLOG_TIMEOUT__"
#endif

#ifndef ENV_JLOG_FILE_LOCAL
#define ENV_JLOG_FILE_LOCAL     "__JLOG_FILE_LOCAL__"
#endif

#ifndef ENV_JLOG_CACHE_LOCAL
#define ENV_JLOG_CACHE_LOCAL    "__JLOG_CACHE_LOCAL__"
#endif

#ifndef ENV_JLOG_FILE_REMOTE
#define ENV_JLOG_FILE_REMOTE    "__JLOG_FILE_REMOTE__"
#endif

#ifndef ENV_JLOG_CACHE_REMOTE
#define ENV_JLOG_CACHE_REMOTE   "__JLOG_CACHE_REMOTE__"
#endif

#define JLOG_KEY_APP        "appname"
#define JLOG_KEY_SUB        "appsubname"
#define JLOG_KEY_FILE       "appfile"
#define JLOG_KEY_FUNC       "appfunc"
#define JLOG_KEY_LINE       "appline"
#define JLOG_KEY_LEVEL      "applevel"
#define JLOG_KEY_INFO       "appinfo"
#define JLOG_KEY_PRI        "apppri"
#define JLOG_KEY_TIME       "time"
#define JLOG_KEY_MAC        "mac"
#define JLOG_KEY_SEQ        "seq"

enum {
    __debug_level_ok        = (1<<0),  /* 成功 */
    __debug_level_bug       = (1<<1),  /* BUG  */
    __debug_level_error     = (1<<2),  /* 错误 */
    __debug_level_trace     = (1<<3),  /* 跟踪 */
    __debug_level_event     = (1<<4),  /* 事件 */
    __debug_level_entry     = (1<<5),  /* 表项 */
    __debug_level_packet    = (1<<6),  /* 报文 */
    __debug_level_signal    = (1<<7),  /* 信号 */
    __debug_level_timeout   = (1<<8),  /* 超时 */
    __debug_level_shell     = (1<<9),  /* system/popen */
    __debug_level_test      = (1<<10),  /* 测试 */
    __debug_level_consume   = (1<<11),  /* 计算cpu消耗 */
};

#define __debug_level_all   ( \
    __debug_level_ok        | \
    __debug_level_bug       | \
    __debug_level_error     | \
    __debug_level_trace     | \
    __debug_level_event     | \
    __debug_level_entry     | \
    __debug_level_packet    | \
    __debug_level_signal    | \
    __debug_level_timeout   | \
    __debug_level_shell     | \
    __debug_level_test      | \
    __debug_level_consume   | \
0)

#ifndef __debug_level_default
#define __debug_level_default   __debug_level_error
#endif

#define __debug_level_maps { \
    OS_KV_INITER("ok",      __debug_level_ok),      \
    OS_KV_INITER("bug",     __debug_level_bug),     \
    OS_KV_INITER("error",   __debug_level_error),   \
    OS_KV_INITER("trace",   __debug_level_trace),   \
    OS_KV_INITER("event",   __debug_level_event),   \
    OS_KV_INITER("entry",   __debug_level_entry),   \
    OS_KV_INITER("packet",  __debug_level_packet),  \
    OS_KV_INITER("signal",  __debug_level_signal),  \
    OS_KV_INITER("timeout", __debug_level_timeout), \
    OS_KV_INITER("shell",   __debug_level_shell),   \
    OS_KV_INITER("test",    __debug_level_test),    \
    OS_KV_INITER("consume", __debug_level_consume), \
}   /* end */

static inline unsigned int
__get_debug_level_byname(char *name)
{
    static os_kv_t map[] = __debug_level_maps;
    int i;

    for (i=0; i<os_count_of(map); i++) {
        if (0==os_strcmp(name, map[i].k)) {
            return map[i].v;
        }
    }

    return 0;
}

static inline char *
__get_debug_level_name(int level)
{
    static os_kv_t map[] = __debug_level_maps;
    int i;

    for (i=0; i<os_count_of(map); i++) {
        if (level & map[i].v) {
            return map[i].k;
        }
    }

    return __unknow;
}

#ifdef __BOOT__
#define	LOG_EMERG	0	/* system is unusable */
#define	LOG_ALERT	1	/* action must be taken immediately */
#define	LOG_CRIT	2	/* critical conditions */
#define	LOG_ERR		3	/* error conditions */
#define	LOG_WARNING	4	/* warning conditions */
#define	LOG_NOTICE	5	/* normal but significant condition */
#define	LOG_INFO	6	/* informational */
#define	LOG_DEBUG	7	/* debug-level messages */
#define	LOG_PRIMASK	0x07	/* mask to extract priority part (internal) */
				/* extract priority */
#define	LOG_PRI(p)	((p) & LOG_PRIMASK)

#define	LOG_MAKEPRI(fac, pri)	    (((fac) << 3) | (pri))
#endif

#define __LOG_LEVELMASK             __debug_level_all
/*
* get private from PRI
*/
#define __LOG_PRIVATE(_PRI)         ((_PRI) >> 3)
/*
* make valid level
*/
#define __LOG_LEVEL(_level)         ((_level) & __LOG_LEVELMASK)

/*
* get level from PRI
*/
#define LOG_LEVEL(_PRI)             __LOG_LEVEL(__LOG_PRIVATE(_PRI))
/*
* make PRI by level & pri
*/
#define LOG_LEVELPRI(_level, _pri)  LOG_MAKEPRI(__LOG_LEVEL(_level), _pri)
#define LOG_DEBUGLEVELPRI(_level)   LOG_LEVELPRI(_level, LOG_DEBUG)

#ifdef __BOOT__
    extern unsigned int *__AKID_DEBUG;   /* atenv_boot_common.c */
#   define __debug_level        (__AKID_DEBUG?(*__AKID_DEBUG):__debug_level_default)
#elif defined(__KERNEL__) || defined(__LIBAPPKEY__)
    /*
    * in boot/kernel/libappkey, should define __debug_level_pointer as below
    *   int xxx;
    *   static int *__debug_level_pointer = &xxx;
    */
#   define __debug_level        (*__debug_level_pointer)
#elif defined(__APP__)
    /*
    * for all app/lib(defined __THIS_NAME)
    */
    extern appkey_t __AKID_DEBUG;
#   define __debug_level        appkey_get(__AKID_DEBUG, __debug_level_default)
#endif

#define __is_debug_level(_level)    (!!(__debug_level & (_level)))
#define __is_debug_level_ok         __is_debug_level(__debug_level_ok)
#define __is_debug_level_bug        __is_debug_level(__debug_level_bug)
#define __is_debug_level_error      __is_debug_level(__debug_level_error)
#define __is_debug_level_trace      __is_debug_level(__debug_level_trace)
#define __is_debug_level_event      __is_debug_level(__debug_level_event)
#define __is_debug_level_entry      __is_debug_level(__debug_level_entry)
#define __is_debug_level_packet     __is_debug_level(__debug_level_packet)
#define __is_debug_level_signal     __is_debug_level(__debug_level_signal)
#define __is_debug_level_timeout    __is_debug_level(__debug_level_timeout)
#define __is_debug_level_shell      __is_debug_level(__debug_level_shell)
#define __is_debug_level_test       __is_debug_level(__debug_level_test)
#define __is_debug_level_consume    __is_debug_level(__debug_level_consume)

static inline void
__jlog_header(char *app, char *sub, const char *file, const char *func, int line, int PRI)
{
    int level = LOG_LEVEL(PRI);
    
    os_printf("[app:%s", app);
    if (sub) {
        os_printf(",sub:%s", sub);
    }
    if (file) {
        os_printf(",file:%s", file);
    }
    if (func) {
        os_printf(",func:%s", func);
    }
    if (line) {
        os_printf(",line:%d", line);
    }
    if (level) {
        os_printf(",level:%s", __get_debug_level_name(level));
    }
    os_printf("] ");
}

#define __jlog_vprintf(_app, _sub, _file, _func, _line, _PRI, _fmt, _args) ({ \
    __jlog_header(_app, _sub, _file, _func, _line, _PRI); \
    os_evprintln(_fmt, _args);  \
    0;                          \
})  /* end */

#define __jlog_printf(_app, _sub, _file, _func, _line, _PRI, _fmt, _args...) ({ \
    __jlog_header(_app, _sub, _file, _func, _line, _PRI); \
    os_eprintln(_fmt, ##_args); \
    0;                          \
})  /* end */

#define jlog_vprintf(_sub, _PRI, _fmt, _args)     \
    __jlog_vprintf(__THIS_NAME, _sub, __THIS_FILE, __func__, __LINE__, _PRI, _fmt, ##_args)

#define jlog_printf(_sub, _PRI, _fmt, _args...)   \
    __jlog_printf(__THIS_NAME, _sub, __THIS_FILE, __func__, __LINE__, _PRI, _fmt, ##_args)

#ifdef __APP__
typedef struct {
    int family;
    int fd;

    os_sockaddr_t server;
} jlog_control_t;

#define JLOG_CONTROL_INITER   { \
    .family = INVALID_VALUE,    \
    .fd     = INVALID_FD,       \
}   /* end */

#ifndef VAR_INLINE_JLOG
#define VAR_INLINE_JLOG     ____VAR_INLINE_JLOG
#endif

#define DECLARE_REAL_JLOG   jlog_control_t VAR_INLINE_JLOG = JLOG_CONTROL_INITER
#define DECLARE_FAKE_JLOG   extern jlog_control_t VAR_INLINE_JLOG

#ifdef __BUSYBOX__
#define DECLARE_JLOG        DECLARE_FAKE_JLOG
#else
#define DECLARE_JLOG        DECLARE_REAL_JLOG
#endif

DECLARE_FAKE_JLOG;

static inline jlog_control_t *
__jlog_control(void)
{
    return &VAR_INLINE_JLOG;
}

static inline int
__jlog_fd(void)
{
    return __jlog_control()->fd;
}

static inline int
__jlog_family(void)
{
    return __jlog_control()->family;
}

static inline struct sockaddr_un *
__jlog_userver(void)
{
    return &__jlog_control()->server.un;
}

static inline struct sockaddr_in *
__jlog_iserver(void)
{
    return &__jlog_control()->server.in;
}

static inline struct sockaddr *
__jlog_server(void)
{
    return &__jlog_control()->server.c;
}

static inline int
__jvlogger(char *app, char *sub, const char *file, const char *func, int line, int PRI, const char *fmt, va_list args);

static inline int
__dvlogger(char *app, char *sub, const char *file, const char *func, int line, int PRI, const char *fmt, va_list args);

static inline int
__jlogger(char *app, char *sub, const char *file, const char *func, int line, int PRI, const char *fmt, ...);

static inline int
__dlogger(char *app, char *sub, const char *file, const char *func, int line, int PRI, const char *fmt, ...);
#else
#define DECLARE_JLOG     os_use_fake_value

#define __jvlogger(_app, _sub, _file, _func, _line, _PRI, _fmt, _args) \
        __jlog_vprintf(_app, _sub, _file, _func, _line, _PRI, _fmt, _args)

#define __dvlogger(_app, _sub, _file, _func, _line, _PRI, _fmt, _args) \
        __jlog_vprintf(_app, _sub, _file, _func, _line, _PRI, _fmt, _args)

#define __jlogger(_app, _sub, _file, _func, _line, _PRI, _fmt, _args...) \
        __jlog_printf(_app, _sub, _file, _func, _line, _PRI, _fmt, ##_args)

#define __dlogger(_app, _sub, _file, _func, _line, _PRI, _fmt, _args...) \
        __jlog_printf(_app, _sub, _file, _func, _line, _PRI, _fmt, ##_args)
#endif

#define jvlogger(_sub, _PRI, _fmt, _args)     \
    __jvlogger(__THIS_NAME, _sub, __THIS_FILE, __func__, __LINE__, _PRI, _fmt, ##_args)

#define dvlogger(_sub, _PRI, _fmt, _args)     \
    __dvlogger(__THIS_NAME, _sub, __THIS_FILE, __func__, __LINE__, _PRI, _fmt, ##_args)

#define jlogger(_sub, _PRI, _fmt, _args...)   \
    __jlogger(__THIS_NAME, _sub, __THIS_FILE, __func__, __LINE__, _PRI, _fmt, ##_args)

#define dlogger(_sub, _PRI, _fmt, _args...)   \
    __dlogger(__THIS_NAME, _sub, __THIS_FILE, __func__, __LINE__, _PRI, _fmt, ##_args)

#define __jvemerg(_sub, _fmt, _args)        jvlogger(_sub, LOG_EMERG, _fmt, _args)
#define __jvalert(_sub, _fmt, _args)        jvlogger(_sub, LOG_ALERT, _fmt, _args)
#define __jvcrit(_sub, _fmt, _args)         jvlogger(_sub, LOG_CRIT, _fmt, _args)
#define __jverror(_sub, _fmt, _args)        jvlogger(_sub, LOG_ERR, _fmt, _args)
#define __jvwarning(_sub, _fmt, _args)      jvlogger(_sub, LOG_WARNING, _fmt, _args)
#define __jvnotice(_sub, _fmt, _args)       jvlogger(_sub, LOG_NOTICE, _fmt, _args)
#define __jvinfo(_sub, _fmt, _args)         jvlogger(_sub, LOG_INFO, _fmt, _args)
#define __jvdebug(_sub, _fmt, _args)        jvlogger(_sub, LOG_DEBUG, _fmt, _args)

#define __jemerg(_sub, _fmt, _args...)      jlogger(_sub, LOG_EMERG, _fmt, ##_args)
#define __jalert(_sub, _fmt, _args...)      jlogger(_sub, LOG_ALERT, _fmt, ##_args)
#define __jcrit(_sub, _fmt, _args...)       jlogger(_sub, LOG_CRIT, _fmt, ##_args)
#define __jerror(_sub, _fmt, _args...)      jlogger(_sub, LOG_ERR, _fmt, ##_args)
#define __jwarning(_sub, _fmt, _args...)    jlogger(_sub, LOG_WARNING, _fmt, ##_args)
#define __jnotice(_sub, _fmt, _args...)     jlogger(_sub, LOG_NOTICE, _fmt, ##_args)
#define __jinfo(_sub, _fmt, _args...)       jlogger(_sub, LOG_INFO, _fmt, ##_args)
#define __jdebug(_sub, _fmt, _args...)      jlogger(_sub, LOG_DEBUG, _fmt, ##_args)

#define jvemerg(_fmt, _args)        __jvemerg(NULL, _fmt, _args)
#define jvalert(_fmt, _args)        __jvalert(NULL, _fmt, _args)
#define jvcrit(_fmt, _args)         __jvcrit(NULL, _fmt, _args)
#define jverror(_fmt, _args)        __jverror(NULL, _fmt, _args)
#define jvwarning(_fmt, _args)      __jvwarning(NULL, _fmt, _args)
#define jvnotice(_fmt, _args)       __jvnotice(NULL, _fmt, _args)
#define jvinfo(_fmt, _args)         __jvinfo(NULL, _fmt, _args)
#define jvdebug(_fmt, _args)        __jvdebug(NULL, _fmt, _args)

#define jemerg(_fmt, _args...)      __jemerg(NULL, _fmt, ##_args)
#define jalert(_fmt, _args...)      __jalert(NULL, _fmt, ##_args)
#define jcrit(_fmt, _args...)       __jcrit(NULL, _fmt, ##_args)
#define jerror(_fmt, _args...)      __jerror(NULL, _fmt, ##_args)
#define jwarning(_fmt, _args...)    __jwarning(NULL, _fmt, ##_args)
#define jnotice(_fmt, _args...)     __jnotice(NULL, _fmt, ##_args)
#define jinfo(_fmt, _args...)       __jinfo(NULL, _fmt, ##_args)
#define jdebug(_fmt, _args...)      __jdebug(NULL, _fmt, ##_args)

#define __jvdlogger(_logger, _level, _fmt, _args)  do{ \
    if (__is_debug_level(_level)) {                  \
        _logger(NULL, LOG_DEBUGLEVELPRI(_level), _fmt, _args);  \
    }                                               \
}while(0)

#define __jdlogger(_logger, _level, _fmt, _args...)  do{ \
    if (__is_debug_level(_level)) {                  \
        _logger(NULL, LOG_DEBUGLEVELPRI(_level), _fmt, ##_args); \
    }                                               \
}while(0)

#if defined(__JLOGGER__) || defined(__JLOGD__) || defined(__LIBAPPKEY__)  || defined(__BOOT__)
#define jvdlogger(_level, _fmt, _args)      __jvdlogger(jlog_vprintf, _level, _fmt, _args)
#define jdlogger(_level, _fmt, _args...)    __jdlogger(jlog_printf, _level, _fmt, ##_args)
#else
#define jvdlogger(_level, _fmt, _args)      __jvdlogger(dvlogger, _level, _fmt, _args)
#define jdlogger(_level, _fmt, _args...)    __jdlogger(dlogger, _level, _fmt, ##_args)
#endif

/*
* use for logger inline function
*/
#define __vdebug_ok(_fmt, _args)        __jvdlogger(jlog_vprintf, __debug_level_ok, _fmt, _args)
#define __vdebug_bug(_fmt, _args)       __jvdlogger(jlog_vprintf, __debug_level_bug, _fmt, _args)
#define __vdebug_error(_fmt, _args)     __jvdlogger(jlog_vprintf, __debug_level_error, _fmt, _args)
#define __vdebug_trace(_fmt, _args)     __jvdlogger(jlog_vprintf, __debug_level_trace, _fmt, _args)
#define __vdebug_event(_fmt, _args)     __jvdlogger(jlog_vprintf, __debug_level_event, _fmt, _args)
#define __vdebug_entry(_fmt, _args)     __jvdlogger(jlog_vprintf, __debug_level_entry, _fmt, _args)
#define __vdebug_packet(_fmt, _args)    __jvdlogger(jlog_vprintf, __debug_level_packet, _fmt, _args)
#define __vdebug_signal(_fmt, _args)    __jvdlogger(jlog_vprintf, __debug_level_signal, _fmt, _args)
#define __vdebug_timeout(_fmt, _args)   __jvdlogger(jlog_vprintf, __debug_level_timeout, _fmt, _args)
#define __vdebug_shell(_fmt, _args)     __jvdlogger(jlog_vprintf, __debug_level_shell, _fmt, _args)
#define __vdebug_test(_fmt, _args)      __jvdlogger(jlog_vprintf, __debug_level_test, _fmt, _args)

#define __debug_ok(_fmt, _args...)      __jdlogger(jlog_printf, __debug_level_ok, _fmt, ##_args)
#define __debug_bug(_fmt, _args...)     __jdlogger(jlog_printf, __debug_level_bug, _fmt, ##_args)
#define __debug_error(_fmt, _args...)   __jdlogger(jlog_printf, __debug_level_error, _fmt, ##_args)
#define __debug_trace(_fmt, _args...)   __jdlogger(jlog_printf, __debug_level_trace, _fmt, ##_args)
#define __debug_event(_fmt, _args...)   __jdlogger(jlog_printf, __debug_level_event, _fmt, ##_args)
#define __debug_entry(_fmt, _args...)   __jdlogger(jlog_printf, __debug_level_entry, _fmt, ##_args)
#define __debug_packet(_fmt, _args...)  __jdlogger(jlog_printf, __debug_level_packet, _fmt, ##_args)
#define __debug_signal(_fmt, _args...)  __jdlogger(jlog_printf, __debug_level_signal, _fmt, ##_args)
#define __debug_timeout(_fmt, _args...) __jdlogger(jlog_printf, __debug_level_timeout, _fmt, ##_args)
#define __debug_shell(_fmt, _args...)   __jdlogger(jlog_printf, __debug_level_shell, _fmt, ##_args)
#define __debug_test(_fmt, _args...)    __jdlogger(jlog_printf, __debug_level_test, _fmt, ##_args)

/*
* use for others
*/
#define vdebug_ok(_fmt, _args)          jvdlogger(__debug_level_ok, _fmt, _args)
#define vdebug_bug(_fmt, _args)         jvdlogger(__debug_level_bug, _fmt, _args)
#define vdebug_error(_fmt, _args)       jvdlogger(__debug_level_error, _fmt, _args)
#define vdebug_trace(_fmt, _args)       jvdlogger(__debug_level_trace, _fmt, _args)
#define vdebug_event(_fmt, _args)       jvdlogger(__debug_level_event, _fmt, _args)
#define vdebug_entry(_fmt, _args)       jvdlogger(__debug_level_entry, _fmt, _args)
#define vdebug_packet(_fmt, _args)      jvdlogger(__debug_level_packet, _fmt, _args)
#define vdebug_signal(_fmt, _args)      jvdlogger(__debug_level_signal, _fmt, _args)
#define vdebug_timeout(_fmt, _args)     jvdlogger(__debug_level_timeout, _fmt, _args)
#define vdebug_shell(_fmt, _args)       jvdlogger(__debug_level_shell, _fmt, _args)
#define vdebug_test(_fmt, _args)        jvdlogger(__debug_level_test, _fmt, _args)

#define debug_ok(_fmt, _args...)        jdlogger(__debug_level_ok, _fmt, ##_args)
#define debug_bug(_fmt, _args...)       jdlogger(__debug_level_bug, _fmt, ##_args)
#define debug_error(_fmt, _args...)     jdlogger(__debug_level_error, _fmt, ##_args)
#define debug_trace(_fmt, _args...)     jdlogger(__debug_level_trace, _fmt, ##_args)
#define debug_event(_fmt, _args...)     jdlogger(__debug_level_event, _fmt, ##_args)
#define debug_entry(_fmt, _args...)     jdlogger(__debug_level_entry, _fmt, ##_args)
#define debug_packet(_fmt, _args...)    jdlogger(__debug_level_packet, _fmt, ##_args)
#define debug_signal(_fmt, _args...)    jdlogger(__debug_level_signal, _fmt, ##_args)
#define debug_timeout(_fmt, _args...)   jdlogger(__debug_level_timeout, _fmt, ##_args)
#define debug_shell(_fmt, _args...)     jdlogger(__debug_level_shell, _fmt, ##_args)
#define debug_test(_fmt, _args...)      jdlogger(__debug_level_test, _fmt, ##_args)

#define __trace_error(_err, _trace_ok, _trace_error, _fmt, _args...) do{    \
    if (_err) {                                     \
        _trace_error("[ERROR:%d]" _fmt, _err, ##_args); \
    } else {                                        \
        _trace_ok("[OK]" _fmt, ##_args);            \
    }                                               \
}while(0)

#define trace_error(_err, _fmt, _args...)           \
        __trace_error(_err, os_eprintln, os_eprintln, _fmt, ##_args)

#define __debug_ok_error(_err, _fmt, _args...)      \
    __trace_error(_err, __debug_ok, __debug_error, _fmt, ##_args)

#define debug_ok_error(_err, _fmt, _args...)        \
    __trace_error(_err, debug_ok, debug_error, _fmt, ##_args)

#define __debug_trace_error(_err, _fmt, _args...)   \
    __trace_error(_err, __debug_trace, __debug_error, _fmt, ##_args)

#define debug_trace_error(_err, _fmt, _args...)     \
    __trace_error(_err, debug_trace, debug_error, _fmt, ##_args)

#define __debug_assert(_condition, _debug, _fmt, _args...) do{ \
    _debug(_fmt, ##_args);  \
    os_assert(_condition);  \
}while(0)

#define __trace_assert(_condition, _fmt, _args...)  \
    __debug_assert(_condition, __debug_trace, _fmt, ##_args)

#define trace_assert(_condition, _fmt, _args...)    \
    __debug_assert(_condition, debug_trace, _fmt, ##_args)

#define __error_assert(_condition, _fmt, _args...)  \
    __debug_assert(_condition, __debug_error, _fmt, ##_args)

#define error_assert(_condition, _fmt, _args...)    \
    __debug_assert(_condition, debug_error, _fmt, ##_args)

#define __debug_assertV(_value, _debug, _fmt, _args...) ({ \
    _debug(_fmt, ##_args);  \
    os_assertV(_value);     \
})  /* end */

#define __trace_assertV(_value, _fmt, _args...) \
    __debug_assertV(_value, __debug_trace, _fmt, ##_args)

#define trace_assertV(_value, _fmt, _args...)   \
    __debug_assertV(_value, debug_trace, _fmt, ##_args)

#define __error_assertV(_value, _fmt, _args...) \
    __debug_assertV(_value, __debug_error, _fmt, ##_args)

#define error_assertV(_value, _fmt, _args...)   \
    __debug_assertV(_value, debug_error, _fmt, ##_args)

#ifdef __APP__
static inline int os_timeval_diff(struct timeval *old, struct timeval *new)
{
    int usec = 0;

    usec += ((int)new->tv_sec  - (int)old->tv_sec) * 1000 * 1000;
    usec += ((int)new->tv_usec - (int)old->tv_usec);
    
    return usec;
}

/*
    usage 1:
        function(...) 
        {
            function_consume_declare // must first

            ...

            function_consume_calculate // must befor return
        }
        
    usage 2:
        function(...) 
        {
            function_consume_declare // must first

            ...

            function_consume_calculate_begin
            ....
            function_consume_calculate_end

            
            function_consume_calculate_begin
            ....
            function_consume_calculate_end

            
            function_consume_calculate_begin
            ....
            function_consume_calculate_end

            ....
        }
*/

#define debug_consume(_fmt, _args...)     __jdlogger(__debug_level_consume, _fmt, ##_args)

#define function_consume_declare                            \
    struct timeval __function_consume_timeval_begin;        \
    struct timeval __function_consume_timeval_end;          \
    gettimeofday(&__function_consume_timeval_begin, NULL)   \
    /* end */

#define function_consume_calculate_begin  \
    gettimeofday(&__function_consume_timeval_begin, NULL)

#define function_consume_calculate_end                      \
    gettimeofday(&__function_consume_timeval_end, NULL);    \
    debug_consume("%s consume %dus",                        \
        __func__,                                           \
        os_timeval_diff(&__function_consume_timeval_begin,  \
            &__function_consume_timeval_end))               \
    /* end */

#define function_consume_calculate          function_consume_calculate_end
#else
#define function_consume_declare            os_do_nothing()
#define function_consume_calculate_begin    os_do_nothing()
#define function_consume_calculate_end      os_do_nothing()
#define function_consume_calculate          os_do_nothing()
#endif

#ifdef __APP__
static inline jobj_t
__jlog_obj_header(jobj_t obj, char *app, char *sub, const char *file, const char *func, int line, int PRI)
{
    int pri     = LOG_PRI(PRI);
    int level   = LOG_LEVEL(PRI);
    int err;

    if (NULL==obj) {
        obj = jobj_new_object();
        if (NULL==obj) {
            __debug_error("create obj error");
            
            goto error;
        }
    }
    
    err = jobj_add_string(obj, JLOG_KEY_APP, app);
    if (err) {
        __debug_error("add app %s error", app);
        
        goto error;
    }
    
    err = jobj_add_string(obj, JLOG_KEY_SUB, sub);
    if (err) {
        __debug_error("add sub %s error", sub);
        
        goto error;
    }
    
    err = jobj_add_string(obj, JLOG_KEY_FILE, file);
    if (err) {
        __debug_error("add file %s error", file);
        
        goto error;
    }
    
    err = jobj_add_string(obj, JLOG_KEY_FUNC, func);
    if (err) {
        __debug_error("add func %s error", func);
        
        goto error;
    }
    
    if (line) {
        err = jobj_add_int(obj, JLOG_KEY_LINE, line);
        if (err) {
            __debug_error("add line %d error", line);
            
            goto error;
        }
    }

    if (level) {
        char *name = __get_debug_level_name(level);
        
        err = jobj_add_string(obj, JLOG_KEY_LEVEL, name);
        if (err) {
            __debug_error("add level %s error", name);
            
            goto error;
        }
    }
    
    err = jobj_add_int(obj, JLOG_KEY_PRI, pri);
    if (err) {
        __debug_error("add pri %d error", pri);
        
        goto error;
    }
    
    return obj;
error:
    jobj_put(obj);

    return NULL;
}

#define use_jlog_connect    1
#define use_jlog_bind       0

static inline int
__jlog_socket(char *app, char *sub, int family)
{
    int fd, err, addrlen;
    
    fd = __jlog_fd();
    if (is_good_fd(fd)) {
        return fd;
    }
    
    fd = socket(family, SOCK_DGRAM, (AF_UNIX==family)?0:IPPROTO_UDP);
    if (fd<0) {
        __debug_trace("socket error:%d", errno);
        
        return -errno;
    }
    os_closexec(fd);
        
#if use_jlog_bind
    os_sockaddr_t client = OS_SOCKADDR_INITER(family);
    
    if (AF_UNIX==family) {
        os_saprintf(get_abstract_path(&client.un), "/tmp/.jlog.%s.%s.%d.unix", app, sub?sub:"-", getpid());

        addrlen = get_abstract_sockaddr_len(&client.un);
    }
    else {
        addrlen = sizeof(struct sockaddr_in);
    }
    
    err = bind(fd, &client.c, addrlen);
    if (err) {
        __debug_trace("bind error:%d", -errno);
        err = -errno; goto error;
    }
#endif

#if use_jlog_connect
    if (AF_UNIX==family) {
        addrlen = get_abstract_sockaddr_len(__jlog_userver());
    } else {
        addrlen = sizeof(struct sockaddr_in);
    }
    
    err = connect(fd, __jlog_server(), addrlen);
    if (err) {
        __debug_trace("connect jlog error:%d", errno);
        err = -errno; goto error;
    }
#endif

    VAR_INLINE_JLOG.fd = fd;

    return fd;
error:
    os_close(fd);

    return err;
}

static inline int
__jlog(jobj_t obj, char *app, char *sub, int PRI)
{
    int fd, len, err;
    
    char *json = NULL;
    
    if (NULL==obj) {
        __debug_error("nil obj");
        
        return -EINVAL0;
    }

    json = jobj_string(obj);
    if (NULL==json) {
        __debug_error("__jlog obj==>json failed");
        
        return -ENOMEM;
    }
    
    if (LOG_DEBUG==LOG_PRI(PRI) && __is_debug_level(LOG_LEVEL(PRI))) {
        os_fprintln(stderr, "%s", json);
    }
    
    len = os_strlen(json);
    if (len<=0) {
        __debug_error("__jlog obj==>json(bad)");
        
        return -EINVAL1;
    }
    
    int family = __jlog_family();
    
    fd = __jlog_socket(app, sub, family);
    if (false==is_good_fd(fd)) {
        return fd;
    }
    
#if use_jlog_connect
    err = send(fd, json, len, 0);
#else
    err = sendto(fd, json, len, 0, __jlog_server(), os_sockaddr_len(__jlog_userver()));
#endif
    if (err<0) { /* yes, <0 */
        __debug_trace("__jlog write error:%d", errno);
        
        return -errno;
    }
    else if (err != len) {
        __debug_trace("__jlog write count(%d) < length(%d)", err, len);
        
        return -EIO;
    }
    
    return 0;
}

/*
* v: as vsprintf
*/
static inline int
__jvlogger(char *app, char *sub, const char *file, const char *func, int line, int PRI, const char *fmt, va_list args)
{
    jobj_t obj = NULL;
    int err;
    
    obj = __jlog_obj_header(NULL, app, sub, file, func, line, PRI);
    if (NULL==obj) {
        err = -ENOMEM; goto error;
    }
    
    err = jobj_vprintf(obj, fmt, args);
    if (err) {
        goto error;
    }
    
    err = __jlog(obj, app, sub, PRI);

error:
    jobj_put(obj);

    return err;
}

/*
* v: as vsprintf
*/
static inline int
__dvlogger(char *app, char *sub, const char *file, const char *func, int line, int PRI, const char *fmt, va_list args)
{
    jobj_t obj = NULL;
    int err;
    
    obj = __jlog_obj_header(NULL, app, sub, file, func, line, PRI);
    if (NULL==obj) {
        err = -ENOMEM; goto error;
    }

    err = jobj_vsprintf(obj, (char *)JLOG_KEY_INFO, fmt, args);
    if (err) {
        goto error;
    }
    
    err = __jlog(obj, app, sub, PRI);

error:
    jobj_put(obj);
    
    return err;
}

/*
* c: by command
*/
static inline int
__jclogger(char *app, char *sub, const char *file, const char *func, int line, int PRI, char *json)
{
    jobj_t obj = NULL;
    int err;

    obj = jobj(json);
    if (NULL==obj) {
        err = -EFORMAT; goto error;
    }

    obj = __jlog_obj_header(obj, app, sub, file, func, line, PRI);
    if (NULL==obj) {
        err = -EFORMAT; goto error;
    }
    
    err = __jlog(obj, app, sub, PRI);
error:
    jobj_put(obj);

    return err;
}

static inline int
__jlogger(char *app, char *sub, const char *file, const char *func, int line, int PRI, const char *fmt, ...)
{
    int err = 0;
    va_list args;
    
    va_start(args, (char *)fmt);
    err = __jvlogger(app, sub, file, func, line, PRI, fmt, args);
    va_end(args);
    
    return err;
}

static inline int
__dlogger(char *app, char *sub, const char *file, const char *func, int line, int PRI, const char *fmt, ...)
{
    int err = 0;
    va_list args;
    
    va_start(args, (char *)fmt);
    err = __dvlogger(app, sub, file, func, line, PRI, fmt, args);
    va_end(args);
    
    return err;
}

static inline int
__jlog_env_init(void)
{
    int family, err;

    family  = __jlog_server()->sa_family
            = VAR_INLINE_JLOG.family
            = get_int_env(ENV_JLOG_FAMILY, AF_UNIX);

    switch(family) {
        case AF_UNIX: {
            err = __copy_string_env(ENV_JLOG_UNIX, JLOG_UNIX, 
                    get_abstract_path(__jlog_userver()),
                    abstract_path_size);
            if (err) {
                return err;
            }
            
        }   break;
        case AF_INET: {
            unsigned int ip;
            char ipaddress[32] = {0};
            struct sockaddr_in *iserver = __jlog_iserver();
            
            iserver->sin_port = get_int_env(ENV_JLOG_PORT, JLOG_PORT);
            __debug_ok("get port:%d", iserver->sin_port);
            
            err = copy_string_env(ENV_JLOG_IP, JLOG_IP, ipaddress);
            if (err) {
                __debug_error("get jlog ip error:%d", errno);
                
                return err;
            } else {
                __debug_ok("get jlog ip:%s", ipaddress);
            }

            ip = inet_addr(ipaddress);
            if (INADDR_NONE==ip) {
                return -EFORMAT;
            }
            
            iserver->sin_addr.s_addr = ip;
            
        }   break;
        default:
            return -ENOSUPPORT;
    }

    return 0;
}

static inline int
__os_system(char *cmd)
{
    int err;

    err = system(cmd);
        debug_shell("%s error:%d", cmd, err);
    if (127==err || -1==err) {
        return -ESYSTEM;
	} else {
        return __os_wait_error(err);
	}
}

#endif

static inline int
jlog_init(void)
{
#ifdef __APP__
    int fd, err;
    int family;
    
    err = __jlog_env_init();
    if (err) {
        return err;
    }
#endif /* __APP__ */

    return 0;
}

static inline int
jlog_fini(void)
{
#ifdef __APP__
#ifndef __JLOGD__
    os_close(VAR_INLINE_JLOG.fd);
#endif
#endif

    return 0;
}
/******************************************************************************/
#endif /* __JLOG_H_1E53C7301905156FEF0817624CC8964D__ */
