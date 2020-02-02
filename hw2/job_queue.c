/* Name: Possawat
 * ID: 6081035
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "job_queue.h"

/* Find the active job with the indicated pgid.  */

// job *find_job (pid_t pgid)
// {
//   job *j;

//   for (j = first_job; j; j = j->next)
//     if (j->pgid == pgid)
//       return j;
//   return NULL;
// }

/* Return true if all processes in the job have stopped or completed.  */
int job_is_stopped (job *j)
{
  process *p;
  for (p = j->first_process; p; p = p->next)
    if (!p->completed && !p->stopped)
      return 0;
  return 1;
}

/* Return true if all processes in the job have completed.  */
int job_is_completed (job *j)
{
  process *p;
  for (p = j->first_process; p; p = p->next)
    if (!p->completed)
      return 0;
  return 1;
}

void push(Queue **q, char *word) {
    // IMPLEMENT
    Node* newnode = (Node*)malloc(sizeof(Node));
    char* copy = (char*)malloc(strlen(word)+1);
    strcpy(copy,word);
    newnode->data = copy;
    newnode->next = NULL;

    if(!*q){
        // printf("CREATED QUEUE!\n");
        *q = (Queue*)malloc(sizeof(Queue));
        (*q)->head = NULL;
        (*q)->tail = NULL;
    }

    if(q[0]->head && q[0]->tail){
        // printf("THERE ARE HEAD AND TAIL! ADD TAIL!\n");

        q[0]->tail->next = newnode;
        q[0]->tail = newnode;
    }else if(q[0]->head && !q[0]->tail){
        // printf("THERE IS HEAD! ADD TAIL!\n");

        q[0]->head->next = newnode;
        q[0]->tail = newnode;
    }else{
        // printf("THERE IS NO TAIL!\n");

        q[0]->head = newnode;
    }
    
}

char *pop(Queue *q) {
    // IMPLEMENT
    Node* popping;
    char* popped;
    if(!isEmpty(q)){
        // printf("POPPING\n");
        popping = q->head;
        popped = popping->data;
        q->head = q->head->next;
        if(q->tail) q->tail = q->tail->next;
        free(popping);
        return popped;
    }else{
        return NULL;
    }
}

void print(Queue *q) {
    // IMPLEMENT
    if(isEmpty(q)){
        printf("No items\n");
    }else{
        // printf("Printing  ");
        Node *qhead = q->head;
        do
        {
            char* toprint = qhead->data;
            printf("%s\n", toprint);
            qhead = qhead->next;
        } while (qhead); 
    }
}

int isEmpty(Queue *q) {
    // IMPLEMENT
    if(q){
        if(!(q->head) && !(q->tail)){
            return 1;
        }else{
            return 0;
        } 
    }else{
        return 1;
    }
}

void delete(Queue *q) {
    // IMPLEMENT
    while(!isEmpty(q)){
        Node* toremove = q->head;
        q->head = q->head->next;
        if(q->tail) q->tail = q->tail->next;
        free(toremove->data);
        free(toremove);
    }
}

/***** Expected output: *****
No items
a
b
c
a
b
c
d
e
f
No items
s = World
t = Hello
*****************************/
// int main(int argc, char **argv)
// {
//     Queue *q = NULL;

//     // print the queue
//     print(q);
//     // printf("Inspecting push\n");

//     // push items
//     push(&q, "a");
//     // printf("It is push\n");

//     push(&q, "b");
//     push(&q, "c");
//     print(q);

//     // pop items
//     while (!isEmpty(q)) {
//         char *item = pop(q);
//         printf("%s\n", item);
//         free(item);
//     }

//     char *item = pop(q);
//     assert(item == NULL);

//     // push again
//     push(&q, "d");
//     push(&q, "e");
//     push(&q, "f");
//     print(q);

//     // destroy the queue
//     delete(q);

//     // print again
//     print(q);

//     // check copy
//     char *s = (char *)malloc(10);
//     strcpy(s, "Hello");
//     push(&q, s);
//     strcpy(s, "World");
//     char *t = pop(q);
//     printf("s = %s\n", s);
//     printf("t = %s\n", t);
//     free(t);
//     free(s);

//     // free the queue
//     free(q);
//     return 0;

// }
