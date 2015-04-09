#ifndef __TIMER_H_7354F2DEF84483568CAD93F29E63C88B__
#define __TIMER_H_7354F2DEF84483568CAD93F29E63C88B__
/******************************************************************************/
#include "utils.h"
/******************************************************************************/
typedef struct { 
    char reserved[3*sizeof(void *) + 2*sizeof(uint32_t) + 1*sizeof(uint64_t)]; /* 28 byte */
} tm_node_t; /* should be embed in you struct */

typedef int tm_callback(tm_node_t *timer);

/*
* return
*   <0: error
*  >=0: sucess tigger times
*/
extern int
os_tm_trigger(uint32_t times);

extern uint64_t
os_tm_ticks(void);

extern void
os_tm_unit_set(uint32_t ms); // how much ms per tick

extern uint32_t //ms, how much ms per tick
os_tm_unit(void);

extern int
os_tm_insert(
    tm_node_t *timer, // should embed in other struct
    int after, // ticks, after now
    tm_callback *cb,
    bool cycle
);

extern int
os_tm_remove(tm_node_t *timer);

extern int
os_tm_change(tm_node_t *timer, uint32_t after);

extern bool
os_tm_is_pending(tm_node_t *timer);

/*
* return
*   >0: left time
*   =0: NOT pending
*   <0: error
*/
extern int //ticks
os_tm_left(tm_node_t *timer);

/*
* return
*   >0: expires(os_tm_insert's after)
*   =0: NOT pending
*   <0: error
*/
extern int //ticks
os_tm_expires(tm_node_t *timer);

/******************************************************************************/
#endif /* __TIMER_H_7354F2DEF84483568CAD93F29E63C88B__ */
