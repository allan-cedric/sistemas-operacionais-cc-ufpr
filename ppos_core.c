// --- Implementação dos principais mecanismos do PingPongOS ---

// Allan Cedric G. B. Alves da Silva - GRR20190351

#include "ppos.h"

// --- Variáveis globais do sistema ---
int pid = 0, num_user_tasks = 0;
int counter_ticks;
unsigned int system_clock = 0;
int lock_kernel;

task_t main_task, dispatcher, *current_task;
task_t *user_tasks_queue = NULL, *sleeping_tasks_queue = NULL;

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

#ifdef DEBUG
    queue_print("PPOS (scheduler): User's ready tasks -> ", (queue_t *)user_tasks_queue, print_task_id);
#endif

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
    @brief  Termina uma tarefa

    @param  task    Ponteiro para uma tarefa
*/
void terminate_task(task_t *task)
{
    fprintf(
        stdout, "Task %i exit: execution time %i ms, processor time %i ms, %i activations\n",
        task->id,
        systime() - task->born_timestamp,
        task->cpu_time,
        task->cpu_activations);

    // Restaura as tarefas em espera (join)
    task_t *cur = task->waiting_tasks;
    while (cur)
    {
        cur->state = READY;
        queue_remove((queue_t **)&task->waiting_tasks, (queue_t *)cur);
        queue_append((queue_t **)&user_tasks_queue, (queue_t *)cur);
        cur = task->waiting_tasks;
    }

    queue_remove((queue_t **)&user_tasks_queue, (queue_t *)task);
    free_task(&task);
    num_user_tasks--;
}

/*!
    @brief  Rotina para o despachante acordar as tarefas que foram dormir (sleep)
*/
void wake_tasks()
{
    int size = queue_size((queue_t *)sleeping_tasks_queue);
    task_t *cur = sleeping_tasks_queue;
    while (size--)
    {
        task_t *next = cur->next; // Salva a ref. para o próximo elemento
        if (systime() >= cur->wake_time)
        {
            cur->state = READY;
            queue_remove((queue_t **)&sleeping_tasks_queue, (queue_t *)cur);
            queue_append((queue_t **)&user_tasks_queue, (queue_t *)cur);

#ifdef DEBUG
            fprintf(stdout, "PPOS (task_sleep): task %i woke up!\n", cur->id);
#endif
        }
        cur = next; // Restaura para o próximo elemento
    }
}

/*!
    @brief  Rotina do despachante de tarefas.
*/
void dispatcher_proc(void *arg)
{
    unsigned int init_disp_time = systime();

#ifdef DEBUG
    fprintf(stdout, "PPOS (dispatcher): Dispatcher was launched!\n");
#endif

    while (num_user_tasks)
    {

        // #ifdef DEBUG
        //         queue_print("PPOS (dispatcher): User's ready tasks -> ", (queue_t *)user_tasks_queue, print_task_id);
        // #endif

        task_t *next_task = scheduler();

        if (next_task)
        {
            counter_ticks = next_task->quantum_ticks;

            unsigned int init_cpu_time = systime();

            dispatcher.cpu_time += (systime() - init_disp_time);
            task_switch(next_task);
            init_disp_time = systime();

            next_task->cpu_time += (systime() - init_cpu_time);

            // Tratamento de estados da tarefa
            switch (next_task->state)
            {
            case TERM:
                terminate_task(next_task);
                break;
            default:
                break;
            }
        }

        // Restaura tarefas dormindo
        wake_tasks();
    }
    dispatcher.cpu_time += (systime() - init_disp_time);
    task_exit(0);
}

/*!
    @brief  Rotina de tratamento de interrupção: Preempção de tarefas
*/
void task_preemption()
{
    system_clock++;

    // Caso seja uma tarefa de usuário e não seja uma rotina de kernel, trata da preempção
    if ((task_id() != 1) && !lock_kernel)
    {
        if (!(--counter_ticks))
            task_switch(&dispatcher);
    }
}

void ppos_init()
{
    lock_kernel = 1;

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
    main_task.waiting_tasks = NULL;
    main_task.exit_code = 0;

    // Inicialização da tarefa atual
    current_task = &main_task;

#ifdef DEBUG
    fprintf(stdout, "PPOS (ppos_init): task %i (%p) was created successfully!\n", task_id(), &main_task);
#endif

    // Adiciona a tarefa main na fila de tarefas prontas do usuário
    queue_append((queue_t **)&user_tasks_queue, (queue_t *)&main_task);
    num_user_tasks++;

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
    lock_kernel = 0;
}

int task_create(task_t *task, void (*start_func)(void *), void *arg)
{
    lock_kernel = 1;

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
    task->waiting_tasks = NULL;
    task->exit_code = 0;
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
    if (task->id != 1)
    {
        queue_append((queue_t **)&user_tasks_queue, (queue_t *)task);
        num_user_tasks++;
    }

    lock_kernel = 0;
    return task->id;
}

void task_exit(int exitCode)
{
    lock_kernel = 1;

#ifdef DEBUG
    fprintf(stdout, "PPOS (task_exit): task %i is terminating...\n", task_id());
#endif

    current_task->state = TERM;
    current_task->exit_code = exitCode;

    switch (task_id())
    {
    case 1: // Despachante
        fprintf(
            stdout, "Task %i exit: execution time %i ms, processor time %i ms, %i activations\n",
            task_id(),
            systime() - current_task->born_timestamp,
            current_task->cpu_time,
            current_task->cpu_activations);
        exit(current_task->exit_code);
        break;
    default: // Tarefa genérica do usuário
        task_switch(&dispatcher);
        break;
    }

    lock_kernel = 0;
}

int task_switch(task_t *task)
{
    lock_kernel = 1;

    // #ifdef DEBUG
    //     fprintf(stdout, "PPOS (task_switch): current task %i to task %i...\n", task_id(), task->id);
    // #endif

    task_t *aux = current_task;
    current_task = task;

    task->cpu_activations++;
    lock_kernel = 0;
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
    lock_kernel = 1;

#ifdef DEBUG
    fprintf(stdout, "PPOS (task_yield): task %i yields the CPU!\n", task_id());
#endif

    task_switch(&dispatcher);
    lock_kernel = 0;
}

void task_setprio(task_t *task, int prio)
{
    lock_kernel = 1;
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
    lock_kernel = 0;
}

int task_getprio(task_t *task)
{
    return (task ? task->static_prio : current_task->static_prio);
}

int task_join(task_t *task)
{
    lock_kernel = 1;

#ifdef DEBUG
    fprintf(stdout, "PPOS (task_join): task %i waits task %i\n", task_id(), task->id);
#endif

    if (!task || task->state == TERM)
    {
        lock_kernel = 0;
        return -1;
    }

    current_task->state = SUSPENDED;
    queue_remove((queue_t **)&user_tasks_queue, (queue_t *)current_task);
    queue_append((queue_t **)&task->waiting_tasks, (queue_t *)current_task);
    task_switch(&dispatcher);

    lock_kernel = 0;
    return task->exit_code;
}

void task_sleep(int t)
{
    lock_kernel = 1;

#ifdef DEBUG
    fprintf(stdout, "PPOS (task_sleep): task %i will sleep for %i ms\n", task_id(), t);
#endif

    current_task->wake_time = systime() + (unsigned int)t;
    current_task->state = SUSPENDED;
    queue_remove((queue_t **)&user_tasks_queue, (queue_t *)current_task);
    queue_append((queue_t **)&sleeping_tasks_queue, (queue_t *)current_task);
    task_switch(&dispatcher);
    lock_kernel = 0;
}

unsigned int systime()
{
    return system_clock;
}