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
 * Test engine API
 */

#ifndef __TESTENG_H
#define __TESTENG_H

/* Cross-platform data type definitions. */
#include "NatureDSP_types.h"
/* Test data vectors tools and SEQ-file reader. */
#include "vectools.h"

#ifdef __cplusplus
extern "C" {
#endif

/*------------------------------------------------------------------------------*
 *                Forward declarations of Test Engine structures                *
 *------------------------------------------------------------------------------*/

/* Test definition: target function + test attributes + test means. */
typedef struct tagTestEngDesc tTestEngDesc;
/* Test engine operation context. */
typedef struct tagTestEngContext tTestEngContext;

/*------------------------------------------------------------------------------*
 *  "Virtual" functions used by the Test Engine throughout the test procedure   *
 *------------------------------------------------------------------------------*/

/* Pointer to a function(s) under test. */
typedef void (*tTestEngTarget)();
/* Create a target algorithm instance and set tTestEngContext::target fields.
 * Return zero if failed. */
typedef int tTestEngTargetCreateFxn( tTestEngContext * context );
/* Destroy the target algorithm instance and free memory block(s) allocated
 * for the target object. Return zero whenever any violation of memory area
 * bounds is detected. */
typedef int tTestEngTargetDestroyFxn( tTestEngContext * context );

/* Allocate in/out vectors for the next test case, and load the data set
 * from the SEQ-file. Return zero if failed. */
typedef int  tTestEngLoadFxn( tTestEngContext * context );
/* Apply the target function to the test case data set. */
typedef void tTestEngProcessFxn( tTestEngContext * context );


/* function pointer */
typedef  void tFunPtr_t() ;

/*------------------------------------------------------------------------------*
 *                        Test engine operation context                         *
 *------------------------------------------------------------------------------*/

struct tagTestEngContext
{
  const tTestEngDesc * desc;        /* Test definition                            */
  char                 seqDir[192]; /* Path to SEQ-file location                  */
  tSeqFile             seqFile;     /* Handle of the SEQ-file with test data      */
  int                  isVerbose;   /* Print additional info on test progress     */

  struct {                          /* Test target spec                           */
    tTestEngTarget     fut;         /* Function Under Test                        */
    void *             handle;      /* User-defined handle (optional)             */
  } target;                                                                       

  struct {                          /* Test case arguments:                       */
    int caseNum;                    /*  - test case number                        */
    int caseType;                   /*  - semantic type (see TBD!!!!)             */
    int M, N, P, L;                 /*  - data dimensions                         */
  } args;                                                                         

  struct {                          /* Test case data set:                        */
    tVec X, Y;                      /*  - input vectors/scalars                   */
    tVec Z, W;                      /*  - output vectors/scalars                  */
    tVec Zlo, Zhi, Wlo, Whi;        /*  - output range boundaries                 */
  } dataSet;

  struct {                          /* Error handling verfication support:        */
    int isEnabled;                  /*  - verification enable flag                */
    int isPassed;                   /*  - verification result for a test case     *
                                     *    (this may involve multiple invocations  *
                                     *    of a scalar FUT, or a single call of a  *
                                     *    vector FUT).                            */
    struct {                        /*  - reference error states:                 */
      tVec edom;                    /*    - errno=EDOM assertion indices          */
      tVec erange;                  /*    - errno=ERANGE asseertion indices       */
      tVec fe_inv;                  /*    - FE_INVALID exception raise indices    */
      tVec fe_divz;                 /*    - FE_DIVBYZERO exception raise indices  */
      tVec fe_ovfl;                 /*    - FE_OVERFLOW exception raise indices   */
    } refStates;                    /*    All indices refer to in/out data vector *
                                     *    positions.                              */
    int entranceExcepts;            /*  - exception flags set just beofore a FUT  *
                                     *    is called (for preserved state check)   */
    int exceptEnable;               /*  - exception enable controls (FUT must not *
                                     *    distort the exc. enable controls)       */
  } errh;
};

/*------------------------------------------------------------------------------*
 *      User-supplied test description and test engine executive function       *
 *------------------------------------------------------------------------------*/

/* Number of dimensional arguments of a test case (M,N,etc.) */
#define TE_ARGNUM_1    1
#define TE_ARGNUM_2    2
#define TE_ARGNUM_3    3
#define TE_ARGNUM_4    4

/* Data vectors alignment options */
#define TE_ALIGN_YES   1
#define TE_ALIGN_NO    0

/* Extra parameter for tests with error handling verification: whether or not
 * the Test Engine verifies that a FUT does not clear any FP exception flags. */
#define TE_ERRH_EXTENDED_TEST_ENABLE   1
#define TE_ERRH_EXTENDED_TEST_DISABLE  0
/* Do not check particular floating-point exception flags. */
#define TE_ERRH_IGNORE_FE_INEXACT      2
#define TE_ERRH_IGNORE_FE_UNDERFLOW    4

/* Test definition: target function + test attributes + test means. All-zero entry marks the
 * end of descriptions table. */
struct tagTestEngDesc
{
  int                        fmt;                /* Data format, a combination of FMT_x symbols (see vectool.h)   */
  uint32_t                   extraParam;         /* extra constant parameter of function, usually zero            */
  int                        argNum;             /* The number of dimensional arguments of a test case (M,N,etc.) */
  int                        isAligned;          /* Use aligned data buffers.                                     */
  tTestEngTargetCreateFxn  * targetCreateFxn;    /* Target algorithm instance creation (optional)                 */
  tTestEngTargetDestroyFxn * targetDestroyFxn;   /* Target algorithm instance deletion (optional)                 */
  tTestEngLoadFxn          * testCaseLoadFxn;    /* Prepares the data set for a test case.                        */
  tTestEngProcessFxn       * testCaseProcessFxn; /* Applies the target function to the data set.                  */
};

/* Test executive function. Performs the specified test on a brief or full version
 * of the designated SEQ-file. Return the test result (non-zero if passed). */
int TestEngRun( tTestEngTarget  targetFxn, const tTestEngDesc * desc,
                const char * seqName, int isFull, int isVerbose, int breakOnError );

/* Test executive function. Look through the test definition table, find an approptiate test description, and
 * perform the specified test on a brief or full version of the designated SEQ-file. Return the test result
 * (non-zero if passed).
 *
 *  first argument is a pointer to the table of type
 *  struct 
 *  {
 *  tTestEngTarget *    funcList[maxFuncNum];
 *  tTestEngDesc        testDesc;
 *  }
 *  the list of functions is ended with NULL and last entry in the arry of those elements 
 *  is a NULL as well
 */
int te_Exec( const void* pTestDefTbl, size_t tblSz, int maxFuncNum,
              tTestEngTarget  targetFxn, const char * seqName,
              int isFull, int isVerbose, int breakOnError );

/*------------------------------------------------------------------------------*
 *            Repertory of common test case load/process functions              *
 *------------------------------------------------------------------------------*/

/* Allocate vectors and load the data set: */
tTestEngLoadFxn    te_loadFxn_vXsYvZ;        /* vector X (in), scalar Y (in), vector Z (out) */
tTestEngLoadFxn    te_loadFxn_vXsY32vZ;      /* vector X (in), scalar int32 Y (in), vector Z (out) */
tTestEngLoadFxn    te_loadFxn_vX32sY32vZ;    /* vector int32 X (in), scalar int32 Y (in), vector Z (out) */
tTestEngLoadFxn    te_loadFxn_vXsY32vZ32;    /* vector X (in), scalar int32 Y (in), vector int32 Z (out) */
tTestEngLoadFxn    te_loadFxn_vXvYvZ;        /* vector X (in), vector Y (in), vector Z (out) */
tTestEngLoadFxn    te_loadFxn_vXvYvZf;       /* vector X (in), vector Y (in), vector Z (out) floating point */
tTestEngLoadFxn    te_loadFxn_vXvYvZd;       /* vector X (in), vector Y (in), vector Z (out) double precision */
tTestEngLoadFxn    te_loadFxn_vXvYsZ;        /* vector X (in), vector Y (in), scalar Z (out) */
tTestEngLoadFxn    te_loadFxn_vXvZ;          /* vector X (in), vector Z (out) */
tTestEngLoadFxn    te_loadFxn_vXcvZ;         /* vector complex X (in), vector Z (out) */
tTestEngLoadFxn    te_loadFxn_vXvZf;         /* vector X(in), vector Z(out) floating point */
tTestEngLoadFxn    te_loadFxn_vXvZd;         /* vector X(in), vector Z(out) double precision */
tTestEngLoadFxn    te_loadFxn_vXsZ;          /* vector X (in), scalar Z (out) */
tTestEngLoadFxn    te_loadFxn_vXsZ16;        /* vector X (in), scalar int16 Z (out) */
tTestEngLoadFxn    te_loadFxn_vXsZ32;        /* vector X (in), scalar int32 Z (out) */
tTestEngLoadFxn    te_loadFxn_vX32sZ;        /* vector int32 X (in), scalar Z (out) */
tTestEngLoadFxn    te_loadFxn_vXsY32sZ64;    /* vector X (in), scalar int32 Y (in), scalar int64 Z (out) */
tTestEngLoadFxn    te_loadFxn_vXsY32sZ64_24;    /* vector X (in), scalar int32 Y (in), scalar int64 Z (out) */
tTestEngLoadFxn    te_loadFxn_vXvYsZ64;      /* vector X (in), vector Y (in), scalar int64 Z (out) */
tTestEngLoadFxn    te_loadFxn_vXvYsZ64_24;      /* vector X (in), vector Y (in), scalar int64 Z (out) */
tTestEngLoadFxn    te_loadFxn_vXvYsZ32;      /* vector X (in), vector Y (in), scalar int32 Z (out) */
tTestEngLoadFxn    te_loadFxn_vXvY16sZ64;      /* vector X (in), vector int16 Y (in), scalar int64 Z (out) */

/* Apply the target function to the test case data set, non-streaming vector variants. */
tTestEngProcessFxn te_processFxn_vXsYvZ;     /* vector X (in), scalar Y (in), vector Z (out) */
tTestEngProcessFxn te_processFxn_vZvXsY;     /* vector Z (out),vector X (in), scalar Y (in)  */
tTestEngProcessFxn te_processFxn_vZvXsY32;   /* vector Z (out),vector X (in), scalar int32 Y (in)  */
tTestEngProcessFxn te_processFxn_vXvYvZ;     /* vector X (in), vector Y (in), vector Z (out) */
tTestEngProcessFxn te_processFxn_vZvXvY;     /* vector Z (out), vector X (in), vector Y (in) */
tTestEngProcessFxn te_processFxn_vYvXvZ;     /* vector Y (in), vector X (in), vector Z (out) */
tTestEngProcessFxn te_processFxn_vZvYvX;     /* vector Z (out), vector Y (in), vector X (in)  */
tTestEngProcessFxn te_processFxn_vXvYsZ;     /* vector X (in), vector Y (in), scalar Z (out) */
tTestEngProcessFxn te_processFxn_vXvZ;       /* vector X (in), vector Z (out) */
tTestEngProcessFxn te_processFxn_vZvX;       /* vector X (in), vector Z (out) */
tTestEngProcessFxn te_processFxn_vXsZ;       /* vector X (in), scalar Z (out) */
tTestEngProcessFxn te_processFxn_vXcsZ;      /* vector complex X (in), scalar Z (out) */
tTestEngProcessFxn te_processFxn_vXsZ32;     /* vector X (in), scalar int32 Z (out) */
tTestEngProcessFxn te_processFxn_vXsY32sZ64; /* vector X (in), scalar Y (in), scalar int64 Z (out) */
tTestEngProcessFxn te_processFxn_vXvZvW;     /* vector X (in), vector Z (out), vector W (out) */
tTestEngProcessFxn te_processFxn_sZ64vXvY;   /* scalar int64 Z (out), vector X (in), vector Y (in) */
tTestEngProcessFxn te_processFxn_sZ32vXvY;   /* scalar int32 Z (out), vector X (in), vector Y (in) */

/* Apply the target function to the test case data set, streaming vector variants. */
tTestEngProcessFxn te_processFxn_s_vXvYvZ; /* vector X (in), vector Y (in), vector Z (out) */
tTestEngProcessFxn te_processFxn_s_vXvZ;   /* vector X (in), vector Z (out) */

/* helper functions for load/process callbacks */
int freeVectors( tTestEngContext * context );

/*------------------------------------------------------------------------------*
 *             Error Handling (ERRH) verification support functions             *
 *------------------------------------------------------------------------------*/

/* Initialize the ERRH support; is called internally by the Test Engine before 
 * it reads a SEQ-file for the first time. */
void te_errh_init( tTestEngContext * context );
/* Read the SEQ-file to load reference error states for a test case.
 * Return zero if failed. */
int te_errh_loadRef( tTestEngContext * context );
/* Free reference error state vectors. */
void te_errh_freeRef( tTestEngContext * context );
/* Prepare to call the function under test: reset the errno and clear
 * FP exception flags. */
void te_errh_resetStates( tTestEngContext * context );
/* Sample errno and FP exception flags and check them against the reference
 * error states. For a scalar function under test the second argument must hold
 * the current index value for in/out test data vectors. For a vector function,
 * assign it a negative value. */
int te_errh_verifyStates( tTestEngContext * context, int idx );

#ifdef __cplusplus
};
#endif

#endif /* __TESTENG_H */
