/*******************************************************************************
Copyright (c) 2012-2015, Autelan Networks. All rights reserved.
*******************************************************************************/
#ifndef __THIS_APP
#define __THIS_APP      smonitord
#endif

#ifndef __THIS_NAME
#define __THIS_NAME     "smonitord"
#endif

#define USE_INLINE_SIMPILE_RES

#include "smonitor/smonitor.h"

OS_INITER;
DECLARE_SIMPILE_RES;

#if 0
static simpile_server_t cli_server = {
    .fd     = INVALID_FD,
    .addr   = OS_SOCKADDR_UN_INITER(SMONITORD_UNIX),
    
    .init   = cli_server_init,
    .handle = cli_server_handle,
};
#endif

static struct {
    struct {
        int fd;
        
        struct sockaddr_un addr;
    } server;

    int timeout;
    
    struct list_head list;
}
smonitord = {
    .server = {
        .fd     = -1,
        .addr   = OS_SOCKADDR_UNIX("\0" SMONITORD_UNIX),
    },

    .timeout = SMONITOR_TIMEOUT,
    
    .list = LIST_HEAD_INIT(smonitord.list),
};

struct smonitor {
    char name[1+SMONITOR_NAMESIZE];
    char command[1+SMONITOR_CMDSIZE];

    int pid;
    int forks;
    bool dynamic;
    
    struct list_head node;
};

#define DAEMONS(_name, _command) {  \
    .name   = _name,                \
    .command= _command,             \
    .dynamic= false,                \
}   /* end */

static struct smonitor daemons[] = {
#if 1
    /* insert static daemon */
#endif
};

static void
__dump_all(char *name)
{
    struct smonitor *entry;

    if (__is_debug_level_trace) {
        list_for_each_entry(entry, &smonitord.list, node) {
            debug_trace("entry:%s pid:%d forks:%d command:%s ", 
                entry->name,
                entry->pid,
                entry->forks,
                entry->command);
        }
    }
}

static struct smonitor *
__get_byname(char *name)
{
    struct smonitor *entry;
    
    if (NULL==name) {
        return NULL;
    }

    list_for_each_entry(entry, &smonitord.list, node) {
        if (0==os_strcmp(name, entry->name)) {
            return entry;
        }
    }
    
    return NULL;
}

static struct smonitor *
__get_bypid(int pid)
{
    struct smonitor *entry;
    
    list_for_each_entry(entry, &smonitord.list, node) {
        if (pid==entry->pid) {
            return entry;
        }
    }

    return NULL;
}

static int
__run(struct smonitor *entry)
{
    int err, pid;
    simpile_cmd_t *cmd = simpile_cmd_argv(entry->command);
    
    pid = vfork();
    if (pid<0) {
        debug_trace("fork error:%d", errno);
        
        return -errno;
    }
    else if (pid>0) { // father
        entry->pid = pid;
        entry->forks++;

        jinfo("%o", 
            "exec", jobj_oprintf("%s%d%d%s",
                        "name",     entry->name,
                        "pid",      entry->pid,
                        "forks",    entry->forks,
                        "command",  entry->command));

        return 0;
    }
    else { // child
        err = execvp(cmd->argv[0], cmd->argv);
        
        debug_error("exec %s error:%d", cmd->argv[0], errno);
        
        exit(err);
    }
}

static int
__kill(struct smonitor *entry)
{
    int err;

    err = kill(entry->pid, SIGKILL);
    if (err) {
        return err;
    }
    waitpid(entry->pid, NULL, WNOHANG);
    
    debug_trace("kill %d:%s:%s", entry->pid, entry->name, entry->command);
    
    return 0;
}

static int
__insert(struct smonitor *entry)
{
    int err;
    
    if (NULL==entry) {
        return -EKEYNULL;
    }
    else if (is_in_list(&entry->node)) {
        return -EINLIST;
    }
    
    list_add(&entry->node, &smonitord.list);

    debug_trace("insert %s:%s", entry->name, entry->command);
    
    err = __run(entry);
    if (err) {
        return err;
    }
    
    return 0;
}

static int
__remove(struct smonitor *entry)
{
    int err;
    
    if (NULL==entry) {
        return 0;
    }
    else if (false==entry->dynamic) {
        return 0;
    }
    else if (false==is_in_list(&entry->node)) {
        return -ENOINLIST;
    }

    list_del(&entry->node);

    debug_trace("remove %s:%s", entry->name, entry->command);
    
    err = __kill(entry); os_free(entry);

    return err;
}

static struct smonitor *
__create(char *name, char *command)
{
    struct smonitor *entry = __get_byname(name);
    
    if (NULL==entry) {
        /*
        * no found, create new
        */
        entry = (struct smonitor *)os_zalloc(sizeof(*entry));
        if (NULL==entry) {            
            return NULL;
        }

        os_strdcpy(entry->name, name);
    } else {
        /*
        * have exist, do nothing
        */
        debug_trace("%s exist", name);
        
        return entry;
    }
    
    os_strdcpy(entry->command, command);
    entry->dynamic = true;
    
    __insert(entry);
    
    return entry;
}

static int
handle_insert(char *args)
{
    char *name      = args; os_cmd_shift(args, 1);
    char *command   = args;
    int err;

    if (NULL==name      ||
        NULL==command) {
        debug_error("NULL name|command");
        
        return -EINVAL1;
    }
    
    struct smonitor *entry = __create(name, command);
    if (NULL==entry) {
        return -ENOMEM;
    }
    
    return 0;
}

static int
handle_remove(char *args)
{
    char *name = args; os_cmd_shift(args, 1);
    if (NULL==name) {
        debug_trace("remove monitor without name");
        
        return -EINVAL3;
    }
    
    struct smonitor *entry = __get_byname(name);
    if (NULL==entry) {
        return -ENOEXIST;
    }

    __remove(entry);
    
    return 0;
}

static void
show(struct smonitor *entry)
{
    simpile_res_sprintf("%s %d%c %d %s" __crlf,
        entry->name,
        entry->pid,
        entry->dynamic?' ':'*',
        entry->forks,
        entry->command);
}

static int
handle_show(char *args)
{
    char *name = args; os_cmd_shift(args, 1);
    struct smonitor *entry;
    bool empty = true;
    
    simpile_res_sprintf("#name pid forks command" __crlf);
    
    list_for_each_entry(entry, &smonitord.list, node) {
        show(entry);

        empty = false;
    }

    if (empty) {
        /*
        * drop head line
        */
        simpile_res_clear();
        
        debug_trace("show monitor(%s) nofound", name);
    }
    
    return 0;
}

static int
__server_handle(fd_set *r)
{
    static cmd_table_t table[] = {
        CMD_ENTRY("insert",  handle_insert),
        CMD_ENTRY("remove",  handle_remove),
        CMD_ENTRY("show",    handle_show),
    };
    int err = 0;

    if (FD_ISSET(smonitord.server.fd, r)) {
        err = simpile_d_client(smonitord.server.fd, table);
    }
    
    return err;
}

static int
server_handle(void)
{
    fd_set rset;
    struct timeval tv = {
        .tv_sec     = time_sec(smonitord.timeout),
        .tv_usec    = time_usec(smonitord.timeout),
    };
    int err;
    
    FD_ZERO(&rset);
    FD_SET(smonitord.server.fd, &rset);

    while(1) {
        err = select(smonitord.server.fd+1, &rset, NULL, NULL, &tv);
        switch(err) {
            case -1:/* error */
                if (EINTR==errno) {
                    // is breaked
                    debug_event("select breaked");
                    continue;
                } else {
                    debug_trace("select error:%d", -errno);
                    return -errno;
                }
            case 0: /* timeout, retry */
                debug_timeout("select timeout");
                return -ETIMEOUT;
            default: /* to accept */
                return __server_handle(&rset);
        }
    }

    return 0;
}

static int
wait_handle(void)
{
    int pid;
    struct smonitor *entry;

    while((pid = waitpid(-1, NULL, WNOHANG)) > 0) {       
        entry = __get_bypid(pid);
        if (NULL==entry) {
            debug_trace("wait pid:%d, but no-found entry, maybe have been removed", pid);

            continue;
        }

        __run(entry);
    }
    
    return 0;
}

static int
init_env(void) 
{
    int err;
    
    smonitord.timeout = get_smonitor_timeout_env();

    err = get_smonitord_path_env(&smonitord.server.addr);
    if (err) {
        return err;
    }

    return 0;
}

static int
init_server(void)
{
    int fd;
    int err;
    
    fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (fd<0) {
    	debug_error("socket error:%d", -errno);
        return -errno;
    }
    os_closexec(fd);
    
    err = bind(fd, (struct sockaddr *)&smonitord.server.addr, get_abstract_sockaddr_len(&smonitord.server.addr));
    if (err) {
        debug_error("bind error:%d", -errno);
        return -errno;
    }
    
    smonitord.server.fd = fd;
    
    debug_ok("init server ok");
    
    return 0;
}

static int
load(void)
{
    int i;

    /*
    * load static
    */
    for (i=0; i<os_count_of(daemons); i++) {
        debug_trace("load static %s", daemons[i].name);
        
        __insert(&daemons[i]);
    }

    return 0;
}

static int 
__main(int argc, char *argv[])
{
    int err;
    
    err = init_env();
    if (err) {
        return shell_error(err);
    }
    
    err = init_server();
    if (err) {
        return shell_error(err);
    }
    
    load();
    
    while (1) {
        server_handle();
        wait_handle();
    }
    
    return 0;
}

#ifndef __BUSYBOX__
#define smonitord_main  main
#endif

int smonitord_main(int argc, char *argv[])
{
    return os_main(__main, argc, argv);
}
/******************************************************************************/
