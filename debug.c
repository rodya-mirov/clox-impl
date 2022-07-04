#include <stdio.h>

#include "debug.h"
#include "value.h"
#include "lines.h"

static int simpleInstruction(const char* name, int offset) {
    printf("%s\n", name);
    return offset + 1;
}

void disassambleChunk(Chunk* chunk, const char* name) {
    printf("== %s ==\n", name);
    for (int offset = 0; offset < chunk->count;) {
        offset = disassembleInstruction(chunk, offset);
    }
}

static int constantInstruction(const char* name, Chunk* chunk, int offset) {
    uint8_t constant = chunk->code[offset+1];
    printf("%-16s %04d '", name, constant);
    printValue(chunk->constants.values[constant]);
    printf("'\n");
    return offset+2;
}

static int constantInstructionLong(const char* name, Chunk* chunk, int offset) {
    uint8_t cIdx1 = chunk->code[offset+1];
    uint8_t cIdx2 = chunk->code[offset+2];
    uint8_t cIdx3 = chunk->code[offset+3];

    int constant = ((int) (cIdx1 << 16)) + ((int) (cIdx2 << 8)) + ((int) cIdx3);
    printf("%-16s %04d '", name, constant);
    printValue(chunk->constants.values[constant]);
    printf("'\n");
    return offset+4;
}

static void printLineNumber(Chunk* chunk, int offset) {
    // TODO: using getLine is fine for random access but this is really bad
    // for iteration performance. Better to use a running counter or something.
    if (offset == 0) {
        int lineIdx = getLine(&chunk->lines, offset);
        printf("%04d ",lineIdx);
        return;
    }

    int thisLine = getLine(&chunk->lines, offset);
    int lastLine = getLine(&chunk->lines, offset-1);

    if (thisLine == lastLine) {
        printf("   | ");
    } else {
        printf("%04d ", thisLine);
    }
}

int disassembleInstruction(Chunk* chunk, int offset) {
    printf("%04d ", offset);

    printLineNumber(chunk, offset);

    uint8_t instruction = chunk->code[offset];
    switch (instruction) {
        case OP_CONSTANT:
            return constantInstruction("OP_CONSTANT", chunk, offset);

        case OP_CONSTANT_LONG:
            return constantInstructionLong("OP_CONSTANT_LONG", chunk, offset);

        case OP_RETURN:
            return simpleInstruction("OP_RETURN", offset);
        
        default:
            printf("Unknown opcode %d\n", instruction);
            return offset + 1;
    }
}