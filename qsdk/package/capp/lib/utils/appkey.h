#ifndef __APPKEY_H_4DCB6A57D69734A56298406D6DF398AA__
#define __APPKEY_H_4DCB6A57D69734A56298406D6DF398AA__
/******************************************************************************/
typedef unsigned int appkey_t;

/*
* enable app debug(use share memory)
*/
#ifdef __APP__
extern appkey_t 
__appkey_getbyname(char *app, char *key);

#define appkey_getbyname(_key)  __appkey_getbyname(__THIS_NAME, _key)

extern int 
__appkey_get(appkey_t akid, unsigned int *pvalue);

static inline unsigned int
appkey_get(appkey_t akid, unsigned int deft)
{
    unsigned int value = deft;

    __appkey_get(akid, &value);

    return value;
}

extern int 
appkey_set(appkey_t akid, unsigned int value);

extern int
appkey_reload(void);

extern int
__appkey_init(void);

extern int
appkey_fini(void);

#define AKID_DEBUG_NAME     "debug"

#define appkey_init()       ({  \
    int err = __appkey_init();  \
    __AKID_DEBUG = appkey_getbyname(AKID_DEBUG_NAME); \
    err;                        \
})

#define DECLARE_DEBUGGER    appkey_t __AKID_DEBUG
#else
/*
* kernel/boot
*/
#define appkey_reload()                 0
#define appkey_getbyname(_key)          0
#define appkey_get(_akid, _deft)        (_deft)
#define appkey_set(_akid, _value)       0

#define appkey_init()                   0
#define appkey_fini()                   0

#define DECLARE_DEBUGGER    extern unsigned int __AKID_DEBUG
#endif /* __APP__ */

typedef struct {
    appkey_t id;
    char *key;
    unsigned int deft;
} appkey_var_t;

#define APPKEY_VAR_INITER(_key, _deft)  {.id = INVALID_VALUE, .key = _key, .deft = _deft}

static inline int
appkey_var_init(appkey_var_t *var)
{
    if (INVALID_VALUE==var->id) {
        var->id = appkey_getbyname(var->key);

        return 0;
    } else {
        return -EREINIT;
    }
}

static inline unsigned int
appkey_var_get(appkey_var_t *var)
{
    return appkey_get(var->id, var->deft);
}

static inline int 
appkey_var_set(appkey_var_t *var, unsigned int value)
{
    return appkey_set(var->id, value);
}
/******************************************************************************/
#endif /* __APPKEY_H_4DCB6A57D69734A56298406D6DF398AA__ */
