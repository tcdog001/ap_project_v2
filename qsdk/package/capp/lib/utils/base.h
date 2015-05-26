#ifndef __BASE_H_DF48B466F7D87EDB327C3D4C73E6E4A3__
#define __BASE_H_DF48B466F7D87EDB327C3D4C73E6E4A3__
/******************************************************************************/
#if defined(__BOOT__) || defined(__APP__)
/* Force a compilation error if condition is true */
#define BUILD_BUG_ON(_condition)    ((void)BUILD_BUG_ON_ZERO(_condition))
/* Force a compilation error if condition is true, but also produce a
   result (of value 0 and type size_t), so the expression can be used
   e.g. in a structure initializer (or where-ever else comma expressions
   aren't permitted). */
#define BUILD_BUG_ON_ZERO(_e)       (sizeof(struct { int:-!!(_e); }))
#define BUILD_BUG_ON_NULL(_e)       ((void *)sizeof(struct { int:-!!(_e); }))

#ifndef offsetof
#define offsetof(_TYPE, _MEMBER)    __builtin_offsetof(_TYPE, _MEMBER)
#endif

#ifndef container_of
/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:	the pointer to the container.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 */
#define container_of(_ptr, _type, _member) \
	    ((_type *)((char *)(_ptr) - offsetof(_type, _member)))
#endif
#endif /* defined(__BOOT__) || defined(__APP__) */

#ifndef os_count_of
#define os_count_of(_x)         (sizeof(_x)/sizeof((_x)[0]))
#endif

#ifndef os_pointer_offset
#define os_pointer_offset(_base, _pointer)  ((char *)(_pointer) - (char *)(_base))
#endif

/*
* GCC 还支持优先级 __attribute__((constructor(priority)))
* 但实验编译不过，暂时未找到原因，
*   可以用函数在文件中的实现顺序来间接支持优先级, 按照如下原则
*       在一个C文件中, 如果实现了多个 os_constructor 函数
*       则先声明的后执行，后声明的先执行
*
* 建议 os_constructor/os_destructor 与 static 结合使用
*/
#ifndef os_constructor
#   ifdef __BUSYBOX__
#       define os_constructor   inline
#   else
#       define os_constructor   __attribute__((constructor))
#   endif
#endif

#ifndef os_destructor
#   ifdef __BUSYBOX__
#       define os_destructor    inline
#   else
#       define os_destructor    __attribute__((destructor))
#   endif
#endif

#ifndef os_do_nothing 
#define os_do_nothing()     do{}while(0)
#endif

#ifndef os_use_fake_value
#define os_use_fake_value   extern int __os_value_not_used_forever
#endif

#ifndef os_wait_forever
#define os_wait_forever()   do{sleep(60);}while(1)
#endif

#define os_swap_value(_a, _b)  do {    \
    typeof(_a) _tmp = (_a);     \
    (_a) = (_b);                \
    (_b) = _tmp;                \
}while(0)

#define INVALID_VALUE       (-1)
#define INVALID_COMMON_ID   INVALID_VALUE
#define INVALID_SEM_ID      INVALID_COMMON_ID
#define INVALID_FD          INVALID_COMMON_ID

#define OS_PROTECTED        0x2012dead

static inline bool is_good_common_id(int id)
{
    return id >= 0;
}

#define is_good_shmid(_shmid)   is_good_common_id(_shmid)
#define is_good_semid(_semid)   is_good_common_id(_semid)
#define is_good_fd(_fd)         is_good_common_id(_fd)

#define __os_close(_fd) ({  \
    int fd = (_fd);         \
    int err = 0;            \
                            \
    if (is_good_fd(fd)) {   \
        err = close(fd);    \
    }                       \
                            \
    err;                    \
})  /* end */

#define os_close(_fd)   ({  \
    int err = __os_close(_fd); \
    _fd = INVALID_FD;       \
    err;                    \
})  /* end */

/*
* check value with [begin, end)
*/
#define is_good_value(_value, _begin, _end) ({  \
    typeof(_value) v = (_value);                \
    (v >= (_begin) && v < (_end));              \
})  /* end */

/*
* check id with [0, end)
*/
#define is_good_enum(_id, _end)     is_good_value(_id, 0, _end)

#define os_safe_value(_value, _min, _max) ({\
    typeof(_value)  __value   = _value;     \
    typeof(_value)  __min     = _min;       \
    typeof(_value)  __max     = _max;       \
                                            \
    if (__value < __min) {          \
        __value = __min;            \
    } else if (__value > __max) {   \
        __value = __max;            \
    }                               \
                                    \
    __value;                        \
})  /* end */

/*
* 忘了min/max在哪个头文件定义了，先放这里
*/
#if 1
#define os_min(_x,_y)   ({  \
    typeof(_x) x = (_x);    \
    typeof(_y) y = (_y);    \
    (void) (&x == &y);      \
    x < y ? x : y;          \
})  /* end */
    
#define os_max(_x,_y)   ({  \
    typeof(_x) x = (_x);    \
    typeof(_y) y = (_y);    \
    (void) (&x == &y);      \
    x > y ? x : y;          \
})  /* end */
#endif

#define OS_IOVEC_INITER(_base, _len) { \
    .iov_base   = _base,    \
    .iov_len    = _len,     \
}   /* end */

#define OS_MSGHDR_INITER(   \
    _iov,                   \
    _iovlen,                \
    _name,                  \
    _namelen,               \
    _control,               \
    _controllen             \
)                           \
{                           \
    .msg_iov        = _iov,         \
    .msg_iovlen     = _iovlen,      \
    .msg_name       = _name,        \
    .msg_namelen    = _namelen,     \
    .msg_control    = _control,     \
    .msg_controllen = _controllen,  \
}   /* end */

#define OS_SOCKADDR_UNIX(_path) {   \
    .sun_family = AF_UNIX,          \
    .sun_path   = _path,            \
}   /* end */

#define OS_SOCKADDR_INET(_ip, _port) { \
    .sun_family = AF_INET,          \
    .sin_port   = _port,            \
    .sin_addr   = {                 \
        .s_addr = _ip,              \
    }                               \
}   /* end */

#define OS_SOCKADDR_NETLINK(_pid, _groups) { \
    .sun_family = AF_NETLINK,       \
    .nl_pid     = _pid,             \
    .nl_groups  = _groups           \                               \
}   /* end */

/*
* copy from tcp.h
*
* seq1 is before seq2
*/
static inline bool os_seq_before(unsigned int seq1, unsigned int seq2)
{
        return ((int)(seq1-seq2)) < 0;
}

/*
* seq1 is after seq2
*/
#define os_seq_after(_seq1, _seq2)  os_seq_before(_seq2, _seq1)

enum {
    __APP_NAMELEN       = (31 - sizeof(unsigned int)),
    __APP_KEY_NAMELEN   = 31,
};

enum {
    OS_IFNAMELEN = 15,
};

#ifndef OS_LINE_LEN
#define OS_LINE_LEN             1023
#endif

#ifndef OS_FILENAME_LEN
#define OS_FILENAME_LEN         1023
#endif

#define __os_call(_new, _free, _call, _args...) ({ \
    void *__obj = (void *)_new();   \
    if (__obj) {                    \
        err = _call(__obj, ##_args);\
    }                               \
    _free(__obj);                   \
                                    \
    err;                            \
})  /* end */

#define os_call(_begin, _end, _call, _arg1, _args...) ({ \
    int err = _begin();             \
    if (0==err) {                   \
        err = _call(_arg1, ##_args);\
    }                               \
    _end();                         \
                                    \
    err;                            \
})  /* end */

#define os_call_1(_begin, _end, _call, _arg1, _args...) ({ \
    int err = _begin(_arg1);        \
    if (0==err) {                   \
        err = _call(_arg1, ##_args);\
    }                               \
    _end(_arg1);                    \
                                    \
    err;                            \
})  /* end */

#define os_call_2(_begin, _end, _call, _arg1, _arg2, _args...) ({ \
    int err = _begin(_arg1, _arg2); \
    if (0==err) {                   \
        err = _call(_arg1, _arg2, ##_args); \
    }                               \
    _end(_arg1, _arg2);             \
                                    \
    err;                            \
})  /* end */    

static inline void 
os_closexec(int fd)
{
#ifdef FD_CLOEXEC
    fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | FD_CLOEXEC);
#endif
}

static inline int
__os_wait_error(int status)
{
    int err = 0;

    if (WIFEXITED(status)) {
        err = WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        if (WCOREDUMP(status)) {
            err = WCOREDUMP(status);
        }
        else if (WTERMSIG(status)) {
            err = WTERMSIG(status);
        }
    }

    return err;
}

typedef struct {
    char *k;
    unsigned int v;
} os_kv_t;

#define OS_KV_INITER(_k, _v)    { .k = _k, .v = _v }
/******************************************************************************/
#endif /* __BASE_H_DF48B466F7D87EDB327C3D4C73E6E4A3__ */
