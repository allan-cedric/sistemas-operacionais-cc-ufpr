// --- Implementação dos principais mecanismos do PingPongOS ---

// Allan Cedric G. B. Alves da Silva - GRR20190351

#include "ppos.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>

// --- Macros ---
#define MAX_PRIO -20
#define MIN_PRIO (-MAX_PRIO)
#define DEFAULT_PRIO 0
#define ALPHA -1

#define DEFAULT_QUANTUM_TICKS 20
#define MAIN_PID 0
#define STACK_SIZE (64 * 1024)

// --- Definições de tipos convenientes ---
typedef char byte_t;
typedef enum task_states_t
{
    TERM,
    READY,
    SUSPENSE
} task_states_t;

// --- Variáveis globais do sistema ---
int pid = 0, num_user_tasks = 0;
int counter_ticks;
unsigned int system_clock = 0;

task_t main_task, dispatcher, *current_task;
task_t *user_tasks_queue = NULL;

struct sigaction preemption;
struct itimerval timer;

/*!
    @brief  Libera memória de uma tarefa.

    @param  task    Endereço do ponteiro que aponta para uma tarefa.
*/
void free_task(task_t **task)
{
    free((*task)->context.uc_stack.ss_sp);
    (*task)->context.uc_stack.ss_sp = NULL;
    (*task)->context.uc_stack.ss_size = 0;
}

/*!
    @brief  Imprime na saída padrão o ID e a prioridade dinâmica de uma tarefa.

    @param  elem    Ponteiro para uma estrutura genérica.
*/
void print_task_id(void *elem)
{
    task_t *task = elem;
    fprintf(stdout, "%i:%i", task->id, task->dynam_prio);
}

/*!
    @brief  Rotina do escalanador de tarefas (Escalonamento por prioridades dinâmicas).

    @return Endereço da tarefa escolhida. 
            Caso não tenha tarefas a executar retorna NULL.
*/
task_t *scheduler()
{
    // Caso a fila de execução esteja vazia
    if (!user_tasks_queue)
        return NULL;

    task_t *cur = user_tasks_queue, *choosen_task = cur;
    int max_prio = user_tasks_queue->dynam_prio;

    // Varre toda a fila de execução
    while ((cur = cur->next) != user_tasks_queue)
    {
        // Prioridade em escala negativa
        if (cur->dynam_prio < max_prio)
        {
            // Envelhece a tarefa escolhida anteriormente
            if (choosen_task->dynam_prio > MAX_PRIO)
                choosen_task->dynam_prio += ALPHA;

            // Atualiza para uma nova tarefa
            choosen_task = cur;
            max_prio = cur->dynam_prio;
        }
        else if (cur->dynam_prio > MAX_PRIO)
            cur->dynam_prio += ALPHA;
    }
    choosen_task->dynam_prio = choosen_task->static_prio;

#ifdef DEBUG
    fprintf(stdout, "PPOS (scheduler): task %i choosen!\n", choosen_task->id);
    queue_print("PPOS (scheduler): User's ready tasks -> ", (queue_t *)user_tasks_queue, print_task_id);
#endif

    return choosen_task;
}

/*!
    @brief  Rotina do despachante de tarefas.
*/
void dispatcher_proc(void *arg)
{

#ifdef DEBUG
    fprintf(stdout, "PPOS (dispatcher): Dispatcher was launched!\n");
#endif

    while (user_tasks_queue)
    {

#ifdef DEBUG
        queue_print("PPOS (dispatcher): User's ready tasks -> ", (queue_t *)user_tasks_queue, print_task_id);
#endif

        task_t *next_task = scheduler();

        if (next_task)
        {
            counter_ticks = next_task->quantum_ticks;

            unsigned int init_cpu_time = systime();
            task_switch(next_task);
            next_task->cpu_time += (systime() - init_cpu_time);

            // Tratamento de estados da tarefa (#TODO)
            switch (next_task->state)
            {
            case TERM:
                fprintf(
                    stdout, "Task %i exit: execution time %i ms, processor time %i ms, %i activations\n",
                    next_task->id,
                    systime() - next_task->born_timestamp,
                    next_task->cpu_time,
                    next_task->cpu_activations);
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

/*!
    @brief  Rotina de tratamento de interrupção: Preempção de tarefas
*/
void task_preemption()
{
    system_clock++;

    // Caso seja uma tarefa de usuário, trata da preempção
    if (task_id() != 1)
    {
        if (!(--counter_ticks))
            task_switch(&dispatcher);
    }
}

void ppos_init()
{
    // Desativa buffer da stdout
    setvbuf(stdout, 0, _IONBF, 0);

#ifdef DEBUG
    fprintf(stdout, "PPOS (ppos_init): OS initialization...\n");
#endif

    // Inicialização da tarefa main
    main_task.born_timestamp = systime();
    main_task.cpu_time = 0;
    main_task.cpu_activations = 0;

    main_task.prev = NULL;
    main_task.next = NULL;
    main_task.id = MAIN_PID;
    main_task.state = READY;
    main_task.static_prio = DEFAULT_PRIO;
    main_task.dynam_prio = DEFAULT_PRIO;
    main_task.quantum_ticks = DEFAULT_QUANTUM_TICKS;

    // Inicialização da tarefa atual
    current_task = &main_task;

#ifdef DEBUG
    fprintf(stdout, "PPOS (ppos_init): task %i (%p) was created successfully!\n", task_id(), &main_task);
#endif

    // Adiciona a tarefa main na fila de tarefas prontas do usuário
    queue_append((queue_t **)&user_tasks_queue, (queue_t *)&main_task);

    // Criando despachante
    task_create(&dispatcher, dispatcher_proc, NULL);

    // Define a rotina de tratamento de preempção de tarefas
    preemption.sa_handler = task_preemption;
    sigemptyset(&preemption.sa_mask);
    preemption.sa_flags = 0;
    if (sigaction(SIGALRM, &preemption, 0) < 0)
    {
        fprintf(stderr, "Error (ppos_init): Failed to set a SIGALRM interrupt!\n");
        exit(1);
    }

    // Define o temporizador para interrupções
    timer.it_value.tv_usec = 1000;
    timer.it_value.tv_sec = 0;
    timer.it_interval.tv_usec = 1000;
    timer.it_interval.tv_sec = 0;
    if (setitimer(ITIMER_REAL, &timer, 0) < 0)
    {
        fprintf(stderr, "Error (ppos_init): Failed to set a timer!\n");
        exit(1);
    }

#ifdef DEBUG
    fprintf(stdout, "PPOS (ppos_init): PingPongOS was initialized successfully!\n\n");
    fprintf(stdout, " ******************************\n");
    fprintf(stdout, " *** Welcome to PingPongOS! ***\n");
    fprintf(stdout, " ******************************\n\n");
#endif

    task_switch(&dispatcher);
}

int task_create(task_t *task, void (*start_func)(void *), void *arg)
{
    if (!task)
    {
        fprintf(stderr, "Error (task_create): There is no task structure!\n");
        return -1;
    }

    task->born_timestamp = systime();
    task->cpu_time = 0;
    task->cpu_activations = 0;

    task->prev = NULL;
    task->next = NULL;
    task->id = ++pid;
    task->state = READY;
    task->static_prio = DEFAULT_PRIO;
    task->dynam_prio = DEFAULT_PRIO;
    task->quantum_ticks = DEFAULT_QUANTUM_TICKS;
    getcontext(&task->context); // Salva o contexto atual

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
    fprintf(stdout, "PPOS (task_create): task %i (%p) was created successfully!\n", task->id, task);
#endif

    // Adiciona a tarefa na fila de tarefas prontas do usuário.
    queue_append((queue_t **)&user_tasks_queue, (queue_t *)task);
    num_user_tasks++;

    return task->id;
}

void task_exit(int exitCode)
{

#ifdef DEBUG
    fprintf(stdout, "PPOS (task_exit): task %i is terminating...\n", task_id());
#endif

    current_task->state = TERM;

    switch (task_id())
    {
    case 1: // Despachante
        fprintf(
            stdout, "Task %i exit: execution time %i ms, processor time %i ms, %i activations\n",
            task_id(),
            systime() - current_task->born_timestamp,
            current_task->cpu_time,
            current_task->cpu_activations);
        exit(0);
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

    task->cpu_activations++;
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

    task_switch(&dispatcher);
}

void task_setprio(task_t *task, int prio)
{
    if (task)
    {
        task->static_prio = prio;
        task->dynam_prio = prio;
    }
    else
    {
        current_task->static_prio = prio;
        current_task->dynam_prio = prio;
    }
}

int task_getprio(task_t *task)
{
    return (task ? task->static_prio : current_task->static_prio);
}

unsigned int systime()
{
    return system_clock;
}