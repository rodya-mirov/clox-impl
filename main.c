#include "common.h"
#include "chunk.h"
#include "debug.h"

int main(int argc, const char* argv[]) {
    Chunk chunk;

    initChunk(&chunk);

    writeConstant(&chunk, 1.2, 0);
    writeConstant(&chunk, -12.1, 0);

    for (int i = 0; i < 256; i++) {
        writeConstant(&chunk, 135 + i, 0);
    }

    writeChunk(&chunk, OP_RETURN, 1);
    disassambleChunk(&chunk, "test chunk");
    freeChunk(&chunk);

    return 0;
}
