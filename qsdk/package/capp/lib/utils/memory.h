#ifndef __MEMORY_H_449CCD8BC1CB461480DB11AD8A1CEB17__
#define __MEMORY_H_449CCD8BC1CB461480DB11AD8A1CEB17__
/******************************************************************************/
#ifdef __BOOT__
#define size_t unsigned int
#endif

static inline void *
os_memcpy(void *dst, const void *src, size_t n)
{
    if (dst && src) {
        return memcpy(dst, src, n);
    } else {
        return os_assertV(dst);
    }
}

static inline void *
os_memset(void *s, int ch, size_t n)
{
    if (s) {
        return memset(s, ch, n);
    } else {
        return os_assertV(s);
    }
}

#ifndef os_memzero
#define os_memzero(_ptr, _size)         os_memset(_ptr, 0, _size)
#endif

static inline int
os_memcmp(const void *a, const void *b, size_t n)
{
    if (a) {
        if (b) {
            return n?memcmp(a, b, n):0;
        } else {
            /*
            * a is good
            * b is zero
            * so, a > b
            */
            return os_assertV(1);
        }
    } else {
        if (b) {
            /*
            * a is zero
            * b is good
            * so, a < b
            */
            return os_assertV(-1);
        } else {
            /*
            * a is zero
            * b is zero
            * so, a = b
            */
            return os_assertV(0);
        }
    }
}

#ifndef os_memeq
#define os_memeq(_a, _b, _size)     (0==os_memcmp(_a, _b, _size))
#endif

#ifndef os_memmem
#ifdef __BOOT__
static inline void *
os_memmem(const void *haystack, size_t haystacklen,
		     const void *needle, size_t needlelen)
{
	register const char *ph;
	register const char *pn;
	const char *plast;
	size_t n;

	if (needlelen == 0) {
		return (void *) haystack;
	}

	if (haystacklen >= needlelen) {
		ph = (const char *) haystack;
		pn = (const char *) needle;
		plast = ph + (haystacklen - needlelen);

		do {
			n = 0;
			while (ph[n] == pn[n]) {
				if (++n == needlelen) {
					return (void *) ph;
				}
			}
		} while (++ph <= plast);
	}

	return NULL;
}
#else
#define os_memmem(_mem, _mem_size, _obj, _obj_size)     memmem(_mem, _mem_size, _obj, _obj_size)
#endif
#endif
/*
* use _array's size
*   _array is array name
*/
#ifndef os_arrayzero
#define os_arrayzero(_array)        os_memzero(_array, sizeof(_array))
#endif

/*
* use _dst's size
*   _dst and _src is array name
*/
#ifndef os_arraydcpy
#define os_arraydcpy(_dst, _src)    os_memcpy(_dst, _src, sizeof(_dst))
#endif

/*
* use _src's size
*   _dst and _src is array name
*/
#ifndef os_arrayscpy
#define os_arrayscpy(_dst, _src)    os_memcpy(_dst, _src, sizeof(_src))
#endif

/*
* use _a's size
*   _a and _b is array name
*/
#ifndef os_arraycmp
#define os_arraycmp(_a, _b)         os_memcmp(_a, _b, sizeof(_a))
#endif

/*
* use _a's size
*   _a and _b is array name
*/
#ifndef os_arrayeq
#define os_arrayeq(_a, _b)          (0==os_arraycmp(_a, _b))
#endif

/*
* use (*_pobj)'s size
*
* obj is a object(struct/array/native type)
*   pobj is the obj's address
*
* if obj is array
*   (1)the array MUST be real array(not function param)
*   (2)the _pobj MUST be the address of array(use as &array)
*
* array sizeof test
*   sizeof(array)==sizeof(*&array)
*   sizeof(*array)==sizeof(array[0])
*   sizeof(&array)==sizeof(void *)
*/
#ifndef os_objzero
#define os_objzero(_pobj)           os_memzero(_pobj, sizeof(*(_pobj)))
#endif

/*
* use (*_dst)'s size
*/
#ifndef os_objdcpy
#define os_objdcpy(_dst, _src)      os_memcpy(_dst, _src, sizeof(*(_dst)))
#endif

/*
* use (*_src)'s size
*/
#ifndef os_objscpy
#define os_objscpy(_dst, _src)      os_memcpy(_dst, _src, sizeof(*(_src)))
#endif

/*
* use (*_dst)'s size
*/
#ifndef os_objcpy
#define os_objcpy(_dst, _src)       os_objdcpy(_dst, _src)
#endif

/*
* use (*_a)'s size
*/
#ifndef os_objcmp
#define os_objcmp(_a, _b)           os_memcmp(_a, _b, sizeof(*(_a)))
#endif

/*
* use (*_a)'s size
*/
#ifndef os_objeq
#define os_objeq(_a, _b)            (0==os_objcmp(_a, _b))
#endif

#define os_objdeft(_obj, _deft) do{ \
    typeof(*_obj) new = _deft;      \
    os_objcpy(_obj, &new);          \
}while(0)


#define os_value_cmp(_a, _b) ({ \
    int ret;                    \
    typeof(_a)  a = (_a);       \
    typeof(_b)  b = (_b);       \
                                \
    if (a > b) {                \
        ret = 1;                \
    }                           \
    else if (a==b) {            \
        ret = 0;                \
    }                           \
    else {                      \
        ret = -1;               \
    }                           \
                                \
    ret;                        \
})

#define os_cmp_always_eq(_a, _b)    0
/*
* bad < good
* bad == bad
* all is good, use _objcmp
*/
#define __os_objcmp(_a, _b, _is_good, _objcmp) ({ \
    int ret;                                \
    typeof(_a)  x = (_a);                   \
    typeof(_b)  y = (_b);                   \
                                            \
    if (_is_good(x)) {                      \
        if (_is_good(y)) {                  \
            /*                              \
            * x is good, y is good          \
            */                              \
            ret = _objcmp(x, y);            \
        } else {                            \
            /*                              \
            * x is good, y is bad           \
            */                              \
            ret = 1;                        \
        }                                   \
    } else {                                \
        if (_is_good(y)) {                  \
            /*                              \
            * x is bad, y is good           \
            */                              \
            ret = -1;                       \
        } else {                            \
            /*                              \
            * x is bad, y is bad            \
            */                              \
            ret = 0;                        \
        }                                   \
    }                                       \
                                            \
    ret;                                    \
}) /* end */

#ifndef __os_getobjarrayidx
#define __os_getobjarrayidx(_array, _obj, _cmp, _begin, _end) ({ \
    int i, idx = (_end);                    \
                                            \
    for (i=(_begin); i<(_end); i++) {       \
        if (0==_cmp((_array)[i], _obj)) {   \
            idx = i;                        \
            break;                          \
        }                                   \
    }                                       \
                                            \
    idx;                                    \
})  /* end */
#endif

#ifndef os_getobjarrayidx
#define os_getobjarrayidx(_array, _obj, _begin, _end) \
        __os_getobjarrayidx(_array, _obj, os_objcmp, _begin, _end)
#endif


#if defined(__BOOT__) || defined(__APP__)
#define os_malloc(_size)            malloc(_size)
#define os_calloc(_count, _size)        ({  \
    void *p = os_malloc((_count)*(_size));  \
    if (p) {                                \
        os_memzero(p, (_count)*(_size));    \
    }                                       \
    p;                                      \
})  /* end */

#define os_realloc(_ptr, _size)     realloc(_ptr, _size)
#define os_free(_ptr) \
        do{ if (_ptr) { free(_ptr); (_ptr) = NULL; } }while(0)
#else
#define os_malloc(_size)            kmalloc(_size, GFP_KERNEL)
#define os_calloc(_count, _size)    calloc(_count, _size)
#define os_realloc(_ptr, _size)     krealloc(_ptr, _size, GFP_KERNEL)
#define os_free(_ptr) \
        do{ if (_ptr) { kfree(_ptr); (_ptr) = NULL; } }while(0)
#endif

#ifdef __APP__
#define os_alloca(_size)            alloca(_size)
#endif

#define os_zalloc(_size)            os_calloc(1, _size)

/******************************************************************************/
#endif /* __MEMORY_H_449CCD8BC1CB461480DB11AD8A1CEB17__ */
