#ifndef __IF_H_D5E04D58395B0E0CB6897433A698AF5E__
#define __IF_H_D5E04D58395B0E0CB6897433A698AF5E__
#ifdef __APP__
/******************************************************************************/
#define OS_IFREQ_ZERO { .ifr_ifrn = { .ifrn_name = __empty }}

static inline int
__os_intf_ioctl(int cmd, char *action, struct ifreq *ifr)
{
    int fd = INVALID_FD, err = 0;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (false==is_good_fd(fd)) {
        err = -errno;
        
        debug_error("intf %s socket error:%d", ifr->ifr_name, err);
        
        goto error;
    }

    err = ioctl(fd, cmd, ifr);
    if (err<0) {
        err = -errno;
        
        debug_error("intf %s ioctl %d:%s error:%d", ifr->ifr_name, cmd, action, err);
        
        goto error;
    }
    
error:
    os_close(fd);

    return err;
}

static inline int
os_intf_get_flag(char *ifname, int *flag)
{
    struct ifreq ifr = OS_IFREQ_ZERO;
    int err;

    os_strcpy(ifr.ifr_name, ifname);

    err = __os_intf_ioctl(SIOCGIFFLAGS, "get-flag", &ifr);
    if (err<0) {
        return err;
    }
    
    *flag = ifr.ifr_flags;
    
    return 0;
}

static inline int
os_intf_set_flag(char *ifname, int flag)
{
    struct ifreq ifr = OS_IFREQ_ZERO;
    int err;

    os_strcpy(ifr.ifr_name, ifname);
    ifr.ifr_flags = flag;

    return __os_intf_ioctl(SIOCSIFFLAGS, "set-flag", &ifr);
}

/*
* <0: error
*  0: link down
* >0: link up
*/
static inline int
os_intf_get_link_updown(char *ifname)
{
    int flag = 0;
    int err = os_intf_get_flag(ifname, &flag);
    if (err<0) {
        return err;
    }

    return (IFF_UP & flag)?1:0;
}

/*
* <0: error
*  0: link down
* >0: link up
*/
static inline int
os_intf_get_admin_updown(char *ifname)
{
    int flag = 0, err;

    err = os_intf_get_flag(ifname, &flag);
    if (err<0) {
        return err;
    }

    return (IFF_RUNNING & flag)?1:0;
}

static inline int
os_intf_set_admin_updown(char *ifname, bool up)
{
    int flag = 0, err;

    err = os_intf_get_flag(ifname, &flag);
    if (err<0) {
        return err;
    }

    if (up) {
        flag |= IFF_UP;
    } else {
        flag &= ~IFF_UP;
    }
    
    return os_intf_set_flag(ifname, flag);
}

/*
* <0: error
*  0: not promisc
* >0: is  promisc
*/
static int
os_intf_get_promisc(char *ifname)
{
    int flag = 0, err;

    err = os_intf_get_flag(ifname, &flag);
    if (err) {
        return err;
    }
    
    return (IFF_PROMISC & flag)?1:0;
}

static int
os_intf_set_promisc(char *ifname)
{
    int flag = 0, err;

    err = os_intf_get_flag(ifname, &flag);
    if (err) {
        return err;
    }
    
    return os_intf_set_flag(ifname, flag | IFF_PROMISC);
}

static inline int
os_intf_get_mtu(char *ifname, int *mtu)
{
    struct ifreq ifr = OS_IFREQ_ZERO;
    int err;

    os_strcpy(ifr.ifr_name, ifname);

    err = __os_intf_ioctl(SIOCGIFMTU, "get-mtu", &ifr);
    if (err<0) {
        return err;
    }
    
    *mtu = ifr.ifr_mtu;

    return 0;
}

static inline int
os_intf_set_mtu(char *ifname, int mtu)
{
    struct ifreq ifr = OS_IFREQ_ZERO;
    int err;

    os_strcpy(ifr.ifr_name, ifname);
    ifr.ifr_mtu = mtu;
    
    return __os_intf_ioctl(SIOCSIFMTU, "set-mtu", &ifr);
}

static inline int
os_intf_get_mac(char *ifname, unsigned char mac[])
{
    struct ifreq ifr = OS_IFREQ_ZERO;
    int err;

    os_strcpy(ifr.ifr_name, ifname);

    err = __os_intf_ioctl(SIOCGIFHWADDR, "get-mac", &ifr);
    if (err) {
        return err;
    }

    os_maccpy(mac, (unsigned char *)ifr.ifr_hwaddr.sa_data);

    return 0;
}

static inline int
os_intf_set_mac(char *ifname, unsigned char mac[])
{
    struct ifreq ifr = OS_IFREQ_ZERO;
    int err;

    os_strcpy(ifr.ifr_name, ifname);
    os_maccpy((unsigned char *)ifr.ifr_hwaddr.sa_data, mac);

    return __os_intf_ioctl(SIOCSIFHWADDR, "set-mac", &ifr);
}

static inline int
os_intf_get_ip(char *ifname, unsigned int *ip)
{
    struct ifreq ifr = OS_IFREQ_ZERO;
    int err;

    os_strcpy(ifr.ifr_name, ifname);

    err = __os_intf_ioctl(SIOCGIFADDR, "get-ip", &ifr);
    if (err) {
        return err;
    }

    *ip = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr;
    
    return 0;
}

static inline int
os_intf_get_netmask(char *ifname, unsigned int *mask)
{
    struct ifreq ifr = OS_IFREQ_ZERO;
    int err;

    os_strcpy(ifr.ifr_name, ifname);

    err = __os_intf_ioctl(SIOCGIFNETMASK, "get-netmask", &ifr);
    if (err) {
        return err;
    }

    *mask = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr;
    
    return 0;
}
/******************************************************************************/
#endif
#endif /* __IF_H_D5E04D58395B0E0CB6897433A698AF5E__ */
