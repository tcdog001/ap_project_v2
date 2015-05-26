#ifndef __TIMER_H_7354F2DEF84483568CAD93F29E63C88B__
#define __TIMER_H_7354F2DEF84483568CAD93F29E63C88B__
/******************************************************************************/
#ifdef __APP__
#ifndef TM_MS
#define TM_MS           10
#endif

#ifndef TM_BIT
#define TM_BIT          8
#endif

#if !(2==TM_BIT || 4==TM_BIT || 8==TM_BIT || 16==TM_BIT)
#error "TM_BIT must be 2/4/8/16"
#endif

enum {
    TM_MASK     = os_mask(TM_BIT),
    TM_SLOT     = (TM_MASK+1),
    TM_RING     = (sizeof(void *)*8/TM_BIT),
    TM_RINGMAX  = (TM_RING-1),
};

enum {
    TM_PENDING  = 0x01,    /* 等待调度 */
    TM_CYCLE    = 0x02,    /* 周期性定时器 */
};

struct __tm_node;
typedef int tm_callback_f(struct __tm_node *timer);

typedef struct __tm_node {
    struct list_head node;
    tm_callback_f *cb;

    uint8_t     flags;
    uint8_t     ring_idx;
    uint16_t    ring_slot;
    uint32_t    expires;
    
    uint64_t    create;
} tm_node_t;

typedef struct {
    struct {
        struct list_head list;
        uint32_t count;
    } slot[TM_SLOT];
    
    int current;
    uint32_t count;
} tm_ring_t;

typedef struct {
    bool        init;
    uint64_t    ticks;
    
    uint64_t    triggered_error;
    uint64_t    triggered_ok;
    uint64_t    inserted;
    uint64_t    removed;
    
    tm_ring_t   ring[TM_RING];
    uint32_t    count;
    uint32_t    unit; // how much ms per tick
} tm_clock_t;

#ifndef VAR_INLINE_TIMER
#define VAR_INLINE_TIMER    ____VAR_INLINE_TIMER
#endif

#define DECLARE_REAL_TIMER  tm_clock_t VAR_INLINE_TIMER
#define DECLARE_FAKE_TIMER  extern DECLARE_REAL_TIMER

#ifdef __BUSYBOX__
#define DECLARE_TIMER       DECLARE_FAKE_TIMER
#else
#define DECLARE_TIMER       DECLARE_REAL_TIMER
#endif

DECLARE_FAKE_TIMER;

static inline tm_clock_t *
__tm_clock(void)
{
#ifdef USE_INLINE_TIMER
    return &VAR_INLINE_TIMER;
#else
    return NULL;
#endif
}

static inline tm_ring_t *
__tm_ring(int idx)
{
    return &__tm_clock()->ring[idx];
}

#define __tm_ring0                  __tm_ring(0)
#define __tm_ringx(_x)              __tm_ring(_x)
#define __tm_ringmax                __tm_ring(TM_RINGMAX)
#define __tm_foreach_ring(_ring)    for((_ring)=__tm_ring0; (_ring)<=__tm_ringmax; (_ring)++)
#define __tm_foreach_slot(_slot)    for((_slot)=0; (_slot)<TM_SLOT; (_slot)++)

static inline struct list_head *
__tm_slot(tm_ring_t *ring, int idx)
{
    return &ring->slot[idx].list;
}

static inline bool
__tm_is_cycle(tm_node_t *timer)
{
    return os_hasflag(timer->flags, TM_CYCLE);
}

static inline bool
__tm_is_pending(tm_node_t *timer)
{
    return os_hasflag(timer->flags, TM_PENDING);
}

static inline uint32_t
__tm_left(tm_node_t *timer)
{
    uint64_t timeout = timer->create + (uint64_t)timer->expires;
    
    if (timeout > __tm_clock()->ticks) {
        return (uint32_t)(timeout - __tm_clock()->ticks);
    } else {
        return 0;
    }
}

static inline tm_ring_t *
__tm_find_ring(tm_node_t *timer, int *slot)
{
    int idx;
    uint32_t left = __tm_left(timer);
    uint32_t offset;
    tm_ring_t *ring;
    
    for (idx=TM_RINGMAX; idx>0; idx--) {
        offset = left;
        offset <<= TM_BIT*(TM_RINGMAX-idx);
        offset >>= TM_BIT*TM_RINGMAX;
        if (offset) {
            break;
        }
    }

    if (0==idx) {
        offset = left;
    }

    ring = __tm_ringx(idx);
    
    *slot = (ring->current + offset) & TM_MASK;
    
    return ring;
}

static inline void
__tm_insert(tm_node_t *timer)
{
    int slot = 0;
    tm_ring_t *ring = __tm_find_ring(timer, &slot);

    list_add(&timer->node, __tm_slot(ring, slot));
    timer->ring_idx     = ring - __tm_ring0;
    timer->ring_slot    = slot;
    timer->flags        |= TM_PENDING;

    ring->slot[slot].count++;
    ring->count++;
    __tm_clock()->count++;
    __tm_clock()->inserted++;
    
    if (__tm_clock()->ticks) {
        //debug_test("timer(expires:%u) insert to ring(%d) slot(%d)", node->expires, node->ring_idx, node->ring_slot);
    }
}

static inline void
__tm_remove(tm_node_t *timer)
{
    tm_ring_t *ring = __tm_ringx(timer->ring_idx);

    if (__tm_clock()->ticks) {
        //debug_test("timer(expires:%u) remove from ring(%d) slot(%d)", node->expires, node->ring_idx, node->ring_slot);
    }
    
    list_del(&timer->node);
    ring->slot[timer->ring_slot].count--;
    ring->count--;
    __tm_clock()->count--;
    __tm_clock()->removed++;
    timer->flags &= ~TM_PENDING;
}

static inline void
__tm_slot_dump(tm_ring_t *ring, int slot)
{
    struct list_head *head = __tm_slot(ring, slot);

    if (false==list_empty(head)) {
        os_println(__tab2 "ring(%d) slot(%d) count(%d)", ring-__tm_ring0, slot, ring->slot[slot].count);
    }
}

static inline void
__tm_ring_dump(tm_ring_t *ring)
{
    int i;
    
    os_println(__tab "ring(%d) count(%d), current(%d)", ring-__tm_ring0, ring->count, ring->current);
    
    if (ring->count) {
        for (i=0; i<TM_SLOT; i++) {
            __tm_slot_dump(ring, i);
        }
    }
}

static inline void
__tm_dump(void)
{
    if (__is_debug_level_test) {
        tm_ring_t *ring;

        os_println("======================");
        os_println("CLOCK count(%u) ticks(%llu)", __tm_clock()->count, __tm_clock()->ticks);
        __tm_foreach_ring(ring) {
            __tm_ring_dump(ring);
        }
        os_println("======================");
    }
}

static inline int
__tm_ring_trigger(tm_ring_t *ring)
{
    tm_node_t *timer, *n;
    struct list_head *head = __tm_slot(ring, ring->current);
    int count = 0;
    
    list_for_each_entry_safe(timer, n, head, node) {
        bool timeout = (0==__tm_left(timer));
        bool insert = false;
        
        __tm_remove(timer);
        
        if (timeout) {
            /*
            * left 0 ticks, trigger it
            */
            int err = (*timer->cb)(timer);
            if (0==err) {
                count++;
                
                __tm_clock()->triggered_ok++;
            } else {
                __tm_clock()->triggered_error++;
            }

            if (__tm_is_cycle(timer)) {
                insert = true;

                /*
                * reset create ticks
                */
                timer->create = __tm_clock()->ticks;
            }
        } else {
            insert = true;
        }
        
        if (insert) {
            __tm_insert(timer);
        }
    }
    
    return count;
}

/* 
* return
*   success trigger nodes
*/
static inline int
__tm_trigger(tm_ring_t *ring)
{
    int count = 0;
    
    ring->current++;
    ring->current &= TM_MASK;
    
    count += __tm_ring_trigger(ring);
    
    // next ring do carry(+1)
    if (0==ring->current && ring < __tm_ringmax) { 
        count += __tm_trigger(ring+1);
    }

    return count;
}

/*
* return
*   <0: error
*  >=0: sucess tigger times
*/
static inline int
os_tm_trigger(uint32_t times)
{
    uint32_t i;
    int err, ret = 0;
    int count = 0;
    
    for (i=0; i<times; i++) {
        __tm_clock()->ticks++;

        __tm_dump();
        err = __tm_trigger(__tm_ring0);
        if (err<0 && 0==ret) {
            ret = err; /* log first error */
        } else {
            count += err;
        }
    }
    
    return ret?ret:count;
}

static inline uint64_t
os_tm_ticks(void)
{
    return __tm_clock()->ticks;
}

static inline void
os_tm_unit_set(uint32_t ms) // how much ms per tick
{
    __tm_clock()->unit = ms?ms:TM_MS;
}

static inline uint32_t //ms, how much ms per tick
os_tm_unit(void)
{
    return __tm_clock()->unit;
}

static inline int
os_tm_insert(
    tm_node_t *timer, // should embed in other struct
    int after, // ticks, after now
    tm_callback_f *cb,
    bool cycle
)
{
    if (NULL==timer) {
        return error_assertV(-EKEYNULL, "timer is nil");
    }
    else if (NULL==cb) {
        return error_assertV(-EINVAL1, "cb is nil");
    }
    else if (after <= 0) {
        return error_assertV(-EINVAL2, "after(%d) <= 0", after);
    }
    else if (__tm_is_pending(timer)) {
        debug_error("cannot insert pending timer");
        
        return -EINLIST; /* NO assert */
    } 
        
    timer->create   = __tm_clock()->ticks;
    timer->expires  = (uint32_t)after;
    timer->flags    = cycle?TM_CYCLE:0;
    timer->cb       = cb;

    __tm_insert(timer);
    
    return 0;
}

static inline int
os_tm_remove(tm_node_t *timer)
{
    if (NULL==timer) {
        return error_assertV(-EKEYNULL, "timer is nil");
    }
    else if (__tm_is_pending(timer)) {
        __tm_remove(timer);
        
        os_objzero(timer);

        return 0;
    } 
    else {
        return -ENOEXIST;
    }
}

static inline int
os_tm_change(tm_node_t *timer, uint32_t after)
{
    if (NULL==timer) {
        return error_assertV(-EKEYNULL, "timer is nil");
    }
    else if (__tm_is_pending(timer)) {
        uint32_t left = __tm_left(timer);

        __tm_remove(timer);

        if (after > left) {
            timer->expires += after - left;
        } else {
            timer->expires -= left - after;
        }
        
        __tm_insert(timer);

        return 0;
    } 
    else {
        return -ENOEXIST;
    }
}

/*
* return
*   >0: left time
*   =0: NOT pending
*   <0: error
*/
static inline int //ticks
os_tm_left(tm_node_t *timer)
{
    if (NULL==timer) {
        return error_assertV(-EKEYNULL, "timer is nil");
    }
    else if (__tm_is_pending(timer)) {
        return __tm_left(timer);
    }
    else {
        return 0;
    }
}

/*
* return
*   >0: expires(os_tm_insert's after)
*   =0: NOT pending
*   <0: error
*/
static inline int //ticks
os_tm_expires(tm_node_t *timer)
{
    if (NULL==timer) {
        return error_assertV(-EKEYNULL, "timer is nil");
    }
    else if (__tm_is_pending(timer)) {
        return timer->expires;
    } 
    else {
        return 0;
    }
}

static inline bool
os_tm_is_pending(tm_node_t *timer)
{
    if (NULL==timer) {
        return error_assertV(false, "timer is nil");
    } else {
        return __tm_is_pending(timer);
    }
}

static inline void
os_tm_init(void)
{
    tm_ring_t *ring;
    int slot;

    if (false==__tm_clock()->init) {
        __tm_clock()->init = true;
        
        __tm_foreach_ring(ring) {
            __tm_foreach_slot(slot) {
                INIT_LIST_HEAD(__tm_slot(ring, slot));
            }
        }
    }
}

static inline void
os_tm_fini(void)
{
    tm_node_t *timer, *n;
    tm_ring_t *ring;
    int slot;

    if (true==__tm_clock()->init) {
        __tm_clock()->init = false;
        
        __tm_foreach_ring(ring) {
            __tm_foreach_slot(slot) {
                struct list_head *head = __tm_slot(ring, slot);
                
                list_for_each_entry_safe(timer, n, head, node) {
                    __tm_remove(timer);
                }
            }
        }
    }
}

static inline int
os_timerfd(uint32_t sec, uint32_t nsec)
{
    struct itimerspec old;
    struct itimerspec new = {
        .it_interval = {
            .tv_sec     = sec,
            .tv_nsec    = nsec,
        },
    };
    int fd = -1;
    
    fd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC);
    if (fd<0) {
        return -errno;
    }

    new.it_value.tv_sec += sec;
    new.it_value.tv_nsec += nsec;
    if (new.it_value.tv_nsec >= 1000*1000*1000) {
        new.it_value.tv_nsec = 0;
        new.it_value.tv_sec++;
    }
    
    timerfd_settime(fd, 0, &new, &old);
    
    return fd;
}

static inline uint32_t
os_timerfd_times(int fd)
{
    uint64_t timeout = 0;
    int err = read(fd, &timeout, sizeof(timeout));
    uint32_t times = (err<0)?1:(uint32_t)timeout;

    debug_timeout("read timerfd %d times", times);
    
    return times;
}

static inline int
os_timerfd_trigger(int fd)
{
    uint64_t timeout = 0;
    int err = read(fd, &timeout, sizeof(timeout));
    uint32_t times = (err<0)?1:(uint32_t)timeout;

    debug_timeout("trigger timer %d times", times);
    
    return os_tm_trigger(times);
}

#endif
/******************************************************************************/
#endif /* __TIMER_H_7354F2DEF84483568CAD93F29E63C88B__ */
