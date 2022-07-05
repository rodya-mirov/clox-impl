#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"

static void repl(VM* vm) {
    // hardcoded line length limit
    // it really doesn't matter because this is just a tutorial and i don't intend to do the "real thing"
    // in C so whatever, but fwiw, this is gross
    char line[1024];

    for (;;) {
        printf("> ");

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }

        interpret(vm, line);
    }
}

static char* readFile(const char* path) {
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(74);
    }

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char* buffer = (char*) malloc(fileSize+1);
    if (buffer == NULL) {
        fprintf(stderr, "Not enough memory to read \"%s\" (%lu bytes).\n", path, fileSize+1);
        exit(74);
    }

    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if (bytesRead != fileSize) {
        fprintf(stderr, "Could not read all of file \"%s\" (%lu bytes of expected %lu).\n", path, bytesRead, fileSize);
    }
    buffer[bytesRead] = '\0';

    fclose(file);
    return buffer;
}

static void runFile(VM* vm, const char* path) {
    char* source = readFile(path);
    InterpretResult result = interpret(vm, source);
    free(source);

    switch (result) {
        case INTERPRET_OK: exit(0);
        case INTERPRET_COMPILE_ERROR: exit(65);
        case INTERPRET_RUNTIME_ERROR: exit(70);
    }
}

int main(int argc, const char* argv[]) {
    VM vm;

    initVM(&vm);

    if (argc == 1) {
        repl(&vm);
    } else if (argc == 2) {
        runFile(&vm, argv[1]);
    } else {
        fprintf(stderr, "Usage: clox [path]\n");
        exit(64);
    }

    freeVM(&vm);
    return 0;
}

/* // TODO: delete all this when we're sure the new implementation is gtg
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
*/
