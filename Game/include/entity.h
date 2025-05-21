#pragma once

#include <Windows.h>
#include <stdlib.h>
#include <gl/GL.h>
#include <assert.h>

#include "object.h"
#include "input.h"
#include "sprite.h"


typedef struct entity_t {
	Object				obj;

	bool				awake;      // for collision -> things that need to have collision checked vs other things (is it moving?)
	bool				isGrounded;
	Bounds2D			gameBounds;
	Coord2D				velocity;
	Coord2D				terminalVelocity;
	CollisionResponse	collresp;

	// These are for players/enemies
	Coord2D flyingSize;
	Coord2D flyingPosition;

} Entity;


extern float ENT_DEFAULT_VELOCITY_CHANGE;
extern float ENT_BALANCE_FLAP_MULTI;
extern float ENT_BALANCE_FLAP_SINGLE;
extern const float MAGIC_NUMBER_PLATFORMFLAP; // This is here to put the entity always inside the top of a platform, so the "isGrounded" field is set while running. It should also be used when the entity is flapping to allow it to get out of said platform.


void entityInit(Entity* entity, ObjVtable* vtable, Coord2D pos, Coord2D size);
void entityDeinit(Entity* entity);


void entityDefaultUpdate(Object* obj, uint32_t milliseconds);
void entityDefaultCollide(Object* thisObj, Object* otherObj, Collision collision);
