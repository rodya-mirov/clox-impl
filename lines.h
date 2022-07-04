#ifndef clox_lines_h
#define clox_lines_h

#include "common.h"

typedef struct {
    // line number
    int lineIdx;
    // code index in the chunk
    int codeIdx;
} LineRecord;

typedef struct {
    int capacity;
    int count;
    LineRecord* records;
} LineRecordArray;

void initLinesArray(LineRecordArray* array);
void writeLinesArray(LineRecordArray* array, int lineIdx, int codeIdx);
void freeLinesArray(LineRecordArray* array);

// returns the lineIdx associated to a given codeIdx
// returns -1 if there is none such.
int getLine(LineRecordArray* array, int codeIdx);

#endif