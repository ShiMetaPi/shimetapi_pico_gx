/*
 * Rock-Paper-Scissors YOLOv8 Detection Application for GK7206
 * Simplified version based on classification sample structure
 *
 * Pipeline: VI → VPSS → VENC (MJPEG web stream)
 *                  → ochn1 (640×640 → NPU YOLOv8 detection)
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
#include "xmedia_sys.h"

#include "yolov8_postprocess.h"

/* ------------------------------------------------------------------ */
/*  Constants                                                          */
/* ------------------------------------------------------------------ */
#define RPS_VPSS_PIPE       0
#define RPS_VPSS_OCHN_ENC   0   /* full resolution → VENC MJPEG */
#define RPS_VPSS_OCHN_NPU   1   /* 640×640 → NPU YOLOv8 */

#define RPS_WIDTH           640
#define RPS_HEIGHT          640   /* YOLOv8 input */

#define RPS_MODEL_PATH      "neuron_network.xmm"  /* preferred model file */

#define MAX_DETECTIONS      10

/* Gesture labels */
static const char *g_gesture_names[3] = {"Rock", "Paper", "Scissors"};

/* Bounds-safe access: class_id == -1 means "no detection" */
static inline const char *rps_gesture_name(int class_id)
{
    if (class_id < 0 || class_id >= 3) {
        return "None";
    }
    return g_gesture_names[class_id];
}

/* Multiple candidate model paths (search order) */
static const char *g_model_paths[] = {
    "./" RPS_MODEL_PATH,                        /* Current directory */
    "/usr/bin/" RPS_MODEL_PATH,                 /* Same as app (recommended) */
    "/home/diskh/ljh/GK7206/app_sample/rock_paper_scissors/" RPS_MODEL_PATH, /* Development path */
    NULL
};

#ifndef ALIGN_UP
#define ALIGN_UP(x, a) (((x) + (a) - 1) & ~((a) - 1))
#endif

/* ------------------------------------------------------------------ */
/*  Data types                                                         */
/* ------------------------------------------------------------------ */
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

/* Simple detection result */
typedef struct {
    int class_id;
    float score;
    float bbox[4]; /* x_min, y_min, x_max, y_max */
} simple_detection_t;

/* ------------------------------------------------------------------ */
/*  Global state                                                       */
/* ------------------------------------------------------------------ */
static volatile int g_stop = 0;

/* NPU */
static xmedia_cl_context g_cl_ctx = NULL;
static xmedia_bool g_cl_inited = XMEDIA_FALSE;
static npu_model_t g_model;
static xmedia_bool g_npu_inited = XMEDIA_FALSE;

/* Detection result */
static simple_detection_t g_detection = {.class_id = -1, .score = 0.0f}; /* Initialize as invalid */
/* Full detection list for /api/detections (best-first after NMS). */
static yolov8_detection_t g_dets[MAX_DETECTIONS];
static int g_det_count = 0;
static pthread_mutex_t g_result_mutex = PTHREAD_MUTEX_INITIALIZER;
static volatile int g_npu_fps = 0;
static volatile xmedia_bool g_npu_running = XMEDIA_FALSE;
static pthread_t g_npu_thread;

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

/* Game state */
static volatile int g_score_player = 0;
static volatile int g_score_computer = 0;

/* Debug: save intermediate images (set to 1 to enable) */
#define DEBUG_SAVE_NPU_INPUT    1   /* save RGB input to NPU as .bin file */
#define DEBUG_PRINT_DETS        1   /* print detection details to console */
#define DEBUG_SAVE_INTERVAL     30  /* save every N frames */
static volatile int g_debug_frame_counter = 0;

/* ------------------------------------------------------------------ */
/*  YUV420SP to RGB888 conversion                                      */
/* ------------------------------------------------------------------ */
static void yuv420sp_to_rgb888(const unsigned char *y_plane,
                               const unsigned char *uv_plane,
                               unsigned char *rgb_out,
                               int width, int height,
                               int y_stride, int uv_stride)
{
    int row, col;
    for (row = 0; row < height; row++) {
        for (col = 0; col < width; col++) {
            int y_idx = row * y_stride + col;
            int uv_row = row / 2;
            int uv_col = col / 2;
            int uv_idx = uv_row * uv_stride + uv_col * 2;

            int y = y_plane[y_idx];
            /* NV21: VU interleaved — uv_plane[uv_idx] is V_real,
             * uv_plane[uv_idx+1] is U_real. The BT.601 formulas below
             * expect "v" to carry V and "u" to carry U, so the assignment
             * from uv bytes must match that order. (Earlier this function
             * assigned V_real into a variable named `u` and U_real into a
             * variable named `v`, which silently shifted R and B toward
             * each other and desaturated warm tones.) */
            int v = uv_plane[uv_idx] - 128;
            int u = uv_plane[uv_idx + 1] - 128;
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
/*  Find available model file                                          */
/* ------------------------------------------------------------------ */
static const char *find_model_file(void)
{
    int i;
    FILE *fp;

    for (i = 0; g_model_paths[i] != NULL; i++) {
        fp = fopen(g_model_paths[i], "rb");
        if (fp) {
            fclose(fp);
            printf("[model] Found model at: %s\n", g_model_paths[i]);
            return g_model_paths[i];
        }
    }

    printf("[model] ERROR: Model file not found in any of these paths:\n");
    for (i = 0; g_model_paths[i] != NULL; i++) {
        printf("  - %s\n", g_model_paths[i]);
    }
    return NULL;
}

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
        printf("[npu] input[%u]: size=%u, dims=[%u,%u,%u,%u]\n",
            i, m->input.tensor[i].size,
            m->input.tensor[i].shape.ndims > 0 ? m->input.tensor[i].shape.dims[0] : 0,
            m->input.tensor[i].shape.ndims > 1 ? m->input.tensor[i].shape.dims[1] : 0,
            m->input.tensor[i].shape.ndims > 2 ? m->input.tensor[i].shape.dims[2] : 0,
            m->input.tensor[i].shape.ndims > 3 ? m->input.tensor[i].shape.dims[3] : 0);
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
        printf("[npu] output[%u]: size=%u, dims=[%u,%u,%u,%u]\n",
            i, m->output.tensor[i].size,
            m->output.tensor[i].shape.ndims > 0 ? m->output.tensor[i].shape.dims[0] : 0,
            m->output.tensor[i].shape.ndims > 1 ? m->output.tensor[i].shape.dims[1] : 0,
            m->output.tensor[i].shape.ndims > 2 ? m->output.tensor[i].shape.dims[2] : 0,
            m->output.tensor[i].shape.ndims > 3 ? m->output.tensor[i].shape.dims[3] : 0);
    }

    /* Set inout */
    ret = xmedia_cl_graph_set_inout(m->graph, &m->input, &m->output);
    if (ret != XMEDIA_CL_SUCCESS) {
        printf("[npu] set_inout failed for %s: %d\n", path, ret);
        return XMEDIA_FAILURE;
    }

    printf("[npu] %s ready (%u inputs, %u outputs)\n",
           path, m->input.num, m->output.num);
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

/* ------------------------------------------------------------------ */
/*  YOLOv8 post-processing                                             */
/*  Outputs are identified by shape (not tensor index) for robustness: */
/*    box tensor [1, 4*reg_max, H, W], cls tensor [1, num_cls, H, W];  */
/*    stride = input_size / H.  Pairs matched by equal H.              */
/* ------------------------------------------------------------------ */
static void postprocess_yolov8(void)
{
    static yolov8_param_t param;
    static int param_inited = 0;
    static int first_frame = 1;
    yolov8_detection_t cand[256];
    const xmedia_cl_tensor *box_t[3] = {NULL, NULL, NULL};
    const xmedia_cl_tensor *cls_t[3] = {NULL, NULL, NULL};
    xmedia_cl_u32 i;
    int slot, kept, n = 0;

    if (!param_inited) {
        yolov8_default_param(&param);
        param_inited = 1;
    }

    /* One-time: dump output tensor shapes + quant params + save raw output + print sample values */
    if (first_frame) {
        first_frame = 0;
        printf("[rps] === Output tensor info (first frame) ===\n");
        for (i = 0; i < g_model.output.num; i++) {
            const xmedia_cl_tensor *t = &g_model.output.tensor[i];
            int ndims = (int)t->shape.ndims;
            printf("[rps] output[%u]: dims=[%u,%u,%u,%u] scale=%.6f zp=%d\n",
                i,
                ndims > 0 ? t->shape.dims[0] : 0,
                ndims > 1 ? t->shape.dims[1] : 0,
                ndims > 2 ? t->shape.dims[2] : 0,
                ndims > 3 ? t->shape.dims[3] : 0,
                t->quant.scale, t->quant.zp);
        }
        printf("[rps] conf_threshold=%.2f, nms_threshold=%.2f\n",
               param.conf_threshold, param.nms_threshold);

        /* Print sample raw uint8 values from cls tensors at center cells */
        printf("[rps] --- Sample cls raw values (center cells) ---\n");
        for (i = 0; i < g_model.output.num; i++) {
            const xmedia_cl_tensor *t = &g_model.output.tensor[i];
            int C = (t->shape.ndims > 1) ? (int)t->shape.dims[1] : 0;
            int H = (t->shape.ndims > 2) ? (int)t->shape.dims[2] : 0;
            int W = (t->shape.ndims > 3) ? (int)t->shape.dims[3] : H;
            if (C != param.num_classes) continue;  /* only cls tensors */

            unsigned char *cls_u8 = (unsigned char *)t->addr;
            int gx = W / 2, gy = H / 2;  /* center cell */
            printf("[rps] cls tensor[%u] H=%d W=%d, center(gx=%d,gy=%d) raw=[",
                   i, H, W, gx, gy);
            for (int c = 0; c < C; c++) {
                int off = (c * H + gy) * W + gx;
                printf("%d", cls_u8[off]);
                if (c < C-1) printf(",");
            }
            printf("] -> dequant=[");
            for (int c = 0; c < C; c++) {
                int off = (c * H + gy) * W + gx;
                float dq = dequant_u8(cls_u8[off], t->quant.zp, t->quant.scale);
                printf("%.2f", dq);
                if (c < C-1) printf(",");
            }
            printf("] -> sigmoid=[");
            for (int c = 0; c < C; c++) {
                int off = (c * H + gy) * W + gx;
                float conf = sigmoid_f(dequant_u8(cls_u8[off], t->quant.zp, t->quant.scale));
                printf("%.4f", conf);
                if (c < C-1) printf(",");
            }
            printf("]\n");
        }

        /* Save raw output tensors for offline analysis */
        for (i = 0; i < g_model.output.num; i++) {
            char fname[64];
            snprintf(fname, sizeof(fname), "/tmp/raw_output_%u.bin", i);
            FILE *fp = fopen(fname, "wb");
            if (fp) {
                fwrite(g_model.output.tensor[i].addr, 1,
                       g_model.output.tensor[i].size, fp);
                fclose(fp);
            }
        }
        printf("[rps] Saved raw outputs to /tmp/raw_output_*.bin\n");
        printf("[rps] ============================================\n");
    }

    /* Group the 6 output tensors into (box, cls) pairs by grid size. */
    for (i = 0; i < g_model.output.num; i++) {
        const xmedia_cl_tensor *t = &g_model.output.tensor[i];
        int ndims = (int)t->shape.ndims;
        int C = (ndims > 1) ? (int)t->shape.dims[1] : 0;
        int H = (ndims > 2) ? (int)t->shape.dims[2] : 0;

        if      (H == 80) slot = 0;
        else if (H == 40) slot = 1;
        else if (H == 20) slot = 2;
        else continue;

        if (C == 4 * param.reg_max)         box_t[slot] = t;
        else if (C == param.num_classes)    cls_t[slot] = t;
    }

    for (slot = 0; slot < 3; slot++) {
        const xmedia_cl_tensor *bt = box_t[slot];
        const xmedia_cl_tensor *ct = cls_t[slot];
        int H, W, stride, gy, gx, c;
        float b_scale, c_scale;
        int b_zp, c_zp;
        unsigned char *box_u8, *cls_u8;

        if (bt == NULL || ct == NULL) continue;

        H = (bt->shape.ndims > 2) ? (int)bt->shape.dims[2] : 0;
        W = (bt->shape.ndims > 3) ? (int)bt->shape.dims[3] : H;
        if (H <= 0 || W <= 0) continue;
        stride = param.input_size / H;
        if (stride <= 0) continue;

        b_scale = bt->quant.scale; b_zp = bt->quant.zp;
        c_scale = ct->quant.scale; c_zp = ct->quant.zp;
        box_u8 = (unsigned char *)bt->addr;
        cls_u8 = (unsigned char *)ct->addr;

        for (gy = 0; gy < H && n < (int)(sizeof(cand) / sizeof(cand[0])); gy++) {
            for (gx = 0; gx < W && n < (int)(sizeof(cand) / sizeof(cand[0])); gx++) {
                int best_cls = -1;
                float best_conf = 0.0f;
                float dist[4];
                float x1, y1, x2, y2;
                int b, k;

                /* Class scores: [num_cls, H, W], NCHW uint8. */
                for (c = 0; c < param.num_classes; c++) {
                    int off = (c * H + gy) * W + gx;
                    float conf = sigmoid_f(dequant_u8(cls_u8[off], c_zp, c_scale));
                    if (conf > best_conf) {
                        best_conf = conf;
                        best_cls = c;
                    }
                }
                if (best_cls < 0 || best_conf < param.conf_threshold) continue;

                /* Box DFL: [4*reg_max, H, W] -> 4 sides, each reg_max bins. */
                for (b = 0; b < 4; b++) {
                    float logits[YOLO_REG_MAX];
                    for (k = 0; k < param.reg_max; k++) {
                        int off = ((b * param.reg_max + k) * H + gy) * W + gx;
                        logits[k] = dequant_u8(box_u8[off], b_zp, b_scale);
                    }
                    dist[b] = dfl_decode(logits, param.reg_max);
                }

                /* DFL distances -> absolute box (SDK formula), clamped. */
                x1 = (-dist[0] + (float)gx + 0.5f) * (float)stride;
                y1 = (-dist[1] + (float)gy + 0.5f) * (float)stride;
                x2 = ( dist[2] + (float)gx + 0.5f) * (float)stride;
                y2 = ( dist[3] + (float)gy + 0.5f) * (float)stride;
                if (x1 < 0.0f) x1 = 0.0f;
                if (y1 < 0.0f) y1 = 0.0f;
                if (x2 > (float)param.input_size) x2 = (float)param.input_size;
                if (y2 > (float)param.input_size) y2 = (float)param.input_size;
                if (x2 - x1 < 1.0f || y2 - y1 < 1.0f) continue;

                cand[n].x_min = x1; cand[n].y_min = y1;
                cand[n].x_max = x2; cand[n].y_max = y2;
                cand[n].score = best_conf;
                cand[n].class_id = best_cls;
                n++;
            }
        }
    }

    kept = yolov8_nms(cand, n, param.nms_threshold, param.max_detections);

    pthread_mutex_lock(&g_result_mutex);
    g_det_count = (kept > MAX_DETECTIONS) ? MAX_DETECTIONS : kept;
    for (i = 0; i < (xmedia_cl_u32)g_det_count; i++) {
        g_dets[i] = cand[i];
    }
    /* Best (highest-confidence) detection drives the game. */
    if (g_det_count > 0) {
        g_detection.class_id = g_dets[0].class_id;
        g_detection.score = g_dets[0].score;
    } else {
        g_detection.class_id = -1;
        g_detection.score = 0.0f;
    }
    pthread_mutex_unlock(&g_result_mutex);
}

/* ------------------------------------------------------------------ */
/*  NPU inference thread                                               */
/* ------------------------------------------------------------------ */
static void *npu_thread_func(void *arg)
{
    (void)arg;
    xmedia_s32 ret;
    xmedia_video_frame_info npu_frame;
    xmedia_s32 milli_sec = 2000;
    int frame_counter = 0;
    int fps_counter = 0;
    struct timespec ts_start, ts_now;
    unsigned char *rgb_buf = NULL;

    printf("[rps] NPU inference thread started\n");
    printf("[rps] YUV420SP->RGB888: NV21 channel order (V,U), feeding RGB to model\n");

    /* Allocate RGB conversion buffer (640×640×3) */
    rgb_buf = (unsigned char *)malloc(RPS_WIDTH * RPS_HEIGHT * 3);
    if (rgb_buf == NULL) {
        printf("[rps] Failed to allocate RGB buffer\n");
        return NULL;
    }

    clock_gettime(CLOCK_MONOTONIC, &ts_start);

    while (g_npu_running && !g_stop) {
        int y_stride, uv_stride;
        xmedia_u32 y_map_size, c_map_size;
        void *frame_y = NULL, *frame_uv = NULL;

        /* 1. Acquire 640×640 frame from VPSS */
        memset(&npu_frame, 0, sizeof(npu_frame));
        ret = xmedia_vpss_acquire_ochn_frame(
            RPS_VPSS_PIPE, RPS_VPSS_OCHN_NPU, &npu_frame, milli_sec);
        if (ret != XMEDIA_SUCCESS) continue;

        /* 2. Get YUV plane sizes */
        y_stride = npu_frame.frame.stride.y_stride ? npu_frame.frame.stride.y_stride : RPS_WIDTH;
        uv_stride = npu_frame.frame.stride.c_stride ? npu_frame.frame.stride.c_stride : RPS_WIDTH;

        y_map_size = y_stride * RPS_HEIGHT;
        c_map_size = uv_stride * (RPS_HEIGHT / 2);

        /* 3. Map Y and UV planes */
        frame_y = xmedia_mmz_map(npu_frame.frame.addr.y_phy_addr, y_map_size, XMEDIA_FALSE);
        frame_uv = xmedia_mmz_map(npu_frame.frame.addr.c_phy_addr, c_map_size, XMEDIA_FALSE);

        if (frame_y != NULL && frame_uv != NULL) {
            /* 4. YUV420SP → RGB888 conversion */
            yuv420sp_to_rgb888(frame_y, frame_uv, rgb_buf,
                               RPS_WIDTH, RPS_HEIGHT, y_stride, uv_stride);

#if DEBUG_SAVE_NPU_INPUT
            /* [verify] Image-format sanity log — fires every 10 frames (≈3 prints/s at
             * 30fps). Two questions this answers:
             *   (a) Did NV21 → RGB keep the channel order, or did R and B come out swapped?
             *       (hypothesis to test: variable names u/v in main.c:168-169 may be
             *        assigned from the wrong UV bytes)
             *   (b) Are the Y / U / V planes actually carrying picture data, vs. all-zero
             *       buffers or unwritten caches?
             * Each sampled point prints:
             *   Y raw, V_real (uv[0]), U_real (uv[1]),
             *   code:R,G,B — what the model is currently being fed,
             *   fixed:R,G,B — what BT.601 full-range + correct byte order would give.
             * If "code R" consistently equals "fixed B" and vice versa, R/B are swapped. */
            if (frame_counter % 10 == 0) {
                static const struct { int row, col; const char *name; } s_pts[] = {
                    { 32,  32,  "TL"}, { 32,  320, "TC"}, { 32,  608, "TR"},
                    {320,  32,  "ML"}, {320,  320, "CC"}, {320,  608, "MR"},
                    {608,  32,  "BL"}, {608,  320, "BC"}, {608,  608, "BR"},
                };
                int si;
                printf("[verify] === frame=%d pixel sample ===\n", frame_counter);
                printf("[verify] label @[r,c]  : Y  V  U  | code(R,G,B)    fixed(R,G,B)\n");
                for (si = 0; si < (int)(sizeof(s_pts) / sizeof(s_pts[0])); si++) {
                    int r = s_pts[si].row, c = s_pts[si].col;
                    int y_i   = r * y_stride + c;
                    int uv_r  = r >> 1;
                    int uv_c  = c >> 1;
                    int uv_i  = uv_r * uv_stride + (uv_c << 1);
                    int Y_raw = ((const unsigned char *)frame_y)[y_i];
                    int V_real= ((const unsigned char *)frame_uv)[uv_i]     - 128;
                    int U_real= ((const unsigned char *)frame_uv)[uv_i + 1] - 128;
                    int rgb_i = r * RPS_WIDTH + c;
                    int cR = rgb_buf[rgb_i * 3 + 0];
                    int cG = rgb_buf[rgb_i * 3 + 1];
                    int cB = rgb_buf[rgb_i * 3 + 2];
                    /* Reference: BT.601 full-range with V in uv_i, U in uv_i+1 (correct order). */
                    int fR = Y_raw + ((1436 * V_real) >> 10);
                    int fG = Y_raw - ((352 * U_real + 731 * V_real) >> 10);
                    int fB = Y_raw + ((1814 * U_real) >> 10);
                    if (fR < 0) fR = 0; else if (fR > 255) fR = 255;
                    if (fG < 0) fG = 0; else if (fG > 255) fG = 255;
                    if (fB < 0) fB = 0; else if (fB > 255) fB = 255;
                    printf("[verify] %s @[%3d,%3d]: %3d %3d %3d | (%3d,%3d,%3d)    (%3d,%3d,%3d)\n",
                           s_pts[si].name, r, c, Y_raw, V_real + 128, U_real + 128,
                           cR, cG, cB, fR, fG, fB);
                }

                /* Whole-image channel means + a "swap-detector" hint.
                 * In typical indoor RPS scenes, hand/skin/warm tones dominate, so the
                 * correct pixel's R should be >= B. If the code is feeding R and B
                 * swapped, we expect code_R < code_B on a noticeable fraction of pixels. */
                unsigned long sR = 0, sG = 0, sB = 0;
                unsigned long warm_swap = 0, warm_total = 0;
                int pxl;
                int npx = RPS_WIDTH * RPS_HEIGHT;
                for (pxl = 0; pxl < npx; pxl++) {
                    int cr = rgb_buf[pxl*3 + 0];
                    int cg = rgb_buf[pxl*3 + 1];
                    int cb = rgb_buf[pxl*3 + 2];
                    sR += cr; sG += cg; sB += cb;
                    /* A "warm" pixel in the input image is one where the code output
                     * has R > B (assumes code's R is still red). If >25% show the
                     * opposite (R < B), that's the swap signature. */
                    if (cr > cb + 8) { warm_total++; }
                    else if (cb > cr + 8) { warm_total++; warm_swap++; }
                }
                printf("[verify] Channel means: R=%.1f  G=%.1f  B=%.1f\n",
                       (double)sR / npx, (double)sG / npx, (double)sB / npx);
                printf("[verify] Swap-detector: %lu / %lu pixels show B > R by >+8 "
                       "(>5%% = R/B likely swapped, <2%% = order looks healthy)\n",
                       warm_swap, warm_total);
                fflush(stdout);
            }
#endif

            /* 5. Copy RGB to model input buffer */
            int input_size = g_model.input.tensor[0].size;
            int rgb_size = RPS_WIDTH * RPS_HEIGHT * 3;
            int copy_size = (rgb_size < input_size) ? rgb_size : input_size;
            memcpy(g_model.input.tensor[0].addr, rgb_buf, copy_size);

            /* 6. Flush cache so NPU sees the data */
            xmedia_mmz_flush_cache(g_model.input_phy, g_model.input_buf, input_size);

            /* 7. Run inference */
            ret = xmedia_cl_graph_process(g_model.graph);

            if (ret == XMEDIA_CL_SUCCESS) {
                /* Dequantize -> DFL decode -> NMS; updates g_detection + g_dets. */
                postprocess_yolov8();

#if DEBUG_SAVE_NPU_INPUT
                /* Save NPU input RGB (640x640x3) as raw .bin file for offline viewing.
                 * Convert on host: python3 -c "from PIL import Image; import numpy as np; \
                 *   a=np.fromfile('npu_input.bin',dtype=np.uint8).reshape(640,640,3); \
                 *   Image.fromarray(a,'RGB').save('npu_input.png')"  */
                if (frame_counter % DEBUG_SAVE_INTERVAL == 0) {
                    char fname[64];
                    snprintf(fname, sizeof(fname), "/tmp/npu_input_%d.bin", frame_counter);
                    FILE *fp = fopen(fname, "wb");
                    if (fp) {
                        fwrite(rgb_buf, 1, RPS_WIDTH * RPS_HEIGHT * 3, fp);
                        fclose(fp);
                        printf("[debug] Saved NPU input RGB to %s\n", fname);
                    }

                    /* One-time RGB channel stats on the very first saved frame.
                     * For natural scenes: mean R ~ mean G > mean B (sky/skin bias toward warm).
                     * If R/B are swapped you will see mean B > mean R on real skin/foliage. */
                    if (frame_counter == 0) {
                        unsigned long sum_r = 0, sum_g = 0, sum_b = 0;
                        int pxl;
                        for (pxl = 0; pxl < RPS_WIDTH * RPS_HEIGHT; pxl++) {
                            sum_r += rgb_buf[pxl * 3 + 0];
                            sum_g += rgb_buf[pxl * 3 + 1];
                            sum_b += rgb_buf[pxl * 3 + 2];
                        }
                        int n = RPS_WIDTH * RPS_HEIGHT;
                        printf("[debug] NPU input channel means: R=%.1f G=%.1f B=%.1f  (RGB-RGB vs BGR-RGB; expect R~G>=B for natural scenes)\n",
                               (double)sum_r / n, (double)sum_g / n, (double)sum_b / n);

                        /* Also write a portable PPM (P6) so we can pull it back and view. */
                        snprintf(fname, sizeof(fname), "/tmp/npu_input_%d.ppm", frame_counter);
                        fp = fopen(fname, "wb");
                        if (fp) {
                            fprintf(fp, "P6\n%d %d\n255\n", RPS_WIDTH, RPS_HEIGHT);
                            fwrite(rgb_buf, 1, RPS_WIDTH * RPS_HEIGHT * 3, fp);
                            fclose(fp);
                            printf("[debug] Saved PPM (portable) to %s\n", fname);
                        }
                    }
                }
#endif

#if DEBUG_PRINT_DETS
                /* Print detection details each frame */
                if (frame_counter % 10 == 0) {
                    pthread_mutex_lock(&g_result_mutex);
                    int cnt = g_det_count;
                    printf("[rps] frame=%d, detections=%d: ", frame_counter, cnt);
                    for (int d = 0; d < cnt && d < 3; d++) {
                        printf("%s(%.2f)[%.0f,%.0f,%.0f,%.0f] ",
                               rps_gesture_name(g_dets[d].class_id),
                               g_dets[d].score,
                               g_dets[d].x_min, g_dets[d].y_min,
                               g_dets[d].x_max, g_dets[d].y_max);
                    }
                    if (cnt == 0) printf("none");
                    printf("\n");
                    pthread_mutex_unlock(&g_result_mutex);
                }
#endif

                if (frame_counter % 30 == 0) {
                    printf("[rps] NPU inference OK (frame %d, %u outputs, %d dets)\n",
                           frame_counter, g_model.output.num, g_det_count);
                }
            } else {
                if (frame_counter % 30 == 0) {
                    printf("[rps] NPU inference failed: %d\n", ret);
                }
            }
        } else {
            printf("[rps] mmz map failed (Y=%p UV=%p)\n", frame_y, frame_uv);
        }

        /* 8. Cleanup */
        if (frame_y != NULL) xmedia_mmz_unmap(frame_y);
        if (frame_uv != NULL) xmedia_mmz_unmap(frame_uv);

        /* Release frame */
        xmedia_vpss_release_ochn_frame(
            RPS_VPSS_PIPE, RPS_VPSS_OCHN_NPU, &npu_frame);

        frame_counter++;
        fps_counter++;

        /* Update FPS once per second */
        clock_gettime(CLOCK_MONOTONIC, &ts_now);
        float elapsed = (ts_now.tv_sec - ts_start.tv_sec) +
                        (ts_now.tv_nsec - ts_start.tv_nsec) / 1e9f;
        if (elapsed >= 1.0f) {
            g_npu_fps = (int)(fps_counter / elapsed);
            fps_counter = 0;
            ts_start = ts_now;
        }
    }

    free(rgb_buf);
    printf("[rps] NPU thread stopped\n");
    return NULL;
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

    /* Load the YOLOv8 model */
    printf("[npu] Loading model...\n");
    const char *model_path = find_model_file();
    if (model_path == NULL) {
        printf("[npu] Model file not found!\n");
        return XMEDIA_FAILURE;
    }

    ret = load_one_model(&g_model, g_cl_ctx, model_path);
    if (ret != XMEDIA_SUCCESS) {
        printf("[npu] Failed to load model from %s\n", model_path);
        return XMEDIA_FAILURE;
    }

    g_npu_inited = XMEDIA_TRUE;
    printf("[npu] Initialized successfully\n");

    /* Start NPU thread */
    g_npu_running = XMEDIA_TRUE;
    pthread_create(&g_npu_thread, NULL, npu_thread_func, NULL);

    return XMEDIA_SUCCESS;
}

static void deinit_npu(void)
{
    if (g_npu_running) {
        g_npu_running = XMEDIA_FALSE;
        pthread_join(g_npu_thread, NULL);
    }

    if (g_npu_inited) {
        unload_one_model(&g_model);
        g_npu_inited = XMEDIA_FALSE;
    }

    if (g_cl_ctx) {
        xmedia_cl_release_context(g_cl_ctx);
        g_cl_ctx = NULL;
    }

    if (g_cl_inited) {
        xmedia_cl_uninit();
        g_cl_inited = XMEDIA_FALSE;
    }
}

/* ------------------------------------------------------------------ */
/*  Video pipeline initialization                                      */
/* ------------------------------------------------------------------ */
static int init_system(void)
{
    xmedia_s32 ret;
    xmedia_s32 vi_dev = 0;
    xmedia_s32 vi_pipe = 0;
    xmedia_s32 vi_chn = 0;
    sample_comm_sensor_type sensor_type = SENSOR0_TYPE;
    xmedia_u32 framerate = 0;
    xmedia_video_size pic_size = {0};
    xmedia_video_size npu_size = {0};

    memset(&g_video_param, 0, sizeof(g_video_param));
    g_video_param.video_fmt = XMEDIA_VIDEO_FMT_LINEAR;
    g_video_param.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420;
    g_video_param.data_width = XMEDIA_VIDEO_DATA_WIDTH_8;
    g_video_param.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;

    sample_comm_vi_get_sensor_info(sensor_type, &g_sensor_info);
    sample_comm_vi_get_framerate_by_sensor(sensor_type, &framerate);

    printf("[init] sensor_type=%d, size=%ux%u, fps=%u\n",
        sensor_type, g_sensor_info.width, g_sensor_info.height, framerate);

    /* System init: OFFLINE mode + proper VB pools (match working classification sample) */
    memset(&g_sys_cfg, 0, sizeof(g_sys_cfg));
    g_sys_cfg.sys_conf.pipe_mode[vi_pipe].vicap_viproc_mode = XMEDIA_WORK_MODE_OFFLINE;
    g_sys_cfg.sys_conf.pipe_mode[vi_pipe].viproc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;
    g_sys_cfg.sys_conf.pipe_mode[vi_pipe].gdc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;
    g_sys_cfg.vb_conf.max_pool_cnt = 25;

    pic_size.width = g_sensor_info.width;
    pic_size.height = g_sensor_info.height;
    npu_size.width = RPS_WIDTH;
    npu_size.height = RPS_HEIGHT;

    /* Pool 0: VI capture buffer */
    xmedia_s32 blk_size = sample_comm_sys_get_buffer_size(pic_size, g_video_param.video_fmt,
        g_sensor_info.pixel_format, g_sensor_info.bit_width, g_video_param.compress_mode);
    g_sys_cfg.vb_conf.common_pool[0].block_size = blk_size;
    g_sys_cfg.vb_conf.common_pool[0].block_cnt = 2;

    /* Pool 1: VPSS full res / VENC */
    blk_size = sample_comm_sys_get_buffer_size(pic_size, g_video_param.video_fmt,
        g_video_param.pixel_fmt, g_video_param.data_width, g_video_param.compress_mode);
    g_sys_cfg.vb_conf.common_pool[1].block_size = blk_size;
    g_sys_cfg.vb_conf.common_pool[1].block_cnt = 5;

    /* Pool 2: VPSS 640×640 NPU input */
    blk_size = sample_comm_sys_get_buffer_size(npu_size, g_video_param.video_fmt,
        g_video_param.pixel_fmt, g_video_param.data_width, g_video_param.compress_mode);
    g_sys_cfg.vb_conf.common_pool[2].block_size = blk_size;
    g_sys_cfg.vb_conf.common_pool[2].block_cnt = 3;

    g_sys_cfg.vb_conf.supplement_config = 1;

    ret = sample_comm_sys_init(&g_sys_cfg);
    if (ret != XMEDIA_SUCCESS) {
        printf("[init] sys_init failed: %d\n", ret);
        return ret;
    }

    /* VI init */
    ret = sample_comm_vi_init();
    if (ret != XMEDIA_SUCCESS) {
        printf("[init] vi_init failed: %d\n", ret);
        return ret;
    }
    g_vi_module_inited = XMEDIA_TRUE;

    /* VENC init */
    ret = sample_comm_venc_init();
    if (ret != XMEDIA_SUCCESS) {
        printf("[init] venc_init failed: %d\n", ret);
        return ret;
    }
    g_venc_module_inited = XMEDIA_TRUE;

    /* VPSS init */
    ret = sample_comm_vpss_init();
    if (ret != XMEDIA_SUCCESS) {
        printf("[init] vpss_init failed: %d\n", ret);
        return ret;
    }
    g_vpss_module_inited = XMEDIA_TRUE;

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
    xmedia_isp_config isp_config = {0};
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
    if (ret != XMEDIA_SUCCESS) {
        printf("[init] isp_init failed: %d\n", ret);
        return ret;
    }
    g_isp_inited = XMEDIA_TRUE;

    ret = sample_comm_vi_start(&g_vi_cfg, &g_video_param);
    if (ret != XMEDIA_SUCCESS) {
        printf("[init] vi_start failed: %d\n", ret);
        return ret;
    }
    g_vi_started = XMEDIA_TRUE;

    ret = sample_comm_isp_start(&g_isp_param);
    if (ret != XMEDIA_SUCCESS) {
        printf("[init] isp_start failed: %d\n", ret);
        return ret;
    }
    g_isp_started = XMEDIA_TRUE;

    /* VPSS config */
    memset(&g_vpss_cfg, 0, sizeof(g_vpss_cfg));
    g_vpss_cfg.pipe_info[RPS_VPSS_PIPE].pipe_en = XMEDIA_TRUE;
    g_vpss_cfg.pipe_info[RPS_VPSS_PIPE].pipe_no = RPS_VPSS_PIPE;

    ret = sample_comm_vpss_get_default_pipe_cfg(
        &g_vpss_cfg.pipe_info[RPS_VPSS_PIPE].pipe_config, pic_size, &g_video_param);
    if (ret != XMEDIA_SUCCESS) {
        printf("[init] vpss pipe config failed: %d\n", ret);
        return ret;
    }

    /* VPSS ochn0: full resolution → VENC */
    g_vpss_cfg.pipe_info[RPS_VPSS_PIPE].chn_info[RPS_VPSS_OCHN_ENC].chn_en = XMEDIA_TRUE;
    g_vpss_cfg.pipe_info[RPS_VPSS_PIPE].chn_info[RPS_VPSS_OCHN_ENC].chn_no = RPS_VPSS_OCHN_ENC;
    ret = sample_comm_vpss_get_default_ochn_cfg(
        &g_vpss_cfg.pipe_info[RPS_VPSS_PIPE].chn_info[RPS_VPSS_OCHN_ENC].chn_config,
        pic_size, &g_video_param);
    if (ret != XMEDIA_SUCCESS) {
        printf("[init] vpss ochn0 config failed: %d\n", ret);
        return ret;
    }

    /* VPSS ochn1: 640×640 → NPU */
    npu_size.width = RPS_WIDTH;
    npu_size.height = RPS_HEIGHT;
    g_vpss_cfg.pipe_info[RPS_VPSS_PIPE].chn_info[RPS_VPSS_OCHN_NPU].chn_en = XMEDIA_TRUE;
    g_vpss_cfg.pipe_info[RPS_VPSS_PIPE].chn_info[RPS_VPSS_OCHN_NPU].chn_no = RPS_VPSS_OCHN_NPU;
    ret = sample_comm_vpss_get_default_ochn_cfg(
        &g_vpss_cfg.pipe_info[RPS_VPSS_PIPE].chn_info[RPS_VPSS_OCHN_NPU].chn_config,
        npu_size, &g_video_param);
    if (ret != XMEDIA_SUCCESS) {
        printf("[init] vpss ochn1 config failed: %d\n", ret);
        return ret;
    }

    ret = sample_comm_vpss_start(&g_vpss_cfg);
    if (ret != XMEDIA_SUCCESS) {
        printf("[init] vpss_start failed: %d\n", ret);
        return ret;
    }
    g_vpss_started = XMEDIA_TRUE;

    /* Bind VI → VPSS */
    ret = sample_comm_sys_vi_bind_vpss(vi_pipe, vi_chn, RPS_VPSS_PIPE, 0);
    if (ret != XMEDIA_SUCCESS) {
        printf("[init] vi_bind_vpss failed: %d\n", ret);
        return ret;
    }
    g_vi_vpss_bound = XMEDIA_TRUE;

    /* VENC config: MJPEG streaming (match classification sample) */
    memset(&g_venc_cfg, 0, sizeof(g_venc_cfg));
    g_venc_cfg.chn_info[0].venc_en = XMEDIA_TRUE;
    g_venc_cfg.chn_info[0].venc_chn = 0;
    g_venc_cfg.chn_info[0].payload_type = PT_MJPEG;
    g_venc_cfg.chn_info[0].rc_mode = VENC_RC_MODE_MJPEGCBR;
    sample_comm_venc_get_default_chn_info(pic_size, framerate, &g_venc_cfg.chn_info[0]);

    ret = sample_comm_venc_start(&g_venc_cfg);
    if (ret != XMEDIA_SUCCESS) {
        printf("[init] venc_start failed: %d\n", ret);
        return ret;
    }
    g_venc_started = XMEDIA_TRUE;

    /* Bind VPSS channel 0 to VENC */
    ret = sample_comm_sys_vpss_bind_venc(RPS_VPSS_PIPE, RPS_VPSS_OCHN_ENC, 0);
    if (ret != XMEDIA_SUCCESS) {
        printf("[init] vpss_bind_venc failed: %d\n", ret);
        return ret;
    }
    g_vpss_venc_bound = XMEDIA_TRUE;

    printf("[init] System initialized successfully\n");
    return XMEDIA_SUCCESS;
}

static void deinit_system(void)
{
    if (g_vpss_venc_bound == XMEDIA_TRUE) {
        sample_comm_sys_vpss_unbind_venc(RPS_VPSS_PIPE, RPS_VPSS_OCHN_ENC, 0);
        g_vpss_venc_bound = XMEDIA_FALSE;
    }

    if (g_vi_vpss_bound == XMEDIA_TRUE) {
        sample_comm_sys_vi_unbind_vpss(0, 0, RPS_VPSS_PIPE, 0);
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

    if (g_vpss_module_inited == XMEDIA_TRUE) {
        sample_comm_vpss_exit();
        g_vpss_module_inited = XMEDIA_FALSE;
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

/* ------------------------------------------------------------------ */
/*  Web server route handler                                           */
/* ------------------------------------------------------------------ */
static int project_route_get(int client_fd, const char *path)
{
    char json_buf[512];

    if (strcmp(path, "/api/status") == 0) {
        int cnt;
        pthread_mutex_lock(&g_result_mutex);
        cnt = g_det_count;
        pthread_mutex_unlock(&g_result_mutex);
        snprintf(json_buf, sizeof(json_buf),
            "{\n"
            "  \"fps\": %d,\n"
            "  \"model\": \"yolov8_rps\",\n"
            "  \"input_size\": \"640x640\",\n"
            "  \"detections\": %d\n"
            "}\n", g_npu_fps, cnt);
        web_send_json_ok(client_fd, json_buf);
        return 0;
    }

    if (strcmp(path, "/api/detections") == 0) {
        char det_buf[1024];
        yolov8_detection_t snap[MAX_DETECTIONS];
        int count, idx, pos = 0, w;
        const char *sep;

        pthread_mutex_lock(&g_result_mutex);
        count = g_det_count;
        if (count > MAX_DETECTIONS) count = MAX_DETECTIONS;
        memcpy(snap, g_dets, sizeof(yolov8_detection_t) * count);
        pthread_mutex_unlock(&g_result_mutex);

        pos += snprintf(det_buf + pos, sizeof(det_buf) - pos,
            "{\n  \"count\": %d,\n  \"detections\": [", count);
        for (idx = 0; idx < count; idx++) {
            sep = (idx < count - 1) ? "," : "";
            w = snprintf(det_buf + pos, sizeof(det_buf) - pos,
                "\n    {\"class\": \"%s\", \"score\": %.2f, \"bbox\": [%.0f, %.0f, %.0f, %.0f]}%s",
                rps_gesture_name(snap[idx].class_id), snap[idx].score,
                snap[idx].x_min, snap[idx].y_min, snap[idx].x_max, snap[idx].y_max, sep);
            if (w < 0 || pos + w >= (int)sizeof(det_buf) - 32) break;
            pos += w;
        }
        snprintf(det_buf + pos, sizeof(det_buf) - pos, "%s]\n}\n",
                 count > 0 ? "\n  " : "");
        web_send_json_ok(client_fd, det_buf);
        return 0;
    }

    if (strcmp(path, "/api/gesture/current") == 0) {
        pthread_mutex_lock(&g_result_mutex);
        snprintf(json_buf, sizeof(json_buf),
            "{\n"
            "  \"gesture\": \"%s\",\n"
            "  \"confidence\": %.2f\n"
            "}\n",
            rps_gesture_name(g_detection.class_id),
            g_detection.score);
        pthread_mutex_unlock(&g_result_mutex);
        web_send_json_ok(client_fd, json_buf);
        return 0;
    }

    if (strcmp(path, "/api/game/start") == 0) {
        int computer = (rand() % 3);
        pthread_mutex_lock(&g_result_mutex);
        int player_gesture = g_detection.class_id;
        pthread_mutex_unlock(&g_result_mutex);

        const char *result = "ongoing";
        if (player_gesture >= 0) {
            if (player_gesture == computer) result = "tie";
            else if ((player_gesture == 0 && computer == 2) ||
                     (player_gesture == 1 && computer == 0) ||
                     (player_gesture == 2 && computer == 1)) {
                result = "win";
                g_score_player++;
            } else {
                result = "lose";
                g_score_computer++;
            }
        }

        snprintf(json_buf, sizeof(json_buf),
            "{\n"
            "  \"player\": \"%s\",\n"
            "  \"computer\": \"%s\",\n"
            "  \"result\": \"%s\"\n"
            "}\n",
            rps_gesture_name(player_gesture),
            g_gesture_names[computer],
            result);
        web_send_json_ok(client_fd, json_buf);
        return 0;
    }

    if (strcmp(path, "/api/game/score") == 0) {
        snprintf(json_buf, sizeof(json_buf),
            "{\n"
            "  \"player\": %d,\n"
            "  \"computer\": %d,\n"
            "  \"rounds\": %d\n"
            "}\n",
            g_score_player, g_score_computer,
            g_score_player + g_score_computer);
        web_send_json_ok(client_fd, json_buf);
        return 0;
    }

    return -1;
}

/* ------------------------------------------------------------------ */
/*  MJPEG streaming — pulls JPEG frames from VENC channel 0 and pushes */
/*  them to the browser as multipart/x-mixed-replace over /mjpeg.       */
/* ------------------------------------------------------------------ */
static volatile int g_mjpeg_stop = 0;

static void mjpeg_request_stop(void) { g_mjpeg_stop = 1; }

static void mjpeg_send_stream(int client_fd)
{
    char header[256];
    xmedia_s32 ret;
    xmedia_s32 venc_chn = 0;                       /* matches g_venc_cfg.chn_info[0].venc_chn */
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
/*  Main                                                               */
/* ------------------------------------------------------------------ */
int main(void)
{
    int ret;

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    srand((unsigned int)time(NULL));

    printf("============================================================\n");
    printf("  Rock-Paper-Scissors YOLOv8 for GK7206\n");
    printf("  Model: %s\n", RPS_MODEL_PATH);
    printf("  Input: %dx%d\n", RPS_WIDTH, RPS_HEIGHT);
    printf("  Web UI: http://<board-ip>/\n");
    printf("============================================================\n\n");

    ret = init_system();
    if (ret != XMEDIA_SUCCESS) {
        printf("System init failed: %d\n", ret);
        return ret;
    }

    ret = init_npu();
    if (ret != XMEDIA_SUCCESS) {
        printf("NPU init failed: %d\n", ret);
        deinit_system();
        return ret;
    }

    printf("Starting web server...\n");
    web_server_set_mjpeg_handler(mjpeg_send_stream, mjpeg_request_stop);
    web_server_run(project_route_get);

    deinit_npu();
    deinit_system();

    printf("Application exited\n");
    return 0;
}