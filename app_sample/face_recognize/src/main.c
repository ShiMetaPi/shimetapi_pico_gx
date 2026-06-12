/*
 * MTCNN Face Detection for GK7206
 *
 * Pipeline: VI → VPSS → VENC (MJPEG web stream)
 *                  → ochn1 (320×180 → NPU MTCNN inference)
 *
 * MTCNN 3-stage cascade:
 *   P-Net: input [1,3,12,12]    → pyramid sliding-window patches
 *   R-Net: input [1,3,24,24]   → per-candidate refine
 *   O-Net: input [1,3,48,48]   → final refine + 5 facial landmarks
 *   + Shape filter (aspect ratio, size, boundary)
 *   + Frame-to-frame tracking (hit/miss, EMA smoothing)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>

#include "sample_comm.h"
#include "sample_comm_sys.h"
#include "sample_comm_vi.h"
#include "sample_comm_venc.h"
#include "sample_comm_vpss.h"
#include "sample_comm_isp.h"
#include "web_server.h"

#include "xmedia_cl.h"
#include "xmedia_cl_common.h"
#include "xmedia_mmz.h"

/* ------------------------------------------------------------------ */
/*  Constants                                                          */
/* ------------------------------------------------------------------ */
#define FACE_VPSS_PIPE       0
#define FACE_VPSS_OCHN_ENC   0   /* full resolution → VENC MJPEG */
#define FACE_VPSS_OCHN_NPU   1   /* 320×180 → NPU inference (16:9 to match sensor) */

#define DET_WIDTH            320
#define DET_HEIGHT           180  /* 16:9 ratio — matches sensor 2560×1440 */

#define PNET_MODEL_PATH      "./models/pnet.xmm"
#define RNET_MODEL_PATH      "./models/rnet.xmm"
#define ONET_MODEL_PATH      "./models/onet.xmm"

/* P-Net sliding window parameters */
#define PNET_PATCH_SIZE      12
#define PNET_STRIDE          4    /* 4 = faster (4x fewer patches); 2 = better small-face recall */
/* Image pyramid: start from 12/min_face_size, shrink by factor each level */
#define PNET_MIN_FACE_SIZE   48   /* minimum detectable face size (pixels); 48=faster, 24=more sensitive */
#define PNET_SCALE_FACTOR    0.707f  /* 1/sqrt(2), standard MTCNN */

/* Cascade confidence thresholds
 *
 * Since the model already outputs softmax probabilities (s1 is the
 * face probability directly, range 0.0–1.0), these thresholds
 * compare against the true probability.
 */
#define FACE_CONF_THRESHOLD  0.50f  /* P-Net: moderate — reduce false candidates */
#define RNET_CONF_THRESHOLD  0.70f  /* R-Net: matches test_mtcnn working threshold */
#define ONET_CONF_THRESHOLD  0.30f  /* O-Net: lowered — quantization reduces scores */
#define FACE_NMS_THRESHOLD   0.50f

/* Face box shape filter: reject non-face-like boxes.
 * NOTE: Quantized MTCNN bbox_reg systematically narrows boxes (dx2 < 0).
 * After P-Net→R-Net→O-Net regression, w/h ratio can drop to ~0.25.
 * Do NOT use a high min aspect ratio — it will reject all real faces. */
#define FACE_MIN_ASPECT      0.20f  /* allow narrow boxes (quantized regression artifact) */
#define FACE_MAX_ASPECT      3.0f   /* max w/h ratio */
#define FACE_MIN_SIZE        6.0f   /* min face size in pixels (detection image) */
#define FACE_MAX_AREA_RATIO  0.90f  /* max face area as fraction of detection image */

/* Face tracking / stabilization */
#define FACE_TRACK_MIN_HITS  1     /* consecutive detections before showing (1=immediate) */
#define FACE_TRACK_MAX_MISS  3     /* consecutive misses before removing */
#define FACE_TRACK_SMOOTH    1.0f  /* EMA smoothing: 1.0 = instant follow, no lag */
#define FACE_TRACK_IOU_THR   0.3f  /* IoU threshold for matching tracks */

#define MAX_CANDIDATES       1000
#define MAX_FACES            50

/* ALIGN_UP already defined in math_fun.h */
#ifndef ALIGN_UP
#define ALIGN_UP(x, a) (((x) + (a) - 1) & ~((a) - 1))
#endif

/* ------------------------------------------------------------------ */
/*  Data types                                                         */
/* ------------------------------------------------------------------ */
typedef struct {
    float x1, y1, x2, y2;
    float score;
    float landmarks[10];   /* 5 keypoints (x,y) × 5, from O-Net */
} face_box_t;

/* Tracked face for frame-to-frame stabilization */
typedef struct {
    face_box_t box;       /* smoothed position (displayed to user) */
    face_box_t raw_box;   /* latest raw detection position */
    int hit_count;        /* consecutive frames detected */
    int miss_count;       /* consecutive frames NOT detected */
    int active;           /* 1 = currently displayed */
    int id;               /* unique track ID */
} tracked_face_t;

/* Encapsulates one NPU model (graph + tensors + buffers) */
typedef struct {
    xmedia_cl_graph graph;
    xmedia_cl_tensor_info_inout input;
    xmedia_cl_tensor_info_inout output;
    void *work_buf;
    void *weight_buf;
    void *input_buf;
    void *output_buf;
    xmedia_u64 work_phy;
    xmedia_u64 weight_phy;
    xmedia_u64 input_phy;
    xmedia_u64 output_phy;
} npu_model_t;

/* ------------------------------------------------------------------ */
/*  Global state                                                       */
/* ------------------------------------------------------------------ */
static volatile int g_stop = 0;

/* NPU */
static xmedia_cl_context g_cl_ctx = NULL;
static xmedia_bool g_cl_inited = XMEDIA_FALSE;
static npu_model_t g_pnet, g_rnet, g_onet;
static xmedia_bool g_npu_inited = XMEDIA_FALSE;

/* Face detection results (shared between NPU thread and HTTP handler) */
static face_box_t g_faces[MAX_FACES];
static volatile int g_face_count = 0;
static pthread_mutex_t g_face_mutex = PTHREAD_MUTEX_INITIALIZER;
static volatile int g_npu_fps = 0;
static volatile xmedia_bool g_npu_running = XMEDIA_FALSE;
static pthread_t g_npu_thread;

/* Face tracking state */
static tracked_face_t g_tracks[MAX_FACES];
static int g_track_count = 0;
static int g_track_next_id = 1;

/* Video pipeline */
static sample_vi_config g_vi_cfg = {0};
static sample_venc_config g_venc_cfg = {0};
static sample_sys_config g_sys_cfg = {0};
static sample_comm_video_param g_video_param = {0};
static sample_isp_param g_isp_param = {0};
static sample_vpss_config g_vpss_cfg = {0};
static vi_sensor_info g_sensor_info = {0};

static xmedia_bool g_venc_started = XMEDIA_FALSE;
static xmedia_bool g_vi_vpss_bound = XMEDIA_FALSE;
static xmedia_bool g_vpss_venc_bound = XMEDIA_FALSE;
static xmedia_bool g_isp_started = XMEDIA_FALSE;
static xmedia_bool g_vi_started = XMEDIA_FALSE;
static xmedia_bool g_isp_inited = XMEDIA_FALSE;
static xmedia_bool g_venc_module_inited = XMEDIA_FALSE;
static xmedia_bool g_vi_module_inited = XMEDIA_FALSE;
static xmedia_bool g_vpss_module_inited = XMEDIA_FALSE;
static xmedia_bool g_vpss_started = XMEDIA_FALSE;

/* ------------------------------------------------------------------ */
/*  MMZ helpers                                                        */
/* ------------------------------------------------------------------ */
static xmedia_s32 mmz_alloc_map(const char *mmz_name, const char *buf_name,
                                xmedia_u64 *phy, void **virt, xmedia_u32 size,
                                xmedia_bool cached)
{
    *phy = xmedia_mmz_alloc(mmz_name, buf_name, size);
    if (*phy == 0) {
        printf("[mmz] alloc failed, size=%u\n", size);
        return XMEDIA_FAILURE;
    }
    *virt = xmedia_mmz_map(*phy, size, cached);
    if (*virt == NULL) {
        printf("[mmz] map failed, size=%u\n", size);
        xmedia_mmz_free(*phy);
        *phy = 0;
        return XMEDIA_FAILURE;
    }
    return XMEDIA_SUCCESS;
}

static void mmz_unmap_free(xmedia_u64 phy, void *virt)
{
    if (virt != NULL) xmedia_mmz_unmap(virt);
    if (phy != 0) xmedia_mmz_free(phy);
}

/* ------------------------------------------------------------------ */
/*  Signal handler                                                     */
/* ------------------------------------------------------------------ */
static void sig_handler(int sig)
{
    (void)sig;
    g_stop = 1;
    web_server_stop();
}

/* ------------------------------------------------------------------ */
/*  NPU model management                                               */
/* ------------------------------------------------------------------ */
/* Forward declarations (helpers used in load_one_model) */
static int model_input_is_nchw(const npu_model_t *m);

static int load_one_model(npu_model_t *m, xmedia_cl_context ctx,
                          const char *path)
{
    xmedia_cl_s32 ret;
    xmedia_cl_u32 worksize = 0, weightsize = 0;
    xmedia_cl_u32 inputsize = 0, outputsize = 0;
    xmedia_u32 offset;
    xmedia_cl_u32 i;

    memset(m, 0, sizeof(*m));

    /* Query sizes */
    ret = xmedia_cl_graph_querysize_from_file(path, &worksize, &weightsize);
    if (ret != XMEDIA_CL_SUCCESS) {
        printf("[npu] query size failed for %s: %d\n", path, ret);
        return XMEDIA_FAILURE;
    }
    printf("[npu] %s: worksize=%u, weightsize=%u\n", path, worksize, weightsize);

    /* Allocate workspace + weight */
    ret = mmz_alloc_map(NULL, "npu_work", &m->work_phy,
                        &m->work_buf, worksize, XMEDIA_TRUE);
    if (ret != XMEDIA_SUCCESS) return ret;

    ret = mmz_alloc_map(NULL, "npu_weight", &m->weight_phy,
                        &m->weight_buf, weightsize, XMEDIA_TRUE);
    if (ret != XMEDIA_SUCCESS) return ret;

    /* Load model */
    ret = xmedia_cl_graph_loadmodel_from_file_withmem(
        &ctx, path, m->work_buf, worksize, m->weight_buf, weightsize, &m->graph);
    if (ret != XMEDIA_CL_SUCCESS) {
        printf("[npu] load model failed for %s: %d\n", path, ret);
        return XMEDIA_FAILURE;
    }
    printf("[npu] %s loaded\n", path);

    /* Get input tensor info (2-pass) */
    ret = xmedia_cl_graph_get_input(m->graph, 0, &m->input);
    if (ret != XMEDIA_CL_SUCCESS) return ret;

    m->input.tensor = malloc(sizeof(xmedia_cl_tensor) * m->input.num);
    m->input.tensor_batch = malloc(sizeof(xmedia_cl_tensor_batch) * m->input.num);
    m->input.current_batch = malloc(sizeof(xmedia_cl_u32) * m->input.num);
    if (!m->input.tensor || !m->input.tensor_batch || !m->input.current_batch)
        return XMEDIA_FAILURE;

    ret = xmedia_cl_graph_get_input(m->graph, m->input.num, &m->input);
    if (ret != XMEDIA_CL_SUCCESS) return ret;

    /* Allocate input buffer */
    for (i = 0; i < m->input.num; i++)
        inputsize += ALIGN_UP(m->input.tensor[i].size, 64);

    ret = mmz_alloc_map(NULL, "npu_in", &m->input_phy,
                        &m->input_buf, inputsize, XMEDIA_FALSE);
    if (ret != XMEDIA_SUCCESS) return ret;

    offset = 0;
    for (i = 0; i < m->input.num; i++) {
        m->input.tensor[i].addr = (char *)m->input_buf + offset;
        offset += ALIGN_UP(m->input.tensor[i].size, 64);
        printf("[npu] %s input[%u]: size=%u, dims=[%u,%u,%u,%u], type=%d, "
               "quant scale=%.6f zp=%d\n",
            path, i, m->input.tensor[i].size,
            m->input.tensor[i].shape.ndims > 0 ? m->input.tensor[i].shape.dims[0] : 0,
            m->input.tensor[i].shape.ndims > 1 ? m->input.tensor[i].shape.dims[1] : 0,
            m->input.tensor[i].shape.ndims > 2 ? m->input.tensor[i].shape.dims[2] : 0,
            m->input.tensor[i].shape.ndims > 3 ? m->input.tensor[i].shape.dims[3] : 0,
            m->input.tensor[i].shape.type,
            m->input.tensor[i].quant.scale, m->input.tensor[i].quant.zp);
    }

    /* Get output tensor info (2-pass) */
    ret = xmedia_cl_graph_get_output(m->graph, 0, &m->output);
    if (ret != XMEDIA_CL_SUCCESS) return ret;

    m->output.tensor = malloc(sizeof(xmedia_cl_tensor) * m->output.num);
    m->output.tensor_batch = malloc(sizeof(xmedia_cl_tensor_batch) * m->output.num);
    m->output.current_batch = malloc(sizeof(xmedia_cl_u32) * m->output.num);
    if (!m->output.tensor || !m->output.tensor_batch || !m->output.current_batch)
        return XMEDIA_FAILURE;

    ret = xmedia_cl_graph_get_output(m->graph, m->output.num, &m->output);
    if (ret != XMEDIA_CL_SUCCESS) return ret;

    /* Allocate output buffer */
    for (i = 0; i < m->output.num; i++)
        outputsize += ALIGN_UP(m->output.tensor[i].size, 64);

    ret = mmz_alloc_map(NULL, "npu_out", &m->output_phy,
                        &m->output_buf, outputsize, XMEDIA_FALSE);
    if (ret != XMEDIA_SUCCESS) return ret;

    offset = 0;
    for (i = 0; i < m->output.num; i++) {
        m->output.tensor[i].addr = (char *)m->output_buf + offset;
        offset += ALIGN_UP(m->output.tensor[i].size, 64);
        printf("[npu] %s output[%u]: size=%u, dims=[%u,%u,%u,%u], type=%d, "
               "quant scale=%.6f zp=%d\n",
            path, i, m->output.tensor[i].size,
            m->output.tensor[i].shape.ndims > 0 ? m->output.tensor[i].shape.dims[0] : 0,
            m->output.tensor[i].shape.ndims > 1 ? m->output.tensor[i].shape.dims[1] : 0,
            m->output.tensor[i].shape.ndims > 2 ? m->output.tensor[i].shape.dims[2] : 0,
            m->output.tensor[i].shape.ndims > 3 ? m->output.tensor[i].shape.dims[3] : 0,
            m->output.tensor[i].shape.type,
            m->output.tensor[i].quant.scale, m->output.tensor[i].quant.zp);
    }

    /* Set inout */
    ret = xmedia_cl_graph_set_inout(m->graph, &m->input, &m->output);
    if (ret != XMEDIA_CL_SUCCESS) {
        printf("[npu] set_inout failed for %s: %d\n", path, ret);
        return XMEDIA_FAILURE;
    }

    printf("[npu] %s ready (%u inputs, %u outputs, layout=%s)\n",
           path, m->input.num, m->output.num,
           model_input_is_nchw(m) ? "NCHW" : "NHWC");
    return XMEDIA_SUCCESS;
}

static void unload_one_model(npu_model_t *m)
{
    if (m->graph != NULL) {
        xmedia_cl_graph_unload(m->graph);
        m->graph = NULL;
    }
    mmz_unmap_free(m->output_phy, m->output_buf);
    m->output_buf = NULL; m->output_phy = 0;
    mmz_unmap_free(m->input_phy, m->input_buf);
    m->input_buf = NULL; m->input_phy = 0;
    mmz_unmap_free(m->weight_phy, m->weight_buf);
    m->weight_buf = NULL; m->weight_phy = 0;
    mmz_unmap_free(m->work_phy, m->work_buf);
    m->work_buf = NULL; m->work_phy = 0;
    free(m->input.tensor); m->input.tensor = NULL;
    free(m->input.tensor_batch); m->input.tensor_batch = NULL;
    free(m->input.current_batch); m->input.current_batch = NULL;
    free(m->output.tensor); m->output.tensor = NULL;
    free(m->output.tensor_batch); m->output.tensor_batch = NULL;
    free(m->output.current_batch); m->output.current_batch = NULL;
}

static int init_npu(void)
{
    xmedia_cl_s32 ret;
    xmedia_cl_device_id *devices = NULL;
    xmedia_cl_u32 num_devices = 0;
    xmedia_cl_s32 err_code = 0;

    printf("[npu] Initializing...\n");

    ret = xmedia_cl_init();
    if (ret != XMEDIA_CL_SUCCESS) {
        printf("[npu] cl_init failed: %d\n", ret);
        return XMEDIA_FAILURE;
    }
    g_cl_inited = XMEDIA_TRUE;

    ret = xmedia_cl_get_device_ids(XMEDIA_CL_DEVICE_ALL, NULL, &num_devices);
    if (ret != XMEDIA_CL_SUCCESS || num_devices == 0) {
        printf("[npu] no NPU device found\n");
        return XMEDIA_FAILURE;
    }
    printf("[npu] Found %u device(s)\n", num_devices);

    devices = calloc(num_devices, sizeof(xmedia_cl_device_id));
    ret = xmedia_cl_get_device_ids(XMEDIA_CL_DEVICE_ALL, devices, &num_devices);
    if (ret != XMEDIA_CL_SUCCESS) { free(devices); return XMEDIA_FAILURE; }

    g_cl_ctx = xmedia_cl_create_context(num_devices, devices, &err_code);
    free(devices);
    if (err_code != XMEDIA_CL_SUCCESS || g_cl_ctx == NULL) {
        printf("[npu] create context failed: %d\n", err_code);
        return XMEDIA_FAILURE;
    }

    /* Load 3 MTCNN models */
    printf("[npu] Loading P-Net...\n");
    ret = load_one_model(&g_pnet, g_cl_ctx, PNET_MODEL_PATH);
    if (ret != XMEDIA_SUCCESS) {
        printf("[npu] P-Net load failed. Need pnet.xmm compiled for [1,3,%d,%d] input.\n",
               PNET_PATCH_SIZE, PNET_PATCH_SIZE);
        return ret;
    }

    printf("[npu] Loading R-Net...\n");
    ret = load_one_model(&g_rnet, g_cl_ctx, RNET_MODEL_PATH);
    if (ret != XMEDIA_SUCCESS) return ret;

    printf("[npu] Loading O-Net...\n");
    ret = load_one_model(&g_onet, g_cl_ctx, ONET_MODEL_PATH);
    if (ret != XMEDIA_SUCCESS) return ret;

    g_npu_inited = XMEDIA_TRUE;
    printf("[npu] All models loaded!\n");
    return XMEDIA_SUCCESS;
}

static void deinit_npu(void)
{
    unload_one_model(&g_onet);
    unload_one_model(&g_rnet);
    unload_one_model(&g_pnet);
    if (g_cl_ctx != NULL) {
        xmedia_cl_release_context(g_cl_ctx);
        g_cl_ctx = NULL;
    }
    if (g_cl_inited) {
        xmedia_cl_uninit();
        g_cl_inited = XMEDIA_FALSE;
    }
    g_npu_inited = XMEDIA_FALSE;
    printf("[npu] Deinitialized\n");
}

/* YUV420SP (NV21) → RGB888 HWC
 *
 * The MTCNN model YAML config specifies input_format: RGB and the build
 * tool bakes the (pixel-127.5)/128 normalization into the quantized model.
 * We just feed uint8 RGB directly — the NPU handles normalization internally.
 *
 * pix_fmt=1 = YVU_SEMIPLANAR_420 = NV21: V first, then U
 */
static void yuv420sp_to_rgb888(const unsigned char *y_data,
                               const unsigned char *uv_data,
                               unsigned char *rgb_out,
                               int width, int height,
                               int y_stride, int uv_stride)
{
    int row, col;
    for (row = 0; row < height; row++) {
        for (col = 0; col < width; col++) {
            int y_idx  = row * y_stride + col;
            int uv_idx = (row >> 1) * uv_stride + (col & ~1);
            int y = y_data[y_idx];
            /* pix_fmt=1 = YVU_SEMIPLANAR_420 = NV21: V first, then U */
            int v = uv_data[uv_idx] - 128;         /* first byte = V (NV21) */
            int u = uv_data[uv_idx + 1] - 128;     /* second byte = U (NV21) */
            int r = y + ((1436 * v) >> 10);
            int g = y - ((352 * u + 731 * v) >> 10);
            int b = y + ((1814 * u) >> 10);
            int out_idx = row * width + col;
            rgb_out[out_idx * 3]     = (unsigned char)((r < 0) ? 0 : (r > 255) ? 255 : r);
            rgb_out[out_idx * 3 + 1] = (unsigned char)((g < 0) ? 0 : (g > 255) ? 255 : g);
            rgb_out[out_idx * 3 + 2] = (unsigned char)((b < 0) ? 0 : (b > 255) ? 255 : b);
        }
    }
}

/* ------------------------------------------------------------------ */
/*  MTCNN utility functions                                            */
/* ------------------------------------------------------------------ */

/* Check if model's first input tensor uses NCHW layout [1,3,H,W] */
static int model_input_is_nchw(const npu_model_t *m)
{
    const xmedia_cl_tensor *t = &m->input.tensor[0];
    if (t->shape.ndims >= 4 && t->shape.dims[1] == 3)
        return 1;
    return 0;
}

/* Copy an RGB HWC image into a model input tensor, respecting the model layout.
 * The converted MTCNN models in this project are [1,3,H,W] NCHW, while the
 * camera preprocessing buffer is HWC RGB. Feeding HWC bytes directly to an
 * NCHW tensor makes the model see scrambled color/spatial data, causing missed
 * detections and false boxes. */
static void copy_rgb_to_model_input(const npu_model_t *m,
                                    const unsigned char *src_hwc,
                                    int width, int height)
{
    unsigned char *dst = (unsigned char *)m->input.tensor[0].addr;
    int x, y, c;

    if (model_input_is_nchw(m)) {
        int plane = width * height;
        for (c = 0; c < 3; c++) {
            for (y = 0; y < height; y++) {
                for (x = 0; x < width; x++) {
                    dst[c * plane + y * width + x] = src_hwc[(y * width + x) * 3 + c];
                }
            }
        }
    } else {
        memcpy(dst, src_hwc, width * height * 3);
    }
}

/*
 * NPU writes output tensors to the output buffer with 8-byte alignment
 * between each tensor. We must calculate the correct offset accordingly.
 */
static unsigned char *get_output_data(const npu_model_t *m, int tensor_idx)
{
    int offset = 0, i;
    for (i = 0; i < tensor_idx; i++)
        offset += ALIGN_UP(m->output.tensor[i].size, 8);
    return (unsigned char *)m->output_buf + offset;
}

static void dequantize_output(const npu_model_t *m, int tensor_idx,
                              float *out, int count)
{
    unsigned char *data = get_output_data(m, tensor_idx);
    float scale = m->output.tensor[tensor_idx].quant.scale;
    int zp = m->output.tensor[tensor_idx].quant.zp;
    int i;
    for (i = 0; i < count; i++)
        out[i] = ((float)data[i] - zp) * scale;
}

/* 2-class score → probability of class 1 (face).
 *
 * IMPORTANT: The quantized MTCNN models already apply softmax internally,
 * so the two dequantized values already sum to ~1.0 (verified empirically
 * from NPU output).  Applying softmax a second time compresses the
 * effective probability range from [0.0, 1.0] to [0.27, 0.73], making
 * the cascade thresholds nearly useless.
 *
 * We just return s1 directly as the face probability.
 */
static float softmax_face_score(float s0, float s1)
{
    (void)s0;
    return s1;
}

/* Calculate IoU of two boxes */
static float calc_iou(const face_box_t *a, const face_box_t *b)
{
    float ix1 = fmaxf(a->x1, b->x1);
    float iy1 = fmaxf(a->y1, b->y1);
    float ix2 = fminf(a->x2, b->x2);
    float iy2 = fminf(a->y2, b->y2);
    float inter = fmaxf(0, ix2 - ix1) * fmaxf(0, iy2 - iy1);
    float area_a = (a->x2 - a->x1) * (a->y2 - a->y1);
    float area_b = (b->x2 - b->x1) * (b->y2 - b->y1);
    return inter / (area_a + area_b - inter + 1e-6f);
}

/* Non-maximum suppression. Returns number of kept boxes. */
static int nms(face_box_t *boxes, int n, float threshold)
{
    int i, j, keep_count = 0;
    int *keep = calloc(n, sizeof(int));

    /* Sort by score descending (simple selection sort) */
    for (i = 0; i < n - 1; i++) {
        int best = i;
        for (j = i + 1; j < n; j++) {
            if (boxes[j].score > boxes[best].score) best = j;
        }
        if (best != i) {
            face_box_t tmp = boxes[i];
            boxes[i] = boxes[best];
            boxes[best] = tmp;
        }
    }

    /* Greedy NMS */
    for (i = 0; i < n; i++) {
        if (keep[i] == -1) continue;
        keep[i] = 1;
        for (j = i + 1; j < n; j++) {
            if (keep[j] == -1) continue;
            if (calc_iou(&boxes[i], &boxes[j]) > threshold)
                keep[j] = -1;
        }
    }

    /* Compact */
    for (i = 0; i < n; i++) {
        if (keep[i] == 1) {
            if (keep_count != i)
                boxes[keep_count] = boxes[i];
            keep_count++;
        }
    }
    free(keep);
    return keep_count;
}

/* ------------------------------------------------------------------ */
/*  Post-detection filtering: shape/size heuristics                    */
/* ------------------------------------------------------------------ */

/* Reject boxes that don't look like faces.
 *  - aspect ratio must be within [FACE_MIN_ASPECT, FACE_MAX_ASPECT]
 *  - both dimensions >= FACE_MIN_SIZE pixels
 *  - area <= FACE_MAX_AREA_RATIO of detection image (reject "huge" boxes)
 * Returns 1 if plausible, 0 to reject.
 */
static int is_face_plausible(const face_box_t *box)
{
    float w = box->x2 - box->x1;
    float h = box->y2 - box->y1;
    float area;
    float ratio;

    if (w <= 0 || h <= 0) return 0;
    if (w < FACE_MIN_SIZE || h < FACE_MIN_SIZE) return 0;

    ratio = w / h;
    if (ratio < FACE_MIN_ASPECT || ratio > FACE_MAX_ASPECT) return 0;

    area = w * h;
    if (area > DET_WIDTH * DET_HEIGHT * FACE_MAX_AREA_RATIO) return 0;

    return 1;
}

/* In-place filter: drop implausible boxes, compact remaining to front.
 * Returns new count. */
static int filter_implausible(face_box_t *boxes, int n)
{
    int i, out = 0;
    for (i = 0; i < n; i++) {
        if (is_face_plausible(&boxes[i])) {
            if (out != i) boxes[out] = boxes[i];
            out++;
        }
    }
    return out;
}

/* ------------------------------------------------------------------ */
/*  Frame-to-frame face tracking / stabilization                      */
/*                                                                     */
/*  Matches new detections to existing tracks by IoU.                  */
/*  New faces require FACE_TRACK_MIN_HITS consecutive frames.          */
/*  Lost faces persist for FACE_TRACK_MAX_MISS frames.                 */
/*  Box positions are smoothed with EMA for stable display.            */
/* ------------------------------------------------------------------ */
static void update_tracks(const face_box_t *detected, int det_count)
{
    int i, j;
    int matched[MAX_FACES];
    int det_matched[MAX_FACES];

    memset(matched, 0, sizeof(matched));
    memset(det_matched, 0, sizeof(det_matched));

    /* --- Match detections to existing tracks (greedy IoU) --- */
    for (i = 0; i < g_track_count; i++) {
        float best_iou = FACE_TRACK_IOU_THR;
        int best_j = -1;
        for (j = 0; j < det_count; j++) {
            float iou;
            if (det_matched[j]) continue;
            iou = calc_iou(&g_tracks[i].raw_box, &detected[j]);
            if (iou > best_iou) {
                best_iou = iou;
                best_j = j;
            }
        }
        if (best_j >= 0) {
            tracked_face_t *t = &g_tracks[i];
            float a = FACE_TRACK_SMOOTH;
            t->raw_box = detected[best_j];
            /* Exponential moving average for smooth display */
            t->box.x1 = t->box.x1 * (1 - a) + detected[best_j].x1 * a;
            t->box.y1 = t->box.y1 * (1 - a) + detected[best_j].y1 * a;
            t->box.x2 = t->box.x2 * (1 - a) + detected[best_j].x2 * a;
            t->box.y2 = t->box.y2 * (1 - a) + detected[best_j].y2 * a;
            t->box.score = detected[best_j].score;
            /* Update landmarks from latest detection (not EMA-smoothed,
             * since absolute landmark positions shift with face movement) */
            memcpy(t->box.landmarks, detected[best_j].landmarks,
                   sizeof(float) * 10);
            t->hit_count++;
            t->miss_count = 0;
            if (t->hit_count >= FACE_TRACK_MIN_HITS)
                t->active = 1;
            matched[i] = 1;
            det_matched[best_j] = 1;
        }
    }

    /* --- Create new tracks for unmatched detections --- */
    for (j = 0; j < det_count && g_track_count < MAX_FACES; j++) {
        if (det_matched[j]) continue;
        tracked_face_t *t = &g_tracks[g_track_count];
        memset(t, 0, sizeof(*t));
        t->raw_box = detected[j];
        t->box = detected[j];
        t->hit_count = 1;
        t->miss_count = 0;
        t->active = 0;
        t->id = g_track_next_id++;
        g_track_count++;
    }

    /* --- Decay unmatched tracks, compact out expired ones --- */
    {
        int write = 0;
        for (i = 0; i < g_track_count; i++) {
            if (!matched[i]) {
                g_tracks[i].miss_count++;
                g_tracks[i].hit_count = 0;
                g_tracks[i].active = 0;
                if (g_tracks[i].miss_count > FACE_TRACK_MAX_MISS)
                    continue;   /* expired — drop */
            }
            if (write != i)
                g_tracks[write] = g_tracks[i];
            write++;
        }
        g_track_count = write;
    }
}

/* Collect active (display-ready) tracked faces into output array.
 * Returns count of active faces. */
static int get_active_faces(face_box_t *out, int max_out)
{
    int i, count = 0;
    for (i = 0; i < g_track_count && count < max_out; i++) {
        if (g_tracks[i].active) {
            out[count] = g_tracks[i].box;
            count++;
        }
    }
    return count;
}

/* Reset all tracking state (e.g., on detection miss) */
static void decay_all_tracks(void)
{
    int i, write = 0;
    for (i = 0; i < g_track_count; i++) {
        g_tracks[i].miss_count++;
        g_tracks[i].active = 0;
        if (g_tracks[i].miss_count <= FACE_TRACK_MAX_MISS) {
            if (write != i) g_tracks[write] = g_tracks[i];
            write++;
        }
    }
    g_track_count = write;
}

/* Apply box regression to an anchor box.
 * reg = [dx1, dy1, dx2, dy2] in the standard MTCNN format:
 *   pred_x1 = anchor_x1 + dx1 * anchor_w
 *   pred_y1 = anchor_y1 + dy1 * anchor_h
 *   pred_x2 = anchor_x2 + dx2 * anchor_w
 *   pred_y2 = anchor_y2 + dy2 * anchor_h
 */
static void bbox_reg(face_box_t *box, const float reg[4])
{
    float w = box->x2 - box->x1;
    float h = box->y2 - box->y1;
    box->x1 += reg[0] * w;
    box->y1 += reg[1] * h;
    box->x2 += reg[2] * w;
    box->y2 += reg[3] * h;
}

/* Crop a region from source RGB image and resize to dst_size × dst_size.
 * Uses bilinear interpolation. Output is HWC RGB. */
static void crop_resize(const unsigned char *src, int src_w, int src_h,
                        const face_box_t *box,
                        unsigned char *dst, int dst_size)
{
    /* Square the box */
    float bw = box->x2 - box->x1;
    float bh = box->y2 - box->y1;
    float side = fmaxf(bw, bh);
    float cx = (box->x1 + box->x2) * 0.5f;
    float cy = (box->y1 + box->y2) * 0.5f;
    float sx1 = cx - side * 0.5f;
    float sy1 = cy - side * 0.5f;
    float sx2 = cx + side * 0.5f;
    float sy2 = cy + side * 0.5f;

    /* Clamp to image bounds */
    if (sx1 < 0) sx1 = 0;
    if (sy1 < 0) sy1 = 0;
    if (sx2 > src_w) sx2 = (float)src_w;
    if (sy2 > src_h) sy2 = (float)src_h;

    float scale_x = (sx2 - sx1) / dst_size;
    float scale_y = (sy2 - sy1) / dst_size;
    int ty, tx, c;

    for (ty = 0; ty < dst_size; ty++) {
        for (tx = 0; tx < dst_size; tx++) {
            float fx = sx1 + (tx + 0.5f) * scale_x;
            float fy = sy1 + (ty + 0.5f) * scale_y;
            int x0 = (int)fx;
            int y0 = (int)fy;
            int x1i = x0 + 1;
            int y1i = y0 + 1;
            float xf = fx - x0;
            float yf = fy - y0;

            /* Clamp coordinates */
            if (x0 < 0) x0 = 0; else if (x0 >= src_w) x0 = src_w - 1;
            if (x1i < 0) x1i = 0; else if (x1i >= src_w) x1i = src_w - 1;
            if (y0 < 0) y0 = 0; else if (y0 >= src_h) y0 = src_h - 1;
            if (y1i < 0) y1i = 0; else if (y1i >= src_h) y1i = src_h - 1;

            for (c = 0; c < 3; c++) {
                float v = (1 - xf) * (1 - yf) * src[(y0 * src_w + x0) * 3 + c]
                        + xf * (1 - yf)       * src[(y0 * src_w + x1i) * 3 + c]
                        + (1 - xf) * yf       * src[(y1i * src_w + x0) * 3 + c]
                        + xf * yf             * src[(y1i * src_w + x1i) * 3 + c];
                dst[(ty * dst_size + tx) * 3 + c] =
                    (unsigned char)(v < 0 ? 0 : v > 255 ? 255 : v);
            }
        }
    }
}

/* Bilinear resize of full RGB image (HWC) */
static void resize_rgb(const unsigned char *src, int src_w, int src_h,
                       unsigned char *dst, int dst_w, int dst_h)
{
    float scale_x = (float)src_w / dst_w;
    float scale_y = (float)src_h / dst_h;
    int ty, tx, c;

    for (ty = 0; ty < dst_h; ty++) {
        for (tx = 0; tx < dst_w; tx++) {
            float fx = (tx + 0.5f) * scale_x - 0.5f;
            float fy = (ty + 0.5f) * scale_y - 0.5f;
            int x0 = (int)fx;
            int y0 = (int)fy;
            int x1i = x0 + 1;
            int y1i = y0 + 1;
            float xf = fx - x0;
            float yf = fy - y0;

            if (x0 < 0) x0 = 0; else if (x0 >= src_w) x0 = src_w - 1;
            if (x1i < 0) x1i = 0; else if (x1i >= src_w) x1i = src_w - 1;
            if (y0 < 0) y0 = 0; else if (y0 >= src_h) y0 = src_h - 1;
            if (y1i < 0) y1i = 0; else if (y1i >= src_h) y1i = src_h - 1;

            for (c = 0; c < 3; c++) {
                float v = (1 - xf) * (1 - yf) * src[(y0 * src_w + x0) * 3 + c]
                        + xf * (1 - yf)       * src[(y0 * src_w + x1i) * 3 + c]
                        + (1 - xf) * yf       * src[(y1i * src_w + x0) * 3 + c]
                        + xf * yf             * src[(y1i * src_w + x1i) * 3 + c];
                dst[(ty * dst_w + tx) * 3 + c] =
                    (unsigned char)(v < 0 ? 0 : v > 255 ? 255 : v);
            }
        }
    }
}

/* Find output tensor index by number of channels (last or 2nd dim) */
static int find_output_by_ch(npu_model_t *m, int target_ch)
{
    int i;
    for (i = 0; i < (int)m->output.num; i++) {
        xmedia_cl_tensor *t = &m->output.tensor[i];
        int ndims = (int)t->shape.ndims;
        if (ndims >= 2) {
            /* Check both NCHW (dim[1]) and NHWC (dim[last]) */
            if ((int)t->shape.dims[1] == target_ch) return i;
            if ((int)t->shape.dims[ndims - 1] == target_ch) return i;
        }
        /* For 2D output [1, C] */
        if (ndims == 2 && (int)t->shape.dims[1] == target_ch) return i;
    }
    return -1;
}

/* ------------------------------------------------------------------ */
/*  MTCNN pipeline                                                     */
/* ------------------------------------------------------------------ */

/*
 * P-Net: image pyramid + 12×12 sliding window.
 * For each scale, resize image and slide 12x12 patches through the model.
 * Returns: number of face candidates.
 */
static int pnet_detect(const unsigned char *rgb_img, face_box_t *candidates)
{
    xmedia_cl_s32 ret;
    int score_idx, box_idx;
    int count = 0;
    float scale;
    unsigned char *scaled;
    struct timespec t0, t1;
    float pnet_ms = 0;

    score_idx = find_output_by_ch(&g_pnet, 2);
    box_idx = find_output_by_ch(&g_pnet, 4);
    if (score_idx < 0 || box_idx < 0) {
        printf("[mtcnn] pnet: cannot identify output tensors (score=%d, box=%d)\n",
               score_idx, box_idx);
        return 0;
    }

    /* Pre-allocate max-size buffer (320*240*3) to avoid per-scale malloc */
    scaled = (unsigned char *)malloc(DET_WIDTH * DET_HEIGHT * 3);
    if (!scaled) return 0;

    /* Build image pyramid: start from 12/min_face, shrink by factor */
    scale = (float)PNET_PATCH_SIZE / PNET_MIN_FACE_SIZE;

    while (count < MAX_CANDIDATES) {
        int sw = (int)(DET_WIDTH * scale);
        int sh = (int)(DET_HEIGHT * scale);
        int i, j, patch_count = 0;

        /* Stop if scaled image is smaller than a single patch */
        if (sw < PNET_PATCH_SIZE || sh < PNET_PATCH_SIZE) break;

        resize_rgb(rgb_img, DET_WIDTH, DET_HEIGHT, scaled, sw, sh);

        clock_gettime(CLOCK_MONOTONIC, &t0);

        /* Slide 12x12 window across the scaled image */
        for (i = 0; i <= sh - PNET_PATCH_SIZE && count < MAX_CANDIDATES;
             i += PNET_STRIDE) {
            for (j = 0; j <= sw - PNET_PATCH_SIZE && count < MAX_CANDIDATES;
                 j += PNET_STRIDE) {
                float s_vals[2] = {0}, b_vals[4] = {0};
                float prob;
                int py, px;

                /* Copy 12x12 RGB patch into P-Net input, respecting NCHW/NHWC layout. */
                {
                    unsigned char patch[PNET_PATCH_SIZE * PNET_PATCH_SIZE * 3];
                    for (py = 0; py < PNET_PATCH_SIZE; py++) {
                        for (px = 0; px < PNET_PATCH_SIZE; px++) {
                            int src = ((i + py) * sw + (j + px)) * 3;
                            int dst = (py * PNET_PATCH_SIZE + px) * 3;
                            patch[dst + 0] = scaled[src + 0]; /* R */
                            patch[dst + 1] = scaled[src + 1]; /* G */
                            patch[dst + 2] = scaled[src + 2]; /* B */
                        }
                    }
                    copy_rgb_to_model_input(&g_pnet, patch, PNET_PATCH_SIZE, PNET_PATCH_SIZE);
                }

                /* Run P-Net inference */
                ret = xmedia_cl_graph_process(g_pnet.graph);
                if (ret != XMEDIA_CL_SUCCESS) continue;

                /* Parse score & box outputs (read from contiguous layout) */
                dequantize_output(&g_pnet, score_idx, s_vals, 2);
                dequantize_output(&g_pnet, box_idx, b_vals, 4);

                prob = softmax_face_score(s_vals[0], s_vals[1]);
                if (prob < FACE_CONF_THRESHOLD) continue;

                /* Map patch coordinates back to original image */
                {
                    float inv_s = 1.0f / scale;
                    candidates[count].x1 = j * inv_s;
                    candidates[count].y1 = i * inv_s;
                    candidates[count].x2 = (j + PNET_PATCH_SIZE) * inv_s;
                    candidates[count].y2 = (i + PNET_PATCH_SIZE) * inv_s;
                    candidates[count].score = prob;
                    memset(candidates[count].landmarks, 0,
                           sizeof(float) * 10);

                    /* Apply box regression */
                    bbox_reg(&candidates[count], b_vals);

                    /* Clamp to detection image bounds */
                    if (candidates[count].x1 < 0)
                        candidates[count].x1 = 0;
                    if (candidates[count].y1 < 0)
                        candidates[count].y1 = 0;
                    if (candidates[count].x2 > DET_WIDTH)
                        candidates[count].x2 = DET_WIDTH;
                    if (candidates[count].y2 > DET_HEIGHT)
                        candidates[count].y2 = DET_HEIGHT;

                    count++;
                    patch_count++;
                }
            }
        }

        clock_gettime(CLOCK_MONOTONIC, &t1);
        pnet_ms += (t1.tv_sec - t0.tv_sec) * 1000.0f +
                   (t1.tv_nsec - t0.tv_nsec) / 1e6f;

        printf("[dbg] pnet scale=%.3f (%dx%d) patches=%d candidates=%d\n",
               scale, sw, sh,
               ((sw - PNET_PATCH_SIZE) / PNET_STRIDE + 1) *
               ((sh - PNET_PATCH_SIZE) / PNET_STRIDE + 1),
               patch_count);

        scale *= PNET_SCALE_FACTOR;
    }

    free(scaled);

    printf("[dbg] pnet total: %d candidates, %.1f ms\n", count, pnet_ms);

    /* NMS */
    count = nms(candidates, count, FACE_NMS_THRESHOLD);
    return count;
}

/*
 * R-Net: per-candidate refinement.
 * Input: 320×240 RGB image + candidates from P-Net.
 * Returns: number of surviving candidates (updated in-place).
 */
static int rnet_filter(const unsigned char *rgb_img, face_box_t *cands, int n)
{
    xmedia_cl_s32 ret;
    int score_idx, box_idx;
    int i, out_count = 0;

    score_idx = find_output_by_ch(&g_rnet, 2);
    box_idx = find_output_by_ch(&g_rnet, 4);
    if (score_idx < 0 || box_idx < 0) return 0;

    /* Read actual model input size from tensor shape */
    {
        xmedia_cl_tensor *t = &g_rnet.input.tensor[0];
        int rsz = model_input_is_nchw(&g_rnet) ? t->shape.dims[2] : t->shape.dims[1];

        for (i = 0; i < n && out_count < MAX_CANDIDATES; i++) {
            float prob;

            /* Crop and resize candidate region to model input size HWC RGB, then copy to the model layout. */
            {
                unsigned char crop[48 * 48 * 3];
                crop_resize(rgb_img, DET_WIDTH, DET_HEIGHT, &cands[i], crop, rsz);
                copy_rgb_to_model_input(&g_rnet, crop, rsz, rsz);
            }

            /* Run R-Net inference */
            ret = xmedia_cl_graph_process(g_rnet.graph);
            if (ret != XMEDIA_CL_SUCCESS) continue;

            /* Parse output */
            {
                float s_vals[2] = {0}, b_vals[4] = {0};
                dequantize_output(&g_rnet, score_idx, s_vals, 2);
                dequantize_output(&g_rnet, box_idx, b_vals, 4);

                prob = softmax_face_score(s_vals[0], s_vals[1]);

                /* Debug: print first 5 candidates */
                if (out_count < 3 || (prob >= RNET_CONF_THRESHOLD && out_count < 10)) {
                    printf("[dbg] rnet[%d]: s=[%.3f,%.3f] prob=%.3f box=[%.3f,%.3f,%.3f,%.3f] "
                           "cand=[%.1f,%.1f,%.1f,%.1f] scale=%.6f zp=%d\n",
                           i, s_vals[0], s_vals[1], prob,
                           b_vals[0], b_vals[1], b_vals[2], b_vals[3],
                           cands[i].x1, cands[i].y1, cands[i].x2, cands[i].y2,
                           g_rnet.output.tensor[score_idx].quant.scale,
                           g_rnet.output.tensor[score_idx].quant.zp);
                }

                if (prob < RNET_CONF_THRESHOLD) continue;

                cands[out_count] = cands[i];
                cands[out_count].score = prob;

                /* Apply box regression */
                bbox_reg(&cands[out_count], b_vals);

                /* Clamp */
                if (cands[out_count].x1 < 0) cands[out_count].x1 = 0;
                if (cands[out_count].y1 < 0) cands[out_count].y1 = 0;
                if (cands[out_count].x2 > DET_WIDTH) cands[out_count].x2 = DET_WIDTH;
                if (cands[out_count].y2 > DET_HEIGHT) cands[out_count].y2 = DET_HEIGHT;

                out_count++;
            }
        }
    }

    out_count = nms(cands, out_count, FACE_NMS_THRESHOLD);
    printf("[dbg] rnet result: %d in, %d out (thr=%.2f)\n", n, out_count, RNET_CONF_THRESHOLD);
    return out_count;
}

/*
 * O-Net: per-candidate final output with landmarks.
 * Returns: number of final face detections.
 */
static int
onet_filter(const unsigned char *rgb_img, face_box_t *cands, int n)
{
    xmedia_cl_s32 ret;
    int score_idx, box_idx, lm_idx;
    int i, out_count = 0;

    score_idx = find_output_by_ch(&g_onet, 2);
    box_idx = find_output_by_ch(&g_onet, 4);
    lm_idx = find_output_by_ch(&g_onet, 10);
    if (score_idx < 0 || box_idx < 0) {
        printf("[mtcnn] onet: cannot find output tensors\n");
        return 0;
    }

    /* Read actual model input size from tensor shape */
    {
        xmedia_cl_tensor *t = &g_onet.input.tensor[0];
        int osz = model_input_is_nchw(&g_onet) ? t->shape.dims[2] : t->shape.dims[1];

        for (i = 0; i < n && out_count < MAX_FACES; i++) {
            /* Crop and resize to model input size HWC RGB, then copy to the model layout. */
            {
                unsigned char crop[48 * 48 * 3];
                crop_resize(rgb_img, DET_WIDTH, DET_HEIGHT, &cands[i], crop, osz);
                copy_rgb_to_model_input(&g_onet, crop, osz, osz);
            }

            /* Run O-Net inference */
            ret = xmedia_cl_graph_process(g_onet.graph);
            if (ret != XMEDIA_CL_SUCCESS) continue;

            /* Parse output */
            {
                float s_vals[2] = {0}, b_vals[4] = {0}, l_vals[10] = {0};
                dequantize_output(&g_onet, score_idx, s_vals, 2);
                dequantize_output(&g_onet, box_idx, b_vals, 4);
                if (lm_idx >= 0)
                    dequantize_output(&g_onet, lm_idx, l_vals, 10);

                float prob = softmax_face_score(s_vals[0], s_vals[1]);

                /* Debug: always print O-Net results for the first few candidates */
                if (out_count == 0 && lm_idx >= 0) {
                    printf("[onet] raw landmarks: ");
                    for (int k = 0; k < 10; k++) printf("%.3f ", l_vals[k]);
                    printf("\n");
                }

                if (i < 5 || prob >= ONET_CONF_THRESHOLD) {
                    printf("[dbg] onet[%d]: s=[%.3f,%.3f] prob=%.3f box=[%.3f,%.3f,%.3f,%.3f] "
                           "cand=[%.1f,%.1f,%.1f,%.1f]\n",
                           i, s_vals[0], s_vals[1], prob,
                           b_vals[0], b_vals[1], b_vals[2], b_vals[3],
                           cands[i].x1, cands[i].y1, cands[i].x2, cands[i].y2);
                }

                if (prob < ONET_CONF_THRESHOLD) continue;

                cands[out_count] = cands[i];
                cands[out_count].score = prob;

                /* Apply box regression */
                bbox_reg(&cands[out_count], b_vals);

                /* Clamp box */
                if (cands[out_count].x1 < 0) cands[out_count].x1 = 0;
                if (cands[out_count].y1 < 0) cands[out_count].y1 = 0;
                if (cands[out_count].x2 > DET_WIDTH) cands[out_count].x2 = DET_WIDTH;
                if (cands[out_count].y2 > DET_HEIGHT) cands[out_count].y2 = DET_HEIGHT;

                /* Store landmarks.
                 * Model output is PLANAR: lv[0..4]=X for 5 points, lv[5..9]=Y for 5 points.
                 * Standard MTCNN order: LE, RE, N, ML, MR */
                if (lm_idx >= 0) {
                    float bw = cands[out_count].x2 - cands[out_count].x1;
                    float bh = cands[out_count].y2 - cands[out_count].y1;
                    for (int k = 0; k < 5; k++) {
                        cands[out_count].landmarks[k * 2] =
                            cands[out_count].x1 + l_vals[k] * bw;
                        cands[out_count].landmarks[k * 2 + 1] =
                            cands[out_count].y1 + l_vals[5 + k] * bh;
                    }
                } else {
                    memset(cands[out_count].landmarks, 0, sizeof(float) * 10);
                }

                out_count++;
            }
        }
    }

    out_count = nms(cands, out_count, FACE_NMS_THRESHOLD);
    return out_count;
}

/* ------------------------------------------------------------------ */
/*  NPU inference thread                                               */
/* ------------------------------------------------------------------ */
static void *npu_inference_thread(void *arg)
{
    xmedia_s32 ret;
    xmedia_video_frame_info npu_frame;
    xmedia_s32 milli_sec = 2000;
    int fps_counter = 0;
    struct timespec ts_start, ts_now;
    face_box_t candidates[MAX_CANDIDATES];
    unsigned char *rgb_buf = NULL;

    (void)arg;
    printf("[mtcnn] Inference thread started\n");

    /* Allocate CPU-side RGB buffer for detection frame (for crop_resize reads) */
    rgb_buf = (unsigned char *)malloc(DET_WIDTH * DET_HEIGHT * 3);
    if (!rgb_buf) {
        printf("[mtcnn] Failed to alloc RGB buffer\n");
        return NULL;
    }

    clock_gettime(CLOCK_MONOTONIC, &ts_start);

    while (g_npu_running && !g_stop) {
        /* 1. Acquire 320×240 frame from VPSS */
        memset(&npu_frame, 0, sizeof(npu_frame));
        ret = xmedia_vpss_acquire_ochn_frame(
            FACE_VPSS_PIPE, FACE_VPSS_OCHN_NPU, &npu_frame, milli_sec);
        if (ret != XMEDIA_SUCCESS) continue;

        /* 2. YUV420SP → RGB888 into CPU buffer */
        {
            int y_stride = npu_frame.frame.stride.y_stride ? npu_frame.frame.stride.y_stride : DET_WIDTH;
            int uv_stride = npu_frame.frame.stride.c_stride ? npu_frame.frame.stride.c_stride : DET_WIDTH;

            /* NOTE: c_head_phy_addr is incorrectly equal to y_head_phy_addr in this
             * SDK version — use c_phy_addr for the actual UV plane. */
            xmedia_u32 y_map_size = y_stride * DET_HEIGHT;
            xmedia_u32 c_map_size = uv_stride * (DET_HEIGHT / 2);
            void *frame_y = xmedia_mmz_map(
                npu_frame.frame.addr.y_phy_addr, y_map_size, XMEDIA_FALSE);
            void *frame_uv = xmedia_mmz_map(
                npu_frame.frame.addr.c_phy_addr, c_map_size, XMEDIA_FALSE);

            if (frame_y && frame_uv) {
                /* Convert to CPU RGB buffer (model expects RGB uint8) */
                yuv420sp_to_rgb888(frame_y, frame_uv,
                    rgb_buf, DET_WIDTH, DET_HEIGHT, y_stride, uv_stride);

                /* Debug: dump the first frame to verify what the model sees */
                {
                    static int s_dumped = 0;
                    if (!s_dumped) {
                        /* Print pixel sample: center of image */
                        int cx = DET_WIDTH/2, cy = DET_HEIGHT/2;
                        int sample_y = ((unsigned char*)frame_y)[cy * y_stride + cx];
                        int sample_v = ((unsigned char*)frame_uv)[(cy>>1) * uv_stride + (cx & ~1)];
                        int sample_u = ((unsigned char*)frame_uv)[(cy>>1) * uv_stride + (cx & ~1) + 1];
                        printf("[diag] center pixel: Y=%d U=%d V=%d → R=%d G=%d B=%d\n",
                               sample_y, sample_u, sample_v,
                               rgb_buf[(cy*DET_WIDTH+cx)*3],
                               rgb_buf[(cy*DET_WIDTH+cx)*3+1],
                               rgb_buf[(cy*DET_WIDTH+cx)*3+2]);

                        FILE *fp = fopen("frame_dump.rgb", "wb");
                        if (fp) {
                            fwrite(rgb_buf, 1, DET_WIDTH * DET_HEIGHT * 3, fp);
                            fclose(fp);
                            printf("[dump] saved frame_dump.rgb (%dx%d)\n",
                                   DET_WIDTH, DET_HEIGHT);
                        }
                        s_dumped = 1;
                    }
                }
            } else {
                printf("[mtcnn] mmz map failed\n");
            }
            if (frame_y) xmedia_mmz_unmap(frame_y);
            if (frame_uv) xmedia_mmz_unmap(frame_uv);
        }

        /* 3. Run MTCNN pipeline (3-stage cascade + filter + tracking) */
        {
            int pnet_count, rnet_count, onet_count;
            struct timespec ts_p0, ts_p1;
            float ms_total;

            clock_gettime(CLOCK_MONOTONIC, &ts_p0);

            /* Stage 1: P-Net (pyramid sliding window) */
            pnet_count = pnet_detect(rgb_buf, candidates);

            if (pnet_count == 0) {
                clock_gettime(CLOCK_MONOTONIC, &ts_p1);
                ms_total = (ts_p1.tv_sec - ts_p0.tv_sec) * 1000.0f +
                           (ts_p1.tv_nsec - ts_p0.tv_nsec) / 1e6f;
                printf("[mtcnn] no face detected (%.1f ms)\n", ms_total);

                /* Decay existing tracks */
                pthread_mutex_lock(&g_face_mutex);
                decay_all_tracks();
                {
                    int n = get_active_faces(g_faces, MAX_FACES);
                    g_face_count = n;
                }
                pthread_mutex_unlock(&g_face_mutex);
                xmedia_vpss_release_ochn_frame(
                    FACE_VPSS_PIPE, FACE_VPSS_OCHN_NPU, &npu_frame);
                goto fps_update;
            }

            /* Stage 2: R-Net (per candidate) */
            rnet_count = rnet_filter(rgb_buf, candidates, pnet_count);
            if (rnet_count == 0) {
                pthread_mutex_lock(&g_face_mutex);
                decay_all_tracks();
                {
                    int n = get_active_faces(g_faces, MAX_FACES);
                    g_face_count = n;
                }
                pthread_mutex_unlock(&g_face_mutex);
                xmedia_vpss_release_ochn_frame(
                    FACE_VPSS_PIPE, FACE_VPSS_OCHN_NPU, &npu_frame);
                goto fps_update;
            }

            /* Stage 3: O-Net — final refine + 5 facial landmarks */
            {
                int filtered;
                int onet_out = onet_filter(rgb_buf, candidates, rnet_count);

                if (onet_out == 0) {
                    pthread_mutex_lock(&g_face_mutex);
                    decay_all_tracks();
                    {
                        int n = get_active_faces(g_faces, MAX_FACES);
                        g_face_count = n;
                    }
                    pthread_mutex_unlock(&g_face_mutex);
                    xmedia_vpss_release_ochn_frame(
                        FACE_VPSS_PIPE, FACE_VPSS_OCHN_NPU, &npu_frame);
                    goto fps_update;
                }

                /* Post-filter: reject non-face-like boxes (shape/size) */
                filtered = filter_implausible(candidates, onet_out);

                clock_gettime(CLOCK_MONOTONIC, &ts_p1);
                ms_total = (ts_p1.tv_sec - ts_p0.tv_sec) * 1000.0f +
                           (ts_p1.tv_nsec - ts_p0.tv_nsec) / 1e6f;

                if (filtered > 0) {
                    printf("[mtcnn] pipeline: pnet=%d rnet=%d onet=%d faces=%d (%.1f ms)\n",
                           pnet_count, rnet_count, onet_out, filtered, ms_total);
                } else {
                    printf("[mtcnn] pipeline: pnet=%d rnet=%d onet=%d faces=%d — filtered (%.1f ms)\n",
                           pnet_count, rnet_count, onet_out, filtered, ms_total);
                }
                onet_count = filtered;
            }

            /* Update tracking state */
            pthread_mutex_lock(&g_face_mutex);
            if (onet_count > 0) {
                update_tracks(candidates, onet_count);
            } else {
                decay_all_tracks();
            }
            {
                int n = get_active_faces(g_faces, MAX_FACES);
                g_face_count = n;
            }
            pthread_mutex_unlock(&g_face_mutex);
        }

        /* 4. Release frame */
        xmedia_vpss_release_ochn_frame(
            FACE_VPSS_PIPE, FACE_VPSS_OCHN_NPU, &npu_frame);

fps_update:
        /* FPS tracking */
        fps_counter++;
        clock_gettime(CLOCK_MONOTONIC, &ts_now);
        {
            float elapsed = (ts_now.tv_sec - ts_start.tv_sec) +
                            (ts_now.tv_nsec - ts_start.tv_nsec) / 1e9f;
            if (elapsed >= 1.0f) {
                g_npu_fps = fps_counter;
                fps_counter = 0;
                ts_start = ts_now;
            }
        }

        /* Throttle (~20fps max) */
        usleep(50000);
    }

    free(rgb_buf);
    printf("[mtcnn] Inference thread stopped\n");
    return NULL;
}

/* ------------------------------------------------------------------ */
/*  MJPEG streaming handler                                            */
/* ------------------------------------------------------------------ */
static volatile int g_mjpeg_stop = 0;

static void mjpeg_request_stop(void) { g_mjpeg_stop = 1; }

static void mjpeg_send_stream(int client_fd)
{
    char header[256];
    xmedia_s32 ret;
    xmedia_s32 venc_chn = 0;
    xmedia_u32 venc_mask = 1U << venc_chn;
    struct timeval timeout_val;

    snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n"
        "Cache-Control: no-store\r\n"
        "Pragma: no-cache\r\n"
        "Connection: close\r\n\r\n");
    send(client_fd, header, strlen(header), 0);

    g_mjpeg_stop = 0;
    while (!g_mjpeg_stop && !g_stop) {
        timeout_val.tv_sec = 2;
        timeout_val.tv_usec = 0;
        ret = xmedia_venc_select(venc_mask, &timeout_val);
        if (ret == XMEDIA_ERRCODE_TIMEOUT) continue;
        if (ret != XMEDIA_SUCCESS) break;

        xmedia_venc_chn_status stat;
        memset(&stat, 0, sizeof(stat));
        ret = xmedia_venc_query_status(venc_chn, &stat);
        if (ret != XMEDIA_SUCCESS || stat.cur_packs == 0) continue;

        xmedia_venc_stream stream;
        memset(&stream, 0, sizeof(stream));
        stream.pack = (xmedia_venc_pack *)malloc(sizeof(xmedia_venc_pack) * stat.cur_packs);
        if (stream.pack == NULL) break;
        stream.pack_count = stat.cur_packs;

        ret = xmedia_venc_get_stream(venc_chn, &stream, -1);
        if (ret != XMEDIA_SUCCESS) { free(stream.pack); break; }

        size_t frame_len = 0;
        xmedia_u32 i;
        for (i = 0; i < stream.pack_count; i++)
            frame_len += stream.pack[i].len - stream.pack[i].offset;

        snprintf(header, sizeof(header),
            "--frame\r\nContent-Type: image/jpeg\r\nContent-Length: %zu\r\n\r\n",
            frame_len);
        send(client_fd, header, strlen(header), 0);

        for (i = 0; i < stream.pack_count; i++) {
            send(client_fd,
                 stream.pack[i].vir_addr + stream.pack[i].offset,
                 stream.pack[i].len - stream.pack[i].offset, 0);
        }
        send(client_fd, "\r\n", 2, 0);

        xmedia_venc_release_stream(venc_chn, &stream);
        free(stream.pack);
    }
}

/* ------------------------------------------------------------------ */
/*  System initialization / deinitialization                           */
/* ------------------------------------------------------------------ */
static void deinit_system(void)
{
    g_npu_running = XMEDIA_FALSE;
    if (g_npu_thread) pthread_join(g_npu_thread, NULL);
    deinit_npu();

    if (g_vpss_venc_bound == XMEDIA_TRUE) {
        sample_comm_sys_vpss_unbind_venc(FACE_VPSS_PIPE, FACE_VPSS_OCHN_ENC, 0);
        g_vpss_venc_bound = XMEDIA_FALSE;
    }
    if (g_vi_vpss_bound == XMEDIA_TRUE) {
        sample_comm_sys_vi_unbind_vpss(0, 0, FACE_VPSS_PIPE, 0);
        g_vi_vpss_bound = XMEDIA_FALSE;
    }
    if (g_venc_started == XMEDIA_TRUE) {
        sample_comm_venc_stop(&g_venc_cfg);
        g_venc_started = XMEDIA_FALSE;
    }
    if (g_vpss_started == XMEDIA_TRUE) {
        sample_comm_vpss_stop(&g_vpss_cfg);
        g_vpss_started = XMEDIA_FALSE;
    }
    if (g_vpss_module_inited == XMEDIA_TRUE) {
        sample_comm_vpss_exit();
        g_vpss_module_inited = XMEDIA_FALSE;
    }
    if (g_isp_started == XMEDIA_TRUE) {
        sample_comm_isp_stop(&g_isp_param);
        g_isp_started = XMEDIA_FALSE;
    }
    if (g_vi_started == XMEDIA_TRUE) {
        sample_comm_vi_stop(&g_vi_cfg);
        g_vi_started = XMEDIA_FALSE;
    }
    if (g_isp_inited == XMEDIA_TRUE) {
        sample_comm_isp_exit(&g_isp_param);
        g_isp_inited = XMEDIA_FALSE;
    }
    if (g_venc_module_inited == XMEDIA_TRUE) {
        sample_comm_venc_exit();
        g_venc_module_inited = XMEDIA_FALSE;
    }
    if (g_vi_module_inited == XMEDIA_TRUE) {
        sample_comm_vi_exit();
        g_vi_module_inited = XMEDIA_FALSE;
    }
    sample_comm_sys_exit();
}

static int init_system(void)
{
    xmedia_s32 ret;
    xmedia_s32 vi_dev = 0, vi_pipe = 0, vi_chn = 0;
    sample_comm_sensor_type sensor_type = SENSOR0_TYPE;
    xmedia_u32 framerate = 0;
    xmedia_isp_config isp_config = {0};
    xmedia_video_size pic_size = {0};
    xmedia_s32 blk_size;
    xmedia_video_size npu_size;

    memset(&g_video_param, 0, sizeof(g_video_param));
    g_video_param.video_fmt = XMEDIA_VIDEO_FMT_LINEAR;
    g_video_param.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420;
    g_video_param.data_width = XMEDIA_VIDEO_DATA_WIDTH_8;
    g_video_param.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;

    sample_comm_vi_get_sensor_info(sensor_type, &g_sensor_info);
    sample_comm_vi_get_framerate_by_sensor(sensor_type, &framerate);
    printf("[init] sensor: %ux%u @ %ufps\n",
           g_sensor_info.width, g_sensor_info.height, framerate);

    /* --- Sys init --- */
    memset(&g_sys_cfg, 0, sizeof(g_sys_cfg));
    g_sys_cfg.sys_conf.pipe_mode[vi_pipe].vicap_viproc_mode = XMEDIA_WORK_MODE_OFFLINE;
    g_sys_cfg.sys_conf.pipe_mode[vi_pipe].viproc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;
    g_sys_cfg.sys_conf.pipe_mode[vi_pipe].gdc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;
    g_sys_cfg.vb_conf.max_pool_cnt = 25;

    pic_size.width = g_sensor_info.width;
    pic_size.height = g_sensor_info.height;

    /* Pool 0: VI capture buffer */
    blk_size = sample_comm_sys_get_buffer_size(pic_size, g_video_param.video_fmt,
        g_sensor_info.pixel_format, g_sensor_info.bit_width, g_video_param.compress_mode);
    g_sys_cfg.vb_conf.common_pool[0].block_size = blk_size;
    g_sys_cfg.vb_conf.common_pool[0].block_cnt = 2;

    /* Pool 1: VPSS full res / VENC */
    blk_size = sample_comm_sys_get_buffer_size(pic_size, g_video_param.video_fmt,
        g_video_param.pixel_fmt, g_video_param.data_width, g_video_param.compress_mode);
    g_sys_cfg.vb_conf.common_pool[1].block_size = blk_size;
    g_sys_cfg.vb_conf.common_pool[1].block_cnt = 5;

    /* Pool 2: VPSS 320×240 NPU input */
    npu_size.width = DET_WIDTH;
    npu_size.height = DET_HEIGHT;
    blk_size = sample_comm_sys_get_buffer_size(npu_size, g_video_param.video_fmt,
        g_video_param.pixel_fmt, g_video_param.data_width, g_video_param.compress_mode);
    g_sys_cfg.vb_conf.common_pool[2].block_size = blk_size;
    g_sys_cfg.vb_conf.common_pool[2].block_cnt = 3;

    g_sys_cfg.vb_conf.supplement_config = 1;

    ret = sample_comm_sys_init(&g_sys_cfg);
    if (ret != XMEDIA_SUCCESS) { printf("[init] sys_init failed: %d\n", ret); return ret; }

    /* VI */
    ret = sample_comm_vi_init();
    if (ret != XMEDIA_SUCCESS) { deinit_system(); return ret; }
    g_vi_module_inited = XMEDIA_TRUE;

    /* VPSS */
    ret = sample_comm_vpss_init();
    if (ret != XMEDIA_SUCCESS) { deinit_system(); return ret; }
    g_vpss_module_inited = XMEDIA_TRUE;

    /* VENC */
    ret = sample_comm_venc_init();
    if (ret != XMEDIA_SUCCESS) { deinit_system(); return ret; }
    g_venc_module_inited = XMEDIA_TRUE;

    /* VI config */
    memset(&g_vi_cfg, 0, sizeof(g_vi_cfg));
    g_vi_cfg.dev_info[vi_dev].dev_en = XMEDIA_TRUE;
    g_vi_cfg.dev_info[vi_dev].dev_no = vi_dev;
    g_vi_cfg.dev_info[vi_dev].sensor_type = sensor_type;
    g_vi_cfg.pipe_info[vi_pipe].pipe_en = XMEDIA_TRUE;
    g_vi_cfg.pipe_info[vi_pipe].pipe_no = vi_pipe;
    g_vi_cfg.pipe_info[vi_pipe].chn_info[vi_chn].chn_en = XMEDIA_TRUE;
    g_vi_cfg.pipe_info[vi_pipe].chn_info[vi_chn].chn_no = vi_chn;
    g_vi_cfg.dev_bind_pipe[vi_dev].pipe[0] = vi_pipe;
    g_vi_cfg.dev_bind_pipe[vi_dev].pipe[1] = -1;

    /* ISP config */
    memset(&g_isp_param, 0, sizeof(g_isp_param));
    isp_config.fps = framerate;
    isp_config.mode_config.work_mode = XMEDIA_ISP_WORK_MODE_MASTER;
    isp_config.mode_config.master_mode.blend_stat_enable = XMEDIA_FALSE;
    isp_config.mode_config.master_mode.slave_num = 0;
    isp_config.pixel_fmt = g_sensor_info.pixel_format;
    isp_config.size.width = g_sensor_info.width;
    isp_config.size.height = g_sensor_info.height;
    isp_config.wdr_mode = g_sensor_info.wdr_mode;
    g_isp_param.pipe[vi_pipe] = vi_pipe;
    g_isp_param.isp_info[vi_pipe].isp_pipe_en = XMEDIA_TRUE;
    g_isp_param.isp_info[vi_pipe].isp_sensor_en = XMEDIA_TRUE;
    g_isp_param.isp_info[vi_pipe].sensor_type = sensor_type;
    g_isp_param.isp_info[vi_pipe].mirror = XMEDIA_FALSE;
    g_isp_param.isp_info[vi_pipe].flip = XMEDIA_FALSE;
    memcpy(&g_isp_param.isp_info[vi_pipe].isp_config, &isp_config, sizeof(xmedia_isp_config));

    ret = sample_comm_isp_init(&g_isp_param, &g_vi_cfg);
    if (ret != XMEDIA_SUCCESS) { deinit_system(); return ret; }
    g_isp_inited = XMEDIA_TRUE;

    ret = sample_comm_vi_start(&g_vi_cfg, &g_video_param);
    if (ret != XMEDIA_SUCCESS) { deinit_system(); return ret; }
    g_vi_started = XMEDIA_TRUE;

    ret = sample_comm_isp_start(&g_isp_param);
    if (ret != XMEDIA_SUCCESS) { deinit_system(); return ret; }
    g_isp_started = XMEDIA_TRUE;

    /* VPSS config: 2 output channels */
    memset(&g_vpss_cfg, 0, sizeof(g_vpss_cfg));
    g_vpss_cfg.pipe_info[FACE_VPSS_PIPE].pipe_en = XMEDIA_TRUE;
    g_vpss_cfg.pipe_info[FACE_VPSS_PIPE].pipe_no = FACE_VPSS_PIPE;

    ret = sample_comm_vpss_get_default_pipe_cfg(
        &g_vpss_cfg.pipe_info[FACE_VPSS_PIPE].pipe_config, pic_size, &g_video_param);
    if (ret != XMEDIA_SUCCESS) { deinit_system(); return ret; }

    /* VPSS ochn0: full resolution → VENC */
    g_vpss_cfg.pipe_info[FACE_VPSS_PIPE].chn_info[FACE_VPSS_OCHN_ENC].chn_en = XMEDIA_TRUE;
    g_vpss_cfg.pipe_info[FACE_VPSS_PIPE].chn_info[FACE_VPSS_OCHN_ENC].chn_no = FACE_VPSS_OCHN_ENC;
    ret = sample_comm_vpss_get_default_ochn_cfg(
        &g_vpss_cfg.pipe_info[FACE_VPSS_PIPE].chn_info[FACE_VPSS_OCHN_ENC].chn_config,
        pic_size, &g_video_param);
    if (ret != XMEDIA_SUCCESS) { deinit_system(); return ret; }

    /* VPSS ochn1: 320×240 → NPU */
    g_vpss_cfg.pipe_info[FACE_VPSS_PIPE].chn_info[FACE_VPSS_OCHN_NPU].chn_en = XMEDIA_TRUE;
    g_vpss_cfg.pipe_info[FACE_VPSS_PIPE].chn_info[FACE_VPSS_OCHN_NPU].chn_no = FACE_VPSS_OCHN_NPU;
    ret = sample_comm_vpss_get_default_ochn_cfg(
        &g_vpss_cfg.pipe_info[FACE_VPSS_PIPE].chn_info[FACE_VPSS_OCHN_NPU].chn_config,
        npu_size, &g_video_param);
    if (ret != XMEDIA_SUCCESS) { deinit_system(); return ret; }

    ret = sample_comm_vpss_start(&g_vpss_cfg);
    if (ret != XMEDIA_SUCCESS) { deinit_system(); return ret; }
    g_vpss_started = XMEDIA_TRUE;

    /* Bind VI → VPSS */
    ret = sample_comm_sys_vi_bind_vpss(vi_pipe, vi_chn, FACE_VPSS_PIPE, 0);
    if (ret != XMEDIA_SUCCESS) { deinit_system(); return ret; }
    g_vi_vpss_bound = XMEDIA_TRUE;

    /* VENC config: MJPEG */
    memset(&g_venc_cfg, 0, sizeof(g_venc_cfg));
    g_venc_cfg.chn_info[0].venc_en = XMEDIA_TRUE;
    g_venc_cfg.chn_info[0].venc_chn = 0;
    g_venc_cfg.chn_info[0].payload_type = PT_MJPEG;
    g_venc_cfg.chn_info[0].rc_mode = VENC_RC_MODE_MJPEGCBR;
    sample_comm_venc_get_default_chn_info(pic_size, framerate, &g_venc_cfg.chn_info[0]);

    ret = sample_comm_venc_start(&g_venc_cfg);
    if (ret != XMEDIA_SUCCESS) { deinit_system(); return ret; }
    g_venc_started = XMEDIA_TRUE;

    /* Bind VPSS → VENC */
    ret = sample_comm_sys_vpss_bind_venc(FACE_VPSS_PIPE, FACE_VPSS_OCHN_ENC, 0);
    if (ret != XMEDIA_SUCCESS) { deinit_system(); return ret; }
    g_vpss_venc_bound = XMEDIA_TRUE;

    printf("[init] Pipeline: VI→VPSS→VENC(MJPEG) + VPSS→NPU(%dx%d MTCNN)\n",
           DET_WIDTH, DET_HEIGHT);
    return XMEDIA_SUCCESS;
}

/* ------------------------------------------------------------------ */
/*  HTTP route handler                                                 */
/* ------------------------------------------------------------------ */
static int project_route_get(int client_fd, const char *path)
{
    char json_buf[4096];

    if (strcmp(path, "/api/faces") == 0) {
        int count, i;
        size_t off;

        pthread_mutex_lock(&g_face_mutex);
        count = g_face_count;
        if (count > MAX_FACES) count = MAX_FACES;

        off = snprintf(json_buf, sizeof(json_buf),
            "{\"count\":%d,\"fps\":%d,\"faces\":[", count, g_npu_fps);

        for (i = 0; i < count && off < sizeof(json_buf) - 256; i++) {
            float fw = g_faces[i].x2 - g_faces[i].x1;
            float fh = g_faces[i].y2 - g_faces[i].y1;
            /* Normalize to 0.0~1.0 relative to detection image */
            off += snprintf(json_buf + off, sizeof(json_buf) - off,
                "%s{\"x\":%.4f,\"y\":%.4f,\"w\":%.4f,\"h\":%.4f,\"score\":%.3f,"
                "\"landmarks\":[",
                i == 0 ? "" : ",",
                g_faces[i].x1 / DET_WIDTH,
                g_faces[i].y1 / DET_HEIGHT,
                fw / DET_WIDTH,
                fh / DET_HEIGHT,
                g_faces[i].score);
            /* 5 facial keypoints from O-Net (normalized) */
            for (int k = 0; k < 5; k++) {
                off += snprintf(json_buf + off, sizeof(json_buf) - off,
                    "%s{\"x\":%.4f,\"y\":%.4f}",
                    k == 0 ? "" : ",",
                    g_faces[i].landmarks[k * 2] / DET_WIDTH,
                    g_faces[i].landmarks[k * 2 + 1] / DET_HEIGHT);
            }
            off += snprintf(json_buf + off, sizeof(json_buf) - off, "]}");
        }

        pthread_mutex_unlock(&g_face_mutex);

        off += snprintf(json_buf + off, sizeof(json_buf) - off, "]}\n");
        web_send_json_ok(client_fd, json_buf);
        return 0;
    }

    if (strcmp(path, "/api/status") == 0) {
        snprintf(json_buf, sizeof(json_buf),
            "{\"fps\":%d,\"faces\":%d}\n", g_npu_fps, g_face_count);
        web_send_json_ok(client_fd, json_buf);
        return 0;
    }

    return -1;
}

/* ------------------------------------------------------------------ */
/*  Main                                                               */
/* ------------------------------------------------------------------ */
int main(void)
{
    int ret;

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    printf("=== MTCNN Face Detection for GK7206 ===\n");
    printf("[ver] build: %s %s  (3-stage MTCNN, 320x180, thr 0.50/0.70/0.30, O-Net+landmarks)\n",
           __DATE__, __TIME__);

    /* 1. Init video pipeline */
    ret = init_system();
    if (ret != XMEDIA_SUCCESS) {
        printf("[main] System init failed: %d\n", ret);
        return ret;
    }

    /* 2. Init NPU (load 3 MTCNN models) */
    ret = init_npu();
    if (ret != XMEDIA_SUCCESS) {
        printf("[main] NPU init failed: %d\n", ret);
        deinit_system();
        return ret;
    }

    /* 3. Start NPU inference thread */
    g_npu_running = XMEDIA_TRUE;
    ret = pthread_create(&g_npu_thread, NULL, npu_inference_thread, NULL);
    if (ret != 0) {
        printf("[main] Failed to create NPU thread\n");
        deinit_npu();
        deinit_system();
        return XMEDIA_FAILURE;
    }

    /* 4. Register MJPEG handler and start web server (blocking) */
    printf("[main] System ready. Open http://<board-ip>/ in browser\n");
    web_server_set_mjpeg_handler(mjpeg_send_stream, mjpeg_request_stop);
    web_server_run(project_route_get);

    /* 5. Cleanup */
    printf("[main] Shutting down...\n");
    g_npu_running = XMEDIA_FALSE;
    pthread_join(g_npu_thread, NULL);
    deinit_npu();
    deinit_system();

    return 0;
}
