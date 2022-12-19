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
    Bi-quad Real Block IIR, 32x32-bit, Direct Form II
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
#define MAGIC     0xb6766c94

/* Reserve memory for alignment. */
#define ALIGNED_SIZE( size, align ) \
      ( (size_t)(size) + (align) - 1 )

/* Align address on a specified boundary. */
#define ALIGNED_ADDR( addr, align ) \
      (void*)( ( (uintptr_t)(addr) + ( (align) - 1 ) ) & ~( (align) - 1 ) )

#define sz_i16  sizeof(int16_t)
#define sz_i32  sizeof(int32_t)

/* Filter instance structure. */
typedef struct tag_bqriir32x32_df2_t
{
  uint32_t        magic;    // Instance pointer validation number
  int             M;        // Number of sections
  int32_t         gain;     // Gain shift amount applied after the last section
  const int32_t * coef_sos; // Per-section num/den coefs in a special order, Q30
  const int16_t * coef_g;   // Per-section scaling gain coefficients, Q15
  int32_t *       state;    // 2 state elements per section, Q31

} bqriir32x32_df2_t, *bqriir32x32_df2_ptr_t;

// Determine the memory area size for a filter instance.
size_t bqriir32x32_df2_alloc( int M )
{
  ASSERT( M >= 0 );
  return ( ALIGNED_SIZE( sizeof(bqriir32x32_df2_t), 4 )
           + // 2 state elements for each of M DFII sections
           ALIGNED_SIZE( 2*M*sz_i32, 8 )
           + // 8 SOS coefficients for each of M sections
           ALIGNED_SIZE( 8*M*sz_i32, 8 )
           + // g coefficient for each of M sections 
           ALIGNED_SIZE( M*sz_i16, 8 ) );

} // bqriir32x32_df2_alloc()

// Given a memory area of sufficient size, initialize a filter instance and 
// return a handle to it.
bqriir32x32_df2_handle_t bqriir32x32_df2_init( void * objmem, int M, 
                                               const int32_t * coef_sos,
                                               const int16_t * coef_g,
                                               int16_t         gain )
{
  bqriir32x32_df2_ptr_t bqriir;

  int32_t * sectState;
  int32_t * sectCoef_sos;
  int16_t * sectCoef_g;
  void *    ptr;

  int m;

  ASSERT( objmem && M >= 0 && coef_sos && coef_g );

  ASSERT( -48 <= gain && gain <= 15 );

  //
  // Partition the memory block
  //

  ptr = objmem;

  bqriir       = (bqriir32x32_df2_ptr_t)ALIGNED_ADDR( ptr, 4 );
  ptr          = bqriir + 1;
  sectState    = (int32_t*)ALIGNED_ADDR( ptr, 8 );
  ptr          = sectState + 2*M;
  sectCoef_sos = (int32_t*)ALIGNED_ADDR( ptr, 8 );
  ptr          = sectCoef_sos + 8*M;
  sectCoef_g   = (int16_t*)ALIGNED_ADDR( ptr, 8 );
  ptr          = sectCoef_g + M;

  ASSERT( (int8_t*)ptr - (int8_t*)objmem <= (int)bqriir32x32_df2_alloc( M ) );

  //
  // Copy filter coefficients for M sections, zero the state elements.
  //

  for ( m=0; m<M; m++ )
  {
    sectCoef_sos[8*m+0] = coef_sos[5*m+0]; // b0, Q30
    sectCoef_sos[8*m+1] = coef_sos[5*m+0]; // b0, Q30
    sectCoef_sos[8*m+2] = coef_sos[5*m+2]; // b2, Q30
    sectCoef_sos[8*m+3] = coef_sos[5*m+1]; // b1, Q30
    sectCoef_sos[8*m+4] = coef_sos[5*m+4]; // a2, Q30
    sectCoef_sos[8*m+5] = coef_sos[5*m+3]; // a1, Q30
    sectCoef_sos[8*m+6] = coef_sos[5*m+3]; // a1, Q30
    sectCoef_sos[8*m+7] = coef_sos[5*m+4]; // a2, Q30

    sectCoef_g[m] = coef_g[m]; // Q15

    sectState[2*m+0] = 0;
    sectState[2*m+1] = 0;
  }

  //
  // Initialize the filter instance.
  //

  bqriir->magic    = MAGIC;
  bqriir->M        = M;
  bqriir->gain     = gain;
  bqriir->coef_sos = sectCoef_sos;
  bqriir->coef_g   = sectCoef_g;
  bqriir->state    = sectState;

  return (bqriir);

} // bqriir32x32_df2_init()

// Process data. The filter instance is identified with a handle returned by
// the initializing function.
void bqriir32x32_df2( bqriir32x32_df2_handle_t _bqriir,
                      void    * restrict       s,
                      int32_t * restrict       r,
                const int32_t *                x, int N )
{
  bqriir32x32_df2_ptr_t bqriir = (bqriir32x32_df2_ptr_t)_bqriir;

  const ae_int32   * restrict X;
        ae_int32x2 * restrict R;
  const ae_f32x2   * restrict C_sos;
  const ae_int16   * restrict C_g;
        ae_f32x2   * restrict S;

  ae_valign R_va;

  ae_f32x2   cf_b0b0, cf_b2b1, cf_a2a1, cf_a1a2;
  ae_int16x4 cf_g;
  ae_f32x2   st;
  ae_int32x2 x0, x1, y_y0y1;
  ae_f64     y0, y1;
  ae_f64     q0, q1;
  ae_int64   u0, u1, u2, u3;

  int gain;

  int M;
  int m, n;

  ASSERT( bqriir && bqriir->magic == MAGIC && r && x );

  // This function requires a scratch area.
  ASSERT( s && IS_ALIGN( s ) );

  M = bqriir->M;

  ASSERT( !( N & 1 ) );
  if(N<=0) return;

  //
  // Setup pointers.
  //

  C_sos = (const ae_f32x2 *)bqriir->coef_sos;
  C_g   = (const ae_int16 *)bqriir->coef_g;
  S     = (      ae_f32x2 *)bqriir->state;

  // When processing the first biquad section, use x[N] for the data input.
  X = (const ae_int32 *)x;

  //----------------------------------------------------------------------------
  // Perform data block processing for each section but the last one.
  //

  for ( m=0; m<M-1; m++ )
  {
    // Output data of all biquads excepting the last one are stored to the
    // scratch.
    R = (ae_int32x2 *)s;

    //
    // Load m-th section's SOS/g coefficients.
    //

    // Q30
    ae_f32x2_loadip( cf_b0b0, C_sos, +8 );
    ae_f32x2_loadip( cf_b2b1, C_sos, +8 );
    ae_f32x2_loadip( cf_a2a1, C_sos, +8 );
    ae_f32x2_loadip( cf_a1a2, C_sos, +8 );

    // Q15
    AE_L16_IP( cf_g, C_g, +2 );

    //
    // Load m-th section's state.
    //

    // st[-2] st[-1]; Q31
    st = ae_f32x2_loadi( S, 0 );

    //
    // Pass the block of input samples through the section. Two outputs
    // y[n], y[n+1] are computed simultaneously:
    //   q0 = g*x[n]   - a1*st[-1] - a2*st[-2]
    //   q1 = g*x[n+1] - a1*q0     - a2*st[-1]
    //   y[n]   = b0*q0 + b1*st[-1] + b2*st[-2]
    //   y[n+1] = b0*q1 + b1*q0     + b2*st[-1]
    //   st[-2] = q0; st[-1] = q1
    //

    for ( n=0; n<N; n+=2 )
    {
      // Read x[n], x[n+1]; Q31
      AE_L32_IP( x0, X, +4 );
      AE_L32_IP( x1, X, +4 );

      // q0 = g*x[n]; q1 = g*x[n+1]
      // Q17.46 <- Q31*Q15
      u0 = AE_MUL32X16_L1( x0, cf_g );
      u1 = AE_MUL32X16_L1( x1, cf_g );

      q0 = ( u0 );
      q1 = ( u1 );

      // q0 -= a2*st[-2] + a1*st[-1]
      // Q17.46 <- Q17.46 + [ Q30*Q31 + 1 - 16 w/ sym. rounding ]
      AE_MULSF32R_HH( q0, cf_a2a1, st );
      AE_MULSF32R_LL( q0, cf_a2a1, st );

      // y0 = b2*st[-2] + b1*st[-1]
      // Q17.46 <- Q17.46 + [ Q30*Q31 + 1 - 16 w/ sym. rounding ]
      y0 = AE_MULF32R_HH( cf_b2b1, st );
      AE_MULAF32R_LL( y0, cf_b2b1, st );

      // q1 -= a2*st[-1]
      // Q17.46 <- Q17.46 + [ Q30*Q31 + 1 - 16 w/ sym. rounding ]
      AE_MULSF32R_LL( q1, cf_a1a2, st );

      // Intermediate update of the delay line
      // st[-2] = st[-1]; st[-1] = q0
      // Q31 <- Q17.46 + 1 - 16 w/ asym. rounding and saturation
      AE_PKSR32( st, q0, 1 );

      //------------

      // q1 -= a1*q0
      // Q17.46 <- Q17.46 + [ Q30*Q31 + 1 - 16 w/ sym. rounding ]
      AE_MULSF32R_LL( q1, cf_a2a1, st );

      // y1 = b2*st[-1] + b1*q0
      // Q17.46 <- Q17.46 + [ Q30*Q31 + 1 - 16 w/ sym. rounding ]
      y1 = AE_MULF32R_HH( cf_b2b1, st );
      AE_MULAF32R_LL( y1, cf_b2b1, st );

      // y0 += b0*q0
      // Q17.46 <- Q17.46 + [ Q30*Q31 + 1 - 16 w/ sym. rounding ]
      AE_MULAF32R_LL( y0, cf_b0b0, st );

      // Final update of the delay line
      // st[-2] = q0; st[-1] = q1
      // Q31 <- Q17.46 + 1 - 16 w/ asym. rounding and saturation
      AE_PKSR32( st, q1, 1 );

      //------------

      // y1 += b0*q1
      // Q17.46 <- Q17.46 + [ Q30*Q31 + 1 - 16 w/ sym. rounding ]
      AE_MULAF32R_LL( y1, cf_b0b0, st );

      u2 = ( y0 );
      u3 = ( y1 );

      // Format and pack outputs y0, y1
      // Q31 <- Q17.46 + 17 - 32 w/ truncation and saturation
      y_y0y1 = AE_TRUNCA32X2F64S( u2, u3, 17 );

      // Store outputs to the scratch memory
      // y[n] = y0; y[n+1] = y1; Q31
      AE_S32X2_IP( y_y0y1, R, +8 );
    }

    //
    // Save m-th section's state.
    //

    // Q31
    ae_f32x2_storeip( st, S, +8 );

    //
    // From now on biquads are fed with outputs of preceding sections.
    //

    X = (const ae_int32 *)s;

  #ifdef COMPILER_XTENSA
    #pragma flush_memory
  #endif
  }

  //----------------------------------------------------------------------------
  // Pass signal through the last biquad and apply the total gain.
  //

  // Last section's outputs will be stored to the output array.
  R = (ae_int32x2 *)r;

  R_va = AE_ZALIGN64();

  //
  // Load last section's SOS/g coefficients.
  //

  // Q30
  ae_f32x2_loadip( cf_b0b0, C_sos, +8 );
  ae_f32x2_loadip( cf_b2b1, C_sos, +8 );
  ae_f32x2_loadip( cf_a2a1, C_sos, +8 );
  ae_f32x2_loadip( cf_a1a2, C_sos, +8 );

  // Q15
  AE_L16_IP( cf_g, C_g, +2 );

  //
  // Load the total gain shift amount.
  //

  gain = bqriir->gain;

  //
  // Load last section's state.
  //

  // st[-2] st[-1]; Q31
  st = ae_f32x2_loadi( S, 0 );

  //
  // Pass the block of input samples through the section. Two outputs
  // r[n], r[n+1] are computed simultaneously:
  //   q0 = g*x[n]   - a1*st[-1] - a2*st[-2]
  //   q1 = g*x[n+1] - a1*q0     - a2*st[-1]
  //   r[n]   = ( b0*q0 + b1*st[-1] + b2*st[-2] )*2^gain
  //   r[n+1] = ( b0*q1 + b1*q0     + b2*st[-1] )*2^gain
  //   st[-2] = q0; st[-1] = q1
  //

  for ( n=0; n<N; n+=2 )
  {
    // Read x[n], x[n+1]; Q31
    AE_L32_IP( x0, X, +4 );
    AE_L32_IP( x1, X, +4 );

    // q0 = g*x[n]; q1 = g*x[n+1]
    // Q17.46 <- Q31*Q15
    u0 = AE_MUL32X16_L1( x0, cf_g );
    u1 = AE_MUL32X16_L1( x1, cf_g );

    q0 = ( u0 );
    q1 = ( u1 );

    // q0 -= a2*st[-2] + a1*st[-1]
    // Q17.46 <- Q17.46 + [ Q30*Q31 + 1 - 16 w/ sym. rounding ]
    AE_MULSF32R_HH( q0, cf_a2a1, st );
    AE_MULSF32R_LL( q0, cf_a2a1, st );

    // y0 = b2*st[-2] + b1*st[-1]
    // Q17.46 <- Q17.46 + [ Q30*Q31 + 1 - 16 w/ sym. rounding ]
    y0 = AE_MULF32R_HH( cf_b2b1, st );
    AE_MULAF32R_LL( y0, cf_b2b1, st );

    // q1 -= a2*st[-1]
    // Q17.46 <- Q17.46 + [ Q30*Q31 + 1 - 16 w/ sym. rounding ]
    AE_MULSF32R_LL( q1, cf_a1a2, st );

    // Intermediate update of the delay line
    // st[-2] = st[-1]; st[-1] = q0
    // Q31 <- Q17.46 + 1 - 16 w/ asym. rounding and saturation
    AE_PKSR32( st, q0, 1 );

    //------------

    // q1 -= a1*q0
    // Q17.46 <- Q17.46 + [ Q30*Q31 + 1 - 16 w/ sym. rounding ]
    AE_MULSF32R_LL( q1, cf_a2a1, st );

    // y1 = b2*st[-1] + b1*q0
    // Q17.46 <- Q17.46 + [ Q30*Q31 + 1 - 16 w/ sym. rounding ]
    y1 = AE_MULF32R_HH( cf_b2b1, st );
    AE_MULAF32R_LL( y1, cf_b2b1, st );

    // y0 += b0*q0
    // Q17.46 <- Q17.46 + [ Q30*Q31 + 1 - 16 w/ sym. rounding ]
    AE_MULAF32R_LL( y0, cf_b0b0, st );

    // Final update of the delay line
    // st[-2] = q0; st[-1] = q1
    // Q31 <- Q17.46 + 1 - 16 w/ asym. rounding and saturation
    AE_PKSR32( st, q1, 1 );

    //------------

    // y1 += b0*q1
    // Q17.46 <- Q17.46 + [ Q30*Q31 + 1 - 16 w/ sym. rounding ]
    AE_MULAF32R_LL( y1, cf_b0b0, st );

    u2 = ( y0 );
    u3 = ( y1 );

    // Apply the total gain shift and format outputs y0, y1
    // Q(31+gain) <- [ Q17.46 + 17 + gain ] - 32 w/ truncation and saturation
    y_y0y1 = AE_TRUNCA32X2F64S( u2, u3, 17 + gain );

    // r[n] = y0; r[n+1] = y1; Q(31+gain)
    AE_SA32X2_IP( y_y0y1, R_va, R );
  }

  AE_SA64POS_FP( R_va, R );

  //
  // Save last section's state.
  //

  ae_f32x2_storeip( st, S, +8 );

} // bqriir32x32_df2()
