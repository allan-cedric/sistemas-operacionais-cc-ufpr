// --- Implementação de uma biblioteca para manipulação de filas circulares (P0) ---

// Autor: Allan Cedric G. B. Alves da Silva - GRR20190351

#include "queue.h"
#include <stdio.h>

int queue_size(queue_t *queue)
{
    // Se a fila está vazia, retorna 0
    if (!queue)
        return 0;

    int size = 1; // Pelo menos tem 1 elemento
    queue_t *aux = queue;
    while ((aux = aux->next) != queue)
        size++;
    return size;
}

void queue_print(char *name, queue_t *queue, void print_elem(void *))
{
    printf("%s", name); // Identificador da fila

    // Se a fila está vazia, imprime uma fila vazia
    if (!queue)
    {
        printf("[]\n");
        return;
    }

    printf("[");
    print_elem(queue); // Pelo menos tem 1 elemento
    queue_t *aux = queue;
    while ((aux = aux->next) != queue)
    {
        printf(" ");
        print_elem(aux);
    }
    printf("]\n");
}

int queue_append(queue_t **queue, queue_t *elem)
{
    if (!queue)
    {
        fprintf(stderr, "Error (queue_append): There is no queue!\n");
        return -1;
    }

    if (!elem)
    {
        fprintf(stderr, "Error (queue_append): There is no element to append!\n");
        return -2;
    }
    else if (elem->prev || elem->next)
    {
        fprintf(stderr, "Error (queue_append): The element already is in a queue!\n");
        return -3;
    }

    // Caso a fila esteja vazia
    if (!*queue)
    {
        *queue = elem; // Cabeça da fila aponta para o elemento

        // Elemento aponta para si próprio
        elem->prev = elem;
        elem->next = elem;
    }
    else
    {
        // O elemento anterior do novo elemento é o último mais recente da fila
        elem->prev = (*queue)->prev;

        // O próximo elemento do novo elemento é o primeiro elemento da fila
        elem->next = *queue;

        // Ajusta o primeiro elemento
        (*queue)->prev = elem;

        // Ajusta o novo penúltimo elemento
        elem->prev->next = elem;
    }
    return 0;
}

int queue_remove(queue_t **queue, queue_t *elem)
{
    if (!queue)
    {
        fprintf(stderr, "Error (queue_remove): There is no queue!\n");
        return -1;
    }

    if (!*queue)
    {
        fprintf(stderr, "Error (queue_remove): Empty queue!\n");
        return -2;
    }

    if (!elem)
    {
        fprintf(stderr, "Error (queue_remove): There is no element to remove!\n");
        return -3;
    }
    else if (!elem->prev || !elem->next)
    {
        fprintf(stderr, "Error (queue_remove): The element is not in any queue!\n");
        return -4;
    }

    queue_t *aux = *queue;
    // Tenta encontrar o elemento na fila corrente
    do
    {
        // Se encontrar o elemento, remove-o da fila
        if (elem == aux)
        {
            if (elem == *queue)
                *queue = (*queue != (*queue)->next ? (*queue)->next : NULL); // Ajusta a cabeça
            if (*queue)
            {
                elem->prev->next = elem->next;
                elem->next->prev = elem->prev;
            }
            elem->next = NULL;
            elem->prev = NULL;
            return 0;
        }
    } while ((aux = aux->next) != *queue);

    fprintf(stderr, "Error (queue_remove): The element is not in this current queue!\n");
    return -5;
}