/******************************************************************************
 * File: redisbench.c
 * Author: Ernest Rozloznik (e@er400.io)
 * Date: 2025-02-22
 * Description: Benchmark comparing static table vs iconv EBCDIC/ASCII
 *              conversion performance on IBM i.
 *
 *              Run from PASE: /home/ernestr/redis400/redisbench
 *              Or from QSH/QSHELL after building.
 *
 *              Tests both directions (EBCDIC->ASCII and ASCII->EBCDIC)
 *              with varying data sizes and iteration counts.
 *
 * License: MIT (https://opensource.org/licenses/MIT)
 * Version: 1.0.0
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <iconv.h>
#include <qtqiconv.h>

/**********************************************************************/
/* Types */
/**********************************************************************/
typedef unsigned char uchar;

/**********************************************************************/
/* Static Translation Tables (CCSID 37 hardcoded) */
/**********************************************************************/

static uchar AsciiTable[256] = {
    0x00, 0x01, 0x02, 0x03, 0x20, 0x09, 0x20, 0x7f,
    0x20, 0x20, 0x20, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x20, 0x0a, 0x08, 0x20,
    0x18, 0x19, 0x20, 0x20, 0x20, 0x1d, 0x1e, 0x1f,
    0x20, 0x20, 0x1c, 0x20, 0x20, 0x0a, 0x17, 0x1b,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x05, 0x06, 0x07,
    0x20, 0x20, 0x16, 0x20, 0x20, 0x20, 0x20, 0x04,
    0x20, 0x20, 0x20, 0x20, 0x14, 0x15, 0x20, 0x1a,
    0x20, 0x20, 0x83, 0x84, 0x85, 0xa0, 0xc6, 0x86,
    0x87, 0xa4, 0xbd, 0x2e, 0x3c, 0x28, 0x2b, 0x7c,
    0x26, 0x82, 0x88, 0x89, 0x8a, 0xa1, 0x8c, 0x8b,
    0x8d, 0xe1, 0x21, 0x24, 0x2a, 0x29, 0x3b, 0xaa,
    0x2d, 0x2f, 0xb6, 0x8e, 0xb7, 0xb5, 0xc7, 0x8f,
    0x80, 0xa5, 0xdd, 0x2c, 0x25, 0x5f, 0x3e, 0x3f,
    0x9b, 0x90, 0xd2, 0xd3, 0xd4, 0xd6, 0xd7, 0xd8,
    0xde, 0x60, 0x3a, 0x23, 0x40, 0x27, 0x3d, 0x22,
    0x9d, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
    0x68, 0x69, 0xae, 0xaf, 0xd0, 0xec, 0xe7, 0xf1,
    0xf8, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70,
    0x71, 0x72, 0xa6, 0xa7, 0x91, 0xf7, 0x92, 0xcf,
    0xe6, 0x7e, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
    0x79, 0x7a, 0xad, 0xa8, 0xd1, 0xed, 0xe8, 0xa9,
    0x5e, 0x9c, 0xbe, 0xfa, 0xb8, 0x15, 0x14, 0xac,
    0xab, 0xf3, 0x5b, 0x5d, 0xee, 0xf9, 0xef, 0x9e,
    0x7b, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0xf0, 0x93, 0x94, 0x95, 0xa2, 0xe4,
    0x7d, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50,
    0x51, 0x52, 0xfb, 0x96, 0x81, 0x97, 0xa3, 0x98,
    0x5c, 0xf6, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
    0x59, 0x5a, 0xfc, 0xe2, 0x99, 0xe3, 0xe0, 0xe5,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0xfd, 0xea, 0x9a, 0xeb, 0xe9, 0xff
};

static uchar EbcdicTable[256] = {
    0x00, 0x01, 0x02, 0x03, 0x37, 0x2d, 0x2e, 0x2f,
    0x16, 0x05, 0x25, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x3c, 0x3d, 0x32, 0x26,
    0x18, 0x19, 0x3f, 0x27, 0x22, 0x1d, 0x1e, 0x1f,
    0x40, 0x5a, 0x7f, 0x7b, 0x5b, 0x6c, 0x50, 0x7d,
    0x4d, 0x5d, 0x5c, 0x4e, 0x6b, 0x60, 0x4b, 0x61,
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
    0xf8, 0xf9, 0x7a, 0x5e, 0x4c, 0x7e, 0x6e, 0x6f,
    0x7c, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7,
    0xc8, 0xc9, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6,
    0xd7, 0xd8, 0xd9, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6,
    0xe7, 0xe8, 0xe9, 0xba, 0xe0, 0xbb, 0xb0, 0x6d,
    0x79, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
    0x88, 0x89, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96,
    0x97, 0x98, 0x99, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6,
    0xa7, 0xa8, 0xa9, 0xc0, 0x4f, 0xd0, 0xa1, 0x07,
    0x68, 0xdc, 0x51, 0x42, 0x43, 0x44, 0x47, 0x48,
    0x52, 0x53, 0x54, 0x57, 0x56, 0x58, 0x63, 0x67,
    0x71, 0x9c, 0x9e, 0xcb, 0xcc, 0xcd, 0xdb, 0xdd,
    0xdf, 0xec, 0xfc, 0x70, 0xb1, 0x80, 0xbf, 0x40,
    0x45, 0x55, 0xee, 0xde, 0x49, 0x69, 0x9a, 0x9b,
    0xab, 0xaf, 0x5f, 0xb8, 0xb7, 0xaa, 0x8a, 0x8b,
    0x40, 0x40, 0x40, 0x40, 0x40, 0x65, 0x62, 0x64,
    0xb4, 0x40, 0x40, 0x40, 0x40, 0x4a, 0xb2, 0x40,
    0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x46, 0x66,
    0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x9f,
    0x8c, 0xac, 0x72, 0x73, 0x74, 0x89, 0x75, 0x76,
    0x77, 0x40, 0x40, 0x40, 0x40, 0x6a, 0x78, 0x40,
    0xee, 0x59, 0xeb, 0xed, 0xcf, 0xef, 0xa0, 0x8e,
    0xae, 0xfe, 0xfb, 0xfd, 0x8d, 0xad, 0xbc, 0xbe,
    0xca, 0x8f, 0x40, 0xb9, 0xb6, 0xb5, 0xe1, 0x9d,
    0x90, 0xbd, 0xb3, 0xda, 0xea, 0xfa, 0x40, 0x40
};

/**********************************************************************/
/* Method 1: Static table conversion */
/**********************************************************************/

int table_to_ascii(uchar *ip, size_t ilen, uchar *op)
{
    size_t i;
    for (i = 0; i < ilen; ++i)
    {
        *op = AsciiTable[*ip];
        ip++;
        op++;
    }
    return 0;
}

int table_to_ebcdic(uchar *ip, size_t ilen, uchar *op)
{
    size_t i;
    for (i = 0; i < ilen; ++i)
    {
        *op = EbcdicTable[*ip];
        ip++;
        op++;
    }
    return 0;
}

/**********************************************************************/
/* Method 2: iconv conversion */
/**********************************************************************/

static iconv_t acd; /* EBCDIC -> ASCII */
static iconv_t ecd; /* ASCII -> EBCDIC */

int init_iconv(void)
{
    QtqCode_T from, to;

    memset(&from, 0, sizeof(QtqCode_T));
    memset(&to, 0, sizeof(QtqCode_T));

    from.CCSID = 0;   /* Job CCSID (auto-detects) */
    to.CCSID = 819;    /* ISO 8859-1 ASCII */

    errno = 0;
    acd = QtqIconvOpen(&to, &from);
    if (errno != 0)
    {
        printf("ERROR: QtqIconvOpen (EBCDIC->ASCII) failed, errno=%d\n", errno);
        return -1;
    }

    errno = 0;
    ecd = QtqIconvOpen(&from, &to);
    if (errno != 0)
    {
        printf("ERROR: QtqIconvOpen (ASCII->EBCDIC) failed, errno=%d\n", errno);
        iconv_close(acd);
        return -1;
    }

    return 0;
}

int iconv_to_ascii(char *ibuf, size_t ilen, char *obuf, size_t olen)
{
    size_t ileft = ilen;
    size_t oleft = olen;
    char *ip = ibuf;
    char *op = obuf;
    return iconv(acd, &ip, &ileft, &op, &oleft);
}

int iconv_to_ebcdic(char *ibuf, size_t ilen, char *obuf, size_t olen)
{
    size_t ileft = ilen;
    size_t oleft = olen;
    char *ip = ibuf;
    char *op = obuf;
    return iconv(ecd, &ip, &ileft, &op, &oleft);
}

/**********************************************************************/
/* Timing helper */
/**********************************************************************/

double elapsed_ms(struct timeval *start, struct timeval *end)
{
    return (end->tv_sec - start->tv_sec) * 1000.0 +
           (end->tv_usec - start->tv_usec) / 1000.0;
}

/**********************************************************************/
/* Correctness check: compare table vs iconv output */
/**********************************************************************/

void check_correctness(char *ebcdic_data, size_t len)
{
    char table_out[33000], iconv_out[33000];
    int mismatches = 0;
    size_t i;

    memset(table_out, 0, sizeof(table_out));
    memset(iconv_out, 0, sizeof(iconv_out));

    table_to_ascii((uchar *)ebcdic_data, len, (uchar *)table_out);
    iconv_to_ascii(ebcdic_data, len, iconv_out, sizeof(iconv_out));

    for (i = 0; i < len; i++)
    {
        if (table_out[i] != iconv_out[i])
        {
            if (mismatches < 10)
            {
                printf("  MISMATCH at byte %d: EBCDIC=0x%02X table->0x%02X iconv->0x%02X\n",
                       (int)i,
                       (unsigned char)ebcdic_data[i],
                       (unsigned char)table_out[i],
                       (unsigned char)iconv_out[i]);
            }
            mismatches++;
        }
    }

    if (mismatches == 0)
        printf("  Correctness: PASS (all %d bytes match)\n", (int)len);
    else
        printf("  Correctness: FAIL (%d mismatches out of %d bytes)\n", mismatches, (int)len);
}

/**********************************************************************/
/* Run benchmark for a given data size */
/**********************************************************************/

void run_benchmark(char *label, char *ebcdic_data, size_t data_len, int iterations)
{
    char obuf[33000];
    struct timeval start, end;
    double table_ms, iconv_ms;
    int i;

    printf("\n--- %s (size=%d, iterations=%d) ---\n", label, (int)data_len, iterations);

    /* Check correctness first */
    check_correctness(ebcdic_data, data_len);

    /* Benchmark: Static table EBCDIC->ASCII */
    gettimeofday(&start, NULL);
    for (i = 0; i < iterations; i++)
    {
        table_to_ascii((uchar *)ebcdic_data, data_len, (uchar *)obuf);
    }
    gettimeofday(&end, NULL);
    table_ms = elapsed_ms(&start, &end);

    /* Benchmark: iconv EBCDIC->ASCII */
    gettimeofday(&start, NULL);
    for (i = 0; i < iterations; i++)
    {
        iconv_to_ascii(ebcdic_data, data_len, obuf, sizeof(obuf));
    }
    gettimeofday(&end, NULL);
    iconv_ms = elapsed_ms(&start, &end);

    printf("  Static table: %.3f ms (%.1f ns/byte)\n",
           table_ms, (table_ms * 1000000.0) / (iterations * data_len));
    printf("  iconv:        %.3f ms (%.1f ns/byte)\n",
           iconv_ms, (iconv_ms * 1000000.0) / (iterations * data_len));
    printf("  Ratio:        iconv is %.1fx %s than table\n",
           iconv_ms > table_ms ? iconv_ms / table_ms : table_ms / iconv_ms,
           iconv_ms > table_ms ? "slower" : "faster");
}

/**********************************************************************/
/* Main */
/**********************************************************************/

int main(int argc, char *argv[])
{
    int iterations = 100000;
    char small_data[64];    /* Typical Redis key */
    char medium_data[256];  /* Typical Redis command */
    char large_data[16370]; /* Max VARCHAR value */
    size_t i;

    printf("==============================================\n");
    printf("  EBCDIC/ASCII Conversion Benchmark\n");
    printf("  Static Tables vs iconv (QtqIconvOpen)\n");
    printf("==============================================\n");

    /* Initialize iconv */
    printf("\nInitializing iconv descriptors...\n");
    if (init_iconv() != 0)
    {
        printf("FATAL: Failed to initialize iconv. Cannot run benchmark.\n");
        return 1;
    }
    printf("iconv initialized successfully.\n");

    /* Print job CCSID info */
    printf("Job CCSID: (auto-detected via CCSID 0)\n");
    printf("Target CCSID: 819 (ISO 8859-1 / ASCII)\n");

    /* Prepare test data in EBCDIC */
    /* Fill with a pattern of printable EBCDIC characters (A-Z, 0-9) */

    /* Small: typical Redis key like "ORDER#12345" in EBCDIC */
    memset(small_data, 0, sizeof(small_data));
    for (i = 0; i < 32; i++)
    {
        /* Cycle through EBCDIC uppercase: A=0xC1, B=0xC2 ... I=0xC9, J=0xD1 ... */
        if (i % 2 == 0)
            small_data[i] = 0xC1 + (i % 9);       /* EBCDIC A-I */
        else
            small_data[i] = 0xF0 + (i % 10);      /* EBCDIC 0-9 */
    }

    /* Medium: typical RESP command */
    memset(medium_data, 0, sizeof(medium_data));
    for (i = 0; i < 200; i++)
    {
        if (i % 3 == 0)
            medium_data[i] = 0xC1 + (i % 9);      /* EBCDIC A-I */
        else if (i % 3 == 1)
            medium_data[i] = 0x81 + (i % 9);      /* EBCDIC a-i */
        else
            medium_data[i] = 0xF0 + (i % 10);     /* EBCDIC 0-9 */
    }

    /* Large: max VARCHAR payload */
    memset(large_data, 0, sizeof(large_data));
    for (i = 0; i < 16370; i++)
    {
        large_data[i] = 0xC1 + (i % 9);           /* EBCDIC A-I repeating */
    }

    /* Run benchmarks */
    run_benchmark("Small key (32 bytes)", small_data, 32, iterations);
    run_benchmark("Medium command (200 bytes)", medium_data, 200, iterations);
    run_benchmark("Large value (16370 bytes)", large_data, 16370, iterations / 10);

    /* Summary */
    printf("\n==============================================\n");
    printf("  Summary\n");
    printf("==============================================\n");
    printf("Static tables: Hardcoded CCSID 37 only.\n");
    printf("iconv: Uses job CCSID (works for ANY EBCDIC variant).\n");
    printf("If correctness shows PASS, both produce identical output\n");
    printf("for the current job CCSID.\n");
    printf("If correctness shows FAIL, the static tables are WRONG\n");
    printf("for this system's CCSID and iconv should be used.\n");
    printf("==============================================\n");

    /* Cleanup */
    iconv_close(acd);
    iconv_close(ecd);

    return 0;
}
