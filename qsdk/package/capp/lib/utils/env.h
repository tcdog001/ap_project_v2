#ifndef __ENV_H_AE12157DFFE5C6220F9C55AA8D427F7__
#define __ENV_H_AE12157DFFE5C6220F9C55AA8D427F7__
/******************************************************************************/
#ifndef ENV_DEBUG
#define ENV_DEBUG   0
#endif

#if ENV_DEBUG
#define __env_debug(_fmt, _args...)     os_println(_fmt, ##_args)
#else
#define __env_debug(_fmt, _args...)     do{}while(0)
#endif

#define os_getenv(_name)    getenv(_name)

#define is_good_env(_env)   is_good_string(_env)

static inline char *
get_string_env(char *envname, char *deft) 
{
    char *env;
    
    if (NULL==envname) {
        return os_assertV(NULL);
    }
    
    env = os_getenv(envname);
    if (false==is_good_env(env)) {
        __env_debug("no-found env:%s", envname);
        
        return deft;
    } else {
        __env_debug("get env:%s=%s", envname, env);
        
        return env;
    }
}

static inline int
__copy_string_env(char *envname, char *deft, char string[], int size) 
{
    char *env = get_string_env(envname, deft);
    if (NULL==env) {
        return 0;
    }
    else if (os_strlen(env) > size - 1) {
        return os_assertV(-ETOOBIG);
    }
    
    os_strlcpy(string, env, size);
    
    return 0;
}

#define copy_string_env(_envname, _deft, _string) \
    __copy_string_env(_envname, _deft, _string, sizeof(_string))

static inline int
get_int_env(char *envname, int deft) 
{
    char *env;
    
    if (NULL==envname) {
        return os_assertV(-EINVAL9);
    }
    
    env = os_getenv(envname);
    if (false==is_good_env(env)) {
        __env_debug("no-found env:%s, use default:%d", envname, deft);
        
        return deft;
    } else {
        int value = os_atoi(env);

        __env_debug("get env:%s=%d", envname, value);

        return value;
    }
}

typedef struct {
    char *name;
    char *deft;
    char value[1+OS_LINE_LEN];
} env_string_t;

typedef struct {
    char *name;
    int deft;
    int value;
} env_number_t;

#define env_var_initer(_name, _deft) { \
    .name = _name, \
    .deft = _deft, \
    .value= _deft, \
}   /* end */

static inline int
env_string_init(env_string_t *env)
{
    return copy_string_env(env->name, env->deft, env->value);
}

static inline void
env_number_init(env_number_t *env)
{
    env->value = get_int_env(env->name, env->deft);
}
/******************************************************************************/
#endif /* __ENV_H_AE12157DFFE5C6220F9C55AA8D427F7__ */
