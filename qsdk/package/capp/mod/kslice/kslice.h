#ifndef __KSLICE_H_B035B6427F63F9E078C635C9FA7672CB__
#define __KSLICE_H_B035B6427F63F9E078C635C9FA7672CB__
/******************************************************************************/
#include "slice/slice.h"

#define SLICE_LOCAL_FOR_RECV_BY_SKB(_skb) {   \
    .len    = (_skb)->len,          \
    .size   = (_skb)->len + skb_tailroom(_skb), \
    .resv   = skb_headroom(_skb),   \
    .head   = (_skb)->data,         \
}

#define SLICE_LOCAL_FOR_SEND_BY_SKB(_skb) \
        __SLICE_INITER((_skb)->data, (_skb)->len + skb_tailroom(_skb), ATNL_HSIZE, true)
/******************************************************************************/
#endif /* __KSLICE_H_B035B6427F63F9E078C635C9FA7672CB__ */
