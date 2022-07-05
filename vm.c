#include <stdio.h>

#include "common.h"
#include "vm.h"
#include "debug.h"
#include "compiler.h"

static void resetStack(VM* vm) {
    vm->stackTop = vm->stack;
}

void initVM(VM* vm) {
    resetStack(vm);
}

void freeVM(VM* vm) {

}

// turn three bytes into an integer. helper for readConstantLong
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

    #define UNARY_OP(op) \
        do { \
            Value* aPtr = vm->stackTop - 1; \
            *aPtr = op (*aPtr); \
        } while(false)

    #define BINARY_OP(op) \
        do { \
            double b = pop(vm); \
            Value* aPtr = vm->stackTop - 1; \
            *aPtr = (*aPtr) op (b); \
        } while(false)

    for(;;) {
        #ifdef DEBUG_TRACE_EXECUTION
            #ifdef DEBUG_TRACE_EXECUTION_PRINT_STACK
                printf("        Stack (depth %ld): ", (vm->stackTop - vm->stack));
                for (Value* slot = vm->stack; slot < vm->stackTop; slot++) {
                    printf("[ ");
                    Value value = *slot;
                    printValue(value);
                    printf(" ]");
                }
                printf("\n");
            #endif

            disassembleInstruction(vm->chunk, (int)(vm->ip - vm->chunk->code));
        #endif

        uint8_t instruction;
        switch (instruction = read_byte(vm)) {
            case OP_CONSTANT: {
                Value constant = readConstant(vm);
                push(vm, constant);
                break;
            }

            case OP_CONSTANT_LONG: {
                Value constant = readConstantLong(vm);
                push(vm, constant);
                break;
            }

            case OP_RETURN: {
                Value val = pop(vm);
                printValue(val);
                printf("\n");
                return INTERPRET_OK;
            }

            case OP_NEGATE:         UNARY_OP(-);  break;

            case OP_ADD:            BINARY_OP(+); break;
            case OP_SUBTRACT:       BINARY_OP(-); break;
            case OP_MULTIPLY:       BINARY_OP(*); break;
            case OP_DIVIDE:         BINARY_OP(/); break;

            default:
                printf("Unknown OP_CODE %0d; aborting run\n", instruction);
                return INTERPRET_COMPILE_ERROR;
        }
    }

    #undef BINARY_OP
    #undef UNARY_OP
}

InterpretResult interpret(VM* vm, const char* source) {
    compile(source);
    return INTERPRET_OK;
}

// TODO: what about stack overflow?
void push(VM* vm, Value value) {
    if (vm->stackTop >= vm->stack + STACK_MAX) {
        fprintf(stderr, "Stack overflow -- max %d", STACK_MAX);
        exit(1);
    }
    *vm->stackTop = value;
    vm->stackTop += 1;
}

// TODO: what about stack underflow?
Value pop(VM* vm) {
    vm->stackTop -= 1;
    return *vm->stackTop;
}