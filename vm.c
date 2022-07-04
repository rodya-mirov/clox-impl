#include <stdio.h>

#include "common.h"
#include "vm.h"
#include "debug.h"

void initVM(VM* vm) {

}

void freeVM(VM* vm) {

}

static int assemble_three(uint8_t a, uint8_t b, uint8_t c) {
    int out = a;
    out = out << 8;
    out += (int) b;
    out = out << 8;
    out += (int) c;
    return out;
}

static uint8_t read_byte(VM* vm) {
    return (*((vm->ip)++));
}

static Value readConstant(VM* vm) {
    int constantIdx = read_byte(vm);
    return vm->chunk->constants.values[constantIdx];
}

static Value readConstantLong(VM* vm) {
    uint8_t a = read_byte(vm);
    uint8_t b = read_byte(vm);
    uint8_t c = read_byte(vm);
    int constantIdx = assemble_three(a, b, c);
    return vm->chunk->constants.values[constantIdx];
}

static InterpretResult run(VM* vm) {

    for(;;) {
        #ifdef DEBUG_TRACE_EXECUTION
            disassembleInstruction(vm->chunk, (int)(vm->ip - vm->chunk->code));
        #endif

        uint8_t instruction;
        switch (instruction = read_byte(vm)) {
            case OP_CONSTANT: {
                Value constant = readConstant(vm);
                printValue(constant);
                printf("\n");
                break;
            }

            case OP_CONSTANT_LONG: {
                Value constant = readConstantLong(vm);
                printValue(constant);
                printf("\n");
                break;
            }

            case OP_RETURN:
                return INTERPRET_OK;

            default:
                printf("Unknown OP_CODE %0d; aborting run\n", instruction);
                return INTERPRET_COMPILE_ERROR;
        }
    }
}

InterpretResult interpret(VM* vm, Chunk* chunk) {
    vm->chunk = chunk;
    // yes, it's setting it to an actual pointer
    vm->ip = vm->chunk->code;

    return run(vm);
}