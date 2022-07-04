#include <stdio.h>

#include "common.h"
#include "memory.h"
#include "lines.h"

void initLinesArray(LineRecordArray* array) {
    array->records = NULL;
    array->capacity = 0;
    array->count = 0;
}

void writeLinesArray(LineRecordArray* array, int lineIdx, int codeIdx) {
    if (array -> capacity < array->count + 1) {
        int oldCapacity = array->capacity;
        array->capacity = GROW_CAPACITY(oldCapacity);
        array->records = GROW_ARRAY(LineRecord, array->records, oldCapacity, array->capacity);
    }
    LineRecord lr;
    lr.lineIdx = lineIdx;
    lr.codeIdx = codeIdx;
    array->records[array->count] = lr;
    array->count += 1;
}

void freeLinesArray(LineRecordArray* array) {
    FREE_ARRAY(LineRecord, array->records, array->capacity);
    initLinesArray(array);
}

// TODO perf -- this should at least be a binary search, this is an embarassment
int getLine(LineRecordArray* array, int codeIdx) {
    for (int i = 0; i < array->count; i++) {
        LineRecord record = array->records[i];
        if (record.codeIdx == codeIdx) {
            return record.lineIdx;
        }
    }
    return -1;
}
