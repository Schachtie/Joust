#include <Windows.h>
#include <assert.h>
#include <math.h>

#include "collisionMgr.h"
#include "collision.h"


static struct collmgr_t {
	Entity**	list;
	uint32_t	max;
	uint32_t	count;
} _collMgr = { NULL, 0, 0 };


// Function Prototypes
static Collision _detectCollision(const Entity* const thisEntity, const Entity* const otherEntity);


/// <summary>
/// Initializes the collision manager with empty values.
/// </summary>
/// <param name="maxObjects"></param>
void collisionMgrInit(uint32_t maxObjects)
{
	// Allocate space for the list
	_collMgr.list = (Entity**)malloc(maxObjects * sizeof(Entity*));
	if (_collMgr.list != NULL)
	{
		// Initialize list as empty
		ZeroMemory(_collMgr.list, maxObjects * sizeof(Entity*));
		_collMgr.max = maxObjects;
		_collMgr.count = 0;
	}
}

/// <summary>
/// Ensures the collision manager is empty, and sets all references to NULL.
/// </summary>
void collisionMgrShutdown()
{
	// Ensure that the collision list has been totally emptied
	assert(_collMgr.count == 0);

	// This manager does not own the objects, so only clean itself up
	free(_collMgr.list);
	_collMgr.list = NULL;
	_collMgr.max = _collMgr.count = 0;
}


/// <summary>
/// Adds an entity to the collision manager if there is an open space.
/// </summary>
/// <param name="entity"></param>
void collisionMgrAdd(Entity* entity)
{
	for (uint32_t i = 0; i < _collMgr.max; ++i)
	{
		if (_collMgr.list[i] == NULL)
		{
			_collMgr.list[i] = entity;
			++_collMgr.count;
			return;
		}
	}
}

/// <summary>
/// Removes the entity from the collision manager.
///		<para>
/// Note: Does not delete/free the entity.
///		</para>
/// </summary>
/// <param name="entity"></param>
void collisionMgrRemove(Entity* entity)
{
	// Check all of them just to be safe
	for (uint32_t i = 0; i < _collMgr.max; ++i)
	{
		if (entity == _collMgr.list[i])
		{
			// Just clear the reference
			_collMgr.list[i] = NULL;
			--_collMgr.count;
			return;
		}
	}
}


/// <summary>
/// Checks for and handles all collisions occurring on the passed in entity.
/// </summary>
/// <param name="entity"></param>
void handleAllCollisions(Entity* entity)
{
	// Check if the object is "awake" or not
		// Loop through all objects in the manager and check for a collision
		// If collision occurs, react appropriately
		// Continue looping to check for more collisions

	if (entity->awake)
	{
		for (uint32_t i = 0; i < _collMgr.max && _collMgr.list[i] != NULL; ++i)
		{
			Entity* otherEntity = _collMgr.list[i];

			// Make sure we don't detect collisions with ourself
			if (otherEntity->obj.enabled && entity != otherEntity)
			{
				// Check for collision with this object (just do AABB)
				Collision collDetResult = _detectCollision(entity, _collMgr.list[i]);

				if (collDetResult.isColliding)
				{
					// This needs to react appropriately now
					// Just jump directly to "this" object's collide function (pass otherObject + Collision)
					entity->obj.vtable->collide((Object*)entity, (Object*)_collMgr.list[i], collDetResult);
				}
			}
		}
	}
}


/// <summary>
/// Detects whether a collision is occurring between the two passed in entities.
/// </summary>
/// <param name="thisEntity"></param>
/// <param name="otherEntity"></param>
/// <returns>Information relating to the collision that may have occurred.</returns>
Collision _detectCollision(const Entity* const thisEntity, const Entity* const otherEntity)
{
	assert(thisEntity != NULL && otherEntity != NULL);

	Collision resultingCollision = { .isColliding = false, .delta = {1.0f, 1.0f}, .intersect = {1.0f, 1.0f} };

	Coord2D thisPosition = thisEntity->obj.position;
	Coord2D thisHalfSize = { .x = thisEntity->obj.size.x / 2, .y = thisEntity->obj.size.y / 2 };
	Coord2D otherPosition = otherEntity->obj.position;
	Coord2D otherHalfSize = { .x = otherEntity->obj.size.x / 2, .y = otherEntity->obj.size.y / 2 };

	float deltaX = otherPosition.x - thisPosition.x;
	float deltaY = otherPosition.y - thisPosition.y;

	float intersectX = (float)fabs(deltaX) - (thisHalfSize.x + otherHalfSize.x);
	float intersectY = (float)fabs(deltaY) - (thisHalfSize.y + otherHalfSize.y);

	resultingCollision.delta.x = deltaX;
	resultingCollision.delta.y = deltaY;
	resultingCollision.intersect.x = intersectX;
	resultingCollision.intersect.y = intersectY;

	if (intersectX < 0.0f && intersectY < 0.0f)
	{
		resultingCollision.isColliding = true;
	}
	return resultingCollision;
}
