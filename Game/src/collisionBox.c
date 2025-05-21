#include <stdlib.h>

#include "collisionBox.h"
#include "shape.h"


static bool _debugDraw = false;


// =============== vTable ===============
static void _collisionBoxDraw(Object* obj);
static ObjVtable _collisionBoxVtable = {
	collisionBoxDelete,
	_collisionBoxDraw,
	NULL,
	NULL
};


/// <summary>
/// Creates a new collisionBox object.
/// </summary>
/// <param name="topLeftPos"></param>
/// <param name="size"></param>
/// <returns></returns>
CollisionBox* collisionBoxNew(Coord2D topLeftPos, Coord2D size)
{
	CollisionBox* collisionBox = (CollisionBox*)malloc(sizeof(CollisionBox));
	if (collisionBox != NULL)
	{
		// Determine the center position of the box
		Bounds2D boxBounds = { .topLeft = topLeftPos, .botRight = {.x = topLeftPos.x + size.x - 1, .y = topLeftPos.y + size.y - 1} };

		entityInit(&collisionBox->entity, &_collisionBoxVtable, boundsGetCenter(&boxBounds), size);
		collisionBox->entity.collresp = COLLRESP_PLATFORM;
	}
	return collisionBox;
}

/// <summary>
/// Deletes the passed in collisionBox object.
/// </summary>
/// <param name="collisionBox"></param>
void collisionBoxDelete(Object* collisionBox)
{
	if (collisionBox != NULL)
	{
		entityDeinit(&((CollisionBox*)collisionBox)->entity);
	}
	free(collisionBox);
}


/// <summary>
/// Draws the outline of the collision box if the internal debug boolean is set to true.
/// </summary>
/// <param name="obj"></param>
static void _collisionBoxDraw(Object* obj)
{
	if (_debugDraw)
	{
		 CollisionBox* collisionBox = (CollisionBox*)obj;

		 float left = collisionBox->entity.obj.position.x - collisionBox->entity.obj.size.x / 2.0f;
		 float right = collisionBox->entity.obj.position.x + collisionBox->entity.obj.size.x / 2.0f;
		 float bottom = collisionBox->entity.obj.position.y - collisionBox->entity.obj.size.y / 2.0f;
		 float top = collisionBox->entity.obj.position.y + collisionBox->entity.obj.size.y / 2.0f;

		 uint8_t r = (uint8_t)(0x0000ff00 >> 16 & 0xFF);
		 uint8_t g = (uint8_t)(0x0000ff00 >> 8 & 0xFF);
		 uint8_t b = (uint8_t)(0x0000ff00 >> 0 & 0xFF);

		 shapeDrawLine(left, top, right, top, r, g, b);
		 shapeDrawLine(right, top, right, bottom, r, g, b);
		 shapeDrawLine(right, bottom, left, bottom, r, g, b);
		 shapeDrawLine(left, bottom, left, top, r, g, b);
	}
}
