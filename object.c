#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "value.h"
#include "vm.h"

#define ALLOCATE_OBJ(type, objectType) \
    (type*) allocateObject(sizeof(type), objectType)

static Obj* allocateObject(size_t size, ObjType type) {
    Obj* object = (Obj*) reallocate(NULL, 0, size);
    object->type = type;
    return object;
}

// helper function to allocate an object which is a string
static ObjString* allocateString(char* chars, int length) {
    ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
    string->length = length;
    string->chars = chars;
    return string;
}

// Like copyString, except it's not a copy; allocates a new object whose
// data is exactly the given chars array
ObjString* takeString(char* chars, int length) {
    return allocateString(chars, length);
}

// copy a string (which is not null terminated) into a null terminated string
// which is then wrapped up as an ObjString. Caller owns the pointer.
// Original chars are not edited and are disjoint from the returned pointer.
ObjString* copyString(const char* chars, int length) {
    char* heapChars = ALLOCATE(char, length+1);
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';
    return allocateString(heapChars, length);
}

void printObject(Value value) {
    switch (OBJ_TYPE(value)) {
        case OBJ_STRING:
            printf("%s", AS_CSTRING(value));
            break;
    }
}