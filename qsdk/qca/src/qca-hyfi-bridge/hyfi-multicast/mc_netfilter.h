/*
 * Copyright (c) 2012 The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2, as published by the Free Software Foundation.
*/

#ifndef _MC_NETFILTER_H_
#define _MC_NETFILTER_H_

int __init mc_netfilter_init(void);
void mc_netfilter_exit(void);

#ifndef HYFI_MC_STANDALONE_NF

#include <linux/netfilter.h>
#include <linux/netdevice.h>

unsigned int mc_pre_routing_hook(unsigned int hooknum, struct sk_buff *skb,
        const struct net_device *in, const struct net_device *out,
        int(*okfn)(struct sk_buff *));

unsigned int mc_forward_hook(unsigned int hooknum, struct sk_buff *skb,
        const struct net_device *in, const struct net_device *out,
        int(*okfn)(struct sk_buff *));
#endif

#endif
