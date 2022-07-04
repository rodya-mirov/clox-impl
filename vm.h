#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"

// TODO: the data layout of this VM doesn't really make sense to me
typedef struct {
    Chunk* chunk;
    // an actual pointer into the code array in chunk, in gross defiance
    // of all that is holy
    // I don't think I'm really cut out for C tbh
    // this is a pointer to the instruction that is _about to be_ executed
    uint8_t* ip;
} VM;

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR,
} InterpretResult;

void initVM(VM* vm);
void freeVM(VM* vm);

// rigs up the VM to start interpreting the given chunk, then executes it until a return
InterpretResult interpret(VM* vm, Chunk* chunk);

#endif