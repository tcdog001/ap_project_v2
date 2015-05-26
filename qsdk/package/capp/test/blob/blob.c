/******************************************************************************/
#ifndef __THIS_APP
#define __THIS_APP      testblob
#endif
    
#ifndef __THIS_NAME
#define __THIS_NAME     "testblob"
#endif

#include "utils.h"

OS_INITER;

static slice_t BS;
static slice_t *bs = &BS;

#define BUFFER_SIZE     1024

static void
put_somthing(void)
{
    blob_put_bool(bs, 0, "bool", true);
    blob_put_binary(bs, 0, "binary", "bbbbbbb", 7);
    blob_put_string(bs, 0, "string", "sssssss");
    blob_sprintf(bs 0, "sprintf", "%s-%d", "sssssss", 7);
    blob_put_u32(bs, 0, "u32", 0xffffffff);
    blob_put_u64(bs, 0, "u64", 0xffffffffffffffff);
    blob_put_i32(bs, 0, "i32", -1);
    blob_put_i64(bs, 0, "i64", -1);
}

#define COUNT   1000
int main(int argc, char *argv[])
{
    char *json;
    void *arr, *obj;
    int i;
    
    slice_alloc(bs, BUFFER_SIZE);    
    blob_root_init(bs, 0, BLOB_T_OBJECT, "root");
    
    put_somthing();
    debug_trace("1:root blob len=%d", blob_len(blob_root(bs)));
    
    obj = blob_object_start(bs, 0, "obj");
    for (i=0; i<COUNT; i++) {
        debug_trace("obj %d begin", i);
        arr = blob_array_start(bs, 0, "array");
        put_somthing();
        blob_array_end(bs, arr);
        debug_trace("obj %d end", i);
    }
    blob_object_end(bs, obj);
    debug_ok("2:root blob len=%d", blob_len(blob_root(bs)));
    
    arr = blob_array_start(bs, 0, "array");
    for (i=0; i<COUNT; i++) {
        debug_trace("array %d begin", i);
        obj = blob_object_start(bs, 0, "obj");
        put_somthing();
        blob_object_end(bs, obj);
        debug_trace("array %d end", i);
    }
    blob_array_end(bs, arr);
    debug_trace("3:root blob len=%d", blob_len(blob_root(bs)));
    
    put_somthing();
    debug_trace("4:root blob len=%d", blob_len(blob_root(bs)));

#if 0
    json = blob_to_json(blob_root(bs), false);
    os_println("%s", json);
#endif

    return 0;
}
/******************************************************************************/
