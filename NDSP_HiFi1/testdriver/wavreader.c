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
    WAV Reader
*/
#include <limits.h>
#include <stdio.h>

#include "wavreader.h"

//
//Search no more than MAX_CHUNCKS_TO_SEARCH chunks for 'data' chunk
//
#define MAX_CHUNCKS_TO_SEARCH 20

//
//Magic constants 
//
#define RIFF_MAGIC 0x46464952ul //'RIFF'
#define WAVE_MAGIC 0x45564157ul //'WAVE'
#define FMT_MAGIC  0x20746D66ul //'fmt '
#define DATA_MAGIC 0x61746164ul //'data'


//
//Redefine these macros according to you data source
//
#if defined(__XTENSA_EB__)
#define READ_4(x)     {unsigned long y;                          \
                       x=0;y=0;                                  \
                       if (fread(&y,1,4,pF) != 4) goto l_fail;   \
                       x=(((y>>24)&0xFF)<< 0)|                   \
                         (((y>>16)&0xFF)<< 8)|                   \
                         (((y>> 8)&0xFF)<<16)|                   \
                         (((y>> 0)&0xFF)<<24);                   \
                      }
#define READ_2(x)     {unsigned short y;                        \
                       x=0; y=0;                                \
                       if (fread(&y,1,2,pF) != 2)  goto l_fail; \
                       x=(((y>> 8)&0xFF)<< 0)|                  \
                         (((y>> 0)&0xFF)<< 8);                  \
                      }
#else
#define READ_4(x)     {x=0;if (fread(&x,1,4,pF) != 4) goto l_fail;}
#define READ_2(x)     {x=0;if (fread(&x,1,2,pF) != 2) goto l_fail;}
#endif

#   define SKIP_BYTES(x) fseek(pF, x, SEEK_CUR)

/*-----------------------------------------------------------------------------

  Function:    WAV_readerOpen

*/ /**

  Opens file and parse WAV header. Copy header info to user-supplied variables 
  (all of them are optional).
  Only PCM WAV's accepted (with FormatTag == WAVE_FORMAT_PCM)
  Opened file position set to the beginning of the audio data.
  Note that WAV file can contain several chunks with misc. info at the end of 
  the file. To avoid processing of this trash, use pnDataSizeBytes parameter.

  @param pFileName       [IN]  File name
  @param pnSampleRateHz  [OUT, opt] Pointer to the sample rate value (for ex. 44100)
  @param pnChannels      [OUT, opt] Pointer to the number of channels (1,2...)
  @param pnBitsPerSample [OUT, opt] Pointer to the number of bits per sample (8,16...)
  @param pnDataSizeBytes [OUT, opt] Pointer to the size of "data" chunk

  @return  Opened FILE handler in case of success or NULL otherwise.

*/ /*
-----------------------------------------------------------------------------*/
FILE * WAV_readerOpen(const char     *pFileName        //[IN] File name
                     ,unsigned long  *pnSampleRateHz   //[OUT, opt] Sample rate, Hz
                     ,unsigned int   *pnChannels       //[OUT, opt] Number of channels
                     ,unsigned int   *pnBitsPerSample  //[OUT, opt] Bits per sample
                     ,unsigned long  *pnDataSizeBytes  //[OUT, opt] PCM data size in bytes
                     )
{
  FILE * pF = fopen(pFileName, "rb");
  if (pF)
  {
    int iSafetyCountDown = MAX_CHUNCKS_TO_SEARCH;
    unsigned long  tmp32;
    unsigned short tmp16;
    int condFormatTagFound = 0;
    
    //Verify file header
    READ_4(tmp32);                            //RiffHeader.Magic    ('RIFF')
//printf("RIFF=%08x\n",tmp32);
    if (tmp32 != RIFF_MAGIC) goto l_fail;
    READ_4(tmp32);                            //RiffHeader.FileSize (ignored)
    READ_4(tmp32);                            //RiffHeader.Type     ('WAVE')
//printf("WAVE=%08x\n",tmp32);
    if (tmp32 != WAVE_MAGIC) goto l_fail;
    
    //Loop through chunks until 'data' is found
    while (iSafetyCountDown--)
    {
      long lSubChunkSize;
      READ_4(tmp32);                          //Chunk.Id ('fmt ' or 'data')
      READ_4(lSubChunkSize);                  //ChunkFmt.Size
//printf("fmt=%08x\n",tmp32);
//printf("lSubChunskSize=%08x\n",lSubChunkSize);
      if (tmp32 == FMT_MAGIC)                 //'fmt ' chunk found: retrive audio info
      {
        if (lSubChunkSize < 16) goto l_fail;  //minimun 'fmt ' chunk size
        READ_2(tmp16);                        //Format.wFormatTag (1 for PCM)
//printf("wFormatTag=%04x\n",tmp16);
        //if (tmp16 != 1) goto l_fail;          //Not PCM data.

        if (tmp16 != 1 && tmp16 != 0xFFFE ) goto l_fail;          //Not PCM data.

        READ_2(tmp16);                        //Format.nChannels
        if (pnChannels) *pnChannels = (unsigned int)tmp16;
        READ_4(tmp32);                        //Format.nSamplesPerSec
        if (pnSampleRateHz) *pnSampleRateHz = tmp32;
        READ_4(tmp32);                        //Format.nAvgBytesPerSec (ignored)
        READ_2(tmp16);                        //Format.nBlockAlign     (ignored)
        READ_2(tmp16);                        //Format.BitsPerSample
        if (pnBitsPerSample) *pnBitsPerSample = (unsigned int)tmp16;
        lSubChunkSize -= 16;                  //Skip the rest of 'fmt ' chunk
        condFormatTagFound = 1;
      }
      else if (tmp32 == DATA_MAGIC)           //'data' chunk magic
      {
        if (!condFormatTagFound) goto l_fail; //'data' chunk before 'fmt ' chunk
        if (pnDataSizeBytes) *pnDataSizeBytes = lSubChunkSize;
        return pF;
      }
      SKIP_BYTES(lSubChunkSize);              //Skip unknown chunk
    }
l_fail:
    fclose(pF);
    pF = NULL;
  }
  return pF;
}
