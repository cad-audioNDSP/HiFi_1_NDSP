/* ------------------------------------------------------------------------ */
/* Copyright (c) 2022 by Cadence Design Systems, Inc. ALL RIGHTS RESERVED.  */
/* These coded instructions, statements, and computer programs ('Cadence    */
/* Libraries') are the copyrighted works of Cadence Design Systems Inc.     */
/* Cadence IP is licensed for use with Cadence processor cores only and     */
/* must not be used for any other processors and platforms. Your use of the */
/* Cadence Libraries is subject to the terms of the license agreement you   */
/* have entered into with Cadence Design Systems, or a sublicense granted   */
/* to you by a direct Cadence license.                                      */
/* ------------------------------------------------------------------------ */
/*  IntegrIT, Ltd.   www.integrIT.com, info@integrIT.com                    */
/*                                                                          */
/* NatureDSP Signal Library                                                 */
/*                                                                          */
/* This library contains copyrighted materials, trade secrets and other     */
/* proprietary information of IntegrIT, Ltd. This software is licensed for  */
/* use with Cadence processor cores only and must not be used for any other */
/* processors and platforms. The license to use these sources was given to  */
/* Cadence, Inc. under Terms and Condition of a Software License Agreement  */
/* between Cadence, Inc. and IntegrIT, Ltd.                                 */
/* ------------------------------------------------------------------------ */
/*          Copyright (C) 2009-2022 IntegrIT, Limited.                      */
/*                      All Rights Reserved.                                */
/* ------------------------------------------------------------------------ */
#ifndef __COMMON_H__
#define __COMMON_H__

#include <assert.h>

#include "NatureDSP_Signal.h"
#ifdef __RENAMING__
//#include "__renaming__.h"
#endif

#if defined COMPILER_XTENSA
  #include <xtensa/config/core-isa.h>
  #include <xtensa/tie/xt_core.h>
  #include <xtensa/tie/xt_misc.h>
  #include <xtensa/tie/xt_hifi3.h>
  #include <xtensa/tie/xt_hifi2.h>
#if XCHAL_HAVE_HIFI1
    #if XCHAL_HAVE_HIFI1_VFPU
    #include <xtensa/tie/xt_FP.h>
    #define HAVE_VFPU 1
  #endif
#endif
#else
#include "core-isa.h"
#if defined COMPILER_MSVC
  #pragma warning( disable :4800 4244)
#endif
/* the code below causes inclusion of file "cstub-"XTENSA_CORE".h" */
#define PPCAT_NX(A, B) A-B
#define PPCAT(A, B) PPCAT_NX(A, B)      /* Concatenate preprocessor tokens A and B after macro-expanding them. */
#define STRINGIZE_NX(A) #A              /* Turn A into a string literal without expanding macro definitions */
#define STRINGIZE(A) STRINGIZE_NX(A)    /*  Turn A into a string literal after macro-expanding it. */
//#include STRINGIZE(PPCAT(cstub,XTENSA_CORE).h)
#include STRINGIZE(PPCAT(PPCAT(cstub,XTENSA_CORE),c.h))
#endif


//-----------------------------------------------------
// C99 pragma wrapper
//-----------------------------------------------------

#ifdef COMPILER_XTENSA
#define __Pragma(a) _Pragma(a)
#else
#define __Pragma(a)
#endif

#define IS_ALIGN(p) ((((int)(p))&0x7) == 0) 

#ifdef _MSC_VER
    #define ALIGN(x)    _declspec(align(x)) 
#else
    #define ALIGN(x)    __attribute__((aligned(x))) 
#endif

extern const int32_t tab_invQ30[128];

#if !(XCHAL_HAVE_NSA)
  inline_ int32_t XT_NSA(int32_t n)
  {
    ae_q56s t;
    if (!n) return 31;
    t = AE_CVTQ48A32S(n);
    return AE_NSAQ56S(t)-8;
  }
#endif

// special XCC type casting of pointers
#ifdef __cplusplus
#define castxcc(type_,ptr)  (ptr)
#else
#define castxcc(type_,ptr)  (type_ *)(ptr)
#endif

// return 64-bit data converting from ae_int64
#ifdef __cplusplus
#define return_int64(x) return vai;
#else
#define return_int64(x) {  union {ae_int64  ai;int64_t   i; } r; r.ai = vai;  return r.i; }
#endif

#if  defined (__cplusplus) || defined(COMPILER_XTENSA)

#else
#error sorry, C compiler is not supported excluding the XCC
#endif


#ifdef COMPILER_MSVC
#define MSC_ALIGNED ALIGN(8)
#define GCC_ALIGNED
#else
#define MSC_ALIGNED
#define GCC_ALIGNED ALIGN(8)
#endif

typedef const int32_t * cint32_ptr;
typedef const uint64_t * cuint64_ptr;
typedef const short * cint16_ptr;


//-----------------------------------------------------
// Conditionalization support
//-----------------------------------------------------
/* place DISCARD_FUN(retval_type,name) instead of function definition for functions
   to be discarded from the executable 
   THIS WORKS only for external library functions declared as extern "C" and
   not supported for internal references without "C" qualifier!
*/
#ifdef COMPILER_MSVC
#pragma section( "$DISCARDED_FUNCTIONS" , execute, discard )
#pragma section( "$$$$$$$$$$" , execute, discard )
#define DISCARD_FUN(retval_type,name,arglist) __pragma (alloc_text( "$DISCARDED_FUNCTIONS",name))\
__pragma(section( "$DISCARDED_FUNCTIONS" , execute, discard ))\
__pragma (warning(push))\
__pragma (warning( disable : 4026 4716))\
retval_type name arglist {}\
__pragma (warning(pop))
#endif

#if defined (COMPILER_GNU)
#define F_UNDERSCORE " "
#define DISCARD_FUN(retval_type,name,arglist)    \
__asm__                        \
(                              \
".section unused_section\n"    \
".globl " F_UNDERSCORE STRINGIZE(name) "\n" \
".type "F_UNDERSCORE STRINGIZE(name)", @function \n"\
F_UNDERSCORE STRINGIZE(name) ":\n"          \
".text"                        \
);
#endif

#if defined(COMPILER_XTENSA)
#define DISCARD_FUN(retval_type,name,arglist)  __asm__(".type "#name", @object\n\t.global "#name"\n\t.align 4\n\t"#name":\n\t.long 0x49438B96,0x4D73F192\n\t");
#endif 

#ifdef __cplusplus
#define externC extern "C" 
#else
#define externC extern 
#endif

/* maximum size (in bytes) allocated storage on stack by temporary arrays inside library functions */
#define MAX_ALLOCA_SZ 512

/* -----------------------------------------------------------------*/
/* redefine some xtfloat2 stuff - to be removed                     */
/* -----------------------------------------------------------------*/
#define XT_AE_MOVXTFLOAT_FROMINT32(x) XT_xtfloatx2_rtor_xtfloat( XT_AE_MOVXTFLOATX2_FROMINT32X2( AE_MOVINT32X2_FROMINT32( x ) ) )
#define XT_AE_MOVINT32_FROMXTFLOAT(x) AE_MOVINT32_FROMINT32X2( XT_AE_MOVINT32X2_FROMXTFLOATX2( XT_xtfloat_rtor_xtfloatx2( x ) ) );

/* -----------------------------------------------------------------*/
/* redefine some Scalar FPU Ops - to be removed                     */
/* -----------------------------------------------------------------*/

#if HAVE_VFPU
#define XT_MUL_LHL_S(a, b) XT_MUL_S(XT_HIGH_S(a), XT_LOW_S(b))
#define XT_MUL_LLL_S(a, b) XT_MUL_S(XT_LOW_S(a), XT_LOW_S(b))
#define XT_MUL_LLH_S(a, b) XT_MUL_S(XT_LOW_S(a), XT_HIGH_S(b))
#define XT_MUL_LHH_S(a, b) XT_MUL_S(XT_HIGH_S(a), XT_HIGH_S(b))

#define XT_MADD_LLL_S(c, a, b) XT_MADD_S(c, XT_LOW_S(a), XT_LOW_S(b))
#define XT_MADD_LLH_S(c, a, b) XT_MADD_S(c, XT_LOW_S(a), XT_HIGH_S(b))
#define XT_MADD_LHH_S(c, a, b) XT_MADD_S(c, XT_HIGH_S(a), XT_HIGH_S(b))
#define XT_MADD_LHL_S(c, a, b) XT_MADD_S(c, XT_HIGH_S(a), XT_LOW_S(b))

#define XT_MSUB_LLL_S(c, a, b) XT_MSUB_S(c, XT_LOW_S(a), XT_LOW_S(b))
#define XT_MSUB_LLH_S(c, a, b) XT_MSUB_S(c, XT_LOW_S(a), XT_HIGH_S(b))
#define XT_MSUB_LHH_S(c, a, b) XT_MSUB_S(c, XT_HIGH_S(a), XT_HIGH_S(b))
#define XT_MSUB_LHL_S(c, a, b) XT_MSUB_S(c, XT_HIGH_S(a), XT_LOW_S(b))
#endif

#endif
