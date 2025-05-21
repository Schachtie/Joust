#include <math.h>

#include "entity.h"
#include "SOIL.h"
#include "joustGlobalConstants.h"
#include "tools.h"


float ENT_DEFAULT_VELOCITY_CHANGE = 5;
float ENT_BALANCE_FLAP_MULTI = 2;
float ENT_BALANCE_FLAP_SINGLE = 25;
const float MAGIC_NUMBER_PLATFORMFLAP = 0.00005f; // This is here to put the entity always inside the top of a platform, so the "isGrounded" field is set while running

static const float GRAVITY = 300;
static const Coord2D DEFAULT_VELOCITY = { .x = 0, .y = 0 };
static const Coord2D DEFAULT_TERMINAL_VELOCITY = { .x = 200, .y = 200 };
static const CollisionResponse DEFAULT_COLLRESP = COLLRESP_NOTHING;


/// <summary>
/// Initializes an entity object.
/// </summary>
/// <param name="entity"></param>
/// <param name="vtable"></param>
/// <param name="pos"></param>
/// <param name="size"></param>
void entityInit(Entity* entity, ObjVtable* vtable, Coord2D pos, Coord2D size)
{
	// Create the object within
	objInit(&entity->obj, vtable, pos, true);
	entity->obj.size = size;

	// Set the default values for this class
	entity->awake = false;
	entity->isGrounded = true;
	entity->velocity = DEFAULT_VELOCITY;
	entity->terminalVelocity = DEFAULT_TERMINAL_VELOCITY;
	entity->collresp = DEFAULT_COLLRESP;

	entity->gameBounds.topLeft.x = 0;
	entity->gameBounds.topLeft.y = 0;
	entity->gameBounds.botRight = SCREEN_RESOLUTION;
}

/// <summary>
/// Deinitializes an entity object.
/// </summary>
/// <param name="entity"></param>
void entityDeinit(Entity* entity)
{
	if (entity != NULL)
	{
		objDeinit(&entity->obj);
	}
}


/// <summary>
/// Applies physics and gravity to an entity object based on its internal state. Checks for collision against game bounds (top/bottom = bounce | left/right = wrap).
///		<para>
/// Note: Also caps velocity based on the entity's internal terminal velocity.
///		</para>
/// </summary>
/// <param name="obj"></param>
/// <param name="milliseconds"></param>
void entityDefaultUpdate(Object* obj, uint32_t milliseconds)
{
	Entity* entity = (Entity*)obj;

	// Physics updates
	float seconds = (float)milliseconds / 1000.0f;
		// Check for velocity cap
	entity->velocity.x = toolClampFloat(entity->velocity.x, -(entity->terminalVelocity.x), entity->terminalVelocity.x);
	entity->velocity.y = toolClampFloat(entity->velocity.y, -(entity->terminalVelocity.y), entity->terminalVelocity.y);

		// Initial position update
	entity->obj.position.x += (entity->velocity.x * seconds);
	entity->obj.position.y += (entity->velocity.y * seconds) + (0.5f * GRAVITY * (float)pow(seconds, 2));

	// Check for game bounds
	if (entity->obj.position.y + entity->obj.size.y / 2 > entity->gameBounds.botRight.y) //this will be where lava kills you? or probably not, as lava will be a collision box
	{
		entity->obj.position.y = entity->gameBounds.botRight.y - entity->obj.size.y / 2;
		entity->velocity.y = 0;
		entity->isGrounded = true;
	}
	else if (entity->obj.position.y - entity->obj.size.y / 2 < entity->gameBounds.topLeft.y)
	{
		entity->obj.position.y = entity->gameBounds.topLeft.y + entity->obj.size.y / 2;
		entity->velocity.y = -(entity->velocity.y);
	}

		// Check for wall wrap
	if (entity->obj.position.x < entity->gameBounds.topLeft.x)
	{
		float overlap = entity->gameBounds.topLeft.x - entity->obj.position.x;
		entity->obj.position.x = entity->gameBounds.botRight.x - overlap;

	}
	else if (entity->obj.position.x > entity->gameBounds.botRight.x)
	{
		float overlap = entity->obj.position.x - entity->gameBounds.botRight.x;
		entity->obj.position.x = entity->gameBounds.topLeft.x + overlap;
	}

	// Default obj update
	objDefaultUpdate(obj, milliseconds);

	// Apply gravity to velocity
	entity->velocity.y += GRAVITY * seconds;
}

/// <summary>
/// Handles collision response against platforms.
/// </summary>
/// <param name="thisObj"></param>
/// <param name="otherObj"></param>
/// <param name="collision"></param>
void entityDefaultCollide(Object* thisObj, Object* otherObj, Collision collision)
{
	Entity* thisEntity = (Entity*)thisObj;
	Entity* otherEntity = (Entity*)otherObj;

	switch (otherEntity->collresp)
	{
		case COLLRESP_PLATFORM:
		{
			// Check for floor collision
				// Vertical push
			if (collision.intersect.x < collision.intersect.y)
			{
				// Top of platform
				if (collision.delta.y > 0.0f) // This could be where the check for the bouncing JP was talking about can fit
				{
					// Snap the player to the floor of the platform
					thisEntity->obj.position.y = otherEntity->obj.position.y - otherEntity->obj.size.y / 2 - thisEntity->obj.size.y / 2 + MAGIC_NUMBER_PLATFORMFLAP;
					thisEntity->isGrounded = true;
					thisEntity->velocity.y = 0;
				}
				else // Below platform
				{
					thisEntity->obj.position.y = otherEntity->obj.position.y + otherEntity->obj.size.y / 2 + thisEntity->obj.size.y / 2;
					thisEntity->velocity.y = -(thisEntity->velocity.y);
				}
			}
			// Horizontal push
			else if (collision.intersect.x > collision.intersect.y)
			{
				// Left of platform
				if (collision.delta.x > 0.0f)
				{
					thisEntity->obj.position.x = otherEntity->obj.position.x - otherEntity->obj.size.x / 2 - thisEntity->obj.size.x / 2;
					thisEntity->velocity.x = -(thisEntity->velocity.x);
				}
				else // Right of platform
				{
					thisEntity->obj.position.x = otherEntity->obj.position.x + otherEntity->obj.size.x / 2 + thisEntity->obj.size.x / 2;
					thisEntity->velocity.x = -(thisEntity->velocity.x);
				}
			}
			break;
		}
		default:
		{
			break;
		}
	}
}
