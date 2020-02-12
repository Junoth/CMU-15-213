/* 
 * Code for basic C skills diagnostic.
 * Developed for courses 15-213/18-213/15-513 by R. E. Bryant, 2017
 */

/*
 * This program implements a queue supporting both FIFO and LIFO
 * operations.
 *
 * It uses a singly-linked list to represent the set of queue elements
 */

#include <stdlib.h>
#include <stdio.h>

#include "harness.h"
#include "queue.h"

/*
  Create empty queue.
  Return NULL if could not allocate space.
*/
queue_t *q_new()
{
    queue_t *q =  malloc(sizeof(queue_t));
    // Return NULL if could not allocate space
    if (q == NULL) return NULL;

    q->head = q->tail = NULL;
    q->size = 0;
    return q;
}

/* Free all storage used by queue */
void q_free(queue_t *q)
{
    // Return if q is NULL
    if (q == NULL) return;

    /* Free queue structure */
    list_ele_t *next = q->head;
    while (next != NULL) {
      list_ele_t *temp = next->next;
      free(next);
      next = temp;
    }

    free(q);
    q = NULL;
}

/*
  Attempt to insert element at head of queue.
  Return true if successful.
  Return false if q is NULL or could not allocate space.
 */
bool q_insert_head(queue_t *q, int v)
{
    if (q == NULL) return false;

    list_ele_t *newh;
    newh = malloc(sizeof(list_ele_t));
    if (newh == NULL) return false;

    newh->value = v;
    newh->next = q->head;
    if (q->head != NULL) {
      q->head = newh;
    } else {
      q->head = q->tail = newh;
    }

    q->size++;
    return true;
}


/*
  Attempt to insert element at tail of queue.
  Return true if successful.
  Return false if q is NULL or could not allocate space.
 */
bool q_insert_tail(queue_t *q, int v)
{
    /* You need to write the complete code for this function */
    /* Remember: It should operate in O(1) time */
    if (q == NULL) return false;

    list_ele_t *newh;
    newh = malloc(sizeof(list_ele_t));
    if (newh == NULL) return false;

    newh->value = v;
    newh->next = NULL;
    if (q->tail != NULL) {
      q->tail->next = newh;
      q->tail = newh;
    } else {
      q->head = q->tail = newh;
    }

    q->size++;
    return true;
}

/*
  Attempt to remove element from head of queue.
  Return true if successful.
  Return false if queue is NULL or empty.
  If vp non-NULL and element removed, store removed value at *vp.
  Any unused storage should be freed
*/
bool q_remove_head(queue_t *q, int *vp)
{
    /* Return false if queue is NULL or empty */
    if (q == NULL || q->head == NULL || vp == NULL) return false;

    *vp = q->head->value;
    if (q->size == 1) {
      free(q->head);
      q->head = q->tail = NULL;
    } else {
      list_ele_t *temp = q->head->next;
      free(q->head);
      q->head = temp;
    }
    q->size--;

    return true;
}

/*
  Return number of elements in queue.
  Return 0 if q is NULL or empty
 */
int q_size(queue_t *q)
{
    /* You need to write the code for this function */
    /* Remember: It should operate in O(1) time */
    if (q == NULL) return 0;
    return q->size;
}

/*
  Reverse elements in queue.

  Your implementation must not allocate or free any elements (e.g., by
  calling q_insert_head or q_remove_head).  Instead, it should modify
  the pointers in the existing data structure.
 */
void q_reverse(queue_t *q)
{
    /* You need to write the code for this function */
    if (q == NULL || q->head == NULL) return;

    list_ele_t* curr = q->head;
    list_ele_t* next = q->head->next;
    q->head->next = NULL;
    while (next != NULL) {
      list_ele_t* temp = next->next;
      next->next = curr;
      curr = next;
      next = temp;
    }
    q->tail = q->head;
    q->head = curr;
}

