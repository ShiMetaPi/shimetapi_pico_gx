/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <semaphore.h>

static sem_t g_sem;

int events_init(void)
{
    return sem_init(&g_sem, 0, 0);
}

int events_wait(void)
{
    return sem_wait(&g_sem);
}

int events_wait_timeout_ms(unsigned long long timeout_ms)
{
    int rc;
    unsigned long long sec, nsec;
    struct timespec tv = {0};

    clock_gettime(CLOCK_REALTIME, &tv);

    sec  = timeout_ms / 1000;
    nsec = (timeout_ms % 1000) * 1000000;

    tv.tv_nsec += nsec;
    tv.tv_sec += sec;

    rc = sem_timedwait(&g_sem, (const struct timespec*)&tv);
    if (rc < 0) {
        if (rc == ETIMEDOUT) {
            printf("sem_timedwait timeout.\n");
        } else {
            printf("sem_timedwait fail.\n");
        }
    }

    return 0;
}

int events_notity_one(void)
{
    return sem_post(&g_sem);
}

int events_deinit(void)
{
    return sem_destroy(&g_sem);
}

int node_wait(void)
{
    return events_wait();
}

int node_wait_timeout_ms(unsigned long long timeout_ms)
{
    return events_wait_timeout_ms(timeout_ms);
}

int node_notity_one(void)
{
    return events_notity_one();
}
