#include "ppos.h"

extern int lock_kernel;
extern task_t *user_tasks_queue, *current_task;

int sem_create(semaphore_t *s, int value)
{
    lock_kernel = 1;
    
    s->queue = NULL;
    s->counter = value;
    s->destroyed = 0;
    
    lock_kernel = 0;
    return 0;
}

int sem_down(semaphore_t *s)
{
    lock_kernel = 1;

    if (s->destroyed)
        return -1;

    s->counter -= 1;
    if (s->counter < 0)
    {
        current_task->state = SUSPENDED;
        queue_remove((queue_t **)&user_tasks_queue, (queue_t *)current_task);
        queue_append((queue_t **)&s->queue, (queue_t *)current_task);
        task_yield();
    }

    lock_kernel = 0;
    return (-s->destroyed);
}

int sem_up(semaphore_t *s)
{
    lock_kernel = 1;

    if (s->destroyed)
        return -1;

    s->counter++;
    if (s->counter <= 0)
    {
        task_t *first = s->queue;
        first->state = READY;
        queue_remove((queue_t **)&s->queue, (queue_t *)first);
        queue_append((queue_t **)&user_tasks_queue, (queue_t *)first);
    }

    lock_kernel = 0;
    return (-s->destroyed);
}

int sem_destroy(semaphore_t *s)
{
    lock_kernel = 1;

    if (s->destroyed)
        return -1;

    s->destroyed = 1;
    task_t *aux = s->queue;
    while (aux)
    {
        aux->state = READY;
        queue_remove((queue_t **)&s->queue, (queue_t *)aux);
        queue_append((queue_t **)&user_tasks_queue, (queue_t *)aux);
        aux = s->queue;
    }

    lock_kernel = 0;
    return 0;
}
