#ifndef __RSH_H_227352D437EF91A690DEE61AD59E4A66__
#define __RSH_H_227352D437EF91A690DEE61AD59E4A66__
/******************************************************************************/
#include "rsh/rsh_def.h"
/******************************************************************************/
struct rsh_base {
    unsigned char mac[OS_MACSIZE];
    uint16_t port;
    uint32_t ip;
    uint32_t permit:1;
    uint32_t fsm:31;
    int fd;
};

struct rsh_sym {
    symbol_t *board;
    symbol_t *tag[RSH_TAG_COUNT];
    symbol_t *rootfs[RSH_ROOTFS_COUNT];
    symbol_t *web;
};

struct rsh_info {
    char board[1+RSH_BOARD_NAMELEN];
    char tag[RSH_TAG_COUNT][1+RSH_TAG_VALUELEN];
    char rootfs[RSH_ROOTFS_COUNT][1+RSH_ROOTFS_NAMELEN];
    char web[1+RSH_WEB_NAMELEN];
};

struct rsh_cache {
    struct rsh_base base;
    struct rsh_sym  sym;
};

struct rsh_entry {
    struct rsh_base base;
    struct rsh_sym  sym;

    struct {
        tm_node_t tm;
        
        struct mlist_node mlist;
    } node;
};

struct rsh_control {
    struct {
        int fd;
        int port;
    } svr, cmd, cfg;

    struct {
        int fd;

        struct epoll_event *ev;
    } ep;

    struct {
        int fd;
    } tm;
    
    struct {
        symbol_table_t board;
        symbol_table_t tag;
        symbol_table_t rootfs;
        symbol_table_t web;
    } sym;
    
    struct {
        char tag[RSH_TAG_COUNT][1+RSH_TAG_NAMELEN];
    } map;
    
    struct {
        struct mlist_table mlist;
    } head;
};

#define RSH_CONTROL_INITER      {   \
    .svr = {                        \
        .fd   = -1,                 \
        .port = RSH_SERVICE_PORT,   \
    },                              \
    .cmd = {                        \
        .fd   = -1,                 \
        .port = RSH_COMMAND_PORT,   \
    },                              \
    .cfg = {                        \
        .fd   = -1,                 \
        .port = RSH_CONFIG_PORT,    \
    },                              \
    .tm = {                         \
        .fd   = -1,                 \
    },                              \
    .ep = {                         \
        .fd   = -1,                 \
    },                              \
}

static inline int
rsh_sym_init(struct rsh_control *c)
{
    int err;
    
    err = mlist_table_init(&c->sym.board, RSH_BOARD_HASH);
    if (err) {
        return err;
    }
    
    err = mlist_table_init(&c->sym.tag, RSH_TAG_HASH);
    if (err) {
        return err;
    }
    
    err = mlist_table_init(&c->sym.rootfs, RSH_ROOTFS_HASH);
    if (err) {
        return err;
    }
    
    err = mlist_table_init(&c->sym.web, RSH_WEB_HASH);
    if (err) {
        return err;
    }

    return 0;
}

static inline int
rsh_ep_init(struct rsh_control *c)
{
    c->ep.ev = (struct epoll_event *)os_zalloc(RSH_FD_COUNT*sizeof(struct epoll_event));
    if (NULL==c->ep.ev) {
        return -ENOEME;
    }

    return 0;
}

static inline int
rsh_tag_idx(struct rsh_control *c, char *name)
{
    int i;

    for (i=0; i<RSH_TAG_COUNT; i++) {
        if (0==os_strcmp(name, c->map.tag[i])) {
            return i;
        }
    }

    return -ENOEXIST;
}

static inline symbol_t *
rsh_sym_board(struct rsh_control *c, char *board)
{
    if (os_strlen(board) > RSH_BOARD_NAMELEN) {
        return NULL;
    } else {
        return symbol_push_string(&c->sym.board, board);
    }
}

static inline symbol_t *
rsh_sym_tag(struct rsh_control *c, char *tag)
{
    if (os_strlen(tag) > RSH_TAG_VALUELEN) {
        return NULL;
    } else {
        return symbol_push_string(&c->sym.tag, tag);
    }
}

static inline symbol_t *
rsh_sym_rootfs(struct rsh_control *c, char *version)
{
    if (os_strlen(version) > RSH_ROOTFS_NAMELEN) {
        return NULL;
    } else {
        return symbol_push_string(&c->sym.rootfs, version);
    }
}

static inline symbol_t *
rsh_sym_web(struct rsh_control *c, char *version)
{
    if (os_strlen(version) > RSH_WEB_NAMELEN) {
        return NULL;
    } else {
        return symbol_push_string(&c->sym.web, version);
    }
}

static inline int
rsh_info_set_tag(struct rsh_control *c, struct rsh_info *info, char *name, char *value)
{
    int idx = rsh_tag_idx(c, name);
    if (idx < 0) {
        return idx;
    }

    if (os_strlen(value) > RSH_TAG_VALUELEN) {
        return -ETOOBIG;
    }
    
    os_strcpy(info->tag[idx], value);
    
    return 0;
}

static inline int
rsh_info_set_rootfs(struct rsh_info *info, int idx, char *version)
{
    if (os_strlen(version) > RSH_ROOTFS_NAMELEN) {
        return -ETOOBIG;
    }
    
    os_strcpy(info->rootfs[idx], version);
    
    return 0;
}

static inline int
rsh_info_set_web(struct rsh_info *info, char *version)
{
    if (os_strlen(version) > RSH_ROOTFS_NAMELEN) {
        return -ETOOBIG;
    }
    
    os_strcpy(info->web, version);
    
    return 0;
}

static inline int
rsh_info_set_board(struct rsh_info *info, char *version)
{
    if (os_strlen(version) > RSH_BOARD_NAMELEN) {
        return -ETOOBIG;
    }
    
    os_strcpy(info->board, version);
    
    return 0;
}

static inline void
rsh_cache_init(struct rsh_control *c, struct rsh_cache *cache, struct rsh_info *info)
{
    int i, idx;

    for (i=0; i<RSH_TAG_COUNT; i++) {
        if (info->tag[i][0]) {
            cache->sym.tag[i] = rsh_sym_tag(c, info->tag[i]);
        }
    }
    
    for (i=0; i<RSH_ROOTFS_COUNT; i++) {
        if (info->rootfs[i][0]) {
            cache->sym.rootfs[i] = rsh_sym_rootfs(c, info->rootfs[i]);
        }
    }
    
    if (info->board[0]) {
        cache->sym.board = rsh_sym_board(c, info->board);
    }
    
    if (info->web[0]) {
        cache->sym.web = rsh_sym_web(c, info->web);
    }
}

static inline int 
rsh_hash(unsigned char mac[])
{
    int high = ((int)mac[4] + (int)mac[3]) & 0xff;
    int low  = ((int)mac[5] + (int)mac[3]) & 0xff;

    return (high << 8) + low;
}

static inline struct rsh_entry *
__rsh_entry(struct mlist_node *node)
{
    return container_of(node, struct rsh_entry, node.mlist);
}

static inline struct rsh_entry *
__rsh_entry_find(struct rsh_control *c, unsigned char mac[], struct symbol_t *board)
{
    int hash(void)
    {
        return rsh_hash(mac);
    }

    bool eq(struct mlist_node *node)
    {
        struct rsh_entry *entry = __rsh_entry(node);

        return board==entry->sym.board && os_maceq(mac, entry->base.mac);
    }

    struct mlist_node *node = mlist_find(&c->head.mlist, hash, eq);
    if (NULL==node) {
        return NULL;
    }

    return __rsh_entry(node);
}

static inline struct rsh_entry *
rsh_entry_find(struct rsh_control *c, unsigned char mac[], struct symbol_t *board)
{
    if (NULL==c) {
        return os_assertV(NULL);
    }
    else if (NULL==mac) {
        return os_assertV(NULL);
    }
    else if (NULL==board) {
        return os_assertV(NULL);
    }
    else {
        return __rsh_entry_find(c, mac, board);
    }
}

static inline bool
__rsh_entry_eq(struct rsh_entry *a, struct rsh_entry *b)
{
    return a->sym.board==b->sym.board
        && os_maceq(a->base.mac, b->base.mac);
}

static inline bool
rsh_entry_eq(struct rsh_entry *a, struct rsh_entry *b)
{
    return os_objeq(&a->base, &b->base)
        && os_objeq(&a->sym,  &b->sym);
}

static inline bool
__rsh_entry_eq_cache(struct rsh_entry *entry, struct rsh_cache *cache)
{
    return entry->sym.board==cache->sym.board 
        && os_maceq(entry->base.mac, cache->base.mac);
}

static inline bool
rsh_entry_eq_cache(struct rsh_entry *entry, struct rsh_cache *cache)
{
    return os_objeq(&entry->base, &cache->base)
        && os_objeq(&entry->sym,  &cache->sym);
}

static inline struct rsh_entry *
__rsh_entry_update(struct rsh_control *c, struct rsh_entry *entry, struct rsh_cache *cache)
{
    /*
    * TODO:
    */
    
    return entry;
}

static inline struct rsh_entry *
__rsh_entry_create(struct rsh_cache *cache)
{
    struct rsh_entry *entry = (struct rsh_entry *)os_zalloc(sizeof(*entry));
    if (NULL==entry) {
        return NULL;
    }
    os_objdcpy(&entry->base, &cache->base);
    os_objdcpy(&entry->sym,  &cache->sym);

    return entry;
}

static inline struct rsh_entry *
__rsh_entry_new(struct rsh_control *c, struct rsh_cache *cache)
{
    struct rsh_entry *entry = rsh_entry_find(c, cache->base.mac, cache->sym.board);
    if (NULL==entry) {
        return __rsh_entry_create(cache);
    }
    else if (__rsh_entry_eq_cache(entry, cache)) {
        return entry;
    }
    else {
        return __rsh_entry_update(c, entry, cache);
    }
}

static inline struct rsh_entry *
rsh_entry_new(struct rsh_control *c, struct rsh_cache *cache)
{
    if (NULL==c) {
        return os_assertV(NULL);
    }
    else if (NULL==cache) {
        return os_assertV(NULL);
    }
    else {
        return __rsh_entry_new(cache);
    }
}

static inline int
__rsh_entry_insert(struct rsh_control *c, struct rsh_cache *cache)
{
    int data_hash(void)
    {
        return rsh_hash(cache->base.mac);
    }
    
    bool eq(struct mlist_node *node)
    {
        struct rsh_entry *entry = __rsh_entry(node);
        
        return __rsh_entry_eq_cache(entry, cache);
    }
    
    int node_hash(struct mlist_node *node)
    {
        struct rsh_entry *entry = __rsh_entry(node);
        
        return rsh_hash(entry->base.mac);
    }
    
    struct mlist_node *new(void)
    {
        struct rsh_entry *entry = __rsh_entry_new(c, cache);

        return entry?&entry->node.mlist:NULL;
    }

    struct mlist_node *node = mlist_insert(&c->head.mlist, dat_hash, node_hash, eq, new);

    return node?__rsh_entry(node):NULL;
}

static inline int
rsh_entry_insert(struct rsh_control *c, struct rsh_cache *cache)
{
    if (NULL==c) {
        return os_assertV(-EINVAL1);
    }
    else if (NULL==cache) {
        return os_assertV(-EINVAL2);
    }
    else {
        return __rsh_entry_insert(c, cache);
    }
}

static inline int
rsh_entry_remove(struct rsh_control *c, struct rsh_entry *entry)
{
    if (NULL==c) {
        return os_assertV(-EINVAL1);
    }
    else if (NULL==entry) {
        return os_assertV(-EINVAL2);
    }
    else {
        return mlist_remove(&c->head.mlist, &entry->node.mlist);
    }
}

static inline int
__rsh_entry_foreach(
    struct rsh_control *c, 
    multi_value_t (*cb)(struct rsh_entry *entry)
)
{
    multi_value_t node_cb(struct mlist_node *node)
    {
        return (*cb)(__rsh_entry(node));
    }

    return mlist_foreach(&c->head.mlist, node_cb);
}

static inline int
rsh_entry_foreach(
    struct rsh_control *c, 
    multi_value_t (*cb)(struct rsh_entry *entry)
)
{
    if (NULL==c) {
        return os_assertV(-EINVAL1);
    }
    else if (NULL==cb) {
        return os_assertV(-EINVAL2);
    }
    else {
        return __rsh_entry_foreach(c, cb);
    }
}

/******************************************************************************/
#endif /* __RSH_H_227352D437EF91A690DEE61AD59E4A66__ */
