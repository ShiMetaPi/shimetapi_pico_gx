/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#ifndef __XMEDIA_THREAD_H__
#define __XMEDIA_THREAD_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */
#include <pthread.h>
#include <semaphore.h>
#include <bsp_defs.h>
#include <stdio.h>

typedef pthread_mutex_t     XMEDIA_THREAD_MUTEX ;
typedef pthread_mutexattr_t XMEDIA_THREAD_MUTEXATTR;

typedef pthread_cond_t      XMEDIA_THREAD_COND;
typedef pthread_condattr_t  XMEDIA_THREAD_CONDATTR;

typedef struct XMEDIA_SYNC_STRU
{
    #if defined(SYNC_USE_COND)
    XMEDIA_THREAD_MUTEX _mutex;
    XMEDIA_THREAD_COND  _cond;
    #else
    sem_t           _sem;
    #endif
}XMEDIA_SYNC_T;

#define MAX_THREAD_NAME_LEN 16

typedef int XMEDIA_PRIORITY_T;

typedef char THREAD_NAME[MAX_THREAD_NAME_LEN];

typedef VOID* (*THREAD_RUN)(VOID* );

typedef struct XMEDIA_THREAD_STRU
{
    THREAD_NAME     szThreadName;
    /**/
    pthread_t       threadid;

    pid_t           pid;

    XMEDIA_PRIORITY_T   priority;

    THREAD_RUN      pf_run;

    BOOL            bStart;

    XMEDIA_THREAD_COND cond_start;
    XMEDIA_THREAD_MUTEX mutex_cond_start;

    VOID*          args;
}XMEDIA_THREAD_T;

EXTERNFUNC XMEDIA_RET bsp_thread_init(XMEDIA_THREAD_T *pThread, const char* szName,
                        THREAD_RUN pf, INT32 priority, BOOL bStart);

VOID* bsp_thread_run_handle(VOID* arg);

EXTERNFUNC void bsp_delay(U32 ulUsDelay);

/*__wait is TRUE, not run when create , wait cond_start to run*/
EXTERNFUNC INT32 bsp_thread_create(XMEDIA_THREAD_T *pThread,const pthread_attr_t*__attr,
                      void*__arg);

EXTERNFUNC INT32 bsp_thread_join(XMEDIA_THREAD_T *pThread, void **thread_return);

EXTERNFUNC INT32 XMEDIA_THREAD_Start(XMEDIA_THREAD_T *pThread);

EXTERNFUNC INT32 bsp_thread_pause(XMEDIA_THREAD_T *pThread);

EXTERNFUNC INT32 bsp_thread_cancel(XMEDIA_THREAD_T *pThread);
EXTERNFUNC VOID bsp_thread_setid(XMEDIA_THREAD_T *pThread);

EXTERNFUNC VOID bsp_thread_setpriority(XMEDIA_THREAD_T *pThread, XMEDIA_PRIORITY_T prio);

/*** ------------------------------------ ***/
EXTERNFUNC INT32 bsp_thread_mutex_init(XMEDIA_THREAD_MUTEX* pMutex, XMEDIA_THREAD_MUTEXATTR* pMutexattr);

EXTERNFUNC INT32 bsp_thread_mutex_lock(XMEDIA_THREAD_MUTEX* pMutex);

EXTERNFUNC INT32 bsp_thread_mutex_trylock(XMEDIA_THREAD_MUTEX* pMutex);

EXTERNFUNC INT32 bsp_thread_mutex_unlock(XMEDIA_THREAD_MUTEX* pMutex);

EXTERNFUNC INT32 bsp_thread_mutex_destroy(XMEDIA_THREAD_MUTEX* pMutex);

/*** ------------------------------------ ***/
EXTERNFUNC INT32 bsp_thread_cond_init(XMEDIA_THREAD_COND* pCond, XMEDIA_THREAD_CONDATTR* pCondattr);
EXTERNFUNC INT32 bsp_thread_cond_signal(XMEDIA_THREAD_COND* pCond);
EXTERNFUNC INT32 bsp_thread_cond_broadcast(XMEDIA_THREAD_COND* pCond);
EXTERNFUNC INT32 bsp_thread_cond_wait(XMEDIA_THREAD_COND* pCond, XMEDIA_THREAD_MUTEX *pMutex, U32 msecs);
EXTERNFUNC INT32 bsp_thread_cond_destroy(XMEDIA_THREAD_COND* pCond);

#define DEFAULT_PRIORITY       0
#define THREAD_PRIORITY_AUDIO 0
#define THREAD_PRIORITY_AZ    10

#define THREAD_SLEEP(msec) usleep(msec * 1000)
/*#define THREAD_SLEEP(msec) */

/*** -------------------------------------***/
EXTERNFUNC XMEDIA_RET bsp_sync_create(IO XMEDIA_SYNC_T *pWait);
EXTERNFUNC XMEDIA_RET bsp_sync_wait(IO XMEDIA_SYNC_T *pWait, IN U32 msec);
EXTERNFUNC XMEDIA_RET bsp_sync_notify(IO XMEDIA_SYNC_T *pWait);
EXTERNFUNC XMEDIA_RET bsp_sync_destroy(IO XMEDIA_SYNC_T *pWait);

#if defined(DEBUG)
#define SYNC_WAIT(pSync, msec, pStr) \
    if (pStr)                        \
        WRITE_LOG_DEBUG(pStr);       \
    bsp_sync_wait(pSync, msec)

#define SYNC_NOTIFY(pSync, msec, pStr) \
    if (pStr)                        \
        WRITE_LOG_DEBUG(pStr);       \
    bsp_sync_notify(pSync)
#else
#define SYNC_WAIT(pSync, msec, pStr) \
    bsp_sync_wait(pSync, msec)

#define SYNC_NOTIFY(pSync, msec, pStr) \
    bsp_sync_notify(pSync)
#endif

/*__wait is TRUE, not run when create , wait cond_start to run*/
#define XMEDIA_THREAD_CREATE (pThread, __attr, __arg)\
                bsp_thread_create(pThread, __attr, __arg)

#define XMEDIA_THREAD_JOIN(pThread, ppthread_return) \
                bsp_thread_join(pThread, ppthread_return)

#define XMEDIA_THREAD_START(pThread) \
                XMEDIA_THREAD_Start(pThread)

#define XMEDIA_THREAD_PAUSE(pThread) \
                bsp_thread_pause(pThread)

#define XMEDIA_THREAD_SETID(pThread) \
                bsp_thread_setid(pThread)

#define XMEDIA_THREAD_SETPRIORITY(pThread ,prio) \
                bsp_thread_setpriority(pThread ,prio)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __XMEDIA_THREAD_H__ */

