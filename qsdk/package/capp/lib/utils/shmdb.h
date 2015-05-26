#ifndef __SDB_H_A0E0242A86C5BDBA9F0D13D3344CEA52__
#define __SDB_H_A0E0242A86C5BDBA9F0D13D3344CEA52__
#ifdef __APP__
/******************************************************************************/
#ifndef SDB_ENTRY_SIZE
#define SDB_ENTRY_SIZE          64
#endif

#ifndef SDB_ENTRY_LIMIT
#define SDB_ENTRY_LIMIT         1023
#endif

#ifndef SDB_HASHSIZE
#define SDB_HASHSIZE            256
#endif

#ifndef SDB_HASHCOUNT
#define SDB_HASHCOUNT           1
#endif

#define sdb_shm_size    (0  \
    + sizeof(sdb_hdr_t)     \
    + sizeof(sdb_entry_t) * SDB_ENTRY_LIMIT \
    + sizeof(sdb_tail_t)    \
)   /* end */

enum {
    SDB_F_USING = 0x01,
};

typedef struct {
    char data[SDB_ENTRY_SIZE];
    
    unsigned int flag;
    
    struct {
        struct hlist_node hash[SDB_HASHCOUNT];
        struct list_head free, used;
    } node;
} sdb_entry_t;

typedef struct {
    unsigned int protect;
    bool inited;
    int count;
    int limit;
    
    sdb_entry_t entry[0];
} sdb_hdr_t;

typedef struct {
    unsigned int protect;
} sdb_tail_t;

typedef struct {
    void (*init)(sdb_entry_t *entry, void *data);
    int (*hash)(void *data, int ii);
    int (*hash_master)(void *data);
    int (*hash_slave)(void *data);
    bool (*match)(sdb_entry_t *entry, void *data);
} sdb_ops_t;

typedef struct {
    os_shm_t shm;

    sdb_ops_t *ops;

    struct {
        struct hlist_head hash[SDB_HASHCOUNT][SDB_HASHSIZE];
        struct list_head free, used;
    } head;
} sdb_table_t;

#ifndef INLINE_SDB
#define INLINE_SDB         __SDB
#endif

#define INLINE_SDB_INITER(_shm_id, _ops) { \
    .shm    = OS_SHM_INITER(_shm_id), \
    .ops    = _ops, \
}   /* end */

#define USE_INLINE_SDB    sdb_table_t INLINE_SDB
extern USE_INLINE_SDB;

static inline sdb_table_t *
__sdb(void)
{
#ifdef __USE_INLINE_SDB
    return &INLINE_SDB;
#else
    return NULL;
#endif
}

static inline sdb_ops_t *
__sdb_ops(void)
{
    return __sdb()->ops;
}

static inline bool
__sdb_ops_match(sdb_entry_t *entry, void *data)
{
    if (__sdb_ops()->match) {
        return (*__sdb_ops()->match)(entry, data);
    } else {
        return os_objeq(entry->data, data);
    }
}

static inline sdb_entry_t *
__sdb_ops_init(sdb_entry_t *entry, void *data)
{
    if (__sdb_ops()->init) {
        (*__sdb_ops()->init)(entry, data);
    } else {
        os_objcpy(entry->data, data);
    }

    return entry;
}

static inline int
__sdb_ops_hash(void *data, int ii)
{
    int idx;

    if (NULL==data) {
        return os_assertV(0);
    }
    else if (false==is_good_enum(ii, SDB_HASHCOUNT)) {
        return os_assertV(0);
    }
    else if (NULL==__sdb_ops()->hash) {
        return os_assertV(0);
    }

    idx = (*__sdb_ops()->hash)(data, ii);
    if (false==is_good_enum(idx, SDB_HASHSIZE)) {
        return os_assertV(0);
    }

    return idx;
}

static inline struct hlist_head *
__sdb_hh(void *data, int ii)
{
    return &__sdb()->head.hash[ii][__sdb_ops_hash(data, ii)];
}

static inline sdb_hdr_t *
__sdb_hdr(void)
{
    return (sdb_hdr_t *)__sdb()->shm.address;
}

static inline sdb_entry_t *
__sdb_entry(int idx)
{
    return &__sdb_hdr()->entry[idx];
}

#define __sdb_inited            __sdb_hdr()->inited
#define __sdb_count             __sdb_hdr()->count
#define __sdb_limit             __sdb_hdr()->limit
#define __sdb_0                 __sdb_entry(0)
#define __sdb_end               __sdb_entry(__sdb_limit)
#define __sdb_foreach(_entry)   for (_entry=__sdb_0; _entry<__sdb_end; _entry++)

static inline sdb_tail_t *
__sdb_tail(void)
{
    return (sdb_tail_t *)__sdb_end;
}

#define __sdb_protect_0         __sdb_hdr()->protect
#define __sdb_protect_1         __sdb_tail()->protect

static inline int 
__sdb_entry_idx(sdb_entry_t *entry)
{
    return entry - __sdb_0;
}

static inline bool
__sdb_entry_is_using(sdb_entry_t *entry)
{
    return os_hasflag(entry->flag, SDB_F_USING);
}

static inline sdb_entry_t *
__sdb_entry_safe(int id)
{
    if (is_good_enum(id, __sdb_limit)) {
        return __sdb_entry(id);
    } else {
        return NULL;
    }
}

static inline sdb_entry_t *
__sdb_entry_using(int id)
{
    sdb_entry_t *entry = __sdb_entry_safe(id);
    
    if (entry && __sdb_entry_is_using(entry)) {
        return entry;
    } else {
        return NULL;
    }
}

static inline sdb_entry_t *
__sdb_entry_find(void *data, int ii)
{
    sdb_entry_t *entry = NULL;
    
    hlist_for_each_entry(entry, __sdb_hh(data, ii), node.hash[ii]) {
        if (__sdb_ops_match(entry, data)) {
            return entry;
        }
    }
    
    return NULL;
}

static inline void
__sdb_hash_insert(sdb_entry_t *entry)
{
    int i;

    for (i=0; i<SDB_HASHCOUNT; i++) {
        hlist_add_head(&entry->node.hash[i], __sdb_hh(entry->data, i));
    }
}

static inline void
__sdb_hash_remove(sdb_entry_t *entry)
{
    int i;

    for (i=0; i<SDB_HASHCOUNT; i++) {
        hlist_del(&entry->node.hash[i]);
    }
}

static inline void
__sdb_entry_init(sdb_entry_t *entry)
{
    if (__sdb_entry_is_using(entry)) {
        __sdb_hash_insert(entry);
        list_add(&entry->node.used, &__sdb()->head.used);
    } else {
        list_add(&entry->node.free, &__sdb()->head.free);
    }
}

static inline sdb_entry_t *
__sdb_entry_insert(sdb_entry_t *entry)
{
    if (entry && false==__sdb_entry_is_using(entry)) {
        __sdb_hash_insert(entry);
        list_add(&entry->node.used, &__sdb()->head.used);
        list_del(&entry->node.free);

        os_setflag(entry->flag, SDB_F_USING);

        __sdb_count++;
    }
    
    return entry;
}

static inline void
__sdb_entry_remove(sdb_entry_t *entry)
{
    if (entry && __sdb_entry_is_using(entry)) {
        __sdb_hash_remove(entry);
        list_del(&entry->node.used);
        list_add(&entry->node.free, &__sdb()->head.free);

        os_clrflag(entry->flag, SDB_F_USING);
        __sdb_count--;
    }
}

static inline sdb_entry_t *
__sdb_entry_new(void)
{
    struct list_head *head = &__sdb()->head.free;
    
    if (list_empty(head)) {
        return NULL;
    } else {
        return list_first_entry(head, sdb_entry_t, node.free);
    }
}

static inline sdb_entry_t *
sdb_new(void *data)
{
    sdb_entry_t *entry;

    if (NULL==data) {
        return NULL;
    }
    
    entry = __sdb_entry_new();
    if (NULL==entry) {
        return NULL;
    }
    
    return __sdb_entry_insert(__sdb_ops_init(entry, data));
}

#define __sdb_free(_entry)  __sdb_entry_remove(_entry)

#define sdb_free(_entry)    do{ \
    __sdb_free(_entry);         \
    _entry = NULL;              \
}while(0)

static inline sdb_entry_t * 
sdb_find(void *data, int ii)
{
    if (NULL==data) {
        return os_assertV(NULL);
    }
    
    return __sdb_entry_find(data, ii);
}

static inline sdb_entry_t * 
sdb_get(int id)
{
    return __sdb_entry_using(id);
}

static inline int 
sdb_fini(void) 
{
    os_shm_destroy(__sdb());
    
    return 0;
}

static inline int 
sdb_init(void) 
{
    os_shm_t    *shm = &__sdb()->shm;
    sdb_entry_t *entry;
    int err = 0;

    err = os_shm_create(shm, sdb_shm_size, false);
    if (err<0) { /* >=0 is valid shm id */
        goto error;
    }
    
    if (false==__sdb_inited) {
        __sdb_inited    = true;
        __sdb_limit     = SDB_ENTRY_LIMIT;
        __sdb_protect_0 = OS_PROTECTED;
        __sdb_protect_1 = OS_PROTECTED;
    }

    __sdb_foreach(entry) {
        __sdb_entry_init(entry);
    }

    debug_ok("sdb init OK!");
    
    return 0;
error:
    debug_error("sdb init failed!");
    
    sdb_fini();
    
    return err;
}

/******************************************************************************/
#endif /* __APP__ */
#endif /* __SDB_H_A0E0242A86C5BDBA9F0D13D3344CEA52__ */
