// Allan Cedric G. B. Alves da Silva - GRR20190351

// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.2 -- Julho de 2017

// interface do gerente de disco rígido (block device driver)

#ifndef __DISK_MGR__
#define __DISK_MGR__

#include "ppos.h"
#include "disk.h"

// estruturas de dados e rotinas de inicializacao e acesso
// a um dispositivo de entrada/saida orientado a blocos,
// tipicamente um disco rigido.

// estrutura da fila de um disco
typedef struct
{
  struct disk_queue_t *prev, *next ;  // ponteiros para usar em filas
  task_t *requester ; // Tarefa solicitante
  int op_type ; // Tipo de operação requisitada
  int block ; // Bloco desejado
  void *buffer ; // Buffer de dados
} disk_queue_t ;

// estrutura que representa um disco no sistema operacional
typedef struct
{
  int num_blocks ; // Quantidade blocos do disco
  int block_size ; // Tamanho de um bloco do disco
  task_t driver ; // Driver do disco
  semaphore_t s_access ;  // Semáforo de acesso ao disco
  disk_queue_t *queue ; // Fila do disco
  struct sigaction sig_handler ; // Tratador de sinais do disco
  int sig_disk ; // Flag que verifica se o disco gerou um sinal(1), senão (0)
  // completar com os campos necessarios
} disk_t ;

// inicializacao do gerente de disco
// retorna -1 em erro ou 0 em sucesso
// numBlocks: tamanho do disco, em blocos
// blockSize: tamanho de cada bloco do disco, em bytes
int disk_mgr_init (int *numBlocks, int *blockSize) ;

// leitura de um bloco, do disco para o buffer
int disk_block_read (int block, void *buffer) ;

// escrita de um bloco, do buffer para o disco
int disk_block_write (int block, void *buffer) ;

#endif
