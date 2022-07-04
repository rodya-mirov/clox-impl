#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"

int main(int argc, const char* argv[]) {
    VM vm;

    initVM(&vm);

    Chunk chunk;

    initChunk(&chunk);
    // ((50 + (-1.2)) - 1.2) / 2 * 6.2
    writeConstant(&chunk, 50, 0); // 50
    writeConstant(&chunk, 1.2, 0); // 50, 1.2
    writeChunk(&chunk, OP_NEGATE, 0); // 50, -1.2
    writeChunk(&chunk, OP_ADD, 0); // 48.8
    writeConstant(&chunk, 1.2, 0); // 48.8, 1.2
    writeChunk(&chunk, OP_SUBTRACT, 0); // 46.6
    writeConstant(&chunk, 2, 0); // 46.6, 2
    writeChunk(&chunk, OP_DIVIDE, 0); // 23.3
    writeConstant(&chunk, 6, 0); // 23.3, 6
    writeChunk(&chunk, OP_MULTIPLY, 0); // 139.8


    writeChunk(&chunk, OP_RETURN, 0);
    // disassambleChunk(&chunk, "test chunk");
    interpret(&vm, &chunk);

    freeChunk(&chunk);

    freeVM(&vm);

    return 0;
}
