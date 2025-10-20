#include "hash_table.h"
#include <stdio.h>
/**
 * Hash function that takes a string (max 20 chars) and returns hash index 0-127
 * Uses djb2 algorithm with modulo to fit range
 * @param str Input string (max 20 characters)
 * @return Hash index in range 0-127
 */
unsigned int myshell_hash_string(const char* str) {
    if (str == NULL) {
        return 0;
    }
    
    unsigned long hash = 5381;  // djb2 initial value
    int c;
    int char_count = 0;

    // Process up to MYSHELL_MAX_HASH_INPUT_LENGTH characters
    while ((c = *str++) && char_count < MYSHELL_MAX_HASH_INPUT_LENGTH) {
        // djb2 hash: hash * 33 + c
        // shift and add is done because multiplication might be costly if
        // compiler optimisations is not good enough, or it is run in a less capable CPU like ARM
        hash = ((hash << 5) + hash) + c;
        char_count++;
    }

    // Return hash modulo MYSHELL_HASH_TABLE_SIZE to fit in range 0-127
    return (unsigned int)(hash % MYSHELL_HASH_TABLE_SIZE);
}

/**
 * Alternative hash function using FNV-1a algorithm
 * @param str Input string (max 20 characters)
 * @return Hash index in range 0-127
 */
unsigned int myshell_hash_string_fnv(const char* str) {
    if (str == NULL) {
        return 0;
    }
    
    // FNV-1a constants
    const unsigned int FNV_OFFSET_BASIS = 2166136261U;
    const unsigned int FNV_PRIME = 16777619U;
    
    unsigned int hash = FNV_OFFSET_BASIS;
    int char_count = 0;

    // Process up to MYSHELL_MAX_HASH_INPUT_LENGTH characters
    while (*str && char_count < MYSHELL_MAX_HASH_INPUT_LENGTH) {
        hash ^= (unsigned int)(*str);
        hash *= FNV_PRIME;
        str++;
        char_count++;
    }

    // Return hash modulo MYSHELL_HASH_TABLE_SIZE to fit in range 0-127
    return hash % MYSHELL_HASH_TABLE_SIZE;
}

/**
 * Simple polynomial rolling hash function
 * @param str Input string (max 20 characters)
 * @return Hash index in range 0-127
 */
unsigned int myshell_hash_string_poly(const char* str) {
    if (str == NULL) {
        return 0;
    }
    
    unsigned long hash = 0;
    unsigned long pow = 1;
    const unsigned long base = 31;  // Common base for polynomial hashing
    int char_count = 0;

    // Process up to MYSHELL_MAX_HASH_INPUT_LENGTH characters
    while (*str && char_count < MYSHELL_MAX_HASH_INPUT_LENGTH) {
        hash = (hash + ((*str - 'a' + 1) * pow)) % MYSHELL_HASH_TABLE_SIZE;
        pow = (pow * base) % MYSHELL_HASH_TABLE_SIZE;
        str++;
        char_count++;
    }
    
    return (unsigned int)hash;
}
