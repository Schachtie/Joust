#pragma once

#include "baseTypes.h"


typedef enum collisionResponse_t {
	COLLRESP_NOTHING,
	COLLRESP_PLAYER,
	COLLRESP_ENEMY,
	COLLRESP_PLATFORM,
	COLLRESP_EGG,
	COLLRESP_GAMEBOUNDS,

	COLLRESP_COUNT
} CollisionResponse;


typedef struct collision_t {
	bool		isColliding;
	Coord2D		delta;
	Coord2D		intersect;
} Collision;


Collision detectCollision(const Bounds2D* const thisBounds, const Bounds2D* const otherBounds);
