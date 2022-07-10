#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "compiler.h"
#include "scanner.h"

#ifdef DEBUG_PRINT_CODE
    #include "debug.h"
#endif

typedef struct {
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;
} Parser;

typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT,    // =
    PREC_OR,            // or
    PREC_AND,           // and
    PREC_EQUALITY,      // ==, !=
    PREC_COMPARISON,    // < > <= >=
    PREC_TERM,          // + -
    PREC_FACTOR,        // * /
    PREC_UNARY,         // ! -
    PREC_CALL,          // . ()
    PREC_PRIMARY,
} Precedence;

// literally, a function that takes no arguments and returns nothing
// TODO: probably alter this typedef? since we don't have global state
typedef void (*ParseFn)(Scanner*, Parser*);

static void number(Scanner* scanner, Parser* parser);
static void unary(Scanner* scanner, Parser* parser);
static void grouping(Scanner* scanner, Parser* parser);
static void binary(Scanner* scanner, Parser* parser);
static void literal(Scanner* scanner, Parser* parser);

// each token type has an associated parse rule
typedef struct {
    // this is the parse function triggered if this token is at the beginning of ... whatever
    // if it can't be at the beginning, set this to null (it will blow up, parser error, on purpose)
    ParseFn     prefix;
    // this is the parse function triggered if this token is in the infix position of ... whatever
    // if it can't be in the middle, set this to null (it will blow up, parser error, on purpose)
    ParseFn     infix;
    // precedence of the operator
    Precedence  precedence;
} ParseRule;

ParseRule rules[] = {
    // key is the token; rhs is a struct literal -- prefix parse fn, infix parse fn, precedence
    // null is used when that structure is not relevant
    [TOKEN_LEFT_PAREN]      = { grouping,   NULL,   PREC_NONE   },
    [TOKEN_RIGHT_PAREN]     = { NULL,       NULL,   PREC_NONE   },
    [TOKEN_LEFT_BRACE]      = { NULL,       NULL,   PREC_NONE   },
    [TOKEN_RIGHT_BRACE]     = { NULL,       NULL,   PREC_NONE   },
    [TOKEN_COMMA]           = { NULL,       NULL,   PREC_NONE   },
    [TOKEN_DOT]             = { NULL,       NULL,   PREC_NONE   },
    [TOKEN_MINUS]           = { unary,      binary, PREC_TERM   },
    [TOKEN_PLUS]            = { NULL,       binary, PREC_TERM   },
    [TOKEN_SEMICOLON]       = { NULL,       NULL,   PREC_NONE   },
    [TOKEN_SLASH]           = { NULL,       binary, PREC_FACTOR },
    [TOKEN_STAR]            = { NULL,       binary, PREC_FACTOR },
    // TODO this feels weird, the ! has incredibly low precedence
    [TOKEN_BANG]            = { unary,      NULL,   PREC_NONE   },
    [TOKEN_BANG_EQUAL]      = { NULL,       NULL,   PREC_NONE   },
    [TOKEN_EQUAL]           = { NULL,       NULL,   PREC_NONE   },
    [TOKEN_EQUAL_EQUAL]     = { NULL,       NULL,   PREC_NONE   },
    [TOKEN_GREATER]         = { NULL,       NULL,   PREC_NONE   },
    [TOKEN_GREATER_EQUAL]   = { NULL,       NULL,   PREC_NONE   },
    [TOKEN_LESS]            = { NULL,       NULL,   PREC_NONE   },
    [TOKEN_LESS_EQUAL]      = { NULL,       NULL,   PREC_NONE   },
    [TOKEN_IDENTIFIER]      = { NULL,       NULL,   PREC_NONE   },
    [TOKEN_STRING]          = { NULL,       NULL,   PREC_NONE   },
    [TOKEN_NUMBER]          = { number,     NULL,   PREC_NONE   },
    [TOKEN_AND]             = { NULL,       NULL,   PREC_NONE   },
    [TOKEN_CLASS]           = { NULL,       NULL,   PREC_NONE   },
    [TOKEN_ELSE]            = { NULL,       NULL,   PREC_NONE   },
    [TOKEN_FALSE]           = { literal,    NULL,   PREC_NONE   },
    [TOKEN_FOR]             = { NULL,       NULL,   PREC_NONE   },
    [TOKEN_FUN]             = { NULL,       NULL,   PREC_NONE   },
    [TOKEN_IF]              = { NULL,       NULL,   PREC_NONE   },
    [TOKEN_NIL]             = { literal,    NULL,   PREC_NONE   },
    [TOKEN_OR]              = { NULL,       NULL,   PREC_NONE   },
    [TOKEN_PRINT]           = { NULL,       NULL,   PREC_NONE   },
    [TOKEN_RETURN]          = { NULL,       NULL,   PREC_NONE   },
    [TOKEN_SUPER]           = { NULL,       NULL,   PREC_NONE   },
    [TOKEN_THIS]            = { NULL,       NULL,   PREC_NONE   },
    [TOKEN_TRUE]            = { literal,    NULL,   PREC_NONE   },
    [TOKEN_VAR]             = { NULL,       NULL,   PREC_NONE   },
    [TOKEN_WHILE]           = { NULL,       NULL,   PREC_NONE   },
    [TOKEN_ERROR]           = { NULL,       NULL,   PREC_NONE   },
    [TOKEN_EOF]             = { NULL,       NULL,   PREC_NONE   },
};

static ParseRule* getRule(TokenType type) {
    return &rules[type];
}

// TODO eliminate this global state, I don't understand it enough right now
Chunk* compilingChunk;

static Chunk* currentChunk() {
    return compilingChunk;
}

static void errorAt(Parser* parser, Token* token, const char* message) {
    // this prevents error cascades; report the first one and then just sort of squelch
    // obviously we need to periodically turn this off or you just get one error per compile
    // which might also be bad (??)
    if (parser->panicMode) {
        return;
    }
    parser->panicMode = true;

    fprintf(stderr, "[line %d] Error", token->line);

    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    } else if (token->type == TOKEN_ERROR) {
        // no type to print, it's an error
    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    parser->hadError = true;
}

static void error(Parser* parser, const char* message) {
    errorAt(parser, &parser->previous, message);
}

static void errorAtCurrent(Parser* parser, const char* message) {
    errorAt(parser, &parser->current, message);
}

static void advance(Scanner* scanner, Parser* parser) {
    parser->previous = parser->current;

    // this loop looks weird; it just means we keep looping through tokens
    // and reporting+skipping errors until we get a real one (which might be EOF)
    for (;;) {
        parser->current = scanToken(scanner);

        if (parser->current.type != TOKEN_ERROR) {
            break;
        }

        errorAtCurrent(parser, parser->current.start);
    }
}

// expect a given token type and consume it (advance); if we didn't get it, throw an error
static void consume(Scanner* scanner, Parser* parser, TokenType type, const char* message) {
    if (parser->current.type == type) {
        advance(scanner, parser);
        return;
    }

    errorAtCurrent(parser, message);
}

static void emitByte(int lineNumber, uint8_t byte) {
    writeChunk(currentChunk(), byte, lineNumber);
}

static void emitBytes(int lineNumber, uint8_t b1, uint8_t b2) {
    emitByte(lineNumber, b1);
    emitByte(lineNumber, b2);
}

static void emitReturn(int lineNumber) {
    emitByte(lineNumber, OP_RETURN);
}

static void endCompiler(Parser* parser) {
    int lineNumber = parser->previous.line;
    emitReturn(lineNumber);

#ifdef DEBUG_PRINT_CODE
    if (!parser->hadError) {
        disassambleChunk(currentChunk(), "code");
    } else {
        printf("Skipping chunk dump, since a parser error was found.\n");
    }
#endif
}

// parse things at or above the given precedence
static void parsePrecedence(Scanner* scanner, Parser* parser, Precedence precedence) {
    advance(scanner, parser);
    ParseFn prefixRule = getRule(parser->previous.type)->prefix;
    if (prefixRule == NULL) {
        error(parser, "Expect expression.");
        return;
    }

    prefixRule(scanner, parser);

    while (precedence <= getRule(parser->current.type)->precedence) {
        advance(scanner, parser);
        ParseFn infixRule = getRule(parser->previous.type)->infix;
        infixRule(scanner, parser);
    }
}

static void expression(Scanner* scanner, Parser* parser) {
    parsePrecedence(scanner, parser, PREC_ASSIGNMENT);
}

// consume and output the rest of a parenthetized expression, given the start paren has been consumed
static void grouping(Scanner* scanner, Parser* parser) {
    expression(scanner, parser);
    consume(scanner, parser, TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

// emit a constant to the chunk. Only uses parser for line number
static void emitConstant(int line, Value value) {
    writeConstant(currentChunk(), value, line);
}

static void number(Scanner* scanner, Parser* parser) {
    double value = strtod(parser->previous.start, NULL);
    emitConstant(parser->previous.line, NUMBER_VAL(value));
}

static void unary(Scanner* scanner, Parser* parser) {
    TokenType operatorType = parser->previous.type;

    parsePrecedence(scanner, parser, PREC_UNARY);

    switch (operatorType) {
        case TOKEN_BANG:
            emitByte(parser->previous.line, OP_NOT);
            break;

        case TOKEN_MINUS:
            emitByte(parser->previous.line, OP_NEGATE);
            break;
        
        default:
            // unreachable
            // TODO surely this is not the best self-defense we can manage???
            return;
    }
}

static void literal(Scanner* scanner, Parser* parser) {
    #define EMIT_BYTE(b)    { int lineNumber = parser->previous.line; emitByte(lineNumber, b); break; }
    switch (parser->previous.type) {
        case TOKEN_FALSE:   EMIT_BYTE(OP_FALSE)
        case TOKEN_TRUE:    EMIT_BYTE(OP_TRUE)
        case TOKEN_NIL:     EMIT_BYTE(OP_NIL)
        default:
            // unreachable?
            return;
    }
}

// parse+consume a binary infix expression
// called after the first operand has been consumed and the operator is in parser->previous
static void binary(Scanner* scanner, Parser* parser) {
    TokenType operatorType = parser->previous.type;
    ParseRule* rule = getRule(operatorType);

    // parse + consume the second operand, based on the precedence of the operand itself
    // note the right hand operation is 1 level higher than the left; this ensure that
    // 1+2+3 is parsed as (1+2)+3
    // aka left associativity
    parsePrecedence(scanner, parser, (Precedence) (rule->precedence + 1));

    // then emit the operand's OP_CODE itself; we use a macro to condense the switch block
    int lineNumber = parser->previous.line;

    #define EMIT_OP(TOK, TOK_OP) case TOK: { emitByte(lineNumber, TOK_OP); break; }

    switch (operatorType) {
        EMIT_OP(TOKEN_PLUS, OP_ADD)
        EMIT_OP(TOKEN_MINUS, OP_SUBTRACT)
        EMIT_OP(TOKEN_STAR, OP_MULTIPLY)
        EMIT_OP(TOKEN_SLASH, OP_DIVIDE)

        default: {
            // unreachable?
            return;
        }
    }

    #undef EMIT_OP
}

bool compile(const char* source, Chunk* chunk) {
    Scanner scanner;
    initScanner(&scanner, source);

    // TODO: i truly do not understand the state management in this book around all these globals
    compilingChunk = chunk;

    Parser parser;

    parser.panicMode = false;
    parser.hadError = false;
    
    advance(&scanner, &parser);
    expression(&scanner, &parser);

    consume(&scanner, &parser, TOKEN_EOF, "Expect end of expression.");

    endCompiler(&parser);

    return !parser.hadError;
}