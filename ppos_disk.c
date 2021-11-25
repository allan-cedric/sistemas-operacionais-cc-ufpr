// --- Implementação de um driver de disco virtual - PingPongOS ---

// Allan Cedric G. B. Alves da Silva - GRR20190351

#include "ppos_disk.h"

extern task_t *user_tasks_queue, *current_task;
extern int lock_kernel, num_user_tasks;

disk_t disk;

void driver_proc(void *arg)
{
    while (1)
    {
        sem_down(&disk.s_access);

        if (disk.sig_disk)
        {
            disk_queue_t *first = disk.queue;
            queue_remove((queue_t **)&disk.queue, (queue_t *)first);

            first->requester->state = READY;
            queue_append((queue_t **)&user_tasks_queue, (queue_t *)first->requester);

            free(first);
            first = NULL;

            disk.sig_disk = 0;
        }

        // Se o disco está livre e tem pedidos pendentes
        if (disk_cmd(DISK_CMD_STATUS, 0, 0) == 1 && disk.queue)
            disk_cmd(disk.queue->op_type, disk.queue->block, disk.queue->buffer);

        sem_up(&disk.s_access);

        // Suspende o gerente de disco
        disk.driver.state = SUSPENDED;
        queue_remove((queue_t **)&user_tasks_queue, (queue_t *)&disk.driver);
        task_yield();
    }
}

void wake_driver()
{
    disk.sig_disk = 1;
    disk.driver.state = READY;
    queue_append((queue_t **)&user_tasks_queue, (queue_t *)&disk.driver);
}

int disk_mgr_init(int *numBlocks, int *blockSize)
{
    lock_kernel = 1;

    // Inicializa disco
    if (!disk_cmd(DISK_CMD_STATUS, 0, 0))
    {
        if (disk_cmd(DISK_CMD_INIT, 0, 0))
            return -1;
    }

    *numBlocks = disk_cmd(DISK_CMD_DISKSIZE, 0, 0);
    if (*numBlocks < 0)
        return -1;
    disk.num_blocks = *numBlocks;

    *blockSize = disk_cmd(DISK_CMD_BLOCKSIZE, 0, 0);
    if (*blockSize < 0)
        return -1;
    disk.block_size = *blockSize;

    // Inicializa driver de disco
    if (task_create(&disk.driver, driver_proc, NULL) < 0)
        return -1;
    num_user_tasks--; // Desconsidera o driver como tarefa do usuário

    // Inicializa semáforo de acesso ao disco
    if (sem_create(&disk.s_access, 1) < 0)
        return -1;

    disk.queue = NULL; // Inicializa a fila do disco

    // Define a rotina de tratamento para acordar o gerente de disco
    disk.sig_handler.sa_handler = wake_driver;
    sigemptyset(&disk.sig_handler.sa_mask);
    disk.sig_handler.sa_flags = 0;
    if (sigaction(SIGUSR1, &disk.sig_handler, 0) < 0)
    {
        fprintf(stderr, "Error (ppos_init): Failed to set a SIGUSR1 interrupt!\n");
        exit(1);
    }

    lock_kernel = 0;
    return 0;
}

int disk_block_read(int block, void *buffer)
{
    // Bloco fora do escopo do disco
    if (block < 0 || block > disk.num_blocks - 1)
        return -1;

    // obtém o semáforo de acesso ao disco
    sem_down(&disk.s_access);

    // inclui o pedido na fila_disco
    disk_queue_t *new_request = (disk_queue_t *)malloc(sizeof(disk_queue_t));
    if (!new_request)
    {
        fprintf(stderr, "Memory allocation error!\n");
        exit(1);
    }
    new_request->requester = current_task;
    new_request->op_type = DISK_CMD_READ;
    new_request->block = block;
    new_request->buffer = buffer;

    queue_append((queue_t **)&disk.queue, (queue_t *)new_request);

    if (disk.driver.state == SUSPENDED)
    {
        // acorda o gerente de disco (põe ele na fila de prontas)
        disk.driver.state = READY;
        queue_append((queue_t **)&user_tasks_queue, (queue_t *)&disk.driver);
    }

    // libera semáforo de acesso ao disco
    sem_up(&disk.s_access);

    // suspende a tarefa corrente (retorna ao dispatcher)
    current_task->state = SUSPENDED;
    queue_remove((queue_t **)&user_tasks_queue, (queue_t *)current_task);

    task_yield();

    return 0;
}

int disk_block_write(int block, void *buffer)
{
    // Bloco fora do escopo do disco
    if (block < 0 || block > disk.num_blocks - 1)
        return -1;

    // obtém o semáforo de acesso ao disco
    sem_down(&disk.s_access);

    // inclui o pedido na fila_disco
    disk_queue_t *new_request = (disk_queue_t *)malloc(sizeof(disk_queue_t));
    if (!new_request)
    {
        fprintf(stderr, "Memory allocation error!\n");
        exit(1);
    }
    new_request->requester = current_task;
    new_request->op_type = DISK_CMD_WRITE;
    new_request->block = block;
    new_request->buffer = buffer;

    queue_append((queue_t **)&disk.queue, (queue_t *)new_request);

    if (disk.driver.state == SUSPENDED)
    {
        // acorda o gerente de disco (põe ele na fila de prontas)
        disk.driver.state = READY;
        queue_append((queue_t **)&user_tasks_queue, (queue_t *)&disk.driver);
    }

    // libera semáforo de acesso ao disco
    sem_up(&disk.s_access);

    // suspende a tarefa corrente (retorna ao dispatcher)
    current_task->state = SUSPENDED;
    queue_remove((queue_t **)&user_tasks_queue, (queue_t *)current_task);

    task_yield();

    return 0;
}