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


/*  
  Profiler for HiFi3 core-based processor
*/

#ifndef INTEGRIT_PROFILER_H
#define INTEGRIT_PROFILER_H
#ifdef COMPILER_XTENSA
#include <xtensa/config/core.h>
#if XCHAL_HAVE_CCOUNT
#ifndef NO_XT_RSR_CCOUNT
#include <xtensa/tie/xt_timer.h>
#endif
#ifdef XT_RSR_CCOUNT
#define GETCLOCK() XT_RSR_CCOUNT()
#else
static unsigned long inline GETCLOCK(void)
{
  unsigned long r;
  __asm__ volatile ("rsr.ccount %0" : "=r" (r));
  return r;
}
#endif
#endif
#endif

#ifndef GETCLOCK
#include <time.h>
#define GETCLOCK() clock()
#endif


#define PROFILE(f) {  \
  uint32_t p0, p1;    \
  p0=GETCLOCK();      \
  f; p1=GETCLOCK();   \
  profiler_clk=p1-p0; }

#define NORMALIZE(y, x, f) { y = x * 100 + f/2; y /= f; }
#define INVERT(y, x, f)    if(x) { y = f * 100 + x/2;  y /= x; } else y=0;

extern uint32_t profiler_clk;
extern uint32_t profiler_cpm;


#endif /* INTEGRIT_PROFILER_H */
