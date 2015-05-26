/*******************************************************************************
Copyright (c) 2012-2015, Autelan Networks. All rights reserved.
*******************************************************************************/
#ifndef __THIS_APP
#define __THIS_APP      partool
#endif
        
#ifndef __THIS_NAME
#define __THIS_NAME     "partool"
#endif

#ifdef __BOOT__
#include <common.h>
#include <command.h>
#endif
#include "utils.h"

OS_INITER;

#ifndef PART_BLOCK_CACHE_SIZE
#define PART_BLOCK_CACHE_SIZE   (64*1024)
#endif

/*
* 如果新产品的定义与本文件默认定义不一致
* 则应该在makefile中重定义以下信息
*/
#ifndef NAME_OSENV
#ifdef __BOOT__
#define NAME_OSENV      "osenv"
#else
#define NAME_OSENV      "mtd2"
#endif
#endif

#ifndef NAME_PRODUCT
#ifdef __BOOT__
#define NAME_PRODUCT    "product"
#else
#define NAME_PRODUCT    "mtd8"
#endif
#endif

#ifndef ADDR_OSENV
#define ADDR_OSENV      0x9f050000
#endif

#ifndef ADDR_PRODUCT
#define ADDR_PRODUCT    0x9ffe0000
#endif

#ifndef SIZE_OSENV
#define SIZE_OSENV      PART_BLOCK_CACHE_SIZE
#elif SIZE_OSENV > PART_BLOCK_CACHE_SIZE
#error "SIZE_OSENV > PART_BLOCK_CACHE_SIZE"
#endif

#ifndef SIZE_PRODUCT
#define SIZE_PRODUCT    PART_BLOCK_CACHE_SIZE
#elif SIZE_PRODUCT > PART_BLOCK_CACHE_SIZE
#error "SIZE_PRODUCT > PART_BLOCK_CACHE_SIZE"
#endif

#define ADDR_BAD        0xffffffff
#define NAME_GOOD       NAME_OSENV "|" NAME_PRODUCT
#define NAME_HELP       "partname must be " NAME_GOOD

enum {
    /*
    * normal read(flash==>block)
    */
    PART_MODE_NORMAL,
    
    /*
    * erase flash
    */
    PART_MODE_EMPTY,
    
    /*
    * check crc
    */
    PART_MODE_CRC,
    
#ifndef __BOOT__
    /*
    * load file to block
    * and write to flash
    */
    PART_MODE_LOAD,
#endif

    PART_MODE_END
};

static inline bool
is_good_part_mode(int mode)
{
    return is_good_enum(mode, PART_MODE_END);
}

static inline char *
part_mode_string(int mode)
{
    static char *string[PART_MODE_END] = {
        "normal",
        "empty",
        "crc",
#ifndef __BOOT__
        "load",
#endif
    };

    return is_good_part_mode(mode)?string[mode]:__unknow;
}

#ifndef PART_XOR
#define PART_XOR            0x94f00d1e
#endif

#ifndef PART_MAX_LINE 
#define PART_MAX_LINE       OS_LINE_LEN
#endif

#ifndef PART_COMMENT
#define PART_COMMENT        '#'
#endif

#ifndef PART_SEPERATOR
#define PART_SEPERATOR      '\t'
#endif

#ifndef PART_CRLF
#define PART_CRLF           '\n'
#endif

#ifndef part_seperators
#define part_seperators     "\t "
#endif

#ifndef PART_BLOCK_DUMP_LEN
#define PART_BLOCK_DUMP_LEN 256
#endif

#define PART_VAR_CREATE     (PART_VAR_CREATE_IF_NOT_EXIT | PART_VAR_UPDATE_IF_EXIT)
#define PART_CACHED         0x10
#define PART_DIRTY          0x20

struct part_item {
    struct list_head node;
    
    struct string_kv c;
};

struct part_block {
    /*
    * block cache, a memory buffer
    *   size==flash partition size
    */
    unsigned int size;
    char *cache;

    /*
    * store k/v
    */
    unsigned int count;
    struct list_head list;

    /*
    * flash partition name
    *   as /dev/XXX
    */
    char *partition;

    /*
    * flash==>cache
    */
    int (*read)(struct part_block *block, char *partition);

    /*
    * cache==>flash
    */
    int (*write)(struct part_block *block, char *partition);
    
    unsigned int flag;
    unsigned int xor;
    int mode;
    int lockfd;
};

#define PART_BLOCK_SIZE(_size)  (sizeof(struct part_block) + (_size))

#ifdef __BOOT__
unsigned int __AKID_DEBUG = 0;

static unsigned char boot_block[PART_BLOCK_SIZE(PART_BLOCK_CACHE_SIZE)];

static inline struct part_block *
part_zalloc(int block_size)
{
    if (block_size > PART_BLOCK_CACHE_SIZE) {
        return os_assertV(NULL);
    } else {
        os_objzero(&boot_block); /* os_objzero(&array) */
        return (struct part_block *)boot_block;
    }
}
#define part_free(_blk)         os_do_nothing()

#define part_lock(_block)       0
#define part_unlock(_block)     os_do_nothing()

#else /* !__BOOT__ */
#define part_zalloc(_size)      (struct part_block *)os_zalloc(PART_BLOCK_SIZE(_size))
#define part_free(_blk)         os_free(_blk)

#ifndef PART_LOCKFILE
#define PART_LOCKFILE       "/var/run/part.lock"
#endif

static inline int 
part_lock(struct part_block *block)
{
    int fd = INVALID_FD;
    int err = 0;
    
    fd = open(PART_LOCKFILE, O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR);
	if (fd < 0) {
		err = -errno;

		goto error;
	}

	if (flock(fd, LOCK_EX)) {
		err = -errno;

		goto error;
	}
    
    block->lockfd = fd;
    
	return 0;
error:
    os_close(fd);

    return err;
}

static inline void 
part_unlock(struct part_block *block)
{
    if (block->lockfd >= 0) {
    	if (flock(block->lockfd, LOCK_UN)) {
    		/* log */
        }
        
    	os_close(block->lockfd);
    	unlink(PART_LOCKFILE);
        
    	block->lockfd = -1;
	}
}
#endif /* __BOOT__ */

bool
part_block_is_good(struct part_block *block)
{
    if (NULL==block) {
        return false;
    }
    else if (NULL==block->cache) {
        return false;
    }
    else if (NULL==block->read) {
        return false;
    }
    else if (NULL==block->write) {
        return false;
    }
    else if (NULL==block->partition) {
        return false;
    }
    else if (0==block->size) {
        return false;
    }
    else if (false==is_good_part_mode(block->mode)) {
        return false;
    }
    else {
        return true;
    }
}

static inline bool 
is_cached(struct part_block *block)
{
    return os_hasflag(block->flag, PART_CACHED);
}

static inline bool 
is_dirty(struct part_block *block)
{
    return os_hasflag(block->flag, PART_DIRTY);
}

/*
* skip crc
*/
static int
part_block_size(struct part_block *block)
{
    return block->size - (1+CRC32_STRINGLEN) ;
}

/*
* skip crc
*/
static char *
part_block_begin(struct part_block *block)
{
    return block->cache + (1+CRC32_STRINGLEN);
}

static inline char *
part_block_end(struct part_block *block)
{
    return block->cache + block->size;
}

static inline unsigned int
part_block_crc(struct part_block *block)
{
    void *begin = part_block_begin(block);
    unsigned int size= part_block_size(block);
    unsigned int crc = os_crc32(begin, size);
    
    debug_trace("calc crc:%x with begin:%p size:%d)", crc, begin, size);
    
    return crc;
}

static unsigned int
part_block_crc_read(struct part_block *block)
{
    char crcstring[1+CRC32_STRINGLEN] = {0};
    unsigned long long crc = 0;
    os_memcpy(crcstring, block->cache, CRC32_STRINGLEN);

    /*
    * uboot not support sscanf, so can not as below
    *   os_sscanf(crcstring, "%x", &crc);
    */
    crc = os_digitstring2number(crcstring, os_strlen(crcstring), 16, typeof(crc));

    debug_trace("read crc(string:%s, number:%llx)", crcstring, crc);
    
    return (unsigned int)crc;
}

static void
part_block_crc_write(struct part_block *block)
{
    char crcstring[1+CRC32_STRINGLEN] = {0};
    unsigned int crc = part_block_crc(block);

    os_saprintf(crcstring, "%x", crc);
    os_memcpy(block->cache, crcstring, CRC32_STRINGLEN);
    block->cache[CRC32_STRINGLEN] = '\n';
}

static inline void
part_block_dump(struct part_block *block, char *header)
{
    if (__is_debug_level_packet) {
        os_println("block cache(%s)", header);
        __os_dump_buffer(block->cache, PART_BLOCK_DUMP_LEN, NULL);
        os_println(__crlf);
    }
}

static char *
part_get_value_from_line(char *line)
{
    char *p;
    char *end = line + os_strlen(line);

    /*
    * find seperator
    */
    for (p=line; p<end; p++) {
        if (strchr(part_seperators, *p)) {
            break;
        }
    }

    if (p==end) {
        return NULL;
    } else {
        *p = 0;

        return (p+1);
    }
}

/*
*   [begin, end) 半开区间
*/
static char *
part_read_line(char *begin, char *end, char *line, int line_size)
{
    char *p, *newline;
    int len;
    
    /* first reset line */
    line[0] = 0;
    
    if (NULL==begin || NULL==end || begin >= end) {
        return NULL;
    }
    
    /*
    * find '\n'
    */
    for (p=begin; p<end; p++) {
        if (PART_CRLF == *p) {
            break;
        }
    }

    /*
    * found '\n', p-->'\n'
    */
    if (PART_CRLF == *p) {
        newline = p + 1;

        if (newline >= end) {
            newline = NULL; /* p is the block last */
        }

        len = (p - begin + 1) - 1/* drop '/n' */;
    } else {
        /*
        * not found '\n', p-->end
        */
        newline = NULL;
        len = end - begin + 1;
    }
    
    if (len >= line_size) {
        return NULL;
    }

    os_strmcpy(line, begin, len);

    return newline;
}

static char *
part_write_line(char *begin, char *end, struct string_kv *c)
{
    int len, left;
    char *name, *separator, *value, *crlf;
    
    if (NULL==begin || NULL==end || NULL==c) {
        return NULL;
    }

    len = c->k.len + 1/* '\t' */ + c->v.len + 1/* '\n' */;
    left = end - begin;
    if (left < (len + 1/* '\0' */)) {
        return NULL;
    }

    name = begin;
    separator = name + c->k.len;
    value = separator + 1;
    crlf = value + c->v.len;
    
    os_memcpy(name, c->k.var, c->k.len);
    *separator = PART_SEPERATOR;
    os_memcpy(value, c->v.var, c->v.len);
    *crlf = PART_CRLF;
    
    return crlf + 1;
}

static void
part_cursor_free(struct part_item *item)
{
    struct string_kv *c = &item->c;
    
    if (c->k.var) {
        os_free(c->k.var);
    }
    if (c->v.var) {
        os_free(c->v.var);
    }
}

static int
part_cursor_init(struct part_item *item, char *name, char *value)
{
    struct string_kv *c = &item->c;
    
    part_cursor_free(item);
    
    c->k.len = os_strlen(name);
    c->k.var = (char *)os_zalloc(1+c->k.len);
    if (NULL==c->k.var) {
        goto error;
    }
    os_memcpy(c->k.var, name, c->k.len);
    
    c->v.len = os_strlen(value);
    c->v.var = (char *)os_zalloc(1+c->v.len);
    if (NULL==c->v.var) {
        goto error;
    }
    os_memcpy(c->v.var, value, c->v.len);

    return 0;
error:
    part_cursor_free(item);

    return -ENOMEM;
}

static void
part_item_free(struct part_item *item)
{
    if (item) {
        part_cursor_free(item);

        os_free(item);
    }
}

static int
part_item_init(struct part_item *item, char *name, char *value)
{
    INIT_LIST_HEAD(&item->node);
    
    return part_cursor_init(item, name, value);
}

static struct part_item *
part_item_new(char *name, char *value)
{
    struct part_item *item = NULL;

    item = (struct part_item *)os_zalloc(sizeof(*item));
    if (NULL==item) {
        goto error;
    }

    if (0!=part_item_init(item, name, value)) {
        goto error;
    }

    return item;
error:
    part_item_free(item);

    return NULL;
}

static int
part_item_insert(struct part_block *block, struct part_item *item)
{
    if (item) {
        list_add_tail(&item->node, &block->list);
        block->count++;

        return 0;
    } else {
        return -ENOMEM;
    }
}

static void
part_item_remove(struct part_block *block, struct part_item *item)
{
    if (block && item) {
        list_del(&item->node);
        block->count--;
    }
}

static inline int
kv_item_destroy(struct part_block *block, struct part_item *item)
{
    if (item) {
        part_item_remove(block, item);
        part_item_free(item);

        block->flag |= PART_DIRTY;
        
        return 0;
    } else {
        return -ENOEXIST;
    }
}

static inline void
part_item_clean(struct part_block *block)
{
    struct part_item *item, *n;

    list_for_each_entry_safe(item, n, &block->list, node) {
        kv_item_destroy(block, item);
    }
}

static inline struct part_item *
part_item_getbycursor(struct string_kv *c)
{
    return container_of(c, struct part_item, c);
}

/* 
* partition ==> cache 
*/
static inline void
part_xor(struct part_block *block)
{
    if (block->xor) {
        int *p;
        int *end = (int *)part_block_end(block);
        
        for (p=(int *)block->cache; p<end; p++) {
            *p ^= block->xor;
        }

        debug_trace("part xor OK.");

        part_block_dump(block, "after xor");
    }
}

#ifndef __BOOT__
/* 
* partition ==> cache 
*/
static inline int
part_human_read(struct part_block *block, char *human)
{
    FILE *f = NULL;
    int err = 0, size;
    struct stat st;

    stat(human, &st);
    size = st.st_size;
    if (size > part_block_size(block)) {
        size = part_block_size(block);
    }
    
    f = fopen(human, "r");
    if (NULL==f) {
        err = -errno;

        debug_error("part load read error(%d)", err);
        
        goto error;
    }

    err = fread(part_block_begin(block), size, 1, f);
    if (1!=err) {
        err = -errno;

        debug_error("part load read error(%d)", err);
        
        goto error;
    }
    
    err = 0;
    debug_trace("part load read OK.");
error:
    if (f) {
        fclose(f);
    }

    return err;
}
#endif

/* 
* partition ==> cache 
*/
static inline int
part_low_read(struct part_block *block)
{
    int err = 0;
    
    err = (*block->read)(block, block->partition);
    if (err) {
        debug_error("part low read error(%d)", err);
    } else {
        debug_trace("part low read OK.");
    }
    
    part_xor(block);
    
    return err;
}

/* 
* cache ==> partition 
*/
static inline int
part_low_wirte(struct part_block *block)
{
    int err = 0;

    err = (*block->write)(block, block->partition);
    if (err) {
        debug_error("part low write error(%d)", err);
    } else {
        debug_trace("part low write OK.");
    }

    return err;
}

static bool 
is_separator(int ch)
{
    return NULL!=strchr(part_seperators, ch);
}

/* 
* cache ==> list
*/
static int
part_line_read(struct part_block *block)
{
    char line[1+PART_MAX_LINE];
    char *begin, *end, *value;
    int err;
    
    begin = part_block_begin(block);
    end   = part_block_end(block);

    while(NULL!=(begin = part_read_line(begin, end, line, PART_MAX_LINE)) || line[0]) {
        /*
        * cut left and right blanks("\t \r\n")
        */
        __string_strim_both(line, NULL);
        
        if (PART_COMMENT==line[0]) {
            continue; /* comment line */
        }
        else if (0==line[0]) {
            continue; /* blank line */
        }
        
        /* reduce k/v separator */
        __string_reduce(line, is_separator);

        value = part_get_value_from_line(line);
        if (NULL==value) {
            /*
            * no found seperators, bad line
            *
            * ignore this line and try next line
            */
            continue;
        }

        debug_trace("insert k(%s)/v(%s)", line, value);
        err = part_item_insert(block, part_item_new(line, value));
        if (err) {
            /* 
            * insert error
            *
            * ignore this line and try next line
            */
            continue;
        }
    }
    
    block->flag |= PART_CACHED;
    block->flag &= ~PART_DIRTY;
    
    return 0;
}

/* 
* list ==> cache
*/
static int
part_line_wirte(struct part_block *block)
{
    struct part_item *item;
    char *begin, *end;

    os_memzero(block->cache, block->size);

    begin = part_block_begin(block);
    end   = part_block_end(block);
    list_for_each_entry(item, &block->list, node) {
        begin = part_write_line(begin, end, &item->c);
        if (NULL==begin) {
            /* log */
        }
    }
    part_block_crc_write(block);
    
    block->flag &= ~PART_DIRTY;
    
    return 0;
}

/* 
* cache ==> list
*/
static int
part_high_read(struct part_block *block)
{
    unsigned int crc_old, crc_new;
    int err;
    
    if (block->count) {
        return -ENOEMPTY;
    }
    
    /*
    * check crc
    */
    crc_old = part_block_crc_read(block);
    crc_new = part_block_crc(block);
    if (crc_old!=crc_new) {
        debug_error("bad crc(old=0x%x, new=0x%x)", crc_old, crc_new);
        
        return -EBADCRC;
    }
    
    err = part_line_read(block);
    if (err) {
        debug_error("part high read error(%d)", err);
    } else {
        debug_trace("part high read OK.");
    }
    
    part_block_dump(block, "after high read");

    return err;
}

/* 
* list ==> cache
*/
static int
part_high_wirte(struct part_block *block)
{
    int err = 0;

    /*
    * cannot check block->count
    *   when do empty, the block->count is 0
    */
    err = part_line_wirte(block);
    if (err) {
        debug_error("part high write error(%d)", err);
    } else {
        debug_trace("part high write OK.");
    }
    
    part_block_dump(block, "after high write");
    
    return err;
}

static int
part_block_init_bynormal(struct part_block *block)
{
    int err;
    
    err = part_low_read(block);
    if (err) {
        return err;
    }
    
    err = part_high_read(block);
    if (err) {
        return err;
    }
    
    return 0;
}

static int
part_block_init_byempty(struct part_block *block)
{
    block->flag |= PART_DIRTY;
    
    debug_trace("part create empty OK");
    
    return 0;
}

#ifndef __BOOT__
static int
part_block_init_byload(struct part_block *block, char *human)
{
    int err;
    
    err = part_human_read(block, human);
    if (err) {
        return err;
    }

    block->flag |= PART_DIRTY;
    
    return 0;
}
#endif

static int
part_block_init_bycrc(struct part_block *block)
{
    int err;
    
    err = part_low_read(block);
    if (err) {
        return err;
    }
    
    return 0;
}

static int
part_block_init(struct part_block *block, char *human)
{
    int err = 0;
    
    switch(block->mode) {
        case PART_MODE_NORMAL:
            err = part_block_init_bynormal(block);
            break;
        case PART_MODE_EMPTY:
            err = part_block_init_byempty(block);
            break;
#ifndef __BOOT__
        case PART_MODE_LOAD:
            err = part_block_init_byload(block, human);
            break;
#endif
        case PART_MODE_CRC:
            err = part_block_init_bycrc(block);
            break;
        default:
            err = -EKEYBAD;
            break;
    }
    
    if (err) {
        debug_error("%s init failed(%d)", part_mode_string(block->mode), err);
    }
    
    return err;
}

static int
part_block_flush(struct part_block *block)
{
    int err;
    
    if (is_dirty(block)) {
        err = part_high_wirte(block);
        if (err) {
            return err;
        }
        
        part_xor(block);
        
        err = part_low_wirte(block);
        if (err) {
            return err;
        }
    }
    
    return 0;
}

static void
part_block_free_only(struct part_block *block)
{
    if (block) {
        part_item_clean(block);
        part_unlock(block);
        part_free(block);
    }
}

/*
* clean block
*   flush list to cache
*   flush cache to partition
*   free block memory
*/
void
__part_block_free(struct part_block *block)
{
    if (block) {
        part_block_flush(block);
        part_block_free_only(block);
    }
}

#define part_block_free(_block) do{ \
    __part_block_free(_block);      \
    (_block) = NULL;                \
}while(0)

static struct part_block * 
__part_block_new(
    int mode,
    unsigned int block_size, 
    int (*read)(struct part_block *block, char *partition), 
    int (*write)(struct part_block *block, char *partition),
    char *partition,
    unsigned int xor
)
{
    struct part_block *block = NULL;
    int err;
    
    block = part_zalloc(block_size);
    if (NULL==block) {
        return os_assertV(NULL);
    }
    block->cache= (char *)(block + 1);
    block->size = block_size;
    block->partition = partition;
    block->xor  = xor?xor:PART_XOR;
    block->mode = mode;
    block->read = read;
    block->write= write;
    INIT_LIST_HEAD(&block->list);
    
    debug_trace("new block with"    __crlf
                __tab "size:%d"     __crlf
                __tab "xor:0x%x"    __crlf
                __tab "mode:%s"     __crlf
                __tab "partition:%s", 
                block->size, 
                block->xor,
                part_mode_string(block->mode),
                os_safestring(partition));

    err = part_lock(block);
    if (err) {
        goto error;
    }
    
    return block;
error:
    part_free(block);

    return NULL;
}

/*
* create block
*   alloc cache memory
*   read from partition to cache
*   resolve cache to list
*
*   @block_size: same to partition size
*   @read: in this callback, you should do below
*       (1) partition==>tmp buf/file(you do it)
*       (2) tmp buf/file==>block cache(call part_block_write)
*       in os, use tmp file
*       in boot, use tmp buf
*   @write: in this callback, you should do below
*       (1) block cache==>tmp buf/file(call part_block_read)
*       (2) tmp buf/file==>partition(you do it)
*   @partition: partition name
*/
struct part_block * 
part_block_create(
    int mode,
    unsigned int block_size, 
    int (*read)(struct part_block *block, char *partition), 
    int (*write)(struct part_block *block, char *partition),
    char *partition,
    char *human,
    unsigned int xor
)
{
    struct part_block *block = NULL;
    int err;
    
    if (false==is_good_part_mode(mode)) {
        return os_assertV(NULL);
    }
    else if (0==block_size) {
        return os_assertV(NULL);
    }
    else if (NULL==read) {
        return os_assertV(NULL);
    }
    else if (NULL==write) {
        return os_assertV(NULL);
    }
    else if (NULL==partition) {
        return os_assertV(NULL);
    }
    
    block = __part_block_new(mode, block_size, read, write, partition, xor);
    if (NULL==block) {
        return NULL;
    }

    err = part_block_init(block, human);
    if (err) {
        goto error;
    }

    return block;
error:
    part_block_free_only(block);
    
    return NULL;
}


static int
part_var_rw_check(struct part_block *block, int offset, void *buf, unsigned int size)
{
    if (false==part_block_is_good(block)) {
        return os_assertV(-EKEYBAD);
    }
    else if (offset >= block->size) {
        return os_assertV(-EINVAL1);
    }
    else if (NULL==buf) {
        return os_assertV(-EINVAL2);
    }
    else if (size < 0) {
        return os_assertV(-EINVAL3);
    }
    else if ((offset + size) > block->size) {
        return os_assertV(-EINVAL4);
    }

    return 0;
}

/*
* copy data from block cache to buf
*/
int
part_block_read(struct part_block *block, unsigned int offset, void *buf, unsigned int size)
{
    int err;

    err = part_var_rw_check(block, offset, buf, size);
    if (err) {
        return err;
    }
    
    os_memcpy(buf, block->cache + offset, size);

    return size;
}

/*
* copy data from buf to block cache
*/
int
part_block_write(struct part_block *block, unsigned int offset, void *buf, unsigned int size)
{
    int err;

    err = part_var_rw_check(block, offset, buf, size);
    if (err) {
        return err;
    }

    os_memcpy(block->cache + offset, buf, size);
    
    return size;
}

int
part_block_crc_get(struct part_block *block, unsigned int *crc_part, unsigned int *crc_calc)
{
    if (false==part_block_is_good(block)) {
        return os_assertV(-EKEYBAD);
    }
    else if (NULL==crc_part) {
        return os_assertV(-EINVAL1);
    }
    else if (NULL==crc_calc) {
        return os_assertV(-EINVAL2);
    }

    *crc_part = part_block_crc_read(block);
    *crc_calc = part_block_crc(block);
    
    return 0;
}

/*
* @c : kv cursor, readonly!!!
*/
typedef multi_value_t part_var_foreach_f(struct part_block *block, struct string_kv *c, void *data);

int
part_var_foreach(struct part_block *block, part_var_foreach_f *foreach, void *data)
{
    struct part_item *item, *n;
    multi_value_u mv = MV_INITER;
    
    if (false==part_block_is_good(block)) {
        return os_assertV(-EKEYBAD);
    }
    else if (NULL==foreach) {
        return os_assertV(-EINVAL1);
    }

    list_for_each_entry_safe(item, n, &block->list, node) {
        mv.value = (*foreach)(block, &item->c, data);
        if (mv2_is_break(mv)) {
            return mv2_result(mv);
        }
    }

    return mv2_result(mv);
}

#ifndef __BOOT__
static multi_value_t 
kv_foreach_byname_prefix_cb(struct part_block *block, struct string_kv *c, void *data)
{
    void **param = (void **)data;
    char *DATA = param[0];
    char *name  = param[1];
    part_var_foreach_f *foreach = param[2];
    int len = os_strlen(name);
    
    if (os_strlen(c->k.var) >= len && os_memeq(c->k.var, name, len)) {
        return (*foreach)(block, c, DATA);
    } else {
        return mv2_OK;
    }
}

/*
* foreach k/v, prefix match k's name by @name
*/
int
part_var_foreach_byname_prefix(struct part_block *block, char *name, part_var_foreach_f *foreach, void *data)
{
    void *param[4] = {
        [0] = data,
        [1] = name,
        [2] = foreach,
    };
    
    if (NULL==name) {
        return os_assertV(-EINVAL9);
    }

    return part_var_foreach(block, kv_foreach_byname_prefix_cb, param);
}
#endif

static struct part_item *
__part_var_find(struct part_block *block, char *name)
{
    struct part_item *item;

    list_for_each_entry(item, &block->list, node) {
        if (0==os_strcmp(name, item->c.k.var)) {
            return item;
        }
    }
    
    return NULL;
}
/*
* find k/v from block list
*/
int
part_var_find(struct part_block *block, char *name, struct string_kv *c)
{
    struct part_item *item;
    
    if (false==part_block_is_good(block)) {
        return os_assertV(-EKEYBAD);
    }
    else if (NULL==name) {
        return os_assertV(-EINVAL1);
    }
    else if (NULL==c) {
        return os_assertV(-EINVAL2);
    }

    item = __part_var_find(block, name);
    if (NULL==item) {
        return -ENOEXIST;
    }

    os_objdcpy(c, &item->c);
    
    return 0;
}

enum {
    PART_VAR_CREATE_IF_NOT_EXIT = 0x01,
    PART_VAR_UPDATE_IF_EXIT     = 0x02,
};

int
__part_var_create(struct part_block *block, char *name, char *value, int flag)
{
    struct part_item *item = NULL;
    
    if (false==part_block_is_good(block)) {
        return os_assertV(-EKEYBAD);
    }
    else if (NULL==name) {
        return os_assertV(-EINVAL1);
    }
    else if (NULL==value) {
        return os_assertV(-EINVAL2);
    }
    else if (0==(PART_VAR_CREATE & flag)) {
        return os_assertV(-EINVAL3);
    }

    item = __part_var_find(block, name);
    if (item) {
        if (os_hasflag(flag, PART_VAR_UPDATE_IF_EXIT)) {
            /* update it */
            block->flag |= PART_DIRTY;

            return part_cursor_init(item, name, value);
        }
        else if (os_hasflag(flag, PART_VAR_CREATE_IF_NOT_EXIT)) {
            return -EEXIST;
        }
    } else {
        if (os_hasflag(flag, PART_VAR_CREATE_IF_NOT_EXIT)) {
            /* create new */
            block->flag |= PART_DIRTY;

            return part_item_insert(block, part_item_new(name, value));
        }
        else if (os_hasflag(flag, PART_VAR_UPDATE_IF_EXIT)) {
            return -ENOEXIST;
        }
    }

    return os_assertV(-EINVAL5);
}

/*
* create k/v in block list
*   if not exist, create new
*   if exist, return -EEXIST
*/
static inline int
part_var_create(struct part_block *block, char *name, char *value)
{
    return __part_var_create(block, name, value, PART_VAR_CREATE_IF_NOT_EXIT);
}

/*
* update k/v in block list
*   if not exist, return NULL
*   if exist, update it
*/
static inline int
part_var_update(struct part_block *block, char *name, char *value)
{
    return __part_var_create(block, name, value, PART_VAR_UPDATE_IF_EXIT);
}

/*
* create k/v in block list
*   if not exist, create new
*   if exist, update it
*/
static inline int
part_var_new(struct part_block *block, char *name, char *value)
{
    return __part_var_create(block, name, value, PART_VAR_CREATE_IF_NOT_EXIT | PART_VAR_UPDATE_IF_EXIT);
}

/*
* delete k/v in block list
*/
int
part_var_delete(struct part_block *block, char *name)
{
    if (false==part_block_is_good(block)) {
        return os_assertV(-EKEYBAD);
    }
    else if (NULL==name) {
        return os_assertV(-EINVAL1);
    }

    return kv_item_destroy(block, __part_var_find(block, name));
}

#ifndef __BOOT__
static multi_value_t 
part_var_delete_byname_prefix_cb(struct part_block *block, struct string_kv *c, void *data)
{
    int err;
    
    err = kv_item_destroy(block, part_item_getbycursor(c));

    return mv2_GO(err);
}

int
part_var_delete_byname_prefix(struct part_block *block, char *name)
{
    return part_var_foreach_byname_prefix(block, name, part_var_delete_byname_prefix_cb, NULL);
}
#endif

static int  partool_size;
static struct part_block *blk;

static struct {
    char *partition;
    unsigned long addr;
    int size;
} part_info[] = {
    {
        .partition  = NAME_OSENV,
        .addr       = ADDR_OSENV,
        .size       = SIZE_OSENV,
    },
    {
        .partition  = NAME_PRODUCT,
        .addr       = ADDR_PRODUCT,
        .size       = SIZE_PRODUCT,
    }
};

static int
part_error(int err, char *name)
{
    switch(err) {
        case -ENOEXIST:
            os_println("%s not exist", os_safestring(name));
            
            break;
        case -EEXIST:
            os_println("%s exist", os_safestring(name));
            
            break;
        default:
            os_println("error(%d)", err);
            
            break;
    }

    return err;
}

static inline unsigned long 
part_begin(char *partition)
{
    int i;

    for (i=0; i<os_count_of(part_info); i++) {
        if (0==os_strcmp(partition, part_info[i].partition)) {
            partool_size = part_info[i].size;
            
            return part_info[i].addr;
        }
    }
    
    return ADDR_BAD;
}

#ifdef __BOOT__

static unsigned char part_tmp[PART_BLOCK_CACHE_SIZE];

static inline void
__partool_clean(void)
{
    os_do_nothing();
}

static inline int
__partool_init(char *partition)
{
    if (ADDR_BAD==part_begin(partition)) {
        os_println(NAME_HELP);
        
        return -EINVAL9;
    } else {
        return 0;
    }
}

static inline int
__partool_read(char *partition)
{
    unsigned long begin = part_begin(partition);
    unsigned char *tmp;
    int i;

    if (ADDR_BAD==begin) {
        return -EINVAL7;
    }
    
    for(i=0, tmp=(unsigned char *)begin; i<partool_size; i++, tmp++) {
	    part_tmp[i] = *tmp;
	}

	return 0;
}

static inline int
__partool_write(char *partition)
{
    unsigned long begin = part_begin(partition);
    unsigned long end   = begin + partool_size - 1;
    unsigned char *tmp;
    int err;
    
    if (ADDR_BAD==begin) {
        return -EINVAL8;
    }
    
    err = flash_sect_erase(begin, end);
    if (err) {
        debug_error("flash erase(begin:0x%x end:0x%x size:0x%x) failed(%d)", 
            begin, end, partool_size, err);
        return err;
    }

    err = flash_write(part_tmp, begin, partool_size);
    if (err) {
        debug_error("flash write(begin:0x%x end:0x%x size:0x%x) failed(%d)", 
            begin, end, partool_size, err);
        return err;
    }

	return 0;
}

#else

#ifndef PART_BASE_PATH
#ifdef  __TEST__
#define PART_BASE_PATH "."
#else
#define PART_BASE_PATH "/dev"
#endif
#endif

static char partool_tmpfile[1+OS_FILENAME_LEN];
static char partool_mtdfile[1+OS_FILENAME_LEN];
static char *partool_humfile;
static unsigned char *part_tmp;

static inline void
init_tmp_filename(char *mtd)
{
    int r, pid, t;
    
    pid = getpid();
    t = time(NULL);
    srand(t);
    r = rand();

    os_saprintf(partool_tmpfile, "/tmp/%s-%u.%u.%u", mtd, pid, t, r);
}

static inline void
init_mtd_filename(char *mtd)
{
    os_saprintf(partool_mtdfile, PART_BASE_PATH "/%s", mtd);
}

static inline void
rm_fille(char *filename)
{
    /*
    * in debugging trace
    *   keep tmp file
    */
    if (false==__is_debug_level_trace && filename[0]) {
        os_system("rm -fr %s", filename);

        filename[0] = 0;
    }
}

static inline void
tmp_2_mtd(void)
{
#if defined(PART_RW_MTD)
#   define part_write_format    "mtd -q write %s %s"
#elif defined(PART_RW_DD)
#   define part_write_format    "dd if=%s of=%s"
#elif defined(PART_RW_CP)
#   define part_write_format    "cp -f %s %s"
#else
#   error "must define PART_RW_MTD|PART_RW_DD|PART_RW_CP in makefile"
#endif

    os_system(part_write_format, partool_tmpfile, partool_mtdfile);

    os_free(part_tmp);
}

static inline int
mtd_2_tmp(char *mtd)
{
    int size;
    
#if defined(PART_RW_MTD) || defined(PART_RW_CP)
#   define part_read_format     "cp -f %s %s"
#elif defined(PART_RW_DD)
#   define part_read_format     "dd if=%s of=%s"
#else
#   error "must define PART_RW_MTD|PART_RW_DD|PART_RW_CP in makefile"
#endif
    init_mtd_filename(mtd);
    init_tmp_filename(mtd);
    
    os_system(part_read_format, partool_mtdfile, partool_tmpfile);

    size = os_sfsize(partool_tmpfile);
    if (size != partool_size) {
        return -EINVAL;
    }

    part_tmp = (unsigned char *)os_zalloc(partool_size);
    if (NULL==part_tmp) {
        return -ENOMEM;
    }
    
    return 0;
}

static inline void
__partool_clean(void)
{
    rm_fille(partool_tmpfile);
}

static inline int
__partool_init(char *partition)
{
    if (ADDR_BAD==part_begin(partition)) {
        os_println(NAME_HELP);
        
        return -EFORMAT;
    } else {
        return mtd_2_tmp(partition);
    }
}

static inline int 
__partool_read(char *partition)
{
    FILE *f = NULL;
    int err;
    
    f = fopen(partition, "r");
    if (NULL==f) {
        err = -errno;

        goto error;
    }

    err = fread(part_tmp, partool_size, 1, f);
    if (1!=err) {
        err = -errno;

        goto error;
    }

    err = 0;
error:
    if (f) {
        fclose(f);
    }
    
    return err;
}

static inline int 
__partool_write(char *partition)
{
    FILE *f = NULL;
    int err;
    
    f = fopen(partition, "r+");
    if (NULL==f) {
        err = -errno;

        goto error;
    }
    
    err = fwrite(part_tmp, partool_size, 1, f);
    if (1!=err) {
        err = -errno;

        goto error;
    }
    
    err = 0;
error:
    if (f) {
        fclose(f);
    }
    if (0==err) {
        tmp_2_mtd();
    }
    
    return err;
}

#endif


static int 
partool_read(struct part_block *block, char *partition)
{
    int err;

    err = __partool_read(partition);
    if (err) {
        goto error;
    }

    err = part_block_write(block, 0, part_tmp, partool_size);
    if (err) {
        goto error;
    }

    err = 0;
error:
    return err;
}

static int 
partool_write(struct part_block *block, char *partition)
{
    int err;
    
    err = part_block_read(block, 0, part_tmp, partool_size);
    if (err) {
        goto error;
    }

    err = __partool_write(partition);
    if (err) {
        goto error;
    }
    
    err = 0;
error:
    return err;
}


static int
partool_init(int mode, int argc, char *argv[])
{
    char *partition = argv[2];
    int err;
    
    err = __partool_init(partition);
    if (err) {
        return err;
    }
    
    blk = part_block_create(
            mode, 
            partool_size, 
            partool_read, 
            partool_write,
#ifdef __BOOT__
            partition,
            NULL,
#else
            partool_tmpfile, 
            partool_humfile, 
#endif
            0);
    if (NULL==blk) {
        return -EINVAL;
    }

    return 0;
}


void
partool_clean(void)
{
    part_block_free(blk);
    
    __partool_clean();
}

static inline void
show_cursor(struct string_kv *c, bool simple)
{
    if (simple) {
        os_println("%s", c->v.var);
    } else {
        os_println("%s=%s", c->k.var, c->v.var);
    }
}

/* {"-part", "partname", "-crc"} */
static int
cmd_crc(int argc, char *argv[])
{
    int err = 0;
    unsigned int crc_part = 0;
    unsigned int crc_calc = 0;
    
    err = partool_init(PART_MODE_CRC, argc, argv);
    if (err) {
        debug_error("partool init error(%d)", err);
        
        /* 
        * if block is good, NO return
        */
        if (false==part_block_is_good(blk)) {
            return err;
        }
    }

    part_block_crc_get(blk, &crc_part, &crc_calc);
    if (err) {
        return part_error(err, NULL);
    }

    os_println("0x%x 0x%x", crc_part, crc_calc);
    
    return 0;
}

/* {"-part", "partname", "-empty"} */
static int
cmd_empty(int argc, char *argv[])
{
    int err = 0;
    
    err = partool_init(PART_MODE_EMPTY, argc, argv);
    if (err) {
        return part_error(err, NULL);
    }
    
    return 0;
}

static multi_value_t 
show_cb(struct part_block *block, struct string_kv *c, void *data)
{
    show_cursor(c, false);
    
    (void)block;
    (void)data;
    
    return mv2_OK;
}

/* {"-part", "partname", "-show"} */
static int
cmd_show_all(int argc, char *argv[])
{
    int err;
    
    err = partool_init(PART_MODE_NORMAL, argc, argv);
    if (err) {
        return part_error(err, NULL);
    }
    
    err = part_var_foreach(blk, show_cb, NULL);
    if (err) {
        return part_error(err, NULL);
    }
    
    return 0;
}

/* {"-part", "partname", "-show", "name"} */
static int 
cmd_show_byname(int argc, char *argv[])
{
    struct string_kv c;
    char *name = argv[4];
    int err = 0;
    
    err = partool_init(PART_MODE_NORMAL, argc, argv);
    if (err) {
        return part_error(err, name);
    }
    
    err = part_var_find(blk, name, &c);
    if (err) {
        return part_error(err, name);
    }

    show_cursor(&c, true);
    
    return 0;
}

/* {"-part", "partname", "-new", "name", "value"} */
int 
cmd_new(int argc, char *argv[])
{
    char *name = argv[4];
    char *value = argv[5];
    int err = 0;
    
    err = partool_init(PART_MODE_NORMAL, argc, argv);
    if (err) {
        return part_error(err, name);
    }

    err = part_var_new(blk, name, value);
    if (err) {
        return part_error(err, name);
    }
    
    return 0;
}

/* {"-part", "partname", "-delete", "name"} */
static int 
cmd_delete(int argc, char *argv[])
{
    char *name = argv[4];
    int err;
    
    err = partool_init(PART_MODE_NORMAL, argc, argv);
    if (err) {
        return part_error(err, name);
    }

    err = part_var_delete(blk, name);
    if (err) {
        return part_error(err, name);
    }
    
    return 0;
}

#ifndef __BOOT__
/* {"-part", "partname", "-load", "file"} */
static int
cmd_load(int argc, char *argv[])
{
    int err = 0;

    partool_humfile = argv[4];
    
    err = partool_init(PART_MODE_LOAD, argc, argv);
    if (err) {
        return part_error(err, NULL);
    }
    
    return 0;
}

/* {"-part", "partname", "-show", "-prefix", "name"} */
static int
cmd_show_byprefix(int argc, char *argv[])
{
    char *name = argv[5];
    int err;
    
    err = partool_init(PART_MODE_NORMAL, argc, argv);
    if (err) {
        return part_error(err, name);
    }
    
    err = part_var_foreach_byname_prefix(blk, name, show_cb, NULL);
    if (err) {
        return part_error(err, name);
    }

    return 0;
}

/* {"-part", "partname", "-delete", "-prefix", "name"} */
static int 
cmd_delete_byprefix(int argc, char *argv[])
{
    char *name = argv[5];
    int err;
    
    err = partool_init(PART_MODE_NORMAL, argc, argv);
    if (err) {
        return part_error(err, name);
    }

    err = part_var_delete_byname_prefix(blk, name);
    if (err) {
        return part_error(err, name);
    }
    
    return 0;
}

/* {"-part", "partname", "-create", "name", "value"} */
static int 
cmd_create(int argc, char *argv[])
{
    char *name = argv[4];
    char *value = argv[5];
    int err = 0;
    
    err = partool_init(PART_MODE_NORMAL, argc, argv);
    if (err) {
        return part_error(err, name);
    }

    err = part_var_create(blk, name, value);
    if (err) {
        return part_error(err, name);
    }
    
    return 0;
}

/* {"-part", "partname", "-update", "name", "value"} */
static int 
cmd_update(int argc, char *argv[])
{
    char *name = argv[4];
    char *value = argv[5];
    int err;
    
    err = partool_init(PART_MODE_NORMAL, argc, argv);
    if (err) {
        return part_error(err, name);
    }

    err = part_var_update(blk, name, value);
    if (err) {
        return part_error(err, name);
    }
    
    return 0;
}
#endif

#define PARTNAME_HELP   "partname(" NAME_GOOD ")"

int 
partool_main
(
#ifdef __BOOT__
	cmd_tbl_t *cmdtp,
	int flag,
#endif
	int argc,
 	char *argv[]
)
{
    struct command_item commands[] = {
        {
            .list = {"-part", PARTNAME_HELP, "-crc"},
            .func = cmd_crc,
            .help = "calc crc",
        },
        {
            .list = {"-part", PARTNAME_HELP, "-empty"},
            .func = cmd_empty,
            .help = "flush empty to mtd",
        },
        {
            .list = {"-part", PARTNAME_HELP, "-show"},
            .func = cmd_show_all,
            .help = "show all",
        },
        {
            .list = {"-part", PARTNAME_HELP, "-show", "name"},
            .func = cmd_show_byname,
            .help = "show by name",
        },
        {
            .list = {"-part", PARTNAME_HELP, "-new", "name", "value"},
            .func = cmd_new,
            .help = "new(create or update)",
        },
        {
            .list = {"-part", PARTNAME_HELP, "-delete", "name"},
            .func = cmd_delete,
            .help = "delete by name, the name must must exist",
        },
#ifndef __BOOT__
        {
            .list = {"-part", PARTNAME_HELP, "-load", "file"},
            .func = cmd_load,
            .help = "load file(human readable) to mtd",
        },
        {
            .list = {"-part", PARTNAME_HELP, "-show", "-prefix", "name"},
            .func = cmd_show_byprefix,
            .help = "show by name prefix",
        },
        {
            .list = {"-part", PARTNAME_HELP, "-delete", "-prefix", "name"},
            .func = cmd_delete_byprefix,
            .help = "delete by name prefix",
        },
        {
            .list = {"-part", PARTNAME_HELP, "-create", "name", "value"},
            .func = cmd_create,
            .help = "create name/value, the name cannot exist",
        },
        {
            .list = {"-part", PARTNAME_HELP, "-update", "name", "value"},
            .func = cmd_update,
            .help = "update name/value, the name must exist",
        },
#endif
    };
    
    struct command_ctrl ctrl = COMMAND_CTRL_INITER(commands);
    int err = 0;
    
    err = os_do_command(argc, argv, &ctrl);
    
    partool_clean();
    
#ifdef __BOOT__
    return 0;
#else
    return err;
#endif
}

#ifdef __BOOT__
U_BOOT_CMD(
	partool,	6,	1,	partool_main,
	"partool     - Partool utility commands\n",

	"partool -part  <partname>  -crc       - show crc\n"
	"partool -part  <partname>  -empty       - clean the mtd\n"
	"partool -part  <partname>  -show       - display <partname> info \n"
	"partool -part  <partname>  -show   <name>           - display <partname> info about <name>\n"
	"partool -part  <partname>  -new   <name>  <value>- write <partname> info about <name> \n"
	"partool -part  <partname>  -delete   <name>  - delete <partname> info about <name> \n"

);

int
boot_partool_new(int argc, char *argv[])
{
    return cmd_new(argc, argv);
}

void
boot_partool_clean(void)
{
    partool_clean();
}

int
boot_partool_show_byname(int argc, char *argv[],void *buf)
{
    struct string_kv c;
    char *name = argv[4];
    int err = 0;
    int len;
    
    err = partool_init(PART_MODE_NORMAL, argc, argv);
    if (err) {
        return part_error(err, name);
    }
    
    err = part_var_find(blk, name, &c);
    if (err) {
        return part_error(err, name);
    }

    strcpy(buf, c.v.var);

    show_cursor(&c, true);
    
    return 0;    
}

#else
int main(int argc, char *argv[])
{
    return partool_main(argc, argv);
}

static os_destructor void
__fini(void)
{
    partool_clean();
}
#endif
/******************************************************************************/
