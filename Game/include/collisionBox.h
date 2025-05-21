#pragma once

#include "entity.h"


typedef struct collisionBox_t {
	Entity		entity;
} CollisionBox;


CollisionBox* collisionBoxNew(Coord2D topLeftPos, Coord2D size);
void collisionBoxDelete(Object* collisionBox);
