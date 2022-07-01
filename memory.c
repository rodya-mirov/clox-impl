#include <stdlib.h>

#include "memory.h"

void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
    if (newSize == 0) {
        free(pointer);
        return NULL;
    }

    void* result = realloc(pointer, newSize);

    if (result == NULL) {
        // TODO: some kind of helpful log about how we can't
        // allocate memory???
        exit(1);
    }

    return result;
}