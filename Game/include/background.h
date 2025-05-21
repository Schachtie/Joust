#pragma once

#include <Windows.h>
#include <gl/GL.h>

#include "object.h"
#include "sprite.h"


// This class's name is a misnomer. It should be called something more like "Image", but there is not enough time to refactor.


typedef struct background_t {
	Object		obj;
	Sprite*		sprite;
} Background;


Background* backgroundNew(const SpriteSheet* const sheet, Bounds2D spriteBounds, Coord2D size);
void backgroundDelete(Object* background);
