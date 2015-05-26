#ifndef __NBUS_PROTOCOL_H_17A3C236FAC0BF153B948CFC2DA95987__
#define __NBUS_PROTOCOL_H_17A3C236FAC0BF153B948CFC2DA95987__
/******************************************************************************/

#if 0
struct nlmsghdr {
	__u32		nlmsg_len;	/* Length of message including header */
	__u16		nlmsg_type;	/* Message content */
	__u16		nlmsg_flags;/* Additional flags */
	__u32		nlmsg_seq;	/* Sequence number */
	__u32		nlmsg_pid;	/* Sending process port ID */
};
#endif

struct nbusmsg {
    int8_t      nlmsg_magic[4]; /* must "nbus" */
    
    uint8_t     nlmsg_subsystem;/* enum ubus_subsystem */
    uint8_t     nlmsg_version;
    uint8_t     nlmsg_reserved[2];
};

enum ubus_subsystem {
    UBUS_SYS_UNKNOW,
    
    UBUS_SYS_OBJ,
    
    UBUS_SYS_END
};

/* UBUS_SYS_OBJ */
/*
     0                   1                   2                   3
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |      class name length      |       class name...
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |      class name length      |       class name...
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |     object name length      |      object name...
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

struct nbusmsg_obj_create {
    uint32_t    class;  
    uint32_t    object;
    
};

/*
    nbus protocol base on netlink

    ---------------------------------------------------------------------------
    |  nlmsghdr  |  atnlhdr  |  netlink attr(as attlv) ... 
    ---------------------------------------------------------------------------
    
    or 
    
    ---------------------------------------------------------------------------
    |  nlmsghdr  |  nlmsgerr  |
    ---------------------------------------------------------------------------
*/


/******************************************************************************/
#endif /* __NBUS_PROTOCOL_H_17A3C236FAC0BF153B948CFC2DA95987__ */
