#include "common.h"
#include "chunk.h"
#include "debug.h"

int main(int argc, const char* argv[]) {
    Chunk chunk;

    initChunk(&chunk);

    int constantIdx = addConstant(&chunk, 1.2);
    writeChunk(&chunk, OP_CONSTANT);
    writeChunk(&chunk, constantIdx);

    int constantIdx2 = addConstant(&chunk, -12.1);
    writeChunk(&chunk, OP_CONSTANT);
    writeChunk(&chunk, constantIdx2);

    writeChunk(&chunk, OP_RETURN);
    disassambleChunk(&chunk, "test chunk");
    freeChunk(&chunk);

    return 0;
}
