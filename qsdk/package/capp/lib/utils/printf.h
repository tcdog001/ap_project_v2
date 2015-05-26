#ifndef __PRINTF_H_6115A6C5DFF707CC6D8400E2B7190543__
#define __PRINTF_H_6115A6C5DFF707CC6D8400E2B7190543__
/******************************************************************************/
#if defined(__BOOT__) || defined(__APP__)
#define os_printf(_fmt, _args...)               printf(_fmt, ##_args)
#define os_vprintf(_fmt, _args)                 vprintf(_fmt, _args)
#elif defined(__KERNEL__)
#define os_printf(_fmt, _args...)               printk(KERN_ALERT _fmt, ##_args)
#define os_vprintf(_fmt, _args)                 vprintk(KERN_ALERT _fmt, _args)
#else
#error  "need printf and vprintf"
#endif

#ifdef __APP__
#define os_fprintf(_stream, _fmt, _args...)     fprintf(_stream, _fmt, ##_args)
#define os_vfprintf(_stream, _fmt, _args)       vfprintf(_stream, _fmt, _args)

#define os_eprintf(_fmt, _args...)              os_fprintf(stderr, _fmt, ##_args)
#define os_evprintf(_fmt, _args)                os_vfprintf(stderr, _fmt, _args)
#else
#define os_fprintf(_stream, _fmt, _args...)     os_printf(_fmt, ##_args)
#define os_vfprintf(_stream, _fmt, _args)       os_vprintf(_fmt, _args)

#define os_eprintf(_fmt, _args...)              os_printf(_fmt, ##_args)
#define os_evprintf(_fmt, _args)                os_vprintf(_fmt, _args)
#endif

#define os_sscanf(_buf, _fmt, _args...)         sscanf(_buf, _fmt, ##_args)
#define os_vsscanf(_buf, _fmt, _args)           vsscanf(_buf, _fmt, _args)

#define os_println(_fmt, _args...)              os_printf(_fmt __crlf, ##_args)
#define os_vprintln(_fmt, _args)                os_vprintf(_fmt __crlf, _args)

#define os_printab(_count, _fmt, _args...) do{ \
    int i;                      \
                                \
    for (i=0; i<_count; i++) {  \
        os_printf(__tab);       \
    }                           \
    os_println(_fmt, ##_args);  \
}while(0)

#define os_vprintab(count, _fmt, _args) do{ \
    int i;                      \
                                \
    for (i=0; i<count; i++) {   \
        os_printf(__tab);       \
    }                           \
    os_vprintln(_fmt, _args);   \
}while(0)

#define os_fprintln(_stream, _fmt, _args...)    os_fprintf(_stream, _fmt __crlf, ##_args)
#define os_vfprintln(_stream, _fmt, _args)      os_vfprintf(_stream, _fmt __crlf, _args)

#define os_eprintln(_fmt, _args...)             os_eprintf(_fmt __crlf, ##_args)
#define os_evprintln(_fmt, _args)               os_evprintf(_fmt __crlf, _args)

#define os_sprintf(_buf, _fmt, _args...)        sprintf(_buf, _fmt, ##_args)
#define os_vsprintf(_buf, _fmt, _args)          vsprintf(_buf, _fmt, _args)

#define os_sprintln(_buf, _fmt, _args...)       os_sprintf(_buf, _fmt __crlf, ##_args)
#define os_vsprintln(_buf, _fmt, _args)         os_vsprintf(_buf, _fmt __crlf, _args)

#define os_snprintf_is_full(_buffer_space, _snprintf_return_value) \
        ((_snprintf_return_value) >= (_buffer_space))

#ifdef __BOOT__
#ifndef BOOT_SNPRINTF_BUFSIZE
#define BOOT_SNPRINTF_BUFSIZE   4096
#endif

#define os_snprintf(_buf, _size, _fmt, _args...)  ({ \
    int nsize = BOOT_SNPRINTF_BUFSIZE;  \
    int bsize = (_size)?(_size)-1:0;    \
    int len;                            \
    char *p = (char *)os_zalloc(nsize); \
    if (p) {                            \
        len = os_sprintf(p, _fmt, ##_args); \
        if (len <= bsize) {             \
            os_strcpy(_buf, p);         \
        } else {                        \
            os_memcpy(_buf, p, bsize);  \
        }                               \
        os_free(p);                     \
    } else {                            \
        len = 0;                        \
    }                                   \
                                        \
    len;                                \
})
#define os_vsnprintf(_buf, _size, _fmt, _args)  BUILD_BUG_ON(true)

/*
* change from uclibc(vasprintf)
*/
#define os_asprintf(_pbuf, _fmt, _args...)       ({ \
	int rv;                                         \
	char **pbuf = (char **)(_pbuf);                 \
                                                    \
 	rv = os_snprintf(NULL, 0, _fmt, ##_args);       \
	*pbuf = NULL;                                   \
                                                    \
	if (rv >= 0) {                                  \
		if ((*pbuf = os_malloc(++rv)) != NULL) {    \
			if ((rv = os_snprintf(*pbuf, rv, _fmt, ##_args)) < 0) { \
				os_free(*pbuf);                     \
			}                                       \
		}                                           \
	}                                               \
                                                    \
	os_assert(rv >= -1);                            \
                                                    \
	return rv;                                      \
})
#define os_vasprintf(_pbuf, _fmt, _args)        BUILD_BUG_ON(true)

#else
#define os_snprintf(_buf, _size, _fmt, _args...) snprintf(_buf, _size, _fmt, ##_args)
#define os_vsnprintf(_buf, _size, _fmt, _args)  vsnprintf(_buf, _size, _fmt, _args)

#define os_asprintf(_pbuf, _fmt, _args...)      asprintf(_buf, _size, _fmt, ##_args)
#define os_vasprintf(_pbuf, _fmt, _args)        vasprintf(_buf, _size, _fmt, _args)
#endif


/*
* snprintf for array buffer
*/
#define os_saprintf(_buf, _fmt, _args...)       os_snprintf(_buf, sizeof(_buf), _fmt, ##_args)
#define os_vsaprintf(_buf, _fmt, _args)         os_vsnprintf(_buf, sizeof(_buf), _fmt, _args)
/*
* snprintf for array buffer + offset
*/
#define os_soprintf(_buf, _offset, _fmt, _args...)  \
    os_snprintf(_buf+(_offset), sizeof(_buf)-(_offset), _fmt, ##_args)
#define os_voaprintf(_buf, _offset, _fmt, _args)    \
    os_vsnprintf(_buf+(_offset), sizeof(_buf)-(_offset), _fmt, _args)

#ifdef __APP__
#define __os_v_xopen(_type, _func, _mod, _fmt, _args...) ({  \
    char cmd[1+OS_LINE_LEN] = {0};  \
    _type tvar;                     \
                                    \
    os_saprintf(cmd, _fmt, ##_args);\
    tvar = _func(cmd, _mod);        \
                                    \
    tvar;                           \
})

#define os_v_popen(_mod, _fmt, _args...)      \
    __os_v_xopen(FILE*, popen, _mod, _fmt, ##_args)
#define os_v_fopen(_mod, _fmt, _args...) \
    __os_v_xopen(FILE*, fopen, _mod, _fmt, ##_args)
#define os_v_open(_flag, _fmt, _args...) \
    __os_v_xopen(int, open, _flag, _fmt, ##_args)

static inline int
__os_system(char *cmd);

static inline int
os_vsystem(const char *fmt, va_list args)
{
    char cmd[1+OS_LINE_LEN] = {0};
    int len, err = 0;

    len = os_vsaprintf(cmd, fmt, args);
    if (os_snprintf_is_full(sizeof(cmd), len)) {
        return -ENOSPACE;
    } else {
        return __os_system(cmd);
	}
}

static inline int
os_system(const char *fmt, ...)
{
    int err;
    va_list args;
    
    va_start(args, (char *)fmt);
    err = os_vsystem(fmt, args);
    va_end(args);
    
    return err;
}
#endif /* __APP__ */
/******************************************************************************/
#endif /* __PRINTF_H_6115A6C5DFF707CC6D8400E2B7190543__ */
