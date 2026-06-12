#ifndef __SAMPLE_COMMON_H__
#define __SAMPLE_COMMON_H__

#include <stdio.h>
#include "xmedia_type.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ERR_CODE_CIPHER(format, arg...)     printf( "\033[0;1;31m" format "\033[0m", ## arg)
#define INFO_CIPHER(format, arg...)         printf( "\033[0;1;32m" format "\033[0m", ## arg)
#define TEST_END_PASS()                     INFO_CIPHER("****************** %s test PASS !!! ******************\n", __FUNCTION__)
#define TEST_END_FAIL()                     ERR_CODE_CIPHER("****************** %s test FAIL !!! ******************\n", __FUNCTION__)
#define TEST_RESULT_PRINT()                 { if (ret) TEST_END_FAIL(); else TEST_END_PASS();}

static inline xmedia_s32 print_buffer(xmedia_char *string, const xmedia_u8 *input, xmedia_u32 length)
{
    xmedia_u32 i = 0;

    if (XMEDIA_NULL != string) {
        printf("%s\n", string);
    }

    for (i = 0 ; i < length; i++) {
        if ((i % 16 == 0) && (i != 0)) printf("\n");
        printf("0x%02x ", input[i]);
    }
    printf("\n");

    return XMEDIA_SUCCESS;
}

#ifdef __cplusplus
}
#endif

#endif
