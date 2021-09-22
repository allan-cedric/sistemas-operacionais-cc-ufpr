#include "queue.h"
#include <stdio.h>

int queue_size(queue_t *queue)
{
    // Se a fila está vazia
    if (!queue)
        return 0;

    // Caso a fila não esteja vazia
    int size = 1;
    queue_t *aux = queue->next;
    while (queue != aux)
    {
        size++;
        aux = aux->next;
    }

    return size;
}

void queue_print(char *name, queue_t *queue, void print_elem(void *))
{
    printf("%s", name);

    // Se a fila está vazia
    if (!queue)
    {
        printf("[]\n");
        return;
    }

    // Caso a fila não esteja vazia
    printf("[");
    print_elem(queue);
    queue_t *aux = queue->next;
    while (queue != aux)
    {
        printf(" ");
        print_elem(aux);
        aux = aux->next;
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

        // Ajusta o penúltimo elemento
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
        fprintf(stderr, "Error (queue_append): The element is not in any queue!\n");
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
                *queue = (*queue != (*queue)->next ? (*queue)->next : NULL); // Atualiza a cabeça
            if (*queue)
            {
                elem->prev->next = elem->next;
                elem->next->prev = elem->prev;
            }
            elem->next = NULL;
            elem->prev = NULL;
            return 0;
        }
        aux = aux->next;
    } while (*queue != aux);

    // Caso não encontre o elemento na fila
    fprintf(stderr, "Error (queue_remove): The element is not in this current queue!\n");
    return -5;
}