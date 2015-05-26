#ifndef __BLOB_H_6E8612026DBB35E2A87ABE557208C182__
#define __BLOB_H_6E8612026DBB35E2A87ABE557208C182__
/******************************************************************************/
enum {
    BLOB_F_FIXED    = 0x01,
    BLOB_F_MIN      = 0x02,
    BLOB_F_MAX      = 0x04,
    BLOB_F_STRING   = 0x08, /* last must zero */
};

enum {
    BLOB_T_EMPTY    = 0,
	BLOB_T_OBJECT   = 1,
	BLOB_T_ARRAY    = 2,
	BLOB_T_STRING   = 3,
	BLOB_T_BINARY   = 4,
	BLOB_T_BOOL     = 5,
	BLOB_T_INT32    = 6,
	BLOB_T_INT64    = 7,
	
	BLOB_T_END
};

static inline bool
is_good_blob_type(int type)
{
    return is_good_enum(type, BLOB_T_END);
}

#define BLOB_ALIGN(_len)    os_align(_len, sizeof(uint32_t))

/*
* liujf:
    data: align by 4
*/
typedef struct {
    uint16_t    id;
    uint8_t     type;       /* enum blob_type */
    uint8_t     name_len;   /* real name len, NOT include '\0' */
	uint32_t    len;        /* real blob len, include data and header */
	
	char data[];            /* include name and value */
} blob_t;

static inline unsigned int
blob_id(const blob_t *blob)
{
    return (unsigned int)blob->id;
}

static inline unsigned int
blob_len(const blob_t *blob)
{
    return (unsigned int)blob->len;
}

static inline int
blob_type(const blob_t *blob)
{
    return blob->type;
}

static inline unsigned int
blob_size(const blob_t *blob)
{
    return BLOB_ALIGN(blob_len(blob));
}

static inline char *
blob_data(const blob_t *blob)
{
    return ((blob_t *)blob)->data;
}

static inline unsigned int
blob_data_len(const blob_t *blob)
{
    return blob_len(blob) - sizeof(*blob);
}

static inline unsigned int
blob_data_size(const blob_t *blob)
{
    return BLOB_ALIGN(blob_data_len(blob));
}

static inline unsigned int
blob_name_len(const blob_t *blob)
{
    return (unsigned int)blob->name_len;
}

static inline unsigned int
blob_name_size(const blob_t *blob)
{
    unsigned int name_len = blob_name_len(blob);
    
    return name_len?BLOB_ALIGN(name_len + 1 /* '\0' */):0;
}

static inline unsigned int
blob_name_pad_len(const blob_t *blob)
{
    unsigned int name_len = blob_name_len(blob);
    
    return name_len?(blob_name_size(blob) - name_len - 1):0;
}

static inline char *
blob_name(const blob_t *blob)
{
    return blob_name_len(blob)?blob_data(blob):NULL;
}

static inline char *
blob_NAME(const blob_t *blob)
{
    return blob_name_len(blob)?blob_data(blob):__empty;
}

static inline void *
blob_name_pad(const blob_t *blob)
{
    unsigned int name_len = blob_name_len(blob);
    
    return name_len?(blob_name(blob) + name_len + 1):NULL;
}

/*
* if value is string,
*   blob_value_len include the '\0'
*/
static inline unsigned int
blob_value_len(const blob_t *blob)
{
    return blob_data_len(blob) - blob_name_size(blob);
}

static inline unsigned int
blob_value_size(const blob_t *blob)
{
    return BLOB_ALIGN(blob_value_len(blob));
}

static inline unsigned int
blob_value_pad_len(const blob_t *blob)
{
    return blob_value_size(blob) - blob_value_len(blob);
}

static inline char *
__blob_value(const blob_t *blob)
{
    return blob_data(blob) + blob_name_size(blob);
}

#define blob_value(_type, _blob)    ((_type *)(_blob))

static inline void *
blob_value_pad(const blob_t *blob)
{
    return __blob_value(blob) + blob_value_len(blob);
}

static inline void
__blob_dump(const blob_t *blob)
{
#if 0
    os_println("blob");
    os_println(__tab "id:%d", blob_id(blob));
    os_println(__tab "len:%d", blob_len(blob));
    os_println(__tab "type:%d", blob_type(blob));
    os_println(__tab "name:%s", blob_NAME(blob));
    os_println(__tab "name_len:%d", blob_name_len(blob));
    switch(blob_type(blob)) {
        case BLOB_T_STRING:
            os_println(__tab "value:%s", (char *)__blob_value(blob));
            break;
        case BLOB_T_BOOL:
            os_println(__tab "value:%d", *(int *)__blob_value(blob));
            break;
        case BLOB_T_INT32:
            os_println(__tab "value:%d", *(int32_t *)__blob_value(blob));
            break;
        case BLOB_T_INT64:
            os_println(__tab "value:%d", *(int64_t *)__blob_value(blob));
            break;
        
    }
    os_println(__tab "value_len:%d", blob_value_len(blob));
    os_println(__tab "data:%p", blob_data(blob));
    os_println(__tab "name:%p", blob_name(blob));
    os_println(__tab "value:%p", __blob_value(blob));
    os_println("");
#endif
}

static inline uint32_t
blob_get_u32(const blob_t *blob)
{
	return *blob_value(uint32_t, blob);
}

static inline uint64_t
blob_get_u64(const blob_t *blob)
{
	return *blob_value(uint64_t, blob);
}

static inline int32_t
blob_get_i32(const blob_t *blob)
{
    return *blob_value(int32_t, blob);
}

static inline int64_t
blob_get_i64(const blob_t *blob)
{
	return *blob_value(int64_t, blob);
}

static inline bool
blob_get_bool(const blob_t *blob)
{
	return !!blob_get_i32(blob);
}

static inline const char *
blob_get_string(const blob_t *blob)
{
	return __blob_value(blob);
}

static inline blob_t *
blob_next(const blob_t *blob)
{
	return (blob_t *) ((char *)blob + blob_size(blob));
}

static inline void
blob_zero_pad(blob_t *blob)
{
    os_memzero(blob_name_pad(blob), blob_name_pad_len(blob));
    os_memzero(blob_value_pad(blob), blob_value_pad_len(blob));
}

static inline void
blob_set_value_len(blob_t *blob, unsigned int len /* real value len */)
{
    /*
    * header + name size + value len
    */
	blob->len = sizeof(*blob) + blob_name_size(blob) + len;
}

static inline void
blob_add_value_len(blob_t *blob, unsigned int len)
{
    blob->len += len;
}

static inline void
blob_sub_value_len(blob_t *blob, unsigned int len)
{
    if (len < blob_value_len(blob)) {
        blob->len -= len;
    } else {
        trace_assert(0, "sub len(%d) < blob_value_len(%d)", len, blob_value_len(blob));
    }
}

static inline bool
blob_eq(const blob_t *a, const blob_t *b)
{
    unsigned int len;
    
	if (!a && !b) {
		return true;
    }
	else if (!a || !b) {
		return false;
    }

    
    len = blob_size(a);
	if (len != blob_size(b)) {
		return false;
    }
    
	return os_memeq(a, b, len);
}

#define __blob_for_each_attr(_pos, _blob, _rem)         \
	for (_pos = (void *) _blob;                         \
	     _rem > 0 && (blob_size(_pos) <= _rem) &&       \
	        (blob_size(_pos) >= sizeof(blob_t));        \
	     _rem -= blob_size(_pos), _pos = blob_next(_pos)) \
    /* end */

/*
* liujf:
*   more safely
*/
#define blob_for_each_attr(_pos, _blob, _rem)           \
	for (_rem = _blob ? blob_size(_blob) : 0,           \
	        _pos = _blob?blob_value(blob_t, _blob):0;   \
	     _rem > 0 && (blob_size(_pos) <= _rem) &&       \
	        (blob_size(_pos) >= sizeof(blob_t));        \
	     _rem -= blob_size(_pos), _pos = blob_next(_pos)) \
    /* end */

static inline bool
blob_base_check(const void *value, unsigned int len, int type)
{
    static struct {
        uint32_t flag;
        uint32_t minsize;
        uint32_t maxsize;
    } rule[BLOB_T_END] = {
        [BLOB_T_OBJECT] = {
            .flag   = BLOB_F_MIN,
            .minsize= sizeof(blob_t),
        },
        [BLOB_T_ARRAY] = {
            .flag   = BLOB_F_MIN,
            .minsize= sizeof(blob_t),
        },
        [BLOB_T_STRING] = {
            .flag   = BLOB_F_MIN | BLOB_F_STRING,
            .minsize= 1,
        },
        [BLOB_T_BINARY] = {
            .flag   = BLOB_F_MIN,
            .minsize= 1,
        },
        [BLOB_T_EMPTY] = {
            .flag   = BLOB_F_FIXED,
        },
        [BLOB_T_BOOL] = {
            .flag   = BLOB_F_FIXED,
            .minsize= sizeof(uint32_t),
        },
        [BLOB_T_INT32] = {
            .flag   = BLOB_F_FIXED,
            .minsize= sizeof(uint32_t),
        },
        [BLOB_T_INT64] = {
            .flag   = BLOB_F_FIXED,
            .minsize= sizeof(uint64_t),
        },
    };
    
    if (false==is_good_blob_type(type)) {
        return false;
    }
    
    int flag    = rule[type].flag;
    int os_min  = rule[type].minsize;
    char *data  = (char *)value;
    
    if ((BLOB_F_FIXED & flag) && len!=os_min) {
        return false;
    }
    else if ((BLOB_F_MIN & flag) && len < os_min) {
        return false;
    }
    else if ((BLOB_F_MAX & flag) && len > rule[type].maxsize) {
        return false;
    }
    else if ((BLOB_F_STRING & flag) && data[len - 1]) {
        return false;
    }
    else {
        return true;
    }
}
    
typedef struct blob_rule {
    const char *name;
    int type;
    unsigned int minlen; /* os_min value len */
    unsigned int maxlen; /* os_max value len */
    bool (*validate)(const struct blob_rule *, blob_t *);
} blob_rule_t;

static inline int
blob_find_byname(const char *name, const blob_rule_t rule[], int count)
{
    int i;

    for (i=0; i<count; i++) {
        if (rule[i].name && 0==os_strcmp(rule[i].name, name)) {
            return i;
        }
    }
    
    return -ENOEXIST;
}

static inline int
blob_parse(blob_t *blob, blob_t *cache[], const blob_rule_t rule[], int count)
{
    blob_t *pos;
    int found = 0, rem;

    if (NULL==cache) {
        return os_assertV(-EINVAL1);
    }
    os_memzero(cache, sizeof(blob_t *) * count);
    
    blob_for_each_attr(pos, blob, rem) {
        int id;
        int nid = -1;
        int bid = blob_id(pos);
        unsigned int len    = blob_value_len(pos);
        const char *name    = blob_name(pos);
        int type = blob_type(pos);
        
        if (false==is_good_blob_type(type)) {
            continue;
        }

        /*
        * try name first
        */
        if (name && rule) {
            nid = blob_find_byname(name, rule, count);
        }
        
        if (is_good_enum(nid, count)) {
            id = nid;
        } else if (is_good_enum(bid, count)) {
            id = bid;
        } else {
            continue; /* bad id */
        }

        if (rule) {
            if (type != rule[id].type) {
                continue;
            }
            
            if (false==blob_base_check(__blob_value(pos), len, type)) {
                continue;
            }
            
            /*
            * 如果不是名字优先, 且规则指定了名字, 但blob名字不匹配
            *   则忽略 
            */
            if (false==is_good_enum(nid, count) && 
                rule[id].name &&
                (NULL==name || 0!=os_strcmp(rule[id].name, name))) {

                continue;
            }
            
            if (rule[id].minlen && len < rule[id].minlen)
                continue;

            if (rule[id].maxlen && len > rule[id].maxlen)
                continue;

            if (rule[id].validate && !rule[id].validate(&rule[id], pos))
                continue;
        }

        if (!cache[id])
            found++;

        cache[id] = pos;
    }
    
    return found;
}

static inline blob_t *
blob_root(slice_t *slice)
{
    return (blob_t *)slice_cookie(slice);
}

static inline void
__blob_init(
    blob_t *blob, 
    int id, 
    int type, 
    const char *name, 
    int payload
)
{
    blob->id          = id;
    blob->type        = type;
    blob->name_len    = name?os_strlen(name):0;
    blob_set_value_len(blob, payload);
}

static inline blob_t *
__blob_new(
    slice_t *slice, 
    bool put, 
    int id, 
    int type, 
    const char *name, 
    int payload
)
{
	blob_t *blob, tba;
    int size;
    
    __blob_init(&tba, id, type, name, payload);
    
    size = blob_size(&tba);
    
    debug_test("__blob_new type=%d, name=%s, payload=%d, size=%d", type, name, payload, size);
    debug_test("__blob_new slice: size=%d, used=%d, remain=%d", 
        slice_size(slice),
        slice_tail(slice) - slice_data(slice),
        slice_remain(slice));

    if ((slice_remain(slice) < size) && slice_grow(slice, 0)) {
        return NULL;
    }
    else if (slice_remain(slice) < size) {
        return NULL;
    }
    
    debug_test("__blob_new slice: size=%d, used=%d, remain=%d", 
        slice_size(slice),
        slice_len(slice),
        slice_remain(slice));
    
    blob = (blob_t *)slice_tail(slice);
    __blob_init(blob, id, type, name, payload);
    
    if (name) {
        os_strcpy(blob_name(blob), name);
    }
    blob_zero_pad(blob);
    
    if (put) {
        blob_t *root = blob_root(slice);
        slice_put(slice, size);
        if (root) {
            blob_add_value_len(root, size);
        }
    }

	return blob;
}

static inline blob_t *
blob_new(slice_t *slice, int id, int type, const char *name, int payload)
{
	return __blob_new(slice, true, id, type, name, payload);
}

static inline blob_t *
blob_root_init(slice_t *slice, int id /* root */, int type, const char *name)
{
    slice_offset(slice) = 0;

    return blob_new(slice, id, type, name, 0);
}

static inline void *
__blob_nest_start(slice_t *slice, int id, bool array, const char *name)
{
	blob_t *root_new;
	int type = array?BLOB_T_ARRAY:BLOB_T_OBJECT;
    
	root_new = blob_new(slice, id, type, name, 0);
	if (NULL==root_new) {
        return NULL;
	} else {
	    int offset = slice_offset(slice);
	    
        slice_offset(slice) = (unsigned char *)root_new - (unsigned char *)slice_data(slice);

        return (void *)offset;
	}
}

static inline void
__blob_nest_end(slice_t *slice, void *cookie)
{
	blob_t *root_new, *root_old;
    
	root_new = blob_root(slice);
	slice_offset(slice) = (int)cookie;
	root_old = blob_root(slice);
	
	blob_add_value_len(root_old, blob_value_size(root_new));
}

static inline void *
blob_object_start(slice_t *slice, int id, const char *name)
{
    return __blob_nest_start(slice, id, false, name);
}

static inline void
blob_object_end(slice_t *slice, void *cookie)
{
    __blob_nest_end(slice, cookie);
}

static inline void *
blob_array_start(slice_t *slice, int id, const char *name)
{
    return __blob_nest_start(slice, id, true, name);
}

static inline void
blob_array_end(slice_t *slice, void *cookie)
{
    __blob_nest_end(slice, cookie);
}

#if 0 /* todo: debug it */
#define blob_sprintf(_slice, _id, _name, _fmt, _args...) ({ \
    __label__ error;                                \
    __label__ ok;                                   \
    int len, space, size;                           \
    int payload = 1 + os_strlen(_fmt);              \
    blob_t *blob = __blob_new(_slice, false, _id,   \
        BLOB_T_STRING, _name, payload);             \
                                                    \
    /* pub blob header/_name */                     \
    size = sizeof(*blob) + blob_name_size(blob);    \
    if (NULL==slice_put(_slice, size)) {           \
        blob = NULL; goto error;                    \
    }                                               \
                                                    \
    space = slice_remain(_slice);                   \
    debug_ok("blob_sprintf: remain %d", space);     \
                                                    \
    len = slice_sprintf(_slice,                     \
        SLICE_F_GROW, _fmt, ##_args);               \
    if (os_snprintf_is_full(space, len)) {          \
        debug_error("blob_sprintf: full");         \
        blob = NULL; goto error;                    \
    }                                               \
                                                    \
    /* pad blob value */                            \
    len++; /* '\0' */                               \
    len = BLOB_ALIGN(len);                          \
    if (os_snprintf_is_full(space, len)             \
        && slice_grow(_slice, len - space)) {       \
        blob = NULL; goto error;                    \
    }                                               \
    /* put blob value */                            \
    slice_put(_slice, len);                         \
    blob_add_value_len(blob, len);                  \
    blob_add_value_len(blob_root(_slice),           \
        blob_size(blob));                           \
    if (__is_debug_level_trace) {                    \
        __blob_dump(blob);                          \
    }                                               \
                                                    \
    goto ok;                                        \
error:                                              \
    slice_trim(_slice, size);                       \
ok:                                                 \
    blob;                                           \
})  /* end */
#else
#define blob_sprintf(_slice, _id, _name, _fmt, _args...)    NULL

#endif

static inline blob_t *
blob_put(
    slice_t *slice, 
    int id, 
    int type, 
    const char *name, 
    const void *value, 
    unsigned int len
)
{
	blob_t *blob;
    
    if (value) {
        if (len) {
            /* do nothing */
        } else {
            trace_assert(0, "blob_put len 0");
            
            return NULL;
        }
    } else {
        if (len) {
            trace_assert(0, "blob_put value nil");
            
            return NULL;
        } else {
            type = BLOB_T_EMPTY;
        }
    }
    
	blob = blob_new(slice, id, type, name, len);
	if (NULL==blob) {
		return NULL;
	}
    
    if (value) {
	    os_memcpy(__blob_value(blob), value, len);
	}
    if (__is_debug_level_trace) {
	    __blob_dump(blob);

        os_printf("blob_put" __crlf
            __tab "slice(size=%d, used=%d, remain=%d)" __crlf
            __tab "root(offset=%d, name=%s, type=%d, len=%d)" __crlf, 
            slice_size(slice),
            slice_tail(slice) - slice_data(slice),
            slice_remain(slice),
            slice_offset(slice),
            blob_NAME(blob_root(slice)),
            blob_type(blob_root(slice)),
            blob_len(blob_root(slice)));
	}

	return blob;
}

static inline blob_t *
blob_put_string(slice_t *slice, int id, const char *name, const char *str)
{
	return blob_put(slice, id, BLOB_T_STRING, name, str, 1 + os_strlen(str));
}

static inline blob_t *
blob_put_binary(slice_t *slice, int id, const char *name, const void *binary, unsigned int len)
{
	return blob_put(slice, id, BLOB_T_BINARY, name, binary, len);
}

static inline blob_t *
blob_put_bool(slice_t *slice, int id, const char *name, bool val)
{
    int32_t b = !!val;
    
	return blob_put(slice, id, BLOB_T_BOOL, name, &b, sizeof(b));
}

static inline blob_t *
blob_put_u32(slice_t *slice, int id, const char *name, uint32_t val)
{
	return blob_put(slice, id, BLOB_T_INT32, name, &val, sizeof(val));
}

static inline blob_t *
blob_put_u64(slice_t *slice, int id, const char *name, uint64_t val)
{
	return blob_put(slice, id, BLOB_T_INT64, name, &val, sizeof(val));
}

static inline blob_t *
blob_put_i32(slice_t *slice, int id, const char *name, int32_t val)
{
	return blob_put(slice, id, BLOB_T_INT32, name, &val, sizeof(val));
}

static inline blob_t *
blob_put_i64(slice_t *slice, int id, const char *name, int64_t val)
{
	return blob_put(slice, id, BLOB_T_INT64, name, &val, sizeof(val));
}

static inline blob_t *
blob_put_blob(slice_t *slice, blob_t *blob)
{
	return blob_put(slice, blob_id(blob), blob_type(blob), blob_name(blob), 
	            __blob_value(blob), blob_value_len(blob));
}

#ifdef __APP__
static inline void
__blob_byteorder(blob_t *blob, bool ntoh)
{
    int rem;
    int type = blob_type(blob);

    if (ntoh) {
        blob->id  = bswap_16(blob->id);
        blob->len = bswap_32(blob->len);
    }
    
    switch(type) {
        case BLOB_T_OBJECT:
        case BLOB_T_ARRAY: {
            blob_t *pos;
            
            blob_for_each_attr(pos, blob, rem) {
                __blob_byteorder(pos, ntoh);
            }
        }   break;
        case BLOB_T_INT64:
            *(int64_t *)blob->data = bswap_64(blob_get_i64(blob));
            break;
        case BLOB_T_BOOL:
        case BLOB_T_INT32:
            *(int32_t *)blob->data = bswap_32(blob_get_i32(blob));
            break;
        case BLOB_T_STRING:
        case BLOB_T_BINARY:
        case BLOB_T_EMPTY:
        default:
            /* do nothinig */
            break;
    }
    
    if (false==ntoh) {
        blob->id  = bswap_16(blob->id);
        blob->len = bswap_32(blob->len);
    }
}

static inline void
blob_ntoh(blob_t *blob)
{
    __blob_byteorder(blob, true);
}

static inline void
blob_hton(blob_t *blob)
{
    __blob_byteorder(blob, false);
}
#endif

/******************************************************************************/
#endif /* __BLOB_H_6E8612026DBB35E2A87ABE557208C182__ */
