/*
 * Copyright (c) XMEDIA. All rights reserved.
 */
/*
 * The main logic about frame cache module
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>

#include "log_base.h"
#include "events.h"
#include "config_svc.h"
#include "frame_cache.h"

static void cache_init(frame_cache_t *cache)
{
    cache->tail  = NULL;
    cache->head  = NULL;
    cache->count = 0;
}

static void cache_destroy(frame_cache_t *cache)
{
    frame_node_t *tmp = NULL;

    while (cache->tail != NULL) {
        tmp = cache->tail;
        cache->tail = tmp->next;
        free(tmp->mem);
        free(tmp);
    }

    cache->tail  = NULL;
    cache->head  = NULL;
    cache->count = 0;
}

static int put_frame_into_cache(frame_cache_t *cache, frame_node_t *node)
{
    node->next = NULL;
    cache->count++;

    // first time
    if (cache->head == NULL && cache->tail == NULL) {
        cache->head = node;
        cache->tail = node;
    } else {
        cache->head->next = node;
        cache->head = node;
    }

    return 0;
}

static int get_frame_from_cache(frame_cache_t *cache, frame_node_t **node)
{
    *node = cache->tail;

    if ((cache->tail != NULL) && (cache->tail->next != NULL)) {
        cache->tail = cache->tail->next;
        cache->count--;
    } else if (cache->tail == cache->head) {
        cache->tail  = NULL;
        cache->head  = NULL;
        cache->count = 0;
    }

    return (*node != NULL) ? 0 : -1;
}

static int free_queue(frame_queue_t *queue)
{
    frame_cache_t *cache = queue->cache;

    if (cache != NULL) {
        cache_destroy(cache);
        free(cache);
        queue->cache = NULL;
    }

    pthread_mutex_destroy(&(queue->locker));

    return 0;
}

static int free_uvc_cache(uvc_cache_t *uvc_cache)
{
    /* free queue*/
    free_queue(uvc_cache->ok_queue);
    free_queue(uvc_cache->free_queue);

    free(uvc_cache->ok_queue);
    free(uvc_cache->free_queue);

    return 0;
}

static int free_uac_cache(uac_cache_t *uac_cache)
{
    /* free queue*/
    free_queue(uac_cache->ok_queue);
    free_queue(uac_cache->free_queue);

    free(uac_cache->ok_queue);
    free(uac_cache->free_queue);

    return 0;
}


static int init_queue(frame_queue_t **queue)
{
    frame_queue_t *q = *queue;

    if (q == NULL) {
        goto ERR;
    }

    q->cache = (frame_cache_t *)malloc(sizeof(frame_cache_t));
    if (q->cache == NULL) {
        LOGE("malloc frame cache failure\n");
        goto ERR;
    }

    cache_init(q->cache);

    /*FIXME: recursive locker....*/
    pthread_mutex_init(&(q->locker), NULL);

    return 0;

ERR:
    return -1;
}

static int init_uvc_frame_cache(uvc_cache_t **cache)
{
    uvc_cache_t *uvc_cache = *cache;

    if (uvc_cache == NULL) {
        LOGE("the uvc cache is null\n");
        goto ERR;
    }

    uvc_cache->ok_queue = NULL;
    uvc_cache->free_queue = NULL;

    uvc_cache->ok_queue = (frame_queue_t *)malloc(sizeof(frame_queue_t));
    if (uvc_cache->ok_queue == NULL) {
        LOGE("malloc ok_queue failure \n");
        goto ERR;
    }

    uvc_cache->free_queue = (frame_queue_t *)malloc(sizeof(frame_queue_t));
    if (uvc_cache->free_queue == NULL) {
        LOGE("malloc free_queue failure\n");
        goto ERR;
    }

    if (init_queue(&(uvc_cache->ok_queue)) < 0) {
        goto ERR;
    }

    if (init_queue(&(uvc_cache->free_queue)) < 0) {
        goto ERR;
    }

    return 0;

ERR:
    if (uvc_cache->ok_queue != NULL) {
        free(uvc_cache->ok_queue);
        uvc_cache->ok_queue = NULL;
    }

    if (uvc_cache->free_queue != NULL) {
        free(uvc_cache->free_queue);
        uvc_cache->free_queue = NULL;
    }

    return -1;
}

static int init_uac_frame_cache(uac_cache_t **cache)
{
    uac_cache_t *uac_cache = *cache;

    if (uac_cache == NULL) {
        LOGE("the uac cache is null\n");
        goto ERR;
    }

    uac_cache->ok_queue   = NULL;
    uac_cache->free_queue = NULL;

    uac_cache->ok_queue = (frame_queue_t *)malloc(sizeof(frame_queue_t));
    if (uac_cache->ok_queue == NULL) {
        LOGE("malloc ok_queue failure \n");
        goto ERR;
    }

    uac_cache->free_queue = (frame_queue_t *)malloc(sizeof(frame_queue_t));
    if (uac_cache->free_queue == NULL) {
        LOGE("malloc free_queue failure\n");
        goto ERR;
    }

    if (init_queue(&(uac_cache->ok_queue)) < 0) {
        goto ERR;
    }

    if (init_queue(&(uac_cache->free_queue)) < 0) {
        goto ERR;
    }

    return 0;

ERR:
    if (uac_cache->ok_queue != NULL) {
        free(uac_cache->ok_queue);
        uac_cache->ok_queue = NULL;
    }

    if (uac_cache->free_queue != NULL) {
        free(uac_cache->free_queue);
        uac_cache->free_queue = NULL;
    }

    return -1;
}

static void clear_frame_cache(frame_cache_t *c)
{
    cache_destroy(c);
}

static void clear_queue(frame_queue_t *q)
{
    if (0 != pthread_mutex_lock(&(q->locker))) {
        LOGE("failed to lock frame cache\n");
        goto ERR;
    }

    clear_frame_cache(q->cache);

    pthread_mutex_unlock(&(q->locker));

ERR:
    return;
}

static int create_cache_node_list(frame_queue_t *q, unsigned int buffer_size, int cache_count)
{
    int i = 0;
    unsigned int page_size;
    frame_node_t *n = NULL;

    page_size = getpagesize();

    buffer_size = (buffer_size + page_size - 1) & ~(page_size - 1);

    for (; i < cache_count; ++i) {
        n = (frame_node_t *)malloc(sizeof(frame_node_t));
        if (!n) {
            LOGE("failed to malloc frame_node_t\n");
            goto ERR;
        }

        n->mem = (unsigned char *)memalign(page_size, buffer_size);
        if (!n->mem) {
            LOGE("failed to malloc frame_node_t->mem field\n");
            goto ERR;
        }

        n->length = buffer_size;
        n->used = 0;
        n->next = NULL;
        n->index = i;

        put_node_to_queue(q, n);
    }

    return 0;

ERR:

    if (n) {
        free(n);
    }

    return -1;
}

//only one instance for each process.
static uvc_cache_t *g_uvc_cache = NULL;
static uac_cache_t *g_uac_cache = NULL;

uvc_cache_t *uvc_cache_get(void)
{
    return g_uvc_cache;
}

int create_uvc_cache(void)
{
    unsigned int buffer_size;
    int cache_count;

    g_uvc_cache = (uvc_cache_t *)malloc(sizeof(uvc_cache_t));
    if (g_uvc_cache == NULL) {
        LOGE("malloc uvc_cache failure\n");
        goto ERR;
    }

    if (init_uvc_frame_cache(&g_uvc_cache) < 0) {
        LOGE("init uvc cache failure\n");
        goto ERR;
    }

    buffer_size = get_config_value("uvc:imagesize", 1843200);
    cache_count = get_config_value("uvc:cache_count", 6);

    create_cache_node_list(g_uvc_cache->free_queue, buffer_size, cache_count);

    events_init();

    return 0;

ERR:
    if (g_uvc_cache != NULL) {
        free(g_uvc_cache);
        g_uvc_cache = NULL;
    }

    return -1;
}

void destroy_uvc_cache(void)
{
    if (uvc_cache_get() != NULL) {
        free_uvc_cache(uvc_cache_get());
        free(uvc_cache_get());
        events_deinit();

        g_uvc_cache = NULL;
    }
}

void clear_uvc_cache(void)
{
    if (uvc_cache_get() != NULL) {
        uvc_cache_t* uvc_cache = uvc_cache_get();

        clear_queue(uvc_cache->free_queue);
        clear_queue(uvc_cache->ok_queue);
    }
}

uac_cache_t* uac_cache_get(void)
{
    return g_uac_cache;
}

int create_uac_cache(void)
{
    int cache_count;
    unsigned int buffer_size;

    g_uac_cache = (uac_cache_t *)malloc(sizeof(uac_cache_t));

    if (g_uac_cache == NULL) {
        LOGE("malloc uac_cache failure\n");
        goto ERR;
    }

    if (init_uac_frame_cache(&g_uac_cache) < 0) {
        LOGE("init uac cache failure\n");
        goto ERR;
    }

    buffer_size = get_config_value("uac:imagesize", 1024);
    cache_count = get_config_value("uac:cache_count", 6);

    create_cache_node_list(g_uac_cache->free_queue, buffer_size, cache_count);

    return 0;

ERR:
    if (g_uac_cache != NULL) {
        free(g_uac_cache);
    }

    g_uac_cache = NULL;

    return -1;
}

void destroy_uac_cache(void)
{
    if (uac_cache_get() != NULL) {
        free_uac_cache(uac_cache_get());
        free(uac_cache_get());

        g_uac_cache = NULL;
    }
}

void clear_uac_cache(void)
{
    if (uac_cache_get() != NULL) {
        uac_cache_t * uac_cache = uac_cache_get();

        clear_queue(uac_cache->free_queue);
        clear_queue(uac_cache->ok_queue);
    }
}

int put_node_to_queue(frame_queue_t *q, frame_node_t* node)
{
    if ((q == NULL) || (node == NULL)) {
        goto ERR;
    }

    if (0 != pthread_mutex_lock(&(q->locker))) {
        LOGE("failed to lock frame cache\n");
        goto ERR;
    }

    if (0 != put_frame_into_cache(q->cache, node)) {
        pthread_mutex_unlock(&(q->locker));

        goto ERR;
    }

    pthread_mutex_unlock(&(q->locker));

    if (g_uvc_cache && g_uvc_cache->ok_queue == q) {
        node_notity_one();
    }

    return 0;

ERR:
    return -1;
}

int get_node_from_queue(frame_queue_t *q, frame_node_t** node)
{
    if ((q == NULL) || (node == NULL)) {
        goto ERR;
    }

    if (0 != pthread_mutex_lock(&(q->locker))) {
        LOGE("failed to lock frame cache\n");
        goto ERR;
    }

    if (0 != get_frame_from_cache(q->cache, node)) {
        pthread_mutex_unlock(&(q->locker));

        goto ERR;
    }

    pthread_mutex_unlock(&(q->locker));

    return 0;
ERR:
    return -1;
}

int wait_queue(frame_queue_t *q)
{
    if (q == NULL) {
        goto ERR;
    }

ERR:
    return -1;
}

void debug_dump_node(frame_node_t *node)
{
    printf("0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
            node->mem[0],
            node->mem[1],
            node->mem[2],
            node->mem[3],
            node->mem[4],
            node->mem[5]);
}

void clear_ok_queue(void)
{
    frame_node_t* node = NULL;
    uvc_cache_t* uvc_cache = uvc_cache_get();

    events_init();
    while (0 == get_node_from_queue(uvc_cache->ok_queue, &node)) {
        node->used = 0;
        put_node_to_queue(uvc_cache->free_queue, node);
    }
}
