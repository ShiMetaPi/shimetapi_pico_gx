/*
 * Copyright (c) XMEDIA. All rights reserved.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "xmedia_type.h"
#include "sample_api.h"

int main(int argc, char *argv[])
{
    xmedia_s32 ret = XMEDIA_FAILURE;
    xmedia_u32 func_num = 0;

    if (argc < 2) {
        printf("\n##########----Run a cipher sample:for example, ./sample 1----###############\n");
        printf("    You can selset a cipher sample to run as fllow:\n"
                   "    [1] CIPHER\n"
                   "    [2] CIPHER-MUTILT\n"
                   "    [3] CIPHER-EFUSE\n"
                   "    [4] HASH\n"
                   "    [5] RNG\n"
                   "    [6] RSA-SIGN\n"
                   "    [7] RSA-ENC\n"
                   "    [8] CIPHER-VIA\n");

        return XMEDIA_FAILURE;
    }

    func_num = strtol(argv[1], XMEDIA_NULL, 0);

    switch (func_num) {
        case 1:
            printf("\n~~~~~~~~~~~~~~~~~~~~~ run sample_cipher ~~~~~~~~~~~~~~~~~~~\n");
            ret = sample_cipher();
            break;
        case 2:
            printf("\n~~~~~~~~~~~~~~~~~~~~~ run sample_multicipher ~~~~~~~~~~~~~~~~~~~\n");
            ret = sample_multicipher();
            break;
        case 3:
            printf("\n~~~~~~~~~~~~~~~~~~~~~ run sample_cipher_efuse ~~~~~~~~~~~~~~~~~~~\n");
            ret = sample_cipher_efuse();
            break;
        case 4:
            printf("\n~~~~~~~~~~~~~~~~~~~~~ run sample_hash ~~~~~~~~~~~~~~~~~~~\n");
            ret = sample_hash();
            break;
        case 5:
            printf("\n~~~~~~~~~~~~~~~~~~~~~ run sample_rng ~~~~~~~~~~~~~~~~~~~\n");
            ret = sample_rng();
            break;
        case 6:
            printf("\n~~~~~~~~~~~~~~~~~~~~~ run sample_rsa_sign ~~~~~~~~~~~~~~~~~~~\n");
            ret = sample_rsa_sign();
            break;
        case 7:
            printf("\n~~~~~~~~~~~~~~~~~~~~~ run sample_rsa_enc ~~~~~~~~~~~~~~~~~~~\n");
            ret = sample_rsa_enc();
            break;
        case 8:
            printf("\n~~~~~~~~~~~~~~~~~~~~~ run sample_cipher_via ~~~~~~~~~~~~~~~~~~~\n");
            ret = sample_cipher_via();
            break;
        default:
            printf("Function Number %d do not exist!\n", func_num);
            ret = XMEDIA_FAILURE;
            break;
    }

    if (XMEDIA_SUCCESS == ret) {
        printf("\n################# FUNCTION %d RUN SUCCESS #################\n", func_num);
    } else {
        printf("\n################# FUNCTION %d RUN FAILURE #################\n", func_num);
    }

    return ret;
}

