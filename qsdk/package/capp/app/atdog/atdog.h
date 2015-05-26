#ifndef __AT_DOG_H_DE28C2611313B5650AF7DB2BFB5828D4__
#define __AT_DOG_H_DE28C2611313B5650AF7DB2BFB5828D4__
/******************************************************************************/
#define AT_REG_DEBUG    0

typedef struct {
    unsigned int addr;
    unsigned int value;
#if AT_REG_DEBUG
    char name[16];
#endif
} at_reg_t;

#if AT_REG_DEBUG
#define AT_REG(_name, _addr, _value)    { \
    .addr   = _addr,    \
    .value  = _value,   \
    .name   = _name,    \
}   /* end */
#else
#define AT_REG(_name, _addr, _value)    { \
    .addr   = _addr,    \
    .value  = _value,   \
}   /* end */
#endif
/*
* just for fastboot !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*/
#if defined(__FASTBOOT__)

#if AT_REG_DEBUG
#define io_read( _addr) ({ \
    unsigned int addr   = (_addr); \
    unsigned int value  = *(volatile unsigned int *)addr; \
    printf("read addr:0x%x, value:0x%x\n", addr, value); \
    value; \
})

#define io_write(_addr, _val) do{   \
    unsigned int addr   = (_addr);  \
    unsigned int value  = (_val);   \
                                    \
    *(volatile unsigned int *)addr = value; \
                                    \
    printf("write addr:0x%x, value:0x%x\n", addr, value); \
}while(0)
#else
#define io_read( _addr)         (*(volatile unsigned int *)(_addr))
#define io_write(_addr, _val)   (*(volatile unsigned int *)(_addr) = (_val))
#endif

static inline void
__at_reg_write(at_reg_t reg[], int count)
{
    int i;

    for (i=0; i<count; i++) {
        io_write(reg[i].addr, reg[i].value);
    }
}
#define at_reg_write(_reg)          __at_reg_write(_reg, sizeof(_reg)/sizeof(_reg[0]))
#endif

enum {
    AT_DOG0 = 0xF8A2C000,
    AT_DOG1 = 0xF8A2D000,
};
#define AT_DOG(_dog)    AT_DOG##_dog

enum {
    AT_DOG_LOAD     = 0x0000,
    AT_DOG_VALUE    = 0x0004,
    AT_DOG_CONTROL  = 0x0008,
    AT_DOG_INTCLR   = 0x000C,
    AT_DOG_RIS      = 0x0010,
    AT_DOG_MIS      = 0x0014,
    AT_DOG_LOCK     = 0x0C00,
};

#define dog_reg(_dog, _reg)         (AT_DOG(_dog) + (_reg))

enum {
    AT_DOG_LOAD_DEFAULT     = (12000000*150),
    AT_DOG_LOCK_OPEN        = 0x1ACCE551,
    AT_DOG_LOCK_CLOSE       = 0,
    AT_DOG_CONTROL_ENABLE   = 0x3,
    AT_DOG_CONTROL_DISABLE  = 0,
};

#define __AT_REG_DOG_ENABLE(_dog)   \
    AT_REG("dog lock",      dog_reg(_dog, AT_DOG_LOCK),     AT_DOG_LOCK_OPEN), \
    AT_REG("dog load",      dog_reg(_dog, AT_DOG_LOAD),     AT_DOG_LOAD_DEFAULT), \
    AT_REG("dog control",   dog_reg(_dog, AT_DOG_CONTROL),  AT_DOG_CONTROL_ENABLE), \
    AT_REG("dog lock",      dog_reg(_dog, AT_DOG_LOCK),     AT_DOG_LOCK_CLOSE) \
    /* end */

#define AT_REG_DOG_ENABLE { \
    __AT_REG_DOG_ENABLE(0), \
    __AT_REG_DOG_ENABLE(1), \
}   /* end */
/******************************************************************************/
enum {
    AT_OTP          = 0xF8AB0000,
    AT_OTP_PRIVATER = (AT_OTP + 0x2B0),
    AT_OTP_CUSTOMER = (AT_OTP + 0x2C0),
};
/******************************************************************************/
#endif /* __AT_DOG_H_DE28C2611313B5650AF7DB2BFB5828D4__ */
