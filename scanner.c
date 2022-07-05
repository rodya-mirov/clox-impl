#include <stdio.h>
#include <string.h>

#include "common.h"
#include "scanner.h"

void initScanner(Scanner* scanner, const char* source) {
    scanner->start = source;
    scanner->current = source;
    // should we start at 1 or 0? idk
    scanner->line = 1;
}

static bool isAtEnd(Scanner* scanner) {
    return *scanner->current == '\0';
}

static Token makeToken(Scanner* scanner, TokenType type) {
    Token token;
    token.type = type;
    token.start = scanner->start;
    token.length = (int)(scanner->current - scanner->start);
    token.line = scanner->line;
    return token;
}

static Token errorToken(Scanner* scanner, const char* message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)(strlen(message));
    token.line = scanner->line;
    return token;
}

static char advance(Scanner* scanner) {
    scanner->current += 1;
    return scanner->current[-1];
}

// if the next character exists and matches the given, consume it and return true
// otherwise return false
static bool match(Scanner* scanner, char expected) {
    if (isAtEnd(scanner)) {
        return false;
    }
    if (*scanner->current != expected) {
        return false;
    }
    scanner->current += 1;
    return true;
}

static char peek(Scanner* scanner) {
    return *scanner->current;
}

static char peekNext(Scanner* scanner) {
    if (isAtEnd(scanner)) {
        return '\0';
    } else {
        return scanner->current[1];
    }
}

// skips whitespace
// skips comments too, whatever
static void skipWhitespace(Scanner* scanner) {
    for (;;) {
        char c = peek(scanner);
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance(scanner);
                break;
            case '\n':
                scanner->line += 1;
                advance(scanner);
                break;
            case '/':
                if (peekNext(scanner) == '/') {
                    while (peek(scanner) != '\n' && !isAtEnd(scanner)) {
                        advance(scanner);
                    }
                } else {
                    // / wasn't whitespace, bail
                    return;
                }
                break;
            default:
                // wasn't whitespace, bail
                return;
        }
    }
}

// consumes and returns a string lexeme
// PRE: the starting " has been consumed
static Token string(Scanner* scanner) {
    // TODO: what about escaped quotes?
    while (peek(scanner) != '"' && !isAtEnd(scanner)) {
        if (peek(scanner) == '\n') {
            scanner->line += 1;
        }
        advance(scanner);
    }

    if (isAtEnd(scanner)) {
        return errorToken(scanner, "Unterminated string.");
    }

    advance(scanner);
    return makeToken(scanner, TOKEN_STRING);
}

static bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

static Token number(Scanner* scanner) {
    while (isDigit(peek(scanner))) {
        advance(scanner);
    }

    if (peek(scanner) == '.' && isDigit(peekNext(scanner))) {
        advance(scanner); // consume the dot

        // then consume the decimal parts
        while (isDigit(peek(scanner))) {
            advance(scanner);
        }
    }

    return makeToken(scanner, TOKEN_NUMBER);
}

Token scanToken(Scanner* scanner) {
    skipWhitespace(scanner);

    // this sets the token start to the current next position
    // note this means whitespace (and comments) are totally dropped
    scanner->start = scanner->current;

    if (isAtEnd(scanner)) {
        return makeToken(scanner, TOKEN_EOF);
    }

    #define ONE_CHAR_TOKEN(c, kind) case c: { return makeToken(scanner, kind); }

    #define ONE_CHAR_FB_TOKEN(c, maybeC2, kindA, kindB) \
        case c: { \
            if (match(scanner, maybeC2)) { \
                return makeToken(scanner, kindA); \
            } else { \
                return makeToken(scanner, kindB); \
            } \
        }

    char c = advance(scanner);

    if (isDigit(c)) {
        return number(scanner);
    }

    switch(c) {
        ONE_CHAR_TOKEN('(', TOKEN_LEFT_PAREN);
        ONE_CHAR_TOKEN(')', TOKEN_RIGHT_PAREN);
        ONE_CHAR_TOKEN('{', TOKEN_LEFT_BRACE);
        ONE_CHAR_TOKEN('}', TOKEN_RIGHT_BRACE);
        ONE_CHAR_TOKEN(';', TOKEN_SEMICOLON);
        ONE_CHAR_TOKEN(',', TOKEN_COMMA);
        ONE_CHAR_TOKEN('.', TOKEN_DOT);
        ONE_CHAR_TOKEN('-', TOKEN_MINUS);
        ONE_CHAR_TOKEN('+', TOKEN_PLUS);
        ONE_CHAR_TOKEN('/', TOKEN_SLASH);
        ONE_CHAR_TOKEN('*', TOKEN_STAR);

        ONE_CHAR_FB_TOKEN('!', '=', TOKEN_BANG_EQUAL, TOKEN_BANG);
        ONE_CHAR_FB_TOKEN('=', '=', TOKEN_EQUAL_EQUAL, TOKEN_EQUAL);
        ONE_CHAR_FB_TOKEN('<', '=', TOKEN_LESS_EQUAL, TOKEN_LESS);
        ONE_CHAR_FB_TOKEN('>', '=', TOKEN_GREATER_EQUAL, TOKEN_GREATER);
    
        case '"': return string(scanner);
    }

    // lexer errors are great
    return errorToken(scanner, "Unexpected character.");

    #undef ONE_CHAR_TOKEN
}