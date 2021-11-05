// Allan Cedric G. B. Alves da Silva - GRR20190351

// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.1 -- Julho de 2016

// Estruturas de dados internas do sistema operacional

#ifndef __PPOS_DATA__
#define __PPOS_DATA__

#include <ucontext.h>		// biblioteca POSIX de trocas de contexto
#include "queue.h"		// biblioteca de filas genéricas
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>

// --- Macros ---
#define MAX_PRIO -20
#define MIN_PRIO (-MAX_PRIO)
#define DEFAULT_PRIO 0
#define ALPHA -1

#define DEFAULT_QUANTUM_TICKS 10
#define MAIN_PID 0
#define STACK_SIZE (64 * 1024)

// --- Definições de tipos convenientes ---
typedef char byte_t;
typedef enum task_states_t
{
    TERM,
    READY,
    SUSPENDED
} task_states_t;

// Estrutura que define um Task Control Block (TCB)
typedef struct task_t
{
   struct task_t *prev, *next ;		// ponteiros para usar em filas
   int id ;				// identificador da tarefa
   ucontext_t context ;			// contexto armazenado da tarefa
   int state ;    // Estado da tarefa
   int static_prio ; // Prioridade estática
   int dynam_prio ; // Prioridade dinâmica
   int quantum_ticks ; // Quantum em ticks da tarefa
   unsigned int born_timestamp ; // Tempo/Momento de nascimento
   unsigned int cpu_time ; // Tempo total de uso da CPU
   unsigned int cpu_activations ; // Ativações na CPU
   struct task_t *waiting_tasks ; // Fila de tarefas em espera
   int exit_code ; // Código de encerramento
   unsigned int wake_time ; // Tempo para acordar
   // ... (outros campos serão adicionados mais tarde)
} task_t ;

// estrutura que define um semáforo
typedef struct
{
  task_t *queue ; // Fila de tarefas do semáforo
  int counter ; // Contador (vagas)
  int destroyed ; // Flag que indica se o semáforo está destruído(1) ou não(0)
  // preencher quando necessário
} semaphore_t ;

// estrutura que define um mutex
typedef struct
{
  // preencher quando necessário
} mutex_t ;

// estrutura que define uma barreira
typedef struct
{
  // preencher quando necessário
} barrier_t ;

// estrutura que define uma fila de mensagens
typedef struct
{
  // preencher quando necessário
} mqueue_t ;

#endif

