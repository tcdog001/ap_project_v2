#ifndef __CMD_H_F3687584F159827DAA20B322924194D1__
#define __CMD_H_F3687584F159827DAA20B322924194D1__
/******************************************************************************/
#if defined(__APP__) || defined(__BOOT__)
#ifndef __COMMAND_COUNT
#define __COMMAND_COUNT         (2*32)
#endif

enum {
    __COMMAND_F_HIDE = 0x01,
};

/*
* 两种模式
*  (1): 参数之间没有关联关系，可以任意组合
*       os_do_command 需要循环处理所有匹配的命令
*       目前不支持
*
*  (2): 参数之间有关联关系，不能任意组合，需要列出全部有效参数组合
*       os_do_command 只处理首次匹配到的命令
*       目前支持的模式
*/

struct command_item {
    /*
    * list数组元素实际个数
    * 无需初始化，do_command 内部会扫描list自动计算
    */
    int count;
    
    /*
    * 命令列表，用于描述命令格式
    *   比如命令 xxx -a address -v value1 value2, 则
    *       list[0] 初始化为 "-a"，     命令关键字
    *       list[1] 初始化为 "address"，"-a"的参数
    *       list[2] 初始化为 "-v"，     命令关键字
    *       list[3] 初始化为 "value1"， "-v"的第一个参数
    *       list[4] 初始化为 "value2"， "-v"的第二个参数
    *       list剩余为NULL
    *
    * os_do_command 会根据 list 自动生成帮助信息，要求
    *   (1)程序至少有一个参数(main's argc>1)，如果无参数会自动生成并打印帮助
    *   (2)执行 "xxx -h" 会自动生成并打印帮助，但不会显示隐藏命令
    *   (3)执行 "xxx --" 会自动生成并打印帮助，包括隐藏命令
    */
    char *list[__COMMAND_COUNT+1];
    
    /*
    * 对整条命令的帮助信息, 可以为 NULL
    */
    char *help;
    
    /*
    * 命令执行函数
    *   argc, argv 是从 main 函数传入的, argv[0] 是程序名
    */
    int (*func)(int argc, char *argv[]);
    
    /*
    * 命令标识
    *   __COMMAND_F_HIDE 置位表示命令为隐藏命令
    *       执行 "xxx" 或 "xxx -h" 时不显示帮助信息，但实际可执行
    *       执行 "xxx --" 会显示帮助信息, "xxx --"自身不在help信息中
    */
    unsigned int flag;

    /*
    * 命令有效性控制，与 command_ctrl is_use_mask/bit 配合使用
    *
    * 先使用32位，后续如不够用改为64位
    */
    unsigned int mask;
};

struct command_ctrl {
    struct command_item *cmd;   /* array of (struct command_item) */
    int count;                  /* count of cmd         */
    /*
    * 如果 is_use_mask 为假，则 bit 及 command的mask 均无意义
    * 如果 is_use_mask 为真，则 对每条命令执行掩码测试 ((1<<ctrl's bit) & cmd's mask)
    *   测试结果为真，则此条命令有效
    *   否则，此条命令无效(不生成帮助，不能执行)
    * 目前主要用于过滤特定产品相关命令
    */
    bool is_use_mask;
    int bit;
};

#define __COMMAND_ARG_HELPS     "-h"
#define __COMMAND_ARG_HELPL     "-help"
#define __COMMAND_ARG_HIDE      "--"

#define __COMMAND_MASK_ALL      0xffffffff

#define __COMMAND_CTRL_INITER(_cmd, _is_use_mask, _bit) { \
    .cmd        = _cmd,             \
    .count      = os_count_of(_cmd),\
    .is_use_mask= _is_use_mask,     \
    .bit        = _bit,             \
}
#define COMMAND_CTRL_INITER(_cmd) \
        __COMMAND_CTRL_INITER(_cmd, false, 0)

static inline struct command_item *
__command_getbyindex(struct command_ctrl *ctrl, int idx)
{
    return &ctrl->cmd[idx];
}


static inline int 
__command_list_count(struct command_item *cmd)
{
    int i;

    for (i=0; i<__COMMAND_COUNT && cmd->list[i]; i++) {
        ;
    }

    return i;
}

static inline void 
__command_init_one(struct command_ctrl *ctrl, int idx)
{
    struct command_item *cmd = __command_getbyindex(ctrl, idx);
    
    /*
    * step:1
    *
    * get command count
    */
    cmd->count = __command_list_count(cmd);

    /*
    * step:2
    *
    * rewrite command mask
    */
    if (ctrl->is_use_mask && 0 == cmd->mask) {
        cmd->mask = __COMMAND_MASK_ALL;
    }
}


static inline void 
__command_init(struct command_ctrl *ctrl)
{
    int i;
    
    for (i=0; i<ctrl->count; i++) {
        __command_init_one(ctrl, i);
    }
}

static inline bool 
__is_command_used(struct command_ctrl *ctrl, int idx)
{
    struct command_item *cmd = __command_getbyindex(ctrl, idx);
    
    return (false == ctrl->is_use_mask || os_hasbit(cmd->mask, ctrl->bit));
}

static inline bool 
__is_command_hide(struct command_item *cmd)
{
    return os_hasflag(cmd->flag, __COMMAND_F_HIDE);
}


static inline void 
__commond_do_help_one(struct command_item *cmd)
{
    int i;

    /* 
    * __tab
    */
    os_printf(__tab);
    
    for (i=0; i<cmd->count; i++) {
        if (cmd->list[i]) {
            os_printf( "%s ", cmd->list[i]);
        }
    }
    
    /* 
    * help string
    */
    os_printf( __crlf __tab2 "%s" __crlf, cmd->help?cmd->help:"......");
}


static inline void 
__commond_help_one(struct command_ctrl *ctrl, int idx, bool show_hide)
{
    struct command_item *cmd = __command_getbyindex(ctrl, idx);
    
    if (false == __is_command_used(ctrl, idx)) {
        return;
    }
    
    /*
    * is hide command, and not show_hide
    */
    if (__is_command_hide(cmd) && false==show_hide) {
        return;
    }
    
    /*
    * then, make help
    */
    __commond_do_help_one(cmd);
}


static inline void 
__commond_help(struct command_ctrl *ctrl, char *name, bool show_hide)
{
    int i;
    
    /* 
    * help head
    */
    os_println("%s:", name);
    
    for (i=0; i<ctrl->count; i++) {
        __commond_help_one(ctrl, i, show_hide);
    }
}

static inline bool 
__command_match(int argc, char *argv[], struct command_ctrl *ctrl, int idx)
{
    int i;
    struct command_item *cmd = __command_getbyindex(ctrl, idx);

    if ((argc-1)!=cmd->count) {
        return false;
    }
    /*
    * not match current
    */
    else if (false==__is_command_used(ctrl, idx)) {
        return false;
    }
    
    for (i=0; i<cmd->count; i++) {
        if (cmd->list[i]
            /* 
            * begin with '-', need compare 
            */
            && '-' == cmd->list[i][0]
            /* 
            * main's argv != cmd's arg
            */
            && 0 != os_strcmp(argv[i+1], cmd->list[i])) {
            return false;
        }
    }
    
    return (i == cmd->count);
}

/* os_do_command 举例

struct command_item commands[] = {
    {
        .list = {"-r", "-a", "address"},    // xxx -r -a address
        .func = reg_read,                   // command function
        .help = "read from the address.",   // command help
    },
    {
        .list = {"-w", "-a", "address", "-v", "value"}, // xxx -w -a address -v value
        .func = reg_write,                      // command function
        .help = "write value to the address.",  // command help
        .flag = __COMMAND_F_HIDE,                 // hide command
    },
};

struct command_ctrl ctrl = __COMMAND_CTRL_INITER(commands, true, 0xffffffff);

os_do_command(argc, argv, &ctrl);

*/
static inline int 
os_do_command(int argc, char *argv[], struct command_ctrl *ctrl)
{
    int i;
    bool show_hide = false;
    
    __command_init(ctrl);

    if (1==argc) {
        goto show_help;
    }
    else if (2 == argc) {
        char *help = argv[1];

        /*
        * "xxx -h"
        * "xxx -help"
        * "xxx --"
        */
        if (0 == os_strcmp(help, __COMMAND_ARG_HELPS)   ||
            0 == os_strcmp(help, __COMMAND_ARG_HELPL)) {
            goto show_help;
        }
        else if (0 == os_strcmp(help, __COMMAND_ARG_HIDE)) {
            show_hide = true;
            goto show_help;
        }
    }

    for (i=0; i<ctrl->count; i++) {
        if (__command_match(argc, argv, ctrl, i)) {
            struct command_item *cmd = __command_getbyindex(ctrl, i);
            
            return (*cmd->func)(argc, argv);
        }
    }
    
show_help:
    __commond_help(ctrl, argv[0], show_hide);
    
    return -EFORMAT;
}
/******************************************************************************/
typedef struct {
    char *tag;

    union {
        void *cb;
        int (*line_cb)(char *args);
        int (*argv_cb)(int argc, char *argv[]);
    } u;
} cmd_table_t;

#define CMD_ENTRY(_tag, _cb)   { \
    .tag    = _tag,         \
    .u      = {             \
        .cb = _cb,          \
    },                      \
}

#define __os_cmd_dump_argv(_dump, _argc, _argv) ({ \
    int i;                                  \
                                            \
    for (i=0; i<_argc; i++) {               \
        _dump("function:%s argv[%d]=%s",    \
            __func__, i, _argv[i]);         \
    }                                       \
                                            \
    _argv[0];                               \
})

#define os_cmd_trace_argv(_argc, _argv)     __os_cmd_dump_argv(debug_trace, _argc, _argv)

#define __cmd_line_next(_args)              __string_next_byifs(_args, ' ')
#define os_cmd_shift(_args, _count)     do{ \
    int i, count = (_count);                \
                                            \
    for (i=0; i<count; i++) {               \
        _args = __cmd_line_next(_args);     \
    }                                       \
}while(0)

static int
cmd_line_handle(cmd_table_t map[], int count, char *tag, char *args)
{
    int i;
    
    for (i=0; i<count; i++) {
        cmd_table_t *table = &map[i];
        
        if (0==os_strcmp(table->tag, tag)) {
            return (*table->u.line_cb)(args);
        }
    }
    
    return -ENOEXIST;
}

static int
cmd_argv_handle(cmd_table_t map[], int count, int argc, char *argv[])
{
    int i;

    if (argc < 1) {
        return -EINVAL0;
    }
    
    for (i=0; i<count; i++) {
        cmd_table_t *table = &map[i];
        
        if (0==os_strcmp(table->tag, argv[0])) {
            return (table->u.argv_cb)(argc-1, argv+1);
        }
    }

    return -ENOEXIST;
}
/******************************************************************************/
#define SIMPILE_CMD_ARGV_COUNT  16

typedef struct {
    int (*handle)(int argc, char *argv[]);
    
    int argc;
    char *argv[SIMPILE_CMD_ARGV_COUNT];
} simpile_cmd_t;

static inline simpile_cmd_t *
simpile_cmd_argv(char *command)
{
    static char line[1+OS_LINE_LEN];
    static simpile_cmd_t cmd;
    char *p;

    os_objzero(&cmd);
    os_arrayzero(line);
    os_strdcpy(line, command);

    for(p = os_strtok(line, " ");
        p;
        p = os_strtok(NULL, " ")) {
        //debug_trace("cmd.argv[%d]=%s", cmd.argc, p);
        
        cmd.argv[cmd.argc++] = p;
    }

    return &cmd;
}

static inline bool
__simpile_cmd_match(simpile_cmd_t *cmd, int argc, char *argv[])
{
    int i;

    if (argc!=cmd->argc) {
        return false;
    }

    for (i=0; i<argc; i++) {
        if (cmd->argv[i] && os_strcmp(argv[i], cmd->argv[i])) {
            return false;
        }
    }

    return true;
}

static inline int
__simpile_cmd_handle(int count, simpile_cmd_t cmd[], int argc, char *argv[], int (*usage)(char *self))
{
    int i;

    for (i=0; i<count; i++) {
        if (__simpile_cmd_match(&cmd[i], argc, argv)) {
            return (*cmd[i].handle)(argc, argv);
        }
    }

    return (*usage)((argv-1)[0]);
}

#define simpile_cmd_handle(_cmd, _argc, _argv, _usage) ({ \
    int err = __simpile_cmd_handle(os_count_of(_cmd), _cmd, _argc-1, _argv+1, _usage); \
    shell_error(err); \
})  /* end */
#endif /* defined(__APP__) || defined(__BOOT__) */

#ifdef __APP__
#ifndef SIMPILE_RESSIZE
#define SIMPILE_RESSIZE         (1024*1024-1)
#endif

typedef struct {
    int err;
    int len;
    char buf[1+SIMPILE_RESSIZE];
} simpile_response_t;

#define simpile_res_hsize       offsetof(simpile_response_t, buf)

#ifndef INLINE_VAR_SIMPILE_RES
#define INLINE_VAR_SIMPILE_RES      ____INLINE_VAR_SIMPILE_RES
#endif

#define DECLARE_FAKE_SIMPILE_RES    extern simpile_response_t INLINE_VAR_SIMPILE_RES
#define DECLARE_REAL_SIMPILE_RES    simpile_response_t INLINE_VAR_SIMPILE_RES

#ifdef __BUSYBOX__
#define DECLARE_SIMPILE_RES     DECLARE_FAKE_SIMPILE_RES
#else
#define DECLARE_SIMPILE_RES     DECLARE_REAL_SIMPILE_RES
#endif

DECLARE_FAKE_SIMPILE_RES;

static inline simpile_response_t *
__simpile_res(void)
{
#ifdef USE_INLINE_SIMPILE_RES
    return &INLINE_VAR_SIMPILE_RES;
#else
    return NULL;
#endif
}

#define simpile_res_err     __simpile_res()->err
#define simpile_res_buf     __simpile_res()->buf
#define simpile_res_len     __simpile_res()->len
#define simpile_res_size    (simpile_res_hsize + simpile_res_len)

#define simpile_res_zero()  os_objzero(__simpile_res())
#define simpile_res_clear() do{ \
    simpile_res_buf[0] = 0;     \
    simpile_res_len = 0;        \
}while(0)
#define simpile_res_error(_err)     (simpile_res_err=(_err))
#define simpile_res_ok              simpile_res_error(0)

#define simpile_res_sprintf(_fmt, _args...) ({  \
    int __len = os_snprintf(                    \
            simpile_res_buf + simpile_res_len,  \
            SIMPILE_RESSIZE - simpile_res_len,  \
            _fmt, ##_args);                     \
    simpile_res_len += __len;                   \
                                                \
    __len;                                      \
})  /* end */

#define simpile_res_recv(_fd, _timeout) \
    io_recv(_fd, (char *)__simpile_res(), sizeof(simpile_response_t), _timeout)

#define simpile_res_send(_fd) \
    io_send(_fd, (char *)__simpile_res(), simpile_res_size)

#define simpile_res_recvfrom(_fd, _timeout, _addr, _paddrlen) \
    io_recvfrom(_fd, (char *)__simpile_res(), sizeof(simpile_response_t), _timeout, _addr, _paddrlen)

#define simpile_res_sendto(_fd, _addr, _addrlen) \
    io_sendto(_fd, (char *)__simpile_res(), simpile_res_size, _addr, _addrlen)

typedef struct {
    int timeout;
    
    struct sockaddr_un server, client;
} simpile_client_t;

#define SIMPILE_CLIENT_INITER(_timeout, _server_path) { \
    .server     = OS_SOCKADDR_UNIX("\0" _server_path),  \
    .client     = OS_SOCKADDR_UNIX(__empty),            \
    .timeout    = _timeout,                             \
}   /* end */

static int
__simpile_d_client_handle(char *line, cmd_table_t *table, int count)
{
    char *method = line;
    char *args   = line;
    int err;
    
    simpile_res_zero();
        
    __string_strim_both(method, NULL);
    __string_reduce(method, NULL);
    
    os_cmd_shift(args, 1);
    
    err = cmd_line_handle(table, count, method, args);
    simpile_res_error(err);

    debug_trace("action:%s %s, error:%d, len:%d, buf:%s", 
        method, args?args:__empty,
        simpile_res_err,
        simpile_res_len,
        simpile_res_buf);

    return 0;
}

static inline int
__simpile_d_client(int fd, cmd_table_t *table, int count)
{
    char buf[1+OS_LINE_LEN] = {0};
    struct sockaddr_un client = OS_SOCKADDR_UNIX(__empty);
    socklen_t addrlen = sizeof(client);
    int err;
    
    err = __io_recvfrom(fd, buf, sizeof(buf), (struct sockaddr *)&client, &addrlen);
    if (err<0) { /* yes, <0 */
        return err;
    }
    buf[err] = 0;
    
    if (is_abstract_sockaddr(&client)) {
        set_abstract_sockaddr_len(&client, addrlen);

        debug_trace("recv from:%s", get_abstract_path(&client));
    }

    err = __simpile_d_client_handle(buf, table, count);
    if (err) {
        /* just log, NOT return */
    }

    err = simpile_res_sendto(fd, (struct sockaddr *)&client, addrlen);
    if (err<0) { /* yes, <0 */
        return err;
    }
    
    return 0;
}

#define simpile_d_client(_fd, _table) \
    __simpile_d_client(_fd, _table, os_count_of(_table))

static int
__simpile_c_client(
    char *buf, 
    struct sockaddr_un *server, 
    struct sockaddr_un *client,
    int timeout
)
{
    int fd;
    int err;
    int len;
    
    fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (fd<0) {
        debug_error("socket error:%d", -errno);
        err = -errno; goto error;
    }
    
    err = bind(fd, (struct sockaddr *)client, get_abstract_sockaddr_len(client));
    if (err) {
        __debug_error("bind error:%d", -errno);
        err = -errno; goto error;
    }

    err = connect(fd, (struct sockaddr *)server, get_abstract_sockaddr_len(server));
    if (err) {
        debug_error("connect error:%d", -errno);
        err = -errno; goto error;
    }

    len = os_strlen(buf);
    err = io_send(fd, buf, len);
    if (err<0) { /* yes, <0 */
        goto error;
    }

    err = simpile_res_recv(fd, timeout);
    if (err<0) { /* yes, <0 */
        goto error;
    }

    if (0==simpile_res_err) {
        os_println("%s", simpile_res_buf);
    }

    debug_trace("action:%s, error:%d, len:%d, buf:%s", 
        buf,
        simpile_res_err,
        simpile_res_len,
        simpile_res_buf);
    err = shell_error(simpile_res_err);
    
error:
    if (is_good_fd(fd)) {
        close(fd);
    }
    
    return err;
}

static int
simpile_c_client(
    char *action,
    int argc, 
    char *argv[],
    struct sockaddr_un *server, 
    struct sockaddr_un *client,
    int timeout
)
{
    char buf[1+OS_LINE_LEN] = {0};
    int i, len;

    len = os_saprintf(buf, "%s", action);
    for (i=0; i<argc; i++) {
        len += os_snprintf(buf + len, OS_LINE_LEN - len, " %s", argv[i]);
    }

    return __simpile_c_client(buf, server, client, timeout);
}

typedef struct simpile_server {
    int fd;
    os_sockaddr_t addr;
    
    int (*init)(struct simpile_server *server);
    int (*handle)(struct simpile_server *server);
} simpile_server_t;

static inline int
simpile_u_server_init(simpile_server_t *server)
{
    int fd, err;
    
    fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (fd<0) {
    	debug_error("socket error:%d", -errno);
        return -errno;
    }
    os_closexec(fd);
    
    err = bind(fd, &server->addr.c, get_abstract_sockaddr_len(&server->addr.un));
    if (err) {
        debug_error("bind error:%d", -errno);
        return -errno;
    }
    
    server->fd = fd;
    
    return 0;
}

#endif /* __APP__ */
/******************************************************************************/
#endif /* __CMD_H_F3687584F159827DAA20B322924194D1__ */
