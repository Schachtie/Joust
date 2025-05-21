#pragma once
#include "baseTypes.h"
#include "collision.h"

#ifdef __cplusplus
extern "C" {
#endif


// object "virtual" functions
typedef struct object_t Object;
typedef void (*ObjDeleteFunc)(Object*);
typedef void (*ObjDrawFunc)(Object*);
typedef void (*ObjUpdateFunc)(Object*, uint32_t);
typedef void (*ObjCollideFunc)(Object*, Object*, Collision);


typedef struct object_vtable_t {
    ObjDeleteFunc   delete;
    ObjDrawFunc     draw;
    ObjUpdateFunc   update;
    ObjCollideFunc  collide;
} ObjVtable;

typedef struct object_t {
    ObjVtable*      vtable;

    bool            markedForDelete; // for garbage collection
    bool            enabled;    // for general things that need to be on or not
    bool            collidable; // I would like this to be similar to an interface or something more general than this

    Coord2D         position;
    Coord2D         size;
} Object;


typedef void (*ObjRegistrationFunc)(Object*);


// class-wide registration methods
void objEnableRegistration(ObjRegistrationFunc registerFunc, ObjRegistrationFunc deregisterFunc);
void objDisableRegistration();

// object API
void objInit(Object* obj, ObjVtable* vtable, Coord2D pos, bool isCollidable);
void objDeinit(Object* obj);
void objDraw(Object* obj);
void objUpdate(Object* obj, uint32_t milliseconds);

void objDefaultUpdate(Object* obj, uint32_t milliseconds);

void objEnable(Object* obj);
void objDisable(Object* obj);

bool objIsEnabled(Object* obj);

#ifdef __cplusplus
}
#endif