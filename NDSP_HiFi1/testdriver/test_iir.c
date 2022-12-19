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
 * Test procedures for IIR
 */

/* Cross-platform data type definitions. */
#include "NatureDSP_types.h"
#include "NatureDSP_Signal.h"
/* Test data vectors tools and SEQ-file reader. */
#include "vectools.h"
#include "testeng_iir.h"
#include "testeng_iir_old.h"
#include "testeng_iir_lat.h"

static const tIirLatDescr api_latr32x32={(tIirLatFxnAlloc*)latr32x32_alloc,(tIirLatFxnInit*)latr32x32_init,(tIirLatFxnProcess*)latr32x32_process};
static const tIirLatDescr api_latr16x16={(tIirLatFxnAlloc*)latr16x16_alloc,(tIirLatFxnInit*)latr16x16_init,(tIirLatFxnProcess*)latr16x16_process};
static const tIirLatDescr api_latr32x16={(tIirLatFxnAlloc*)latr32x16_alloc,(tIirLatFxnInit*)latr32x16_init,(tIirLatFxnProcess*)latr32x16_process};
static const tIirLatDescr api_latr24x24={(tIirLatFxnAlloc*)latr24x24_alloc,(tIirLatFxnInit*)latr24x24_init,(tIirLatFxnProcess*)latr24x24_process};
static const tIirLatDescr api_latrf    ={(tIirLatFxnAlloc*)latrf_alloc,    (tIirLatFxnInit*)latrf_init,    (tIirLatFxnProcess*)latrf_process    };
static const tTestEngDesc descr_latr32x32     = { FMT_REAL | FMT_FRACT32, FMT_FRACT32 ,TE_ARGNUM_1, TE_ALIGN_NO, te_create_iir_lat, te_destroy_iir_lat, &te_loadFxn_iir_lat, &te_processFxn_iir_lat };
static const tTestEngDesc descr_latr16x16     = { FMT_REAL | FMT_FRACT16, FMT_FRACT16 ,TE_ARGNUM_1, TE_ALIGN_NO, te_create_iir_lat, te_destroy_iir_lat, &te_loadFxn_iir_lat, &te_processFxn_iir_lat };
static const tTestEngDesc descr_latr32x16     = { FMT_REAL | FMT_FRACT16, FMT_FRACT32 ,TE_ARGNUM_1, TE_ALIGN_NO, te_create_iir_lat, te_destroy_iir_lat, &te_loadFxn_iir_lat, &te_processFxn_iir_lat };
static const tTestEngDesc descr_latr24x24     = { FMT_REAL | FMT_FRACT32, FMT_FRACT32 ,TE_ARGNUM_1, TE_ALIGN_NO, te_create_iir_lat, te_destroy_iir_lat, &te_loadFxn_iir_lat, &te_processFxn_iir_lat };
static const tTestEngDesc descr_latrf         = { FMT_REAL | FMT_FLOAT32, FMT_FLOAT32 ,TE_ARGNUM_1, TE_ALIGN_NO, te_create_iir_lat, te_destroy_iir_lat, &te_loadFxn_iir_lat, &te_processFxn_iir_lat };

static size_t bqriir16x16_df1_getScratch(int N, int M) { (void)N; (void)M; return BQRIIR16X16_DF1_SCRATCH_SIZE( N, M ) ; }
static size_t bqriir16x16_df2_getScratch(int N, int M) { (void)N; (void)M; return BQRIIR16X16_DF2_SCRATCH_SIZE( N, M ) ; }
static size_t bq3iir16x16_df1_getScratch(int N, int M) { (void)N; (void)M; return BQ3IIR16X16_DF1_SCRATCH_SIZE( N, M ) ; }
static size_t bq3iir16x16_df2_getScratch(int N, int M) { (void)N; (void)M; return BQ3IIR16X16_DF2_SCRATCH_SIZE( N, M ) ; }
static size_t bqriir24x24_df1_getScratch(int N, int M) { (void)N; (void)M; return BQRIIR24X24_DF1_SCRATCH_SIZE( N, M ) ; }
static size_t bqriir24x24_df2_getScratch(int N, int M) { (void)N; (void)M; return BQRIIR24X24_DF2_SCRATCH_SIZE( N, M ) ; }
static size_t bqriir32x16_df1_getScratch(int N, int M) { (void)N; (void)M; return BQRIIR32X16_DF1_SCRATCH_SIZE( N, M ) ; }
static size_t bqriir32x16_df2_getScratch(int N, int M) { (void)N; (void)M; return BQRIIR32X16_DF2_SCRATCH_SIZE( N, M ) ; }
static size_t bqriir32x32_df1_getScratch(int N, int M) { (void)N; (void)M; return BQRIIR32X32_DF1_SCRATCH_SIZE( N, M ) ; }
static size_t bqriir32x32_df2_getScratch(int N, int M) { (void)N; (void)M; return BQRIIR32X32_DF2_SCRATCH_SIZE( N, M ) ; }
static size_t bq3iir32x16_df1_getScratch(int N, int M) { (void)N; (void)M; return BQ3IIR32X16_DF1_SCRATCH_SIZE( N, M ) ; }
static size_t bq3iir32x16_df2_getScratch(int N, int M) { (void)N; (void)M; return BQ3IIR32X16_DF2_SCRATCH_SIZE( N, M ) ; }
static size_t bq3iir32x32_df1_getScratch(int N, int M) { (void)N; (void)M; return BQ3IIR32X32_DF1_SCRATCH_SIZE( N, M ) ; }
static size_t bq3iir32x32_df2_getScratch(int N, int M) { (void)N; (void)M; return BQ3IIR32X32_DF2_SCRATCH_SIZE( N, M ) ; }

static const tIirOldDescr api_bqriir16x16_df1={(tIirOldFxnAlloc*)bqriir16x16_df1_alloc,(tIirOldFxnInit*)bqriir16x16_df1_init,bqriir16x16_df1_getScratch,(tIirOldFxnProcess*)bqriir16x16_df1};
static const tIirOldDescr api_bqriir16x16_df2={(tIirOldFxnAlloc*)bqriir16x16_df2_alloc,(tIirOldFxnInit*)bqriir16x16_df2_init,bqriir16x16_df2_getScratch,(tIirOldFxnProcess*)bqriir16x16_df2};
static const tIirOldDescr api_bq3iir16x16_df1={(tIirOldFxnAlloc*)bq3iir16x16_df1_alloc,(tIirOldFxnInit*)bq3iir16x16_df1_init,bq3iir16x16_df1_getScratch,(tIirOldFxnProcess*)bq3iir16x16_df1};
static const tIirOldDescr api_bq3iir16x16_df2={(tIirOldFxnAlloc*)bq3iir16x16_df2_alloc,(tIirOldFxnInit*)bq3iir16x16_df2_init,bq3iir16x16_df2_getScratch,(tIirOldFxnProcess*)bq3iir16x16_df2};
static const tIirOldDescr api_bq3iir32x16_df1={(tIirOldFxnAlloc*)bq3iir32x16_df1_alloc,(tIirOldFxnInit*)bq3iir32x16_df1_init,bq3iir32x16_df1_getScratch,(tIirOldFxnProcess*)bq3iir32x16_df1};
static const tIirOldDescr api_bq3iir32x16_df2={(tIirOldFxnAlloc*)bq3iir32x16_df2_alloc,(tIirOldFxnInit*)bq3iir32x16_df2_init,bq3iir32x16_df2_getScratch,(tIirOldFxnProcess*)bq3iir32x16_df2};
static const tIirOldDescr api_bq3iirf_df1    ={(tIirOldFxnAlloc*)bq3iirf_df1_alloc    ,(tIirOldFxnInit*)bq3iirf_df1_init    ,NULL                      ,(tIirOldFxnProcess*)bq3iirf_df1    };
static const tIirOldDescr api_bq3iirf_df2    ={(tIirOldFxnAlloc*)bq3iirf_df2_alloc    ,(tIirOldFxnInit*)bq3iirf_df2_init    ,NULL                      ,(tIirOldFxnProcess*)bq3iirf_df2    };
static const tIirOldDescr api_bqriirf_df1    ={(tIirOldFxnAlloc*)bqriirf_df1_alloc    ,(tIirOldFxnInit*)bqriirf_df1_init    ,NULL                      ,(tIirOldFxnProcess*)bqriirf_df1    };
static const tIirOldDescr api_bqriirf_df2    ={(tIirOldFxnAlloc*)bqriirf_df2_alloc    ,(tIirOldFxnInit*)bqriirf_df2_init    ,NULL                      ,(tIirOldFxnProcess*)bqriirf_df2    };
static const tIirOldDescr api_bqriirf_df2t   ={(tIirOldFxnAlloc*)bqriirf_df2t_alloc   ,(tIirOldFxnInit*)bqriirf_df2t_init   ,NULL                      ,(tIirOldFxnProcess*)bqriirf_df2t   };
static const tIirOldDescr api_bqciirf_df1    ={(tIirOldFxnAlloc*)bqciirf_df1_alloc    ,(tIirOldFxnInit*)bqciirf_df1_init    ,NULL                      ,(tIirOldFxnProcess*)bqciirf_df1    };
static const tIirOldDescr api_bqriir24x24_df1={(tIirOldFxnAlloc*)bqriir24x24_df1_alloc,(tIirOldFxnInit*)bqriir24x24_df1_init,bqriir24x24_df1_getScratch,(tIirOldFxnProcess*)bqriir24x24_df1};
static const tIirOldDescr api_bqriir24x24_df2={(tIirOldFxnAlloc*)bqriir24x24_df2_alloc,(tIirOldFxnInit*)bqriir24x24_df2_init,bqriir24x24_df2_getScratch,(tIirOldFxnProcess*)bqriir24x24_df2};
static const tIirOldDescr api_bqriir32x16_df1={(tIirOldFxnAlloc*)bqriir32x16_df1_alloc,(tIirOldFxnInit*)bqriir32x16_df1_init,bqriir32x16_df1_getScratch,(tIirOldFxnProcess*)bqriir32x16_df1};
static const tIirOldDescr api_bqriir32x16_df2={(tIirOldFxnAlloc*)bqriir32x16_df2_alloc,(tIirOldFxnInit*)bqriir32x16_df2_init,bqriir32x16_df2_getScratch,(tIirOldFxnProcess*)bqriir32x16_df2};
static const tIirOldDescr api_bqriir32x32_df1={(tIirOldFxnAlloc*)bqriir32x32_df1_alloc,(tIirOldFxnInit*)bqriir32x32_df1_init,bqriir32x32_df1_getScratch,(tIirOldFxnProcess*)bqriir32x32_df1};
static const tIirOldDescr api_bqriir32x32_df2={(tIirOldFxnAlloc*)bqriir32x32_df2_alloc,(tIirOldFxnInit*)bqriir32x32_df2_init,bqriir32x32_df2_getScratch,(tIirOldFxnProcess*)bqriir32x32_df2};
static const tIirOldDescr api_bq3iir32x32_df1={(tIirOldFxnAlloc*)bq3iir32x32_df1_alloc,(tIirOldFxnInit*)bq3iir32x32_df1_init,bq3iir32x32_df1_getScratch,(tIirOldFxnProcess*)bq3iir32x32_df1};
static const tIirOldDescr api_bq3iir32x32_df2={(tIirOldFxnAlloc*)bq3iir32x32_df2_alloc,(tIirOldFxnInit*)bq3iir32x32_df2_init,bq3iir32x32_df2_getScratch,(tIirOldFxnProcess*)bq3iir32x32_df2};

static const tTestEngDesc descr_bqriir16x16_df1 = { FMT_REAL | FMT_FRACT16, FMT_FRACT16 | TE_IIR_DF1 ,TE_ARGNUM_1, TE_ALIGN_NO, te_create_iir_old, te_destroy_iir_old, &te_loadFxn_iir_old, &te_processFxn_iir_old };
static const tTestEngDesc descr_bqriir16x16_df2 = { FMT_REAL | FMT_FRACT16, FMT_FRACT16 | TE_IIR_DF2 ,TE_ARGNUM_1, TE_ALIGN_NO, te_create_iir_old, te_destroy_iir_old, &te_loadFxn_iir_old, &te_processFxn_iir_old };
static const tTestEngDesc descr_bq3iir16x16_df1 = { FMT_REAL | FMT_FRACT16, FMT_FRACT16 | TE_IIR_DF1 ,TE_ARGNUM_1, TE_ALIGN_NO, te_create_iir_old, te_destroy_iir_old, &te_loadFxn_iir_old, &te_processFxn_iir_old };
static const tTestEngDesc descr_bq3iir16x16_df2 = { FMT_REAL | FMT_FRACT16, FMT_FRACT16 | TE_IIR_DF2 ,TE_ARGNUM_1, TE_ALIGN_NO, te_create_iir_old, te_destroy_iir_old, &te_loadFxn_iir_old, &te_processFxn_iir_old };
static const tTestEngDesc descr_bq3iir32x16_df1 = { FMT_REAL | FMT_FRACT32, FMT_FRACT16 | TE_IIR_DF1 ,TE_ARGNUM_1, TE_ALIGN_NO, te_create_iir_old, te_destroy_iir_old, &te_loadFxn_iir_old, &te_processFxn_iir_old };
static const tTestEngDesc descr_bq3iir32x16_df2 = { FMT_REAL | FMT_FRACT32, FMT_FRACT16 | TE_IIR_DF2 ,TE_ARGNUM_1, TE_ALIGN_NO, te_create_iir_old, te_destroy_iir_old, &te_loadFxn_iir_old, &te_processFxn_iir_old };
static const tTestEngDesc descr_bq3iirf_df1     = { FMT_REAL | FMT_FLOAT32, FMT_FLOAT32 | TE_IIR_DF1 | TE_IIR_FLOAT,TE_ARGNUM_1, TE_ALIGN_NO, te_create_iir_old, te_destroy_iir_old, &te_loadFxn_iir_old, &te_processFxn_iir_old };
static const tTestEngDesc descr_bq3iirf_df2     = { FMT_REAL | FMT_FLOAT32, FMT_FLOAT32 | TE_IIR_DF2 | TE_IIR_FLOAT,TE_ARGNUM_1, TE_ALIGN_NO, te_create_iir_old, te_destroy_iir_old, &te_loadFxn_iir_old, &te_processFxn_iir_old };
static const tTestEngDesc descr_bqriirf_df1     = { FMT_REAL | FMT_FLOAT32, FMT_FLOAT32 | TE_IIR_DF1 | TE_IIR_FLOAT,TE_ARGNUM_1, TE_ALIGN_NO, te_create_iir_old, te_destroy_iir_old, &te_loadFxn_iir_old, &te_processFxn_iir_old };
static const tTestEngDesc descr_bqriirf_df2     = { FMT_REAL | FMT_FLOAT32, FMT_FLOAT32 | TE_IIR_DF2 | TE_IIR_FLOAT,TE_ARGNUM_1, TE_ALIGN_NO, te_create_iir_old, te_destroy_iir_old, &te_loadFxn_iir_old, &te_processFxn_iir_old };
static const tTestEngDesc descr_bqriirf_df2t    = { FMT_REAL | FMT_FLOAT32, FMT_FLOAT32 | TE_IIR_DF2T| TE_IIR_FLOAT,TE_ARGNUM_1, TE_ALIGN_NO, te_create_iir_old, te_destroy_iir_old, &te_loadFxn_iir_old, &te_processFxn_iir_old };
static const tTestEngDesc descr_bqciirf_df1     = { FMT_CPLX | FMT_FLOAT32, FMT_FLOAT32 | TE_IIR_DF1 | TE_IIR_FLOAT,TE_ARGNUM_1, TE_ALIGN_NO, te_create_iir_old, te_destroy_iir_old, &te_loadFxn_iir_old, &te_processFxn_iir_old };
static const tTestEngDesc descr_bqriir24x24_df1 = { FMT_REAL | FMT_FRACT32, FMT_FRACT32 | TE_IIR_DF1 ,TE_ARGNUM_1, TE_ALIGN_NO, te_create_iir_old, te_destroy_iir_old, &te_loadFxn_iir_old, &te_processFxn_iir_old };
static const tTestEngDesc descr_bqriir24x24_df2 = { FMT_REAL | FMT_FRACT32, FMT_FRACT32 | TE_IIR_DF2 ,TE_ARGNUM_1, TE_ALIGN_NO, te_create_iir_old, te_destroy_iir_old, &te_loadFxn_iir_old, &te_processFxn_iir_old };
static const tTestEngDesc descr_bqriir32x16_df1 = { FMT_REAL | FMT_FRACT32, FMT_FRACT16 | TE_IIR_DF1 ,TE_ARGNUM_1, TE_ALIGN_NO, te_create_iir_old, te_destroy_iir_old, &te_loadFxn_iir_old, &te_processFxn_iir_old };
static const tTestEngDesc descr_bqriir32x16_df2 = { FMT_REAL | FMT_FRACT32, FMT_FRACT16 | TE_IIR_DF2 ,TE_ARGNUM_1, TE_ALIGN_NO, te_create_iir_old, te_destroy_iir_old, &te_loadFxn_iir_old, &te_processFxn_iir_old };
static const tTestEngDesc descr_bqriir32x32_df1 = { FMT_REAL | FMT_FRACT32, FMT_FRACT32 | TE_IIR_DF1 ,TE_ARGNUM_1, TE_ALIGN_NO, te_create_iir_old, te_destroy_iir_old, &te_loadFxn_iir_old, &te_processFxn_iir_old };
static const tTestEngDesc descr_bqriir32x32_df2 = { FMT_REAL | FMT_FRACT32, FMT_FRACT32 | TE_IIR_DF2 ,TE_ARGNUM_1, TE_ALIGN_NO, te_create_iir_old, te_destroy_iir_old, &te_loadFxn_iir_old, &te_processFxn_iir_old };
static const tTestEngDesc descr_bq3iir32x32_df1 = { FMT_REAL | FMT_FRACT32, FMT_FRACT32 | TE_IIR_DF1 ,TE_ARGNUM_1, TE_ALIGN_NO, te_create_iir_old, te_destroy_iir_old, &te_loadFxn_iir_old, &te_processFxn_iir_old };
static const tTestEngDesc descr_bq3iir32x32_df2 = { FMT_REAL | FMT_FRACT32, FMT_FRACT32 | TE_IIR_DF2 ,TE_ARGNUM_1, TE_ALIGN_NO, te_create_iir_old, te_destroy_iir_old, &te_loadFxn_iir_old, &te_processFxn_iir_old };

typedef struct
{
  int                 phaseNum;
  const tTestEngDesc *pIirDescr;
  tTestEngTarget      fxns;
  int                 runAlways;   	/* 1-always */
  int                 testvecFlag;  /* 2-sanity; 1-brief; 0-full */
  const char*         seqFile;
}
tTbl;

static const tTbl tests[] =
{
  { 1, &descr_bqriir32x16_df1    , (tTestEngTarget)&api_bqriir32x16_df1    , 1, 2,"bqriir32x16_df1_lpf1.seq" },
  { 1, &descr_bqriir32x16_df1    , (tTestEngTarget)&api_bqriir32x16_df1    , 1, 1,"bqriir32x16_df1_bpf1.seq" },
  { 1, &descr_bqriir32x16_df1    , (tTestEngTarget)&api_bqriir32x16_df1    , 0, 0,"bqriir32x16_df1_bpf2.seq" },
  { 1, &descr_bqriir32x16_df1    , (tTestEngTarget)&api_bqriir32x16_df1    , 0, 0,"bqriir32x16_df1_bsf1.seq" },
  { 1, &descr_bqriir32x16_df1    , (tTestEngTarget)&api_bqriir32x16_df1    , 0, 0,"bqriir32x16_df1_hpf1.seq" },

  { 1, &descr_bqriir32x16_df2    , (tTestEngTarget)&api_bqriir32x16_df2    , 0, 1,"bqriir32x16_df2_lpf1.seq" },
  { 1, &descr_bqriir32x16_df2    , (tTestEngTarget)&api_bqriir32x16_df2    , 1, 1,"bqriir32x16_df2_bpf1.seq" },
  { 1, &descr_bqriir32x16_df2    , (tTestEngTarget)&api_bqriir32x16_df2    , 0, 0,"bqriir32x16_df2_bpf2.seq" },
  { 1, &descr_bqriir32x16_df2    , (tTestEngTarget)&api_bqriir32x16_df2    , 0, 0,"bqriir32x16_df2_bsf1.seq" },
  { 1, &descr_bqriir32x16_df2    , (tTestEngTarget)&api_bqriir32x16_df2    , 0, 0,"bqriir32x16_df2_hpf1.seq" },

  { 1, &descr_bqriir32x32_df2    , (tTestEngTarget)&api_bqriir32x32_df2    , 1, 2,"bqriir32x32_df2_lpf1.seq" },
  { 1, &descr_bqriir32x32_df2    , (tTestEngTarget)&api_bqriir32x32_df2    , 1, 1,"bqriir32x32_df2_bpf1.seq" },
  { 1, &descr_bqriir32x32_df2    , (tTestEngTarget)&api_bqriir32x32_df2    , 0, 0,"bqriir32x32_df2_bpf2.seq" },
  { 1, &descr_bqriir32x32_df2    , (tTestEngTarget)&api_bqriir32x32_df2    , 0, 0,"bqriir32x32_df2_bsf1.seq" },
  { 1, &descr_bqriir32x32_df2    , (tTestEngTarget)&api_bqriir32x32_df2    , 0, 0,"bqriir32x32_df2_hpf1.seq" },

  { 1, &descr_bqriir24x24_df1    , (tTestEngTarget)&api_bqriir24x24_df1    , 1, 2,"bqriir24x24_df1_lpf1.seq" },
  { 1, &descr_bqriir24x24_df1    , (tTestEngTarget)&api_bqriir24x24_df1    , 1, 1,"bqriir24x24_df1_bpf1.seq" },
  { 1, &descr_bqriir24x24_df1    , (tTestEngTarget)&api_bqriir24x24_df1    , 0, 0,"bqriir24x24_df1_bpf2.seq" },
  { 1, &descr_bqriir24x24_df1    , (tTestEngTarget)&api_bqriir24x24_df1    , 0, 0,"bqriir24x24_df1_bsf1.seq" },
  { 1, &descr_bqriir24x24_df1    , (tTestEngTarget)&api_bqriir24x24_df1    , 0, 0,"bqriir24x24_df1_hpf1.seq" },

  { 1, &descr_bqriir24x24_df2    , (tTestEngTarget)&api_bqriir24x24_df2    , 1, 2,"bqriir24x24_df2_lpf1.seq" },
  { 1, &descr_bqriir24x24_df2    , (tTestEngTarget)&api_bqriir24x24_df2    , 1, 1,"bqriir24x24_df2_bpf1.seq" },
  { 1, &descr_bqriir24x24_df2    , (tTestEngTarget)&api_bqriir24x24_df2    , 0, 0,"bqriir24x24_df2_bpf2.seq" },
  { 1, &descr_bqriir24x24_df2    , (tTestEngTarget)&api_bqriir24x24_df2    , 0, 0,"bqriir24x24_df2_bsf1.seq" },
  { 1, &descr_bqriir24x24_df2    , (tTestEngTarget)&api_bqriir24x24_df2    , 0, 0,"bqriir24x24_df2_hpf1.seq" },

  { 1, &descr_latr24x24          , (tTestEngTarget)&api_latr24x24          , 1, 2, "latr24x24_lpf1.seq"       },
  { 1, &descr_latr24x24          , (tTestEngTarget)&api_latr24x24          , 1, 1, "latr24x24_lpf5.seq"       },
  { 1, &descr_latr24x24          , (tTestEngTarget)&api_latr24x24          , 1, 1, "latr24x24_lpf9.seq"       },
  { 1, &descr_latr24x24          , (tTestEngTarget)&api_latr24x24          , 0, 0, "latr24x24_lpf2.seq"       },
  { 1, &descr_latr24x24          , (tTestEngTarget)&api_latr24x24          , 0, 0, "latr24x24_lpf3.seq"       },
  { 1, &descr_latr24x24          , (tTestEngTarget)&api_latr24x24          , 0, 0, "latr24x24_lpf4.seq"       },
  { 1, &descr_latr24x24          , (tTestEngTarget)&api_latr24x24          , 0, 0, "latr24x24_lpf6.seq"       },
  { 1, &descr_latr24x24          , (tTestEngTarget)&api_latr24x24          , 0, 0, "latr24x24_lpf7.seq"       },
  { 1, &descr_latr24x24          , (tTestEngTarget)&api_latr24x24          , 0, 0, "latr24x24_lpf8.seq"       },

  { 1, &descr_latr32x16          , (tTestEngTarget)&api_latr32x16          , 1, 2, "latr32x16_lpf1.seq"       },
  { 1, &descr_latr32x16          , (tTestEngTarget)&api_latr32x16          , 1, 1, "latr32x16_lpf5.seq"       },
  { 1, &descr_latr32x16          , (tTestEngTarget)&api_latr32x16          , 1, 1, "latr32x16_lpf9.seq"       },
  { 1, &descr_latr32x16          , (tTestEngTarget)&api_latr32x16          , 0, 0, "latr32x16_lpf2.seq"       },
  { 1, &descr_latr32x16          , (tTestEngTarget)&api_latr32x16          , 0, 0, "latr32x16_lpf3.seq"       },
  { 1, &descr_latr32x16          , (tTestEngTarget)&api_latr32x16          , 0, 0, "latr32x16_lpf4.seq"       },
  { 1, &descr_latr32x16          , (tTestEngTarget)&api_latr32x16          , 0, 0, "latr32x16_lpf6.seq"       },
  { 1, &descr_latr32x16          , (tTestEngTarget)&api_latr32x16          , 0, 0, "latr32x16_lpf8.seq"       },

  /*
   * Stage 2
   */
  { 2, &descr_bqriirf_df1        , (tTestEngTarget)&api_bqriirf_df1        , 1, 2, "bqriirf_df1_lpf1.seq" },
  { 2, &descr_bqriirf_df1        , (tTestEngTarget)&api_bqriirf_df1        , 0, 1, "bqriirf_df1_bpf1.seq" },
  { 2, &descr_bqriirf_df1        , (tTestEngTarget)&api_bqriirf_df1        , 0, 0, "bqriirf_df1_bpf2.seq" },
  { 2, &descr_bqriirf_df1        , (tTestEngTarget)&api_bqriirf_df1        , 0, 0, "bqriirf_df1_bsf1.seq" },
  { 2, &descr_bqriirf_df1        , (tTestEngTarget)&api_bqriirf_df1        , 0, 0, "bqriirf_df1_hpf1.seq" },

  { 2, &descr_bqriirf_df2        , (tTestEngTarget)&api_bqriirf_df2        , 1, 2, "bqriirf_df2_lpf1.seq" },
  { 2, &descr_bqriirf_df2        , (tTestEngTarget)&api_bqriirf_df2        , 0, 1, "bqriirf_df2_bpf1.seq" },
  { 2, &descr_bqriirf_df2        , (tTestEngTarget)&api_bqriirf_df2        , 0, 0, "bqriirf_df2_bpf2.seq" },
  { 2, &descr_bqriirf_df2        , (tTestEngTarget)&api_bqriirf_df2        , 0, 0, "bqriirf_df2_bsf1.seq" },
  { 2, &descr_bqriirf_df2        , (tTestEngTarget)&api_bqriirf_df2        , 0, 0, "bqriirf_df2_hpf1.seq" },

  { 2, &descr_bqriirf_df2t       , (tTestEngTarget)&api_bqriirf_df2t       , 0, 2, "bqriirf_df2t_lpf1.seq" },
  { 2, &descr_bqriirf_df2t       , (tTestEngTarget)&api_bqriirf_df2t       , 0, 2, "bqriirf_df2t_bpf1.seq" },
  { 2, &descr_bqriirf_df2t       , (tTestEngTarget)&api_bqriirf_df2t       , 0, 0, "bqriirf_df2t_bpf2.seq" },
  { 2, &descr_bqriirf_df2t       , (tTestEngTarget)&api_bqriirf_df2t       , 0, 0, "bqriirf_df2t_bsf1.seq" },
  { 2, &descr_bqriirf_df2t       , (tTestEngTarget)&api_bqriirf_df2t       , 0, 0, "bqriirf_df2t_hpf1.seq" },

  { 2, &descr_bqciirf_df1        , (tTestEngTarget)&api_bqciirf_df1        , 1, 2, "bqciirf_df1_lpf1.seq" },
  { 2, &descr_bqciirf_df1        , (tTestEngTarget)&api_bqciirf_df1        , 1, 2, "bqciirf_df1_bpf1.seq" },
  { 2, &descr_bqciirf_df1        , (tTestEngTarget)&api_bqciirf_df1        , 0, 0, "bqciirf_df1_bpf2.seq" },
  { 2, &descr_bqciirf_df1        , (tTestEngTarget)&api_bqciirf_df1        , 0, 0, "bqciirf_df1_bsf1.seq" },
  { 2, &descr_bqciirf_df1        , (tTestEngTarget)&api_bqciirf_df1        , 0, 0, "bqciirf_df1_hpf1.seq" },

  { 2, &descr_latrf              , (tTestEngTarget)&api_latrf              , 1, 2, "latrf_lpf1.seq"     },
  { 2, &descr_latrf              , (tTestEngTarget)&api_latrf              , 1, 1, "latrf_lpf5.seq"     },
  { 2, &descr_latrf              , (tTestEngTarget)&api_latrf              , 1, 1, "latrf_lpf9.seq"     },
  { 2, &descr_latrf              , (tTestEngTarget)&api_latrf              , 0, 0, "latrf_lpf2.seq"     },
  { 2, &descr_latrf              , (tTestEngTarget)&api_latrf              , 0, 0, "latrf_lpf3.seq"     },
  { 2, &descr_latrf              , (tTestEngTarget)&api_latrf              , 0, 0, "latrf_lpf4.seq"     },
  { 2, &descr_latrf              , (tTestEngTarget)&api_latrf              , 0, 0, "latrf_lpf6.seq"     },
  { 2, &descr_latrf              , (tTestEngTarget)&api_latrf              , 0, 0, "latrf_lpf7.seq"     },
  { 2, &descr_latrf              , (tTestEngTarget)&api_latrf              , 0, 0, "latrf_lpf8.seq"     },
  /*
   * Stage 3
   */
  { 3, &descr_bqriir16x16_df1    , (tTestEngTarget)&api_bqriir16x16_df1    , 1, 2, "bqriir16x16_df1_lpf1.seq" },
  { 3, &descr_bqriir16x16_df1    , (tTestEngTarget)&api_bqriir16x16_df1    , 1, 2, "bqriir16x16_df1_bpf1.seq" },
  { 3, &descr_bqriir16x16_df1    , (tTestEngTarget)&api_bqriir16x16_df1    , 0, 0, "bqriir16x16_df1_bpf2.seq" },
  { 3, &descr_bqriir16x16_df1    , (tTestEngTarget)&api_bqriir16x16_df1    , 0, 0, "bqriir16x16_df1_bsf1.seq" },
  { 3, &descr_bqriir16x16_df1    , (tTestEngTarget)&api_bqriir16x16_df1    , 0, 0, "bqriir16x16_df1_hpf1.seq" },

  { 3, &descr_bqriir16x16_df2    , (tTestEngTarget)&api_bqriir16x16_df2    , 1, 2, "bqriir16x16_df2_lpf1.seq" },
  { 3, &descr_bqriir16x16_df2    , (tTestEngTarget)&api_bqriir16x16_df2    , 1, 2, "bqriir16x16_df2_bpf1.seq" },
  { 3, &descr_bqriir16x16_df2    , (tTestEngTarget)&api_bqriir16x16_df2    , 0, 0, "bqriir16x16_df2_bpf2.seq" },
  { 3, &descr_bqriir16x16_df2    , (tTestEngTarget)&api_bqriir16x16_df2    , 0, 0, "bqriir16x16_df2_bsf1.seq" },
  { 3, &descr_bqriir16x16_df2    , (tTestEngTarget)&api_bqriir16x16_df2    , 0, 0, "bqriir16x16_df2_hpf1.seq" },

  { 3, &descr_bqriir32x32_df1    , (tTestEngTarget)&api_bqriir32x32_df1    , 1, 2, "bqriir32x32_df1_lpf1.seq" },
  { 3, &descr_bqriir32x32_df1    , (tTestEngTarget)&api_bqriir32x32_df1    , 1, 2, "bqriir32x32_df1_bpf1.seq" },
  { 3, &descr_bqriir32x32_df1    , (tTestEngTarget)&api_bqriir32x32_df1    , 0, 0, "bqriir32x32_df1_bpf2.seq" },
  { 3, &descr_bqriir32x32_df1    , (tTestEngTarget)&api_bqriir32x32_df1    , 0, 0, "bqriir32x32_df1_bsf1.seq" },
  { 3, &descr_bqriir32x32_df1    , (tTestEngTarget)&api_bqriir32x32_df1    , 0, 0, "bqriir32x32_df1_hpf1.seq" },

  { 3, &descr_bq3iir16x16_df1    , (tTestEngTarget)&api_bq3iir16x16_df1    , 1, 2, "bq3iir16x16_df1_lpf1.seq" },
  { 3, &descr_bq3iir16x16_df1    , (tTestEngTarget)&api_bq3iir16x16_df1    , 1, 2, "bq3iir16x16_df1_bpf1.seq" },
  { 3, &descr_bq3iir16x16_df1    , (tTestEngTarget)&api_bq3iir16x16_df1    , 0, 0, "bq3iir16x16_df1_bpf2.seq" },
  { 3, &descr_bq3iir16x16_df1    , (tTestEngTarget)&api_bq3iir16x16_df1    , 0, 0, "bq3iir16x16_df1_bsf1.seq" },
  { 3, &descr_bq3iir16x16_df1    , (tTestEngTarget)&api_bq3iir16x16_df1    , 0, 0, "bq3iir16x16_df1_hpf1.seq" },

  { 3, &descr_bq3iir16x16_df2    , (tTestEngTarget)&api_bq3iir16x16_df2    , 1, 2, "bq3iir16x16_df2_lpf1.seq" },
  { 3, &descr_bq3iir16x16_df2    , (tTestEngTarget)&api_bq3iir16x16_df2    , 1, 2, "bq3iir16x16_df2_bpf1.seq" },
  { 3, &descr_bq3iir16x16_df2    , (tTestEngTarget)&api_bq3iir16x16_df2    , 0, 0, "bq3iir16x16_df2_bpf2.seq" },
  { 3, &descr_bq3iir16x16_df2    , (tTestEngTarget)&api_bq3iir16x16_df2    , 0, 0, "bq3iir16x16_df2_bsf1.seq" },
  { 3, &descr_bq3iir16x16_df2    , (tTestEngTarget)&api_bq3iir16x16_df2    , 0, 0, "bq3iir16x16_df2_hpf1.seq" },

  { 3, &descr_bq3iir32x16_df1    , (tTestEngTarget)&api_bq3iir32x16_df1    , 1, 2, "bq3iir32x16_df1_lpf1.seq" },
  { 3, &descr_bq3iir32x16_df1    , (tTestEngTarget)&api_bq3iir32x16_df1    , 1, 2, "bq3iir32x16_df1_bpf1.seq" },
  { 3, &descr_bq3iir32x16_df1    , (tTestEngTarget)&api_bq3iir32x16_df1    , 0, 0, "bq3iir32x16_df1_bpf2.seq" },
  { 3, &descr_bq3iir32x16_df1    , (tTestEngTarget)&api_bq3iir32x16_df1    , 0, 0, "bq3iir32x16_df1_bsf1.seq" },
  { 3, &descr_bq3iir32x16_df1    , (tTestEngTarget)&api_bq3iir32x16_df1    , 0, 0, "bq3iir32x16_df1_hpf1.seq" },

  { 3, &descr_bq3iir32x16_df2    , (tTestEngTarget)&api_bq3iir32x16_df2    , 1, 2, "bq3iir32x16_df2_lpf1.seq" },
  { 3, &descr_bq3iir32x16_df2    , (tTestEngTarget)&api_bq3iir32x16_df2    , 1, 2, "bq3iir32x16_df2_bpf1.seq" },
  { 3, &descr_bq3iir32x16_df2    , (tTestEngTarget)&api_bq3iir32x16_df2    , 0, 0, "bq3iir32x16_df2_bpf2.seq" },
  { 3, &descr_bq3iir32x16_df2    , (tTestEngTarget)&api_bq3iir32x16_df2    , 0, 0, "bq3iir32x16_df2_bsf1.seq" },
  { 3, &descr_bq3iir32x16_df2    , (tTestEngTarget)&api_bq3iir32x16_df2    , 0, 0, "bq3iir32x16_df2_hpf1.seq" },

  { 3, &descr_bq3iir32x32_df1    , (tTestEngTarget)&api_bq3iir32x32_df1    , 1, 2, "bq3iir32x32_df1_lpf1.seq" },
  { 3, &descr_bq3iir32x32_df1    , (tTestEngTarget)&api_bq3iir32x32_df1    , 1, 2, "bq3iir32x32_df1_bpf1.seq" },
  { 3, &descr_bq3iir32x32_df1    , (tTestEngTarget)&api_bq3iir32x32_df1    , 0, 0, "bq3iir32x32_df1_bpf2.seq" },
  { 3, &descr_bq3iir32x32_df1    , (tTestEngTarget)&api_bq3iir32x32_df1    , 0, 0, "bq3iir32x32_df1_bsf1.seq" },
  { 3, &descr_bq3iir32x32_df1    , (tTestEngTarget)&api_bq3iir32x32_df1    , 0, 0, "bq3iir32x32_df1_hpf1.seq" },

  { 3, &descr_bq3iir32x32_df2    , (tTestEngTarget)&api_bq3iir32x32_df2    , 1, 2, "bq3iir32x32_df2_lpf1.seq" },
  { 3, &descr_bq3iir32x32_df2    , (tTestEngTarget)&api_bq3iir32x32_df2    , 1, 2, "bq3iir32x32_df2_bpf1.seq" },
  { 3, &descr_bq3iir32x32_df2    , (tTestEngTarget)&api_bq3iir32x32_df2    , 0, 0, "bq3iir32x32_df2_bpf2.seq" },
  { 3, &descr_bq3iir32x32_df2    , (tTestEngTarget)&api_bq3iir32x32_df2    , 0, 0, "bq3iir32x32_df2_bsf1.seq" },
  { 3, &descr_bq3iir32x32_df2    , (tTestEngTarget)&api_bq3iir32x32_df2    , 0, 0, "bq3iir32x32_df2_hpf1.seq" },

  { 3, &descr_bq3iirf_df1        , (tTestEngTarget)&api_bq3iirf_df1        , 1, 2, "bq3iirf_df1_lpf1.seq" },
  { 3, &descr_bq3iirf_df1        , (tTestEngTarget)&api_bq3iirf_df1        , 1, 2, "bq3iirf_df1_bpf1.seq" },
  { 3, &descr_bq3iirf_df1        , (tTestEngTarget)&api_bq3iirf_df1        , 0, 0, "bq3iirf_df1_bpf2.seq" },
  { 3, &descr_bq3iirf_df1        , (tTestEngTarget)&api_bq3iirf_df1        , 0, 0, "bq3iirf_df1_bsf1.seq" },
  { 3, &descr_bq3iirf_df1        , (tTestEngTarget)&api_bq3iirf_df1        , 0, 0, "bq3iirf_df1_hpf1.seq" },

  { 3, &descr_bq3iirf_df2        , (tTestEngTarget)&api_bq3iirf_df2        , 1, 2, "bq3iirf_df2_lpf1.seq" },
  { 3, &descr_bq3iirf_df2        , (tTestEngTarget)&api_bq3iirf_df2        , 1, 2, "bq3iirf_df2_bpf1.seq" },
  { 3, &descr_bq3iirf_df2        , (tTestEngTarget)&api_bq3iirf_df2        , 0, 0, "bq3iirf_df2_bpf2.seq" },
  { 3, &descr_bq3iirf_df2        , (tTestEngTarget)&api_bq3iirf_df2        , 0, 0, "bq3iirf_df2_bsf1.seq" },
  { 3, &descr_bq3iirf_df2        , (tTestEngTarget)&api_bq3iirf_df2        , 0, 0, "bq3iirf_df2_hpf1.seq" },

  { 3, &descr_latr16x16          , (tTestEngTarget)&api_latr16x16          , 1, 2, "latr16x16_lpf1.seq"    },
  { 3, &descr_latr16x16          , (tTestEngTarget)&api_latr16x16          , 1, 1, "latr16x16_lpf5.seq"    },
  { 3, &descr_latr16x16          , (tTestEngTarget)&api_latr16x16          , 1, 1, "latr16x16_lpf9.seq"    },
  { 3, &descr_latr16x16          , (tTestEngTarget)&api_latr16x16          , 0, 0, "latr16x16_lpf2.seq"    },
  { 3, &descr_latr16x16          , (tTestEngTarget)&api_latr16x16          , 0, 0, "latr16x16_lpf3.seq"    },
  { 3, &descr_latr16x16          , (tTestEngTarget)&api_latr16x16          , 0, 0, "latr16x16_lpf4.seq"    },
  { 3, &descr_latr16x16          , (tTestEngTarget)&api_latr16x16          , 0, 0, "latr16x16_lpf6.seq"    },
  { 3, &descr_latr16x16          , (tTestEngTarget)&api_latr16x16          , 0, 0, "latr16x16_lpf7.seq"    },
  { 3, &descr_latr16x16          , (tTestEngTarget)&api_latr16x16          , 0, 0, "latr16x16_lpf8.seq"    },

  { 3, &descr_latr32x32          , (tTestEngTarget)&api_latr32x32          , 1, 2, "latr32x32_lpf1.seq"    },
  { 3, &descr_latr32x32          , (tTestEngTarget)&api_latr32x32          , 1, 1, "latr32x32_lpf5.seq"    },
  { 3, &descr_latr32x32          , (tTestEngTarget)&api_latr32x32          , 1, 1, "latr32x32_lpf9.seq"    },
  { 3, &descr_latr32x32          , (tTestEngTarget)&api_latr32x32          , 0, 0, "latr32x32_lpf2.seq"    },
  { 3, &descr_latr32x32          , (tTestEngTarget)&api_latr32x32          , 0, 0, "latr32x32_lpf3.seq"    },
  { 3, &descr_latr32x32          , (tTestEngTarget)&api_latr32x32          , 0, 0, "latr32x32_lpf4.seq"    },
  { 3, &descr_latr32x32          , (tTestEngTarget)&api_latr32x32          , 0, 0, "latr32x32_lpf6.seq"    },
  { 3, &descr_latr32x32          , (tTestEngTarget)&api_latr32x32          , 0, 0, "latr32x32_lpf7.seq"    },
  { 3, &descr_latr32x32          , (tTestEngTarget)&api_latr32x32          , 0, 0, "latr32x32_lpf8.seq"    },

};

/* Perform all tests for IIR API functions. */
int main_iir( int phaseNum, int isFull, int isVerbose, int breakOnError )
{
    int n;
    int res = 1;
    printf( "\nIIR filters:\n" );
    for (n=0; n<(int)(sizeof(tests)/sizeof(tests[0])); n++)
    {
        if ( (phaseNum == 0 || phaseNum == tests[n].phaseNum) && tests[n].runAlways )
        {
            res = TestEngRun(tests[n].fxns, tests[n].pIirDescr, tests[n].seqFile, isFull, isVerbose, breakOnError);
            if (res == 0 && breakOnError) break;
        }
        else if ( isFull <= tests[n].testvecFlag)
        {
  		  res = TestEngRun(tests[n].fxns, tests[n].pIirDescr, tests[n].seqFile, isFull, isVerbose, breakOnError);
  		  if (res == 0 && breakOnError) break;
        }
    }
    return (res);
} /* main_iir() */
