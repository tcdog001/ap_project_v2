#ifndef __ATENV_H_31502FF802B81D17AEDABEB5EDB5C121__
#define __ATENV_H_31502FF802B81D17AEDABEB5EDB5C121__
#include "utils.h"
/******************************************************************************/
#define AT_PRODUCT_LTEFI_MD     1
#define AT_PRODUCT_LTEFI_AP     2
#define AT_PRODUCT_LTEFI_MD2    3
#define AT_PRODUCT_LTEFI_AP2    4

#ifndef AT_PRODUCT
#define AT_PRODUCT              AT_PRODUCT_LTEFI_MD2
#endif

#define AT_UPGRADE_ONE          1
#define AT_UPGRADE_TWO          2

#ifndef AT_UPGRADE
#define AT_UPGRADE              AT_UPGRADE_ONE
#endif

#ifndef AT_DEV_BLOCK_SIZE
#define AT_DEV_BLOCK_SIZE       512
#endif

#ifndef AT_ENV_SIZE
#define AT_ENV_SIZE             4096
#endif

#ifndef AT_BOOT_SIZE
#define AT_BOOT_SIZE            (512*1024)
#endif

#define AT_ENV_START            AT_BOOT_SIZE

#ifdef __PC__
#define AT_ROOT                 __empty
#else
#define AT_ROOT                 "/"
#endif

#define AT_ROOTFS_MODE_TYPE_RW  1
#define AT_ROOTFS_MODE_TYPE_RO  2

#ifndef AT_ROOTFS_MODE_TYPE
#define AT_ROOTFS_MODE_TYPE     AT_ROOTFS_MODE_TYPE_RW
#endif

#ifndef AT_ROOTFS_MODE
#   if AT_ROOTFS_MODE_TYPE==AT_ROOTFS_MODE_TYPE_RW
#       define AT_ROOTFS_MODE          "rw"
#   elif AT_ROOTFS_MODE_TYPE==AT_ROOTFS_MODE_TYPE_RO
#       define AT_ROOTFS_MODE          "ro"
#   else
#       error "bad AT_ROOTFS_MODE_TYPE"
#   endif
#endif

#if AT_PRODUCT==AT_PRODUCT_LTEFI_MD2
#   define AT_OS_COUNT          7
#   define AT_OS_CURRENT        1
#   define AT_DEV_PREFIX        "p"
#   define AT_DEV_FLASH_MASTER  "dev/mmcblk0"
#   define AT_DEV_SD_MASTER     "dev/mmcblk1"
#   define AT_DEV_HD_MASTER     "dev/udisk"
#   define AT_DEV_USB_MASTER    "dev/udisk1310"
#elif AT_PRODUCT==AT_PRODUCT_LTEFI_AP2
#   define AT_OS_COUNT          1
#   define AT_OS_CURRENT        0
#   define AT_DEV_PREFIX        __empty
#   define AT_DEV_FLASH_MASTER  "dev/mtdblock"
#   define AT_DEV_SD_MASTER     __empty
#   define AT_DEV_HD_MASTER     __empty
#   define AT_DEV_USB_MASTER    __empty
#endif

#define AT_FILE(_file)          AT_ROOT _file
#define AT_DIR(_dir)            AT_ROOT _dir

#define AT_MASTER(_master)      AT_FILE(_master AT_DEV_PREFIX)
#define AT_DEV(_master, _idx)   AT_MASTER(_master) #_idx
#define AT_DEV_FLASH(_idx)      AT_DEV(AT_DEV_FLASH_MASTER, _idx)
#define AT_DEV_SD(_idx)         AT_DEV(AT_DEV_SD_MASTER, _idx)
#define AT_DEV_HD(_idx)         AT_DEV(AT_DEV_HD_MASTER, _idx)
#define AT_DEV_USB(_idx)        AT_DEV(AT_DEV_USB_MASTER, _idx)

#define AT_DIR_FLASH            AT_DIR("mnt/flash")
#define AT_DIR_SD               AT_DIR("mnt/sd")
#define AT_DIR_HD               AT_DIR("mnt/hd")
#define AT_DIR_USB              AT_DIR("mnt/usb")

#define __AT_DIR_KERNEL         AT_DIR_FLASH "/kernel"
#define __AT_DIR_ROOTFS         AT_DIR_FLASH "/rootfs"
#define __AT_DIR_CONFIG         AT_DIR_FLASH "/config"
#define __AT_DIR_DATA           AT_DIR_FLASH "/data"
#define __AT_DIR_OTHER          AT_DIR_FLASH "/other"

#define AT_DIR_KERNEL(_idx)     __AT_DIR_KERNEL #_idx
#define AT_DIR_ROOTFS(_idx)     __AT_DIR_ROOTFS #_idx
#define AT_DIR_CONFIG(_idx)     __AT_DIR_CONFIG #_idx
#define AT_DIR_DATA(_idx)       __AT_DIR_DATA   #_idx
#define AT_DIR_OTHER            __AT_DIR_OTHER

#ifndef AT_DEV_BOOTENV
#ifdef __PC__
#   define AT_DEV_BOOT          "boot"
#   define AT_DEV_BOOTENV       "bootenv"
#else
#   define AT_DEV_BOOT          AT_DEV_FLASH(1) /* boot */
#   define AT_DEV_BOOTENV       AT_DEV_FLASH(2) /* boot env */
#endif
#endif

#define AT_PROC_CMDLINE         "/proc/cmdline"

#if AT_PRODUCT==AT_PRODUCT_LTEFI_MD2
#define __DEV_KERNEL0       AT_DEV_FLASH(6)
#define __DEV_KERNEL1       AT_DEV_FLASH(7)
#define __DEV_KERNEL2       AT_DEV_FLASH(8)
#define __DEV_KERNEL3       AT_DEV_FLASH(9)
#define __DEV_KERNEL4       AT_DEV_FLASH(10)
#define __DEV_KERNEL5       AT_DEV_FLASH(11)
#define __DEV_KERNEL6       AT_DEV_FLASH(12)

#define __DEV_ROOTFS0       AT_DEV_FLASH(13)
#define __DEV_ROOTFS1       AT_DEV_FLASH(14)
#define __DEV_ROOTFS2       AT_DEV_FLASH(15)
#define __DEV_ROOTFS3       AT_DEV_FLASH(16)
#define __DEV_ROOTFS4       AT_DEV_FLASH(17)
#define __DEV_ROOTFS5       AT_DEV_FLASH(18)
#define __DEV_ROOTFS6       AT_DEV_FLASH(19)

#define __DEV_CONFIG0       AT_DEV_FLASH(20)
#define __DEV_CONFIG1       AT_DEV_FLASH(21)

#define __DEV_DATA0         AT_DEV_FLASH(22)
#define __DEV_DATA1         AT_DEV_FLASH(23)

#define __DEV_OTHER         AT_DEV_FLASH(24)

#define DEV_BOOT            AT_DEV_FLASH(1)
#define DEV_BOOTENV         AT_DEV_BOOTENV

#define DEV_HD              AT_DEV_HD(1)
#define DEV_SD              AT_DEV_SD(1)
#define DEV_USB             AT_DEV_USB(1)

#define DEV_KERNEL(_idx)    __DEV_KERNEL##_idx
#define DEV_ROOTFS(_idx)    __DEV_ROOTFS##_idx
#define DEV_CONFIG(_idx)    __DEV_CONFIG##_idx
#define DEV_DATA(_idx)      __DEV_DATA##_idx
#define DEV_OTHER           __DEV_OTHER

#define CONFIG_BOOTARGS_HEAD    \
    "root=" AT_DEV_FLASH(14)    \
        " "                     \
    /* end */

#define CONFIG_BOOTARGS_BODY    \
    "mem=2G"                    \
        " "                     \
    "console=ttyAMA0,115200"    \
        " "                     \
    "rootfstype=ext4"           \
        " "                     \
    "rootwait"                  \
        " "                     \
    AT_ROOTFS_MODE              \
        " "                     \
    "blkdevparts="              \
        "mmcblk0:"              \
        "512K(fastboot),"/*01 */\
        "512K(bootenv),"/* 02 */\
        "4M(baseparam),"/* 03 */\
        "4M(pqparam),"  /* 04 */\
        "3M(logo),"     /* 05 */\
                                \
        "16M(kernel0)," /* 06 */\
        "16M(kernel1)," /* 07 */\
        "16M(kernel2)," /* 08 */\
        "16M(kernel3)," /* 09 */\
        "16M(kernel4)," /* 10 */\
        "16M(kernel5)," /* 11 */\
        "16M(kernel6)," /* 12 */\
                                \
        "256M(rootfs0),"/* 13 */\
        "256M(rootfs1),"/* 14 */\
        "256M(rootfs2),"/* 15 */\
        "256M(rootfs3),"/* 16 */\
        "256M(rootfs4),"/* 17 */\
        "256M(rootfs5),"/* 18 */\
        "256M(rootfs6),"/* 19 */\
                                \
        "32M(config0)," /* 20 */\
        "32M(config1)," /* 21 */\
                                \
        "820M(data0),"  /* 22 */\
        "820M(data1),"  /* 23 */\
        "-(others)"             \
        " "                     \
    "mmz=ddr,0,0,300M"          \
    /* end */

#ifdef CONFIG_BOOTARGS
#undef CONFIG_BOOTARGS
#endif

#define CONFIG_BOOTARGS         \
    CONFIG_BOOTARGS_HEAD        \
    CONFIG_BOOTARGS_BODY        \
    /* end */
#endif

#define AT_ENV_BLOCK_SIZE   AT_DEV_BLOCK_SIZE
#define AT_ENV_BLOCK_COUNT  (AT_ENV_SIZE/AT_ENV_BLOCK_SIZE)    /* 8 */

#ifndef AT_TRYS
#define AT_TRYS             3
#endif

#ifndef AT_VENDOR
#define AT_VENDOR           "Autelan"
#endif

#ifndef AT_COMPANY
#define AT_COMPANY          "Beijing Autelan Technology Co.,Ltd."
#endif

#ifndef AT_PRODUCT_MODEL
#define AT_PRODUCT_MODEL    "AQ2000-LV1"
#endif

#ifndef AT_PCBA_MODEL
#define AT_PCBA_MODEL       "need define it"
#endif

#ifndef AT_PCBA_VERSION
#define AT_PCBA_VERSION     "need define it"
#endif

#define AT_MARK_COUNT       (AT_ENV_BLOCK_SIZE/sizeof(unsigned int))    /* 128 */

#ifndef AT_INFO_SIZE
#define AT_INFO_SIZE        64
#endif

#define AT_INFO_COUNT_PER_BLOCK     (AT_ENV_BLOCK_SIZE/AT_INFO_SIZE)    /* 512/64=8 */
#define AT_INFO_COUNT       ((AT_ENV_BLOCK_COUNT-3)*AT_INFO_COUNT_PER_BLOCK)    /* 5*8=40 */

#ifndef at_kernel_size
#define at_kernel_size      0x8000  /* block count */
#endif

#ifndef at_rootfs_size
#define at_rootfs_size      0x80000 /* block count */
#endif

#define at_obj_size(_obj)   at_##_obj##_size

#define is_good_atidx(_idx)         is_good_enum(_idx, AT_OS_COUNT)
#define is_normal_atidx(_idx)       is_good_value(_idx, 1, AT_OS_COUNT)

#define __at_foreach(_i, _begin)    for (_i=_begin; _i<AT_OS_COUNT; _i++)
#define at_foreach(_i)              __at_foreach(_i, 1)

/*
* ok > unknow > fail
*/
enum {
    AT_FSM_FAIL,
    AT_FSM_UNKNOW,
    AT_FSM_OK,

    AT_FSM_END
};
#define AT_FSM_INVALID  AT_FSM_END

#define AT_FSM_STRINGS      {   \
    [AT_FSM_FAIL]   = "fail",   \
    [AT_FSM_UNKNOW] = "unknow", \
    [AT_FSM_OK]     = "ok",     \
}   /* end */

static inline bool
is_good_at_fsm(int fsm)
{
    return is_good_enum(fsm, AT_FSM_END);
}

/*
* ok or unknow
*/
static inline bool
is_canused_at_fsm(int fsm)
{
    return is_good_value(fsm, AT_FSM_UNKNOW, AT_FSM_END);
}

static inline char **
__at_fsm_strings(void)
{
    static char *array[AT_FSM_END] = AT_FSM_STRINGS;

    return array;
}

static inline char *
at_fsm_string(int fsm)
{
    char **array = __at_fsm_strings();

    return is_good_at_fsm(fsm)?array[fsm] : __unknow;
}

static inline int
at_fsm_idx(char *fsm)
{
    char **array = __at_fsm_strings();

    return os_getstringarrayidx(array, fsm, 0, AT_FSM_END);
}

static inline int
at_fsm_cmp(int a, int b)
{
    return a - b;
}

#define AT_COOKIE_SIZE      (0  \
    + sizeof(AT_VENDOR)         \
    + sizeof(AT_COMPANY)        \
    + sizeof(AT_PCBA_MODEL)     \
    + sizeof(AT_PCBA_VERSION)   \
)   /* end */

typedef struct {
    struct {
        char vendor[sizeof(AT_VENDOR)];
        char company[sizeof(AT_COMPANY)];
    }
    product;

    struct {
        char model[sizeof(AT_PCBA_MODEL)];
        char version[sizeof(AT_PCBA_VERSION)];
    }
    pcba;

    char pad[AT_ENV_BLOCK_SIZE - AT_COOKIE_SIZE];
} at_cookie_t;            /* 512 */

static inline void
__at_cookie_show(at_cookie_t *cookie)
{
    os_println("vendor :%s", cookie->product.vendor);
    os_println("company:%s", cookie->product.company);
    os_println("model  :%s", cookie->pcba.model);
    os_println("version:%s", cookie->pcba.version);
}

#define AT_DEFT_COOKIE              {   \
    .product        = {                 \
        .vendor     = AT_VENDOR,        \
        .company    = AT_COMPANY,       \
    },                                  \
    .pcba           = {                 \
        .model      = AT_PCBA_MODEL,    \
        .version    = AT_PCBA_VERSION,  \
    },                                  \
}   /* end */

typedef struct {
    unsigned char number[4];
} at_version_t;

#define AT_MIN_VERSION_NUMBER   0
#define AT_MAX_VERSION_NUMBER   255
#define AT_MIN_VERSION_STRING   "0.0.0.0"
#define AT_MAX_VERSION_STRING   "255.255.255.255"
#define AT_MIN_VERSION          {   \
    .number = {                     \
        AT_MIN_VERSION_NUMBER,      \
        AT_MIN_VERSION_NUMBER,      \
        AT_MIN_VERSION_NUMBER,      \
        AT_MIN_VERSION_NUMBER,      \
    },                              \
}   /* end */

#define AT_MAX_VERSION          {   \
    .number = {                     \
        AT_MAX_VERSION_NUMBER,      \
        AT_MAX_VERSION_NUMBER,      \
        AT_MAX_VERSION_NUMBER,      \
        AT_MAX_VERSION_NUMBER,      \
    },                              \
}   /* end */

#define AT_INVALID_VERSION      AT_MIN_VERSION
#define AT_DEFT_VERSION         AT_MIN_VERSION

#define AT_VERSION_STRING_SIZE  sizeof(AT_MAX_VERSION_STRING)    /* 16 */

static inline char *
__at_version_itoa(at_version_t * version, char string[])
{
    os_sprintf(string, "%d.%d.%d.%d",
           version->number[0],
           version->number[1], version->number[2], version->number[3]);

    return string;
}

static inline char *
at_version_itoa(at_version_t * version)
{
    static char string[AT_VERSION_STRING_SIZE];

    return __at_version_itoa(version, string);
}

static inline int
at_version_atoi(at_version_t * version, char *string)
{
    char line[1 + OS_LINE_LEN] = { 0 };
    char *number[4] = { line, NULL, NULL, NULL };
    int i;

    if (NULL==string) {
        return -EFORMAT;
    }
    
    os_strdcpy(line, string);

    for (i = 1; i < 4; i++) {
        number[i] = os_strchr(number[i - 1], '.');
        if (NULL == number[i]) {
            debug_error("bad version:%s", string);

            return -EFORMAT;
        }

        *(number[i])++ = 0;
    }

    for (i = 0; i < 4; i++) {
        char *end = NULL;

        version->number[i] = os_strtoul(number[i], &end, 0);
        if (false==os_strtonumber_is_good(end)) {
            debug_error("bad-version:%s", string);

            return -EFORMAT;
        }
    }

    return 0;
}

static inline int
at_version_cmp(at_version_t * a, at_version_t * b)
{
    return os_objcmp(a, b);
}
#define at_version_eq(_a, _b)   (0==at_version_cmp(_a, _b))

#define AT_VCS_COOKIE_SIZE  15

typedef struct {
    unsigned int self;
    unsigned int other;
    unsigned int upgrade;
    unsigned int error;
    
    at_version_t version;

    char cookie[1+AT_VCS_COOKIE_SIZE];
} at_vcs_t; /* 36 */

#define __AT_VCS(_self, _other, _upgrade, _error, _version)   { \
    .self       = _self,        \
    .other      = _other,       \
    .upgrade    = _upgrade,     \
    .error      = _error,       \
    .version    = _version,     \
    .cookie   = {0},            \
}   /* end */

static inline void
__at_vcs_deft(at_vcs_t *vcs)
{
    vcs->error      = 0;
    vcs->upgrade    = AT_FSM_OK;
    vcs->other      = AT_FSM_UNKNOW;
    vcs->self       = AT_FSM_UNKNOW;

    os_objzero(vcs->cookie);
}

#define AT_DEFT_VCS \
        __AT_VCS(AT_FSM_UNKNOW, AT_FSM_UNKNOW, AT_FSM_OK, 0, AT_DEFT_VERSION)
#define AT_MIN_VCS \
        __AT_VCS(AT_FSM_FAIL, AT_FSM_FAIL, AT_FSM_FAIL, AT_TRYS, AT_MIN_VERSION)
#define AT_MAX_VCS \
        __AT_VCS(AT_FSM_OK, AT_FSM_OK, AT_FSM_OK, 0, AT_MAX_VERSION)
#define AT_INVALID_VCS \
        __AT_VCS(AT_FSM_INVALID, AT_FSM_INVALID, AT_FSM_INVALID, AT_TRYS, AT_INVALID_VERSION)

#define is_at_good(_error)          is_good_enum(_error, AT_TRYS)
#define is_at_error(_error)         (false==is_at_good(_error))

static inline int
at_error_cmp(unsigned int a, unsigned b)
{
    return __os_objcmp(a, b, is_at_good, os_cmp_always_eq);
}

static inline bool
at_vcs_is_good(at_vcs_t * vcs)
{
    return is_at_good(vcs->error)           /* no error */
        && AT_FSM_OK == vcs->upgrade        /* upgrade ok */
        && is_canused_at_fsm(vcs->self);    /* self startup ok, or have not startup */
}

typedef struct {
    at_vcs_t kernel;
    at_vcs_t rootfs;
} at_firmware_t;        /* 2*36 = 72 */

static inline void
__at_firmware_deft(at_firmware_t *firmware)
{
    __at_vcs_deft(&firmware->rootfs);
    __at_vcs_deft(&firmware->kernel);
}

#define AT_DEFT_FIRMWARE  { \
    .kernel = AT_DEFT_VCS,  \
    .rootfs = AT_DEFT_VCS,  \
}   /* end */

#define AT_INVALID_FIRMWARE  {  \
    .kernel = AT_INVALID_VCS,   \
    .rootfs = AT_INVALID_VCS,   \
}   /* end */

#define AT_OS_SIZE  (2*sizeof(unsigned int) + AT_OS_COUNT*sizeof(at_firmware_t))

typedef struct {
    unsigned int current;
    unsigned int reserved;

    at_firmware_t firmware[AT_OS_COUNT];
    char pad[AT_ENV_BLOCK_SIZE - AT_OS_SIZE];
} at_os_t; /* 512 */


#define AT_DEFT_OS  {   \
    .current    = AT_OS_CURRENT, \
    .firmware   = {     \
        [0 ... (AT_OS_COUNT-1)] = AT_DEFT_FIRMWARE, \
    },                  \
}   /* end */

static inline void
__at_os_show(at_os_t *os)
{
    int i;
    
#if 0
        index   error   upgrade self    other   version         cookie
kernel  0       0       unknow  unknow  unknow  255.255.255.255 0123456789abcdef
rootfs  0       0       ok      ok      unknow
kernel* 1       0       fail    fail    unknow
rootfs* 1       0       fail    unknow  unknow
#endif

#define at_os_show_obj(_obj, _idx) do{ \
    os_println("%s%c %-7d %-7d %-7s %-7s %-7s %-15s %s",\
        #_obj, _idx==os->current?'*':' ',               \
        _idx, os->firmware[_idx]._obj.error,            \
        at_fsm_string(os->firmware[_idx]._obj.upgrade), \
        at_fsm_string(os->firmware[_idx]._obj.self),    \
        at_fsm_string(os->firmware[_idx]._obj.other),   \
        at_version_itoa(&os->firmware[_idx]._obj.version), \
        os->firmware[_idx]._obj.cookie);                \
    }while(0)
    
    os_println("        index   error   upgrade self    other   version         cookie");
    os_println("======================================================================");
    for (i=0; i<AT_OS_COUNT; i++) {
        at_os_show_obj(kernel, i);
        at_os_show_obj(rootfs, i);
        
        if (i<(AT_OS_COUNT-1)) {
            os_println
              ("----------------------------------------------------------------------");
        }
    }
    os_println("======================================================================");
#undef at_os_show_obj
}

typedef struct {
    unsigned int key[AT_MARK_COUNT];
} at_mark_t;            /* 512 */

#define AT_DEFT_MARK { .key = {0} }

typedef struct {
    char var[AT_INFO_COUNT][AT_INFO_SIZE];
} at_info_t;

#define AT_DEFT_INFO { .var = {{0}} }

enum {
    AT_ENV_COOKIE   = 0,
    AT_ENV_OS       = 1,
    AT_ENV_MARK     = 2,
    AT_ENV_INFO     = 3,

    AT_ENV_END
};

typedef struct {
    at_cookie_t cookie;
    at_os_t os;
    at_mark_t mark;
    at_info_t info;
} at_env_t;

static inline int
__at_ops_is(unsigned int offset)
{
    if (offset >= sizeof(at_env_t)) {
        return -ENOEXIST;
    }
    else if (offset >= offsetof(at_env_t, info)) {
        return AT_ENV_INFO;
    }
    else if (offset >= offsetof(at_env_t, mark)) {
        return AT_ENV_MARK;
    }
    else if (offset >= offsetof(at_env_t, os)) {
        return AT_ENV_OS;
    }
    else {
        return AT_ENV_COOKIE;
    }
}

#define AT_DEFT_ENV         {   \
    .cookie = AT_DEFT_COOKIE,   \
    .os     = AT_DEFT_OS,       \
    .mark   = AT_DEFT_MARK,     \
    .info   = AT_DEFT_INFO,     \
}   /* end */

typedef struct struct_at_ops at_ops_t;
struct struct_at_ops {
    char *path;
    unsigned int offset;
    unsigned int flag;

    int  (*check)(at_ops_t* ops, char *value);
    void (*write)(at_ops_t* ops, char *value);
    void (*show) (at_ops_t* ops);
};

typedef struct {
    char *value;        /* for write */
    bool showit;        /* for show */
} at_cache_t;

/* 
* path without last '*' 
*/
static inline bool
at_ops_match_wildcard(at_ops_t * ops, char *path, int len)
{
    return 0==len || 0==os_memcmp(path, ops->path, len);
}

/* 
* path maybe with last '*' 
*/
static inline bool
at_ops_match(at_ops_t * ops, char *path, int len, bool wildcard)
{
    if (wildcard) {
        return at_ops_match_wildcard(ops, path, len - 1);
    } else {
        return len == os_strlen(ops->path)
            && 0==os_memcmp(ops->path, path, len);
    }
}

typedef struct {
    at_env_t *env;
    at_ops_t *ops;
    at_cache_t *cache;
    int ops_count;
    
    int show_count;
    bool dirty[AT_ENV_BLOCK_COUNT];
    char *self;
    int fd;
} at_control_t;

#define AT_CONTROL_DEFT(_env, _ops, _cache) {   \
    .env        = _env,                         \
    .ops        = _ops,                         \
    .cache      = _cache,                       \
    .ops_count  = (_ops)?os_count_of(_ops):0,   \
    .fd         = -1,                           \
}   /* end */

extern at_control_t at_control;
#define __at_control    (&at_control)

#define __at_fd         __at_control->fd
#define __at_env        __at_control->env
#define __at_ops        __at_control->ops
#define __at_ops_count  __at_control->ops_count
#define __at_show_count __at_control->show_count
#define __at_self       __at_control->self
#define __at_dirty      __at_control->dirty
#define __at_cache      __at_control->cache
#define __at_cookie     (&__at_env->cookie)
#define __at_os         (&__at_env->os)
#define __at_current    __at_os->current
#define __at_firmware   __at_os->firmware
#define __at_mark       (&__at_env->mark)
#define __at_info       (&__at_env->info)

#define at_mark(_idx)   __at_mark->key[_idx]
#define at_info(_idx)   __at_info->var[_idx]

static inline at_ops_t *
at_ops(int idx)
{
    return &__at_ops[idx];
}

static inline int
at_ops_idx(at_ops_t * ops)
{
    return (at_ops_t *) ops - __at_ops;
}

static inline char *
__at_ops_obj(at_ops_t * ops)
{
    return (char *)__at_env + ops->offset;
}

#define at_ops_obj(_type, _ops) ((_type *)__at_ops_obj(_ops))

static inline void
__at_set_dirty(int idx)
{
    __at_dirty[idx] = true;
    
    debug_trace("set block[%d] dirty", idx);
}

static inline void
at_set_dirty(at_ops_t * ops)
{
    int offset = at_ops_obj(char, ops) - (char *)__at_env;
    int idx = offset / AT_ENV_BLOCK_SIZE;
    
    __at_dirty[idx] = true;
    
    debug_trace("set block[%d] dirty, offset=%d", idx, offset);
}

static inline void
at_set_dirty_by(int begin, int end)
{
    int i;

    for (i=begin; i<end; i++) {
        __at_set_dirty(i);
    }
}

static inline void
at_set_dirty_all(void)
{
    at_set_dirty_by(0, AT_ENV_BLOCK_COUNT);
}

static inline int
at_ops_check(at_ops_t * ops, char *value)
{
    if (NULL == ops->write) {
        debug_error("no support write %s", ops->path);

        return -ENOSUPPORT;
    } else if (ops->check) {
        return (*ops->check)(ops, value);
    } else {
        return 0;
    }
}

static inline void
at_ops_show(at_ops_t * ops)
{
    if (ops->show) {
        (*ops->show)(ops);
    }
}

static inline void
at_ops_write(at_ops_t * ops, char *value)
{
    if (ops->write) {
        (*ops->write)(ops, value);

        at_set_dirty(ops);
    }
}

static inline at_cache_t *
at_cache(at_ops_t * ops)
{
    return &__at_cache[at_ops_idx(ops)];
}

#define at_cache_value(_ops)    at_cache(_ops)->value
#define at_cache_showit(_ops)   at_cache(_ops)->showit

static inline void
__at_ops_dump(at_ops_t * ops)
{
    debug_trace("ops idx=%d, path=%s, value=%s, showit=%s",
        at_ops_idx(ops),
        ops->path,
        at_cache_value(ops)?at_cache_value(ops):"nothing",
        at_cache_showit(ops)?"true":"false");
}

static inline at_firmware_t *
at_firmware(int idx)
{
    return &__at_firmware[idx];
}

static inline at_vcs_t *
at_kernel(int idx)
{
    return &at_firmware(idx)->kernel;
}

static inline at_vcs_t *
at_rootfs(int idx)
{
    return &at_firmware(idx)->rootfs;
}
#define at_obj(_obj, _idx)  at_##_obj(_idx)

static inline bool
__at_kernel_is_good(int idx)
{
    return is_good_atidx(idx)
        && at_vcs_is_good(at_kernel(idx));
}

static inline bool
__at_rootfs_is_good(int idx)
{
    return is_good_atidx(idx)
        && at_vcs_is_good(at_rootfs(idx));
}

static inline bool
__at_firmware_is_good(int idx)
{
    return __at_rootfs_is_good(idx)
        && __at_kernel_is_good(idx);
}
#define at_obj_is_good(_obj, _idx)  __at_##_obj##_is_good(_idx)

#define __at_kernel_is_bad(_idx)    (false==__at_kernel_is_good(_idx))
#define __at_rootfs_is_bad(_idx)    (false==__at_rootfs_is_good(_idx))
#define __at_firmware_is_bad(_idx)  (false==__at_firmware_is_good(_idx))
#define at_obj_is_bad(_obj, _idx)   (false==at_obj_is_good(_obj, _idx))

static inline at_version_t *
at_kernel_version(int idx)
{
    return &at_kernel(idx)->version;
}

static inline at_version_t *
at_rootfs_version(int idx)
{
    return &at_rootfs(idx)->version;
}
#define at_firmware_version(_idx)   at_rootfs_version(_idx)
#define at_obj_version(_obj, _idx)  at_##_obj##_version(_idx)

static inline char *
at_kernel_version_string(int idx)
{
    return at_version_itoa(at_kernel_version(idx));
}

static inline char *
at_rootfs_version_string(int idx)
{
    return at_version_itoa(at_rootfs_version(idx));
}
#define at_firmware_version_string(_idx)    at_rootfs_version_string(_idx)
#define at_obj_version_string(_obj, _idx)   at_##_obj##_version_string(_idx)

static inline int
__at_kernel_version_cmp(int a, int b)
{
    return at_version_cmp(at_kernel_version(a), at_kernel_version(b));
}

static inline int
__at_rootfs_version_cmp(int a, int b)
{
    return at_version_cmp(at_rootfs_version(a), at_rootfs_version(b));
}

static inline int
__at_firmware_version_cmp(int a, int b)
{
    int ret;

    ret = __at_rootfs_version_cmp(a, b);
    if (ret) {
        return ret;
    }

    ret = __at_kernel_version_cmp(a, b);
    if (ret) {
        return ret;
    }

    return 0;
}

#define at_obj_version_cmp(_obj, _a, _b)    __at_##_obj##_version_cmp(_a, _b)
#define at_obj_version_eq(_obj, _a, _b)     (0==at_obj_version_cmp(_obj, _a, _b))

static inline bool
is_at_cookie_deft(void)
{
    at_cookie_t deft = AT_DEFT_COOKIE;
    
    return os_objeq(&deft, __at_cookie);
}

static inline void
__at_deft_cookie(void)
{
    os_objdeft(__at_cookie, AT_DEFT_COOKIE);

    __at_set_dirty(AT_ENV_COOKIE);
}

static inline void
__at_deft_os(void)
{
    int i;

    for (i=0; i<AT_OS_COUNT; i++) {
        __at_firmware_deft(at_firmware(i));
    }
    __at_current = AT_OS_CURRENT;

    __at_set_dirty(AT_ENV_OS);
}

static inline void
__at_deft(void)
{
    os_objdeft(__at_env, AT_DEFT_ENV);

    at_set_dirty_all();
}

static inline void
__at_clean_cookie(void)
{
    os_objzero(__at_cookie);
    
    __at_set_dirty(AT_ENV_COOKIE);
    
    os_println("cookie clean");
}

static inline void
__at_clean_mark(void)
{
    unsigned int debug = at_mark(2);
    os_objzero(__at_mark);
    at_mark(2) = debug;

    __at_set_dirty(AT_ENV_MARK);
    
    os_println("mark clean");
}

static inline void
__at_clean_info(void)
{
    os_objzero(__at_info);
    
    at_set_dirty_by(AT_ENV_INFO, AT_ENV_BLOCK_COUNT);
    
    os_println("info clean");
}

static inline void
__at_clean(void)
{
    unsigned int debug = at_mark(2);
    os_objzero(__at_env);
    at_mark(2) = debug;
    
    at_set_dirty_all();
    
    os_println("all clean");
}

/*
* step 1:normal
* 1. firmware, bad < good
* 2. version, small < big
*
* step 2:exact
* 1. error, bad < good
* 2. upgrade, fail < unknow < ok
* 3. other, fail < unknow < ok
* 4. self, fail < unknow < ok
*/
static inline int
at_vcs_cmp(at_vcs_t * a, at_vcs_t * b)
{
    int ret;

    ret = !!(int)at_vcs_is_good(a) - !!(int)at_vcs_is_good(b);
    if (ret) {
        return ret;
    }

    ret = at_version_cmp(&a->version, &b->version);
    if (ret) {
        return ret;
    }
    
    ret = at_error_cmp(a->error, b->error);
    if (ret) {
        return ret;
    }

    ret = at_fsm_cmp(a->upgrade, b->upgrade);
    if (ret) {
        return ret;
    }

    ret = at_fsm_cmp(a->other, b->other);
    if (ret) {
        return ret;
    }

    ret = at_fsm_cmp(a->self, b->self);
    if (ret) {
        return ret;
    }

    return 0;
}

static inline int
at_kernel_cmp(int a, int b)
{
    return at_vcs_cmp(at_kernel(a), at_kernel(b));
}

static inline int
at_rootfs_cmp(int a, int b)
{
    return at_vcs_cmp(at_rootfs(a), at_rootfs(b));
}

/*
* 1. first,  cmp rootfs
* 2. second, cmp kernel
*/
static inline int
at_firmware_cmp(int a, int b)
{
    int ret;

    ret = at_rootfs_cmp(a, b);
    if (ret) {
        return ret;
    }

    ret = at_kernel_cmp(a, b);
    if (ret) {
        return ret;
    }

    return 0;
}

/*
* return sort idx
*/
#define at_maxmin(_sort, _count, _cmp, _sym)   ({  \
    int i, idx = 0;                     \
                                        \
    for (i = 1; i < _count; i++) {      \
        if (_cmp(_sort[i], _sort[idx]) _sym 0) { \
            idx = i;                    \
        }                               \
    }                                   \
                                        \
    idx;                                \
})  /* end */

static inline int
at_kernel_min(int sort[], int count)
{
    return at_maxmin(sort, count, at_kernel_cmp, <);
}

static inline int
at_kernel_max(int sort[], int count)
{
    return at_maxmin(sort, count, at_kernel_cmp, >);
}

static inline int
at_rootfs_min(int sort[], int count)
{
    return at_maxmin(sort, count, at_rootfs_cmp, <);
}

static inline int
at_rootfs_max(int sort[], int count)
{
    return at_maxmin(sort, count, at_rootfs_cmp, >);
}

static inline int
at_firmware_min(int sort[], int count)
{
    return at_maxmin(sort, count, at_firmware_cmp, <);
}

static inline int
at_firmware_max(int sort[], int count)
{
    return at_maxmin(sort, count, at_firmware_cmp, >);
}
#define at_obj_min(_obj)    at_##_obj##_min
#define at_obj_max(_obj)    at_##_obj##_max

static inline void
__at_sort(int sort[], int count, int (*maxmin)(int sort[], int count))
{
    if (count <= 1) {
        return;
    }
    
    int idx = (*maxmin)(sort, count);
    if (idx) {
        os_swap_value(sort[0], sort[idx]);
    }
    
    __at_sort(sort+1, count-1, maxmin);
}

#define __at_skips(_idx)            (os_bit(0) | os_bit(__at_current) | os_bit(_idx))
#define is_at_skip(_idx, _skips)    os_hasbit(_skips, _idx)

static inline int
at_first_idx(int current, int skips)
{
    int i;

    at_foreach(i) {
        if (false==is_at_skip(i, skips)) {
            return i;
        }
    }

    trace_assert(0, "no found first idx");
    
    return (1==current?2:1);
}

static inline int
__at_sort_count(int skips, int sort[], int size)
{
    int i, count = 0;

    for (i=0; i<size; i++) {
        if (false==is_at_skip(i, skips)) {
            sort[count++] = i;
        }
    }

    return count;
}
#define at_sort_count(_skips, _sort)    __at_sort_count(_skips, _sort, os_count_of(_sort))

#define AT_SORT_DUMP    1

static inline void
__at_sort_head_dump(char *name, int sort[], int count)
{
#if AT_SORT_DUMP
    int i;
    
    os_println("%s-sort index", name);
    for (i=0; i<count; i++) {
        os_printf("%-4d", i);
    }
    os_println(__empty);
#endif
}

static inline void
__at_sort_entry_dump(int sort[], int count)
{
#if AT_SORT_DUMP
    int i;
    
    for (i=0; i<count; i++) {
        os_printf("%-4d", sort[i]);
    }
    os_println(__empty);
#endif
}

static inline void
__at_sort_before(char *name, int sort[], int count)
{
    __at_sort_head_dump(name, sort, count);
    __at_sort_entry_dump(sort, count);
}

static inline void
__at_sort_after(int sort[], int count)
{
    __at_sort_entry_dump(sort, count);
}

#define __at_upsort(_obj, _sort, _count)    do{ \
    int __count = _count;                       \
    __at_sort_before("up", _sort, __count);     \
    __at_sort(_sort, __count, at_obj_min(_obj));\
    __at_sort_after(_sort, __count);            \
}while(0)

#define __at_downsort(_obj, _sort, _count)  do{ \
    int __count = _count;                       \
    __at_sort_before("down", _sort, __count);   \
    __at_sort(_sort, __count, at_obj_max(_obj));\
    __at_sort_after(_sort, __count);            \
}while(0)

#define at_upsort(_obj, _skips, _sort) \
        __at_upsort(_obj, _sort, at_sort_count(_skips, _sort))

#define at_downsort(_obj, _skips, _sort) \
        __at_downsort(_obj, _sort, at_sort_count(_skips, _sort))

/*
* skip 0 and current
*/
#define __at_find_xest(_obj, _skips, _sort_func, _is_func) ({ \
    int sort[AT_OS_COUNT] = {0};        \
    int i, idx, xest = -ENOEXIST, count;\
                                        \
    count = at_sort_count(_skips, sort);\
    _sort_func(_obj, sort, count);      \
                                        \
    for (i=0; i<count; i++) {           \
        idx = sort[i];                  \
                                        \
        if (_is_func(_obj, idx)) {      \
            xest = idx;                 \
                                        \
            break;                      \
        }                               \
    }                                   \
                                        \
    xest;                               \
}) /* end */
    
#define __at_obj_always_true(_obj, _idx)    true

#define at_find_best(_obj, _skips) ({   \
    int idx = __at_find_xest(_obj, _skips, __at_downsort, at_obj_is_good); \
    debug_trace("find best:%d", idx);   \
    idx;                                \
})  /* end */

#define at_find_worst(_obj, _skips) ({  \
    int idx = __at_find_xest(_obj, _skips, __at_upsort, __at_obj_always_true); \
    debug_trace("find worst:%d", idx);  \
    idx;                                \
})  /* end */


#if AT_UPGRADE_TWO==AT_UPGRADE
/*
* skip 0 and idx
*/
#define __at_find_first_buddy(_obj, _idx, _skips, _is_func) ({    \
    int i, ret = -ENOEXIST;             \
    int skips = (_skips) | os_bit(_idx);\
                                        \
    at_foreach(i) {                     \
        if (false==is_at_skip(i, skips) \
            && at_obj_version_eq(_obj, i, _idx) \
            && at_obj_is_good(_obj, i)) { \
            ret = i; break;             \
        }                               \
    }                                   \
                                        \
    ret;                                \
})  /* end */

#define at_find_first_buddy(_obj, _idx, _skips) \
        __at_find_first_buddy(_obj, _idx, _skips, __at_obj_always_true)

#define at_find_first_good_buddy(_obj, _idx, _skips) \
        __at_find_first_buddy(_obj, _idx, _skips, at_obj_is_good)

#define at_find_first_bad_buddy(_obj, _idx, _skips) \
        __at_find_first_buddy(_obj, _idx, _skips, at_obj_is_bad)
#endif /* AT_UPGRADE */

#define __at_find_first_byversion(_obj, _version, _skips, _is_func) ({ \
    int i, idx = -ENOEXIST; \
    int ____skips = _skips; \
    at_version_t *__version = _version; \
                            \
    at_foreach(i) {         \
        if (false==is_at_skip(i, ____skips) \
            && at_version_eq(__version, at_obj_version(_obj, i)) \
            && _is_func(_obj, i)) { \
            idx = i; break; \
        }                   \
    }                       \
                            \
    idx;                    \
})  /* end */

#define at_find_first_byversion(_obj, _version, _skips) \
        __at_find_first_byversion(_obj, _version, _skips, __at_obj_always_true)

#define at_find_first_good_byversion(_obj, _version, _skips) \
        __at_find_first_byversion(_obj, _version, _skips, at_obj_is_good)

#define at_find_first_bad_byversion(_obj, _version, _skips) \
        __at_find_first_byversion(_obj, _version, _skips, at_obj_is_bad)

static inline void
__at_show_header(at_ops_t * ops)
{
    if (__at_show_count > 1) {
        os_printf("%s=", ops->path);
    }
}

static inline void
__at_show_number(at_ops_t * ops)
{
    __at_show_header(ops);

    os_println("%u", *at_ops_obj(unsigned int, ops));
}

static inline void
__at_show_string(at_ops_t * ops)
{
    char *string = at_ops_obj(char, ops);

    if (string[0]) {
        __at_show_header(ops);

        os_println("%s", string);
    }
}

static inline void
__at_set_number(at_ops_t * ops, char *value)
{
    *at_ops_obj(unsigned int, ops) =
        (unsigned int)(value[0] ? os_atoi(value) : 0);
}

static inline void
__at_set_string(at_ops_t * ops, char *value)
{
    char *string = at_ops_obj(char, ops);

    if (value[0]) {
        os_strcpy(string, value);
    } else {
        string[0] = 0;
    }
}

static inline int
__at_check_version(at_ops_t * ops, char *value)
{
    at_version_t version = AT_INVALID_VERSION;

    if (at_version_atoi(&version, value)) {
        /*
         * when set version, must input value
         */
        return -EFORMAT;
    } else {
        return 0;
    }
}

static inline void
__at_show_version(at_ops_t * ops)
{
    __at_show_header(ops);

    os_println("%s", at_version_itoa(at_ops_obj(at_version_t, ops)));
}

static inline void
__at_set_version(at_ops_t * ops, char *value)
{
    at_version_t *v = at_ops_obj(at_version_t, ops);

    if (value[0]) {
        at_version_atoi(v, value);
    } else {
        at_version_t version = AT_DEFT_VERSION;

        os_objscpy(v, &version);
    }
}

static inline int
__at_check_fsm(at_ops_t * ops, char *value)
{
    if (0==value[0]) {
        /* 
         * clear fsm(to unknow)
         */
        return 0;
    } else if (is_good_at_fsm(at_fsm_idx(value))) {
        return 0;
    } else {
        debug_error("bad fsm:%s", value);

        /*
         * when set self/other/upgrade, must input value
         */
        return -EFORMAT;
    }
}

static inline void
__at_show_fsm(at_ops_t * ops)
{
    int fsm = *at_ops_obj(unsigned int, ops);

    __at_show_header(ops);

    os_println("%s", at_fsm_string(fsm));
}

static inline void
__at_set_fsm(at_ops_t * ops, char *value)
{
    unsigned int fsm;

    if (value[0]) {
        fsm = at_fsm_idx(value);
    } else {
        fsm = AT_FSM_UNKNOW;
    }

    
    *at_ops_obj(unsigned int, ops) = fsm;
}

static inline int
__at_check_current(at_ops_t * ops, char *value)
{
    char *end = NULL;
    int idx = os_strtol(value, &end, 0);
    
    if (0==value[0]) {
        /*
         * when set kernel/rootfs current, must input value
         */
        debug_error("set current failed, must input value");

        return -EFORMAT;
    }
    else if (false==os_strtonumber_is_good(end)) {
        debug_error("input invalid current:%s", value);
        
        return -EFORMAT;
    }
    else if (false==is_good_atidx(idx)) {
        debug_error("bad current %d", idx);

        return -EFORMAT;
    }
    else{
        return 0;
    }
}

static inline int
__at_check_string(at_ops_t * ops, char *value)
{
    int size = 0;

    switch(__at_ops_is(ops->offset)) {
        case AT_ENV_OS:
            size = AT_VCS_COOKIE_SIZE;
            break;
        case AT_ENV_INFO:
            size = AT_INFO_SIZE;
            break;
        case AT_ENV_COOKIE:
        case AT_ENV_MARK:
        default:
            /*
            * no support set string
            */
            break;
    }
    
    if (size && value[0]) {
        if (os_strlen(value) < size) {
            return 0;
        } else {
            debug_error("max string length %d", size);
            
            return -EFORMAT;
        }
    } else {
        /*
         * when set var string, may NOT input value
         */
        return 0;
    }
}

#define AT_OPS(_path, _member, _check, _write, _show) { \
    .path   = _path,    \
    .offset = offsetof(at_env_t, _member), \
    .check  = _check,   \
    .write  = _write,   \
    .show   = _show,    \
}   /* end */

#define __AT_COOKIE_OPS(_path, _member, _show) \
    AT_OPS("cookies/" _path, cookie._member, NULL, NULL, _show) \
    /* end */

#define AT_COOKIE_OPS \
    __AT_COOKIE_OPS("product/vendor",       \
        product.vendor,  __at_show_string), \
    __AT_COOKIE_OPS("product/company",      \
        product.company, __at_show_string), \
    __AT_COOKIE_OPS("pcba/model",           \
        pcba.model,      __at_show_string), \
    __AT_COOKIE_OPS("pcba/version",         \
        pcba.version,    __at_show_string)  \
    /* end */

#define __AT_OS_OPS(_path, _member, _check, _write, _show) \
    AT_OPS("os/" _path, os._member, _check, _write, _show) \
    /* end */

#define __AT_FIRMWARE_OPS(_obj, _idx)           \
    __AT_OS_OPS(#_obj "/" #_idx "/self",        \
        firmware[_idx]._obj.self,               \
        __at_check_fsm, __at_set_fsm, __at_show_fsm),   \
    __AT_OS_OPS(#_obj "/" #_idx "/other",       \
        firmware[_idx]._obj.other,              \
        __at_check_fsm, __at_set_fsm, __at_show_fsm),   \
    __AT_OS_OPS(#_obj "/" #_idx "/upgrade",     \
        firmware[_idx]._obj.upgrade,            \
        __at_check_fsm, __at_set_fsm, __at_show_fsm),   \
    __AT_OS_OPS(#_obj "/" #_idx "/error",       \
        firmware[_idx]._obj.error,              \
        NULL, __at_set_number, __at_show_number),       \
    __AT_OS_OPS(#_obj "/" #_idx "/version",     \
        firmware[_idx]._obj.version,            \
        __at_check_version, __at_set_version, __at_show_version), \
    __AT_OS_OPS(#_obj "/" #_idx "/cookie",      \
        firmware[_idx]._obj.cookie,             \
        __at_check_string, __at_set_string, __at_show_string) \
    /* end */

#define AT_FIRMWARE_OPS(_idx)       \
    __AT_FIRMWARE_OPS(kernel, _idx),    \
    __AT_FIRMWARE_OPS(rootfs, _idx)     \
    /* end */

#define AT_OS_COMMON_OPS            \
    __AT_OS_OPS("current", current, \
        __at_check_current, __at_set_number, __at_show_number) \
    /* end */

#if 1==AT_OS_COUNT
#define AT_OS_OPS       \
    AT_OS_COMMON_OPS,   \
    AT_FIRMWARE_OPS(0)  \
    /* end */

#elif 7==AT_OS_COUNT
#define AT_OS_OPS       \
    AT_OS_COMMON_OPS,   \
    AT_FIRMWARE_OPS(0), \
    AT_FIRMWARE_OPS(1), \
    AT_FIRMWARE_OPS(2), \
    AT_FIRMWARE_OPS(3), \
    AT_FIRMWARE_OPS(4), \
    AT_FIRMWARE_OPS(5), \
    AT_FIRMWARE_OPS(6)  \
    /* end */
#endif

#define __AT_MARK_OPS(_path, _idx, _check, _set, _show) \
    AT_OPS("marks/" _path, mark.key[_idx], _check, __at_set_number, __at_show_number)

#define __AT_MARK_OPS_IDX(_idx) \
    __AT_MARK_OPS(#_idx, _idx, NULL, __at_set_number, __at_show_number)

#define __AT_MARK_OPS_IDX_RO(_idx) \
    __AT_MARK_OPS(#_idx, _idx, NULL, NULL, __at_show_number)

enum {
    __at_mark_ptest_control = 0,
    __at_mark_ptest_result  = 1,
    __at_mark_debug         = 2,
    __ar_mark_uptimes       = 3,
    __at_mark_runtime       = 4,
    __at_mark_cid_mid       = 5,
    __at_mark_cid_psn       = 6,
};

static inline unsigned int *
at_mark_pointer(int idx)
{
    return &at_mark(idx);
}

static inline unsigned int
at_mark_get(int idx)
{
    return at_mark(idx);
}

static inline unsigned int
at_mark_set(int idx, unsigned int value)
{
    return (at_mark(idx) = value);
}

static inline unsigned int
at_mark_add(int idx, int value)
{
    return (at_mark(idx) += value);
}

#define AT_MARK_OPS_NAMES \
    __AT_MARK_OPS("ptest/control",  0, NULL, __at_set_number, __at_show_number), \
    __AT_MARK_OPS("ptest/result",   1, NULL, __at_set_number, __at_show_number)  \
    /* end */

#ifdef __BOOT__
#define AT_MARK_OPS_IDX   \
    __AT_MARK_OPS_IDX(2), \
    __AT_MARK_OPS_IDX_RO(3), \
    __AT_MARK_OPS_IDX_RO(4), \
    __AT_MARK_OPS_IDX_RO(5), \
    __AT_MARK_OPS_IDX_RO(6)  \
    /* end */

#elif defined(__APP__)

#define AT_MARK_OPS_IDX   \
    __AT_MARK_OPS_IDX(2), \
    __AT_MARK_OPS_IDX_RO(3), \
    __AT_MARK_OPS_IDX_RO(4), \
    __AT_MARK_OPS_IDX_RO(5), \
    __AT_MARK_OPS_IDX_RO(6), \
    __AT_MARK_OPS_IDX(7), \
    __AT_MARK_OPS_IDX(8), \
    __AT_MARK_OPS_IDX(9), \
                          \
    __AT_MARK_OPS_IDX(10), \
    __AT_MARK_OPS_IDX(11), \
    __AT_MARK_OPS_IDX(12), \
    __AT_MARK_OPS_IDX(13), \
    __AT_MARK_OPS_IDX(14), \
    __AT_MARK_OPS_IDX(15), \
    __AT_MARK_OPS_IDX(16), \
    __AT_MARK_OPS_IDX(17), \
    __AT_MARK_OPS_IDX(18), \
    __AT_MARK_OPS_IDX(19), \
                           \
    __AT_MARK_OPS_IDX(20), \
    __AT_MARK_OPS_IDX(21), \
    __AT_MARK_OPS_IDX(22), \
    __AT_MARK_OPS_IDX(23), \
    __AT_MARK_OPS_IDX(24), \
    __AT_MARK_OPS_IDX(25), \
    __AT_MARK_OPS_IDX(26), \
    __AT_MARK_OPS_IDX(27), \
    __AT_MARK_OPS_IDX(28), \
    __AT_MARK_OPS_IDX(29), \
                           \
    __AT_MARK_OPS_IDX(30), \
    __AT_MARK_OPS_IDX(31), \
    __AT_MARK_OPS_IDX(32), \
    __AT_MARK_OPS_IDX(33), \
    __AT_MARK_OPS_IDX(34), \
    __AT_MARK_OPS_IDX(35), \
    __AT_MARK_OPS_IDX(36), \
    __AT_MARK_OPS_IDX(37), \
    __AT_MARK_OPS_IDX(38), \
    __AT_MARK_OPS_IDX(39), \
    __AT_MARK_OPS_IDX(40), \
                           \
    __AT_MARK_OPS_IDX(41), \
    __AT_MARK_OPS_IDX(42), \
    __AT_MARK_OPS_IDX(43), \
    __AT_MARK_OPS_IDX(44), \
    __AT_MARK_OPS_IDX(45), \
    __AT_MARK_OPS_IDX(46), \
    __AT_MARK_OPS_IDX(47), \
    __AT_MARK_OPS_IDX(48), \
    __AT_MARK_OPS_IDX(49), \
                           \
    __AT_MARK_OPS_IDX(50), \
    __AT_MARK_OPS_IDX(51), \
    __AT_MARK_OPS_IDX(52), \
    __AT_MARK_OPS_IDX(53), \
    __AT_MARK_OPS_IDX(54), \
    __AT_MARK_OPS_IDX(55), \
    __AT_MARK_OPS_IDX(56), \
    __AT_MARK_OPS_IDX(57), \
    __AT_MARK_OPS_IDX(58), \
    __AT_MARK_OPS_IDX(59), \
                           \
    __AT_MARK_OPS_IDX(60), \
    __AT_MARK_OPS_IDX(61), \
    __AT_MARK_OPS_IDX(62), \
    __AT_MARK_OPS_IDX(63), \
    __AT_MARK_OPS_IDX(64), \
    __AT_MARK_OPS_IDX(65), \
    __AT_MARK_OPS_IDX(66), \
    __AT_MARK_OPS_IDX(67), \
    __AT_MARK_OPS_IDX(68), \
    __AT_MARK_OPS_IDX(69), \
                           \
    __AT_MARK_OPS_IDX(70), \
    __AT_MARK_OPS_IDX(71), \
    __AT_MARK_OPS_IDX(72), \
    __AT_MARK_OPS_IDX(73), \
    __AT_MARK_OPS_IDX(74), \
    __AT_MARK_OPS_IDX(75), \
    __AT_MARK_OPS_IDX(76), \
    __AT_MARK_OPS_IDX(77), \
    __AT_MARK_OPS_IDX(78), \
    __AT_MARK_OPS_IDX(79), \
                           \
    __AT_MARK_OPS_IDX(80), \
    __AT_MARK_OPS_IDX(81), \
    __AT_MARK_OPS_IDX(82), \
    __AT_MARK_OPS_IDX(83), \
    __AT_MARK_OPS_IDX(84), \
    __AT_MARK_OPS_IDX(85), \
    __AT_MARK_OPS_IDX(86), \
    __AT_MARK_OPS_IDX(87), \
    __AT_MARK_OPS_IDX(88), \
    __AT_MARK_OPS_IDX(89), \
                           \
    __AT_MARK_OPS_IDX(90), \
    __AT_MARK_OPS_IDX(91), \
    __AT_MARK_OPS_IDX(92), \
    __AT_MARK_OPS_IDX(93), \
    __AT_MARK_OPS_IDX(94), \
    __AT_MARK_OPS_IDX(95), \
    __AT_MARK_OPS_IDX(96), \
    __AT_MARK_OPS_IDX(97), \
    __AT_MARK_OPS_IDX(98), \
    __AT_MARK_OPS_IDX(99), \
                           \
    __AT_MARK_OPS_IDX(100), \
    __AT_MARK_OPS_IDX(101), \
    __AT_MARK_OPS_IDX(102), \
    __AT_MARK_OPS_IDX(103), \
    __AT_MARK_OPS_IDX(104), \
    __AT_MARK_OPS_IDX(105), \
    __AT_MARK_OPS_IDX(106), \
    __AT_MARK_OPS_IDX(107), \
    __AT_MARK_OPS_IDX(108), \
    __AT_MARK_OPS_IDX(109), \
                            \
    __AT_MARK_OPS_IDX(110), \
    __AT_MARK_OPS_IDX(111), \
    __AT_MARK_OPS_IDX(112), \
    __AT_MARK_OPS_IDX(113), \
    __AT_MARK_OPS_IDX(114), \
    __AT_MARK_OPS_IDX(115), \
    __AT_MARK_OPS_IDX(116), \
    __AT_MARK_OPS_IDX(117), \
    __AT_MARK_OPS_IDX(118), \
    __AT_MARK_OPS_IDX(119), \
                            \
    __AT_MARK_OPS_IDX(120), \
    __AT_MARK_OPS_IDX(121), \
    __AT_MARK_OPS_IDX(122), \
    __AT_MARK_OPS_IDX(123), \
    __AT_MARK_OPS_IDX(124), \
    __AT_MARK_OPS_IDX(125), \
    __AT_MARK_OPS_IDX(126), \
    __AT_MARK_OPS_IDX(127)  \
    /* end */
#endif

#define AT_MARK_OPS      \
    AT_MARK_OPS_NAMES,   \
    AT_MARK_OPS_IDX      \
    /* end */

#define __AT_INFO_OPS(_path, _idx) \
    AT_OPS("infos/" _path, info.var[_idx], __at_check_string, __at_set_string, __at_show_string) \
    /* end */

enum {
    __at_info_pcba_model        = 0,
    __at_info_pcba_version      = 1,
    __at_info_product_vendor    = 2,
    __at_info_product_company   = 3,
    __at_info_product_model     = 4,
    __at_info_product_mac       = 5,
    __at_info_product_sn        = 6,
    __at_info_product_lms       = 7,
    __at_info_product_version   = 8,

    __at_info_oem_vendor        = 10,
    __at_info_oem_company       = 11,
    __at_info_oem_model         = 12,
    __at_info_oem_mac           = 13,
    __at_info_oem_sn            = 14,
    __at_info_oem_lms           = 15,
    __at_info_oem_version       = 16,
    
    __at_info_idx_max           = 16,
    __at_info_block_named_count = (1 + __at_info_idx_max/AT_INFO_COUNT_PER_BLOCK),
};

static inline char *
at_info_get(int idx)
{
    return at_info(idx);
}

#define AT_INFO_OPS_NAMES_COMMON            \
    __AT_INFO_OPS("pcba/model",     0),     \
    __AT_INFO_OPS("pcba/version",   1),     \
    __AT_INFO_OPS("product/vendor", 2),     \
    __AT_INFO_OPS("product/company",3),     \
    __AT_INFO_OPS("product/model",  4),     \
    __AT_INFO_OPS("product/mac",    5),     \
    __AT_INFO_OPS("product/sn",     6),     \
    __AT_INFO_OPS("product/lms",    7),     \
    __AT_INFO_OPS("product/version",8),     \
                                            \
    __AT_INFO_OPS("oem/vendor",     10),    \
    __AT_INFO_OPS("oem/company",    11),    \
    __AT_INFO_OPS("oem/model",      12),    \
    __AT_INFO_OPS("oem/mac",        13),    \
    __AT_INFO_OPS("oem/sn",         14),    \
    __AT_INFO_OPS("oem/lms",        15),    \
    __AT_INFO_OPS("oem/version",    16)     \
    /* end */

#ifdef __BOOT__
#define AT_INFO_OPS_NAMES \
    AT_INFO_OPS_NAMES_COMMON \
    /* end */

#define AT_INFO_OPS \
    AT_INFO_OPS_NAMES \
    /* end */

#elif defined(__APP__)
#define AT_INFO_OPS_NAMES \
    AT_INFO_OPS_NAMES_COMMON \
    /* end */

#define __AT_INFO_OPS_IDX(_idx) \
    __AT_INFO_OPS(#_idx, _idx) \
    /* end */

#define AT_INFO_OPS_IDX     \
    __AT_INFO_OPS_IDX(9),   \
                            \
    __AT_INFO_OPS_IDX(17),  \
    __AT_INFO_OPS_IDX(18),  \
    __AT_INFO_OPS_IDX(19),  \
                            \
    __AT_INFO_OPS_IDX(20),  \
    __AT_INFO_OPS_IDX(21),  \
    __AT_INFO_OPS_IDX(22),  \
    __AT_INFO_OPS_IDX(23),  \
    __AT_INFO_OPS_IDX(24),  \
    __AT_INFO_OPS_IDX(25),  \
    __AT_INFO_OPS_IDX(26),  \
    __AT_INFO_OPS_IDX(27),  \
    __AT_INFO_OPS_IDX(28),  \
    __AT_INFO_OPS_IDX(29),  \
                            \
    __AT_INFO_OPS_IDX(30),  \
    __AT_INFO_OPS_IDX(31),  \
    __AT_INFO_OPS_IDX(32),  \
    __AT_INFO_OPS_IDX(33),  \
    __AT_INFO_OPS_IDX(34),  \
    __AT_INFO_OPS_IDX(35),  \
    __AT_INFO_OPS_IDX(36),  \
    __AT_INFO_OPS_IDX(37),  \
    __AT_INFO_OPS_IDX(38),  \
    __AT_INFO_OPS_IDX(39)   \
    /* end */

#define AT_INFO_OPS      \
    AT_INFO_OPS_NAMES,   \
    AT_INFO_OPS_IDX      \
    /* end */
#endif

#define AT_DEFT_OPS {   \
    AT_COOKIE_OPS,      \
    AT_OS_OPS,          \
    AT_MARK_OPS,        \
    AT_INFO_OPS,        \
}   /* end */

static inline void
__at_show_byprefix(char *prefix)
{
    int i;
    int len = prefix ? os_strlen(prefix) : 0;

    for (i = 0; i < __at_ops_count; i++) {
        at_ops_t *ops = at_ops(i);

        if (at_ops_match_wildcard(ops, prefix, len)) {
            at_ops_show(ops);
        }
    }
}

#define at_show_byprefix(_fmt, _args...) do{\
    char prefix[1+OS_LINE_LEN];             \
                                            \
    os_saprintf(prefix, _fmt, ##_args);     \
                                            \
    __at_show_byprefix(prefix);             \
}while(0)

static inline void
__at_handle_write(at_ops_t * ops)
{
    char *value = at_cache_value(ops);

    if (value) {
        at_ops_write(ops, value);
    }
}

static inline void
__at_handle_show(at_ops_t * ops)
{
    at_ops_show(ops);
}

static inline void
__at_handle(at_ops_t * ops)
{
    /*
     * show
     */
    if (at_cache_showit(ops)) {
        __at_handle_show(ops);
    }
    /*
     * wirite
     */
    else if (at_cache_value(ops)) {
        __at_handle_write(ops);
    }
}

static inline void
at_handle(int argc, char *argv[])
{
    int i;

    for (i = 0; i < __at_ops_count; i++) {
        __at_handle(at_ops(i));
    }
}

static inline int
at_usage(void)
{
#if USE_USAGE
    os_fprintln(stderr, "%s name ==> show env by name", __at_self);
    os_fprintln(stderr, "%s name1=value1 name2=value2 ... ==> set env by name and value", __at_self);
#endif

    return -EHELP;
}

static inline int
__at_analysis_write(at_ops_t * ops, char *args)
{
    char *path = args;
    char *eq = os_strchr(args, '=');
    char *value = eq + 1;
    int err;

    if (at_ops_match(ops, path, eq - path, false)) {
        err = at_ops_check(ops, value);
        if (err) {
            return err;
        }

        at_cache_value(ops) = value;
    }

    return 0;
}

static inline int
at_analysis_write(char *args)
{
    int i, err = 0;

    for (i = 0; i < __at_ops_count; i++) {
        err = __at_analysis_write(at_ops(i), args);
        if (err) {
            return err;
        }
    }

    return 0;
}

static inline int
__at_analysis_show(at_ops_t * ops, char *args)
{
    char *wildcard = os_strlast(args, '*');
    
    /*
     * if found '*'
     *   first '*' is not last '*'
     * if NOT found '*'
     *   but found last '*'
     */
    if (os_strchr(args, '*') != wildcard) {
        debug_error("only support show 'xxx*'");

        return -EFORMAT;
    } else if (at_ops_match(ops, args, os_strlen(args), !!wildcard)) {
        at_cache_showit(ops) = true;

        __at_show_count++;
    }

    return 0;
}

static inline int
at_analysis_show(char *args)
{
    int i, err = 0;

    for (i = 0; i < __at_ops_count; i++) {
        err = __at_analysis_show(at_ops(i), args);
        if (err) {
            return err;
        }
    }

    return 0;
}

static inline int
__at_analysis(char *args)
{
    /*
     * first is '=', bad format
     */
    if ('=' == args[0]) {
        debug_error("first is '='");

        return -EFORMAT;
    }
    /*
     * first is '*', bad format
     */
    else if ('*' == args[0]) {
        debug_error("first is '*'");

        return -EFORMAT;
    }
    /*
     * found '=', is wirte
     */
    else if (os_strchr(args, '=')) {
        return at_analysis_write(args);
    }
    /*
     * no found '=', is show
     */
    else {
        return at_analysis_show(args);
    }
}

static inline int
at_analysis(int argc, char *argv[])
{
    int i, err;

    for (i = 0; i < argc; i++) {
        err = __at_analysis(argv[i]);
        if (err) {
            return err;
        }
    }

    return 0;
}

static inline void
at_ops_dump(char *name)
{
    int i;

    debug_trace("[%s] ops count=%d", name, __at_ops_count);
    
    for (i = 0; i < __at_ops_count; i++) {
        __at_ops_dump(at_ops(i));
    }
}

static inline int
at_command(int argc, char *argv[])
{
    int err;

    if (0==argc) {
        return at_usage();
    }

    err = at_analysis(argc, argv);
    if (err) {
        return err;
    }

    at_ops_dump("command");

    at_handle(argc, argv);

    return 0;
}

#ifdef __BOOT__
extern int 
at_emmc_read(unsigned int begin, void *buf, int size);

extern int 
at_emmc_write(unsigned int begin, void *buf, int size);

#define at_open()       0
#define at_close()      0

extern int __at_load(int idx);    /* idx is atenv's block */
extern int __at_save(int idx);    /* idx is atenv's block */
#elif defined(__APP__)
static inline int
at_open(void)
{
    __at_fd = __os_lock_file(AT_DEV_BOOTENV, O_RDWR, 0, true);

    return is_good_fd(__at_fd) ? 0 : __at_fd;
}

static inline int
at_close(void)
{
    return os_unlock_file(AT_DEV_BOOTENV, __at_fd);
}

static inline int
__at_load(int idx)
{
    int offset = AT_ENV_BLOCK_SIZE * idx;
    void *obj = (char *)__at_env + offset;
    
    int err;
    
    err = lseek(__at_fd, offset, SEEK_SET);
    if (err < 0) {        /* <0 is error */
        debug_error("seek atenv error:%d", errno);

        return err;
    }

    if (AT_ENV_BLOCK_SIZE != read(__at_fd, obj, AT_ENV_BLOCK_SIZE)) {
        debug_error("read atenv error:%d", errno);

        return -errno;
    }

    return 0;
}

static inline int
__at_save(int idx /* atenv's block */ )
{
    int offset = AT_ENV_BLOCK_SIZE * idx;
    void *obj = (char *)__at_env + offset;
    int err;

    err = lseek(__at_fd, offset, SEEK_SET);
    if (err < 0) {        /* <0 is error */
        debug_error("seek atenv error:%d", errno);

        return -errno;
    }

    if (AT_ENV_BLOCK_SIZE != write(__at_fd, obj, AT_ENV_BLOCK_SIZE)) {
        debug_error("write atenv error:%d", errno);

        return -errno;
    }

    return 0;
}
#endif

static inline int
__at_nothing(int idx /* atenv's block */ )
{
    return 0;
}

static inline int
__at_load_by(int begin, int count)
{
    int i, err = 0;

    for (i = begin; i < count; i++) {
        err = __at_load(i);
            debug_ok_error(err, "load atenv block %d", i);
        if (err) {
            /* todo: log */
        }
    }
    
    return err;
}

static inline int
at_load_cookie(void)
{
    return __at_load(AT_ENV_COOKIE);
}

static inline int
at_load_os(void)
{
    return __at_load(AT_ENV_OS);
}

static inline int
at_load_mark(void)
{
    return __at_load(AT_ENV_MARK);
}

static inline int
at_load_info(void)
{
    return __at_load_by(AT_ENV_INFO, AT_ENV_BLOCK_COUNT);
}

static inline int
at_load_info_named(void)
{
    return __at_load_by(AT_ENV_INFO, AT_ENV_INFO + __at_info_block_named_count);
}

static inline int
at_load(void)
{
    return __at_load_by(0, AT_ENV_BLOCK_COUNT);
}

static inline int
at_save(void)
{
    int i, err = 0;
    int begin = __at_dirty[0]?0:1;

    /* 
     * block 0 is read only
     */
    for (i = begin; i < AT_ENV_BLOCK_COUNT; i++) {
        if (__at_dirty[i]) {
            err = __at_save(i);
                debug_ok_error(err, "save atenv block %d", i);
            if (err) {
                /* todo: log */
            }

            __at_dirty[i] = false;
        }
    }

    return err;
}

static inline int
__at_init(void)
{
    int err;

    BUILD_BUG_ON(AT_COOKIE_SIZE     > AT_ENV_BLOCK_SIZE);
    BUILD_BUG_ON(AT_OS_SIZE         > AT_ENV_BLOCK_SIZE);
    BUILD_BUG_ON(__at_info_idx_max  >= AT_INFO_COUNT);
    
    err = os_init();
    if (err) {
        return err;
    }
    
    __at_show_count = 0;
    if (__at_cache) {
        os_memzero(__at_cache, __at_ops_count * sizeof(at_cache_t));
    }

    os_arrayzero(__at_dirty);

    err = at_open();
        debug_ok_error(err, "open atenv");
    if (err) {
        return err;
    }

    err = at_load();
        debug_ok_error(err, "load atenv");
    if (err) {
        return err;
    }

    return 0;
}

static inline int
__at_fini(void)
{
    int err;
    
    err = at_save();
        debug_ok_error(err, "save atenv");
        
    err = at_close();
        debug_ok_error(err, "close atenv");

    return err;
}

static inline void
__at_show_cookie(void)
{
    __at_cookie_show(__at_cookie);
}

static inline void
__at_show_os(void)
{
    __at_os_show(__at_os);
}

static inline bool
__at_cmd(int argc, char *argv[])
{
#define __at_cmd_item(_action, _obj, _handle) {.action = _action, .obj = _obj, .handle = _handle}
    static struct {
        char *action;
        char *obj;
        void (*handle)(void);
    } cmd[] = {
        __at_cmd_item("show",   "cookie",   __at_show_cookie),
        __at_cmd_item("show",   "os",       __at_show_os),
        __at_cmd_item("reset",  "os",       __at_deft_os),
        __at_cmd_item("reset",  "all",      __at_deft),
        __at_cmd_item("clean",  "cookie",   __at_clean_cookie),
        __at_cmd_item("clean",  "mark",     __at_clean_mark),
        __at_cmd_item("clean",  "info",     __at_clean_info),
        __at_cmd_item("clean",  "all",      __at_clean),
    };
#undef __at_cmd_item
    char *action    = argv[1];
    char *obj       = argv[2];
    int i;

    if (3==argc) {
        for (i=0; i<os_count_of(cmd); i++) {
            if (0==os_strcmp(action, cmd[i].action) && 0==os_strcmp(obj, cmd[i].obj)) {
                (*cmd[i].handle)();
                
                return true;
            }
        }
    }
    
    return false;
}

static inline int
__at_main(int argc, char *argv[])
{
    __at_self = os_cmd_trace_argv(argc, argv);
    
    if (true==__at_cmd(argc, argv)) {
        return 0;
    }

    return at_command(argc - 1, argv + 1);
}

static inline int
at_main(int argc, char *argv[])
{
    return os_call(__at_init, __at_fini, __at_main, argc, argv);
}

static inline void *
at_get(char *path)
{
    int i;

    for (i = 0; i < __at_ops_count; i++) {
        at_ops_t *ops = at_ops(i);

        if (0==os_strcmp(path, ops->path)) {
            return __at_ops_obj(ops);
        }
    }

    return NULL;
}
/******************************************************************************/
#endif /* __ATENV_H_31502FF802B81D17AEDABEB5EDB5C121__ */
