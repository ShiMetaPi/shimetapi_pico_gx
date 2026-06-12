/*
 * Copyright (c) XMEDIA. All rights reserved.
 */
#include <sys/time.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include "bsp.h"

#include "bsp_thread.h"

/*** Mutex Port Begin **/

INT32 bsp_thread_mutex_init(XMEDIA_THREAD_MUTEX* pMutex, XMEDIA_THREAD_MUTEXATTR* pMutexattr)
{
    return (pthread_mutex_init(pMutex, pMutexattr));
}

INT32 bsp_thread_mutex_lock(XMEDIA_THREAD_MUTEX* pMutex)
{
    return (pthread_mutex_lock(pMutex));
}

INT32 bsp_thread_mutex_trylock(XMEDIA_THREAD_MUTEX* pMutex)
{
    return (pthread_mutex_trylock(pMutex));
}

INT32 bsp_thread_mutex_unlock(XMEDIA_THREAD_MUTEX* pMutex)
{
    return (pthread_mutex_unlock(pMutex));
}

INT32 bsp_thread_mutex_destroy(XMEDIA_THREAD_MUTEX* pMutex)
{
    return (pthread_mutex_destroy(pMutex));
}

INT32 bsp_thread_cond_init(XMEDIA_THREAD_COND* pCond, XMEDIA_THREAD_CONDATTR* pCondattr)
{
    return pthread_cond_init(pCond, pCondattr);
}

INT32 bsp_thread_cond_destroy(XMEDIA_THREAD_COND* pCond)
{
    return pthread_cond_destroy(pCond);
}
INT32 bsp_thread_cond_broadcast(XMEDIA_THREAD_COND* pCond)
{
    return    pthread_cond_broadcast(pCond);
}

INT32 bsp_thread_cond_wait(XMEDIA_THREAD_COND* pCond, XMEDIA_THREAD_MUTEX *pMutex, U32 msecs)
{
    struct timeval now;
    struct timespec timeout;

    if (msecs == 0)
    {
        return pthread_cond_wait(pCond, pMutex);
    }
    else
    {
        gettimeofday(&now, NULL);
        timeout.tv_sec = now.tv_sec;
        timeout.tv_nsec = ( now.tv_usec + (msecs * 1000 )) * 1000;
        return pthread_cond_timedwait(pCond, pMutex, &timeout);
    }

}

/*** Mutex Port END **/

INT32 bsp_thread_join(XMEDIA_THREAD_T *pThread, void **thread_return)
{
    return (pthread_join(pThread->threadid, thread_return));
}
INT32 bsp_thread_create(XMEDIA_THREAD_T *pThread,const pthread_attr_t*__attr,
                      void*__arg)
{
    if (NULL == pThread)
    {
        WRITE_LOG_ERROR("No routine to be run for create thread. \n");
        return XMEDIA_FAILURE;
    }

    if (NULL == pThread->pf_run)
    {
        WRITE_LOG_ERROR("No routine to be run for create thread. \n");
        return XMEDIA_FAILURE;
    }
    #if 0
    if (0 == pthread_create(&(pThread->threadid), __attr, pThread->pf_run, __arg))
    {
        WRITE_LOG_DEBUG("thread %s create, id:%u\n", pThread->szThreadName, pThread->threadid);
        return XMEDIA_SUCCESS;
    }
    else
    {
        return XMEDIA_FAILURE;
    }
    #endif

    pThread->args = __arg;

    if (0 == pthread_create(&(pThread->threadid), __attr,
                              bsp_thread_run_handle,
                              (void*)(pThread))
                            )
    {
        WRITE_LOG_DEBUG("thread %s create, threadid:%u.\n", pThread->szThreadName, pThread->threadid);
        return XMEDIA_SUCCESS;
    }
    else
    {
        return XMEDIA_FAILURE;
    }

}

INT32 bsp_thread_cancel(XMEDIA_THREAD_T *pThread)
{
    if (pThread == NULL)
    {
        return XMEDIA_FAILURE;
    }

    WRITE_LOG_DEBUG("cancel thread %s. pid:%d,tid:%d.\n",
            pThread->szThreadName, pThread->pid, pThread->threadid);

    return pthread_cancel(pThread->threadid);
}

VOID* bsp_thread_run_handle(VOID* arg)
{
    if (NULL == arg)
    {
        return NULL;
    }

    XMEDIA_THREAD_T *pThread = (XMEDIA_THREAD_T*)arg;

    pThread->pid = getpid();
    pThread->threadid = pthread_self();

    WRITE_LOG_DEBUG("thread %s run, pid:%u, threadid:%u\n",
                    pThread->szThreadName,
                    pThread->pid,
                    pThread->threadid);

    if (pThread->pf_run)
    {
        return pThread->pf_run(pThread->args);
    }
    else
    {
        return NULL;
    }
}

EXTERNFUNC XMEDIA_RET bsp_thread_init(XMEDIA_THREAD_T *pThread, const char* szName,
                        THREAD_RUN pf, INT32 priority, BOOL bStart)
{
    pThread->pf_run = pf;
    pThread->priority = priority;
    pThread->bStart = bStart;
    if(strlen(szName) >= MAX_THREAD_NAME_LEN)
    {
        printf("The len of szName is more than MAX_THREAD_NAME_LEN\n");
	    return XMEDIA_FAILURE;
    }else{
        strcpy(pThread->szThreadName, szName);
    }
    return XMEDIA_SUCCESS;
}

/* work at 100MHz */
EXTERNFUNC void bsp_delay(U32 ulUsDelay)
{
    U32 ulLoop = 0;

    while(ulLoop<ulUsDelay * 10)
    {
        ulLoop++;
    }
    //return XMEDIA_SUCCESS;
}

#if defined(SYNC_USE_COND)
EXTERNFUNC XMEDIA_RET bsp_sync_create(IO XMEDIA_SYNC_T *pWait)
{
    bsp_thread_cond_init(&pWait->_cond, NULL);
    bsp_thread_mutex_init(&pWait->_mutex, NULL);
    return XMEDIA_SUCCESS;
}
EXTERNFUNC XMEDIA_RET bsp_sync_wait(IO XMEDIA_SYNC_T *pWait,IN U32 msec)
{
    return bsp_thread_cond_wait(&pWait->_cond, &pWait->_mutex, msec);
}

EXTERNFUNC XMEDIA_RET bsp_sync_notify(IO XMEDIA_SYNC_T *pWait)
{
    return bsp_thread_cond_broadcast(&pWait->_cond);
}

EXTERNFUNC XMEDIA_RET bsp_sync_destroy(IO XMEDIA_SYNC_T *pWait)
{
    bsp_thread_cond_init(&pWait->_cond, NULL);
    bsp_thread_mutex_init(&pWait->_mutex, NULL);
    return XMEDIA_SUCCESS;
}
#else
EXTERNFUNC XMEDIA_RET bsp_sync_create(IO XMEDIA_SYNC_T *pWait)
{
    sem_init(&pWait->_sem, 0, 0);
    return XMEDIA_SUCCESS;
}
EXTERNFUNC XMEDIA_RET bsp_sync_wait(IO XMEDIA_SYNC_T *pWait,IN U32 msec)
{
    (void)msec;
    sem_wait(&pWait->_sem);
    return XMEDIA_SUCCESS;
}

EXTERNFUNC XMEDIA_RET bsp_sync_notify(IO XMEDIA_SYNC_T *pWait)
{
    sem_post(&pWait->_sem);
    return XMEDIA_SUCCESS;
}

EXTERNFUNC XMEDIA_RET bsp_sync_destroy(IO XMEDIA_SYNC_T *pWait)
{
    sem_destroy(&pWait->_sem);
    return XMEDIA_SUCCESS;
}

#endif
