#include "ppos.h"

extern int lock_kernel;
extern task_t *user_tasks_queue, *current_task;

int sem_create(semaphore_t *s, int value)
{
    lock_kernel = 1;

    if (!s)
        return -1;

    s->queue = NULL;
    s->counter = value;
    s->destroyed = 0;

    lock_kernel = 0;
    return 0;
}

int sem_down(semaphore_t *s)
{
    lock_kernel = 1;

    if (!s || s->destroyed)
        return -1;

    s->counter--;
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

    if (!s || s->destroyed)
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

    if (!s || s->destroyed)
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

int mqueue_create(mqueue_t *queue, int max, int size)
{
    lock_kernel = 1;

    if (!queue)
        return -1;

    queue->buffer = malloc(max * size);
    if (!queue->buffer)
    {
        fprintf(stderr, "Memory allocation error!\n");
        exit(1);
    }

    queue->buffer_head = 0;
    queue->buffer_index = 0;
    queue->num_msgs = 0;
    queue->max_msgs = max;
    queue->msg_size = size;

    sem_create(&queue->s_buffer, 1);
    sem_create(&queue->s_produce, queue->max_msgs);
    sem_create(&queue->s_consume, 0);

    lock_kernel = 0;
    return 0;
}

int mqueue_send(mqueue_t *queue, void *msg)
{
    if (!queue || !queue->buffer)
        return -1;

    if (sem_down(&queue->s_produce) < 0 || sem_down(&queue->s_buffer) < 0)
        return -1;

    memcpy(queue->buffer + queue->buffer_index * queue->msg_size, msg, queue->msg_size);
    queue->buffer_index = (queue->buffer_index + 1) % queue->max_msgs;
    queue->num_msgs++;

    if (sem_up(&queue->s_buffer) < 0 || sem_up(&queue->s_consume) < 0)
        return -1;

    return 0;
}

int mqueue_recv(mqueue_t *queue, void *msg)
{
    if (!queue || !queue->buffer)
        return -1;

    if (sem_down(&queue->s_consume) < 0 || sem_down(&queue->s_buffer) < 0)
        return -1;

    memcpy(msg, queue->buffer + queue->buffer_head * queue->msg_size, queue->msg_size);
    queue->buffer_head = (queue->buffer_head + 1) % queue->max_msgs;
    queue->num_msgs--;

    if (sem_up(&queue->s_buffer) < 0 || sem_up(&queue->s_produce) < 0)
        return -1;

    return 0;
}

int mqueue_destroy(mqueue_t *queue)
{
    lock_kernel = 1;

    if (!queue || !queue->buffer)
        return -1;

    free(queue->buffer);
    queue->buffer = NULL;

    queue->num_msgs = 0;

    sem_destroy(&queue->s_buffer);
    sem_destroy(&queue->s_produce);
    sem_destroy(&queue->s_consume);

    lock_kernel = 0;
    return 0;
}

int mqueue_msgs(mqueue_t *queue)
{
    if (!queue || !queue->buffer)
        return -1;

    return queue->num_msgs;
}
