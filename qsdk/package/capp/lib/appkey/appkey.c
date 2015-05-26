/*******************************************************************************
Copyright (c) 2012-2015, Autelan Networks. All rights reserved.
********************************************************************************
    manage app key
*******************************************************************************/
#ifndef __THIS_APP
#define __THIS_APP      libappkey
#endif

#ifndef __THIS_NAME
#define __THIS_NAME     "libappkey"
#endif

#ifndef __AKID_DEBUG
#define __AKID_DEBUG    __libappkey_debugger
#endif

extern unsigned int __AKID_DEBUG;
static unsigned int *__debug_level_pointer = &__AKID_DEBUG;

#define __LIBAPPKEY__
#include "utils.h"

unsigned int __AKID_DEBUG = __debug_level_default;
/******************************************************************************/
#ifndef APPKEY_PATH
#define APPKEY_PATH         "/tmp/appkey"
#endif

#ifndef APPKEY_PATH_DEFT
#define APPKEY_PATH_DEFT    "/etc/appkey"
#endif

#define INVALID_APPKEY      0

struct appkey {
    char app[1+__APP_NAMELEN];
    char key[1+__APP_KEY_NAMELEN];
    unsigned int value;
};

struct appkey_hdr {
    unsigned int protect;
    bool inited;
    int count;
    int limit;
    struct appkey keys[0];
};

union appkey_u {
    appkey_t akid;

    struct {
        unsigned short idx;
        unsigned short offset;
    } s;
};

#define AK_MAKE(_idx, _offset)              \
{                                           \
    .s = {                                  \
        .idx = (unsigned short)(_idx),      \
        .offset = (unsigned short)(_offset),\
    }                                       \
}

#define AK_INIT(_akid)   { .akid = _akid }

static inline appkey_t
ak_make(int idx, int offset)
{
    union appkey_u ak = AK_MAKE(idx, offset);

    return ak.akid;
}

static inline int
ak_idx(appkey_t akid)
{
    union appkey_u ak = AK_INIT(akid);

    return (int)ak.s.idx;
}

static inline int
ak_offset(appkey_t akid)
{
    union appkey_u ak = AK_INIT(akid);

    return (int)ak.s.offset;
}

#ifndef APPKEY_LIMIT
#define APPKEY_LIMIT    1023
#endif

#define appkey_shm_size     (0  \
    + sizeof(struct appkey_hdr) \
    + sizeof(struct appkey) * APPKEY_LIMIT \
    + sizeof(unsigned int)      \
)   /* end */

enum {
    AK_SYS_DEBUG,

    AK_SYS_END
};

static struct {
    os_shm_t shm;
    char *syskey[AK_SYS_END];
} __ak = {
    .shm = OS_SHM_INITER(OS_APPKEY_SHM_ID),
    .syskey = {
        [AK_SYS_DEBUG]  = AKID_DEBUG_NAME,
    },
};

static inline int 
syskey(char *key)
{
    int i;

    for (i=0; i<AK_SYS_END; i++) {
        if (0==os_strcmp(key, __ak.syskey[i])) {
            return i;
        }
    }

    return -ENOEXIST;
}

static inline struct appkey_hdr *
__hdr(void)
{
    return (struct appkey_hdr *)(__ak.shm.address);
}

#define ak_inited           __hdr()->inited
#define ak_count            __hdr()->count
#define ak_limit            __hdr()->limit
#define ak_key(_idx)        (&__hdr()->keys[_idx])
#define ak_0                ak_key(0)
#define ak_end              ak_key(ak_count)
#define ak_END              ak_key(ak_limit)
#define ak_foreach(ak)      for (ak=ak_0; ak<ak_end; ak++)
#define ak_protect_0        __hdr()->protect
#define ak_protect_1        (*(unsigned int *)ak_END)

static inline int 
getidx(struct appkey *key)
{
    return key - ak_0;
}

static inline int 
getoffset(struct appkey *key)
{
    return (char *)key - (char *)__hdr();
}

static inline struct appkey *
getbyid(appkey_t akid)
{
    char *address = (char *)__hdr() + ak_offset(akid);
    struct appkey *key = NULL;

    if (false==is_good_enum(ak_idx(akid), ak_count)) {
        return NULL;
    }
    
    key = ak_key(ak_idx(akid));

    return (address==(char *)key)?key:NULL;
}

static inline struct appkey *
getbyname(char *app, char *key)
{
    struct appkey *ak = NULL;
    
    ak_foreach(ak) {
        if (0==os_stracmp(ak->app, app) && 0==os_stracmp(ak->key, key)) {
            
            return ak;
        }
    }
    
    return NULL;
}

static struct appkey *
ak_new(char *app, char *key)
{
    if (ak_count < ak_limit) {
        int idx = ak_count++;
        struct appkey *ak = ak_key(idx);
        
        os_strdcpy(ak->app, app);
        os_strdcpy(ak->key, key);
        
        return ak;
    } else {
        debug_error("count(%u) >= limit(%u)", ak_count, ak_limit);
        
        return NULL;
    }
}

appkey_t 
__appkey_getbyname(char *app, char *key)
{
    struct appkey *ak;

    if (NULL==app) {
        return error_assertV(INVALID_APPKEY, "$app is nil");
    }
    else if (NULL==key) {
        return error_assertV(INVALID_APPKEY, "$key is nil");
    }

    ak = getbyname(app, key);

    return ak?ak_make(getidx(ak), getoffset(ak)):INVALID_APPKEY;
}

int 
__appkey_get(appkey_t akid, unsigned int *pvalue)
{
    struct appkey *ak = getbyid(akid);

    if (NULL==pvalue) {
        return -EKEYNULL;
    }
    else if (NULL==ak) {
        return -ENOEXIST;
    }
    
    *pvalue = ak->value;
    
    return 0;
}

int 
appkey_set(appkey_t akid, unsigned int value)
{
    struct appkey *ak = getbyid(akid);

    if (NULL==ak) {
        return -ENOEXIST;
    }
    
    ak->value = value;
    
    return 0;
}

struct akinfo {
    char *filename;
    char *line;
    char app[1+OS_LINE_LEN];
    char key[1+OS_LINE_LEN];
    char var[1+OS_LINE_LEN];
    unsigned int value;
};
#define AKINFO_INITER(_filename, _line) {  \
    .filename = _filename, \
    .line = _line, \
}

static int
syskey_debug(struct akinfo *info)
{
    char var[1+OS_LINE_LEN] = {0};
    char *name;
    unsigned int value = 0;

    os_strdcpy(var, info->var);
    os_strtok_foreach(name, var, "|") {
        if ('*'==name[0]) {
            value = __debug_level_all;
        }
        else if ('-'==name[0]) {
            value &= ~ __get_debug_level_byname(name+1);
        }
        else {
            value |= __get_debug_level_byname(name);
        }
    }

    if (0==os_stracmp(info->app, __THIS_NAME)) {
        /*
        * reload self(libappkey) debug key
        */
        __AKID_DEBUG = value;
    }

    info->value = value;
    
    return 0;
}

static int
syskey_value(int sys, struct akinfo *info)
{
    /*
    * try "*"
    */
    if ('*'==info->var[0] && 0==info->var[1]) {
        info->value = 0xffffffff;
        
        return 0;
    }
    
    switch(sys) {
        case AK_SYS_DEBUG:
            return syskey_debug(info);
        default:
            return -EKEYBAD;
    }
}

static int
load_line_app(struct akinfo *info)
{
    int len;

    /*
    * filename's format is "xxx.key"
    *   cannot use os_sscanf(filename, "%s.key", app)
    */
    if (1!=os_sscanf(info->filename, "%[^.]", info->app)) {
        debug_error("read file(%s) bad line(%s)", info->filename, info->line);
        
        return -EFORMAT;
    }
    
    len = os_strlen(info->app);
    if (len >= __APP_NAMELEN) {
        debug_error("app(%s) length(%d) > %d", info->app, len, __APP_NAMELEN);
        
        return -ETOOBIG;
    }

    return 0;
}

static int
load_line_kv(struct akinfo *info)
{
    int len;
    
    /* 
    * read key & var 
    */
    if (2!=os_sscanf(info->line, "%s %s", info->key, info->var)) {
        debug_error("read app(%s) bad line(%s)", info->app, info->line);
        
        return -EFORMAT;
    }
    len = os_strlen(info->key);
    if (len >= __APP_KEY_NAMELEN) {
        debug_error("key(%s) length(%d) > %d", info->key, len, __APP_KEY_NAMELEN);
        
        return -ETOOBIG;
    }

    return 0;
}

static int
load_line_value(struct akinfo *info)
{
    int sys = -1;
    char *end = NULL;
    unsigned int value = 0;
    
    /*
    * first, try var as digit string
    */
    value = strtoul(info->var, &end, 0);
    if (NULL==end || 0==end[0]) {
        info->value = value;
        
        return 0;
    }
    
    /*
    * not digit string, try syskey
    */
    sys = syskey(info->key);
    if (sys<0) {
        return -EFORMAT;
    } else {
        return syskey_value(sys, info);
    }
}

static multi_value_t 
load_line(char *filename/* not include path */, char *line, void *__no_used)
{
    struct akinfo info = AKINFO_INITER(filename, line);
    struct appkey *ak;
    int err;

    debug_trace("load file(%s) line(%s)", filename, line);
    
    err = load_line_app(&info);
    if (err) {
        return mv2_GO(err);
    }

    err = load_line_kv(&info);
    if (err) {
        return mv2_GO(err);
    }

    err = load_line_value(&info);
    if (err) {
        return mv2_GO(err);
    }
    
    ak = getbyname(info.app, info.key);
    if (NULL==ak) {
        ak = ak_new(info.app, info.key);
        if (NULL==ak) {
            debug_error("limit(%d)", ak_limit);
            
            return mv2_BREAK(-ELIMIT);
        }
    }

    debug_trace("load %s.%s(0x%x==>0x%x)",
        ak->app,
        ak->key,
        ak->value,
        info.value);
    
    ak->value = info.value;
    
    return mv2_OK;
}

/*
* just handle file "*.key"
*/
static bool 
filefilter(char *path, char *filename, void *control)
{
    char *p = os_strchr(filename, '.');
    
    if (p && 0==os_strcmp(p, ".key")) {
        return false;
    } else {
        debug_trace("ignore %s", filename);
        
        return true;
    }
}

static char *
get_path(void)
{
    DIR *dir = opendir(APPKEY_PATH);
    if (dir) {
        closedir(dir);

        return APPKEY_PATH;
    } else {
        return APPKEY_PATH_DEFT;
    }
}

static int 
load(void) 
{
    int ret = 0;
    char *path = get_path();
    
    ret = os_sfscan_dir(path, false, filefilter, load_line, NULL);
    if (ret<0) {
        debug_error("load(%s) error%d", path, ret);
        
        return ret;
    }
    
    debug_trace("load(%s) appkey count=%u, limit=%u",
        path,
        ak_count,
        ak_limit);
    
    return 0;
}


static void 
show(void) 
{
    int i;
    
    for (i=0; i<ak_count; i++) {
        struct appkey *key = ak_key(i);
        
        os_println("reload %s.%s=%u",
            key->app,
            key->key,
            key->value);
    }
}

int 
appkey_reload(void)
{
    load();
    show();
    
    return 0;
}

static int 
init(unsigned int limit)
{
    if (false==ak_inited) {
        ak_inited   = true;
        ak_limit    = limit;
        ak_protect_0 = OS_PROTECTED;
        ak_protect_1 = OS_PROTECTED;
        
        load();
    }
    
    return 0;
}

int 
appkey_fini(void) 
{
    if (INVALID_SHM_ADDR != __ak.shm.address) {
        shmdt(__ak.shm.address);
        __ak.shm.address = INVALID_SHM_ADDR;
        
        debug_trace("shm fini shmdt(address:%p)", __ak.shm.address);
    }

    return 0;
}

int 
__appkey_init(void) 
{
    int err = 0;

    err = os_shm_create(&__ak.shm, appkey_shm_size, false);
    if (err<0) { /* >=0 is valid shm id */
        goto error;
    }
    
    init(APPKEY_LIMIT);

    debug_ok("init OK!");
    
    return 0;
error:
    debug_error("init failed!");
    
    appkey_fini();

    return err;
}
/******************************************************************************/
