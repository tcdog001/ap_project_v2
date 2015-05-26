extern unsigned int __AKID_DEBUG;
static unsigned int *__debug_level_pointer = &__AKID_DEBUG;

#include "kmnl.h"
/******************************************************************************/
#ifndef NETLINK_MNL
#define NETLINK_MNL    28
#endif

#ifndef MNL_GROUPS
#define MNL_GROUPS     128
#endif

#ifndef MNL_DOMAINS
#define MNL_DOMAINS     32
#endif

#if 1

#ifndef KERNEL_VERSION
#define KERNEL_VERSION(_a, _b, _c)  ((_a)*256*256 + (_b)*256 + (_c))
#endif

#ifndef LINUX_VERSION_CODE
#define LINUX_VERSION_CODE          KERNEL_VERSION(3, 8, 0)
#endif

#endif

/*
struct sock *
netlink_kernel_create(int unit, unsigned int groups,
		      void (*input)(struct sock *sk, int len),
		      struct mutex *cb_mutex, struct module *module)
*/
#define NETLINK_API_SOCK    1

/*
struct sock *
netlink_kernel_create(struct net *net, int unit, unsigned int groups,
		      void (*input)(struct sk_buff *skb),
		      struct mutex *cb_mutex, struct module *module)
*/
#define NETLINK_API_SKB     2

/*
struct netlink_kernel_cfg {
	unsigned int	groups;
	unsigned int	flags;
	void		(*input)(struct sk_buff *skb);
	struct mutex	*cb_mutex;
	void		(*bind)(int group);
	bool		(*compare)(struct net *net, struct sock *sk);
};

static inline struct sock *
netlink_kernel_create(struct net *net, int unit, struct netlink_kernel_cfg *cfg)
{
	return __netlink_kernel_create(net, unit, THIS_MODULE, cfg);
}
*/
#define NETLINK_API_CFG     3


/*
* 暂时没时间调查 netlink_kernel_create 接口变化, 先不判断内核版本区间
*/
#if     (LINUX_VERSION_CODE == KERNEL_VERSION(2,6,15)) || \
        (LINUX_VERSION_CODE == KERNEL_VERSION(2,6,22))
        /* LINUX_VERSION_CODE >= KERNEL_VERSION (2,6,22) */
#define NETLINK_API     NETLINK_API_SOCK
        /* LINUX_VERSION_CODE >= KERNEL_VERSION (2,6,24) */
#elif   (LINUX_VERSION_CODE == KERNEL_VERSION(2,6,31)) || \
        (LINUX_VERSION_CODE == KERNEL_VERSION(2,6,35))
#define NETLINK_API     NETLINK_API_SKB
#elif   (LINUX_VERSION_CODE == KERNEL_VERSION(3,8,0))
#define NETLINK_API     NETLINK_API_CFG
#else
#error "autelan netlink not support linux kernel!!!"
#endif

static int mnl_family = NETLINK_MNL;
static int mnl_groups = MNL_GROUPS;
static int mnl_domains= MNL_DOMAINS;
unsigned int __AKID_DEBUG = __debug_level_default;

module_param(mnl_family, int, 0);
module_param(mnl_groups, int, 0);
module_param(mnl_domains, int, 0);
module_param(__AKID_DEBUG, unsigned int, 0);

/*
* mnl G
*/
static struct {
    struct mutex mutex;
    struct sock *sk;

    struct mnl_family **familys;
} G = {
    .mutex  = __MUTEX_INITIALIZER(G.mutex),
};

#define __MNL_OP(_domain)           G.familys[_domain]->op
#define MNL_OP(_domain, _skb, _nlh) __MNL_OP(_domain)(_skb, _nlh)


#define mnl_lock()       mutex_lock(&G.mutex)
#define mnl_unlock()     mutex_unlock(&G.mutex)
#define mnl_trylock()    mutex_trylock(&G.mutex)

static int mc_recv(struct sk_buff *skb, struct nlmsghdr *nlh)
{
    struct mnlmsghdr *mnl = (struct mnlmsghdr *)(nlh+1);
    int dst_group = mnl->u.m.group;
    
    if (MNL_P_MULTICAST!=mnl->protocol) {
        return -1010;
    }
    
    if (MNL_VERSION!=mnl->u.m.version) {
        return -1011;
    }
    
    if (mnl->u.m.group >= mnl_groups) {
        return -1012;
    }
    
    nlh->nlmsg_pid = 0;
    NETLINK_CB(skb).portid	= 0;
	NETLINK_CB(skb).dst_group = dst_group;
    atomic_inc(&skb->users); // see netlink_sendmsg(before netlink_broadcast)
    
    return mnl_broadcast(skb, dst_group);
}

static int mnl_rcv_msg(struct sk_buff *skb, struct nlmsghdr *nlh)
{
    struct mnlmsghdr *mnl = (struct mnlmsghdr *)(nlh+1);
    
    if (false==os_memeq(mnl->magic, MNL_MAGIC, 3)) {
        return -1000;
    }

    if (false==is_good_enum(mnl->protocol, MNL_P_END)) {
        return -1001;
    }
    
    return MNL_OP(mnl->protocol, skb, nlh);
}

#if (NETLINK_API == NETLINK_API_SOCK)

static void mnl_rcv(struct sock *sk, int len)
{
	unsigned int qlen = 0;

	do {
		if (mnl_trylock()) {
			return;
		}
		
		netlink_run_queue(sk, &qlen, mnl_rcv_msg);
		mnl_unlock();
	} while (qlen && G.sk && G.sk->sk_receive_queue.qlen);
}

#elif (NETLINK_API == NETLINK_API_SKB) || \
      (NETLINK_API == NETLINK_API_CFG)

static void mnl_rcv(struct sk_buff *skb)
{
	mnl_lock();
	netlink_rcv_skb(skb, &mnl_rcv_msg);
	mnl_unlock();
}

#endif

int
mnl_family_register(int domain, struct mnl_family *family)
{
    if (false==is_good_enum(domain, mnl_domains)) {
        return -EINVAL1;
    }
    else if (NULL==family) {
        return -EINVAL2;
    }
    
    mnl_lock();
    G.familys[domain] = family;
    mnl_unlock();
    
    debug_ok("mnl(domain:%d, name=%s) register OK.", domain, family->name);
    
    return 0;
}
EXPORT_SYMBOL(mnl_family_register);

void
mnl_family_unregister(int domain)
{
    if (is_good_enum(domain, mnl_domains) && G.familys[domain]) {
        mnl_lock();
        {
            debug_ok("mnl(domain:%d, name=%s) unregister OK.", domain, G.familys[domain]->name);

            G.familys[domain] = NULL;
        }
        mnl_unlock();
    }
}
EXPORT_SYMBOL(mnl_family_unregister);


int
mnl_broadcast(struct sk_buff *skb, unsigned int group)
{
    if (G.sk) {
	    return netlink_broadcast(G.sk, skb, 0, group, GFP_KERNEL);
    } else {
        return -EINVAL1;
    }
}
EXPORT_SYMBOL(mnl_broadcast);

int
mnl_unicast(struct sk_buff *skb, unsigned int pid)
{
    if (G.sk) {
        return netlink_unicast(G.sk, skb, pid, MSG_DONTWAIT);
    } else {
        return -EINVAL1;
    }
}
EXPORT_SYMBOL(mnl_unicast);


static struct mnl_family MC = {
    .name = "mc-netlink",
    .op = mc_recv,
};

static int mnl_init(void) 
{
#if NETLINK_API==NETLINK_API_CFG
    struct netlink_kernel_cfg mnl_cfg = {
        .groups = mnl_groups,
        .input  = mnl_rcv,
    };
#endif
    
#if NETLINK_API==NETLINK_API_SOCK
#define netlink_kernel_create_params    mnl_family, mnl_groups, mnl_rcv, NULL, THIS_MODULE
#elif NETLINK_API==NETLINK_API_SKB
#define netlink_kernel_create_params    &init_net, mnl_family, mnl_groups, mnl_rcv, NULL, THIS_MODULE
#elif NETLINK_API==NETLINK_API_CFG
#define netlink_kernel_create_params    &init_net, mnl_family, &mnl_cfg
#endif

    G.sk = netlink_kernel_create(netlink_kernel_create_params);
    if (NULL == G.sk) {
		panic("MNL: Cannot initialize mnl netlink" __crlf);
	}

    G.familys = (struct mnl_family **)os_calloc(mnl_domains, sizeof(struct mnl_family *));
    if (NULL==G.familys) {
        panic("MNL: Cannot initialize mnl netlink" __crlf);
    }
    
    mnl_family_register(MNL_P_MULTICAST, &MC);
    
 	debug_ok("init OK.");
    
    return 0;
}

static void mnl_fini(void)
{
    sock_release(G.sk->sk_socket);
    os_free(G.familys);
    
    //debug_ok("clean OK.");
}

module_init(mnl_init);
module_exit(mnl_fini);

MODULE_DESCRIPTION("multicast netlink");
MODULE_AUTHOR("l_j_f@163.com");
MODULE_LICENSE("GPL");

