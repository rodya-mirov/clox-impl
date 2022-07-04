#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"

int main(int argc, const char* argv[]) {
    VM vm;

    initVM(&vm);

    Chunk chunk;

    initChunk(&chunk);

    writeConstant(&chunk, 1.2, 0);
    writeConstant(&chunk, -12.1, 0);

    for (int i = 0; i < 256; i++) {
        writeConstant(&chunk, 135 + i, 0);
    }

    writeChunk(&chunk, OP_RETURN, 1);
    // disassambleChunk(&chunk, "test chunk");
    interpret(&vm, &chunk);

    freeChunk(&chunk);

    freeVM(&vm);

    return 0;
}
