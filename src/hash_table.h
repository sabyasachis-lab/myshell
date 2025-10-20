#ifndef MYSHELL_HASH_TABLE_H
#define MYSHELL_HASH_TABLE_H

#include <stdio.h>   // for fprintf
#include <stdlib.h>  // for malloc/free

// Hash functions
unsigned int myshell_hash_string(const char* str);
unsigned int myshell_hash_string_fnv(const char* str);
unsigned int myshell_hash_string_poly(const char* str);

#define MYSHELL_HASH_TABLE_SIZE 128
#define MYSHELL_MAX_HASH_INPUT_LENGTH 20
#define MYSHELL_HASH_INDEX(str) myshell_hash_string(str)

/*
#define myshell_hash_table(item_type) \
    struct myshell_hash_table_entry { \
        const char* key; \
        item_type handler; \
    }; \


myshell_hash_table(myshell_command_handler_t);
*/
typedef struct myshell_hash_table {
    void* entries[MYSHELL_HASH_TABLE_SIZE];
} myshell_hash_table_t;

#define MYSHELL_HASH_TABLE_INIT(item_type, hash_table_ptr) do { \
    fflush(stderr); \
    if(hash_table_ptr != NULL) { \
        fprintf(stderr, "ERROR: Hash table already initialized, ptr = %p\n", (void*)hash_table_ptr); \
        fflush(stderr); \
        abort(); \
    } \
    fflush(stderr); \
    hash_table_ptr = (myshell_hash_table_t*)malloc(sizeof(myshell_hash_table_t)); \
    if(hash_table_ptr == NULL) { \
        fprintf(stderr, "ERROR: malloc failed for hash table\n"); \
        fflush(stderr); \
        abort(); \
    } \
    fflush(stderr); \
    for (unsigned int i = 0; i < MYSHELL_HASH_TABLE_SIZE; i++) { \
        hash_table_ptr->entries[i] = NULL; \
    } \
    fflush(stderr); \
} while(0);

#define MYSHELL_HASH_TABLE_FREE(hash_table_ptr) do { \
    free(hash_table_ptr); \
    hash_table_ptr = NULL; \
} while(0);

#define MYSHELL_HASH_TABLE_INSERT(item_type, hash_table_ptr, key, value_ptr) do { \
    unsigned int index = MYSHELL_HASH_INDEX(key); \
    hash_table_ptr->entries[index] = (item_type*)value_ptr; \
} while(0);

#define MYSHELL_HASH_TABLE_LOOKUP(item_type, hash_table_ptr, key, result_ptr) do { \
    fflush(stderr); \
    if (hash_table_ptr == NULL) { \
        fprintf(stderr, "ERROR: Hash table pointer is NULL during lookup!\n"); \
        fflush(stderr); \
        result_ptr = NULL; \
        break; \
    } \
    unsigned int index = MYSHELL_HASH_INDEX(key); \
    fflush(stderr); \
    result_ptr = (item_type*)hash_table_ptr->entries[index]; \
    fflush(stderr); \
} while(0);

#endif // MYSHELL_HASH_TABLE_H