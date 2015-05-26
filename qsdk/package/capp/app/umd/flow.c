#ifndef __THIS_APP
#define __THIS_APP      umd
#endif

#ifndef __THIS_NAME
#define __THIS_NAME     "umd"
#endif

#ifndef __THIS_FILE
#define __THIS_FILE     "flow"
#endif

#include "umd.h"

static struct um_flow flow;

static inline bool
is_local_mac(unsigned char mac[])
{
    return os_maceq(mac, umd.intf[UM_INTF_TC].mac);
}

static inline bool
is_local_ip(unsigned int ip)
{
    return ip==umd.intf[UM_INTF_TC].ip;
}

static inline bool
is_user_ip(unsigned int ip)
{
    struct um_intf *intf = &umd.intf[UM_INTF_TC];
    
    return (ip & intf->mask)==(intf->ip & intf->mask);
}

static int
pkt_recv(simpile_server_t *server)
{
    int err, len;
    socklen_t addrlen = sizeof(server->addr.ll);

    err = recvfrom(server->fd, flow.packet, sizeof(flow.packet), 0, &server->addr.c, &addrlen);
    if (err<0) { /* yes, <0 */
        return err;
    }
    flow.len = err;

    return 0;
}

static int
intf_check(simpile_server_t *server)
{
    if (server->addr.ll.sll_ifindex == umd.intf[UM_INTF_TC].index) {
        return 0;
    } else {
        return -EBADIDX;
    }
}

static int
eth_check(simpile_server_t *server)
{
    struct ether_header *eth = flow.eth = (struct ether_header *)flow.packet;
    unsigned char *smac = eth->ether_shost;
    unsigned char *dmac = eth->ether_dhost;
    
    if (eth->ether_type != flow.ether_type) {
        /*
        * bad ether type
        */
        return -EFORMAT;
    }
#if 0
    else if (false==is_good_mac(smac)) {
        /*
        * bad src mac
        */
        return -EFORMAT;
    }
    else if (false==is_good_mac(dmac)) {
        /*
        * bad dst mac
        */
        return -EFORMAT;
    }
#endif
    else if (is_local_mac(smac)) {
        /*
        * src mac is self
        *
        * local==>user
        */
        flow.dir = um_flow_dir_down;
        flow.usermac = dmac;
    }
    else if (is_local_mac(dmac)) {
        /*
        * dst mac is self
        *
        * user==>local
        */
        flow.dir = um_flow_dir_up;
        flow.usermac = smac;
    }
    else {
        return -EFORMAT;
    }

    flow.eth_packets++;
    flow.eth_bytes += flow.len;
    
    return 0;
}

static int
ip_check(simpile_server_t *server)
{
    struct ip *iph = flow.iph = (struct ip *)(flow.eth+1);
    unsigned int sip = iph->ip_src.s_addr;
    unsigned int dip = iph->ip_dst.s_addr;
    
    if (4!=iph->ip_v) {
        return -EFORMAT;
    }
    else if (5!=iph->ip_hl) {
        return -EFORMAT;
    }
    else if (is_local_ip(sip)) {
        if (is_local_ip(dip)) {
            /*
            * local==>local
            */
            return -EFORMAT;
        }
        else if (is_user_ip(dip)) {
            /*
            * local==>user
            */
            flow.type = um_flow_type_lan;
            flow.userip = dip;
        }
        else { /* dip is cloud */
            /*
            * local==>cloud
            */
            flow.type = um_flow_type_local;
            
            return -EFORMAT;
        }
    }
    else if (is_user_ip(sip)) {
        if (is_local_ip(dip)) {
            /*
            * user==>local
            */
            flow.type = um_flow_type_lan;
            flow.userip = sip;
        }
        else if (is_user_ip(dip)) {
            /*
            * user==>user
            */
            return -EFORMAT;
        }
        else { /* dip is cloud */
            /*
            * user==>cloud
            */
            flow.type = um_flow_type_wan;
            flow.userip = sip;
        }
    }
    else { /* sip is cloud */
        if (is_local_ip(dip)) {
            /*
            * cloud==>local
            */
            flow.type = um_flow_type_local;
            
            return -EFORMAT;
        }
        else if (is_user_ip(dip)) {
            /*
            * cloud==>user
            */
            flow.type = um_flow_type_wan;
            flow.userip = dip;
        }
        else { /* dip is cloud */
            /*
            * cloud==>cloud
            */
            return -EFORMAT;
        }
    }

    flow.ip_packets++;
    flow.ip_bytes += flow.len;

    return 0;
}

static int
pkt_debug(simpile_server_t *server)
{
    if (__is_debug_level_packet) {
        char smacstring[1+MACSTRINGLEN_L];
        char dmacstring[1+MACSTRINGLEN_L];
        
        char sipstring[1+OS_IPSTRINGLEN];
        char dipstring[1+OS_IPSTRINGLEN];

        os_strcpy(smacstring, os_macstring(flow.eth->ether_shost));
        os_strcpy(dmacstring, os_macstring(flow.eth->ether_dhost));
        
        os_strcpy(sipstring, os_ipstring(flow.iph->ip_src.s_addr));
        os_strcpy(dipstring, os_ipstring(flow.iph->ip_dst.s_addr));
        
        debug_packet("recv packet length:%d smac=%s dmac=%s sip=%s, dip=%s protocol:%d", 
            flow.len,
            smacstring,
            dmacstring,
            sipstring,
            dipstring,
            flow.iph->ip_p);
        __os_dump_buffer(flow.packet, flow.len, NULL);
    }

    return 0;
}

static void
__flow_update(struct um_user *user, int type, int dir)
{
    uint64_t max = __flow_max(user, type, dir);
    uint64_t now = __flow_now(user, type, dir) + flow.len;
    
    __flow_now(user, type, dir) = now;
    
    if (max && max < now) {
        user_deauth(user, UM_DEAUTH_FLOWLIMIT);
    }
}

static int
flow_update(simpile_server_t *server)
{
    struct um_user *user;
    
#if UM_USE_AUTO
    user = um_user_create(flow.usermac);
#else
    user = um_user_get(flow.usermac);
#endif
    if (NULL==user) {
        return -ENOEXIST;
    }

    if (um_flow_type_wan==flow.type && is_auth(user->state)) {
        user->aging = UM_AGING;
    }

    if (have_bind(user->state)) {
        __flow_update(user, flow.type, flow.dir);
        __flow_update(user, um_flow_dir_all, flow.dir);
    }
    
    return 0;
}

jobj_t
um_jflow(void)
{
    jobj_t obj = jobj_new_object();
    jobj_t eth = jobj_new_object();
    jobj_t ip  = jobj_new_object();

    jobj_add_int(eth, "packets", flow.eth_packets);
    jobj_add_int64(eth, "bytes", flow.eth_bytes);
    jobj_add(obj, "eth", eth);

    jobj_add_int(ip, "packets", flow.ip_packets);
    jobj_add_int64(ip, "bytes", flow.ip_bytes);
    jobj_add(obj, "ip", ip);

    return obj;
}

static int
flow_server_init(simpile_server_t *server)
{
    int fd, err;

    flow.ether_type = htons(ETHERTYPE_IP);
    
    fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if (fd<0) {
        debug_error("socket error:%d", errno);

        return -errno;
    }
    
    fcntl(fd, F_SETFL, O_NONBLOCK);

#if UM_USE_BIND
    struct sockaddr_ll addr = {
        .sll_family     = AF_PACKET,
        .sll_protocol   = flow.ether_type,
        .sll_ifindex    = umd.intf[UM_INTF_TC].index,
    };
    err = bind(fd, (struct sockaddr *)&addr, sizeof(addr));
    if (err) {
        return err;
    }
#endif

    server->fd = fd;
    
    debug_ok("init flow server ok");
    
    return 0;
}

static int
__flow_server_handle(simpile_server_t *server)
{
    static int (*handle[])(simpile_server_t *) = {
        pkt_recv,
        intf_check,
        eth_check,
        ip_check,
        pkt_debug,
        flow_update,
    };
    int i, err;

    for (i=0; i<os_count_of(handle); i++) {
        err = (*handle[i])(server);
        if (err) {
            return err;
        }
    }
    
    return 0;
}

static int
flow_server_handle(simpile_server_t *server)
{
    int i, err;

    for (i=0; i<UM_SNIFF_COUNT; i++) {
        err = __flow_server_handle(server);
        if (err) {
            return err;
        }
    }

    return 0;
}

simpile_server_t um_flow_server = {
    .fd     = INVALID_FD,
    .addr   = OS_SOCKADDR_INITER(AF_PACKET),
    
    .init   = flow_server_init,
    .handle = flow_server_handle,
};

/******************************************************************************/
