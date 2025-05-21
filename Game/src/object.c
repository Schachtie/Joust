#include "baseTypes.h"
#include "object.h"


static ObjRegistrationFunc _registerFunc = NULL;
static ObjRegistrationFunc _deregisterFunc = NULL;


/// @brief Enable callback to a registrar on ObjInit/Deinit
/// @param registerFunc 
/// @param deregisterFunc 
void objEnableRegistration(ObjRegistrationFunc registerFunc, ObjRegistrationFunc deregisterFunc)
{
    _registerFunc = registerFunc;
    _deregisterFunc = deregisterFunc;
}

/// @brief Disable registration during ObjInit/Deinit
void objDisableRegistration()
{
    _registerFunc = _deregisterFunc = NULL;
}


/// @brief Initialize an object. Intended to be called from subclass constructors
/// @param obj 
/// @param vtable 
/// @param pos 
/// @param vel 
void objInit(Object* obj, ObjVtable* vtable, Coord2D pos, bool isCollidable)
{
    obj->vtable = vtable;
    obj->markedForDelete = false;
    obj->enabled = true;
    obj->collidable = isCollidable; //must have a param here for this until a better registering is available/created;
    obj->position = pos;
    obj->size.x = 0;
    obj->size.y = 0;

    if (_registerFunc != NULL)
    {
        _registerFunc(obj);
    }
}

/// @brief Deinitialize an object
/// @param obj 
void objDeinit(Object* obj)
{
    if (_deregisterFunc != NULL)
    {
        _deregisterFunc(obj);
    }
}


/// @brief Draw this object, using it's vtable
/// @param obj 
void objDraw(Object* obj)
{
    if (obj->vtable != NULL && obj->vtable->draw != NULL) 
    {
        obj->vtable->draw(obj);
    }
}

/// @brief Update this object, using it's vtable
/// @param obj 
/// @param milliseconds 
void objUpdate(Object* obj, uint32_t milliseconds)
{
    if (obj->enabled)
    {
        if (obj->vtable != NULL && obj->vtable->update != NULL)
        {
            obj->vtable->update(obj, milliseconds);
            return;
        }
        objDefaultUpdate(obj, milliseconds);
    }
}

/// @brief For object cleanup, deletes the object if it is marked for deletion.
/// @param obj 
/// @param milliseconds 
void objDefaultUpdate(Object* obj, uint32_t milliseconds)
{
    if (obj->markedForDelete) { obj->vtable->delete(obj); } 
}


/// <summary>
/// Enables the passed in object.
/// </summary>
/// <param name="obj"></param>
void objEnable(Object* obj)
{
    obj->enabled = true;
}

/// <summary>
/// Disables the passed in object.
/// </summary>
/// <param name="obj"></param>
void objDisable(Object* obj)
{
    obj->enabled = false;
}


/// <param name="obj"></param>
/// <returns>Whether the object is enabled or not.</returns>
bool objIsEnabled(Object* obj)
{
    return obj->enabled;
}
