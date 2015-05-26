/*******************************************************************************
Copyright (c) 2012-2015, Autelan Networks. All rights reserved.
*******************************************************************************/
#ifndef __THIS_APP
#define __THIS_APP      jlogd
#endif
        
#ifndef __THIS_NAME
#define __THIS_NAME     "jlogd"
#endif

#define __JLOGD__

#include "utils.h"

OS_INITER;

#ifdef __BUSYBOX__
DECLARE_REAL_JLOG;
#endif

#ifdef __PC__
#   define SCRIPT_CUT       "./jlogcut"
#   define SCRIPT_PUSH      "./jlogpush"
#   define SCRIPT_GETMAC    "ip link show eth0 | grep link | awk '{print $2}'"
#else
#   define SCRIPT_CUT       "jlogcut"
#   define SCRIPT_PUSH      "jlogpush"
#   define SCRIPT_GETMAC    "ifconfig | grep 'eth0 ' | awk '{print $5}'"
#endif

static char RX[1 + JLOG_BUFSIZE];

typedef struct {
    char *name;
    int family;
    int fd;
    os_sockaddr_t addr;
    
    struct {
        FILE *stream;
        
        env_string_t *envar;

        unsigned int count;
    } log, cache;
} jlog_server_t;

#define JLOG_SERVER(_name, _family, _var_log, _var_cache) { \
    .name       = _name,        \
    .family     = _family,      \
    .fd         = INVALID_FD,   \
    .addr = {                   \
        .c  = {                 \
            .sa_family=_family, \
        }                       \
    },                          \
    .log  = {                   \
        .envar  = _var_log,     \
    },                          \
    .cache  = {                 \
        .envar  = _var_cache,   \
    },                          \
}   /* end */

static env_string_t envar_ulog  = env_var_initer(ENV_JLOG_FILE_LOCAL, JLOG_FILE_LOCAL);
static env_string_t envar_ucache= env_var_initer(ENV_JLOG_CACHE_LOCAL, JLOG_CACHE_LOCAL);

static env_string_t envar_ilog  = env_var_initer(ENV_JLOG_FILE_REMOTE, JLOG_FILE_REMOTE);
static env_string_t envar_icache= env_var_initer(ENV_JLOG_CACHE_REMOTE, JLOG_CACHE_REMOTE);

enum {
    JLOG_USERVER,
    JLOG_ISERVER,
    
    JLOG_END_SERVER
};

typedef struct {
    int pri;
    int timeout;
    int cut;
    unsigned int seq;
    unsigned int event;
    char mac[1+MACSTRINGLEN_L];

    jlog_server_t server[JLOG_END_SERVER];
} jlogd_t;

static jlogd_t jlogd = {
    .pri    = JLOG_PRI,
    .timeout= JLOG_TIMEOUT,
    .cut    = JLOG_CUTCOUNT,
    .mac    = {0},
    .server = {
        [JLOG_USERVER] = JLOG_SERVER("userver", AF_UNIX, &envar_ulog, &envar_ucache),
        [JLOG_ISERVER] = JLOG_SERVER("iserver", AF_INET, &envar_ilog, &envar_icache),
    },
};

#define jlogserver0         (&jlogd.server[0])
#define jlogservermax       (&jlogd.server[os_count_of(jlogd.server)])

#define foreach_server(_server) \
    for(_server=jlogserver0; _server<jlogservermax; _server++)

static int
jcut(jlog_server_t *server, int cut)
{
    /*
    * open cache file
    */
    if (NULL==server->cache.stream) {
        server->cache.stream = os_fopen(server->cache.envar->value, "a+");
        if (NULL==server->cache.stream) {
        	debug_error("open file:%s error:%d", server->cache.envar->value, -errno);
            return -errno;
        }

        debug_trace("open %s", server->cache.envar->value);
    }
    
    /*
    * cut log file
    */
    if ((0==cut || server->log.count >= cut) && server->log.stream) {
        os_fclose(server->log.stream);
        
        os_system(SCRIPT_CUT " %s %d", server->log.envar->value, server->log.count);
        debug_trace("cut %s", server->log.envar->value);
        
        os_fclean(server->log.envar->value);
        server->log.count = 0;
    }
    
    /*
    * reopen log file
    */
    if (NULL==server->log.stream) {
        server->log.stream = os_fopen(server->log.envar->value, "a+");
        if (NULL==server->log.stream) {
        	debug_error("open file:%s error:%d", server->log.envar->value, -errno);
            return -errno;
        }

        debug_trace("open %s", server->log.envar->value);
    }

    return 0;
}

static void
jtrycut(void)
{
    if (os_hasbit(jlogd.event, SIGUSR1)) {
        jlog_server_t *server;
        
        os_clrbit(jlogd.event, SIGUSR1);

        foreach_server(server) {
            jcut(server, 0);
        }
    }
}

static int
jmac(void)
{
    if (0==jlogd.mac[0]) {
        char line[1+OS_LINE_LEN] = {0};
        int err;
        
        err = os_v_spgets(line, OS_LINE_LEN, SCRIPT_GETMAC);
        if (err || 0==line[0]) {
            __debug_error("no-found mac, err:%d, line:%s", err, line);
            
            return -EBADMAC;
        } else {
            os_strmcpy(jlogd.mac, line, MACSTRINGLEN_L);

            return 0;
        }
    } else {        
        return 0;
    }
}

static int
jadd(jlog_server_t *server)
{
    int len = 0, pri, err;
    jobj_t obj, opri;
    
    obj = jobj(RX);
    if (NULL==obj) {
        len = -EFORMAT; goto error;
    }

    opri = jobj_get(obj, JLOG_KEY_PRI);
    if (NULL==opri) {
        __debug_error("no-found pri");
        
        len = -EFORMAT; goto error;
    }
    pri = LOG_PRI(jobj_get_int(opri));
    
    err = jobj_add_string(obj, JLOG_KEY_TIME, get_full_time(NULL));
    if (err) {
        __debug_error("add time error");
        
        len = err; goto error;
    }
    
    err = jobj_add_int(obj, JLOG_KEY_SEQ, ++jlogd.seq);
    if (err) {
        __debug_error("add seq error");
        
        len = err; goto error;
    }

    if (0==jmac()) {
        err = jobj_add_string(obj, JLOG_KEY_MAC, jlogd.mac);
        if (err) {
            __debug_error("add mac error");
            
            len = err; goto error;
        }
    }

    char *json = jobj_string(obj);
    if (NULL==json) {
        __debug_error("obj==>json failed");
        
        len = -EFORMAT; goto error;
    }
    len = os_strlen(json);
    
    os_strmcpy(RX, json, len);
    
    if (pri <= jlogd.pri) {
        os_system(SCRIPT_PUSH " %s &", RX);
    }
    
error:
    jobj_put(obj);
    
    return len;
}

static int
jhandle(jlog_server_t *server)
{
    int err, len;
    os_sockaddr_t client = OS_SOCKADDR_INITER(server->family);
    socklen_t addrlen = os_sockaddr_len(&client.c);
    
    len = __io_recvfrom(server->fd, RX, sizeof(RX), &client.c, &addrlen);
    if (len<0) {
        debug_error("read error:%d", len);
        return len;
    }

    if (is_abstract_sockaddr(&client.c)) {
        set_abstract_sockaddr_len(&client.un, addrlen);

        debug_trace("recv from:%s", get_abstract_path(&client.un));
    }
    
    RX[len] = 0;
    debug_trace("read:%s, len:%d", RX, len);

    err = jadd(server);
    if (err<0) { /* yes, <0 */
        /* log */
    } else {
        len = err;
    }

    RX[len++] = '\n';
    RX[len] = 0;

    err = os_fwrite(server->log.stream, RX, len);
        debug_trace_error(err, "write %s", server->log.envar->value);
    err = os_fwrite(server->cache.stream, RX, len);
        debug_trace_error(err, "write %s", server->cache.envar->value);

    server->log.count++;
    server->cache.count++;

    if (server->log.count >= jlogd.cut) {
        return jcut(server, jlogd.cut);
    } else {
        return 0;
    }
}

static int
__server_handle(fd_set *r)
{
    jlog_server_t *server;
    int err = 0;

    foreach_server(server) {
        if (FD_ISSET(server->fd, r)) {
            err = jhandle(server);
        }
    }

    jtrycut();
    
    return err;
}

static int
server_handle(void)
{
    fd_set rset;
    struct timeval tv = {
        .tv_sec     = time_sec(jlogd.timeout),
        .tv_usec    = time_usec(jlogd.timeout),
    };
    jlog_server_t *server;
    int err, fdmax = 0;
    
    FD_ZERO(&rset);
    foreach_server(server) {
        FD_SET(server->fd, &rset);
        fdmax = os_max(fdmax, server->fd);
    }
    
    while(1) {
        err = select(fdmax+1, &rset, NULL, NULL, &tv);
        switch(err) {
            case -1:/* error */
                if (EINTR==errno) {
                    // is breaked
                    debug_event("select breaked");

                    jtrycut();
                    
                    continue;
                } else {
                    debug_error("select error:%d", -errno);
                    return -errno;
                }
            case 0: /* timeout, retry */
                debug_timeout("select timeout");
                return -ETIMEOUT;
            default: /* to read */
                return __server_handle(&rset);
        }
    }
}

static int
init_env(void) 
{
    jlog_server_t *server;
    int err;
    
    jlogd.pri       = get_int_env(ENV_JLOG_PRI,     JLOG_PRI);
    jlogd.timeout   = get_int_env(ENV_JLOG_TIMEOUT, JLOG_TIMEOUT);
    jlogd.cut       = get_int_env(ENV_JLOG_CUTCOUNT,  JLOG_CUTCOUNT);

    foreach_server(server) {
        err = env_string_init(server->log.envar);
        if (err) {
            return err;
        }
        
        err = env_string_init(server->cache.envar);
        if (err) {
            return err;
        }
    }

    err = __copy_string_env(ENV_JLOG_UNIX, JLOG_UNIX, 
            get_abstract_path(&jlogd.server[JLOG_USERVER].addr.un), 
            abstract_path_size);
    if (err) {
        return err;
    }

    jlogd.server[JLOG_ISERVER].addr.in.sin_port = get_int_env(ENV_JLOG_PORT, JLOG_PORT);
    {
        unsigned int ip;
        char ipaddress[32] = {0};

        err = copy_string_env(ENV_JLOG_IP, JLOG_IP, ipaddress);
        if (err) {
            return err;
        }
        
        ip = inet_addr(ipaddress);
        if (INADDR_NONE==ip) {
            return -EFORMAT;
        }
        
        jlogd.server[JLOG_ISERVER].addr.in.sin_addr.s_addr = ip;
    }
    
    return 0;
}

static int
init_server(jlog_server_t *server)
{
    int fd, err, protocol, addrlen;

    err = jcut(server, jlogd.cut);
    if (err) {
        return err;
    }

    if (AF_UNIX==server->family) {
        protocol    = 0;
        addrlen     = get_abstract_sockaddr_len(&server->addr.un);
    } else {
        protocol = IPPROTO_UDP;
        addrlen     = sizeof(struct sockaddr_in);
    }
    
    fd = socket(server->family, SOCK_DGRAM, protocol);
    if (false==is_good_fd(fd)) {
    	debug_error("socket error:%d", -errno);
        return -errno;
    }
    os_closexec(fd);

    err = bind(fd, &server->addr.c, addrlen);
    if (err) {
        debug_error("bind error:%d", -errno);
        return -errno;
    }
    
    server->fd = fd;
    
    return 0;
}

static int 
init_all_server(void)
{
    jlog_server_t *server;
    int err;

    foreach_server(server) {
        err = init_server(server);
            debug_trace_error(err, "init %s", server->name);
        if (err < 0) {
            return err;
        }
    }
    
    return 0;
}


static void
usr1(int sig)
{
    debug_signal("recv USR1");
    
    if (false==os_hasbit(jlogd.event, SIGUSR1)) {
        os_setbit(jlogd.event, SIGUSR1);
    }
}

static void
setup(void)
{
    int sig_user[] = {SIGUSR1};

    os_setup_signal(usr1, sig_user);

    os_setup_signal_default(NULL);
}

static int 
__main(int argc, char *argv[])
{
    int err;

    setup();
    
    __os_system("mkdir -p " JLOG_PATH);
    
    err = init_env();
        debug_trace_error(err, "init env");
    if (err) {
        return shell_error(err);
    }

    err = init_all_server();
    if (err) {
        return shell_error(err);
    }
    
    while (1) {
        server_handle();
    }
    
    return 0;
}

#ifndef __BUSYBOX__
#define jlogd_main  main
#endif

int jlogd_main(int argc, char *argv[])
{
    return os_main(__main, argc, argv);
}
/******************************************************************************/
