#ifndef __UTILS_H_F5F47009AF9E5B811C77BFEA13C326CB__
#define __UTILS_H_F5F47009AF9E5B811C77BFEA13C326CB__
/******************************************************************************/
#ifdef __FASTBOOT__
#define __BOOT__
#endif

#ifdef __BOOT__
#   include <malloc.h>
#   include <command.h>
#   include <common.h>
#   include <linux/ctype.h>
#   include <linux/string.h>
#   ifndef __AKID_DEBUG
#       define __AKID_DEBUG    __boot_debuger
#   endif
#elif defined(__KERNEL__)
#   include <linux/moduleparam.h>
#   include <linux/module.h>
#   include <linux/skbuff.h>
#   include <linux/gfp.h>
#   include <linux/init.h>
#   include <linux/in.h>
#   include <linux/inet.h>
#   include <linux/netlink.h>
#   include <linux/rtc.h>
#   include <linux/ctype.h>
#   include <linux/string.h>
#   include <linux/time.h>
#   include <linux/errno.h>
#   include <linux/list.h>
#   include <net/sock.h>
#else /* __APP__ */
#   define __APP__
#   ifdef __BUSYBOX__
#       include "libbb.h"
#   endif
#   include <stdint.h>
#   include <stdarg.h>
#   include <stdlib.h>
#   include <stdio.h>
#   include <signal.h>
#   include <string.h>
#   include <malloc.h>
#   include <assert.h>
#   include <unistd.h>
#   include <fcntl.h>
#   include <ctype.h>
#   include <errno.h>
#   include <time.h>
#   include <dirent.h>
#   include <syslog.h>
#   include <ucontext.h>
#   include <byteswap.h>
#   include <libgen.h>
#   include <sys/types.h>
#   include <sys/stat.h>
#   include <sys/file.h>
#   include <sys/epoll.h>
#   include <sys/wait.h>
#   include <sys/ioctl.h>
#   include <sys/ipc.h>
#   include <sys/msg.h>
#   include <sys/sem.h>
#   include <sys/shm.h>
#   include <sys/socket.h>
#   include <sys/un.h>
#   include <sys/time.h>
#   include <sys/timerfd.h>
#   include <netpacket/packet.h>
#   include <netinet/ether.h>
#   include <netinet/in.h>
#   include <netinet/ip.h>
#   include <netinet/tcp.h>
#   include <netinet/udp.h>
#   include <arpa/inet.h>
#   include <net/if.h>
#endif /* !__APP__ */

#ifndef __THIS_APP
#error "must defined __THIS_APP"
#endif

#ifndef __THIS_NAME
#error "must defined __THIS_NAME"
#endif

#ifndef __THIS_FILE
#define __THIS_FILE     __THIS_NAME
#endif

#ifndef __AKID_DEBUG
#define __AKID_DEBUG    __##__THIS_APP##_debugger
#endif

#ifndef USE_USAGE
#define USE_USAGE       1
#endif

#include "utils/type.h"
#include "utils/base.h"
#include "utils/bits.h"
#include "utils/assert.h"
#include "utils/errno.h"
#include "utils/number.h"
#include "utils/memory.h"
#include "utils/list.h"
#include "utils/string.h"
#include "utils/printf.h"
#include "utils/env.h"
#include "utils/appkey.h"
#include "utils/jsonapi.h"
#include "utils/socket.h"
#include "utils/jlog.h"
#include "utils/debug.h"
#include "utils/dump.h"
#include "utils/time.h"
#include "utils/sem.h"
#include "utils/shm.h"
#include "utils/sfile.h"
#include "utils/crc32.h"
#include "utils/mac.h"
#include "utils/ip.h"
#include "utils/if.h"
#include "utils/io.h"
#include "utils/kv.h"
#include "utils/cmd.h"
#include "utils/slice.h"
#include "utils/blob.h"
#include "utils/timer.h"
#include "utils/autoarray.h"
#include "utils/channel.h"
#include "utils/coroutine.h"
#include "utils/fd.h"
/******************************************************************************/
#define OS_INITER       \
    DECLARE_DEBUGGER;   \
    DECLARE_JLOG        \
    /* end */

static inline int
os_init(void)
{
    int err;
    
    err = appkey_init();
    if (err) {
        return err;
    }
    
    err = jlog_init();
    if (err) {
        return err;
    }
    
    return 0;
}

static inline int
os_fini(void)
{
    jlog_fini();
    appkey_fini();

    return 0;
}

static inline int
os_main(int (*func)(int, char **), int argc, char **argv)
{
    int err = os_call(os_init, os_fini, func, argc, argv);

    return shell_error(err);
}
/******************************************************************************/
#endif /* __UTILS_H_F5F47009AF9E5B811C77BFEA13C326CB__ */
