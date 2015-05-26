extern unsigned int __AKID_DEBUG;
static unsigned int *__debug_level_pointer = &__AKID_DEBUG;


/******************************************************************************/
#include "kblob.h"
unsigned int __AKID_DEBUG = __debug_level_default;
#include "blob/blob.c"



EXPORT_SYMBOL(blob_parse);
EXPORT_SYMBOL(blob_new);
EXPORT_SYMBOL(blob_root_init);
EXPORT_SYMBOL(__blob_nest_start);
EXPORT_SYMBOL(__blob_nest_end);
EXPORT_SYMBOL(blob_sprintf);
EXPORT_SYMBOL(blob_put);

/******************************************************************************/
