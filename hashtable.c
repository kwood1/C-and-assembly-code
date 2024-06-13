* Kevin Wood 111235445 kwood123 */

#include <stdio.h> 
#include <string.h>
#include <stdlib.h>
#include "hashtable.h"

/* initalizes all bucket states to empty */
void init_table(Table * t){
  int i = 0;
  
  if(t != NULL){
    for(i = 0; i < NUM_BUCKETS;i++){
      Hash_bucket h = {EMPTY, {"",""}};
      t->buckets[i] = h;
    }
  }
}

/* resets all bucket states to empty, deletes all data pairs */
void reset_table(Table * t){
  int i = 0;
 
  if(t != NULL){
    for(i = 0; i < NUM_BUCKETS; i++){
      t->buckets[i].state = EMPTY;
      strcpy(t->buckets[i].data.key, "");
      strcpy(t->buckets[i].data.value, "");
      
    }
  }
  t->key_ct = 0;
}

/* inserts key/value pair into table if there is room */
int insert(Table * t, const char * key, const char * val){
  unsigned long hash = 0; 
  int i = 0;
  int j = 0; 
  int check = 0;
  char mutable_val[MAX_STR_SIZE];
  
  /* checking for possible failures */
  if(key == NULL || val == NULL){
    return -1;
  }
  if(strcmp(key, "") == 0 || strcmp(val,"") == 0){
    return -1;
  }
  if(strlen(key) > MAX_STR_SIZE || strlen(val) > MAX_STR_SIZE){
    return -1;
  }
  if(t == NULL){
    return -1;
  }
  
  /* to allow passing into search, create non-const string */
  strcpy(mutable_val, val);

  check = search(t,key, mutable_val);
  /* checks if key is in the table */

  hash = hash_code(key); /* getting hashcode of key */

  /* if key was found, need to overwrite value */
   if(check == 0){
     for(i = hash % NUM_BUCKETS;j < NUM_BUCKETS; j++){
       if(t->buckets[i].state == FULL 
	  && strcmp(t->buckets[i].data.key, key) == 0){ 
	 strcpy(t->buckets[i].data.value, val);
	 return 0;
       }
       i = (i+1) % NUM_BUCKETS; /* incrementing i */
     }
     return 0;
  }

  /* if key isn't in table and the table is full, insertion fails */
  if(t->key_ct >= NUM_BUCKETS && check != 0){
    return -1;
  }
  
  /* inserting into an empty/deleted spot if key isn't already in table */
  for(i = hash%NUM_BUCKETS;j < NUM_BUCKETS; j++){
    if(t->buckets[i].state == EMPTY || t->buckets[i].state == DELETED){ 
      strcpy(t->buckets[i].data.key, key);
      strcpy(t->buckets[i].data.value, val);
      t->buckets[i].state = FULL;
      t->key_ct++;
      return 0;
    }
    i = (i+1) % NUM_BUCKETS; /* incrementing i */
  }
  return -1; 
}

/* searches for a key, assigns parameter *val to found value */
int search(Table * t, const char * key, char * val){
  int i = 0;
  int j = 0;
  unsigned long hash = hash_code(key);

  /* checking for failure */
  if(t == NULL || key == NULL){
    return -1;
  }

  /* searching for key, starting at hash value */
  for(i = hash % NUM_BUCKETS;j < NUM_BUCKETS; j++ ){
    if(t->buckets[i].state == FULL){
      if(strcmp(t->buckets[i].data.key, key) == 0){
	 /* if parameter is NULL, don't copy but return 0 for found */
	if(val == NULL){
	  return 0;
	}
	 /* if not NULL, copying and returning 0 */
	strcpy(val, t->buckets[i].data.value);
	return 0;
      }
    }
   
    i = (i+1) % NUM_BUCKETS; /* incrementing i */
  }
  
  /* if key was not found */
  return -1;
}

/* deletes key from table, along with its value */
int delete(Table * t, const char * key){
  int i = 0;
  int j = 0;
  unsigned long hash = hash_code(key);
  
  /* checking for failure */
  if(t == NULL || key == NULL){
    return -1;
  }
  
  /* searching for key to delete */
  for(i = hash % NUM_BUCKETS;j < NUM_BUCKETS; j++ ){
    if(t->buckets[i].state == FULL){
      if(strcmp(t->buckets[i].data.key, key) == 0){
	t->buckets[i].state = DELETED;
	t->key_ct--;
	return 0;
      }
    }
    i = (i+1) % NUM_BUCKETS;
  }

 return -1;
}

/* returns key count of the table */
int key_count(Table * t){
  if(t == NULL){
    return -1;
  }
  return t->key_ct;
}

/* returns bucket count (length) of the table */
int bucket_count(Table * t){
  if(t == NULL){
    return -1;
  }
  return NUM_BUCKETS;
}

/* returns the hash code of the string */
unsigned long hash_code(const char * str){
  int i = 0;
  unsigned long ret = 0;
  
  if (str == NULL || strlen(str) == 0){
    return 0;
  }
  /* iterating through each element of string */
  for ( i = 0; str[i] != '\0'; i++) {
    ret *= 65599;
    ret += (int) str[i];
  }
  return ret;
}
