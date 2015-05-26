#ifndef __ASSERT_H_4F501D5C3D10778EA387560D3B8CB7C0__
#define __ASSERT_H_4F501D5C3D10778EA387560D3B8CB7C0__

#if defined(__BOOT__) || defined(__APP__)
#define os_assert(_x)           assert(_x)
#else
/*
*   return os_assertV(x); 
*       ==> return ((void)0, x);
*
*   os_assert(0);
*       ==> (void)0;
*/
#define os_assert(_x)           (void)0
#endif

#define os_assertV(_x)          (os_assert(0), _x)
/******************************************************************************/
#endif /* __ASSERT_H_4F501D5C3D10778EA387560D3B8CB7C0__ */
