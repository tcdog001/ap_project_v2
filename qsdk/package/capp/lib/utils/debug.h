#ifndef __DEBUG_H_E3D558ADAC195223DC447321A03A7DEC__
#define __DEBUG_H_E3D558ADAC195223DC447321A03A7DEC__
/******************************************************************************/
#ifdef __APP__

#define BACKTRACE_by_none               0
#define BACKTRACE_by_libc_backtrace     1
#define BACKTRACE_by_libunwind          2

#ifndef BACKTRACE_TYPE
#   define BACKTRACE_TYPE      BACKTRACE_by_libc_backtrace
#endif

#define BACKTRACE_DESTROY_STACK_FOREVER(_up) do{   \
    int var;            \
    int *p = &var;      \
                        \
    while(1) {          \
        os_println("write %p to zero", p); \
        if (_up) {      \
            *p++ = 0;   \
        } else {        \
            *p-- = 0;   \
        }               \
    }                   \
}while(0)

#define BACKTRACE_DESTROY_STACK_BYSIZE(_up, _size)  do{   \
    int var;                        \
    char *p = (char *)&var;         \
                                    \
    os_println("write stack %s %d's byte to zero", _up?"up":"down", _size); \
                                    \
    if (_up) {                      \
        os_memzero(p, _size);          \
    } else {                        \
        os_memzero(p - _size, _size);  \
    }                               \
}while(0)

#define BACKTRACE_WRITE_POINTER(_v)    do {   \
    int i;                  \
    int *p = _v;            \
                            \
    for (i=0; i<10; i++) {  \
        os_println("write pointer(%p) in function %s", p, __func__); \
    }                       \
                            \
    *p = 0;                 \
}while(0)

#define BACKTRACE_READ_POINTER(_v)    do {   \
    int i;                  \
    int *p = _v;            \
                            \
    for (i=0; i<10; i++) {  \
        os_println("read pointer(%p) in function %s", p, __func__); \
    }                       \
                            \
    i = *p;                 \
}while(0)

#ifndef BACKTRACE_PATH
#ifdef __PC__
#define BACKTRACE_PATH          "."
#else
#define BACKTRACE_PATH          "/usr/app/backtrace/" __THIS_NAME
#endif
#endif

#ifndef BACKTRACE_ASIZE
#define BACKTRACE_ASIZE         256
#endif

#if BACKTRACE_TYPE==BACKTRACE_by_libunwind
#include <libunwind.h>

static inline void 
__os_sighandle_callstack(int sig)
{
    unw_cursor_t    cursor;
    unw_context_t   context;

    unw_getcontext(&context);
    unw_init_local(&cursor, &context);

    while (unw_step(&cursor) > 0) {
        unw_word_t  offset, pc;
        char        fname[128];

        unw_get_reg(&cursor, UNW_REG_IP, &pc);

        fname[0] = '\0';
        (void) unw_get_proc_name(&cursor, fname, sizeof(fname), &offset);

        printf ("%p : (%s+0x%x) [%p]\n", (void *)pc, fname, offset, (void *)pc);
    }

    exit(sig);
}

#elif BACKTRACE_TYPE==BACKTRACE_by_libc_backtrace
#include <execinfo.h>

static inline void 
__os_sighandle_callstack(int sig)
{
    void *array[BACKTRACE_ASIZE] = {NULL};
    char backtrace_name[1+OS_LINE_LEN];
    int i, count, fd;
    int pid = getpid();
    unsigned int now = (unsigned int)time(NULL);

    os_saprintf(backtrace_name, "%u.%d", now, sig);
    
    /*
    * create app trace file
    */
    fd = os_v_open(O_WRONLY | O_CREAT | O_TRUNC, BACKTRACE_PATH "/%s.trace", backtrace_name);
    count = backtrace(array, os_count_of(array));    
    backtrace_symbols_fd(array, count, fd);
    close(fd);
    
    os_println("backtrace array count(%d)", count);
    for (i=0; i<count; i++) {
        os_println(__tab "backtrace array[%d]=%p", i, array[i]);
    }
    os_println("backtrace array end");
    
    /*
    * copy app maps file
    */
    os_system("cp /proc/%d/maps " BACKTRACE_PATH "/%s.maps", pid, backtrace_name);
    
    /*
    * locate bug file & line
    */
#ifdef __PC__
    char lib[256];
    unsigned long base_address = 0;
    unsigned long dead_address = (unsigned long)array[3];
    
    FILE *stream = os_v_popen("r", "awk -F\"[- ]\" '{if($1<=\"%lx\" && $2>=\"%lx\") print$1, $NF}' ./%s.maps",
        dead_address,
        dead_address,
        backtrace_name);
    fscanf(stream, "%lx %s", &base_address, lib);
    
    if (NULL==strstr(lib, ".so.")) {
        /*
        * exe
        */
        os_system("addr2line -e ./%s 0x%lx > ./%s.locate", __THIS_NAME, dead_address, __THIS_NAME);
    } else {
        /*
        * lib
        */
        os_system("addr2line -e %s 0x%lx > ./%s.locate", lib, dead_address - base_address, __THIS_NAME);
    }
#endif

/*
    $(APP) is exe or lib's filename
    $(ADDRESS) is dead address, format as AABBCCDD
    
    readelf -s $(APP) | sort -k 2 | awk 'BEGIN{address="00000000";name=""}{if($2<="$(ADDRESS)"){address=$2;name=$8}}END{print address,name}'
*/
    exit(sig);
}
#else
static inline void
__os_sighandle_callstack(int sig)
{
    os_do_nothing();
}
#endif /* BACKTRACE_TYPE */

static inline void 
__os_sighandle_exit(int sig)
{
    exit(sig);
}

static inline void
__os_setup_signal(void (*handle)(int signo), int sigs[], int count)
{
    struct sigaction s = {
        .sa_flags   = 0,
        .sa_handler = handle,
    };
    int i;
    
	for (i=0; i<count; i++) {
        sigaction(sigs[i], &s, NULL);
	}
}

#define os_setup_signal(_handle, _sigs)  \
        __os_setup_signal(_handle, _sigs, os_count_of(_sigs))

static inline void
os_setup_signal_exit(void (*handle)(int sig))
{
    int sigs[] = {SIGINT, SIGHUP, SIGKILL};

    os_setup_signal(handle?handle:__os_sighandle_exit, sigs);
}

static inline void
os_setup_signal_callstack(void (*handle)(int sig))
{
    int sigs[] = {SIGBUS, SIGILL, SIGFPE, SIGSEGV};

    os_setup_signal(handle?handle:__os_sighandle_callstack, sigs);
}

static inline void
os_setup_signal_default(void (*handle)(int sig))
{
    os_setup_signal_exit(NULL);
    os_setup_signal_callstack(NULL);
}
#endif /* __APP__ */
/******************************************************************************/
#endif /* __DEBUG_H_E3D558ADAC195223DC447321A03A7DEC__ */
