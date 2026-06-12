#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "quickstart_comm.h"


xmedia_void quickstart_media_start(xmedia_void)
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;
    xmedia_s32 venc_chn[2] = {0,1};

    s32Ret = media_comm_media_init();
    if (XMEDIA_SUCCESS != s32Ret) {
        MEDIA_PRT("media init fail \n");
        goto error0;
    }

    s32Ret = quickstart_comm_venc_start_get_stream(venc_chn, 2);
    if (XMEDIA_SUCCESS != s32Ret) {
        MEDIA_PRT("media init fail \n");
        goto error1;
    }

    sleep(15);

error1:
    quickstart_comm_venc_stop_get_stream();

error0:
    media_comm_media_exit();
    return;
}


xmedia_s32 main(xmedia_void)
{
    quickstart_media_start();
    return 0;
}
