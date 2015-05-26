#ifndef __KV_H_B5FC55C7E424CCEF2F2B056FE536D4F8__
#define __KV_H_B5FC55C7E424CCEF2F2B056FE536D4F8__
/******************************************************************************/
#if 0
#ifndef KV_KMAXLEN
#define KV_KMAXLEN      15
#endif

#ifndef KV_VMAXLEN
#define KV_VMAXLEN      31
#endif

#ifndef KV_MAX
#define KV_MAX          32
#endif

typedef struct {
	string_t *k;
	string_t *v;

	struct list_head node;
} kv_t;

typedef struct {
	uint32_t hash;
	uint32_t ref:31;
    uint32_t sort:1; /* false:not sort, true:up sort */
    uint32_t count;
    struct mlist_node node;
    struct list_head  list;
} kvs_t;

typedef struct mlist_table kvc_t;

static inline uint32_t
__kv_hash(kv_t *kv)
{
	return __bkdr_push(kv->k->hash, kv->v->hash);
}

static inline uint32_t
__kvs_hash_push(kvs_t *kvs, kv_t *kv)
{
    kvs->hash = __bkdr_push(kvs->hash, __kv_hash(kv));

    return kvs->hash;
}

static inline uint32_t
__kvs_hash_pop(kvs_t *kvs, kv_t *kv)
{
    kvs->hash = __bkdr_pop(kvs->hash, __kv_hash(kv));

    return kvs->hash;
}

static inline bool
__kv_eq(kv_t *a, kv_t *b)
{
	if (__kv_hash(a) != __kv_hash(b)) {
		return false;
	} else {
		return string_eq(a->k, b->k) && string_eq(a->v, b->v);
	}
}

static inline bool
__kv_cmp(kv_t *a, kv_t *b)
{
    int ret;

    ret = string_cmp(a->k, b->k);
    if (ret) {
        return ret;
    }

    ret = string_cmp(a->v, b->v);
    if (ret) {
        return ret;
    }

    return ret;
}

static inline bool
__kv_cmp_bystring(kv_t *kv, char *name, char *value)
{
    int ret;

    ret = os_strcmp(kv->k->string, name);
    if (ret) {
        return ret;
    }

    ret = os_strcmp(kv->v->string, value);
    if (ret) {
        return ret;
    }

    return ret;
}

static inline int
__kv_insert_bysort(kvs_t *kvs, kv_t *kv)
{
    kv_t *p;
    
    list_for_each_entry(p, &kvs->list, node) {
        int ret = __kv_cmp(kv, p);
        if (0==ret) {
            return os_assertV(-EEXIST);
        }
        else if (ret <= 0) {
            /* 
            * kv <= p 
            *
            * insert kv before p
            */
            __list_add(&kv->node, p->node.prev, &p->node);

            return 0;
        }
    }

    list_add_tail(&kv->node, &kvs->list);

    return 0;
}

static inline int
__kv_insert(kvs_t *kvs, kv_t *kv)
{
    int err = 0;
    
    if (is_in_list(&kv->node)) {
        return os_assertV(-EINLIST);
    }
    else if (false==kvs->sort) {
        list_add(&kv->node, &kvs->list);
    } 
    else {
        err = __kv_insert_bysort(kvs, kv);
    }

    __kvs_hash_push(kvs, kv);

    return err;
}

static inline int
kv_insert(kvs_t *kvs, kv_t *kv)
{
    if (NULL==kvs) {
        return os_assertV(-EINVAL1);
    }
    else if (NULL==kv) {
        return os_assertV(-EINVAL2);
    }
    else {
        return __kv_insert(kvs, kv);
    }
}

static inline kv_t *
__kv_remove(kvs_t *kvs, kv_t *kv)
{
    if (is_in_list(&kv->node)) {
        list_del(&kv->node);

        __kvs_hash_pop(kvs, kv);
    }

    return kv;
}

static inline kv_t *
kv_remove(kvs_t *kvs, kv_t *kv)
{
    if (NULL==kvs) {
        return NULL; /* NOT assert */
    }
    else if (NULL==kv) {
        return NULL; /* NOT assert */
    }
    else {
        return __kv_remove(kvs, kv);
    }
}

static inline void
__kv_release_kv(kv_t *kv)
{
	if (kv) {
		os_free(kv->k);
		os_free(kv->v);
	}
}

static inline void
__kv_release(kv_t *kv)
{
    __kv_release_kv(kv);
    os_free(kv);
}

#define __kv_SET(_kv, _obj, _string)    ({ \
	string_t *old = (_kv)->_obj;            \
    int err = 0;                            \
                                            \
	(_kv)->_obj = string_new(0, _string);   \
	if (NULL==(_kv)->_obj) {                \
		err = -ENOMEM;                      \
	} else {                                \
        os_free(old);                       \
	}                                       \
                                            \
	err;                                    \
})

static inline int
__kv_setk(kv_t *kv, char *name)
{
    return __kv_SET(kv, k, name);
}

static inline int
__kv_setv(kv_t *kv, char *value)
{
	return __kv_SET(kv, v, value);
}

static inline int
__kv_set(kv_t *kv, char *name, char *value)
{
	__kv_setk(kv, name);
	if (NULL==kv->k) {
		goto error;
	}

	__kv_setv(kv, value);
	if (NULL==kv->v) {
		goto error;
	}

	return 0;
 error:
	__kv_release_kv(kv);

	return -ENOMEM;
}

static inline kv_t *
__kv_new(char *name, char *value)
{
    kv_t *kv = (kv_t *)os_zalloc(*kv);
    if (NULL==kv) {
        goto error;
    }

    if (__kv_set(kv, name, value)) {
        goto error;
    }

    return kv;
error:
    os_free(kv);

    return NULL;
}

static inline int
__kvs_ref_add(kvs_t * kvs)
{
    return ++kvs->ref;
}

static inline int
__kvs_ref_sub(kvs_t *kvs)
{
    if (kvs->ref) {
    	return --kvs->ref;
    } else {
        return os_assertV(0);
    }
}

static inline void
__kvs_release(kvs_t *kvs)
{
    if (kvs->ref) {
        __kvs_ref_sub(kvs);
    }
    else {
    	kv_t *kv, *tmp;

    	list_for_each_entry_safe(kv, tmp, &kvs->list, node) {
            __kv_release(__kv_remove(kvs, kv));
    	}

    	kvs->hash = 0;
	}
}

static inline void
kvs_release(kvs_t *kvs)
{
    if (NULL==kvs) {
        os_assert(0);
    }
    else if (kvs->ref) {
        __kvs_release(kvs);
    }
}

static inline kvs_t *
__kvs_clone(kvs_t *kvs)
{
	__kvs_ref_add(kvs);

	return kvs;
}

static inline kvs_t *
kvs_clone(kvs_t *kvs)
{
	if (NULL==kvs) {
        return os_assertV(NULL);
	} else {
        return __kvs_clone(kvs);
	}
}

static inline kv_t *
__kvs_find_byname(kvs_t *kvs, char *name)
{
	kv_t *kv;

    list_for_each_entry(kv, &kvs->list, node) {
        int ret = os_strcmp(kv->k->string, name);
        if (0==ret) {
            return kv;
        }
        else if (ret>0 && kvs->sort) {
            /*
            * kvs list is up-sort
            * 
            * kv > name, then 
            */
            return NULL;
        }
	}

	return NULL;
}

static inline kv_t *
kvs_find_byname(kvs_t *kvs, char *name)
{
	if (NULL==kvs) {
		return os_assertV(NULL);
	} else if (NULL==name) {
		return os_assertV(NULL);
	} else {
        return __kvs_find_byname(kvs, name);
	}
}

static inline kv_t *
__kvs_find(kvs_t *kvs, char *name, char *value)
{
	kv_t *kv;

    list_for_each_entry(kv, &kvs->list, node) {
        int ret = __kv_cmp_bystring(kv, name, value);
        if (0==ret) {
            return kv;
        }
        else if (ret>0 && kvs->sort) {
            /*
            * kvs list is up-sort
            * 
            * kv > (name & value), then 
            */
            return NULL;
        }
	}

	return NULL;
}

static inline kv_t *
kvs_find(kvs_t *kvs, char *name, char *value)
{
	if (NULL==kvs) {
		return os_assertV(NULL);
	} else if (NULL==name) {
		return os_assertV(NULL);
	} else if (NULL==value) {
		return os_assertV(NULL);
	} else {
        return __kvs_find(kvs, name, value);
	}
}

static inline kv_t *
__kvs_find_bykv(kvs_t *kvs, kv_t *__kv)
{
	kv_t *kv;

    list_for_each_entry(kv, &kvs->list, node) {
        int ret = __kv_cmp(kv, __kv);
        if (0==ret) {
            return kv;
        }
        else if (ret>0 && kvs->sort) {
            /*
            * kvs list is up-sort
            * 
            * kv > __kv, then 
            */
            return NULL;
        }
	}

	return NULL;
	
}

static inline kv_t *
kvs_find_bykv(kvs_t *kvs, kv_t *kv)
{
	if (NULL==kvs) {
		return os_assertV(NULL);
	} else if (NULL==kv) {
		return os_assertV(NULL);
	} else {
        return __kvs_find_bykv(kvs, kv);
	}
}

static inline int
__kvs_push(kvs_t *kvs, char *name, char *value)
{
    int err = 0;
    kv_t *kv = NULL;

    if (kvs->count >= KV_MAX) {
		err = -ENOSPACE; goto error;
	}
    else if (kvs_find_byname(kvs, name)) {
		err = -EEXIST; goto error;
	}
	
	kv_t *kv = __kv_new(name, value);
	if (NULL==kv) {
        err = -ENOMEM; goto error;
	}
	
	err = __kv_insert(kvs, kv);
	if (err) {
        goto error;
	}

	return 0;
error:
    if (kv) {
        __kv_release(kv_remove(kvs, kv));
    }
    
    return err;
}

static inline int
kvs_push(kvs_t *kvs, char *name, char *value)
{
	if (NULL==kvs) {
		return os_assertV(-EINVAL1);
	} else if (NULL==name) {
        return os_assertV(-EINVAL2);
	} else if (NULL==value) {
	    return os_assertV(-EINVAL3);
	} else {
        return __kvs_push(kvs, name, value);
	}
}

static inline kv_t *
__kvs_pop(kvs_t *kvs, char *name, char *value)
{
    kv_t *kv = kvs_find(kvs, name, value);

    return kv?kv_remove(kvs, kv):NULL;
}

static inline kv_t *
kvs_pop(kvs_t *kvs, char *name, char *value)
{
	if (NULL==kvs) {
		return os_assertV(NULL);
	} else if (NULL==name) {
        return os_assertV(NULL);
	} else if (NULL==value) {
	    return os_assertV(NULL);
	} else {
        return __kvs_pop(kvs, name, value);
	}
}

static inline bool
__kvs_in_nosort(kvs_t *a, kvs_t *b)
{
	kv_t *kva, *kvb;
	
	list_for_each_entry(kva, &a->list, node) {
	    if (NULL==kvs_find_bykv(b, kva)) {
            return false;
	    }
	}

    return true;
}

/*
* is a in b ???
*/
static inline bool
__kvs_in_bysort(kvs_t *a, kvs_t *b)
{
	kv_t *kva;
	kv_t *kvb = list_first_entry(&b->list, typeof(*kvb), node);
	
	list_for_each_entry(kva, &a->list, node) {
        while(&kvb->node != &b->list) {
            kv_t *kvs = kvb; kvb = list_next_entry(kvb, node);
            
            if (__kv_eq(kva, kvs)) {
                goto match;
            }
        }

        return false;
match:
        /* continue */
	}
    
	return true;
}

/*
* is a in b ???
*/
static inline bool
__kvs_in(kvs_t *a, kvs_t *b)
{
    if (0==a->count) {
        return true;
    }
    else if (a->count > b->count) {
        return false;
    }
    else if (a->sort && b->sort) {
        return __kvs_in_bysort(a, b);
    }
    else {
        return __kvs_in_nosort(a, b);
    }
}

/*
* is a in b ???
*/
static inline bool
kvs_in(kvs_t *a, kvs_t *b)
{
	if (NULL==a) {
		return os_assertV(false);
	}
	else if (NULL==b) {
		return os_assertV(false);
	}
    else {
        return __kvs_in(a, b);
    }
}

static inline bool
__kvs_eq_bysort(kvs_t *a, kvs_t *b)
{
	kv_t *kva;
	kv_t *kvb = list_first_entry(&b->list, typeof(*kvb), node);
	
	list_for_each_entry(kva, &a->list, node) {
		if (false==__kv_eq(kva, kvb)) {
			return false;
		}

        kvb = list_next_entry(kvb, node);
	}
    
	return true;
}

static inline bool
__kvs_eq(kvs_t *a, kvs_t *b)
{
    if (0==a->count && 0==b->count) {
        return true;
    }
    else if (a->count != b->count) {
        return false;
    }
	else if (a->hash != b->hash) {
        return false;
    }
    else if (a->sort && b->sort) {
        return __kvs_eq_bysort(a, b);
    }
    else {
        return __kvs_in_nosort(a, b);
    }
}

static inline bool
kvs_eq(kvs_t *a, kvs_t *b)
{
	if (NULL==a) {
		return os_assertV(false);
	}
	else if (NULL==b) {
		return os_assertV(false);
	}
    else {
        return __kvs_eq(a, b);
    }
}

static inline kvs_t *
kvs_new(void)
{
	kvs_t *kvs = (kvs_t *) os_zalloc(*kvs);
	if (NULL==kvs) {
		return NULL;
	}
	
	INIT_LIST_HEAD(&kvs->node.list);
	__kvs_ref_add(kvs);

	return kvs;
}

#ifndef KV_CONTAINER_HASHSIZE
#define KV_CONTAINER_HASHSIZE   (1024*1024)
#endif

static inline uint32_t
kvs_hash(kvs_t *kvs)
{
	return kvs->hash & (KV_CONTAINER_HASHSIZE-1);
}

static inline int
kvc_init(struct mlist_table *table, uint32_t size)
{
    return mlist_table_init(table, size);
}

static inline kvs_t *
kvc_find(kvc_t *kvc, kvs_t *kvs)
{
	struct hlist_head *head = &kvc->hash[kvs_hash(kvs)];
	kvs_t *node;

	hlist_for_each_entry(node, head, node.hash) {
		if (kvs_eq(kvs, node)) {
			return node;
		}
	}

	return NULL;
}


static inline kvs_t *
__kvc_insert(kvc_t *kvc, kvs_t *kvs)
{
	kvs->ref++;
	list_add(&kvs->node.list, &kvc->list);
	hlist_add_head(&kvs->node.hash, &kvc->hash[kvs_hash(kvs)]);
	kvc->count++;
	
    return kvs;
}

static inline kvs_t *
kvc_insert(kvc_t *kvc, kvs_t *cache)
{
    if (NULL==kvc) {
        return os_assertV(NULL);
    }
    else if (NULL==cache) {
        return os_assertV(NULL);
    }
    
	kvs_t *kvs = kvc_find(kvc, cache);
    if (kvs) {
        return kvs;
    } else {
	    return __kvc_insert(kvc, cache);
    }
}

static inline int
__kvc_remove(kvc_t *kvc, kvs_t *kvs)
{
	kvs->ref--;
	if (0==kvs->ref) {
		list_del(&kvs->node.list);
		hlist_del(&kvs->node.hash);
		kvc->count--;

		kvs_release(kvs);
	}

	return 0;
}

static inline int
kvc_remove(kvc_t *kvc, kvs_t *cache)
{
	if (NULL==kvc) {
		return -EINVAL;
	} else if (NULL==cache) {
		return -EINVAL;
	}

	kvs_t *kvs = kvc_find(kvc, cache);
	if (NULL==kvs) {
        return -ENOEXIST;
	}
	else if (kvs->ref <= 0) {
		return -EBADREF;
	}
	else {
	    return __kvc_remove(kvc, kvs);
    }
}
#endif
/******************************************************************************/
#endif				/* __KV_H_B5FC55C7E424CCEF2F2B056FE536D4F8__ */
