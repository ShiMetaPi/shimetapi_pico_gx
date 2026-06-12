/*
 * Copyright (c) XCAM. All rights reserved.
 *
 * This file contains the media buffer implementations for media stream.
 */

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>

#include "list.h"
#include "xcam_log.h"
#include "xcam_mbuf.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/*macro definition */
#define MBUF_DEBUG 0

#define MBUF_INFO(fmt...)      XCAM_LOG_INFO_PRINT("MBUF", fmt)
#define MBUF_WARN(fmt...)      XCAM_LOG_WARN_PRINT("MBUF", fmt)
#define MBUF_ERROR(fmt...)     XCAM_LOG_ERR_PRINT("MBUF", fmt)
#define MBUF_FATAL(fmt...)     XCAM_LOG_FATAL_PRINT("MBUF", fmt)

#define MBUF_ALIGN(x, a)          (((x) + ((a) - 1)) & ~((a) - 1))
#define MBUF_PAYLOAD_READABLE     0x01
#define MBUF_PAYLOAD_WRITABLE     0x02
#define MBUF_INVALID_64BITS_PTS   ((xcam_u64)(-1))

#define MBUF_MALLOC(addr)         malloc(addr)
#define MBUF_FREE(addr)           free(addr)

#define MBUF_MUTEX_LOCK(mutex)    pthread_mutex_lock(&mutex)
#define MBUF_MUTEX_UNLOCK(mutex)  pthread_mutex_unlock(&mutex)

#define IS_READING(block) (MBUF_BLOCK_READING == block->block_status)
#define IS_WRITING(block) (MBUF_BLOCK_WRITING == block->block_status)
#define IS_FREE(block)    (MBUF_BLOCK_FREE == block->block_status)

#define IS_HEAD_OF_LIST(block, inst) (&block->block_node == inst->block_head.next)
#define IS_FULL(inst, size) (inst->free < size)

#define MBUF_ASSERT_RETURN(condition, ret_value) \
    do { \
        if (!(condition)) { \
            MBUF_WARN("[%s %d]assert warning\n",__func__,__LINE__); \
            return ret_value; \
        } \
    }while(0)

#define MBUF_FIND_BLOCK_BY_ADDR(addr, inst, block) \
    { \
        struct list_head* pos; \
        struct list_head* n; \
        cam_mbuf_block_info* tmp; \
        list_for_each_safe(pos, n, &inst->block_head) { \
            tmp = list_entry(pos, cam_mbuf_block_info, block_node); \
            if (addr == tmp->addr) { \
                block = tmp; \
                break; \
            } \
        } \
    }

#define MBUF_FREE_BLOCK_LIST(inst) \
    { \
        struct list_head* pos; \
        struct list_head* n; \
        list_for_each_safe(pos, n, &inst->block_head) { \
            list_del(pos); \
        } \
    }

#define MBUF_FIND_TAIL_BLOCK(inst, block) \
    { \
        block = list_entry(inst->block_head.prev, cam_mbuf_block_info, block_node); \
    }

#define MBUF_FIND_HEAD_BLOCK(inst, block) \
    { \
        block = list_entry(inst->block_head.next, cam_mbuf_block_info, block_node); \
    }

typedef enum {
    MBUF_BLOCK_FREE = 0,
    MBUF_BLOCK_READING,
    MBUF_BLOCK_WRITING,
    MBUF_BLOCK_BUTT
} cam_mbuf_block_status;

typedef struct {
    xcam_u8 *addr;              /* User virtual address */
    xcam_u32 size;              /* Buffer size, in the unit of byte */
} cam_mbuf_buf_info;

/* Describe a buffer block */
typedef struct {
    xcam_u8  *addr;             /* base address of block */
    xcam_u32  size;             /* size of block */
    xcam_u64  pts;
    xcam_u32  seq;
    xcam_u16  payload_type;    /* mbuffer data payload type */
    xcam_u8   key_frame;       /* is used to video */
    xcam_u8   pack_count;      /* pack_count < XCAM_MBUF_MAX_PACK */
    xcam_u32  pack_size[XCAM_MBUF_MAX_PACK];    /* pack size of pre pack */

    cam_mbuf_block_status block_status; /* Status of block */
    struct list_head block_node;        /* Block list node */
} cam_mbuf_block_info;

typedef struct {
    xcam_u16   payload_type;     /* mbuffer data payload type */
    xcam_u8    mode;             /* readable,writable */
    xcam_u8    pad;              /* reserved */
} cam_mbuf_payload_attr;

/* Describe a buffer instance */
typedef struct {
    xcam_u8   *addr;            /* Start user virtual address of the buffer instance */
    xcam_u32   size;            /* Size of the buffer instance */
    xcam_u32   payload_count;   /* payload type count of the mbuffer */
    cam_mbuf_payload_attr *payload_attr;  /* readable,writable of the payload */

    xcam_u32   used;            /* Used size */
    xcam_u32   free;            /* Free size */
    xcam_u32   freeze;          /* Freeze size */
    pthread_mutex_t  mutex_lock;   /* mutex lock */
    struct list_head block_head;   /* Buffer manager block list head */
} cam_mbuf_inst;

/*static function */
static xcam_s32 cam_mbuf_get_write_buffer(xcam_handle *handle, cam_mbuf_buf_info *buf_info)
{
    cam_mbuf_inst *buf_inst   = XCAM_NULL;
    cam_mbuf_block_info *block = XCAM_NULL;
    cam_mbuf_block_info *block_head = XCAM_NULL;
    xcam_u32 read_offset;
    xcam_u32 write_offset;
    xcam_u32 tail_free;
    xcam_u32 head_free;
    xcam_u32 acq_size;
    xcam_bool is_alloc = XCAM_FALSE;

    MBUF_ASSERT_RETURN(buf_info != XCAM_NULL, XCAM_ERR_MBUF_PARAM_INVALID);
    MBUF_ASSERT_RETURN(buf_info->size > 0, XCAM_ERR_MBUF_PARAM_INVALID);

    buf_inst = (cam_mbuf_inst *)handle;
    MBUF_MUTEX_LOCK(buf_inst->mutex_lock);
    /*Data align*/
    buf_info->size = MBUF_ALIGN(buf_info->size,sizeof(xcam_u32));
    /* Buffer full */
    acq_size = buf_info->size + sizeof(cam_mbuf_block_info);
    if (IS_FULL(buf_inst, acq_size)) {
        MBUF_MUTEX_UNLOCK(buf_inst->mutex_lock);
        return XCAM_ERR_MBUF_BUFFER_FULL;
    }

    /* If empty, write from start */
    if (list_empty(&buf_inst->block_head)) {
        write_offset = read_offset = 0;
    } else { /*Find write offset and read offset,only support one writer*/
        MBUF_FIND_TAIL_BLOCK(buf_inst, block);
        if (IS_WRITING(block)) {
            buf_info->addr = block->addr + sizeof(cam_mbuf_block_info);
            buf_info->size = block->size - sizeof(cam_mbuf_block_info);

            MBUF_MUTEX_UNLOCK(buf_inst->mutex_lock);
            MBUF_ERROR("mbuf block is busy:%p,size:%d\n",buf_info->addr,buf_info->size);

            return XCAM_ERR_MBUF_BUSY;
        }
        write_offset = (xcam_u32)block->addr + block->size - (xcam_u32)buf_inst->addr;

        MBUF_FIND_HEAD_BLOCK(buf_inst, block_head);
        read_offset = (xcam_u32)block_head->addr - (xcam_u32)buf_inst->addr;
    }

    /* Reverse: write pointer before read pointer, the free area is continuous */
    if (write_offset <= read_offset) {
        is_alloc = XCAM_TRUE;
    } else { /* Normal: write pointer after read pointer, the free area isn't continuous */
        tail_free = buf_inst->size - write_offset;
        head_free = buf_inst->free - tail_free;

        if (tail_free >= acq_size) {
            is_alloc = XCAM_TRUE;
        } else if (head_free >= acq_size) {
            is_alloc = XCAM_TRUE;

            /* Alloc from head */
            write_offset = 0;

            /* Freeze the last area */
            buf_inst->freeze = tail_free;
            buf_inst->free -= tail_free;
        }
    }

    /* Allocate fail, return */
    if (!is_alloc) {
        MBUF_MUTEX_UNLOCK(buf_inst->mutex_lock);
        return XCAM_ERR_MBUF_BUFFER_FULL;
    }

    /* Allocate new block,init block parameter*/
    block = (cam_mbuf_block_info *)(buf_inst->addr + write_offset);
    block->addr = (xcam_u8 *)block;

    buf_info->addr = block->addr + sizeof(cam_mbuf_block_info);
    block->size = acq_size;
    block->pts = MBUF_INVALID_64BITS_PTS;
    block->block_status = MBUF_BLOCK_WRITING;

    /* Add block to list */
    list_add_tail(&block->block_node, &buf_inst->block_head);
    MBUF_MUTEX_UNLOCK(buf_inst->mutex_lock);

    return XCAM_SUCCESS;
}

static xcam_s32 cam_mbuf_put_write_buffer(xcam_handle *handle, const xcam_mbuf_pack_info *pack_info, cam_mbuf_buf_info *buf_info)
{
    cam_mbuf_inst *buf_inst   = XCAM_NULL;
    cam_mbuf_block_info *block = XCAM_NULL;
    xcam_u32 write_size;
    xcam_u8 i;

    MBUF_ASSERT_RETURN(buf_info != XCAM_NULL, XCAM_ERR_MBUF_PARAM_INVALID);
    MBUF_ASSERT_RETURN(buf_info->size > 0, XCAM_ERR_MBUF_PARAM_INVALID);

    buf_inst = (cam_mbuf_inst *)handle;
    MBUF_MUTEX_LOCK(buf_inst->mutex_lock);

    /* Find the tail block */
    MBUF_FIND_TAIL_BLOCK(buf_inst, block);

    write_size = buf_info->size + sizeof(cam_mbuf_block_info);
    if (write_size > block->size) {
        MBUF_MUTEX_UNLOCK(buf_inst->mutex_lock);
        MBUF_ERROR("Put size err:%d>%d\n", write_size, block->size);
        return XCAM_ERR_MBUF_PUT_BUF_FAILED;
    }

    /* The block must be WRITING status and its address must be right */
    if (IS_WRITING(block) && ((xcam_u32)(buf_info->addr - sizeof(cam_mbuf_block_info)) == (xcam_u32)block->addr)) {
        /* If size=0, drop this block */
        if (0 == buf_info->size) {
            /* Delete block from list */
            list_del(&block->block_node);
            MBUF_MUTEX_UNLOCK(buf_inst->mutex_lock);
            return XCAM_SUCCESS;
        }

        block->pts = pack_info->pts;
        block->seq = pack_info->seq;
        block->key_frame = pack_info->key_frame;
        block->payload_type = pack_info->payload_type;
        block->pack_count = pack_info->pack_count;

        for (i = 0; i < pack_info->pack_count; i++) {
            block->pack_size[i] = pack_info->pack_size[i];
        }

        block->block_status = MBUF_BLOCK_FREE;
    } else {
        MBUF_MUTEX_UNLOCK(buf_inst->mutex_lock);
        MBUF_ERROR("Put fail.\n");
        return XCAM_ERR_MBUF_PUT_BUF_FAILED;
    }

    /* Manage instance */
    buf_inst->used += write_size;
    buf_inst->free -= write_size;

#if (MBUF_DEBUG == 1)
    if (buf_inst->size != buf_inst->used + buf_inst->free + buf_inst->freeze) {
        MBUF_FATAL("Error:unmatch, size:%d!= used:%d+free:%d+freeze:%d\n",buf_inst->size,buf_inst->used,
                     buf_inst->free,buf_inst->freeze);
    }
#endif
    MBUF_MUTEX_UNLOCK(buf_inst->mutex_lock);

    return XCAM_SUCCESS;
}

static xcam_s32 cam_mbuf_acq_read_buffer(xcam_handle *handle, xcam_u16 payload_type, xcam_mbuf_pack_info *pack_info)
{
    cam_mbuf_inst *buf_inst   = XCAM_NULL;
    cam_mbuf_block_info *buf_block = XCAM_NULL;
    xcam_u32 offset;
    xcam_u8 i;

    MBUF_ASSERT_RETURN(pack_info != XCAM_NULL, XCAM_ERR_MBUF_PARAM_INVALID);

    buf_inst = (cam_mbuf_inst *)handle;
    MBUF_MUTEX_LOCK(buf_inst->mutex_lock);

    /* Buffer empty */
    if ((0 == buf_inst->used) || (list_empty(&buf_inst->block_head))) {
        MBUF_MUTEX_UNLOCK(buf_inst->mutex_lock);
        return XCAM_ERR_MBUF_BUFFER_EMPTY;
    }

    /* Find head block */
    MBUF_FIND_HEAD_BLOCK(buf_inst, buf_block);

    /* The block must be FREE status */
    if (!IS_FREE(buf_block)) {
        MBUF_MUTEX_UNLOCK(buf_inst->mutex_lock);
        return XCAM_ERR_MBUF_BUFFER_EMPTY;
    }

    /* Read from the block */
    pack_info->pts = buf_block->pts;
    pack_info->seq = buf_block->seq;
    pack_info->key_frame = buf_block->key_frame;
    pack_info->payload_type = buf_block->payload_type;
    pack_info->pack_count = buf_block->pack_count;

    offset = 0;
    for (i = 0;i < pack_info->pack_count;i++) {
        pack_info->pack_addr[i] = buf_block->addr + sizeof(cam_mbuf_block_info) + offset;
        pack_info->pack_size[i] = buf_block->pack_size[i];
        offset += buf_block->pack_size[i];
    }

    /* Change status */
    buf_block->block_status = MBUF_BLOCK_READING;
    MBUF_MUTEX_UNLOCK(buf_inst->mutex_lock);

    return XCAM_SUCCESS;
}

static xcam_s32 cam_mbuf_read_buffer_over(xcam_handle *handle,  xcam_u16 payload_type)
{
    cam_mbuf_inst *buf_inst   = XCAM_NULL;
    cam_mbuf_block_info *block = XCAM_NULL;

    MBUF_ASSERT_RETURN(handle != XCAM_NULL, XCAM_ERR_MBUF_PARAM_INVALID);

    buf_inst = (cam_mbuf_inst *)handle;
    MBUF_MUTEX_LOCK(buf_inst->mutex_lock);

    /* Find head block */
    MBUF_FIND_HEAD_BLOCK(buf_inst, block);
    if (XCAM_NULL == block) {
        MBUF_MUTEX_UNLOCK(buf_inst->mutex_lock);
        return XCAM_ERR_MBUF_RELEASE_FAILED;
    }

    /* Change status */
    if (IS_READING(block)) {
        block->block_status = MBUF_BLOCK_FREE;
    }

    MBUF_MUTEX_UNLOCK(buf_inst->mutex_lock);

    return XCAM_SUCCESS;
}

xcam_s32 cam_mbuf_release_read_buffer(xcam_handle *handle, cam_mbuf_buf_info *buf_info)
{
    cam_mbuf_inst *buf_inst   = XCAM_NULL;
    cam_mbuf_block_info *block = XCAM_NULL;

    MBUF_ASSERT_RETURN(buf_info != XCAM_NULL, XCAM_ERR_MBUF_PARAM_INVALID);

    buf_inst = (cam_mbuf_inst *)handle;
    MBUF_MUTEX_LOCK(buf_inst->mutex_lock);

    if (buf_info == XCAM_NULL) {
        /* Find head block */
        MBUF_FIND_HEAD_BLOCK(buf_inst, block);
    } else {
        /* Find block by address */
        xcam_u8 *addr = buf_info->addr - sizeof(cam_mbuf_block_info);
        MBUF_FIND_BLOCK_BY_ADDR(addr, buf_inst, block);
    }

    if (XCAM_NULL == block) {
        MBUF_MUTEX_UNLOCK(buf_inst->mutex_lock);
        return XCAM_ERR_MBUF_READ_FAILED;
    }

    /* Unfreeze */
    if ((0 != buf_inst->freeze) &&
        (block->addr + block->size == buf_inst->addr + buf_inst->size - buf_inst->freeze)) {
        buf_inst->free  += buf_inst->freeze;
        buf_inst->freeze = 0;
    }

    /* Only support free orderly */
    if (IS_HEAD_OF_LIST(block, buf_inst) && IS_READING(block)) {
        /* Change parameter of instance */
        buf_inst->used -= block->size;
        buf_inst->free += block->size;

        /* Delete block from list */
        list_del(&block->block_node);
    } else {
        MBUF_MUTEX_UNLOCK(buf_inst->mutex_lock);
        return XCAM_ERR_MBUF_READ_FAILED;
    }

#if (MBUF_DEBUG == 1)
    if (buf_inst->size != buf_inst->used + buf_inst->free + buf_inst->freeze) {
        MBUF_FATAL("Error:unmatch,size:%d!=used:%d+free:%d+freeze:%d\n",buf_inst->size,buf_inst->used,
                    buf_inst->free,buf_inst->freeze);
    }
#endif

    MBUF_MUTEX_UNLOCK(buf_inst->mutex_lock);

    return XCAM_SUCCESS;
}

static xcam_s32 cam_mbuf_release_buffer(xcam_handle *handle,  xcam_u16 payload_type)
{
    cam_mbuf_inst *buf_inst   = XCAM_NULL;
    cam_mbuf_block_info *block = XCAM_NULL;

    MBUF_ASSERT_RETURN(handle != XCAM_NULL, XCAM_ERR_MBUF_PARAM_INVALID);

    buf_inst = (cam_mbuf_inst *)handle;
    MBUF_MUTEX_LOCK(buf_inst->mutex_lock);

    /* Find head block */
    MBUF_FIND_HEAD_BLOCK(buf_inst, block);
    if (XCAM_NULL == block) {
        MBUF_MUTEX_UNLOCK(buf_inst->mutex_lock);
        return XCAM_ERR_MBUF_RELEASE_FAILED;
    }

    /* Unfreeze */
    if ((0 != buf_inst->freeze) &&
        (block->addr + block->size == buf_inst->addr + buf_inst->size - buf_inst->freeze)) {
        buf_inst->free  += buf_inst->freeze;
        buf_inst->freeze = 0;
    }

    /* Only support free orderly */
    if (IS_READING(block) || IS_FREE(block)) {
        /* Change parameter of instance */
        buf_inst->used -= block->size;
        buf_inst->free += block->size;

        /* Delete block from list */
        list_del(&block->block_node);
    } else {
        MBUF_MUTEX_UNLOCK(buf_inst->mutex_lock);
        return XCAM_ERR_MBUF_RELEASE_FAILED;
    }

#if (MBUF_DEBUG == 1)
    if (buf_inst->size != buf_inst->used + buf_inst->free + buf_inst->freeze) {
        MBUF_FATAL("Error:unmatch,size:%d!=used:%d+free:%d+freeze:%d\n",buf_inst->size,buf_inst->used,
                    buf_inst->free,buf_inst->freeze);
    }
#endif

    MBUF_MUTEX_UNLOCK(buf_inst->mutex_lock);

    return XCAM_SUCCESS;
}


static xcam_s32 cam_mbuf_reset(xcam_handle *handle)
{
    cam_mbuf_inst *buf_inst = XCAM_NULL;

    MBUF_ASSERT_RETURN(handle != XCAM_NULL, XCAM_ERR_MBUF_PARAM_INVALID);

    buf_inst = (cam_mbuf_inst *)handle;
    MBUF_MUTEX_LOCK(buf_inst->mutex_lock);

    /* Free all block */
    MBUF_FREE_BLOCK_LIST(buf_inst);

    /* Set used/free/freeze size */
    buf_inst->used    = 0;
    buf_inst->free    = buf_inst->size;
    buf_inst->freeze  = 0;

    MBUF_MUTEX_UNLOCK(buf_inst->mutex_lock);

    return XCAM_SUCCESS;
}

static xcam_bool cam_mbuf_check_payload_valid(xcam_handle *handle, xcam_u16 payload_type, xcam_u8 rw_mode)
{
    cam_mbuf_inst *buf_inst = XCAM_NULL;
    cam_mbuf_payload_attr *attr;
    xcam_bool ret = XCAM_FALSE;
    xcam_s32 i;

    MBUF_ASSERT_RETURN(handle != XCAM_NULL, XCAM_FALSE);

    buf_inst = (cam_mbuf_inst *)handle;
    MBUF_MUTEX_LOCK(buf_inst->mutex_lock);

    buf_inst = (cam_mbuf_inst *)handle;
    attr = (cam_mbuf_payload_attr *)buf_inst->payload_attr;
    for (i = 0; i < buf_inst->payload_count; i++) {
        if ((payload_type == XCAM_MBUF_PAYLOAD_TYPE_ALL) || (payload_type == attr[i].payload_type)) {
            if ((attr[i].mode & rw_mode) == rw_mode) {
                ret = XCAM_TRUE;
            } else {
                ret = XCAM_FALSE;
                MBUF_ERROR("Payload type:%hu,mode:%hhu is invalid\n",payload_type,attr[i].mode);
            }
            MBUF_MUTEX_UNLOCK(buf_inst->mutex_lock);
            return ret;
        }
    }
    MBUF_ERROR("Payload type:%hu not found\n",payload_type);
    MBUF_MUTEX_UNLOCK(buf_inst->mutex_lock);

    return ret;
}

xcam_s32 xcam_mbuf_get_buffer(xcam_handle **handle, xcam_mbuf_cfg *buf_cfg)
{
    xcam_u8* buf_addr;
    cam_mbuf_inst* buf_inst = XCAM_NULL;
    xcam_u32 attr_size;
    int ret;
    int i;

    MBUF_ASSERT_RETURN(buf_cfg != XCAM_NULL, XCAM_ERR_MBUF_PARAM_INVALID);
    MBUF_ASSERT_RETURN(buf_cfg->buf_size != 0, XCAM_ERR_MBUF_PARAM_INVALID);
    if ((buf_cfg->payload_count == 0) || (buf_cfg->payload_count > XCAM_MBUF_MAX_PACK)) {
        return XCAM_ERR_MBUF_PARAM_INVALID;
    }

    /* Allocate an instance */
    attr_size = buf_cfg->payload_count * sizeof(cam_mbuf_payload_attr);
    if ((buf_inst = (cam_mbuf_inst*)MBUF_MALLOC(sizeof(cam_mbuf_inst) + attr_size + buf_cfg->buf_size)) == XCAM_NULL) {
        MBUF_FATAL("Malloc failed. need buff size=%d KB\n", buf_cfg->buf_size/1024);
        return XCAM_ERR_MBUF_ALLOC_FAILED;
    }
    memset(buf_inst,0,sizeof(cam_mbuf_inst));

    buf_addr = (xcam_u8 *)buf_inst + sizeof(cam_mbuf_inst) + attr_size;

    /* Init instance parameter */
    buf_inst->addr = buf_addr;
    buf_inst->size = buf_cfg->buf_size;
    buf_inst->payload_count = buf_cfg->payload_count;
    buf_inst->payload_attr = (cam_mbuf_payload_attr *)((xcam_u8 *)buf_inst + sizeof(cam_mbuf_inst));
    for (i = 0; i < buf_inst->payload_count; i++) {
        buf_inst->payload_attr[i].payload_type = XCAM_MBUF_PAYLOAD_TYPE_INVALID;
        buf_inst->payload_attr[i].mode = 0;
    }

    if ((ret = pthread_mutex_init(&buf_inst->mutex_lock,XCAM_NULL)) != 0) {
        MBUF_FATAL("pthread_mutex_init error ret : %x\n",ret);
        MBUF_FREE(buf_inst);

        return XCAM_ERR_MBUF_MUTEX_INIT_FAILED;
    }

    /* Init block list parameter */
    buf_inst->free   = buf_inst->size;
    buf_inst->freeze = 0;
    buf_inst->used    = 0;
    INIT_LIST_HEAD(&buf_inst->block_head);

    *handle = (xcam_void *)buf_inst;

    return XCAM_SUCCESS;
}



xcam_s32 xcam_mbuf_release_buffer(xcam_handle *handle)
{
    cam_mbuf_inst *buf_inst = XCAM_NULL;

    MBUF_ASSERT_RETURN(handle != XCAM_NULL, XCAM_ERR_MBUF_PARAM_INVALID);

    buf_inst = (cam_mbuf_inst *)handle;
    MBUF_MUTEX_LOCK(buf_inst->mutex_lock);

    /* If has blocks, free them */
    if (!list_empty(&buf_inst->block_head)) {
        MBUF_FREE_BLOCK_LIST(buf_inst);
    }

    /* Free resource */
    MBUF_MUTEX_UNLOCK(buf_inst->mutex_lock);
    MBUF_FREE(buf_inst);

    return XCAM_SUCCESS;
}


xcam_s32 xcam_mbuf_register_payload(xcam_handle *handle, xcam_u16 payload_type)
{
    cam_mbuf_inst *buf_inst = XCAM_NULL;
    cam_mbuf_payload_attr *attr;
    xcam_s32 i;

    MBUF_ASSERT_RETURN(handle != XCAM_NULL, XCAM_ERR_MBUF_PARAM_INVALID);
    MBUF_ASSERT_RETURN(payload_type != XCAM_MBUF_PAYLOAD_TYPE_INVALID, XCAM_ERR_MBUF_PARAM_INVALID);

    buf_inst = (cam_mbuf_inst *)handle;
    MBUF_MUTEX_LOCK(buf_inst->mutex_lock);

    attr = (cam_mbuf_payload_attr *)buf_inst->payload_attr;
    for (i = 0; i < buf_inst->payload_count; i++) {
        if (attr[i].payload_type == XCAM_MBUF_PAYLOAD_TYPE_INVALID) {
            attr[i].payload_type = payload_type;
            attr[i].mode = MBUF_PAYLOAD_READABLE | MBUF_PAYLOAD_WRITABLE;
            MBUF_MUTEX_UNLOCK(buf_inst->mutex_lock);

            return XCAM_SUCCESS;
        }
    }
    MBUF_MUTEX_UNLOCK(buf_inst->mutex_lock);

    /*Register space is full*/
    return XCAM_FAILURE;
}

xcam_s32 xcam_mbuf_unregister_payload(xcam_handle *handle, xcam_u16 payload_type)
{
    cam_mbuf_inst *buf_inst = XCAM_NULL;
    cam_mbuf_payload_attr *attr;
    xcam_s32 i;

    MBUF_ASSERT_RETURN(handle != XCAM_NULL, XCAM_ERR_MBUF_PARAM_INVALID);
    MBUF_ASSERT_RETURN(payload_type != XCAM_MBUF_PAYLOAD_TYPE_INVALID, XCAM_ERR_MBUF_PARAM_INVALID);

    buf_inst = (cam_mbuf_inst *)handle;
    MBUF_MUTEX_LOCK(buf_inst->mutex_lock);

    attr = (cam_mbuf_payload_attr *)buf_inst->payload_attr;
    for (i = 0; i < buf_inst->payload_count; i++) {
        if (attr[i].payload_type == payload_type) {
            attr[i].payload_type = XCAM_MBUF_PAYLOAD_TYPE_INVALID;
            attr[i].mode = 0;
            MBUF_MUTEX_UNLOCK(buf_inst->mutex_lock);
            return XCAM_SUCCESS;
        }
    }
    MBUF_MUTEX_UNLOCK(buf_inst->mutex_lock);

    return XCAM_FAILURE;
}

xcam_s32 xcam_mbuf_set_rw_enable(xcam_handle *handle, xcam_u16 payload_type, xcam_bool enable_read, xcam_bool enable_write)
{
    cam_mbuf_inst *buf_inst = XCAM_NULL;
    cam_mbuf_payload_attr *attr;
    xcam_s32 i;

    MBUF_ASSERT_RETURN(handle != XCAM_NULL, XCAM_ERR_MBUF_PARAM_INVALID);
    MBUF_ASSERT_RETURN(payload_type != XCAM_MBUF_PAYLOAD_TYPE_INVALID, XCAM_ERR_MBUF_PARAM_INVALID);

    buf_inst = (cam_mbuf_inst *)handle;
    MBUF_MUTEX_LOCK(buf_inst->mutex_lock);
    attr = (cam_mbuf_payload_attr *)buf_inst->payload_attr;
    for (i = 0; i < buf_inst->payload_count; i++) {
        if (attr[i].payload_type == payload_type) {
            attr[i].mode = 0;
            if (enable_read)
                attr[i].mode |= MBUF_PAYLOAD_READABLE;
            if (enable_write)
                attr[i].mode |= MBUF_PAYLOAD_WRITABLE;
            MBUF_MUTEX_UNLOCK(buf_inst->mutex_lock);
            return XCAM_SUCCESS;
        }
    }
    MBUF_MUTEX_UNLOCK(buf_inst->mutex_lock);

    /*Not found payload type*/
    return XCAM_ERR_MBUF_NOT_REGISTERED;
}

xcam_s32 xcam_mbuf_get_rw_enable(xcam_handle *handle, xcam_u16 payload_type, xcam_bool *enable_read, xcam_bool *enable_write)
{
    cam_mbuf_inst *buf_inst = XCAM_NULL;
    cam_mbuf_payload_attr *attr;
    xcam_s32 i;

    MBUF_ASSERT_RETURN(handle != XCAM_NULL, XCAM_ERR_MBUF_PARAM_INVALID);
    MBUF_ASSERT_RETURN(payload_type != XCAM_MBUF_PAYLOAD_TYPE_INVALID, XCAM_ERR_MBUF_PARAM_INVALID);
    *enable_read = XCAM_FALSE;
    *enable_write = XCAM_FALSE;

    buf_inst = (cam_mbuf_inst *)handle;
    MBUF_MUTEX_LOCK(buf_inst->mutex_lock);
    attr = (cam_mbuf_payload_attr *)buf_inst->payload_attr;
    for (i = 0; i < buf_inst->payload_count; i++) {
        if (attr[i].payload_type == payload_type) {
            if (attr[i].mode & MBUF_PAYLOAD_READABLE) {
                *enable_read = XCAM_TRUE;
            }

            if (attr[i].mode & MBUF_PAYLOAD_WRITABLE) {
                *enable_write = XCAM_TRUE;
            }
            MBUF_MUTEX_UNLOCK(buf_inst->mutex_lock);
            return XCAM_SUCCESS;
        }
    }
    MBUF_MUTEX_UNLOCK(buf_inst->mutex_lock);

    /*Not found payload type*/
    return XCAM_ERR_MBUF_NOT_REGISTERED;

}

xcam_s32 xcam_mbuf_write_pack(xcam_handle *handle, const xcam_mbuf_pack_info *pack_info)
{
    xcam_u32 i = 0;
    xcam_s32 ret;
    cam_mbuf_buf_info buf_info;
    xcam_u8 *write_addr;
    xcam_u32 offset;

    /*Check payload type and rw mode*/
    if (!cam_mbuf_check_payload_valid(handle, pack_info->payload_type, MBUF_PAYLOAD_WRITABLE)) {
        return XCAM_FAILURE;
    }

    memset(&buf_info,0,sizeof(cam_mbuf_buf_info));
    for (i = 0; i < pack_info->pack_count; i++) {
        buf_info.size += pack_info->pack_size[i];
    }

    /*Get write buffer space*/
    ret = cam_mbuf_get_write_buffer(handle, &buf_info);
    if (ret == XCAM_ERR_MBUF_BUFFER_FULL) {
        MBUF_WARN("Mbuf is full!\n");
        cam_mbuf_reset(handle);
        ret = cam_mbuf_get_write_buffer(handle, &buf_info);
    }

    if (XCAM_SUCCESS != ret) {
        return ret;
    }

    write_addr = buf_info.addr;

    if (write_addr == XCAM_NULL) {
        return XCAM_FAILURE;
    }

    /*Write package data to buffer space*/
    offset = 0;
    for (i = 0; i < pack_info->pack_count; i++)
    {
        memcpy(write_addr + offset, pack_info->pack_addr[i], pack_info->pack_size[i]);
        offset += pack_info->pack_size[i];
    }

    /*Update buffer information*/
    ret = cam_mbuf_put_write_buffer(handle,pack_info, &buf_info);

    return ret;

}

xcam_s32 xcam_mbuf_read_pack(xcam_handle *handle, xcam_u16 payload_type, xcam_mbuf_pack_info *pack_info)
{
    xcam_s32 ret = XCAM_SUCCESS;

    /*Check payload type and rw mode*/
    if (!cam_mbuf_check_payload_valid(handle, payload_type, MBUF_PAYLOAD_READABLE))
    {
        return XCAM_ERR_MBUF_PAYLOAD_CHECK_ERR;
    }

    ret = cam_mbuf_acq_read_buffer(handle, payload_type, pack_info);

    return ret;
}

xcam_s32 xcam_mbuf_forward(xcam_handle *handle, xcam_u16 payload_type, xcam_u32 step)
{
    xcam_s32 ret = XCAM_SUCCESS;
    xcam_s32 i = 0;

    /*Restore buffer status from reading to free*/
    if (step == 0) {
        ret = cam_mbuf_read_buffer_over(handle,payload_type);
        return ret;
    }
    for (i = 0; i < step; i++) {
        if ((ret = cam_mbuf_release_buffer(handle, payload_type)) != XCAM_SUCCESS) {
            break;
        }
    }

    return ret;
}

xcam_s32 xcam_mbuf_debug(xcam_handle *handle, xcam_u16 payload_type)
{
    cam_mbuf_inst *buf_inst   = XCAM_NULL;
    cam_mbuf_block_info *block = XCAM_NULL;
    cam_mbuf_payload_attr *attr;
    struct list_head *pos;
    struct list_head *n;
    xcam_s32 i = 0;

    MBUF_ASSERT_RETURN(handle != XCAM_NULL,XCAM_ERR_MBUF_PARAM_INVALID);
    MBUF_ASSERT_RETURN(payload_type != XCAM_MBUF_PAYLOAD_TYPE_INVALID, XCAM_ERR_MBUF_PARAM_INVALID);

    buf_inst = (cam_mbuf_inst *)handle;
    MBUF_MUTEX_LOCK(buf_inst->mutex_lock);

    MBUF_INFO("---------------------mbuf info-----------------------\n");
    MBUF_INFO("addr:%p,size:%d =? total:%d(used:%d+free:%d+freeze:%d) \n", buf_inst->addr,buf_inst->size,
        buf_inst->used+buf_inst->free+buf_inst->freeze,buf_inst->used,buf_inst->free,buf_inst->freeze);

    attr = (cam_mbuf_payload_attr *)buf_inst->payload_attr;
    MBUF_INFO("register info:\n");
    for (i = 0; i < buf_inst->payload_count; i++) {
        MBUF_INFO("i:%d,type:%d,mode:%d\n",i,attr[i].payload_type,attr[i].mode);
    }

    if (list_empty(&buf_inst->block_head)) {
        MBUF_INFO("\tNone node\n");
    } else {
        list_for_each_safe(pos, n, &buf_inst->block_head)
        {
            block = list_entry(pos, cam_mbuf_block_info, block_node);
            MBUF_INFO("\tNode %d: addr=%p size=%d status=%d\n", i++, block->addr, block->size,
                           block->block_status);
        }
    }
    MBUF_INFO("----------------------------------------------------\n");

    MBUF_MUTEX_UNLOCK(buf_inst->mutex_lock);

    return XCAM_SUCCESS;
}


xcam_s32 xcam_mbuf_get_pack_data_offset(xcam_handle *handle, const xcam_mbuf_pack_info *pack_info, xcam_u32 *offset)
{
    return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


