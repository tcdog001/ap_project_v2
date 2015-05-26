extern unsigned int __AKID_DEBUG;
static unsigned int *__debug_level_pointer = &__AKID_DEBUG;


#include "kslice.h"
unsigned int __AKID_DEBUG = __debug_level_default;
#include "slice/slice.c"
/******************************************************************************/


EXPORT_SYMBOL(slice_vsprintf);
EXPORT_SYMBOL(slice_sprintf);

/******************************************************************************/

