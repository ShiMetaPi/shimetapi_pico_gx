#include "sample_comm_thrd_pool.h"

// 线程池工作函数
xmedia_void *sample_comm_thrd_pool_worker(xmedia_void* arg)
{
    thread_pool_t *pool = (thread_pool_t *)arg;
    
    while (1) {
        // 加锁
        pthread_mutex_lock(&pool->mutex);
        
        // 等待任务或关闭信号
        while (pool->queue_size == 0 && !pool->shutdown) {
            pthread_cond_wait(&pool->cond, &pool->mutex);
        }
        
        // 如果线程池要关闭，退出循环
        if (pool->shutdown == XMEDIA_TRUE) {
            pthread_mutex_unlock(&pool->mutex);
            break;
        }
        
        // 取出任务
        task_t task = pool->task_queue[pool->queue_front];
        pool->queue_front = (pool->queue_front + 1) % pool->queue_capacity;
        pool->queue_size--;
        
        // 解锁
        pthread_mutex_unlock(&pool->mutex);
        
        // 执行任务
        task.function(task.arg);
    }
    
    pthread_exit(NULL);
    //return;
}

// 初始化线程池
thread_pool_t *sample_comm_thrd_pool_init(xmedia_s32 thread_count, xmedia_s32 queue_capacity) 
{
    thread_pool_t *pool = (thread_pool_t *)malloc(sizeof(thread_pool_t));
    if (!pool) {
        perror("thread pool malloc failed!");
        return NULL;
    }
    
    // 初始化任务队列
    pool->queue_capacity = queue_capacity;
    pool->task_queue = (task_t *)malloc(sizeof(task_t) * queue_capacity);
    if (!pool->task_queue) {
        perror("task queue malloc failed!");
        free(pool);
        return NULL;
    }
    pool->queue_size = 0;
    pool->queue_front = 0;
    pool->queue_rear = 0;
    
    // 初始化线程数组
    pool->thread_count = thread_count;
    pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * thread_count);
    if (!pool->threads) {
        perror("thread handle malloc failed!");
        free(pool->task_queue);
        free(pool);
        return NULL;
    }
    
    // 初始化互斥锁和条件变量
    if (pthread_mutex_init(&pool->mutex, NULL) != 0 ||
        pthread_cond_init(&pool->cond, NULL) != 0) {
        perror("pthread_mutex_init or pthread_cond_init failed!");
        free(pool->threads);
        free(pool->task_queue);
        free(pool);
        return NULL;
    }
    
    pool->shutdown = XMEDIA_FALSE;
    
    // 创建工作线程
    for (int i = 0; i < thread_count; i++) {
        if (pthread_create(&pool->threads[i], NULL, sample_comm_thrd_pool_worker, pool) != 0) {
            perror("pthread_create failed!");
            // 清理已创建的资源
            pool->shutdown = XMEDIA_TRUE;
            for (int j = 0; j < i; j++) {
                pthread_join(pool->threads[j], NULL);
            }
            pthread_mutex_destroy(&pool->mutex);
            pthread_cond_destroy(&pool->cond);
            free(pool->threads);
            free(pool->task_queue);
            free(pool);
            return NULL;
        }
    }
    
    return pool;
}

// 向线程池提交任务
xmedia_s32 sample_comm_thrd_job_submit(thread_pool_t* pool, xmedia_void (*function)(xmedia_void*), xmedia_void* arg)
{
    if (!pool || !function) return XMEDIA_FAILURE;
    
    pthread_mutex_lock(&pool->mutex);
    
    // 检查队列是否已满
    if (pool->queue_size == pool->queue_capacity) {
        pthread_mutex_unlock(&pool->mutex);
        fprintf(stderr, "task queue is full,can not submit new task!\n");
        return XMEDIA_FAILURE;
    }
    
    // 添加任务到队列
    pool->task_queue[pool->queue_rear].function = function;
    pool->task_queue[pool->queue_rear].arg = arg;
    pool->queue_rear = (pool->queue_rear + 1) % pool->queue_capacity;
    pool->queue_size++;
    
    // 唤醒一个等待的线程
    pthread_cond_signal(&pool->cond);
    
    pthread_mutex_unlock(&pool->mutex);
    return XMEDIA_SUCCESS;
}

// 销毁线程池
xmedia_void sample_comm_thrd_pool_destroy(thread_pool_t* pool)
{
    if (!pool) return;
    
    // 标记线程池为关闭状态
    pthread_mutex_lock(&pool->mutex);
    pool->shutdown = XMEDIA_TRUE;
    pthread_mutex_unlock(&pool->mutex);
    
    // 唤醒所有等待的线程
    pthread_cond_broadcast(&pool->cond);
    
    // 等待所有线程结束
    for (int i = 0; i < pool->thread_count; i++) {
        pthread_join(pool->threads[i], NULL);
    }
    
    // 释放资源
    free(pool->threads);
    free(pool->task_queue);
    pthread_mutex_destroy(&pool->mutex);
    pthread_cond_destroy(&pool->cond);
    free(pool);
}

