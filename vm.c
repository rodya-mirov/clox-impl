#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "vm.h"
#include "debug.h"
#include "compiler.h"
#include "memory.h"

static void resetStack(VM* vm) {
    vm->stackTop = vm->stack;
}

static Value peek(VM* vm, int distance);

// takes vm, format string, and format args
// prints the formatted args, then some debug information about where the error occurred
static void runtimeError(VM* vm, const char* format, ...);

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

static bool isFalsey(Value v) {
    if (IS_NIL(v)) {
        return true;
    }
    if (IS_BOOL(v) && !AS_BOOL(v)) {
        return true;
    }
    return false;
}

static void concatenate(VM* vm) {
    ObjString* b = AS_STRING(pop(vm));
    ObjString* a = AS_STRING(pop(vm));

    int length = a->length + b->length;
    char* chars = ALLOCATE(char, length+1);
    memcpy(chars, a->chars, a->length);
    memcpy(chars + a->length, b->chars, b->length);
    chars[length] = '\0';

    ObjString* result = takeString(chars, length);
    push(vm, OBJ_VAL(result));
}

static InterpretResult run(VM* vm) {

    // an efficient helper function for taking a numerical argument and
    // applying a unary function to it
    #define UNARY_OP(valueType, op) \
        do { \
            if (!IS_NUMBER(peek(vm, 0))) { \
                runtimeError(vm, "Operand must be a number."); \
                return INTERPRET_RUNTIME_ERROR; \
            } \
            Value* aPtr = vm->stackTop - 1; \
            double aNum = AS_NUMBER(*aPtr); \
            *aPtr = valueType(op (aNum)); \
        } while(false)

    // an efficient helper function for taking two numerical arguments
    // and applying a binary function to them
    #define BINARY_OP(valueType, op) \
        do { \
            if (!IS_NUMBER(peek(vm, 0)) || !IS_NUMBER(peek(vm, 1))) { \
                runtimeError(vm, "Operands must be numbers."); \
                return INTERPRET_RUNTIME_ERROR; \
            } \
            double b = AS_NUMBER(pop(vm)); \
            Value* aPtr = vm->stackTop - 1; \
            double aVal = AS_NUMBER(*aPtr); \
            *aPtr = valueType((aVal) op (b)); \
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

            case OP_NIL:            push(vm, NIL_VAL);          break;
            case OP_TRUE:           push(vm, BOOL_VAL(true));   break;
            case OP_FALSE:          push(vm, BOOL_VAL(false));  break;

            case OP_EQUAL: {
                Value b = pop(vm);
                Value* aPtr = vm->stackTop - 1;
                *aPtr = BOOL_VAL(valuesEqual(*aPtr, b));
                break;
            }

            case OP_NOT_EQUAL: {
                Value b = pop(vm);
                Value* aPtr = vm->stackTop - 1;
                *aPtr = BOOL_VAL(!valuesEqual(*aPtr, b));
                break;
            }

            case OP_GREATER:        BINARY_OP(BOOL_VAL, >); break;
            case OP_GREATER_EQUAL:  BINARY_OP(BOOL_VAL, >=); break;
            case OP_LESS:           BINARY_OP(BOOL_VAL, <); break;
            case OP_LESS_EQUAL:     BINARY_OP(BOOL_VAL, <=); break;

            case OP_NEGATE:         UNARY_OP(NUMBER_VAL, -);  break;
            case OP_NOT: {
                Value* aPtr = vm->stackTop - 1;
                *aPtr = BOOL_VAL(isFalsey(*aPtr));
                break;
            }

            case OP_ADD: {
                Value b = peek(vm, 0);
                Value a = peek(vm, 1);

                if (IS_STRING(a) && IS_STRING(b)) {
                    concatenate(vm);
                } else if (IS_NUMBER(a) && IS_NUMBER(b)) {
                    BINARY_OP(NUMBER_VAL, +);
                } else {
                    runtimeError(vm, "Operands must be two strings or two numbers");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_SUBTRACT:       BINARY_OP(NUMBER_VAL, -); break;
            case OP_MULTIPLY:       BINARY_OP(NUMBER_VAL, *); break;
            case OP_DIVIDE:         BINARY_OP(NUMBER_VAL, /); break;

            default:
                printf("Unknown OP_CODE %0d; aborting run\n", instruction);
                return INTERPRET_COMPILE_ERROR;
        }
    }

    #undef BINARY_OP
    #undef UNARY_OP
}

InterpretResult interpret(VM* vm, const char* source) {
    Chunk chunk;
    initChunk(&chunk);

    if (!compile(source, &chunk)) {
        freeChunk(&chunk);
        return INTERPRET_COMPILE_ERROR;
    }

    vm->chunk = &chunk;
    vm->ip = vm->chunk->code;

    InterpretResult result = run(vm);

    freeChunk(&chunk);

    return result;
}

void push(VM* vm, Value value) {
    if (vm->stackTop >= vm->stack + STACK_MAX) {
        fprintf(stderr, "Stack overflow -- max %d", STACK_MAX);
        exit(1);
    }
    *vm->stackTop = value;
    vm->stackTop += 1;
}

// TODO: what about stack underflow? shouldn't happen except in a compiler bug
// so idk how much i care
Value pop(VM* vm) {
    vm->stackTop -= 1;
    return *vm->stackTop;
}

static Value peek(VM* vm, int distance) {
    return vm->stackTop[-1 - distance];
}

static void runtimeError(VM* vm, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    size_t instruction = vm->ip - vm->chunk->code - 1;
    LineRecordArray* lines = &vm->chunk->lines;
    int line = getLine(lines, instruction);
    fprintf(stderr, "[line %d] in script\n", line);
    resetStack(vm);
}