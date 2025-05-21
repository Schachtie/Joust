#pragma once


#include "sprite.h"


typedef struct animation_t {
	Sprite**	spriteFrames;
	uint8_t		numFrames;
} Animation;


Animation* animationNew(uint8_t numFrames, const SpriteSheet* const sheet, Bounds2D firstSpriteBounds, float pixelsBetweenFrames, float depth);
void animationDelete(Animation* animation);

void animationDraw(const Animation* const animation, uint8_t frameNumber, Coord2D screenPosition, Coord2D objDimensions, bool horzReflect);
