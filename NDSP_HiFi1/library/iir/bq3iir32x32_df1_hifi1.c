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
  NatureDSP Signal Processing Library. IIR part
    Bi-quad Real Block IIR, 32x32-bit, Direct Form I
    Optimized code for HiFi1
*/

/*-------------------------------------------------------------------------
  Bi-quad Block IIR
  Computes a IIR filter (cascaded IIR direct form I or II using 5 
  coefficients per bi-quad + gain term) . Input data are stored in vector x. 
  Filter output samples are stored in vector r. The filter calculates N output 
  samples using SOS and G matrices.
  Filters are able to process data in following formats:
  -  real (just array of samples)
  -  2-way or complex (interleaved real/imaginary samples)
  -  3-way (stream of interleaved samples from 3 channels)
  The same coefficients are used for filtering of multiple channels or 
  real/imaginary parts and they are processed independently. 
  The same format have to be used both for input and output streams.
  NOTES:
  1. Bi-quad coefficients may be derived from standard SOS and G matrices
     generated by MATLAB. However, typically biquad stages have big peaks 
     in their step response which may cause undesirable overflows at the 
     intermediate outputs. To avoid that the additional scale factors 
     coef_g[M] may be applied. These per-section scale factors may require 
     some tuning to find a compromise between quantization noise and possible
     overflows. Output of the last section is directed to an additional 
     multiplier, with the gain factor being a power of two, either negative 
     or non-negative. It is specified through the total gain shift amount 
     parameter gain of each filter initialization function.
  2. 16x16 filters may suffer more from accumulation of the roundoff errors, 
     so filters should be properly designed to match noise requirements

  Precision: 
  16x16  16-bit data, 16-bit coefficients, 16-bit intermediate stage outputs 
         (DF I, DF II), real data
  16x16  16-bit data, 16-bit coefficients, 16-bit intermediate stage outputs 
         (DF I, DF II), 3-way data
  24x24  32-bit data, 24-bit coefficients, 32-bit intermediate stage outputs 
         (DF I, DF II), real data
  32x16  32-bit data, 16-bit coefficients, 32-bit intermediate stage outputs 
         (DF I, DF II), real data
  32x16  32-bit data, 16-bit coefficients, 32-bit intermediate stage outputs 
         (DF I, DF II), 3-way data
  32x32  32-bit data, 32-bit coefficients, 32-bit intermediate stage outputs 
         (DF I, DF II)
  32x32  32-bit data, 32-bit coefficients, 32-bit intermediate stage outputs 
         (DF I, DF II) 3-way data
  f      floating point (DF I, DF II and DF IIt)
  f      floating point (DF I), 2-way (complex) data
  f      floating point (DF I, DF II) 3-way data

   ----------------+--------------------------------
   Functon         | Scratch memory, bytes
   ----------------+--------------------------------
   bqriir16x16_df1 | BQRIIR16X16_DF1_SCRATCH_SIZE(M)
   bqriir16x16_df2 | BQRIIR16X16_DF2_SCRATCH_SIZE(M)
   bq3iir16x16_df1 | BQ3IIR16X16_DF1_SCRATCH_SIZE(M)
   bq3iir16x16_df2 | BQ3IIR16X16_DF2_SCRATCH_SIZE(M)
   bqriir24x24_df1 | BQRIIR24X24_DF1_SCRATCH_SIZE(M)
   bqriir24x24_df2 | BQRIIR24X24_DF2_SCRATCH_SIZE(M)
   bqriir32x16_df1 | BQRIIR32X16_DF1_SCRATCH_SIZE(M)
   bqriir32x16_df2 | BQRIIR32X16_DF2_SCRATCH_SIZE(M)
   bq3iir32x16_df1 | BQ3IIR32X16_DF1_SCRATCH_SIZE(M)
   bq3iir32x16_df2 | BQ3IIR32X16_DF2_SCRATCH_SIZE(M)
   bqriir32x32_df1 | BQRIIR32X32_DF1_SCRATCH_SIZE(M)
   bqriir32x32_df2 | BQRIIR32X32_DF2_SCRATCH_SIZE(M)
   bq3iir32x32_df1 | BQ3IIR32X32_DF1_SCRATCH_SIZE(M)
   bq3iir32x32_df2 | BQ3IIR32X32_DF2_SCRATCH_SIZE(M)
   ----------------+--------------------------------
  Input:
  N             length of input sample block. For 3-way functions (bq3iirxxx), 
                N is a number of triplets
  M             number of bi-quad sections
  s[]           scratch memory area (for fixed-point functions only), Minimum 
                number of bytes depends on selected filter structure and 
                precision. see table above .If a particular macro returns zero, 
                then the corresponding IIR doesn't require a scratch area and 
                parameter s may hold zero

  coef_sos[M*5] filter coefficients stored in blocks of 5 numbers: 
                b0 b1 b2 a1 a2. 
                For fixed-point funcions, fixed point format of filter 
                coefficients is Q1.14 for 32x16, or Q1.30 for 32x16 and 
                24x24 (in the latter case 8 LSBs are actually ignored). 
  coef_g[M]     scale factor for each section, Q15 (for fixed-point 
                functions only). Please note that 24x24 DFI implementation 
                internally truncates scale factors to Q7 values.
  gain          total gain shift amount applied to output signal of the
                last section, -48..15
  x[N]          input samples, Q31, Q15 or floating point. For 3-way functions 
                (bq3iirxxx), N is a number of triplets, so array size should be 
                3*N.

  Output:
  r[N]          output data, Q31, Q15 or floating point. For 3-way functions 
                (bq3iirxxx), N is a number of triplets, so array size should be 
                3*N.

  Restriction:
  x,r,s,coef_g,coef_sos should not overlap
  N   - must be a multiple of 2
  s[] - whenever supplied must be aligned on an 8-bytes boundary

-------------------------------------------------------------------------*/

/* Portable data types. */
#include "NatureDSP_types.h"
/* Signal Processing Library API. */
#include "NatureDSP_Signal.h"
/* Common utility and macros declarations. */
#include "common.h"

/* Instance pointer validation number. */
#define MAGIC     0xB421C7CD

/* Reserve memory for alignment. */
#define ALIGNED_SIZE( size, align ) \
      ( (size_t)(size) + (align) - 1 )

/* Align address on a specified boundary. */
#define ALIGNED_ADDR( addr, align ) \
      (void*)( ( (uintptr_t)(addr) + ( (align) - 1 ) ) & ~( (align) - 1 ) )

#define sz_i32 sizeof(int32_t)

/* Filter instance structure. */
typedef struct tag_bq3iir32x32_df1_t
{
  uint32_t        magic; // Instance pointer validation number
  int             M;     // Number of sections
  int16_t         gain;  // Total gain shift amount for the last biquad
  const int32_t * coef;  // Num/den coefs (Q30) and gain (Q31) for each biquad
  int32_t *       state; // 3*4 state elements per section, Q31

} bq3iir32x32_df1_t, *bq3iir32x32_df1_ptr_t;

// Determine the memory area size for a filter instance.
size_t bq3iir32x32_df1_alloc( int M )
{
  ASSERT( M >= 0 );

  return ( ALIGNED_SIZE( sizeof(bq3iir32x32_df1_t), 4 )
           + // 4 state elements for each of M DFI sections.
           ALIGNED_SIZE( 3*4*M*sz_i32, 2*sz_i32 )
           + // 6 coefficients for each of M sections
           ALIGNED_SIZE( 6*M*sz_i32, 2*sz_i32 ) );

} // bq3iir32x32_df1_alloc()

// Given a memory area of sufficient size, initialize a filter instance and 
// return a handle to it.
bq3iir32x32_df1_handle_t bq3iir32x32_df1_init( void * objmem,  int M, 
                                               const int32_t * coef_sos,
                                               const int16_t * coef_g,
                                               int16_t         gain )
{
  bq3iir32x32_df1_ptr_t bq3iir;

  ae_int16 * restrict Pcoef_g;
  ae_int32 * restrict Pcoef_sos;
  ae_int32 * restrict PsectCoef;
  ae_int32 * restrict PsectState;
  ae_int16x4 _g;
  ae_int32x2 b0,b1,b2,a1,a2,_0;
  ae_f64 acc;
  int32_t * sectState;
  int32_t * sectCoef;
  void *    ptr;
  int m;

  ASSERT( objmem &&  M >= 0 && coef_sos && coef_g );

  ASSERT( -48 <= gain && gain <= 15 );

  //
  // Partition the memory block
  //

  ptr = objmem;

  bq3iir    = (bq3iir32x32_df1_ptr_t)ALIGNED_ADDR( ptr, 4 );
  ptr       = bq3iir + 1;
  sectState = (int32_t *)ALIGNED_ADDR( ptr, 2*sz_i32 );
  ptr       = sectState + 3*4*M;
  sectCoef  = (int32_t *)ALIGNED_ADDR( ptr, 2*sz_i32 );
  ptr       = sectCoef + 6*M;

  ASSERT( (int8_t*)ptr - (int8_t*)objmem <= (int)bq3iir32x32_df1_alloc(M ) );
  
  //
  // Copy filter coefficients for M sections, zero the state elements.
  //
  Pcoef_g   = (ae_int16 *)coef_g;
  Pcoef_sos = (ae_int32 *)coef_sos;
  PsectCoef = (ae_int32 *)sectCoef;
  PsectState = (ae_int32 *)sectState;
  _0 = 0;
  for ( m=0; m<M; m++ )
  {
    AE_L16_IP(_g,Pcoef_g,sizeof(int16_t));

    AE_L32_IP(b0,Pcoef_sos,sz_i32);
    AE_L32_IP(b1,Pcoef_sos,sz_i32);
    AE_L32_IP(b2,Pcoef_sos,sz_i32);
    AE_L32_IP(a1,Pcoef_sos,sz_i32);
    AE_L32_IP(a2,Pcoef_sos,sz_i32);
    //
    // Apply per-section gain to the filter coefficients
    //
    acc = AE_MUL32X16_L0(b0, _g);
    b0 = AE_MOVINT32X2_FROMF64(AE_PKSR32_0(acc,1));
    acc = AE_MUL32X16_L0(b1, _g);
    b1 = AE_MOVINT32X2_FROMF64(AE_PKSR32_0(acc,1));
    acc = AE_MUL32X16_L0(b2, _g);
    b2 = AE_MOVINT32X2_FROMF64(AE_PKSR32_0(acc,1));

    AE_S32_L_IP(b0,PsectCoef,sz_i32);// b0 (Q30)
    AE_S32_L_IP(b0,PsectCoef,sz_i32);// b0 (Q30)
    AE_S32_L_IP(b1,PsectCoef,sz_i32);// b1 (Q30)
    AE_S32_L_IP(b2,PsectCoef,sz_i32);// b2 (Q30)
    AE_S32_L_IP(a1,PsectCoef,sz_i32);// a1 (Q30)
    AE_S32_L_IP(a2,PsectCoef,sz_i32);// a2 (Q30)

    AE_S32X2_IP(_0, castxcc(ae_int32x2, PsectState), 2 * sz_i32);
    AE_S32X2_IP(_0, castxcc(ae_int32x2, PsectState), 2 * sz_i32);
    AE_S32X2_IP(_0, castxcc(ae_int32x2, PsectState), 2 * sz_i32);
    AE_S32X2_IP(_0, castxcc(ae_int32x2, PsectState), 2 * sz_i32);
    AE_S32X2_IP(_0, castxcc(ae_int32x2, PsectState), 2 * sz_i32);
    AE_S32X2_IP(_0, castxcc(ae_int32x2, PsectState), 2 * sz_i32);
  }

  //
  // Initialize the filter instance.
  //

  bq3iir->magic = MAGIC;
  bq3iir->M     = M;
  bq3iir->gain  = gain;
  bq3iir->coef  = sectCoef;
  bq3iir->state = sectState;

  return (bq3iir);

} // bq3iir32x32_df1_init()

// Process data. The filter instance is identified with a handle returned by
// the initializing function.
void bq3iir32x32_df1( bq3iir32x32_df1_handle_t _bq3iir,
                      void    * restrict       s,
                      int32_t * restrict       r,
                const int32_t *                x, int N )
{
  bq3iir32x32_df1_ptr_t bq3iir = (bq3iir32x32_df1_ptr_t)_bq3iir;
  ae_int32x2 * restrict Px;
  ae_int32x2 * restrict Pr;
  ae_int32x2 * restrict Pstate;
  ae_int32x2 * restrict Psosg;
  ae_f32x2 sx1sx0, sr1sr0;
  ae_f32x2 sx1, sx0, sr1, sr0, x1, x0;
  ae_f32x2 b0b0, b1b2, a1a2;
  ae_int32x2 R0x2, R1x2;
  ae_int32 X0, X1, R0, R1;
  ae_f64 acc0, acc1;

  int M;
  int m,n,l,gain;

  ASSERT( bq3iir && bq3iir->magic == MAGIC && r && x );
  NASSERT(N%2==0);
  if(N<=0) return;

  M = bq3iir->M;
  gain = bq3iir->gain;
  Pstate = (ae_int32x2 *)(bq3iir->state);
  
  // Process the input 3-way data by 1 stream
  for (l=0; l<3; l++)
  {
    Psosg  = (ae_int32x2 *)(bq3iir->coef );
    Px     = (ae_int32x2 *)(x+l);
    // Perform data block processing for each section. Use the output array r[N]
    // for temporal storage of inter-section signal.
    for ( m=0; m<M-1; m++ )
    {
      //
      // Load 32-bit section coefficients.
      //
      ae_f32x2_loadip(b0b0,Psosg,2*sz_i32);
      ae_f32x2_loadip(b1b2,Psosg,2*sz_i32);
      ae_f32x2_loadip(a1a2,Psosg,2*sz_i32);

      //
      // Load 32-bit section's state elements.
      //
      sx1sx0 = ae_int32x2_loadi(Pstate, 0);
      sr1sr0 = ae_int32x2_loadi(Pstate, 2*sz_i32);
      sx1 = AE_SEL32_HH(sx1sx0, sx1sx0);
      sx0 = AE_SEL32_LL(sx1sx0, sx1sx0);
      sr1 = AE_SEL32_HH(sr1sr0, sr1sr0);
      sr0 = AE_SEL32_LL(sr1sr0, sr1sr0);

      //
      // Pass the block of input samples through the section. n-th sample at the
      // output of a biquad section:
      //   r[n] = g*x[n]*b0 + g*x[n-1]*b1 + g*x[n-2]*b2 - r[n-1]*a1 - r[n-2]*a2
      //
      Pr = (ae_int32x2 *)(r+l);
      __Pragma("loop_count min=1")
      for ( n=0; n<(N>>1); n++ )
      {
        // Load 2 input samples.
        ae_int32_loadip(X0,castxcc(const ae_int32, Px),3*sz_i32);
        ae_int32_loadip(X1,castxcc(const ae_int32, Px),3*sz_i32);
        x0 = AE_MOVF32X2_FROMINT32(X0);
        x1 = AE_MOVF32X2_FROMINT32(X1);
      
        // Compute the 1-st output sample.
        // Q17.46 <- Q17.46 + [ Q30*Q31 + 1 - 16 w/ sym. rounding ]
        acc0 = AE_MULF32R_LL(x0 , b0b0);
        AE_MULAF32R_LL(acc0, sx1, b1b2);
        AE_MULSF32R_LL(acc0, sr1, a1a2);
        AE_MULAF32R_LH(acc0, sx0, b1b2);
        AE_MULSF32R_LH(acc0, sr0, a1a2);
        // Q31 <- Q17.46 + 1 - 16 w/ asym. rounding and saturation
        R0=AE_MOVINT32_FROMF64(AE_PKSR32_0(acc0,1));
        R0x2=AE_MOVINT32X2_FROMINT32(R0);
      
        // Compute the 2-nd output sample.
        // Q17.46 <- Q17.46 + [ Q30*Q31 + 1 - 16 w/ sym. rounding ]
        acc1 = AE_MULF32R_LL(x1 , b0b0);
        AE_MULAF32R_LL(acc1, sx0, b1b2);
        AE_MULSF32R_LL(acc1, sr0, a1a2);
        AE_MULAF32R_LH(acc1, x0 , b1b2);
        AE_MULSF32R_LH(acc1, R0x2,a1a2);
        // Q31 <- Q17.46 + 1 - 16 w/ asym. rounding and saturation
        R1=AE_MOVINT32_FROMF64(AE_PKSR32_0(acc1,1));
        R1x2=AE_MOVINT32X2_FROMINT32(R1);

        // Update the section's state elements.
        sx1 = x0;
        sx0 = x1;
        sr1 = R0x2;
        sr0 = R1x2;

        // Save the output samples.
        ae_int32_storeip(R0,castxcc(ae_int32, Pr),3*sz_i32);
        ae_int32_storeip(R1,castxcc(ae_int32, Pr),3*sz_i32);
      }

      //
      // Save section's state elements.
      //
      sx1sx0 = AE_SEL32_LL(sx1, sx0);
      sr1sr0 = AE_SEL32_LL(sr1, sr0);
      ae_int32x2_storeip(sx1sx0,Pstate,2*sz_i32);
      ae_int32x2_storeip(sr1sr0,Pstate,2*sz_i32);

      //
      // 2nd to the last sections are fed with output samples of the preceding
      // biquad.
      //

      Px = (ae_int32x2 *)(r+l);
    }

    //----------------------//
    // Process last section //
    //----------------------//

    {
      //
      // Load 32-bit section coefficients.
      //
      b0b0 = ae_f32x2_loadi((ae_f32x2 *)Psosg,0*sz_i32);
      b1b2 = ae_f32x2_loadi((ae_f32x2 *)Psosg,2*sz_i32);
      a1a2 = ae_f32x2_loadi((ae_f32x2 *)Psosg,4*sz_i32);

      //
      // Load 32-bit section's state elements.
      //
      sx1sx0 = ae_int32x2_loadi(Pstate, 0);
      sr1sr0 = ae_int32x2_loadi(Pstate, 2*sz_i32);
      sx1 = AE_SEL32_HH(sx1sx0, sx1sx0);
      sx0 = AE_SEL32_LL(sx1sx0, sx1sx0);
      sr1 = AE_SEL32_HH(sr1sr0, sr1sr0);
      sr0 = AE_SEL32_LL(sr1sr0, sr1sr0);

      //
      // Pass the block of input samples through the section. n-th sample at the
      // output of a biquad section:
      //   r[n] = g*x[n]*b0 + g*x[n-1]*b1 + g*x[n-2]*b2 - r[n-1]*a1 - r[n-2]*a2
      //
      Pr = (ae_int32x2 *)(r+l);
      __Pragma("loop_count min=1")
      for ( n=0; n<(N>>1); n++ )
      {
        // Load 2 input samples.
        ae_int32_loadip(X0,castxcc(const ae_int32, Px),3*sz_i32);
        ae_int32_loadip(X1,castxcc(const ae_int32, Px),3*sz_i32);
        x0 = AE_MOVF32X2_FROMINT32(X0);
        x1 = AE_MOVF32X2_FROMINT32(X1);
      
        // Compute the 1-st output sample.
        // Q17.46 <- Q17.46 + [ Q30*Q31 + 1 - 16 w/ sym. rounding ]
        acc0 = AE_MULF32R_LL(x0 , b0b0);
        AE_MULAF32R_LL(acc0, sx1, b1b2);
        AE_MULSF32R_LL(acc0, sr1, a1a2);
        AE_MULAF32R_LH(acc0, sx0, b1b2);
        AE_MULSF32R_LH(acc0, sr0, a1a2);
        // Q31 <- Q17.46 + 1 - 16 w/ asym. rounding and saturation
        R0=AE_MOVINT32_FROMF64(AE_PKSR32_0(acc0,1));
        R0x2=AE_MOVINT32X2_FROMINT32(R0);
      
        // Compute the 2-nd output sample.
        // Q17.46 <- Q17.46 + [ Q30*Q31 + 1 - 16 w/ sym. rounding ]
        acc1 = AE_MULF32R_LL(x1 , b0b0);
        AE_MULAF32R_LL(acc1, sx0, b1b2);
        AE_MULSF32R_LL(acc1, sr0, a1a2);
        AE_MULAF32R_LH(acc1, x0 , b1b2);
        AE_MULSF32R_LH(acc1, R0x2,a1a2);
        // Q31 <- Q17.46 + 1 - 16 w/ asym. rounding and saturation
        R1=AE_MOVINT32_FROMF64(AE_PKSR32_0(acc1,1));
        R1x2=AE_MOVINT32X2_FROMINT32(R1);

        // Update the section's state elements.
        sx1 = x0;
        sx0 = x1;
        sr1 = R0x2;
        sr0 = R1x2;

        // Apply the total gain shift
        // Q(17.47+gain) <- [ Q17.46 + 1 + gain ] w/ saturation
        acc0 = AE_F64_SLAS(acc0, 1+gain);
        acc1 = AE_F64_SLAS(acc1, 1+gain);
      
        // Format and save the output samples.
        // Q31 <- Q17.47 - 16 w/ asym. rounding and saturation
		ae_f32x2 tmp = AE_ROUND32X2F48SASYM(acc0,acc1);
		ae_f32x2 tmp_1 = AE_INTSWAP(tmp);
		AE_S32_L_IP(tmp_1, castxcc(ae_int32, Pr), 3 * sz_i32);
		AE_S32_L_IP(tmp, castxcc(ae_int32, Pr), 3 * sz_i32);
      }

      //
      // Save section's state elements.
      //
      sx1sx0 = AE_SEL32_LL(sx1, sx0);
      sr1sr0 = AE_SEL32_LL(sr1, sr0);
      ae_int32x2_storeip(sx1sx0,Pstate,2*sz_i32);
      ae_int32x2_storeip(sr1sr0,Pstate,2*sz_i32);
    }
  }
}

