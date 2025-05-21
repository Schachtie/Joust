#pragma once

#include <Windows.h>
#include <gl/GL.h>

#include "sprite.h"
#include "baseTypes.h"


typedef struct spriteSheet_t {
	GLuint textureHandle;
	uint16_t WIDTH_PIXELS;
	uint16_t HEIGHT_PIXELS;
} SpriteSheet;

typedef struct sprite_t {
	const SpriteSheet* spriteSheet;
	Bounds2D	spriteBounds; //these should be in UV
	float		depth;
} Sprite;


Sprite* spriteNew(const SpriteSheet* const sheet, Bounds2D spriteUV, float depth);
void spriteDelete(Sprite* sprite);

void spriteDraw(const Sprite* const sprite, Coord2D screenPosition, Coord2D objDimensions, bool horzReflect);
