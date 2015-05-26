#ifndef __FD_H_334F35EDBB322572CCDB9496B7F84A15__
#define __FD_H_334F35EDBB322572CCDB9496B7F84A15__
/******************************************************************************/
#ifdef __APP__
#ifndef FD_INIT_COUNT
#define FD_INIT_COUNT       1024
#endif

#ifndef FD_GROW_COUNT
#define FD_GROW_COUNT       1024
#endif

#ifndef FD_LIMIT
#define FD_LIMIT            (1024*1024)
#endif

#ifndef FD_READTIMEOUT
#define FD_READTIMEOUT      5000 /* ms */
#endif

#ifndef FD_WRITETIMEOUT
#define FD_WRITETIMEOUT     3000 /* ms */
#endif

#if 0
enum EPOLL_EVENTS
{
    EPOLLIN         = 0x001,
    EPOLLPRI        = 0x002,
    EPOLLOUT        = 0x004,
    EPOLLERR        = 0x008,
    EPOLLHUP        = 0x010,
#endif

#if 0
    EPOLLRDNORM     = 0x040,
    EPOLLRDBAND     = 0x080,
    EPOLLWRNORM     = 0x100,
    EPOLLWRBAND     = 0x200,
    EPOLLMSG        = 0x400,
#endif
#if 0
    EPOLLRDHUP      = 0x2000,
#endif

#define FD_F_BIND       (1<<16) //0x00010000
#define FD_F_LISTEN     (1<<17) //0x00020000
#define FD_F_CONNECT    (1<<18) //0x00040000
#define FD_F_SHARE      (1<<19) //0x00080000
#define FD_F_FILE       (1<<20) //0x00100000
#define FD_F_SOCK       (1<<21) //0x00200000
#define FD_F_EPFD       (1<<21) //0x00400000

#if 0
    EPOLLONESHOT    = 1u << 30,
    EPOLLET         = 1u << 31
};
#endif

#define FD_SO_SHARE        0x4000// setsockopt
#define FD_SO_READTIMEOUT  0x4001
#define FD_SO_WRITETIMEOUT 0x4002

/* 
* should be embed in your struct 
*/
typedef struct loop_node {
    int (*cb)(int fd, int event, struct loop_node *node);
} loop_node_t;

typedef struct {
    loop_node_t loop;
    loop_node_t *node;
    uint32_t events;
    uint32_t flag;
    co_id_t coid;
    co_cred_t cred;
    int read_timeout;
    int write_timeout;
} fd_map_t;

typedef struct {
    bool init;
    int epfd;

    /*
    * idle is the first coroutine
    *   so use 0 as invalid looper
    */
    co_id_t looper;

    /*
    * (map & evs) have same count
    */
    autoarray_t map; /* fd_map_t       */
    autoarray_t evs; /* struct epoll_event */
} fd_control_t;

#ifndef VAR_INLINE_FD
#define VAR_INLINE_FD       ____VAR_INLINE_FD
#endif

#define DECLARE_REAL_FD     fd_control_t VAR_INLINE_FD
#define DECLARE_FAKE_FD     extern DECLARE_REAL_FD

#ifdef __BUSYBOX__
#define DECLARE_FD          DECLARE_FAKE_FD
#else
#define DECLARE_FD          DECLARE_REAL_FD
#endif

DECLARE_FAKE_FD;

static inline fd_control_t *
__fd(void)
{
#ifdef USE_INLINE_FD
    return &VAR_INLINE_FD;
#else
    return NULL;
#endif
}

static inline void
__fd_control_init(void)
{
    __fd()->epfd = -1;
}

#define ____fd_map          (&__fd()->map)
#define ____fd_evs          (&__fd()->evs)
#define ____fd_count        os_aa_count(____fd_map)
#define ____fd_epfd         __fd()->epfd
#define ____fd_looper       __fd()->looper

/******************************************************************************/
static inline struct epoll_event *
__fd_evs(int idx) 
{ 
    return (struct epoll_event *)os_aa_get(____fd_evs, idx, false); 
}

static inline fd_map_t *
__fd_map(int fd, bool grow)
{
    if (grow) {
        /*
        * try grow evs 
        */
        (void)os_aa_get(____fd_evs, fd, true);
    }
    
    return (fd_map_t *)os_aa_get(____fd_map, fd, grow);
}

static inline fd_map_t *
__fd_get(int fd)
{
    fd_map_t *map = __fd_map(fd, false);

    return (map && map->flag)?map:NULL;
}

static inline bool
__fd_is_sock(fd_map_t *map)
{
    return os_hasflag(map->flag, FD_F_SOCK);
}

static int 
__fd_watcher(int fd, int event, loop_node_t *node)
{
    fd_map_t *map = container_of(node, fd_map_t, loop);

    switch(event) {
        case EPOLLIN:
            break;
        default:
            return 0;
    }
    
    co_ready(map->coid, map->cred, false);

    return 0;
}

static int
__fd_create(int fd, int flag)
{
    fd_map_t *map;

    /* check exist? */
    map = __fd_get(fd);
    if (map) {
        trace_assert(0, "fd(%d) exist", fd);
    }
    
    map = __fd_map(fd, true);
    if (NULL==map) {
        return -ENOMEM;
    }
    map->flag = flag;
    if (____fd_looper) {
        map->coid = co_self();
        map->loop.cb = __fd_watcher;
        map->read_timeout = FD_READTIMEOUT;
        map->write_timeout= FD_WRITETIMEOUT;
    }
    
    /* no block */
    if (__fd_is_sock(map)) {
        os_closexec(fd);
    }
    
    return fd;
}

static void
__fd_loop_handle(void)
{
    co_mask_t mask = co_evmask_read_and_zero();
    
    if (mask) {
        union co_mail mail;
        
        if (os_hasbit(mask, CO_EV_KILL)) {
            //do nothing
        }
        
        if (os_hasbit(mask, CO_EV_MAIL)) {
            while(0==co_mail_recv(&mail)) {
                // handle mail
            }
        }
    }
}

static inline int 
__fd_epoll_create(int size)
{
    int fd;
    
    fd = epoll_create(size);
    if (is_good_fd(fd)) {
        fd = __fd_create(fd, FD_F_EPFD);
    }

    return fd;
}

static inline int
__fd_epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout)
{
    return epoll_wait(epfd, events, maxevents, timeout);
}

static void
__fd_loop_once(void)
{
    int i, count;
    int timeout = ____fd_looper?0:1;

    count = __fd_epoll_wait(____fd_epfd, __fd_evs(0), ____fd_count, timeout);
    for  (i=0; i<count; i++) {
        struct epoll_event *evs = __fd_evs(i);
        fd_map_t *map;
        loop_node_t *node;
        uint32_t events;
        int err, fd;
        
        if (NULL==evs) {
            trace_assert(0, "evs nill");
            
            continue;
        }
        
        fd = evs->data.fd;
        if (false==is_good_enum(fd, ____fd_count)) {
            trace_assert(0, "bad fd(%d)", fd);
            
            continue;
        }

        map = __fd_get(fd);
        if (NULL==map) {
            trace_assert(0, "no found fd(%d) map", fd);

            continue;
        }
        
        node = map->node;
        if (NULL==node) {
            trace_assert(0, "fd map node is nil");
            
            continue;
        }

        events = evs->events & map->events;
        if (!events) {
            continue;
        }
        
        err = node->cb(fd, events, node);
        if (err) {
            // error, log it
            continue;
        }
    }
}

static int
__fd_loop_main(void *no_used)
{
    (void)no_used;
    
    while(1) {
        if (____fd_looper) {
            __fd_loop_handle();
        }
        
        __fd_loop_once();

        if (____fd_looper) {
            co_yield();
        }
    }

    return 0;
}

static int
__fd_add_watcher(int fd, int events, loop_node_t *node)
{
    int err;
    fd_map_t *map;
    struct epoll_event ev = {
        .data = { .fd = fd },
        .events = events,
    };
    
    map = __fd_get(fd);
    if (NULL==map) {
        return os_assertV(-EINVAL1);
    }
    
    if (map->node) {
        return os_assertV(-EEXIST);
    }
    
    ev.events = events;
    err = epoll_ctl(____fd_epfd, EPOLL_CTL_ADD, fd, &ev);
    if (err < 0) {
        return -errno;
    }
    map->node   = node;
    map->events = events;
    
    return 0;
}

static int
__fd_mod_watcher(int fd, int events)
{
    int err;
    fd_map_t *map;
    struct epoll_event ev = {
        .data = { .fd = fd },
    };

    map = __fd_get(fd);
    if (NULL==map) {
        return os_assertV(-EINVAL1);
    }

    if (NULL==map->node) {
        return os_assertV(-EBADF);
    }

    if (events==map->events) {
        return 0;
    }

    ev.events = events;
    err = epoll_ctl(____fd_epfd, EPOLL_CTL_MOD, fd, &ev);
    if (err < 0) {
        return -errno;
    }
    map->events = events;

    return 0;
}

static int
__fd_del_watcher(int fd)
{
    int err;
    fd_map_t *map;
    struct epoll_event ev = {
        .data = { .fd = fd },
        .events = 0xffffffff,
    };

    map = __fd_get(fd);
    if (NULL==map) {
        return os_assertV(-EINVAL1);
    }

    if (NULL==map->node) {
        return os_assertV(-ENOENT);
    }
    
    err = epoll_ctl(____fd_epfd, EPOLL_CTL_DEL, fd, &ev);
    if (err < 0) {
        return -errno;
    }
    map->events = 0;
    map->node   = NULL;
    
    return 0;
}

#define __fd_op(_fd, _op, _flag) do{ \
    fd_map_t *map = __fd_get(_fd);    \
    if (map) {                          \
        map->flag _op (_flag);          \
    }                                   \
}while(0)

static inline void
__fd_close(int fd)
{
    __fd_op(fd, =, 0);
}

static inline void
__fd_bind(int fd)
{
    __fd_op(fd, |=, FD_F_BIND);
}

static inline void
__fd_listen(int fd)
{
    __fd_op(fd, |=, FD_F_LISTEN);
}

static inline void
__fd_connect(int fd)
{
    __fd_op(fd, |=, FD_F_CONNECT);
}

static inline int
____fd_read(int fd)
{
    fd_map_t *map;
    
    map = __fd_get(fd);
    if (NULL==map) {
        return os_assertV(-EINVAL1);
    }
    else if (false==__fd_is_sock(map)) {
        return 0;
    }
    
    map->cred = co_cred();

    __fd_add_watcher(fd, EPOLLIN, &map->loop);
    
    return co_suspend(map->read_timeout, map->cred);
}

static int
__fd_getopt(int fd, int level, int optname, void *optval, socklen_t *optlen)
{
    fd_map_t *map;

    map = __fd_get(fd);
    if (NULL==map) {
        return os_assertV(-EINVAL1);
    }
    else if (false==__fd_is_sock(map)) {
        return 0;
    }
    
    switch(optname) {
        case FD_SO_SHARE:
            *(bool *)optval = os_hasflag(map->flag, FD_F_SHARE);
            
            break;
        case FD_SO_READTIMEOUT:
            *(int *)optval = map->read_timeout;
            
            break;
            
            break;
        case FD_SO_WRITETIMEOUT:
            *(int *)optval = map->write_timeout;
            
            break;
        default:
            break;
    }

    return 0;
}

static int
__fd_setopt(int fd, int level, int optname, const void *optval, socklen_t optlen)
{
    fd_map_t *map;

    map = __fd_get(fd);
    if (NULL==map) {
        return os_assertV(-EINVAL1);
    }
    else if (false==__fd_is_sock(map)) {
        return 0;
    }
    
    switch(optname) {
        case FD_SO_SHARE:
            map->flag |= FD_F_SHARE;
            
            break;
        case FD_SO_READTIMEOUT:
            map->read_timeout = *(int *)optval;
            
            break;
        case FD_SO_WRITETIMEOUT:
            map->write_timeout= *(int *)optval;
            
            break;
        default:
            break;
    }

    return 0;
}

#define __fd_read(_fd, _buf, _size)    ({   \
    int err = read(_fd, _buf, _size);       \
    if (err) {                              \
        debug_error(                        \
            "fd read error(%d) errno(%d)",  \
            err, -errno);                   \
    }                                       \
    err;                                    \
})  /* end */

#define __fd_recv(_fd, _buf, _size, _flag) ({  \
    int err = recv(_fd, _buf, _size, _flag);\
    if (err) {                              \
        debug_error(                        \
            "fd recv error(%d) errno(%d)",  \
            err, -errno);                   \
    }                                       \
    err;                                    \
})  /* end */

#define __fd_recvfrom(_fd, _buf, _size, _flag, _addr, _plen)   ({  \
    int err = recvfrom(_fd, _buf, _size, _flag, _addr, _plen);  \
    if (err) {                              \
        debug_error(                        \
            "fd recvfrom error(%d) errno(%d)",  \
            err, -errno);                   \
    }                                       \
    err;                                    \
})  /* end */

#define __fd_recvmsg(_fd, _msg, _flag) ({   \
    int err = recvmsg(_fd, _msg, _flag);    \
    if (err) {                              \
        debug_error(                        \
            "fd recvmsg error(%d) errno(%d)",  \
            err, -errno);                   \
    }                                       \
    err;                                    \
})  /* end */
/******************************************************************************/
static inline int
fd_add_watcher(int fd, int events, loop_node_t *node)
{
    if (false==is_good_fd(fd)) {
        return os_assertV(-EBADF);
    }
    else if (0==events) {
        return os_assertV(-EINVAL1);
    }
    else if (NULL==node) {
        return os_assertV(-EINVAL2);
    }
    else {
        return __fd_add_watcher(fd, events, node);
    }
}

static inline int
fd_mod_watcher(int fd, int events)
{
    if (false==is_good_fd(fd)) {
        return os_assertV(-EBADF);
    }
    else if (0==events) {
        return os_assertV(-EINVAL1);
    }
    else {
        return __fd_mod_watcher(fd, events);
    }
}

static inline int
fd_del_watcher(int fd)
{
    if (false==is_good_fd(fd)) {
        return os_assertV(-EBADF);
    } else {
        return __fd_del_watcher(fd);
    }
}

static inline void
fd_loop(void)
{
    if (____fd_looper) {
        co_resume(____fd_looper);
    } else {
        __fd_loop_main(NULL);
    }
}

static inline co_id_t
fd_looper(void)
{
    if (0==____fd_looper) {
        ____fd_looper = co_new("looper", __fd_loop_main, NULL, -1, 0, CO_PRI_HIGHEST, false);
    } else {
        trace_assert(0, "fd looper exist");
    }
    
    return ____fd_looper;
}


static inline bool
fd_is_sock(int fd)
{
    fd_map_t *map = __fd_get(fd);

    return map && __fd_is_sock(map);
}




static inline bool
fd_is_good(int fd)
{
    if (is_good_fd(fd)) {
        return NULL!=__fd_get(fd);
    } else {
        return false;
    }
}

static inline int
fd_open(const char *filename, int flags, ...)
{
    mode_t mode = 0;
    int fd;
    
    if (flags & O_CREAT) {
        va_list args;
        
        va_start(args, flags);
        mode = va_arg(args, mode_t);
        va_end(args);
    }
    
    fd = open(filename, flags, mode);
    if (fd<0) {
        return fd;
    } else {
        return __fd_create(fd, FD_F_FILE);
    }
}

static inline int
fd_socket(int domain, int type, int protocol)
{
    int fd;

    fd = socket(domain, type, protocol);
    if (is_good_fd(fd)) {
        fd = __fd_create(fd, FD_F_SOCK);
    }
    if (fd<0) {
        debug_error("fd create error(%d) errno(%d)", fd, -errno);
    }

    return fd;
}

static inline int
fd_close(int fd)
{
    int err = 0;

    if (is_good_fd(fd)) {
        __fd_close(fd);
        err = os_close(fd);
        if (err) {
            debug_error("fd close error(%d) errno(%d)", err, -errno);
        }
    }
    
    return err;
}

static inline int
fd_bind(int fd, struct sockaddr *addr, socklen_t length)
{
    int err = 0;

    err = bind(fd, addr, length);
    if (0==err) {
        __fd_bind(fd);
    } else {
        debug_error("fd bind error(%d)", err);
    }
    
    return err;
}

static inline int
fd_connect(int fd, struct sockaddr *addr, socklen_t length)
{
    int err = 0;

    err = connect(fd, addr, length);
    if (0==err) {
        __fd_connect(fd);
    } else {
        debug_error("fd connect error(%d) errno(%d)", err, -errno);
    }
        
    return err;
}

static inline int
fd_listen(int fd, int n)
{
    int err = 0;

    err = listen(fd, n);
    if (0==err) {
        __fd_listen(fd);
    } else {
        debug_error("fd listen error(%d) errno(%d)", err, -errno);
    }

    return err;
}

static inline int
fd_accept(int fd, struct sockaddr *addr, socklen_t *length)
{
    int err = 0;
    
    err = accept(fd, addr, length);
    if (is_good_fd(err)) {
        err = __fd_create(err, FD_F_SOCK);
    }
    if (err) {
        debug_error("fd accept error(%d) errno(%d)", err, -errno);
    }
    
    return err;
}

static inline ssize_t
fd_write(int fd, const void *buffer, size_t size)
{
    int err = 0;

    err = write(fd, buffer, size);
    if (err) {
        debug_error("fd write error(%d) errno(%d)", err, -errno);
    }
    
    if (-ENOBUFS==err && fd_is_sock(fd)) {
        /* 
        * no sndbuf space
        *   need to epoll ???
        */
    }
    
    return err;
}


static inline ssize_t
fd_read(int fd, void *buffer, size_t size)
{
    int err = 0;

    if (0==____fd_looper) {
        return __fd_read(fd, buffer, size);
    }
    
    err = ____fd_read(fd);
    if (err) {
        // retry ???
        return err;
    }
    else if (1==err) {
        return -ETIMEOUT;// timeout
    }
    else {
        return __fd_read(fd, buffer, size);
    }
}

static inline ssize_t
fd_send(int fd, const void *buffer, size_t size, int flags)
{
    int err = 0;

    err = send(fd, buffer, size, flags);
    if (err) {
        debug_error("fd send error(%d) errno(%d)", err, -errno);
    }
    if (-ENOBUFS==err) {
        /* 
        * no sndbuf space
        *   need to epoll ???
        */
    }

    return err;
}


static inline ssize_t 
fd_recv(int fd, void *buffer, size_t size, int flags)
{
    int err = 0;

    if (0==____fd_looper) {
        return __fd_recv(fd, buffer, size, flags);
    }

    err = ____fd_read(fd);
    if (err) {
        // retry ???
        return err;
    }
    else if (1==err) {
        return -ETIMEOUT;// timeout
    }
    else {
        return __fd_recv(fd, buffer, size, flags);
    }
}

static inline ssize_t
fd_sendto(int fd, const void *buffer, size_t size, int flags, struct sockaddr *addr, socklen_t length)
{
    int err = 0;

    err = sendto(fd, buffer, size, flags, addr, length);
    if (err) {
        debug_error("fd sendto error(%d) errno(%d)", err, -errno);
    }
    if (-ENOBUFS==err) {
        /* 
        * no sndbuf space
        *   need to epoll ???
        */
    }

    return err;
}

static inline ssize_t
fd_recvfrom(int fd, void *buffer, size_t size, int flags, struct sockaddr *addr, socklen_t *length)
{
    int err = 0;

    if (0==____fd_looper) {
        return __fd_recvfrom(fd, buffer, size, flags, addr, length);
    }

    err = ____fd_read(fd);
    if (err) {
        // retry ???
        return err;
    }
    else if (1==err) {
        return -ETIMEOUT;// timeout
    }
    else {
        return __fd_recvfrom(fd, buffer, size, flags, addr, length);
    }
}

static inline ssize_t
fd_sendmsg(int fd, const struct msghdr *message, int flags)
{
    int err = 0;

    err = sendmsg(fd, message, flags);
    if (err) {
        debug_error("fd sendmsg error(%d) errno(%d)", err, -errno);
    }
    if (-ENOBUFS==err) {
        /* 
        * no sndbuf space
        *   need to epoll ???
        */
    }

    return err;
}

static inline ssize_t
fd_recvmsg(int fd, struct msghdr *message, int flags)
{
    int err = 0;

    if (0==____fd_looper) {
        return __fd_recvmsg(fd, message, flags);
    }

    err = ____fd_read(fd);
    if (err) {
        // retry ???
        return err;
    }
    else if (1==err) {
        return -ETIMEOUT;// timeout
    }
    else {
        return __fd_recvmsg(fd, message, flags);
    }
}

static inline int
fd_getsockopt(int fd, int level, int optname, void *optval, socklen_t *optlen)
{
    switch(optname) {
        case FD_SO_SHARE:
        case FD_SO_READTIMEOUT:
        case FD_SO_WRITETIMEOUT:
            return __fd_getopt(fd, level, optname, optval, optlen);
        default:
            return getsockopt(fd, level, optname, optval, optlen);
    }    
}

static inline int
fd_setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen)
{
    switch(optname) {
        case FD_SO_SHARE:
        case FD_SO_READTIMEOUT:
        case FD_SO_WRITETIMEOUT:
            return __fd_setopt(fd, level, optname, optval, optlen);
        default:
            return setsockopt(fd, level, optname, optval, optlen);
    }    
}

static inline int
fd_shutdown(int fd, int how)
{
    int err = 0;
    
    err = shutdown(fd, how);
    if (err) {
        return err;
    }

    __fd_close(fd);

    return err;
}

/******************************************************************************/
static inline void
fd_fini(void)
{
    if (__fd()->init) {
        __fd()->init = false;
        
        fd_close(____fd_epfd);
        os_aa_clean(____fd_map);
        os_aa_clean(____fd_evs);
        
        co_fini();
    }
}

static inline void
fd_init(void)
{
    int err = 0;

    if (false==__fd()->init) {
        __fd()->init = true;
        
        co_init();

        __fd_control_init();
        
#define FD_AA_ARGS  FD_INIT_COUNT, FD_LIMIT, FD_GROW_COUNT, NULL, NULL
        err = os_aa_init(____fd_map, sizeof(fd_map_t), FD_AA_ARGS);
        if (err) {
            goto error;
        }
        
        err = os_aa_init(____fd_evs, sizeof(struct epoll_event), FD_AA_ARGS);
        if (err) {
            goto error;
        }
        
        ____fd_epfd = __fd_epoll_create(FD_INIT_COUNT);
        if (____fd_epfd < 0) {
            goto error;
        }
    }
    
    return;
error:
   fd_fini();
}
#endif
/******************************************************************************/
#endif /* __FD_H_334F35EDBB322572CCDB9496B7F84A15__ */
