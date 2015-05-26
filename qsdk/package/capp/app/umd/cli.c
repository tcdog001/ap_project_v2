#ifndef __THIS_APP
#define __THIS_APP      umd
#endif

#ifndef __THIS_NAME
#define __THIS_NAME     "umd"
#endif

#ifndef __THIS_FILE
#define __THIS_FILE     "cli"
#endif

#define USE_INLINE_SIMPILE_RES

#include "umd.h"

DECLARE_SIMPILE_RES;

/*
* bind {mac} {ip}
*/
static int
handle_bind(char *args)
{
    char *mac   = args; os_cmd_shift(args, 1);
    char *ip    = args; os_cmd_shift(args, 1);

    if (false==is_good_macstring(mac)) {
        debug_trace("bad mac %s", mac);

        return -EFORMAT;
    }
    else if (false==is_good_ipstring(ip)) {
        debug_trace("bad ip %s", ip);

        return -EFORMAT;
    }

    um_user_bind(os_mac(mac), inet_addr(ip));

    return 0;
}

/*
* unbind {mac}
*/
static int
handle_unbind(char *args)
{
    char *mac = args; os_cmd_shift(args, 1);

    if (false==is_good_macstring(mac)) {
        debug_trace("bad mac %s", mac);

        return -EFORMAT;
    }
    
    um_user_unbind(os_mac(mac));
    
    return 0;
}

/*
* auth {mac} {group} {json}
*/
static int
handle_auth(char *args)
{
    char *mac   = args; os_cmd_shift(args, 1);
    char *group = args; os_cmd_shift(args, 1);
    char *json  = args; os_cmd_shift(args, 1);
    int groupid = 0;
    
    if (false==is_good_macstring(mac)) {
        debug_trace("bad mac %s", mac);

        return -EFORMAT;
    }
    else if ((groupid = os_atoi(group)) < 0) {
        debug_trace("bad group %s", group);

        return -EFORMAT;
    }

    jobj_t obj = jobj(json);
    if (NULL==obj) {
        debug_trace("bad json %s", json);

        return -EFORMAT;
    }

    um_user_auth(os_mac(mac), groupid, obj);
    
    jobj_put(obj);
    
    return 0;
}

/*
* deauth {mac}
*/
static int
handle_deauth(char *args)
{
    char *mac   = args; os_cmd_shift(args, 1);
    char *json  = args; os_cmd_shift(args, 1);
    
    if (false==is_good_macstring(mac)) {
        debug_trace("bad mac %s", mac);

        return -EFORMAT;
    }
    else if (json && false==is_good_json(json)) {
        debug_trace("bad json %s", json);

        return -EFORMAT;
    }
    
    um_user_deauth(os_mac(mac), UM_DEAUTH_ADMIN);
    
    return 0;
}

static multi_value_t
show_user(struct um_user *user)
{
    jobj_t obj = um_juser(user);
    
    if (obj) {
        simpile_res_sprintf(__tab "%s" __crlf, jobj_string(obj));
        jobj_put(obj);
    }
    
    return mv2_OK;
}

static multi_value_t
show_user_byjson(struct um_user *user, char *json)
{
    return mv2_OK;
}

static int
show_stat(void)
{
    jobj_t obj = jobj_new_object();
    if (NULL==obj) {
        return -ENOMEM;
    }

    jobj_add_string(obj, "mac", os_macstring(umd.basemac));
    jobj_add_int(obj, "user", umd.head.count);
    jobj_add(obj, "flow", um_jflow());

    simpile_res_sprintf("%s" __crlf, jobj_string(obj));
    
    jobj_put(obj);
    
    return 0;
}

/*
* show [json]
*/
static int
handle_show(char *args)
{
    char *json = args; /* json maybe include space, not shift */
    
    multi_value_t show_user_by(struct um_user *user)
    {
        return show_user_byjson(user, json);
    }
    
    if (NULL==json) {
        return um_user_foreach(show_user);
    }
    else if (is_good_json(json)) {
        return um_user_foreach(show_user_by);
    }
    else if (0==os_strcmp("stat", json)) {
        return show_stat();
    }
    else {
        return -EFORMAT;
    }
}

static int
cli_server_handle(simpile_server_t *server)
{
    static cmd_table_t table[] = {
        CMD_ENTRY("bind",   handle_bind),
        CMD_ENTRY("unbind", handle_unbind),
        CMD_ENTRY("auth",   handle_auth),
        CMD_ENTRY("deauth", handle_deauth),
        CMD_ENTRY("show",   handle_show),
    };
    
    return simpile_d_client(server->fd, table);
}

static int
cli_env_init(simpile_server_t *server)
{
    int err;
    
    err = get_umd_path_env(&server->addr.un);
    if (err) {
        return err;
    }

    return 0;
}

static int
cli_server_init(simpile_server_t *server)
{
    int err;
    
    err = cli_env_init(server);
    if (err) {
        return err;
    }

    err = simpile_u_server_init(server);
    if (err) {
        return err;
    }
    
    debug_ok("init cli server ok");
    
    return 0;
}

simpile_server_t um_cli_server = {
    .fd     = INVALID_FD,
    .addr   = OS_SOCKADDR_INITER(AF_UNIX),

    .init   = cli_server_init,
    .handle = cli_server_handle,
};
/******************************************************************************/
