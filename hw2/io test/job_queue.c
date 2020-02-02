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

int job_push(job** j, char* cmd, pid_t pgid, process* childprocess){
    job *newjob = (job *)malloc(sizeof(job));
    char *cmdcp = (char*)malloc(strlen(cmd)+1);
    strcpy(cmdcp, cmd);
    newjob->command = cmdcp;
    newjob->pgid = pgid;
    newjob->next = NULL;
    newjob->first_process = childprocess;

    if(!*j){
        printf("*****************first job*****************\n");
        *j = newjob;
    }else{
        printf("*****************new job*****************\n");
        job* ji = *j;
        for (ji = *j; ji; ji = ji->next){
          if(!ji->next){
            ji->next = newjob;
            return 1;
          } 
        } 
    }
}

process *initpro(pid_t pid, char** argv, int status){
    process *newprocess = malloc(sizeof(process));
    newprocess->argv = argv;
    newprocess->completed = 0;
    newprocess->pid = pid;
    newprocess->stopped = 0;
    newprocess->status = status;
    newprocess->next = NULL;
    return newprocess;
}


// void push(Queue **q, char *word) {
//     // IMPLEMENT
//     Node* newnode = (Node*)malloc(sizeof(Node));
//     char* copy = (char*)malloc(strlen(word)+1);
//     strcpy(copy,word);
//     newnode->data = copy;
//     newnode->next = NULL;

//     if(!*q){
//         // printf("CREATED QUEUE!\n");
//         *q = (Queue*)malloc(sizeof(Queue));
//         (*q)->head = NULL;
//         (*q)->tail = NULL;
//     }

//     if(q[0]->head && q[0]->tail){
//         // printf("THERE ARE HEAD AND TAIL! ADD TAIL!\n");

//         q[0]->tail->next = newnode;
//         q[0]->tail = newnode;
//     }else if(q[0]->head && !q[0]->tail){
//         // printf("THERE IS HEAD! ADD TAIL!\n");

//         q[0]->head->next = newnode;
//         q[0]->tail = newnode;
//     }else{
//         // printf("THERE IS NO TAIL!\n");

//         q[0]->head = newnode;
//     }
    
// }

// char *pop(Queue *q) {
//     // IMPLEMENT
//     Node* popping;
//     char* popped;
//     if(!isEmpty(q)){
//         // printf("POPPING\n");
//         popping = q->head;
//         popped = popping->data;
//         q->head = q->head->next;
//         if(q->tail) q->tail = q->tail->next;
//         free(popping);
//         return popped;
//     }else{
//         return NULL;
//     }
// }

// void print(Queue *q) {
//     // IMPLEMENT
//     if(isEmpty(q)){
//         printf("No items\n");
//     }else{
//         // printf("Printing  ");
//         Node *qhead = q->head;
//         do
//         {
//             char* toprint = qhead->data;
//             printf("%s\n", toprint);
//             qhead = qhead->next;
//         } while (qhead); 
//     }
// }

// int isEmpty(Queue *q) {
//     // IMPLEMENT
//     if(q){
//         if(!(q->head) && !(q->tail)){
//             return 1;
//         }else{
//             return 0;
//         } 
//     }else{
//         return 1;
//     }
// }

// void delete(Queue *q) {
//     // IMPLEMENT
//     while(!isEmpty(q)){
//         Node* toremove = q->head;
//         q->head = q->head->next;
//         if(q->tail) q->tail = q->tail->next;
//         free(toremove->data);
//         free(toremove);
//     }
// }

// int main(int argc, char **argv)
// {
//     job *first_job = NULL;
    
//     return 0;

// }
