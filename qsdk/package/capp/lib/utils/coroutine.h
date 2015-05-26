#ifndef __COROUTINE_H_5D324594B45198CC07E58DDC22E1CBFC__
#define __COROUTINE_H_5D324594B45198CC07E58DDC22E1CBFC__
/******************************************************************************/
#ifdef __APP__
#ifndef CO_STACK_DEFAULT
#define CO_STACK_DEFAULT        (1024*8)
#endif

#ifndef CO_MAIL_DEFAULT
#define CO_MAIL_DEFAULT         32
#endif

#ifndef CO_SUSPEND_TIMEOUT
#define CO_SUSPEND_TIMEOUT      2000//ms
#endif

#ifndef CO_NAME_LEN
#define CO_NAME_LEN             31
#endif

#ifndef CO_IDLE_TIME
#define CO_IDLE_TIME            1000 /* 1000ns==1ms, for usleep */
#endif

#ifndef CO_IDLE_NAME 
#define CO_IDLE_NAME            "idle"
#endif

#ifndef CO_NOEXIST_NAME
#define CO_NOEXIST_NAME         "noexist"
#endif

#ifndef CO_MAIL_POINTER_COUNT
#define CO_MAIL_POINTER_COUNT   4
#endif

#if 0
TYPE        LP32    ILP32   LP64    ILP64   LLP64
CHAR        8       8       8       8       8
SHORT       16      16      16      16      16
INT         16      32      32      64      32
LONG        32      32      64      64      32
LONG LONG   64      64      64      64      64
POINTER     32      32      64      64      64       
#endif

enum co_pri {
    CO_PRI_HIGHEST  = 0,
    CO_PRI_NORMAL   = 8,
    CO_PRI_LOWEST   = 15,

    CO_PRI_DEFAULT  = CO_PRI_NORMAL,
    
    CO_PRI_END
};

enum co_event {
    /*
    * write: user
    * read : user
    */
    CO_EV_READABLE,

    /*
    * write: system
    * read : user
    */
    CO_EV_KILL,
    CO_EV_MAIL,

    /*
    * write: system
    * read : system
    */
    CO_EV_SUSPEND_TIMEOUT,  /* timeout */

    CO_EV_END
};

enum co_fsm {
    CO_READY,
    CO_SUSPEND,
    CO_RUNNING,
    CO_DEAD,
    
    CO_END
};

union co_mail {
    void *P[CO_MAIL_POINTER_COUNT];
     uint8_t    B[CO_MAIL_POINTER_COUNT*sizeof(void*)/sizeof( uint8_t)];
    uint16_t    S[CO_MAIL_POINTER_COUNT*sizeof(void*)/sizeof(uint16_t)];
    uint32_t    I[CO_MAIL_POINTER_COUNT*sizeof(void*)/sizeof(uint32_t)];
    uint64_t    L[CO_MAIL_POINTER_COUNT*sizeof(void*)/sizeof(uint64_t)];
};

typedef uint32_t co_id_t;
typedef uint64_t co_cred_t;

#if OS64
typedef uint32_t co_mask_t;
#else
typedef uint64_t co_mask_t;
#endif

typedef int co_main_f(void *data);

typedef struct {
    uint32_t    protectedH; /* first */
    char name[1+CO_NAME_LEN];
    ucontext_t  ctx;
    co_id_t id;
    co_cred_t cred;
    
    struct {
        struct list_head node;
        enum co_fsm state;
    } fsm;
    
    struct {
        uint64_t fsm[CO_END];
    } st;
    
    struct {
        co_main_f *func;
        void *data;
    } main;

    struct {
        enum co_pri old;
        enum co_pri now;
    } pri;

    uint32_t *protectedT;
    
    struct {
        int size;
        char *sp;
    } stack;

    struct {
        co_mask_t mask;
        int mail_limit;
        
        channel_t *box[CO_EV_END];
    } ev;

    struct {
        co_cred_t cred;
        tm_node_t timer;
    } suspend;
    
    uint32_t    protectedM; /* last */
}
coroutine_t;

typedef struct {
    coroutine_t *idle;
    coroutine_t *running;
    coroutine_t *dead;
    ucontext_t main;
    
    int count;
    struct {
        int count;
        struct list_head list;
    } fsm[CO_END];

    bool init;
    
    int size;
    coroutine_t **array;
} co_control_t;

#ifndef INLINE_VAR_COROUTINE
#define INLINE_VAR_COROUTINE    ____INLINE_VAR_COROUTINE
#endif

#define DECLARE_REAL_COROUTINE  co_control_t INLINE_VAR_COROUTINE
#define DECLARE_FAKE_COROUTINE  extern DECLARE_REAL_COROUTINE

#ifdef __BUSYBOX__
#define DECLARE_COROUTINE       DECLARE_FAKE_COROUTINE
#else
#define DECLARE_COROUTINE       DECLARE_REAL_COROUTINE
#endif

DECLARE_FAKE_COROUTINE;

static inline co_control_t *
__co(void)
{
#ifdef USE_INLINE_COROUTINE
    return &INLINE_VAR_COROUTINE;
#else
    return NULL;
#endif
}

static inline void
__co_control_init(void)
{
    int i;

    for (i=0; i<CO_END; i++) {
        INIT_LIST_HEAD(&__co()->fsm[i].list);
    }
}

#define CO_GROW             32
#define CO_ARRAY(_id)       __co()->array[_id]

static inline coroutine_t *
__co_getbyid(co_id_t id)
{
    return CO_ARRAY(id);
}

static inline bool
__is_god_coev(int ev)
{
    return is_good_enum(ev, CO_EV_END);
}

static inline bool
is_good_id(co_id_t id)
{
    return is_good_enum(id, __co()->size);
}

static inline void
__co_dump(coroutine_t *co)
{
    if (co) {
        os_println(
            __tab "CO(%s) id=%d fsm=%d",
            co->name,
            co->id,
            co->fsm.state);
    }
}

static inline void
__co_dump_all(void)
{
    coroutine_t *co;

    os_println("===========CO-RUNNING========");
    __co_dump(__co()->running);

    os_println("===========CO-READY==========");
    list_for_each_entry(co, &__co()->fsm[CO_READY].list, fsm.node) {
        __co_dump(co);
    }
    
    os_println("===========CO-SUSPEND========");
    list_for_each_entry(co, &__co()->fsm[CO_SUSPEND].list, fsm.node) {
        __co_dump(co);
    }

    os_printf(__crlf);
}


static inline bool
__co_check(coroutine_t *co)
{
    bool check = 
            OS_PROTECTED==  co->protectedH &&
            OS_PROTECTED==  co->protectedM &&
            OS_PROTECTED== *co->protectedT;

    if (false==check) {
        debug_error("protected H=0x%x, M=0x%x, T=0x%x", 
            co->protectedH,
            co->protectedM,
            *co->protectedT);
    }
    
    return check;
}

static inline bool
__is_good_co(coroutine_t *co)
{
    return co && __co_check(co) && is_good_id(co->id) && co==__co_getbyid(co->id);
}

static inline bool
__is_good_co_running(void)
{
    return __is_good_co(__co()->running);
}

static inline bool
__is_good_coid(co_id_t id)
{
    return is_good_id(id) && __is_good_co(__co_getbyid(id));
}

static inline int // return 0 or error
__co_a_grow(int grow)
{
    int size = __co()->size;
    
    __co()->array = (coroutine_t **)os_realloc(__co()->array, (size + grow)*sizeof(coroutine_t *));
    if (NULL==__co()->array) {
        return -ENOMEM;
    }
    os_memzero(__co()->array+size, grow*sizeof(coroutine_t *));

    __co()->size += grow;
    
    return 0;
}

static inline int // return old size or error
__co_aa_grow(void)
{
    int grow = CO_GROW;
    
    if (8==sizeof(void *)) { //64bit
        grow = CO_GROW*CO_GROW;
    }

    return __co_a_grow(grow);
}

static inline void
____co_a_bind(coroutine_t *co, co_id_t id)
{
    CO_ARRAY(id) = co;
    co->id = id;

    //debug_trace("__a_bind %s:%d", co->name, id);
}

static inline void
____co_a_unbind(coroutine_t *co, co_id_t id)
{
    //debug_trace("__a_unbind %s:%d", co->name, id);
    
    CO_ARRAY(id) = NULL;
    co->id = INVALID_COMMON_ID;
}

static int
__co_a_bind(coroutine_t *co, co_id_t id)
{
    int i, err, found = INVALID_COMMON_ID;
    bool try = false;

    /*
    * check co is binded ?
    */
    for (i=0; i<__co()->size; i++) {
        if (__co_getbyid(i) && co==__co_getbyid(i)) {
            return -EEXIST;
        }
    }

    /*
    * id is good, try use it
    */
    if (is_good_id(id)) {
        if (NULL==__co_getbyid(id)) { // NOT exist, can use it
            ____co_a_bind(co, id);
            
            return 0;
        } else { // exist, can NOT use it
            return -EEXIST;
        }
    }

again:
    /*
    * id is invalid, find free-id
    */
    for (i=0; i<__co()->size; i++) {
        if (NULL==__co_getbyid(i)) { // NOT exist
            found = i;
                        
            break;
        }
    }

    /*
    * full
    */
    if (false==is_good_id(found)) {
        err = __co_aa_grow();
        if (err) {
            return err;
        }

        if (false==try) {
            try = true;
            
            goto again;
        } else {
            return -ENOMEM;
        }
    }

    ____co_a_bind(co, found);
    
    return 0;
}

static void
__co_a_unbind(coroutine_t *co, co_id_t id)
{
    if (co) {
        int i;
        
        if (is_good_id(id) && co==__co_getbyid(id)) {
            ____co_a_unbind(co, id);

            return;
        }
        
        for (i=0; i<__co()->size; i++) {
            if (co==__co_getbyid(id)) {
                ____co_a_unbind(co, id);

                return;
            }
        }
    }
}

static inline int
__co_aa_bind(coroutine_t *co)
{
    return __co_a_bind(co, INVALID_COMMON_ID);
}

static inline void
__co_aa_unbind(coroutine_t *co)
{
    __co_a_unbind(co, co->id);
}

static inline void
__co_fsm_insert(coroutine_t *co, bool first)
{
    if (CO_READY==co->fsm.state || CO_SUSPEND==co->fsm.state) {
        if (first) {
            list_add(&co->fsm.node, &__co()->fsm[co->fsm.state].list);
        } else {
            list_add_tail(&co->fsm.node, &__co()->fsm[co->fsm.state].list);
        }
        __co()->fsm[co->fsm.state].count++;
    }
}

static inline void
__co_fsm_remove(coroutine_t *co)
{
    if (CO_READY==co->fsm.state || CO_SUSPEND==co->fsm.state) {
        list_del(&co->fsm.node);
        __co()->fsm[co->fsm.state].count--;
    }
}

static inline void
____co_fsm_change(coroutine_t *co, enum co_fsm fsm, bool first)
{
    __co_fsm_remove(co);
    if (fsm != co->fsm.state) {
        co->fsm.state = fsm;
        co->st.fsm[fsm]++;
    }
    __co_fsm_insert(co, first);
}

static inline void
__co_fsm_change(coroutine_t *co, enum co_fsm fsm)
{
    if (co) {
        ____co_fsm_change(co, fsm, false);
    }
}

static inline coroutine_t *
__co_schedule(void)
{
    struct list_head *head;
    coroutine_t *co;
    
    head = &__co()->fsm[CO_READY].list;
    if (false==list_empty(head)) {
        co = list_first_entry(head, coroutine_t, fsm.node);
    } else {
        co = __co()->idle;
    }
    
    debug_trace("SCHEDULE %s==>%s", __co()->running->name, co->name);
    
    return co;
}

static inline void
__co_ev_clean(coroutine_t *co)
{
    os_ch_free(co->ev.box[CO_EV_MAIL]);
}

static inline void
____co_destroy(coroutine_t *co)
{
    if (co) {
        __co_ev_clean(co);
        __co_aa_unbind(co);
        os_free(co);
    }
}

#define __co_destory(_co)   do{ \
    if (_co) {                  \
        __co_fsm_remove(_co);   \
        ____co_destroy(_co);    \
        __co()->count--;        \
        _co = NULL;             \
    }                           \
}while(0)

static void
__co_main(uint32_t L32, uint32_t H32)
{
    int err = 0;
    coroutine_t *co;
#if defined(LP64) || defined(LLP64)
    uint64_t L = (uint64_t)L32;
    uint64_t H = (uint64_t)H32;
    uint64_t V = L | (H<<32);
    co = (coroutine_t *)V;
#else
    co = (coroutine_t *)L32;
    (void)H32;
#endif

    debug_trace("CO(%s) begin(%p)", co->name, co);
    err = (*co->main.func)(co->main.data);
    if (err) {
        // log
    }
    debug_trace("CO(%s) end(%p)", co->name, co);
    
    ____co_fsm_change(co, CO_DEAD, true);
    debug_trace("CO(%s) exit(pre CO.dead=(CO(%s):%p))", 
        co->name, __co()->dead?__co()->dead->name:__unknow, 
        __co()->dead);
    if (__co()->dead) {
        trace_assert(0, "co is dead");
    }
    __co()->dead = co;
    debug_trace("CO(%s) exit(set CO.dead=(CO(%s):%p))", co->name, __co()->dead->name, __co()->dead);
}

static int
__co_ev_init(coroutine_t *co)
{
    co->ev.box[CO_EV_MAIL] = os_och_new(co->ev.mail_limit, sizeof(union co_mail));
    if (NULL==co->ev.box[CO_EV_MAIL]) {
        return -ENOMEM;
    }

    return 0;
}

static int
__co_init(coroutine_t *co)
{
    int err;
    
    err = __co_aa_bind(co);
    if (err) {
        debug_error("co(%s) aotubind error(%d)", co->name, err);
        
        return err;
    }

    err = __co_ev_init(co);
    if (err) {
        debug_error("co(%s) ev init error(%d)", co->name, err);
        
        return err;
    }

    if (__co()->idle) {
        err = getcontext(&co->ctx);
        if (err) {
            debug_error("co(%s) get context error(%d)", co->name, err);
            
            return err;
        }

#if defined(LP64) || defined(LLP64)
        uint64_t V = (uint64_t)co;
        uint64_t L = (V<<32)>>32;
        uint64_t H = V>>32;
        makecontext(&co->ctx, (void (*)(void))__co_main, 2, (uint32_t)L, (uint32_t)H);
#else
        makecontext(&co->ctx, (void (*)(void))__co_main, 2, (uint32_t)co, 0);
#endif
    }
    
    __co_fsm_insert(co, false);
    __co()->count++;

    return 0;
}

static coroutine_t *
__co_create(
    const char *name,
    co_main_f *main, 
    void *data, 
    int stack_size, 
    int mail_limit,
    enum co_pri pri, 
    enum co_fsm fsm
)
{
    coroutine_t *co = NULL;
    int err;

    if (stack_size<0) {
        stack_size = CO_STACK_DEFAULT;
    }
    
    co = (coroutine_t *)os_zalloc(sizeof(*co) + stack_size + sizeof(uint32_t)/* protectedT */);
    if (NULL==co) {
        return NULL;
    }
    strcpy(co->name, name);
    
    co->main.func       = main;
    co->main.data       = data;
    co->pri.now         = is_good_enum(pri, CO_PRI_END)?pri:CO_PRI_DEFAULT;
    co->fsm.state       = fsm;
    co->stack.size      = stack_size;
    co->stack.sp        = (char *)(co + 1);
    co->id              = INVALID_COMMON_ID;
    co->ev.mail_limit   = (mail_limit<=0)?CO_MAIL_DEFAULT:mail_limit;
    
    if (__co()->idle) {
        co->ctx.uc_stack.ss_sp   = co->stack.sp;
        co->ctx.uc_stack.ss_size = co->stack.size;
        co->ctx.uc_link = &__co()->idle->ctx;
    }
    
    co->protectedH = OS_PROTECTED;
    co->protectedM = OS_PROTECTED;
    co->protectedT = (uint32_t *)(co->stack.sp + stack_size);
    *co->protectedT= OS_PROTECTED;
    
    err = __co_init(co);
    if (err) {
        ____co_destroy(co);

        return NULL;
    }

    debug_trace("CO(%s) created(%p)", co->name, co);
    
    return co;
}

static inline co_mask_t
__co_evmask_read(coroutine_t *co)
{
    return co->ev.mask;
}

static inline co_mask_t
__co_evmask_read_and_zero(coroutine_t *co)
{
    co_mask_t mask = co->ev.mask;
    
    co->ev.mask = 0;

    return mask;
}

static inline bool
__co_ev_read(coroutine_t *co, enum co_event ev)
{
    return os_hasbit(co->ev.mask, ev); 
}

static inline bool
__co_ev_read_and_zero(coroutine_t *co, enum co_event ev)
{
    bool is_set = os_hasbit(co->ev.mask, ev);
    
    os_clrbit(co->ev.mask, ev);
    
    return is_set;
}

static inline int
__co_signal(coroutine_t *co, enum co_event ev)
{
    os_setbit(co->ev.mask, ev);

    return 0;
}

static inline int
__co_mail_send(coroutine_t *co, enum co_event ev, union co_mail *mail)
{
    trace_assert(co->ev.box[ev], "co send mail when empty");

    __co_signal(co, ev);
    
    return os_och_write(co->ev.box[ev], mail);
}

static int
__co_mail_recv(coroutine_t *co, enum co_event ev, union co_mail *mail)
{
    trace_assert(co->ev.box[ev], "co recv mail when not empty");
    
    __co_ev_read_and_zero(co, ev);
    
    return os_och_read(co->ev.box[ev], mail);
}

static inline int
__co_ready(coroutine_t *co, co_cred_t cred, bool immediately)
{
    if (cred && cred==co->suspend.cred) {
        return -EKEYBAD;
    } else {
        co->suspend.cred = 0;
        
        ____co_fsm_change(co, CO_READY, immediately);
        
        return 0;
    }
}

static int
__co_resume(coroutine_t *create, bool suspend_old)
{
    int err;
    coroutine_t *old = __co()->running;
    
    if (CO_SUSPEND==create->fsm.state) {
        return os_assertV(-EKEYBAD);
    }
    else if (NULL==old) {
        // old is main, do nothing
    }
    else if (old==__co()->idle) {
        // old is idle, do nothing
    }
    else if (old==create) {
        return os_assertV(0);
    }

    __co_fsm_change(old, suspend_old?CO_SUSPEND:CO_READY);
    __co_fsm_change(create, CO_RUNNING);
    __co()->running = create;

    if (NULL==old) {
        old = __co()->idle;
    }

    __is_good_co_running();
    err = swapcontext(&old->ctx, &create->ctx);
    __is_good_co_running();
    if (err) {
        return err;
    }
    
    return 0;
}

static inline int 
__co_suspend_timeout(tm_node_t *timer)
{
    coroutine_t *co;

    if (NULL==timer) {
        return os_assertV(-EKEYNULL);
    }
    
    co = container_of(timer, coroutine_t, suspend.timer);

    __co_signal(co, CO_EV_SUSPEND_TIMEOUT);
    
    __co_ready(co, 0, true);
    
    return 0;
}

static int
__co_suspend(int timeout/* ms */, co_cred_t cred)
{
    int err, after = 0;
    coroutine_t *co = __co()->running;
    bool timeouted = false;

    if (timeout<0) {
        after = 0;
    }
    else if (0==timeout) {
        after = 1;
    } else {
        after = timeout/os_tm_unit();
        if (0==after) {
            after = 1;
        }
    }
    
    co->suspend.cred = cred;
    if (after) {
        err = os_tm_insert(&co->suspend.timer, after, __co_suspend_timeout, false);
        if (err) {
            //log
        }
    }
    
    __co_resume(__co_schedule(), true);
    
    /*
    * now come back
    *   have setuped timer but NOT timeout, so need to remove timer
    */
    timeouted = __co_ev_read_and_zero(co, CO_EV_SUSPEND_TIMEOUT);
    if (after) {
        err = os_tm_remove(&co->suspend.timer);
        if (err) {
            //log
        }
    }
    
    return timeouted?1:0;
}

static inline int
__co_yield(void)
{
    return __co_resume(__co_schedule(), false);
}


static inline void
co_fini(void)
{
    if (__co()->init) {
        __co()->init = false;
        
        os_tm_fini();
    }
}

static inline void
co_init(void)
{
    if (false==__co()->init) {
        __co()->init = true;
        
        os_tm_init();
        
        __co_control_init();
        
        /*
        * idle is the first coroutine
        */
        __co()->idle = __co_create(CO_IDLE_NAME, NULL, NULL, 0, 0, CO_PRI_LOWEST, CO_READY);
        if (NULL==__co()->idle) {
            // log
        }
    }
}

/*
* CAN call in main
*
* return: CO ID
* 
* param:
*   @name: CO Name
*   @main: CO function
*   @data: CO function param
*   @stack_size: CO stack size
*       <0, use default(CO_STACK_DEFAULT)
*   @mail_limit: CO mail box size
*       <=0, use default(CO_MAIL_DEFAULT)
*   @pri: CO pri
*   @suspend: init fsm
*       true: init fsm as CO_SUSPEND
*       false:init fsm as CO_READY
*/
static inline co_id_t
co_new(
    char *name,
    co_main_f *main, 
    void *data, 
    int stack_size, /* default 8K */
    int mail_limit,
    enum co_pri pri,
    bool suspend
)
{
    coroutine_t *co;
    static char *reserved_name[] = {
        CO_IDLE_NAME,
        CO_NOEXIST_NAME,
    };
    int i;
    
    if (NULL==main) {
        return os_assertV(-EINVAL1);
    }
    else if (NULL==name) {
        name = __unknow;
    }
    else if (os_strlen(name) > CO_NAME_LEN) {
        return os_assertV(-EINVAL2);
    }
    
    for (i=0; i<os_count_of(reserved_name); i++) {
        if (0==os_strcmp(name, reserved_name[i])) {
            return os_assertV(-EINVAL3);
        }
    }
    
    co = __co_create(name, main, data, stack_size, mail_limit, pri, suspend?CO_SUSPEND:CO_READY);
    if (NULL==co) {
        return -ENOMEM;
    }

    return co->id;
}

/*
* if call in main, return the IDLE ID
*/
static inline co_id_t
co_self(void)
{
    if (__is_good_co_running()) {
        return __co()->running->id;
    } else {
        return __co()->idle->id;
    }
}

/*
* can NOT call in main
*/
static inline void
co_resume(co_id_t id)
{
    int err;

    if (false==__is_good_coid(id)) {
        trace_assert(0, "bad coid:%d", id);

        return;
    }
    else if (false==__is_good_co_running()) {
        trace_assert(0, "co is not running");

        return;
    }
    
    err = __co_resume(__co_getbyid(id), false);
    if (err) {
        //log
    }
}

/*
* can NOT call in main
*/
static inline void
co_yield(void)
{
    int err = __co_yield();
    if (err) {
        // log
    }
}

/*
* CAN call in main
*/
static inline void
co_idle(void)
{
    coroutine_t *create;
    
    __co()->running = __co()->idle;

    while(1) {
        create = __co_schedule();
        if (create==__co()->idle) {
            /*
            * maybe IDLE is the first READY,
            *   fsm_change remove IDLE to the last
            *   then next schedule can get READY coroutine
            */
            __co_fsm_change(create, CO_READY);
            debug_trace("IDLE fsm change READY");
            usleep(CO_IDLE_TIME);
        } else {
            __co_resume(create, false);
            if (__co()->dead) {
                /*
                * someone exit and go here, need free it
                */
                __co()->running = __co()->idle;
                
                debug_trace("IDLE free dead(CO(%s):%p)", __co()->dead->name, __co()->dead);
                __co_destory(__co()->dead);
            } else {
                /*
                * someone yeild and go here, do nothing
                */
            }
        }
        
        __co_dump_all();
    }
}

/*
* can NOT call in main
*/
static inline co_cred_t
co_cred(void)
{
    return ++__co()->running->cred;
}

/*
* can NOT call in main
*
* @timeout(ms)
*   <0: wait forever, not setup timer
*   =0: wait one tick, auto setup timer
*   >0: wait timeout(ms), auto setup timer
*
* return:
*   <0: error
*   =0: ok
*   >0: timeout
*/
static inline int
co_suspend(int timeout/* ms */, co_cred_t cred)
{
    if (cred && __is_good_co_running()) {
        return __co_suspend(timeout, cred);
    } else {
        return os_assertV(-EINVAL1);
    }
}

/*
* can NOT call in main
*/
static inline int
co_ready(co_id_t id, co_cred_t cred, bool immediately)
{
    coroutine_t *co;

    if (false==__is_good_coid(id)) {
        return os_assertV(-EKEYBAD);
    }
    else if (false==__is_good_co_running()) {
        return os_assertV(-EINVAL1);
    }
    
    co = __co_getbyid(id);
    if (CO_SUSPEND==co->fsm.state) {
        if (0==cred) {
            return os_assertV(-EINVAL2);
        }
        else if (cred!=co->suspend.cred) {
            return -EINVAL3;
        }
        
        /*
        * remove suspend timer and timeout-event
        */
        os_tm_remove(&co->suspend.timer);
        __co_ev_read_and_zero(co, CO_EV_SUSPEND_TIMEOUT);

        return __co_ready(co, cred, immediately);
    }
    else if (CO_RUNNING==co->fsm.state || co==__co()->running) {
        return os_assertV(-EINVAL4);
    }
    
    return 0;
}

/*
* can NOT call in main
*/
static inline char *
co_name(co_id_t id)
{
    coroutine_t *co;

    if (false==__is_good_coid(id)) {
        return os_assertV(CO_NOEXIST_NAME);
    }
    else if (false==__is_good_co_running()) {
        return os_assertV(CO_NOEXIST_NAME);
    }
    
    co = __co_getbyid(id);
    return co->name;
}

/*
* CAN call in main
*/
static inline char *
co_self_name(void)
{
    return co_name(co_self());
}

static inline int
co_signal(co_id_t id, enum co_event ev)
{
    if (false==__is_good_coid(id)) {
        return os_assertV(-EKEYBAD);
    }
    else if (false==__is_good_co_running()) {
        return os_assertV(-EINVAL1);
    }
    else if (false==__is_god_coev(ev)) {
        return os_assertV(-EINVAL2);
    }
    else if (CO_EV_MAIL==ev) {
        return os_assertV(-EINVAL3);
    }
    
    return __co_signal(__co_getbyid(id), ev);
}


static inline co_mask_t
co_evmask_read(void)
{
    if (false==__is_good_co_running()) {
        return os_assertV(0);
    }

    return __co_evmask_read(__co()->running);
}


static inline co_mask_t
co_evmask_read_and_zero(void)
{
    if (false==__is_good_co_running()) {
        return os_assertV(0);
    }

    return __co_evmask_read_and_zero(__co()->running);
}


static inline bool
co_ev_read(enum co_event ev)
{
    if (false==__is_good_co_running()) {
        return os_assertV(false);
    }
    else if (false==__is_god_coev(ev)) {
        return os_assertV(false);
    }

    return __co_ev_read(__co()->running, ev);
}


static inline bool
co_ev_read_and_zero(enum co_event ev)
{
    if (false==__is_good_co_running()) {
        return os_assertV(false);
    }
    else if (false==__is_god_coev(ev)) {
        return os_assertV(false);
    }

    return __co_ev_read_and_zero(__co()->running, ev);
}


/*
* aoto set CO_EV_MAIL event
*/
static inline int
co_mail_send(co_id_t id, union co_mail *mail)
{
    if (false==__is_good_coid(id)) {
        return os_assertV(-EKEYBAD);
    }
    else if (false==__is_good_co_running()) {
        return os_assertV(-EINVAL1);
    }
    else if (NULL==mail) {
        return os_assertV(-EINVAL2);
    }

    return __co_mail_send(__co_getbyid(id), CO_EV_MAIL, mail);
}


/*
* aoto clear CO_EV_MAIL event
*/
static inline int
co_mail_recv(union co_mail *mail)
{
    if (false==__is_good_co_running()) {
        return os_assertV(-EINVAL1);
    }
    else if (NULL==mail) {
        return os_assertV(-EKEYNULL);
    }

    return __co_mail_recv(__co()->running, CO_EV_MAIL, mail);
}
/******************************************************************************/
#endif
#endif /* __COROUTINE_H_5D324594B45198CC07E58DDC22E1CBFC__ */
