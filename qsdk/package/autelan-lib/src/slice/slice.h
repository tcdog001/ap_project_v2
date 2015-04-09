#ifndef __SLICE_H_91462AFE324E762AF291F223339C3F7E__
#define __SLICE_H_91462AFE324E762AF291F223339C3F7E__
/******************************************************************************/
#include "utils.h"
/*
|<--slice_SIZE --------------------------------------------------------------->|
|                |<--slice_size ---------------------------------------------->|
|<--slice_resv-->|<--slice_len----------------------------->|<--slice_remain-->|
|                |<--slice_offset----->|                    |                  |
+----------------+---------------------+--------------------+------------------+
|      resv      |           data(used)                     |  data(no used)   |
+----------------+---------------------+--------------------+------------------+
|<--slice_head   |<--slice_data        |<--slice_cookie     |<--slice_tail     |<--slice_end

slice rules:
(1) slice_SIZE  == slice_size + slice_resv
(2) slice_size  == slice_len  + slice_remain
(3) slice_data  == slice_head + slice_resv
(4) slice_cookie== slice_data + slice_offset
(5) slice_tail  == slice_data + slice_len
(6) slice_end   == slice_tail + slice_remain
*/
#define SLICE_STACK 0x01

struct slice {
    int     flag;
    int     resv;
    int     offset;
    int     len;
    int     size;
    
    byte    *head; /* fixed */
};

#define slice_len(_slice)       (_slice)->len
#define slice_size(_slice)      (_slice)->size
#define slice_resv(_slice)      (_slice)->resv
#define slice_offset(_slice)    (_slice)->offset
#define slice_head(_slice)      (_slice)->head
#define slice_flag(_slice)      (_slice)->flag

static inline bool
slice_in_stack(const struct slice *slice)
{
    return os_hasflag(slice_flag(slice), SLICE_STACK);
}

#define slice_in_heap(_slice)   (false==slice_in_stack(_slice))

static inline bool 
slice_is_clean(const struct slice *slice)
{
    return  0==slice_len(slice)     &&
            0==slice_size(slice)    &&
            0==slice_resv(slice)    &&
            NULL==slice_head(slice);
}

static inline bool
slice_is_empty(const struct slice *slice)
{
    os_assert(slice);
    
    return !!slice_len(slice);
}

#define __SLICE_INITER(_data, _size, _resv, _is_local) { \
    .flag   = (_is_local)?SLICE_STACK:0,    \
    .len    = 0,                            \
    .size   = (int)(_size) - (int)(_resv),  \
    .resv   = (int)(_resv),                 \
    .head   = (byte *)(_data),     \
}

#define SLICE_INITER(_data, _size, _is_local) \
        __SLICE_INITER(_data, _size, 0, _is_local)

#define __SLICE_LOCAL_GUID(_slice, _size, _resv, _guid) \
    byte buffer_##_guid[_size]; \
    struct slice slice_##_guid = __SLICE_INITER(buffer_##_guid, _size, _resv, true); \
    struct slice *_slice = &slice_##_guid

#define __SLICE_LOCAL(_slice, _size, _resv) \
        __SLICE_LOCAL_GUID(_slice, _size, _resv, 91462AFE324E762AF291F223339C3F7E)

#define SLICE_LOCAL(_slice, _size) \
        __SLICE_LOCAL(_slice, _size, 0)

#define __SLICE_CLONER(_slice, _len) { \
    .len    = _len,                 \
    .size   = slice_size(_slice),   \
    .resv   = slice_resv(_slice),   \
    .head   = slice_head(_slice),   \
    .flag   = slice_flag(_slice),   \
}

#define SLICE_CLONER(_slice) \
        __SLICE_CLONER(_slice, slice_len(_slice))

#define SLICE_CLONE_INITER(_slice)  \
        __SLICE_CLONER(_slice, 0)

#define SLICE_TO_IOV(_slice, _iov)      do{ \
    (_iov)->iov_base = slice_data(_slice);  \
    (_iov)->iov_len  = slice_len(_slice);   \
}while(0)

static inline int 
slice_reinit(struct slice *slice, int size, int resv, bool local)
{
    os_assert(slice);
    os_assert(size > 0);
    os_assert(resv >= 0);

    if (false==slice_is_clean(slice) && resv >= size) {
        debug_ok("slice_reinit: resv=%d, size=%d", resv, size);
        
        return -EINVAL1;
    }

    debug_ok("slice_reinit: local = %d", local);
    
    slice_flag(slice)   = local?SLICE_STACK:0;
    slice_len(slice)    = 0;
    slice_size(slice)   = size - resv;
    slice_resv(slice)   = resv;
    slice_offset(slice) = 0;
    
    return 0;
}

static inline void 
slice_init_resv(struct slice *slice, byte *data, int size, int resv, bool local)
{
    slice_head(slice) = data;

    slice_reinit(slice, size, resv, local);
}

static inline void
slice_init(struct slice *slice, byte *data, int size, bool local)
{
    slice_init_resv(slice, data, size, 0, local);
}

/* real seize */
static inline int 
slice_SIZE(const struct slice *slice)
{
    return slice_size(slice) + slice_resv(slice);
}

static inline int 
slice_remain(const struct slice *slice)
{
    return slice_size(slice) - slice_len(slice);
}

static inline byte *
slice_data(const struct slice *slice)
{
    return slice_head(slice) + slice_resv(slice);
}

static inline byte *
slice_cookie(const struct slice *slice)
{
    return slice_data(slice) + slice_offset(slice);
}

static inline byte *
slice_end(const struct slice *slice)
{
    return slice_data(slice) + slice_size(slice);
}

static inline byte *
slice_tail(const struct slice *slice)
{
    return slice_data(slice) + slice_len(slice);
}

static inline void 
slice_zero(struct slice *slice)
{
    os_memzero(slice_data(slice), slice_size(slice));
    
    slice_len(slice) = 0;
}

static inline int
slice_alloc(struct slice *slice, int size)
{
    void *buf = NULL;

    buf = os_zalloc(size);
    if (NULL==buf) {        
        return -ENOMEM;
    }

    slice_init(slice, buf, size, false);

    return 0;
}

static inline void
slice_release(struct slice *slice)
{
    if (slice && slice_head(slice)) {
        os_free(slice_head(slice));
    }
}

static inline struct slice *
slice_clone(struct slice *dst, const struct slice *src)
{
    void *buf;
    
    buf = os_zalloc(slice_SIZE(src));
    if (NULL==buf) {
        return NULL;
    }
    
    os_memcpy(buf, slice_head(src), slice_resv(src) + slice_len(src));
    
    os_objdcpy(dst, src);
    slice_flag(dst) = (slice_flag(src) & ~SLICE_STACK);
    slice_head(dst) = buf;

    return dst;
}

#ifndef SLICE_GROW_DOUBLE_LIMIT
#define SLICE_GROW_DOUBLE_LIMIT     (256*1024)
#endif

#ifndef SLICE_GROW_STEP
#define SLICE_GROW_STEP             (4*1024)
#endif

static inline int
slice_grow(struct slice *slice, int grow)
{
    void *buf;
    int size = slice_SIZE(slice);

    if (slice_in_stack(slice)) {
        debug_error("slice is in statck, can not grow");
        
        return os_assert_value(-ENOSUPPORT);
    }

    if (grow < 0) {
        grow = 0;
    }

    if (0==grow) {
        if (size < SLICE_GROW_DOUBLE_LIMIT) {
            grow = size;
        } else {
            grow = SLICE_GROW_STEP;
        }
    }

    buf = os_realloc(slice_head(slice), size + grow);
    if (NULL==buf) {
        return -ENOMEM;
    }

    slice_head(slice) = buf;
    slice_size(slice) += grow;

    debug_trace("slice_grow: size %d, grow %d", slice_size(slice) - grow, grow);
    
    return 0;
}

/*
* as skb_pull 
*   remove data from the start of a buffer
*/
static inline byte *
slice_pull(struct slice *slice, int len)
{
    os_assert(slice);

    if (len < 0) {
        return os_assert_value(NULL);
    }
    else if (slice_size(slice) < len) {
        return os_assert_value(NULL);
    }
    
    if (0==slice_len(slice)) {
        /*
        * slice 刚初始化完毕，还没有填充数据，
        *
        * 可以 pull，无需调整 slice_len
        */
    }
    else if (slice_len(slice) < len) {
        /*
        * slice 已经使用并填充了数据
        *   但 (填充的数据长度) < (本次pull的长度)
        *
        * pull 失败
        */
        return NULL;
    } 
    else {
        /*
        * slice 已经使用并填充了数据
        *   但 (填充的数据长度) >= (本次pull的长度)
        *
        * 可以 pull，需要调整 slice_len
        */
        slice_len(slice) -= len;
    } 
    
    slice_size(slice) -= len;
    slice_resv(slice) += len;
    
    return slice_data(slice);
}

/*
* as skb_push
*   add data to the start of a buffer
*/
static inline byte *
slice_push(struct slice *slice, int len)
{
    os_assert(slice);

    if (len < 0) {
        return os_assert_value(NULL);
    }
    else if (slice_resv(slice) < len) {
        return os_assert_value(NULL);
    }
    
    slice_len(slice)    += len;
    slice_size(slice)   += len;
    slice_resv(slice)   -= len;

    return slice_data(slice);
}

static inline byte *
slice_unpull(struct slice *slice)
{
    return slice_push(slice, slice_resv(slice));
}

/*
* as skb_put
*   add data to a buffer
*/
static inline byte *
slice_put(struct slice *slice, int len)
{
    os_assert(slice);
    
    if (len < 0) {
        return os_assert_value(NULL);
    }
    else if (len > slice_remain(slice)) {
        return os_assert_value(NULL);
    }
    
    slice_len(slice) += len;
    
    return slice_tail(slice);
}

static inline byte *
slice_trim(struct slice *slice, int len)
{
    os_assert(slice);
    
    if (len < 0) {
        return os_assert_value(NULL);
    }
    else if (len > slice_len(slice)) {
        return os_assert_value(NULL);
    }
    
    slice_len(slice) -= len;
    
    return slice_tail(slice);
}

static inline byte *
slice_put_char(struct slice *slice, int ch)
{
    byte *new;
    
    os_assert(slice);

    new  = slice_put(slice, 1);
    if (new) {
        new[0] = (ch & 0xff);
    }
    
    return new;
}

static inline byte *
slice_put_buf(struct slice *slice, void *buf, int len)
{
    os_assert(slice);

    if (len < 0) {
        return os_assert_value(NULL);
    }
    else if (NULL==buf) {
        return os_assert_value(NULL);
    }
    
    os_memcpy(slice_tail(slice), buf, len);
    
    return slice_put(slice, len);
}

#define SLICE_F_GROW    0x01

extern int
slice_vsprintf(struct slice *slice, int flag, const char *fmt, va_list args);
 
/*
* 语义类似 snprintf
* 
* (1)从 slice_tail 开始写入
*
* (2)空间大小为 slice_remain
*
* (3)空间足够 : 
*   slice_tail 写入完整数据(包括 '/0'), 假设长度为 L(不包括 '/0')，
*   slice_len  增长 L，此时 slice_remain > 0
*   函数返回 L
*
* (4)空间不足 : 
*   slice_tail 写入不完整数据(假设长度为 S, 实际需要长度 L, S和L均不包括'/0'), 
*   slice_len  增长 S, 此时 slice_remain == 1
*   函数返回 L
*
* (5)调用本函数前，需要先本地缓存 slice_remain;
*    调用本函数后，用 os_snprintf_is_full 判断写入是否完整
*    例如
*       int space = slice_remain(slice);
*       int len = slice_sprintf(slice, fmt, ...);
*       if (os_snprintf_is_full(space, len)) {
*           此时写入数据不完整
*       } else {
*           此时写入数据完整
*       }
*/
extern int 
slice_sprintf(struct slice *slice, int flag, const char *fmt, ...)
    __attribute__((format(printf, 3, 4)));

#ifndef __KERNEL__

static inline void 
__slice_to_msg
(
    struct slice    *slice, 
    bool            is_send,
    struct iovec    *iov,
    struct msghdr   *msg,
    struct sockaddr *remote
)
{
    iov->iov_base   = slice_data(slice);
 	iov->iov_len    = (true == is_send)?slice_len(slice):slice_size(slice);
 	msg->msg_name   = remote;
 	msg->msg_namelen= sizeof(*remote);
 	msg->msg_iov    = iov;
 	msg->msg_iovlen = 1;
}

static inline int slice_send(int fd, struct slice *slice, struct sockaddr *remote, int flag)
{
    struct iovec    iov = {0};
    struct msghdr   msg = {0};

    __slice_to_msg(slice, true, &iov, &msg, remote);
    
    return sendmsg(fd, &msg, flag);
}

static inline int slice_recv(int fd, struct slice *slice, struct sockaddr *remote, int flag)
{
    struct iovec    iov = {0};
    struct msghdr   msg = {0};
    
    __slice_to_msg(slice, false, &iov, &msg, remote);
    
    return recvmsg(fd, &msg, flag);
}
#endif

/******************************************************************************/
#endif /* __SLICE_H_91462AFE324E762AF291F223339C3F7E__ */
