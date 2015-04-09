/******************************************************************************/
#include "timer.h"

#ifndef TM_MS
#define TM_MS           10
#endif

#ifndef TM_BIT
#define TM_BIT          8
#elif 2!=TM_BIT || 4!=TM_BIT || 8!=TM_BIT || 16!=TM_BIT
#error "TM_BIT must be 2/4/8/16"
#endif

#define TM_MASK         os_mask(TM_BIT)
#define TM_SLOT         (TM_MASK+1)
#define TM_RING         (sizeof(void *)*8/TM_BIT)
#define TM_RINGMAX      (TM_RING-1)

#define TM_PENDING      0x01    /* 等待调度 */
#define TM_CYCLE        0x02    /* 周期性定时器 */

struct tm_node {
    /* 3*sizeof(void *) */
    struct list_head node;
    tm_callback *cb;

    /* 2*sizeof(uint32_t) */
    uint8_t     flags;
    uint8_t     ring_idx;
    uint16_t    ring_slot;
    uint32_t    expires;
    
    /* 1*sizeof(uint64_t) */
    uint64_t    create;
};

struct tm_ring {
    struct {
        struct list_head list;
        uint32_t count;
    } slot[TM_SLOT];
    
    int current;
    uint32_t count;
};

static struct {
    uint64_t    ticks;
    
    uint64_t    triggered_error;
    uint64_t    triggered_ok;
    uint64_t    inserted;
    uint64_t    removed;
    
    struct tm_ring ring[TM_RING];
    uint32_t    count;
    uint32_t    unit; // how much ms per tick
} CLOCK;


static inline struct tm_ring *
__tm_ring(int idx)
{
    return &CLOCK.ring[idx];
}

#define tm_ring0                __tm_ring(0)
#define tm_ringx(x)             __tm_ring(x)
#define tm_ringmax              __tm_ring(TM_RINGMAX)
#define foreach_ring(_ring)     for((_ring)=tm_ring0; (_ring)<=tm_ringmax; (_ring)++)
#define foreach_slot(_slot)     for((_slot)=0; (_slot)<TM_SLOT; (_slot)++)

static inline struct list_head *
tm_slot(struct tm_ring *ring, int idx)
{
    return &ring->slot[idx].list;
}

static inline bool
tm_is_cycle(struct tm_node *node)
{
    return os_hasflag(node->flags, TM_CYCLE);
}

static inline bool
tm_is_pending(struct tm_node *node)
{
    return os_hasflag(node->flags, TM_PENDING);
}

static inline uint32_t
tm_left(struct tm_node *node)
{
    uint64_t timeout = node->create + (uint64_t)node->expires;
    
    if (timeout > CLOCK.ticks) {
        return (uint32_t)(timeout - CLOCK.ticks);
    } else {
        return 0;
    }
}

static inline struct tm_ring *
tm_find_ring(struct tm_node *node, int *slot)
{
    int idx;
    uint32_t left = tm_left(node);
    uint32_t offset;
    struct tm_ring *ring;
    
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

    ring = tm_ringx(idx);
    
    *slot = (ring->current + offset) & TM_MASK;
    
    return ring;
}

static void
tm_insert(struct tm_node *node)
{
    int slot = 0;
    struct tm_ring *ring = tm_find_ring(node, &slot);

    list_add(&node->node, tm_slot(ring, slot));
    node->ring_idx = ring - tm_ring0;
    node->ring_slot= slot;
    node->flags |= TM_PENDING;

    ring->slot[slot].count++;
    ring->count++;
    CLOCK.count++;

    if (CLOCK.ticks) {
        //debug_test("timer(expires:%u) insert to ring(%d) slot(%d)", node->expires, node->ring_idx, node->ring_slot);
    }
}

static inline void
tm_remove(struct tm_node *node)
{
    struct tm_ring *ring = tm_ringx(node->ring_idx);

    if (CLOCK.ticks) {
        //debug_test("timer(expires:%u) remove from ring(%d) slot(%d)", node->expires, node->ring_idx, node->ring_slot);
    }
    
    list_del(&node->node);
    ring->slot[node->ring_slot].count--;
    ring->count--;
    CLOCK.count--;
    node->flags &= ~TM_PENDING;
}

static void
tm_slot_dump(struct tm_ring *ring, int slot)
{
    struct list_head *head = tm_slot(ring, slot);

    if (false==list_empty(head)) {
        os_println(__tab2 "ring(%d) slot(%d) count(%d)", ring-tm_ring0, slot, ring->slot[slot].count);
    }
}

static void
tm_ring_dump(struct tm_ring *ring)
{
    int i;
    
    os_println(__tab "ring(%d) count(%d), current(%d)", ring-tm_ring0, ring->count, ring->current);
    
    if (ring->count) {
        for (i=0; i<TM_SLOT; i++) {
            tm_slot_dump(ring, i);
        }
    }
}

static void
tm_dump()
{
    if (__is_debug_init_test) {
        struct tm_ring *ring;

        os_println("======================");
        os_println("CLOCK count(%u) ticks(%llu)", CLOCK.count, CLOCK.ticks);
        foreach_ring(ring) {
            tm_ring_dump(ring);
        }
        os_println("======================");
    }
}

/* 
* return
*   success trigger nodes
*/
static int
tm_ring_trigger(struct tm_ring *ring)
{
    struct tm_node *node, *n;
    struct list_head *head = tm_slot(ring, ring->current);
    int count = 0;
    
    list_for_each_entry_safe(node, n, head, node) {
        bool timeout = (0==tm_left(node));
        bool insert = false;
        
        tm_remove(node);
        
        if (timeout) { // left 0 ticks, trigger it
            int err = (*node->cb)((tm_node_t *)node);
            if (0==err) {
                count++;
                
                CLOCK.triggered_ok++;
            } else {
                CLOCK.triggered_error++;
            }

            if (tm_is_cycle(node)) {
                insert = true;
                
                node->create = CLOCK.ticks; //reset create ticks
            }
        } else {
            insert = true;
        }
        
        if (insert) {
            tm_insert(node);
        }
    }
    
    return count;
}

/* 
* return
*   success trigger nodes
*/
static int
tm_trigger(struct tm_ring *ring)
{
    int count = 0;
    
    ring->current++;
    ring->current &= TM_MASK;
    
    count += tm_ring_trigger(ring);
    
    // next ring do carry(+1)
    if (0==ring->current && ring < tm_ringmax) { 
        count += tm_trigger(ring+1);
    }

    return count;
}

int
os_tm_trigger(uint32_t times)
{
    uint32_t i;
    int err, ret = 0;
    int count = 0;
    
    for (i=0; i<times; i++) {
        CLOCK.ticks++;

        tm_dump();
        err = tm_trigger(tm_ring0);
        if (err<0 && 0==ret) {
            ret = err; /* log first error */
        } else {
            count += err;
        }
    }

    return ret?ret:count;
}

uint64_t
os_tm_ticks(void)
{
    return CLOCK.ticks;
}

void
os_tm_unit_set(uint32_t ms) // how much ms per tick
{
    CLOCK.unit = ms?ms:TM_MS;
}

uint32_t //ms, how much ms per tick
os_tm_unit(void)
{
    return CLOCK.unit;
}

int
os_tm_insert(
    tm_node_t *timer, 
    int after,
    tm_callback *cb,
    bool cycle
)
{
    struct tm_node *node = (struct tm_node *)timer;

    if (NULL==timer) {
        return os_assert_value(-EKEYNULL);
    }
    else if (NULL==cb) {
        return os_assert_value(-EINVAL1);
    }
    else if (after <= 0) {
        return os_assert_value(-EINVAL2);
    }
    
    node->create    = CLOCK.ticks;
    node->expires   = (uint32_t)after;
    node->flags     = cycle?TM_CYCLE:0;
    node->cb        = cb;

    tm_insert(node);
    
    CLOCK.inserted++;
    
    return 0;
}

int
os_tm_remove(tm_node_t *timer)
{
    struct tm_node *node = (struct tm_node *)timer;

    if (NULL==timer) {
        return os_assert_value(-EKEYNULL);
    }
    else if (tm_is_pending(node)) {
        tm_remove(node);
        
        CLOCK.removed++;
        os_objzero(node);

        return 0;
    } 
    else {
        return -ENOEXIST;
    }
}

int
os_tm_change(tm_node_t *timer, uint32_t after)
{
    struct tm_node *node = (struct tm_node *)timer;
    
    if (NULL==timer) {
        return os_assert_value(-EKEYNULL);
    }
    else if (tm_is_pending(node)) {
        uint32_t left = tm_left(node);

        tm_remove(node);

        if (after > left) {
            node->expires += after - left;
        } else {
            node->expires -= left - after;
        }
        
        tm_insert(node);

        return 0;
    } 
    else {
        return -ENOEXIST;
    }
}

int
os_tm_left(tm_node_t *timer)
{
    struct tm_node *node = (struct tm_node *)timer;

    if (NULL==timer) {
        return os_assert_value(-EKEYNULL);
    }
    else if (tm_is_pending(node)) {
        return tm_left(node);
    }
    else {
        return 0;
    }
}

int
os_tm_expires(tm_node_t *timer)
{
    struct tm_node *node = (struct tm_node *)timer;

    if (NULL==timer) {
        return os_assert_value(-EKEYNULL);
    }
    else if (tm_is_pending(node)) {
        return node->expires;
    } 
    else {
        return 0;
    }
}

bool
os_tm_is_pending(tm_node_t *timer)
{
    struct tm_node *node = (struct tm_node *)timer;

    if (NULL==timer) {
        return os_assert_value(false);
    } else {
        return tm_is_pending(node);
    }
}

static os_constructor void
__init(void)
{
    struct tm_ring *ring;
    int slot;
    
    if (sizeof(tm_node_t) != sizeof(struct tm_node)) {
        os_assert(0);
    }

    foreach_ring(ring) {
        foreach_slot(slot) {
            INIT_LIST_HEAD(tm_slot(ring, slot));
        }
    }
}

static os_destructor void
__fini(void)
{
    struct tm_node *node, *n;
    struct tm_ring *ring;
    int slot;

    foreach_ring(ring) {
        foreach_slot(slot) {
            struct list_head *head = tm_slot(ring, slot);
            
            list_for_each_entry_safe(node, n, head, node) {
                tm_remove(node);
            }
        }
    }
}

/******************************************************************************/
AKID_DEBUGER; /* must last os_constructor */

