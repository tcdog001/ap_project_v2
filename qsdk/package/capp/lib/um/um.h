#ifndef __UM_H_A4CC4D1DB85063305FF56A7C1F1D3A0F__
#define __UM_H_A4CC4D1DB85063305FF56A7C1F1D3A0F__
/******************************************************************************/
#include "utils.h"
/******************************************************************************/
#ifndef ENV_UMD_UNIX
#define ENV_UMD_UNIX            "__UMD_UNIX__"
#endif

#ifndef ENV_UMC_UNIX
#define ENV_UMC_UNIX            "__UMC_UNIX__"
#endif

#ifndef ENV_UM_TIMEOUT
#define ENV_UM_TIMEOUT          "__UM_TIMEOUT__"
#endif

#ifndef UMD_UNIX
#define UMD_UNIX                "/tmp/.umd.unix"
#endif

#ifndef UMC_UNIX
#define UMC_UNIX                "/tmp/.umc.%d.unix"
#endif

#ifndef UM_TIMEOUT
#define UM_TIMEOUT              3000    /* ms */
#endif

static inline int
get_umd_path_env(struct sockaddr_un *addr) 
{
    return __copy_string_env(ENV_UMD_UNIX, UMD_UNIX, 
                get_abstract_path(addr),
                abstract_path_size);
}

static inline int
get_umc_path_env(struct sockaddr_un *addr) 
{
    char path[1+OS_LINE_LEN] = {0};

    os_saprintf(path, UMC_UNIX, getpid());
    
    return __copy_string_env(ENV_UMC_UNIX, path, 
                get_abstract_path(addr),
                abstract_path_size);
}

static inline int
get_um_timeout_env(void) 
{
    return get_int_env(ENV_UM_TIMEOUT, UM_TIMEOUT);
}
/******************************************************************************/
#endif /* __UM_H_A4CC4D1DB85063305FF56A7C1F1D3A0F__ */
