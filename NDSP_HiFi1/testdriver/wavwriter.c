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
    WAV Writer
*/
#include <stdio.h>
#include "wavwriter.h"

/**
*    Standard WAV header size
*/
#define WAV_HEADER_SIZE 44

/**
*   Endian-independent byte-write macros
*/
#define WR(x, n) *p++ = (char)((x) >> 8*n)
#define WRITE_2(x) WR(x,0); WR(x,1);
#define WRITE_4(x) WR(x,0); WR(x,1); WR(x,2); WR(x,3);

/**
*   Writes standard WAV header in the beginning of the file.
*   return 1 in case of success, or 0 in case of write error or invalid params.
*/
static int WAV_writeHeader(
    FILE * pF,                  //!< FILE handle
    unsigned long hz,           //!< Sample rate value (for ex. 44100)
    unsigned int  ch,           //!< Number of channels (1,2...)
    unsigned int  bips,         //!< Number of bits per sample (8,16...)
    unsigned long file_size,    //!< Total file size in bytes (assume file 
                                //!< was opened with WAV_writerOpen())
    int           format_tag    //!< WAV format tag
    )
{
    int success = 0;
    if (pF)
    {
        unsigned long nAvgBytesPerSec = bips * ch * hz >> 3;
        unsigned int nBlockAlign      = bips * ch >> 3;
        char h[WAV_HEADER_SIZE];
        char * p = h;

        WRITE_4(0x46464952);                   //  0: RiffHeader.Magic = 'RIFF'
        WRITE_4(file_size - 8);                //  4: RiffHeader.FileSize = File size - 8
        WRITE_4(0x45564157);                   //  8: RiffHeader.Type = 'WAVE'
        WRITE_4(0x20746D66);                   //  C: ChunkFmt.Id = 'fmt ' (format description)
        WRITE_4(16L);                          // 10: ChunkFmt.Size = 16   (descriptor size)
        WRITE_2(format_tag);                   // 14: Format.wFormatTag    (see E_WAV_tag type)
        WRITE_2(ch);                           // 16: Format.ch
        WRITE_4(hz);                           // 18: Format.nSamplesPerSec
        WRITE_4(nAvgBytesPerSec);              // 1C: Format.nAvgBytesPerSec
        WRITE_2(nBlockAlign);                  // 20: Format.nBlockAlign
        WRITE_2(bips);                         // 22: Format.BitsPerSample
        WRITE_4(0x61746164);                   // 24: ChunkData.Id = 'data'
        WRITE_4(file_size - WAV_HEADER_SIZE);  // 28: ChunkData.Size = File size - 44
                                               //     Total size: 0x2C (44) bytes
        rewind(pF);
        success = fwrite(h, WAV_HEADER_SIZE, 1, pF);
    }
    return success;
}

/**
*   Opens file for writing, and appends WAV header template.
*   Overwrites existing files.
*   Return opened FILE handle.
* 
*   Example:
*
*   FILE * pWavFile = WAV_writerOpen("MyFile.wav");
*/
FILE * WAV_writerOpen(
    const char * pFileName      //!< File name to create.
    )
{
    FILE * pF = fopen(pFileName, "wb");
    if (pF)
    {
        if (!WAV_writeHeader(pF, 0, 0, 0, WAV_HEADER_SIZE, 1))
        {
            fclose(pF);
            pF = NULL;
        }
    }
    return pF;
}

/**
*   Writes WAV header and close the file. Assumes file was opened with 
*   WAV_writerOpen().
*   @return 1 in case of success, or 0 in case of write error or invalid params.
*
*   \warning If RTL ftell() badly implemented, use WAV_writeHeaderEx() 
* 
*   Example:
*
*   WAV_writerClose(pWavFile, 44100, 2, 16); // CD-format audio
*/
int WAV_writerClose(
    FILE * pF,                  //!< FILE handle
    unsigned long hz,           //!< Sample rate, Hz (44100, ...)
    unsigned int  ch,           //!< Number of channels (1,2 ...)
    unsigned int  bips          //!< Bits per sample (8,16 ...)
    )
{
    int status = 0;
    if (pF)
    {
        long size;
        fseek(pF, 0, SEEK_END);
        size = ftell(pF);
        status = WAV_writeHeader(pF, hz, ch, bips, size, 1);
        fclose(pF);
    }
    return status;
}


