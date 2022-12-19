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
    test module/example for testing NatureDSP_Signal library
	Integrit, 2006-2015
*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* Cross-platform data types. */
#include "NatureDSP_types.h"
/* Fixed-point arithmetic primitives. */
#include "NatureDSP_Math.h"
/* Cycles measurement API. */
#include "mips.h"


int main_fir     ( int phaseNum, int isFull, int isVerbose, int breakOnError );   void mips_fir     ( int phaseNum, FILE* f );
int main_iir     ( int phaseNum, int isFull, int isVerbose, int breakOnError );   void mips_iir     ( int phaseNum, FILE* f );
int main_mtx     ( int phaseNum, int isFull, int isVerbose, int breakOnError );   void mips_mtx     ( int phaseNum, FILE* f );
int main_math    ( int phaseNum, int isFull, int isVerbose, int breakOnError );   void mips_math    ( int phaseNum, FILE* f );
int main_vec     ( int phaseNum, int isFull, int isVerbose, int breakOnError );   void mips_vec     ( int phaseNum, FILE* f );
int main_fft     ( int phaseNum, int isFull, int isVerbose, int breakOnError );   void mips_fft     ( int phaseNum, FILE* f );
int main_cfft    ( int phaseNum, int isFull, int isVerbose, int breakOnError );   void mips_cfft    ( int phaseNum, FILE* f );
int main_rfft    ( int phaseNum, int isFull, int isVerbose, int breakOnError );   void mips_rfft    ( int phaseNum, FILE* f );
int main_dct     ( int phaseNum, int isFull, int isVerbose, int breakOnError );   void mips_dct     ( int phaseNum, FILE* f );
int main_mtxinv  ( int phaseNum, int isFull, int isVerbose, int breakOnError );   void mips_mtxinv  ( int phaseNum, FILE* f );
int main_help    ( int phaseNum, int isFull, int isVerbose, int breakOnError );   void mips_help    ( int phaseNum, FILE* f );
int main_mips    ( int phaseNum, int isFull, int isVerbose, int breakOnError );
int main_func	 ( int phaseNum, int isFull, int isVerbose, int breakOnError) { return 0;}

typedef struct
{
    const char * cmd;
    const char * help;
    int (*fntest)(int,int,int,int);
    void (*fnmips)(int,FILE*);
    uint64_t mask;
}
tTbl;

#define PACKAGE_MASK  0x0001ffffffffffffULL
#define PHASE_MASK    0x0700000000000000ULL
#define MIPS_FLAG     0x0800000000000000ULL
#define NOABORT_FLAG  0x1000000000000000ULL
#define VERBOSE_FLAG  0x2000000000000000ULL
#define FUNC_FLAG     0x0040000000000000ULL
#define FULL_FLAG     0x4000000000000000ULL
#define BRIEF_FLAG    0x0020000000000000ULL
#define SANITY_FLAG   0x0002000000000000ULL
#define HELP_FLAG     0x8000000000000000ULL

static const tTbl tbl[]=
{
  { "-fir"			, "FIR "									, main_fir		, mips_fir		, 0x0000000000000001ULL },
  { "-iir"			, "IIR "									, main_iir		, mips_iir		, 0x0000000000000004ULL },
  { "-mtx"			, "matrix operations"						, main_mtx		, mips_mtx		, 0x0000000000000008ULL },
  { "-mtxinv"		, "matrix inversion" 						, main_mtxinv   , mips_mtxinv	, 0x0000000000000010ULL },
  { "-vec"			, "vector operations "						, main_vec		, mips_vec		, 0x0000000000000020ULL },
  { "-math"			, "vector mathematics"						, main_math		, mips_math		, 0x0000000000000080ULL },
  { "-fft"			, "Fixed point FFT "						, main_fft		, mips_fft		, 0x0000000000000100ULL },
  { "-dct"			, "DCT "									, main_dct		, mips_dct		, 0x0000000000000200ULL },
  { "-cfft"			, "Complex FFT with memory improved usage "	, main_cfft  	, mips_cfft		, 0x0000000000000800ULL },
  { "-rfft"			, "Real FFT with memory improved usage "	, main_rfft  	, mips_rfft		, 0x0000000000001000ULL },

  { "-mips"         , "test for performance"              		, main_mips     , NULL          , MIPS_FLAG    },
  { "-phase<n>"     , "limit test coverage to library phase n" 	, NULL          , NULL          , PHASE_MASK   },
  { "-noabort"      , "do not stop after the first failure"    	, NULL          , NULL          , NOABORT_FLAG },
  { "-verbose"      , "print test progess info"                	, NULL          , NULL          , VERBOSE_FLAG },
  { "-func"         , "functional testing"                     	, main_func     , NULL          , FUNC_FLAG    },
  { "-full"         , "use full-size test vectors"             	, NULL          , NULL          , FULL_FLAG    },
  { "-brief"        , "use shorter test vectors"                , NULL          , NULL          , BRIEF_FLAG   },
  { "-sanity"       , "use sanity test vectors"                 , NULL          , NULL          , SANITY_FLAG   },
  { "-help"         , "this help"                              	, main_help     , NULL          , HELP_FLAG    },
  { "-h"            , 0                                        	, main_help     , NULL          , HELP_FLAG    }
};

int main_help( int phaseNum, int isFull, int isVerbose, int breakOnError )
{
    int k;
    printf("Available switches:\n");
    for (k=0; k<(int)(sizeof(tbl)/sizeof(tbl[0])); k++)
    {
        if (tbl[k].help==NULL) continue;
        printf("%-8s  %s\n",tbl[k].cmd,tbl[k].help);
    }
    return (1);
}

int main ( int argc, char * argv[] )
{

  /*----------------------------------------------------------------------------*
   *                            Show Library info                               *
   *----------------------------------------------------------------------------*/
  {
    char lib_version[32], api_version[32];

    NatureDSP_Signal_get_library_version(lib_version);
    NatureDSP_Signal_get_library_api_version(api_version);

    printf( "NatureDSP Signal Library version: %s\n"
            "NatureDSP Signal API version:     %s\n",
            lib_version, api_version);
  }


  /*----------------------------------------------------------------------------*
   *                         Scan the command line                              *
   *----------------------------------------------------------------------------*/
  const int tblSize = sizeof(tbl)/sizeof(tbl[0]);
  uint64_t flags = 0, packageMask;
  int m, n, phaseNum;

  for ( m=1; m<argc; m++ )
  {
    for ( n=0; n<tblSize; n++ )
    {
      size_t len = strcspn( tbl[n].cmd, "<[" );

      if ( len < strlen( tbl[n].cmd ) )
      {
        /* Command line option with numeric argument. */
        if ( !strncmp( argv[m], tbl[n].cmd, len ) && isdigit( (int)argv[m][len] ) )
        {
        	uint64_t lsb, p, mask = tbl[n].mask;
          /* Get 1 in the lowest bit position of a mask, and zeros elsewhere. */
          lsb = ( (mask<<1) ^ mask ) & mask;
          /* Retrieve the actual parameter. */
          p = lsb*( argv[m][len] - '0' );
          /* Check if actual parameter is non-zero and matches the mask.  */
          if ( p>0 && ( p & mask ) == p ) { flags |= p; break; }
        }
      }
      /* Simple command line option with no argument. */
      else if ( !strcmp( argv[m], tbl[n].cmd ) )
      {
        flags |= tbl[n].mask; break;
      }
    }

    if ( n >= tblSize )
    {
      printf( "Invalid command line option: %s\n", argv[m] );
      return (-1);
    }
  }

  /* Retrieve a library phase number, if 0 then test everything */
  {
	  /* Get 1 in the lowest bit position of the mask, and zeros elsewhere. */
	  const uint64_t lsb = ( (PHASE_MASK<<1) ^ PHASE_MASK ) & PHASE_MASK;
	  /* Place the phase number at lowest bit positions. */
	  phaseNum = (int)( ( flags & PHASE_MASK ) >> ( 32+30 - S_exp0_l( (uint32_t)(lsb>>32) ) ) );
  }

  /* If no particular packages are specified, then we test all the packages. */
  packageMask = ( !( flags & PACKAGE_MASK ) ? PACKAGE_MASK : ( flags & PACKAGE_MASK ) );
  if((flags & (MIPS_FLAG|FUNC_FLAG))==0) flags |= MIPS_FLAG;

  /*----------------------------------------------------------------------------*
   *                         Run the selected action                            *
   *----------------------------------------------------------------------------*/

  if ( flags & HELP_FLAG )
  {
    main_help( 0, 0, 0, 0 );
  }
  else if ( flags & MIPS_FLAG )
  {
    FILE * fout;

    fout = fopen( "performance.txt", "wb" );
    perf_info( fout, "Function Name\tCycles Measurements\t\n\tInvocation Parameters\tCycles\n" );

    /* Initialize mips testing. */
    main_mips( 0, 0, 0, 0 );

    for ( n=0; n<tblSize; n++ )
    {
      if ( 0 != ( tbl[n].mask & packageMask ) && NULL != tbl[n].fnmips )
      {
        tbl[n].fnmips( phaseNum, fout );
      }
    }

    fclose( fout );
    printf("================= MIPS test completed =================\n");
  }


  if (flags & FUNC_FLAG)
  {
    //int optFull    = ( 0 != ( flags & FULL_FLAG    ) );
    int optFull = (flags & FULL_FLAG) ? 0 : (flags & SANITY_FLAG) ? 2 : 1;
    int optVerbose = ( 0 != ( flags & VERBOSE_FLAG ) );
    int optBreak   = ( 0 == ( flags & NOABORT_FLAG ) );

    for ( n=0; n<tblSize; n++ )
    {
      if ( 0 == ( tbl[n].mask & packageMask ) || NULL == tbl[n].fntest ) continue;

      if ( !tbl[n].fntest( phaseNum, optFull, optVerbose, optBreak ) && optBreak ) break;
    }

    printf("================= FUNCTIONAL Test completed =================\n");
  }

  return (1);
} // end of main

