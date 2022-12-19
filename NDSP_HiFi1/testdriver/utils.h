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
    utility finctions
*/
#ifndef __UTILS_H__
#define __UTILS_H__

#include "wavwriter.h"
#include "wavreader.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PI
    #define PI  3.1415926535897932384626433832795
#endif

#ifdef _MSC_VER
    #define ALIGN(x)    _declspec(align(x)) 
#else
    #define ALIGN(x)    __attribute__((aligned(x))) 
#endif

#define abs_f24(x) ((x) >= 0 ? (x) : (x) != 0xff800000 ? -(x) : 0x007fffff)

void dump_open (const char *pFileName);
void dump_close();
void dump_i16  (const void *x, int size, const char *info);
void dump_i24  (const void *x, int size, const char *info);
void dump_f24  (const void *x, int size, const char *info);
void dump_i32  (const void *x, int size, const char *info);


#define RAND_RESET_A 0
#define RAND_RESET_B 0
void Rand_reset(int a, int b);
int  Rand(void);
void Rand_i32(void *x, int n);
void Rand_i16(void *x, int n);


int compare_i32(const int   *x, const int   *y, int N, int maxAllowedDiff);
int compare_f24(const long  *x, const long  *y, int N, int maxAllowedDiff);
int compare_i16(const short *x, const short *y, int N, int maxAllowedDiff);

int writepcm16(const int16_t *x, int n, FILE *fp);
int writepcm24(const int32_t *x, int n, FILE *fp);
int writepcm32(const int32_t *x, int n, FILE *fp);
int readpcm16(       int16_t *x, int n, FILE *fp);
int readpcm24(       int32_t *x, int n, FILE *fp);
int readpcm32(       int32_t *x, int n, FILE *fp);

#define WAVREADER_OPEN_16_MONO(fp, name, lHz)                   \
    {                                                           \
        unsigned nCh, bps;                                      \
        fp = WAV_readerOpen(name, &lHz, &nCh, &bps, NULL);   \
        if(fp == NULL) {                                        \
            printf("ERROR: can't open for reading %s\n", name); \
            hr = TEST_FAILED_FILEOPEN;                          \
            goto exit;                                          \
        }                                                       \
        if(nCh != 1) {                                          \
            printf("ERROR: %s has %d channels, but mono input required\n", name, nCh);   \
            hr = TEST_FAILED_FILEOPEN;                          \
            goto exit;                                          \
        }                                                       \
        if(bps != 16) {                                         \
            printf("ERROR: %s is %d-bit PCM, but 16-bit input reqired\n", name, bps);   \
            hr = TEST_FAILED_FILEOPEN;                          \
            goto exit;                                          \
        }                                                       \
    }

#define WAVREADER_OPEN_24_MONO(fp, name, lHz)                   \
    {                                                           \
        unsigned nCh, bps;                                      \
        fp = WAV_readerOpen(name, &lHz, &nCh, &bps, NULL);   \
        if(fp == NULL) {                                        \
            printf("ERROR: can't open for reading %s\n", name); \
            hr = TEST_FAILED_FILEOPEN;                          \
            goto exit;                                          \
        }                                                       \
        if(nCh != 1) {                                          \
            printf("ERROR: %s has %d channels, but mono input required\n", name, nCh);   \
            hr = TEST_FAILED_FILEOPEN;                          \
            goto exit;                                          \
        }                                                       \
        if(bps != 24) {                                         \
            printf("ERROR: %s is %d-bit PCM, but 24-bit input reqired\n", name, bps);   \
            hr = TEST_FAILED_FILEOPEN;                          \
            goto exit;                                          \
        }                                                       \
    }

#define WAVREADER_OPEN_32_MONO(fp, name, lHz)                   \
    {                                                           \
        unsigned nCh, bps;                                      \
        fp = WAV_readerOpen(name, &lHz, &nCh, &bps, NULL);   \
        if(fp == NULL) {                                        \
            printf("ERROR: can't open for reading %s\n", name); \
            hr = TEST_FAILED_FILEOPEN;                          \
            goto exit;                                          \
        }                                                       \
        if(nCh != 1) {                                          \
            printf("ERROR: %s has %d channels, but mono input required\n", name, nCh);   \
            hr = TEST_FAILED_FILEOPEN;                          \
            goto exit;                                          \
        }                                                       \
        if(bps != 32) {                                         \
            printf("ERROR: %s is %d-bit PCM, but 32-bit input reqired\n", name, bps);   \
            hr = TEST_FAILED_FILEOPEN;                          \
            goto exit;                                          \
        }                                                       \
    }

#define WAVREADER_OPEN_16_STEREO(fp, name, lHz)                 \
    {                                                           \
        unsigned nCh, bps;                                      \
        fp = WAV_readerOpen(name, &lHz, &nCh, &bps, NULL);   \
        if(fp == NULL) {                                        \
            printf("ERROR: can't open for reading %s\n", name); \
            hr = TEST_FAILED_FILEOPEN;                          \
            goto exit;                                          \
        }                                                       \
        if(nCh != 2) {                                          \
            printf("ERROR: %s has %d channels, but stereo input required\n", name, nCh);   \
            hr = TEST_FAILED_FILEOPEN;                          \
            goto exit;                                          \
        }                                                       \
        if(bps != 16) {                                         \
            printf("ERROR: %s is %d-bit PCM, but 16-bit input reqired\n", name, bps);   \
            hr = TEST_FAILED_FILEOPEN;                          \
            goto exit;                                          \
        }                                                       \
    }

#define WAVREADER_OPEN_24_STEREO(fp, name, lHz)                 \
    {                                                           \
        unsigned nCh, bps;                                      \
        fp = WAV_readerOpen(name, &lHz, &nCh, &bps, NULL);   \
        if(fp == NULL) {                                        \
            printf("ERROR: can't open for reading %s\n", name); \
            hr = TEST_FAILED_FILEOPEN;                          \
            goto exit;                                          \
        }                                                       \
        if(nCh != 2) {                                          \
            printf("ERROR: %s has %d channels, but stereo input required\n", name, nCh);   \
            hr = TEST_FAILED_FILEOPEN;                          \
            goto exit;                                          \
        }                                                       \
        if(bps != 24) {                                         \
            printf("ERROR: %s is %d-bit PCM, but 24-bit input reqired\n", name, bps);   \
            hr = TEST_FAILED_FILEOPEN;                          \
            goto exit;                                          \
        }                                                       \
    }

#define WAVREADER_OPEN_32_STEREO(fp, name, lHz)                 \
    {                                                           \
        unsigned nCh, bps;                                      \
        fp = WAV_readerOpen(name, &lHz, &nCh, &bps, NULL);   \
        if(fp == NULL) {                                        \
            printf("ERROR: can't open for reading %s\n", name); \
            hr = TEST_FAILED_FILEOPEN;                          \
            goto exit;                                          \
        }                                                       \
        if(nCh != 2) {                                          \
            printf("ERROR: %s has %d channels, but stereo input required\n", name, nCh);   \
            hr = TEST_FAILED_FILEOPEN;                          \
            goto exit;                                          \
        }                                                       \
        if(bps != 32) {                                         \
            printf("ERROR: %s is %d-bit PCM, but 32-bit input reqired\n", name, bps);   \
            hr = TEST_FAILED_FILEOPEN;                          \
            goto exit;                                          \
        }                                                       \
    }

#define WAVWRITER_OPEN(fp, name)                                \
    {                                                           \
        fp = WAV_writerOpen(name);                              \
        if(fp == NULL) {                                        \
            printf("ERROR: can't open for writing %s\n", name); \
            hr = TEST_FAILED_FILEOPEN;                          \
            goto exit;                                          \
        }                                                       \
    }

#define WAVREADER_CLOSE(fp)                                     \
    if(fp) {                                                    \
        fclose(fp);                                             \
        fp = NULL;                                              \
    }

#define WAVWRITER_CLOSE(fp, lHz, nCh, bps)                      \
    if(fp) {                                                    \
        WAV_writerClose(fp, lHz, nCh, bps);                     \
        fp = NULL;                                              \
    }

// compare 2 files
void filecompare(const char* fname1, const char* fname2);

// Randomize unused bits of 32-bit words holding 24-bit numbers.
void randomizeUnusedBits( f24 * restrict x, int N );

/* 32-bit CRC update for a data block. Number of octets must be even! */
uint32_t crc32( uint32_t crc, const uint8_t * buf, int len );
 
#define HEADER() printf("%-20s test accuracy align scalar\n", "name")
#define DO_TEST(test, info)         \
    {                               \
        int hr;                     \
        char report[128];           \
        FILE *fp = NULL;            \
        printf("%-20s ", info);     \
        fflush(stdout);             \
        hr = test;                  \
        sprintf(report, "%4s %8s %5s %5s\n", !IS_OK(hr)             ? "FAIL" : "OK",   \
                                     hr == TEST_FAILED_ACCURACY ? "fail" : "ok",   \
                                     hr == TEST_FAILED_ALIGN    ? "fail" : "ok",   \
                                     hr == TEST_FAILED_SCALAR   ? "fail" : "ok");  \
        printf("%s", report);                                       \
        fp = fopen("report.txt", "a");                              \
        if(fp == NULL) {                                            \
            printf("ERROR: can't open report.txt fpr writing.\n");  \
        } else {                                                    \
            fprintf(fp, "%-20s %s", info, report);                  \
            fclose(fp);                                             \
        }                                                           \
    }

#ifdef __cplusplus
};
#endif

#endif//__UTILS_H__
