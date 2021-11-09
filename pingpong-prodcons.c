#include "ppos.h"
#include <time.h>

#define NUM_PRODUCER 3
#define NUM_CONSUMER 2
#define NUM_BUFFER 5

task_t prod[NUM_PRODUCER];
task_t cons[NUM_CONSUMER];

semaphore_t s_produce, s_consume, s_buffer;

typedef struct buffer_t
{
    struct buffer_t *prev;
    struct buffer_t *next;
    int item;
} buffer_t;

buffer_t buffer_items[NUM_BUFFER];
buffer_t *buffer = NULL;
int buffer_index = 0;

void producer(void *arg)
{
    while (1)
    {
        task_sleep(1000);
        int item = rand() % 100;

        sem_down(&s_produce);
        sem_down(&s_buffer);

        buffer_items[buffer_index].item = item;
        queue_append((queue_t **)&buffer, (queue_t *)&buffer_items[buffer_index]);

        if (buffer_index >= NUM_BUFFER - 1)
            buffer_index = 0;
        else
            buffer_index++;

        sem_up(&s_buffer);
        sem_up(&s_consume);

        printf("%s produced %i\n", (char *)arg, item);
    }
}

void consumer(void *arg)
{
    while (1)
    {
        sem_down(&s_consume);
        sem_down(&s_buffer);

        buffer_t *first = buffer;
        queue_remove((queue_t **)&buffer, (queue_t *)first);

        int item = first->item;

        sem_up(&s_buffer);
        sem_up(&s_produce);

        printf("\t\t\t%s consumed %i\n", (char *)arg, item);

        task_sleep(1000);
    }
}

int main()
{
    srand(time(NULL));
    ppos_init();

    sem_create(&s_produce, NUM_BUFFER);
    sem_create(&s_consume, 0);
    sem_create(&s_buffer, 1);

    for (int i = 0; i < NUM_BUFFER; i++)
    {
        buffer_items[i].prev = NULL;
        buffer_items[i].next = NULL;
    }

    task_create(&prod[0], producer, "p1");
    task_create(&prod[1], producer, "p2");
    task_create(&prod[2], producer, "p3");

    task_create(&cons[0], consumer, "c1");
    task_create(&cons[1], consumer, "c2");

    task_exit(0);
}