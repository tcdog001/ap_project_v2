#ifndef __AUTOARRAY_H_D8422899B40DF42D4E9371468AD4BE8A__
#define __AUTOARRAY_H_D8422899B40DF42D4E9371468AD4BE8A__
/******************************************************************************/
typedef struct {
    unsigned char *base; /* array memory */
    int size;   /* item size */
    int count;  /* item total count */
    int limit;  /* item total limit */
    int grow;   /* once grow count */
    
    void (*init)(void *item);
    void (*clean)(void *item);
} autoarray_t;

static inline void *
__os_aa_item(autoarray_t *aa, int idx)
{
    if (idx < aa->count) {
        return (void *)(aa->base + idx * aa->size);
    } else {
        return NULL;
    }
}

static inline int
__os_aa_grow_to(autoarray_t *aa, int count)
{
    int old  = aa->count;
    int grow = count - old;
    
    if (grow <= 0) {
        return 0; //needn't grow
    }
    
    if (aa->limit && count > aa->limit) {
        return -ESPIPE;
    }
    
    aa->base = (unsigned char *)os_realloc(aa->base, aa->size * count);
    if (NULL==aa->base) {
        return -ENOMEM;
    }
    aa->count = count;
    
    if (aa->init) {
        int i;
        
        for (i=old; i<count; i++) {
            (*aa->init)(__os_aa_item(aa, i));
        }
    } else {
        os_memzero(__os_aa_item(aa, old), aa->size * grow);
    }
    
    return 0;
}

static inline int
__os_aa_grow(autoarray_t *aa)
{
    int count;
    
    if (NULL==aa) {
        return error_assertV(-EKEYNULL, "$aa is nil");
    }

    count = aa->count + aa->grow;
    if (aa->limit) {
        if (aa->count==aa->limit) {
            return -ESPIPE;
        }

        if (count > aa->limit) {
            count = aa->limit;
        }
    }

    return __os_aa_grow_to(aa, count);
}

static inline void
os_aa_clean(autoarray_t *aa)
{
    if (aa) {
        if (aa->clean) {
            int i;
            
            for (i=0; i<aa->count; i++) {
                (*aa->clean)(__os_aa_item(aa, i));
            }
        }

        if (aa->base) {
            os_free(aa->base);
        }
    }
}

static inline int
os_aa_init(
    autoarray_t *aa, 
    int size, 
    int count,
    int limit, 
    int grow, 
    void (*init)(void *item),
    void (*clean)(void *item)
)
{
    if (NULL==aa) {
        return error_assertV(-EKEYNULL, "aa is nil");
    }
    else if (size<=0) {
        return error_assertV(-EINVAL1, "size(%d)<=0", size);
    }
    else if (count<=0) {
        return os_assertV(-EINVAL2);
    }
    else if (limit<0) {
        return os_assertV(-EINVAL3);
    }
    else if (limit && count > limit) {
        return os_assertV(-EINVAL4);
    }
    else if (grow<0) {
        return os_assertV(-EINVAL5);
    }
    
    aa->size    = size;
    aa->count   = count;
    aa->limit   = limit;
    aa->grow    = grow;
    aa->init    = init;
    aa->clean   = clean;
    
    aa->base = (unsigned char *)os_calloc(aa->count, aa->size);
    if (NULL==aa->base) {
        return -ENOMEM;
    }
    
    if (aa->init) {
        int i;
        
        for (i=0; i<aa->count; i++) {
            (*aa->init)(__os_aa_item(aa, i));
        }
    }
    
    return 0;
}

static inline void *
os_aa_get(autoarray_t *aa, int idx, bool grow)
{
    void *value;
    
    if (NULL==aa) {
        return os_assertV(NULL);
    }
    
    if (idx<0) {
        return os_assertV(NULL);
    }
    
    value = __os_aa_item(aa, idx);
    if (NULL==value /* no space */ 
        && grow     /* try grow */ 
        && aa->grow /* can grow */ 
        && 0==__os_aa_grow_to(aa, idx) /* grow ok */
        ) {
        return __os_aa_item(aa, idx);
    }
    
    return value;
}

static inline int
os_aa_count(autoarray_t *aa)
{
    if (aa) {
        return aa->count;
    } else {
        return os_assertV(0);
    }
}
/******************************************************************************/
#endif /* __AUTOARRAY_H_D8422899B40DF42D4E9371468AD4BE8A__ */
