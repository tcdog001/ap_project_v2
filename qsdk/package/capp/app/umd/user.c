#ifndef __THIS_APP
#define __THIS_APP      umd
#endif
    
#ifndef __THIS_NAME
#define __THIS_NAME     "umd"
#endif

#ifndef __THIS_FILE
#define __THIS_FILE     "user"
#endif

#include "umd.h"

static void
create_cb(struct um_user *user)
{
#if 0
    os_vsystem(UM_SCRIPT_EVENT " create %s &",
            os_macstring(user->mac));
#endif
}

static void
bind_cb(struct um_user *user)
{
    os_system(UM_SCRIPT_EVENT " bind %s %s",
            os_macstring(user->mac),
            os_ipstring(user->ip));
}

static void
unbind_cb(struct um_user *user)
{
    os_system(UM_SCRIPT_EVENT " unbind %s %s",
            os_macstring(user->mac),
            os_ipstring(user->ip));
}

static void
auth_cb(struct um_user *user, int group, char *json)
{
    os_system(UM_SCRIPT_EVENT " auth %s %s %d %s",
            os_macstring(user->mac),
            os_ipstring(user->ip),
            group,
            json);
}

static void
deauth_cb(struct um_user *user, int reason)
{
    os_system(UM_SCRIPT_EVENT " deauth %s %s %s",
        os_macstring(user->mac),
        os_ipstring(user->ip),
        deauth_reason_string(reason));
}

#define __update_online(_user, _obj, _TYPE, _field) do { \
    limit_online(_user, _TYPE)->_field = \
        jobj_get_leaf_int(_obj, #_TYPE, "online", #_field, NULL); \
}while(0)

#define update_online(_user, _obj, _field)  do{ \
    __update_online(_user, _obj, lan, _field);  \
    __update_online(_user, _obj, wan, _field);  \
}while(0)

#define __update_flow(_user, _obj, _TYPE, _DIR, _field) do{ \
    limit_flow(_user, _TYPE, _DIR)->_field = \
        1000*jobj_get_leaf_int64(_obj, #_TYPE, "flow", #_DIR, #_field, NULL); \
}while(0)

#define updata_flow(_user, _obj, _field)        do{ \
    __update_flow(_user, _obj, lan, up, _field);    \
    __update_flow(_user, _obj, wan, up, _field);    \
    __update_flow(_user, _obj, lan, down, _field);  \
    __update_flow(_user, _obj, wan, down, _field);  \
    __update_flow(_user, _obj, lan, all, _field);   \
    __update_flow(_user, _obj, wan, all, _field);   \
}while(0)

#define __update_rate(_user, _obj, _TYPE, _DIR, _field) do{ \
    limit_rate(_user, _TYPE, _DIR)->_field = \
        jobj_get_leaf_int(_obj, #_TYPE, "rate", #_DIR, #_field, NULL); \
}while(0)

#define updata_rate(_user, _obj, _field)        do{ \
    __update_rate(_user, _obj, lan, up, _field);    \
    __update_rate(_user, _obj, wan, up, _field);    \
    __update_rate(_user, _obj, lan, down, _field);  \
    __update_rate(_user, _obj, wan, down, _field);  \
    __update_rate(_user, _obj, lan, all, _field);   \
    __update_rate(_user, _obj, wan, all, _field);   \
}while(0)

static void
__limit_update(struct um_user *user, jobj_t obj)
{
    update_online(user, obj, max);
    
    updata_flow(user, obj, max);
    updata_flow(user, obj, now);
    
    updata_rate(user, obj, max);
    updata_rate(user, obj, avg);
}

void limit_update_test(void)
{
#if UM_TEST & UM_TEST_JSON
    struct um_user user = {
        .ip = umd.intf[UM_INTF_TC].ip,
    };
    os_maccpy(user.mac, umd.intf[UM_INTF_TC].mac);
    
    jobj_t obj = jobj_file("./auth.json");
    if (NULL==obj) {
        os_println("read auth.json failed");

        exit(1);
    }
    
    __limit_update(&user, obj);
    jobj_put(obj);

    obj = um_juser(&user);
    os_println("%s", jobj_string(obj));
    os_system("echo '%s' | jq .", jobj_string(obj));
    jobj_put(obj);
#endif
}

static int
__hashbuf(unsigned char *buf, int len)
{
    int i;
    int sum = 0;
    
    for (i=0; i<len; i++) {
        sum += (int)buf[i];
    }

    return sum & UM_HASHMASK;
}

static inline int
hashmac(unsigned char mac[])
{
    return __hashbuf(mac, OS_MACSIZE);
}

static inline int
haship(uint32_t ip)
{
    return __hashbuf((unsigned char *)&ip, sizeof(ip));
}

static inline struct hlist_head *
headmac(unsigned char mac[])
{
    return &umd.head.mac[hashmac(mac)];
}

static inline struct hlist_head *
headip(uint32_t ip)
{
    return &umd.head.ip[haship(ip)];
}

static time_t
lan_uptime(struct um_user *user)
{
    return time(NULL);
}

static time_t
wan_uptime(struct um_user *user)
{
    return time(NULL);
}

static void
lan_online(struct um_user *user)
{
    if (0==__online_uptime(user, um_flow_type_lan)) {
        time_t now = lan_uptime(user);
        
        __online_uptime(user, um_flow_type_lan) = now;
    
        debug_event("user %s lan online", os_macstring(user->mac));
    }
}

static void
wan_online(struct um_user *user)
{
    if (0==__online_uptime(user, um_flow_type_wan)) {
        time_t now = wan_uptime(user);
        
        __online_uptime(user, um_flow_type_wan) = now;
        
        debug_event("user %s wan online", os_macstring(user->mac));
    }
}

static void
lan_offline(struct um_user *user)
{
    __online_uptime(user, um_flow_type_lan) = 0;
        
    debug_event("user %s lan offline", os_macstring(user->mac));
}

static void
wan_offline(struct um_user *user)
{
    __online_uptime(user, um_flow_type_wan) = 0;
        
    debug_event("user %s wan offline", os_macstring(user->mac));
}

static inline void
add_iphash(struct um_user *user)
{
    hlist_add_head(&user->node.ip,  headip(user->ip));
}

static inline void
del_iphash(struct um_user *user)
{
    hlist_del_init(&user->node.ip);
}

static inline void
add_machash(struct um_user *user)
{
    hlist_add_head(&user->node.mac, headmac(user->mac));
}

static inline void
del_machash(struct um_user *user)
{
    hlist_del_init(&user->node.mac);
}

static struct um_user *
__user_remove(struct um_user *user)
{
    if (NULL==user) {
        debug_bug("user nil");
        
        return NULL;
    }
    /*
    * not in list
    */
    else if (false==is_in_list(&user->node.list)) {
        debug_bug("user not in list");
        
        return user;
    }
    
    list_del(&user->node.list);
    if (is_good_mac(user->mac)) {
        del_machash(user);
        
    }
    if (user->ip) {
        del_iphash(user);
    }
    umd.head.count--;
    
    um_user_dump(user, "remove");

    return user;
}

static struct um_user *
__user_insert(struct um_user *user)
{
    if (NULL==user) {
        debug_bug("user nil");
        
        return NULL;
    }
    /*
    * have in list
    */
    else if (is_in_list(&user->node.list)) {
        debug_bug("user have in list");
        
        __user_remove(user);
    }
    
    list_add(&user->node.list, &umd.head.list);
    if (is_good_mac(user->mac)) {
        add_machash(user);
    }
    if (user->ip) {
        add_iphash(user);
    }
    umd.head.count++;
    
    um_user_dump(user, "insert");
    
    return user;
}

static void
__set_group(struct um_user *user, int group)
{
    if (NULL==user) {
        debug_bug("user nil");
        
        return;
    }
    else if (group==user->group) {
        return;
    }
    
    user->group = group;
}

static void
__set_ip(struct um_user *user, uint32_t ip)
{
    if (NULL==user) {
        debug_bug("user nil");
        
        return;
    }
    else if (ip==user->ip) {
        return;
    }
    
    if (user->ip) {
        del_iphash(user);
    }
    user->ip = ip;
    if (user->ip) {
        add_iphash(user);
    }
}

static void
__set_state(struct um_user *user, int state)
{
    if (NULL==user) {
        debug_bug("user nil");
        
        return;
    }
    else if (state==user->state) {
        return;
    }

    char *new = user_state_string(state);
    char *old = user_state_string(user->state);
    
    debug_trace("user %s state change %s==>%s", 
        os_macstring(user->mac), 
        old, new);

    user->state = state;
}

static struct um_user *
__user_create(unsigned char mac[], void (*cb)(struct um_user *user))
{
    struct um_user *user;

    user = (struct um_user *)os_zalloc(sizeof(*user));
    if (NULL==user) {
        return NULL;
    }
    os_maccpy(user->mac, mac);

    __set_state(user, UM_STATE_NONE);
    
    lan_online(user);
    
    INIT_HLIST_NODE(&user->node.mac);
    INIT_HLIST_NODE(&user->node.ip);
    INIT_LIST_HEAD(&user->node.list);
    
    __user_insert(user);

    if (cb) {
        (*cb)(user);
    }
    
    return user;
}

static void
__user_deauth(struct um_user *user, int reason, void (*cb)(struct um_user *user, int reason))
{
    if (NULL==user) {
        return;
    }
    else if (UM_STATE_AUTH != user->state) {
        return;
    }
    else if (cb) {
        (*cb)(user, reason);
    }
    
    /*
    * auth==>bind
    */
    __set_state(user, UM_STATE_BIND);
    __set_group(user, 0);
    
    wan_offline(user);
}

static void
__user_unbind(struct um_user *user, void (*cb)(struct um_user *user))
{
    __user_deauth(user, UM_DEAUTH_INITIATIVE, deauth_cb);
    
    if (NULL==user) {
        return;
    }
    else if (UM_STATE_BIND != user->state) {
        return;
    }
    else if (cb) {
        (*cb)(user);
    }
    
    __set_ip(user, 0);
    /*
    * bind==>connect
    */
    __set_state(user, UM_STATE_NONE);
    lan_offline(user);
}

static struct um_user *
__user_bind(struct um_user *user, uint32_t ip, void (*cb)(struct um_user *user))
{
    if (NULL==user) {
        return NULL;
    }

    if (have_bind(user->state)) {
        if (ip==user->ip) {
            return user;
        } else {
            __user_unbind(user, unbind_cb);
        }
    }
    
    __set_ip(user, ip);
    __set_state(user, UM_STATE_BIND);
    
    lan_online(user);
    
    if (cb) {
        (*cb)(user);
    }

    return user;
}

static struct um_user *
__user_auth(struct um_user *user, int group, jobj_t obj, void (*cb)(struct um_user *user, int group, char *json))
{
    int err;
    
    if (NULL==user) {
        return NULL;
    }

    if (no_bind(user->state)) {
        char ipaddress[1+OS_LINE_LEN];
        
        err = os_v_spgets(ipaddress, OS_LINE_LEN, UM_SCRIPT_IP " %s", os_macstring(user->mac));
        if (err) {
            return NULL;
        }
        else if (false==is_good_ipstring(ipaddress)) {
            return NULL;
        }
        else {
            __user_bind(user, inet_addr(ipaddress), bind_cb);
        }
    }
    
    /*
    * bind==>auth
    */
    __set_state(user, UM_STATE_AUTH);
    __set_group(user, group);
    user->aging = UM_AGING;
    wan_online(user);
    
    __limit_update(user, obj);
    
    if (cb) {
        (*cb)(user, group, jobj_string(obj));
    }

    return user;
}

static struct um_user *
__user_get(unsigned char mac[])
{
    struct um_user *user;
    
    hlist_for_each_entry(user, headmac(mac), node.mac) {
        if (os_maceq(user->mac, mac)) {
            return user;
        }
    }

    return NULL;
}

static struct um_user *
user_get(unsigned char mac[])
{
    struct um_user *user = __user_get(mac);
    if (NULL==user) {
        debug_trace("no-found user %s", os_macstring(mac));
    }

    return user;
}

static struct um_user *
user_create(unsigned char mac[])
{
    return __user_create(mac, create_cb);
}

static struct um_user *
user_bind(struct um_user *user, uint32_t ip)
{
    return __user_bind(user, ip, bind_cb);
}

static void
user_unbind(struct um_user *user)
{
    __user_unbind(user, unbind_cb);
}

static struct um_user *
user_auth(struct um_user *user, int group, jobj_t obj)
{
    return __user_auth(user, group, obj, auth_cb);
}

void
user_deauth(struct um_user *user, int reason)
{
    if (is_good_deauth_reason(reason)) {
        __user_deauth(user, reason, deauth_cb);
    }
}

struct um_user *
um_user_create(unsigned char mac[])
{
    struct um_user *user = __user_get(mac);

    return user?user:user_create(mac);
}

struct um_user *
um_user_bind(unsigned char mac[], uint32_t ip)
{
    return user_bind(um_user_create(mac), ip);
}

void
um_user_unbind(unsigned char mac[])
{
    user_unbind(user_get(mac));
}

struct um_user *
um_user_auth(unsigned char mac[], int group, jobj_t obj)
{
    return user_auth(user_get(mac), group, obj);
}

void
um_user_deauth(unsigned char mac[], int reason)
{
    user_deauth(user_get(mac), reason);
}

int
um_user_foreach(um_foreach_f *foreach)
{
    multi_value_u mv;
    struct um_user *user;
    
    list_for_each_entry(user, &umd.head.list, node.list) {
        mv.value = (*foreach)(user);
        
        if (mv2_is_break(mv)) {
            return mv2_result(mv);
        }
    }
    
    return 0;
}

int
um_user_foreach_safe(um_foreach_f *foreach)
{
    multi_value_u mv;
    struct um_user *user, *n;
    
    list_for_each_entry_safe(user, n, &umd.head.list, node.list) {
        mv.value = (*foreach)(user);
        
        if (mv2_is_break(mv)) {
            return mv2_result(mv);
        }
    }
    
    return 0;
}

struct um_user *
um_user_get(unsigned char mac[])
{
    return __user_get(mac);
}

struct um_user *
um_user_getbyip(uint32_t ip)
{
    struct um_user *user;
    
    hlist_for_each_entry(user, headip(ip), node.ip) {
        if (user->ip==ip) {
            return user;
        }
    }

    return NULL;
}

int
um_user_del(struct um_user *user)
{
    /*
    * just set disconnect, NOT free
    */
    user_unbind(user);
    
    return 0;
}

int
um_user_delbymac(unsigned char mac[])
{
    return um_user_del(user_get(mac));
}

int
um_user_delbyip(uint32_t ip)
{
    return um_user_del(um_user_getbyip(ip));
}

static jobj_t
juser_online(struct um_user *user, int type)
{
    jobj_t obj = jobj_new_object();
    
    jobj_add_int(obj, "max", __online_max(user, type));

    return obj;
}

static jobj_t
__juser_flow(struct um_user *user, int type, int dir)
{
    jobj_t obj = jobj_new_object();
    
    jobj_add_int64(obj, "max", __flow_max(user, type, dir));
    jobj_add_int64(obj, "now", __flow_now(user, type, dir));

    return obj;
}

static jobj_t
juser_flow(struct um_user *user, int type)
{
    jobj_t obj = jobj_new_object();
    int dir;

    for (dir=0; dir<um_flow_dir_end; dir++) {
        jobj_add(obj, flow_dir_string(dir), __juser_flow(user, type, dir));
    }

    return obj;
}

static jobj_t
__juser_rate(struct um_user *user, int type, int dir)
{
    jobj_t obj = jobj_new_object();
    
    jobj_add_int(obj, "max", __rate_max(user, type, dir));
    jobj_add_int(obj, "avg", __rate_avg(user, type, dir));

    return obj;
}

static jobj_t
juser_rate(struct um_user *user, int type)
{
    jobj_t obj = jobj_new_object();
    int dir;

    for (dir=0; dir<um_flow_dir_end; dir++) {
        jobj_add(obj, flow_dir_string(dir), __juser_rate(user, type, dir));
    }

    return obj;
}

static jobj_t
__juser_limit(struct um_user *user, int type)
{
    jobj_t obj = jobj_new_object();

    jobj_add(obj, "online", juser_online(user, type));
    jobj_add(obj, "flow", juser_flow(user, type));
    jobj_add(obj, "rate", juser_rate(user, type));

    return obj;
}

static jobj_t
juser_limit(struct um_user *user)
{
    jobj_t obj = jobj_new_object();
    int type;

    for (type=0; type<um_flow_type_end; type++) {
        jobj_add(obj, flow_type_string(type), __juser_limit(user, type));
    }

    return obj;
}

jobj_t
um_juser(struct um_user *user)
{
    jobj_t obj = jobj_new_object();

    jobj_add_string(obj, "mac",    os_macstring(user->mac));
    jobj_add_string(obj, "ip",     os_ipstring(user->ip));
    jobj_add_string(obj, "state",  user_state_string(user->state));
    jobj_add_string(obj, "reason", deauth_reason_string(user->reason));
    
    jobj_add(obj, "limit", juser_limit(user));

    return obj;
}

void
um_user_dump(struct um_user *user, char *action)
{
    if (__is_debug_level_event && __is_debug_level_entry) {
        os_println("=====%s user begin======", action);
        os_println("%s", jobj_string(um_juser(user)));
        os_println("=====%s user end(%d)====", action, umd.head.count);
        os_println(__crlf2);
    }
}

static inline bool
match_mac(unsigned char umac[], unsigned char fmac[], unsigned char mask[])
{
    if (is_good_mac(fmac)) {
        if (is_zero_mac(mask)) {
            /*
            * mac NOT zero
            * macmask zero
            *
            * use mac filter
            */
            if (false==os_maceq(umac, fmac)) {
                return false;
            }
        } else {
            /*
            * mac NOT zero
            * macmask NOT zero
            *
            * use mac/macmask filter
            */
            if (false==os_macmaskmach(umac, fmac, mask)) {
                return false;
            }
        }
    }

    return true;
}


static inline bool
match_ip(unsigned int uip, unsigned int fip, unsigned int mask)
{
    if (fip) {
        if (0==mask) {
            /*
            * ip NOT zero
            * ipmask zero
            *
            * use ip filter
            */
            if (uip != fip) {
                return false;
            }
        } else {
            /*
            * ip NOT zero
            * ipmask NOT zero
            *
            * use ip/ipmask filter
            */
            if (false==os_ipmatch(uip, fip, mask)) {
                return false;
            }
        }
    }

    return true;
}

static bool
match_user(struct um_user *user, struct um_user_filter *filter)
{
    if (have_bind(filter->state) && filter->state != user->state) {
        return false;
    }
    
    if (false==match_mac(user->mac, filter->mac, filter->macmask)) {
        return false;
    }
    
    if (false==match_ip(user->ip, filter->ip, filter->ipmask)) {
        return false;
    }
    
    /* all matched */
    return true;
}

int
um_user_delby(struct um_user_filter *filter)
{
    multi_value_t cb(struct um_user *user)
    {
        if (match_user(user, filter)) {
            um_user_del(user);
        }

        return mv2_OK;
    }
    
    return um_user_foreach(cb);
}

int
um_user_getby(struct um_user_filter *filter, um_get_f *get)
{
    multi_value_t cb(struct um_user *user)
    {
        if (match_user(user, filter)) {
            return (*get)(user);
        } else {
            return mv2_OK;
        }
    }
    
    return um_user_foreach(cb);
}

multi_value_t 
um_user_aging(struct um_user *user)
{
#if UM_USE_AGING
    if (is_auth(user->state)) {
        user->aging--;
        if (user->aging <= 0) {
            user_deauth(user, UM_DEAUTH_AGING);
        }
    }
#endif
    
    return mv2_OK;
}

multi_value_t 
um_user_timeout(struct um_user *user, time_t now)
{
#if UM_USE_ONLINE
    unsigned int max, uptime;

    if (is_auth(user->state)) {
        max     = __online_max(user, um_flow_type_wan);
        uptime  = __online_uptime(user, um_flow_type_wan);
        if (max && (uptime < now) && (now - uptime > max)) {
            user_deauth(user, UM_DEAUTH_ONLINETIME);
        }
    }

    if (is_bind(user->state)) {
        max     = __online_max(user, um_flow_type_lan);
        uptime  = __online_uptime(user, um_flow_type_lan);
        if (max && (uptime < now) && (now - uptime > max)) {
            user_unbind(user);
        }
    }
#endif

    return mv2_OK;
}

/******************************************************************************/
