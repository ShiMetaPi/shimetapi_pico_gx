#ifndef __SAMPLE_COMM_THRD_POOL_H__
#define __SAMPLE_COMM_THRD_POOL_H__

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include "xmedia_type.h"

// 任务结构体
typedef struct {
    xmedia_void (*function)(xmedia_void*);  // 任务函数
    xmedia_void *arg;                // 任务参数
} task_t;

// 线程池结构体
typedef struct {
    task_t *task_queue;         // 任务队列
    xmedia_s32 queue_capacity;       // 队列容量
    xmedia_s32 queue_size;           // 当前任务数量
    xmedia_s32 queue_front;          // 队头索引
    xmedia_s32 queue_rear;           // 队尾索引
    
    pthread_t *threads;       // 线程数组
    xmedia_s32 thread_count;         // 线程数量
    
    pthread_mutex_t mutex;    // 互斥锁
    pthread_cond_t cond;      // 条件变量
    xmedia_bool shutdown;            // 线程池关闭标志
} thread_pool_t;


thread_pool_t *sample_comm_thrd_pool_init(xmedia_s32 thread_count, xmedia_s32 queue_capacity);
xmedia_s32 sample_comm_thrd_job_submit(thread_pool_t* pool, xmedia_void (*function)(xmedia_void*), xmedia_void* arg);
xmedia_void *sample_comm_thrd_pool_worker(xmedia_void* arg);
xmedia_void sample_comm_thrd_pool_destroy(thread_pool_t* pool);
#endif
