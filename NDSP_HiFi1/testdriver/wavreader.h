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
#ifndef __WAVREADER_H__
#define __WAVREADER_H__

/*
// Opens WAV PCM (i.e. FormatTag == WAVE_FORMAT_PCM) file for reading, and 
// parse WAV header.
// All output parameters are optional.
// Returns opened FILE handle or NULL.
// 
// Example:

  unsigned long ulSampleRateHz;
  unsigned int  uiChannels;
  unsigned int  uiBitsPerSample;
  unsigned long ulDataSizeBytes;
  FILE * pWavFile = WAV_readerOpen("MyFile.wav", &ulSampleRateHz, &uiChannels, &uiBitsPerSample, &ulDataSizeBytes);

*/
FILE * WAV_readerOpen(const char     *pFileName        //[IN] File name
                     ,unsigned long  *pnSampleRateHz   //[OUT, opt] Sample rate
                     ,unsigned int   *pnChannels       //[OUT, opt] Number of channels
                     ,unsigned int   *pnBitsPerSample  //[OUT, opt] Bits per sample
                     ,unsigned long  *pnDataSizeBytes  //[OUT, opt] PCM data size in bytes
                     );

#endif //__WAVREADER_H__
