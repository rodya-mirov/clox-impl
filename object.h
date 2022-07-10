#include "common.h"
#include "value.h"

#ifndef clox_object_h
#define clox_object_h

#define OBJ_TYPE(value)     (AS_OBJ(value)->type)

#define IS_STRING(value)    isObjType(value, OBJ_STRING)

#define AS_STRING(value)    ((ObjString*) AS_OBJ(value))
#define AS_CSTRING(value)   (((ObjString*) AS_OBJ(value))->chars)

typedef enum {
    OBJ_STRING
} ObjType;

// general (parent type) of objects. These are allocated on the heap;
// Values which are objects have an Obj* field.
struct Obj {
    ObjType type;
};

// ObjString is an "extension" of Obj
// So we can "cast" an ObjString* to an Obj*
struct ObjString {
    Obj obj;
    int length;
    char* chars;
};

ObjString* copyString(const char* chars, int length);
void printObject(Value value);

static inline bool isObjType(Value value, ObjType objType) {
    return IS_OBJ(value) && AS_OBJ(value)->type == objType;
}

#endif