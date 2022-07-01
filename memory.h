#ifndef clox_memory_h
#define clox_memory_h

#include "common.h"

// Not sure why this is a macro instead of a normal function but whatever
// Given the current capacity (specified) is too little, returns the new
// capacity of the array after it grows
#define GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) * 2)

// Does the actual growing, given args:
//  (a) the type of element in the array (used for sizing)
//  (b) the pointer to the array (will be mutated, so the existing references will still work),
//  (c) the oldCount (current size of the array)
//  (d) the newCount (desired size of the array)
#define GROW_ARRAY(type, pointer, oldCount, newCount) \
    (type*) reallocate(pointer, sizeof(type) * (oldCount), sizeof(type) * (newCount))

#define FREE_ARRAY(type, pointer, oldCount) \
    (type*) reallocate(pointer, sizeof(type) * oldCount, 0)

void* reallocate(void* pointer, size_t oldSize, size_t newSize);

#endif