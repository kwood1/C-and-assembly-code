#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "mergesort.h"

Chunk * head;
pthread_mutex_t mymutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct thread_params{
  size_t elem_size;
  Compare_fn f;
  size_t total_size;
} thread_params;


void merge(void *arr, int first_1, int first_2, size_t num_elem_1, size_t num_elem_2,
size_t elem_size, Compare_fn f);

void* worker_thread(void*);

/* sorts a void* array based in ascending order,
compared with parameter function */
void mergesort(void *arr, size_t num_elem, size_t elem_size, Compare_fn f){

  int mid;

  if(num_elem <= 1)
    return;
  if(arr == NULL || f == NULL)
    return;

  mid = ((num_elem) / 2);
 
  mergesort(arr, mid, elem_size, f);
  mergesort((void*) ((char*)arr+mid*elem_size), mid, elem_size, f);
 
  merge(arr, 0, mid, mid, mid, elem_size, f);
}

void merge(void *arr, int first_1, int first_2, size_t num_elem_1, size_t num_elem_2,
  size_t elem_size, Compare_fn f){

  void* left = malloc(num_elem_1*elem_size);
  void* right = malloc(num_elem_2*elem_size);
  void* arr_ptr = arr;
  void* left_end = (void*)((char*)left+num_elem_1*elem_size);
  void* right_end = (void*)((char*)right+num_elem_2*elem_size);
  memmove(left, (void*)((char*)arr+first_1*elem_size), num_elem_1*elem_size);
  memmove(right, (void*)((char*)arr+first_2*elem_size), num_elem_2*elem_size);
  
  while(left != left_end || right != right_end){
    if(left != left_end && right != right_end){
      if(f(left, right) <= 0){
        memmove(arr_ptr, left, elem_size);
        arr_ptr = (void*)((char*)arr_ptr+elem_size);
        left = (void*)((char*)left+elem_size);
      }
      else{
        memmove(arr_ptr, right, elem_size);
        arr_ptr = (void*)((char*)arr_ptr+elem_size);
        right = (void*)((char*)right+elem_size);
      }
    }
    else if(left != left_end){
      memmove(arr_ptr, left, elem_size);
      arr_ptr = (void*)((char*)arr_ptr+elem_size);
      left = (void*)((char*)left+elem_size);
    }
    else if(right != right_end){
      memmove(arr_ptr, right, elem_size);
      arr_ptr = (void*)((char*)arr_ptr+elem_size);
      right = (void*)((char*)right+elem_size);
    }
  }
}

void mt_mergesort(void *arr, size_t num_elem, int chunk_size, size_t elem_size,
  Compare_fn f, int num_threads){
  pthread_t tid1;
  int i = 0;
  Chunk * curr;
  void* arr_ptr = arr;
  thread_params * params;
  
  if(arr == NULL || num_threads < 1 || num_threads > 1024)
    return;
  if(chunk_size < 1 || chunk_size > (1073741824))
    return;
  params = malloc(sizeof(thread_params));
  params->elem_size = elem_size;
  params->f = f;
  params->total_size = num_elem;
  
  head = (Chunk*)malloc(sizeof(Chunk));
  curr = head;
  for(i = 0; i < num_elem / chunk_size; i++){ /* creating task queue */
    curr->next = (Chunk*)malloc(sizeof(Chunk));
    curr = curr->next;
    
    curr->arr = arr_ptr;
    curr->first = i*chunk_size;
    curr->size = chunk_size;
    curr->done = 0;
  }

  /*linked list created, now create worker threads */
  for(i = 0; i < num_threads; i++){
    pthread_create(&tid1, NULL, worker_thread,(void*) params);
  }
  
  for(i = 0; i < num_threads; i++){
    pthread_join(tid1, NULL);
  }
  
  pthread_mutex_destroy(&mymutex);
  /**/
  
  
  return;
}

int can_merge(Chunk* ch1, Chunk* ch2, size_t elem_size) {
  return ch1->size == ch2->size && ch1->done && ch2->done &&
    (ch1->first == ch2->first + ch2->size || ch2->first == ch1->first + ch1->size);
}

Chunk* merge_chunks(Chunk* ch1, Chunk* ch2, size_t elem_size, Compare_fn cmp){
  Chunk* temp;
  if(ch1->first > ch2->first){
    temp = ch1;
    ch1 = ch2;
    ch2 = temp;
  }
  merge(ch1->arr, ch1->first, ch2->first, ch1->size, ch2->size, elem_size, cmp);
  temp = malloc(sizeof(Chunk));
  temp->arr = ch1->arr;
  temp->size = ch1->size*2;
  temp->first = ch1->first;
  temp->done = 0;
  temp->next = NULL;
  free(ch1);
  free(ch2);
  return temp;
}

void queue_remove(Chunk* ch){
  Chunk* temp = head;
  while(temp->next != NULL){
    if(temp->next == ch){
      temp->next = temp->next->next;
      break;
    }
    temp = temp->next;
  }
}

void queue_add(Chunk* ch){
  Chunk* temp = head;
  while(temp->next != NULL){
    temp = temp->next;
  }
  temp->next = ch;
  ch->next = NULL;
}

/* mergesorts unfinished chunks, them merges done chunks */
void* worker_thread(void* f){

  Chunk* curr;
  Chunk* temp;
  Chunk* buddy;
  size_t elem_size = ((thread_params*) f)->elem_size;
  Compare_fn cmp = ((thread_params*) f)->f;
  size_t total_size = ((thread_params*) f)->total_size;
  int size = 0;
  
  while(1){
    pthread_mutex_lock(&mymutex);
    if(head->next == NULL){
      pthread_mutex_unlock(&mymutex);
      return NULL;
    }
    if(head->next->size == total_size){
      pthread_mutex_unlock(&mymutex);
      return NULL;
    }
    
    curr = NULL;
    temp = head->next;
    while(temp != NULL){
      if(temp->done == 0){
        curr = temp;
        break;
      }
      temp = temp->next;
    }
    if(curr != NULL){
      queue_remove(curr);
    }
    else{
      return NULL;
    }
    
    curr->next = NULL;
    pthread_mutex_unlock(&mymutex);
    
    if((curr->done) == 0){
      mergesort(curr->arr, curr->size, elem_size, cmp);
      curr->done = 1;
    }
    
    pthread_mutex_lock(&mymutex);
    buddy = head->next;
    while(buddy != NULL){
      if(buddy->done == 0 || curr->size != buddy->size){
        buddy = buddy->next;
        continue;
      }
      
      if(can_merge(curr, buddy, elem_size)){
        queue_remove(buddy);
        curr = merge_chunks(curr, buddy, elem_size, cmp);
      }
      
      buddy = buddy->next;
    }
    
    queue_add(curr);
    
    pthread_mutex_unlock(&mymutex);
  }
  
  return NULL;
}
