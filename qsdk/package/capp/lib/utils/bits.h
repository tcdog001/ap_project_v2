#ifndef __BITS_H_92033DEDA810AF54224FF623B3115513__
#define __BITS_H_92033DEDA810AF54224FF623B3115513__
/******************************************************************************/
#ifndef __os_align
#define __os_align(_x, _align)      ((((_x)+(_align)-1)/(_align))*(_align))
#endif

#ifndef __os_align_down
#define __os_align_down(_x, _align) ((((_x)+(_align)-1)/(_align) - 1)*(_align))
#endif

#ifndef os_align
#define os_align(_x, _align)        (((_x)+(_align)-1) & ~((_align)-1))
#endif

#ifndef os_align_down
#define os_align_down(_x, _align)   ((_x) & ~((_align)-1))
#endif

#ifndef os_setflag
#define os_setflag(_value, _flag)   do{(_value) |= (_flag);}while(0)
#endif

#ifndef os_clrflag
#define os_clrflag(_value, _flag)   do{(_value) &= ~(_flag);}while(0)
#endif

#ifndef os_hasflag
#define os_hasflag(_value, _flag)   ((_flag) == ((_value) & (_flag)))
#endif

#ifndef os_bit
#define os_bit(_bit)                (1<<(_bit))
#endif

#ifndef os_mask
#define os_mask(_bit)               (os_bit(_bit) - 1)
#endif

#ifndef os_maskmatch
#define os_maskmatch(_a, _b, _mask) (((_a) & (_mask))!=((_b) & (_mask)))
#endif

#ifndef os_setbit
#define os_setbit(_value, _bit)     os_setflag(_value, os_bit(_bit))
#endif

#ifndef os_clrbit
#define os_clrbit(_value, _bit)     os_clrflag(_value, os_bit(_bit))
#endif

#ifndef os_hasbit
#define os_hasbit(_value, _bit)     os_hasflag(_value, os_bit(_bit))
#endif

static inline bool
os_bufmaskmatch(unsigned char *a, unsigned char *b, unsigned char *mask, int len)
{
    int i;

    for (i=0; i<len; i++) {
        if (false==os_maskmatch(a[i], b[i], mask[i])) {
            return false;
        }
    }

    return true;
}

#ifndef BITMAPBITS
#define BITMAPBITS  1024
#endif

enum {
    BITMAPSLOT  = (sizeof(uint32_t)*8),
    BITMAPSIZE  = (BITMAPBITS/BITMAPSLOT),
};

typedef struct {
    uint32_t map[BITMAPSIZE];
} os_bitmap_t;

static inline void
os_bitmap_set(os_bitmap_t *bmp, int bit)
{
    if (is_good_enum(bit, BITMAPBITS)) {
        int __bit = bit%BITMAPSLOT;
        
        os_setbit(bmp->map[bit/BITMAPSLOT], __bit);
    }
}

static inline void
os_bitmap_clr(os_bitmap_t *bmp, int bit)
{
    if (is_good_enum(bit, BITMAPBITS)) {
        int __bit = bit%BITMAPSLOT;
        
        os_clrbit(bmp->map[bit/BITMAPSLOT], __bit);
    }
}

static inline bool
os_bitmap_isset(os_bitmap_t *bmp, int bit)
{
    if (is_good_enum(bit, BITMAPBITS)) {
        int __bit = bit%BITMAPSLOT;
        
        return os_hasbit(bmp->map[bit/BITMAPSLOT], __bit);
    } else {
        return false;
    }
}
/******************************************************************************/
#endif /* __BITS_H_92033DEDA810AF54224FF623B3115513__ */
