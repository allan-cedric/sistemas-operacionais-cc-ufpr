#include "ppos.h"
#include <stdio.h>
#include <stdlib.h>

#define MAIN_PID 0
#define STACK_SIZE 64 * 1024

typedef char byte_t;

int pid = 0;
task_t main_task, *current_task;

void ppos_init()
{
#ifdef DEBUG
    fprintf(stdout, "PPOS (ppos_init): Initialization...\n");
#endif

    // Desativa buffer da stdout
    setvbuf(stdout, 0, _IONBF, 0);

    // Inicialização da tarefa main
    main_task.prev = NULL;
    main_task.next = NULL;
    main_task.id = MAIN_PID;

#ifdef DEBUG
    fprintf(stdout, "PPOS (ppos_init): Main task(%i)(%p) was created successfully!\n", main_task.id, &main_task);
#endif

    // Inicialização da tarefa atual
    current_task = &main_task;

#ifdef DEBUG
    fprintf(stdout, "PPOS (ppos_init): Current task was initialized with the main task(%i)\n\n", current_task->id);
#endif
}

int task_create(task_t *task, void (*start_func)(void *), void *arg)
{
#ifdef DEBUG
    fprintf(stdout, "PPOS (task_create): Creating a new task...\n");
#endif

    task->prev = NULL;
    task->next = NULL;
    task->id = ++pid;
    getcontext(&task->context); // Salva o corrente contexto na tarefa

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

#ifdef DEBUG
    fprintf(stdout, "PPOS (task_create): Task(%i)(%p) was created successfully!\n\n", task->id, task);
#endif

    return 0;
}

void task_exit(int exitCode)
{
    if (current_task == &main_task)
        exit(1);

#ifdef DEBUG
    fprintf(stdout, "PPOS (task_exit): Terminating task(%i)...\n", current_task->id);
#endif

#ifdef DEBUG
    fprintf(stdout, "PPOS (task_exit): Task(%i) was terminated successfully!\n", current_task->id);
#endif

    task_switch(&main_task);
}

int task_switch(task_t *task)
{
    
#ifdef DEBUG
    fprintf(stdout, "PPOS (task_switch): Switching current task(%i) to task(%i)...\n", current_task->id, task->id);
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