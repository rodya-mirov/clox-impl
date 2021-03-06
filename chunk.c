#include <stdlib.h>

#include "chunk.h"
#include "memory.h"
#include "lines.h"

void initChunk(Chunk* chunk) {
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;
    initLinesArray(&chunk->lines);
    initValueArray(&chunk->constants);
}

// Write a byte into the chunk
void writeChunk(Chunk* chunk, uint8_t byte, int line) {
    writeLinesArray(&chunk->lines, line, chunk->count);

    if (chunk->capacity < chunk->count + 1) {
        int oldCapacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(oldCapacity);
        chunk->code = GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
    }

    chunk->code[chunk->count] = byte;
    chunk->count ++;
}

void freeChunk(Chunk* chunk) {
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
    freeLinesArray(&chunk->lines);
    freeValueArray(&chunk->constants);
    initChunk(chunk);
}

void writeConstant(Chunk* chunk, Value value, int line) {
    writeValueArray(&chunk->constants, value);
    int valueIdx = chunk->constants.count - 1;
    if (valueIdx < (1 << 8)) {
        writeChunk(chunk, OP_CONSTANT, line);
        writeChunk(chunk, valueIdx & (0xFF), line);
    } else if (valueIdx < (1 << 24)) {
        writeChunk(chunk, OP_CONSTANT_LONG, line);
        writeChunk(chunk, (valueIdx >> 16) & (0xFF), line);
        writeChunk(chunk, (valueIdx >> 8) & (0xFF), line);
        writeChunk(chunk, (valueIdx) & (0xFF), line);
    }
}