#include "ppos.h"
#include <stdio.h>
#include <stdlib.h>

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

task_t *scheduler()
{
    return user_tasks_queue; // Retorna a cabeça (Escalonador FCFS)
}

void print_task_id(void *elem)
{
    task_t *task = (task_t *)elem;
    printf("%i", task->id);
}

void dispatcher_proc(void *arg)
{
    while (num_user_tasks)
    {
#ifdef DEBUG
        queue_print("Ready user's tasks: ", (queue_t *)user_tasks_queue, print_task_id);
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
    default: // Tarefa genérico do usuário
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
    fprintf(stdout, "PPOS: task %i yields the CPU!\n", task_id());
#endif
    if (user_tasks_queue && task_id() > 1)
        user_tasks_queue = user_tasks_queue->next;
    task_switch(&dispatcher);
}