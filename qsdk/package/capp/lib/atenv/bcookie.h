#ifndef __BCOOKIE_H_31502FF802B81D17AEDABEB5EDB5C121__
#define __BCOOKIE_H_31502FF802B81D17AEDABEB5EDB5C121__
#include "atenv/atenv.h"
/******************************************************************************/
enum {
    BCOOKIE_OTP = 538050839,
    BCOOKIE_CID,
};

#define BCOOKIE_COUNT   7

struct bcookie {
    unsigned int id[BCOOKIE_COUNT];
};

#define BCOOKIE(_id)    {   \
    .id = {                 \
        [0] = 0x01330235,   \
        [1] = 0x2012dead,   \
        [2] = 0xf00d1e00,   \
        [3] = _id,          \
        [4] = 0x00e1d00f,   \
        [5] = 0xdaed2102,   \
        [6] = 0x53023310,   \
    },                      \
}   /* end */

#define BCOOKIE_ID(_obj)    (_obj)->id[BCOOKIE_COUNT/2]
#define BCOOKIE_OBJ(_id)    { .header = BCOOKIE(_id) }

static inline void
bcookie_add_ref(struct bcookie *bc)
{
    bc->id[BCOOKIE_COUNT/2]++;
    bc->id[BCOOKIE_COUNT/2]--;
}

struct bcookie_otp {
    struct bcookie header;
    unsigned char custom[16];
    unsigned char private[16];
};

struct bcookie_cid {
    struct bcookie header;
    unsigned int cid[4];
};

#define bcookie_cid_psn(_cid)   (((_cid[2] & 0xffff) << 16) | (_cid[3] >> 16))
#define bcookie_cid_mid(_cid)   (_cid[0] >> 24)

static inline void
bcookie_cid_dump(struct bcookie_cid *bcid)
{
#if 1
    int i;

    for (i=0; i<4; i++) {
        os_println("CID:%d", bcid->cid[i]);
    }

    os_println("MID:%d", bcookie_cid_mid(bcid->cid));
    os_println("PSN:%d", bcookie_cid_psn(bcid->cid));
#endif
}

static inline int
bcookie_find(unsigned char *mem, int size, int id)
{
    struct bcookie bc = BCOOKIE(id);
    if (size < sizeof(bc)) {
        return -ETOOSMALL;
    }
    
    unsigned char *p = (unsigned char *)os_memmem(mem, size, &bc, sizeof(bc));
    if (NULL==p) {
        debug_error("no-found bcookie:%d", id);
        
        return -ENOEXIST;
    }

    return p - mem;
}

#ifdef __BOOT__
static inline int
__bcookie_check(unsigned int begin, unsigned int size, struct bcookie *obj, unsigned int osize)
{
    if (osize<=sizeof(struct bcookie)) {
        return os_assertV(-EINVAL5);
    }
    else if (size<osize) {
        return os_assertV(-EINVAL4);
    }
    else {
        return 0;
    }
}

static inline int
__bcookie_load(int begin, int size, struct bcookie *obj, int osize)
{
    int err = 0, offset = -1;
    int id = BCOOKIE_ID(obj);
    unsigned char *mem = NULL;

    err = __bcookie_check(begin, size, obj, osize);
    if (err) {
        return err;
    }

    mem = (unsigned char *)os_malloc(size);
    if (NULL==mem) {
        err = -ENOMEM; goto error;
    }

    if (size != at_emmc_read(begin, mem, size)) {
        debug_error("read emmc error:%d", errno);
        
        err = -errno; goto error;
    }
    
    offset = bcookie_find(mem, size, id);
    if (offset<0) { /* yes, <0 */
        err = offset; goto error;
    }

    os_memcpy(obj, mem + offset, osize);
    
error:
    if (mem) {
        os_free(mem);
    }
    
    return err;
}

static inline int
__bcookie_save(int begin, int size, struct bcookie *obj, int osize)
{
    int err = 0, offset = -1;
    int id = BCOOKIE_ID(obj);
    unsigned char *mem = NULL;
    
    err = __bcookie_check(begin, size, obj, osize);
    if (err) {
        return err;
    }

    mem = (unsigned char *)os_malloc(size);
    if (NULL==mem) {
        err = -ENOMEM; goto error;
    }
    
    if (size != at_emmc_read(begin, mem, size)) {
        debug_error("read emmc error:%d", errno);
        
        err = -errno; goto error;
    }
    
    offset = bcookie_find(mem, size, id);
    if (offset<0) { /* yes, <0 */
        err = offset; goto error;
    }
    os_memcpy(mem + offset, obj, osize);

    unsigned int block_begin = os_align_down(offset, AT_ENV_BLOCK_SIZE)/AT_ENV_BLOCK_SIZE;
    unsigned int block_end   = os_align(offset + osize - 1, AT_ENV_BLOCK_SIZE)/AT_ENV_BLOCK_SIZE;
    unsigned int block_count = block_end - block_begin + 1;
    
    if (block_count * AT_ENV_BLOCK_SIZE != at_emmc_write(begin + block_begin * AT_ENV_BLOCK_SIZE, 
                    mem + block_begin * AT_ENV_BLOCK_SIZE, 
                    block_count * AT_ENV_BLOCK_SIZE)) {
        debug_error("write emmc error:%d", errno);

        err = -errno; goto error;
    }
    
error:
    if (mem) {
        os_free(mem);
    }
    
    return err;
}

static inline int
bcookie_load(struct bcookie *obj, int size)
{
    return __bcookie_load(0, AT_BOOT_SIZE, obj, size);
}

static inline int
bcookie_save(struct bcookie *obj, int size)
{
    return __bcookie_save(0, AT_BOOT_SIZE, obj, size);
}
#elif defined(__APP__)
static inline int
__bcookie_check(char *file, unsigned int fsize, struct bcookie *obj, unsigned int osize)
{
    if (NULL==file) {
        return os_assertV(-EINVAL8);
    }
    else if (NULL==obj) {
        return os_assertV(-EINVAL7);
    }
    else if (osize<=sizeof(struct bcookie)) {
        return os_assertV(-EINVAL5);
    }
    else if (fsize<osize) {
        return os_assertV(-EINVAL4);
    }
    else {
        return 0;
    }
}

static inline int
__bcookie_load(char *file, int fsize, struct bcookie *obj, int osize)
{
    int fd = -1, err = 0, offset = -1;
    int id = BCOOKIE_ID(obj);
    unsigned char *mem = NULL;

    err = __bcookie_check(file, fsize, obj, osize);
    if (err) {
        return err;
    }

    debug_trace("before open %s", file);
    
    fd = __os_lock_file(file, O_RDONLY, S_IRUSR | S_IWUSR, false);
    if (false==is_good_fd(fd)) {
        err = -errno; goto error;
    }
    
    mem = (unsigned char *)os_malloc(fsize);
    if (NULL==mem) {
        err = -ENOMEM; goto error;
    }

    if (fsize != read(fd, mem, fsize)) {
        debug_error("read boot error:%d", errno);
        
        err = -errno; goto error;
    }
    
    offset = bcookie_find(mem, fsize, id);
    if (offset<0) { /* yes, <0 */
        err = offset; goto error;
    }

    os_memcpy(obj, mem + offset, osize);
    
error:
    os_unlock_file(file, fd);
    if (mem) {
        os_free(mem);
    }
    
    return err;
}

static inline int
__bcookie_save(char *file, int fsize, struct bcookie *obj, int osize)
{
    int fd = -1, err = 0, offset = -1;
    int id = BCOOKIE_ID(obj);
    unsigned char *mem = NULL;
    
    err = __bcookie_check(file, fsize, obj, osize);
    if (err) {
        return err;
    }
        
    fd = __os_lock_file(file, O_RDWR, S_IRUSR | S_IWUSR, false);
    if (false==is_good_fd(fd)) {
        err = -errno; goto error;
    }
        
    mem = (unsigned char *)os_malloc(fsize);
    if (NULL==mem) {
        err = -ENOMEM; goto error;
    }
    
    if (fsize != read(fd, mem, fsize)) {
        debug_error("read boot error:%d", errno);
        
        err = -errno; goto error;
    }
    
    offset = bcookie_find(mem, fsize, id);
    if (offset<0) { /* yes, <0 */
        err = offset; goto error;
    }
    
    err = lseek(fd, offset, SEEK_SET);
    if (err < 0) {        /* <0 is error */
        debug_error("seek boot error:%d", errno);

        err = -errno; goto error;
    }
    
    if (osize != write(fd, obj, osize)) {
        debug_error("write boot error:%d", errno);

        err = -errno; goto error;
    }
    
error:
    os_unlock_file(file, fd);
    if (mem) {
        os_free(mem);
    }
    
    return err;
}

static inline int
bcookie_load(struct bcookie *obj, int size)
{
    return __bcookie_load(AT_DEV_BOOT, AT_BOOT_SIZE, obj, size);
}

static inline int
bcookie_save(struct bcookie *obj, int size)
{
    return __bcookie_save(AT_DEV_BOOT, AT_BOOT_SIZE, obj, size);
}
#endif

/******************************************************************************/
#endif /* __BCOOKIE_H_31502FF802B81D17AEDABEB5EDB5C121__ */
