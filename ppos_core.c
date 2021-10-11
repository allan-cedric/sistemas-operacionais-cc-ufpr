#include "ppos.h"
#include <stdio.h>
#include <stdlib.h>

#define MAX_PRIOR -20
#define MIN_PRIOR (-MAX_PRIOR)
#define DEFAULT_PRIOR 0
#define ALPHA -1
#define MAIN_PID 0
#define STACK_SIZE 64 * 1024

typedef char byte_t;

typedef enum task_states_t
{
    TERM,
    READY,
    SUSPENSE
} task_states_t;

int pid = 0, num_user_tasks = 0;
task_t main_task, dispatcher, *current_task;
task_t *user_tasks_queue;

void free_task(task_t **task)
{
    free((*task)->context.uc_stack.ss_sp);
    (*task)->context.uc_stack.ss_sp = NULL;
    (*task)->context.uc_stack.ss_size = 0;
}

void print_task_id(void *elem)
{
    task_t *task = (task_t *)elem;
    printf("%i:%i", task->id, task->dynam_prior);
}

task_t *scheduler()
{
    // Caso a fila de execução esteja vazia
    if (!user_tasks_queue)
        return NULL;

    int max_prior = user_tasks_queue->dynam_prior;
    task_t *cur = user_tasks_queue, *choosen_task = cur;

    // Varre toda a fila de execução
    while ((cur = cur->next) != user_tasks_queue)
    {
        // Prioridade em escala negativa
        if (cur->dynam_prior < max_prior)
        {
            // Envelhece a tarefa escolhida anteriormente
            if (choosen_task->dynam_prior > MAX_PRIOR)
                choosen_task->dynam_prior += ALPHA;

            // Atualiza para uma nova tarefa
            max_prior = cur->dynam_prior;
            choosen_task = cur;
        }
        else if (cur->dynam_prior > MAX_PRIOR)
            cur->dynam_prior += ALPHA;
    }
    choosen_task->dynam_prior = choosen_task->static_prior;

#ifdef DEBUG
    fprintf(stdout, "PPOS (scheduler): task %i choosen!\n", choosen_task->id);
    queue_print("PPOS (scheduler): Ready user's tasks ->", (queue_t *)user_tasks_queue, print_task_id);
#endif

    return choosen_task;
    // return user_tasks_queue; // Retorna a cabeça (Escalonador FCFS)
}

void dispatcher_proc(void *arg)
{
    while (num_user_tasks)
    {
#ifdef DEBUG
        queue_print("PPOS (dispatcher): Ready user's tasks ->", (queue_t *)user_tasks_queue, print_task_id);
#endif
        task_t *next_task = scheduler();

        if (next_task)
        {
            task_switch(next_task);

            // Tratamento de estados da tarefa (#TODO)
            switch (next_task->state)
            {
            case TERM:
                queue_remove((queue_t **)&user_tasks_queue, (queue_t *)next_task);
                free_task(&next_task);
                num_user_tasks--;
                break;
            default:
                break;
            }
        }
    }
    task_exit(0);
}

void ppos_init()
{
#ifdef DEBUG
    fprintf(stdout, "PPOS (ppos_init): OS initialization...\n");
#endif

    // Desativa buffer da stdout
    setvbuf(stdout, 0, _IONBF, 0);

    // Inicialização da tarefa main
    main_task.prev = NULL;
    main_task.next = NULL;
    main_task.id = MAIN_PID;
    main_task.state = READY;
    main_task.static_prior = MAX_PRIOR;
    main_task.dynam_prior = MAX_PRIOR;

    // Inicialização da tarefa atual
    current_task = &main_task;

#ifdef DEBUG
    fprintf(stdout, "PPOS (ppos_init): task %i (%p) was created successfully!\n", task_id(), &main_task);
#endif

    // Inicialização da fila de execução (usuário)
    user_tasks_queue = NULL;

    // Criando despachante
    task_create(&dispatcher, dispatcher_proc, NULL);

#ifdef DEBUG
    fprintf(stdout, "PPOS (ppos_init): PingPongOS was initialized successfully!\n\n");
    fprintf(stdout, " ******************************\n");
    fprintf(stdout, " *** Welcome to PingPongOS! ***\n");
    fprintf(stdout, " ******************************\n\n");
#endif
}

int task_create(task_t *task, void (*start_func)(void *), void *arg)
{
    task->prev = NULL;
    task->next = NULL;
    task->id = ++pid;
    task->state = READY;
    task->static_prior = DEFAULT_PRIOR;
    task->dynam_prior = DEFAULT_PRIOR;
    getcontext(&task->context); // Salva um contexto genérico

    // Inicialização da stack
    byte_t *stack = (byte_t *)malloc(STACK_SIZE);
    if (stack)
    {
        task->context.uc_stack.ss_sp = stack;
        task->context.uc_stack.ss_size = STACK_SIZE;
        task->context.uc_stack.ss_flags = 0;
        task->context.uc_link = 0;
    }
    else
    {
        fprintf(stderr, "Error (task_create): It's not possible to create a stack!\n");
        exit(1);
    }

    makecontext(&task->context, (void *)start_func, 1, arg); // Ajusta o contexto da tarefa

    // Caso não seja o despachante,
    // adiciona a tarefa na fila de tarefas prontas do usuário.
    if (task->id > 1)
    {
        queue_append((queue_t **)&user_tasks_queue, (queue_t *)task);
        num_user_tasks++;
    }

#ifdef DEBUG
    fprintf(stdout, "PPOS (task_create): task %i (%p) was created successfully!\n", task->id, task);
#endif

    return 0;
}

void task_exit(int exitCode)
{
#ifdef DEBUG
    fprintf(stdout, "PPOS (task_exit): task %i is terminating...\n", task_id());
#endif

    current_task->state = TERM;

    switch (task_id())
    {
    case 0: // Main
        exit(1);
    case 1: // Despachante
        task_switch(&main_task);
        break;
    default: // Tarefa genérica do usuário
        task_switch(&dispatcher);
        break;
    }
}

int task_switch(task_t *task)
{

#ifdef DEBUG
    fprintf(stdout, "PPOS (task_switch): current task %i to task %i...\n", task_id(), task->id);
#endif

    task_t *aux = current_task;
    current_task = task;
    if (swapcontext(&aux->context, &task->context) < 0)
    {
        fprintf(stderr, "Error (task_switch): Unable to switch the context!\n");
        exit(1);
    }
    return 0;
}

int task_id()
{
    return current_task->id;
}

void task_yield()
{
#ifdef DEBUG
    fprintf(stdout, "PPOS (task_yield): task %i yields the CPU!\n", task_id());
#endif
    if (user_tasks_queue && task_id() > 1)
        user_tasks_queue = user_tasks_queue->next;
    task_switch(&dispatcher);
}

void task_setprio (task_t *task, int prio)
{
    if(task)
        task->static_prior = prio;
    else
        current_task->static_prior = prio;
}

int task_getprio (task_t *task)
{
    return (task ? task->static_prior : current_task->static_prior);
}