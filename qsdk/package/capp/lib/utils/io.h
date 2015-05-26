#ifndef __IO_H_D4A9C58D7819A78E004AD0C1E795940A__
#define __IO_H_D4A9C58D7819A78E004AD0C1E795940A__
/******************************************************************************/
#include "utils/time.h"
/******************************************************************************/
#ifdef __APP__
static inline int
io_read(int fd, char *buf, int size, int timeout /* ms */)
{
    fd_set rset;
    struct timeval tv = {
        .tv_sec     = time_sec(timeout),
        .tv_usec    = time_usec(timeout),
    };
    int err, count;
    
    FD_ZERO(&rset);
    FD_SET(fd, &rset);

    while(1) {
        err = select(fd + 1, &rset, NULL, NULL, &tv);
        switch(err) {
            case -1:/* error */
                if (EINTR==errno) {
                    // is breaked
                    debug_event("read breaked");
                    
                    continue;
                } else {
                    debug_trace("read error:%d", -errno);
                    
                    return -errno;
                }
            case 0: /* timeout, retry */
                debug_timeout("read timeout");
                
                return -ETIMEOUT;
            default: /* to accept */
                count = read(fd, buf, size);
                if (count >=0) {
                    os_dump_buffer(__is_debug_level_packet, buf, count, NULL);
                }
                
                return count;
        }
    }
}

static inline int
io_write(int fd, char *buf, int len)
{
    int count = 0;
    int err;

    while(count < len) {
        err = write(fd, buf + count, len - count);
        if (err<0) { /* yes, <0 */
            debug_error("write error:%d", -errno);
            
            return -errno;
        } else {
            count += err;
        }
    }

    os_dump_buffer(__is_debug_level_packet, buf, len, NULL);
    
    return count;
}

static inline int
__io_recvfrom(int fd, char *buf, int size, struct sockaddr *addr, socklen_t *paddrlen)
{
    int count;

    count = recvfrom(fd, buf, size, 0, addr, paddrlen);
    if (count<0) {
        debug_error("recvfrom error:%d", -errno);
    }
    os_dump_buffer(__is_debug_level_packet, buf, count, NULL);
        
    return count;
}

static inline int
io_recvfrom(int fd, char *buf, int size, int timeout /* ms */, struct sockaddr *addr, socklen_t *paddrlen)
{
    fd_set rset;
    struct timeval tv = {
        .tv_sec     = time_sec(timeout),
        .tv_usec    = time_usec(timeout),
    };
    int err, count;
    
    FD_ZERO(&rset);
    FD_SET(fd, &rset);

    while(1) {
        err = select(fd + 1, &rset, NULL, NULL, &tv);
        switch(err) {
            case -1:/* error */
                if (EINTR==errno) {
                    // is breaked
                    debug_event("read breaked");
                    
                    continue;
                } else {
                    debug_trace("read error:%d", -errno);
                    
                    return -errno;
                }
            case 0: /* timeout, retry */
                debug_timeout("read timeout");
                
                return -ETIMEOUT;
            default: { /* to accept */
                return __io_recvfrom(fd, buf, size, addr, paddrlen);
            }
        }
    }
}

static inline int
io_sendto(int fd, char *buf, int len, struct sockaddr *addr, int addrlen)
{
    int count = 0;
    int err;

    err = sendto(fd, buf, len, 0, addr, addrlen);
    if (err<0) { /* yes, <0 */
        debug_error("write error:%d", errno);
        
        return -errno;
    }
    else if (err != len) {
        debug_error("write count(%d) < length(%d)", err, len);
        
        return -EIO;
    }
    
    os_dump_buffer(__is_debug_level_packet, buf, len, NULL);
    
    return count;
}

static inline int
__io_recv(int fd, char *buf, int size)
{
    int count;
    
    count = recv(fd, buf, size, 0);
    if (count<0) {
        debug_error("recv error:%d", -errno);
    }
    os_dump_buffer(__is_debug_level_packet, buf, count, NULL);
    
    return count;
}

static inline int
io_recv(int fd, char *buf, int size, int timeout /* ms */)
{
    fd_set rset;
    struct timeval tv = {
        .tv_sec     = time_sec(timeout),
        .tv_usec    = time_usec(timeout),
    };
    int err, count;
    
    FD_ZERO(&rset);
    FD_SET(fd, &rset);

    while(1) {
        err = select(fd + 1, &rset, NULL, NULL, &tv);
        switch(err) {
            case -1:/* error */
                if (EINTR==errno) {
                    // is breaked
                    debug_event("read breaked");
                    
                    continue;
                } else {
                    debug_trace("read error:%d", -errno);
                    
                    return -errno;
                }
            case 0: /* timeout, retry */
                debug_timeout("read timeout");
                
                return -ETIMEOUT;
            default: { /* to accept */
                return __io_recv(fd, buf, size);
            }
        }
    }
}

static inline int
io_send(int fd, char *buf, int len)
{
    int count = 0;
    int err;

    err = send(fd, buf, len, 0);
    if (err<0) { /* yes, <0 */
        debug_error("write error:%d", errno);
        
        return -errno;
    }
    else if (err != len) {
        debug_error("write count(%d) < length(%d)", err, len);
        
        return -EIO;
    }
    
    os_dump_buffer(__is_debug_level_packet, buf, len, NULL);
    
    return count;
}

#endif
/******************************************************************************/
#endif /* __IO_H_D4A9C58D7819A78E004AD0C1E795940A__ */
