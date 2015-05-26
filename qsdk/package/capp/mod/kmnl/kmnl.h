#ifndef __KMNL_H_50B50258D31867D43008409C0F99A5B2__
#define __KMNL_H_50B50258D31867D43008409C0F99A5B2__
/******************************************************************************/
#include "utils.h"
#include "mnl/mnl.h"

#define MNL_NAMSIZ      15

struct mnl_family {
    char    name[1+MNL_NAMSIZ];
    int     (*op)(struct sk_buff *skb, struct nlmsghdr *nlh);
};

extern int  mnl_family_register(int domain, struct mnl_family *family);
extern void mnl_family_unregister(int domain);

static inline struct sk_buff *mnl_new(int size)
{
    return alloc_skb(size, GFP_KERNEL);
}

extern int 
mnl_broadcast(struct sk_buff *skb, unsigned int group);

extern int
mnl_unicast(struct sk_buff *skb, unsigned int pid);

/******************************************************************************/
#endif /* __KMNL_H_50B50258D31867D43008409C0F99A5B2__ */
