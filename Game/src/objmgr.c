#include <Windows.h>
#include <stdlib.h>
#include <assert.h>

#include "objmgr.h"
#include "baseTypes.h"
#include "collisionMgr.h"


static struct objmgr_t {
    Object** list;
    uint32_t max;
    uint32_t count;
} _objMgr = { NULL, 0, 0 };


static bool _isFirstUpdate = true;


/// @brief Initialize the object manager
/// @param maxObjects 
void objMgrInit(uint32_t maxObjects)
{
    // allocate the required space
    _objMgr.list = malloc(maxObjects * sizeof(Object*));
    if (_objMgr.list != NULL) {
        // initialize as empty
        ZeroMemory(_objMgr.list, maxObjects * sizeof(Object*));
        _objMgr.max = maxObjects;
        _objMgr.count = 0;
    }

    // setup registration, so all initialized objects are logged w/ the manager
    objEnableRegistration(objMgrAdd, objMgrRemove);
}

/// @brief Shutdown the object manager
void objMgrShutdown()
{
    // disable registration, since the object manager is shutting down
    objDisableRegistration();

    // this isn't strictly required, but want to enforce proper cleanup
    assert(_objMgr.count == 0);

    // objMgr doesn't own the objects, so just clean up self
    free(_objMgr.list);
    _objMgr.list = NULL;
    _objMgr.max = _objMgr.count = 0;
}


/// @brief Add an object to be tracked by the manager, and to the collision manager if necessary.
/// @param obj 
void objMgrAdd(Object* obj)
{
    for (uint32_t i = 0; i < _objMgr.max; ++i)
    {
        if (_objMgr.list[i] == NULL)
        {
            _objMgr.list[i] = obj;
            ++_objMgr.count;

            // Add to collision manager if necessary
            if (obj->collidable) { collisionMgrAdd((Entity*)obj); }

            return;
        }
    }

    // out of space to add object!
    assert(false);
}

/// @brief Remove an object from the manager's tracking, and from collision manager if necessary.
/// @param obj 
void objMgrRemove(Object* obj)
{
    for (uint32_t i = 0; i < _objMgr.max; ++i)
    {
        if (obj == _objMgr.list[i])
        {
            // Remove from collision manager if necessary (CONSIDER THE LOCATION OF THIS)
            if (obj->collidable) { collisionMgrRemove((Entity*)obj); }

            // no need to free memory, so just clear the reference
            _objMgr.list[i] = NULL;
            --_objMgr.count;

            return;
        }
    }

    // could not find object to remove!
    assert(false);
}


/// @brief Draws all registered objects
void objMgrDraw() 
{
    for (uint32_t i = 0; i < _objMgr.max; ++i)
    {
        Object* obj = _objMgr.list[i];
        if (obj != NULL && obj->enabled)
        {
            // TODO - consider draw order?
            objDraw(obj);
        }
    }
}

/// @brief Updates all registered objects, and handles their collisions if necessary.
/// @param milliseconds 
void objMgrUpdate(uint32_t milliseconds)
{
    if (_isFirstUpdate) { milliseconds = 0; _isFirstUpdate = false; }

    for (uint32_t i = 0; i < _objMgr.max; ++i)
    {
        Object* obj = _objMgr.list[i];
        if (obj != NULL && obj->enabled)
        {
            objUpdate(obj, milliseconds);
            if (obj->collidable) { handleAllCollisions((Entity*)obj); } // I want to abstract this more, I don't like that this is being called inside object manager.
        }
    }
}
