/*-----------------------------------------------------------------------------
 *      Name:         DV_Typedefs.h 
 *      Purpose:      Test framework filetypes and structures description
 *----------------------------------------------------------------------------
 *      Copyright(c) KEIL - An ARM Company
 *----------------------------------------------------------------------------*/
#ifndef __TYPEDEFS_H__
#define __TYPEDEFS_H__

#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "unity.h"

typedef unsigned int    BOOL;

#ifndef __TRUE
 #define __TRUE         1
#endif
#ifndef __FALSE
 #define __FALSE        0
#endif

#ifndef ENABLED
 #define ENABLED        1
#endif
#ifndef DISABLED
 #define DISABLED       0
#endif

#ifndef NULL
 #ifdef __cplusplus              // EC++
  #define NULL          0
 #else
  #define NULL          ((void *) 0)
 #endif
#endif

#ifdef ARRAY_SIZE
  #undef ARRAY_SIZE
  #define ARRAY_SIZE(arr) (sizeof(arr)/sizeof((arr)[0]))
#endif

#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
  
/* Assertions and test results */
#define SET_RESULT(res, desc) __set_result(__FILENAME__, __LINE__, res, desc);
#define ASSERT_TRUE(cond) TEST_ASSERT_TRUE(cond);

#endif /* __TYPEDEFS_H__ */
