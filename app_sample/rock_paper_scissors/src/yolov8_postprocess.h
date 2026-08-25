#ifndef __YOLOV8_POSTPROCESS_H__
#define __YOLOV8_POSTPROCESS_H__

#include <math.h>
#include <stdlib.h>

/*
 * YOLOv8 post-processing helpers for the split DFL output format used by the
 * GK7206 NPU export of the Rock-Paper-Scissors model.
 *
 * Output layout (6 tensors, NCHW uint8, per-tensor affine quant):
 *   box tensors: [1, 64, H, W]  where 64 = 4 box sides * reg_max(16) DFL bins
 *   cls tensors: [1,  3, H, W]  (3 classes: rock / paper / scissors)
 *   three scales: H = W = 80/40/20  <->  stride 8 / 16 / 32
 *
 * Reference: source/gmp/usr/svp/src/post_process/yolov8.c
 *            (compute_dfl, svp_yolov8_get_detect_result, yolov8_det_nms)
 */

#define YOLO_REG_MAX        16      /* DFL bins per box side   */
#define YOLO_NUM_CLASSES    3       /* rock / paper / scissors */

/* Detection in model input pixel coordinates ([0, input_size]). */
typedef struct {
    float x_min;      /* top-left x */
    float y_min;      /* top-left y */
    float x_max;      /* bottom-right x */
    float y_max;      /* bottom-right y */
    float score;      /* confidence (0..1, after sigmoid) */
    int   class_id;   /* 0=rock, 1=paper, 2=scissors */
} yolov8_detection_t;

/* Tunable parameters (defaults read from model_meta.txt). */
typedef struct {
    int   input_size;       /* model input W == H (640)            */
    int   num_classes;      /* 3                                   */
    int   reg_max;          /* 16                                  */
    float conf_threshold;   /* 0.25                                */
    float nms_threshold;    /* 0.45                                */
    int   max_detections;   /* cap on kept detections             */
} yolov8_param_t;

static inline void yolov8_default_param(yolov8_param_t *p)
{
    p->input_size     = 640;
    p->num_classes    = YOLO_NUM_CLASSES;
    p->reg_max        = YOLO_REG_MAX;
    p->conf_threshold = 0.05f;  /* DIAGNOSIS: lowered from 0.25 to expose all weak detections */
    p->nms_threshold  = 0.45f;
    p->max_detections = 32;
}

/* Affine dequantization of a uint8 tensor value to float. */
static inline float dequant_u8(unsigned char q, int zero_point, float scale)
{
    return ((float)q - (float)zero_point) * scale;
}

static inline float sigmoid_f(float x)
{
    return 1.0f / (1.0f + expf(-x));
}

/* DFL decode: softmax over reg_max bins then weighted sum by bin index.
 * `logits` points to reg_max consecutive float values for one box side.
 * Returns the decoded distance in [0, reg_max-1]. */
static inline float dfl_decode(const float *logits, int reg_max)
{
    float max_val = logits[0];
    int i;
    float exp_v[YOLO_REG_MAX];
    float exp_sum = 0.0f;
    float acc = 0.0f;

    for (i = 1; i < reg_max; i++) {
        if (logits[i] > max_val) max_val = logits[i];
    }
    for (i = 0; i < reg_max; i++) {
        exp_v[i] = expf(logits[i] - max_val);
        exp_sum += exp_v[i];
    }
    for (i = 0; i < reg_max; i++) {
        acc += (exp_v[i] / exp_sum) * (float)i;
    }
    return acc;
}

/* IoU between two axis-aligned boxes. */
static inline float compute_iou(const yolov8_detection_t *a, const yolov8_detection_t *b)
{
    float x1 = fmaxf(a->x_min, b->x_min);
    float y1 = fmaxf(a->y_min, b->y_min);
    float x2 = fminf(a->x_max, b->x_max);
    float y2 = fminf(a->y_max, b->y_max);

    float inter = fmaxf(0.0f, x2 - x1) * fmaxf(0.0f, y2 - y1);
    float area_a = fmaxf(0.0f, a->x_max - a->x_min) * fmaxf(0.0f, a->y_max - a->y_min);
    float area_b = fmaxf(0.0f, b->x_max - b->x_min) * fmaxf(0.0f, b->y_max - b->y_min);
    float uni = area_a + area_b - inter;

    return (uni > 1e-6f) ? (inter / uni) : 0.0f;
}

/* Greedy per-class NMS. Sorts `dets` (descending score) in place and compacts
 * the kept detections to the front. Returns the kept count (<= max_keep). */
static inline int yolov8_nms(yolov8_detection_t *dets, int num_dets,
                             float nms_thresh, int max_keep)
{
    int i, j, k, kept = 0;
    int *suppress = NULL;

    if (num_dets <= 0) return 0;

    /* Selection sort by score descending (typical N is small). */
    for (i = 0; i < num_dets - 1; i++) {
        int best = i;
        for (j = i + 1; j < num_dets; j++) {
            if (dets[j].score > dets[best].score) best = j;
        }
        if (best != i) {
            yolov8_detection_t tmp = dets[i];
            dets[i] = dets[best];
            dets[best] = tmp;
        }
    }

    suppress = (int *)calloc((size_t)num_dets, sizeof(int));
    if (suppress == NULL) return num_dets < max_keep ? num_dets : max_keep;

    for (i = 0; i < num_dets && kept < max_keep; i++) {
        if (suppress[i]) continue;
        kept++;
        for (j = i + 1; j < num_dets; j++) {
            if (suppress[j]) continue;
            if (dets[i].class_id != dets[j].class_id) continue;
            if (compute_iou(&dets[i], &dets[j]) > nms_thresh) suppress[j] = 1;
        }
    }

    /* Compact survivors to the front. */
    k = 0;
    for (i = 0; i < num_dets && k < max_keep; i++) {
        if (!suppress[i]) dets[k++] = dets[i];
    }

    free(suppress);
    return k;
}

#endif /* __YOLOV8_POSTPROCESS_H__ */
