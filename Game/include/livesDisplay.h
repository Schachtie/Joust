#pragma once

#include "object.h"
#include "sprite.h"


// NOTE: this class and "numberDisplay" are very similar. If there is time to refactor, have them inherit from a common "horizontalLayoutGroup" class or something


typedef struct livesDisplay_t {
	Object obj; // Note: the internal object's position should be TL position of highest digit to print. Size will be used to determine the other values in this object.

	Sprite* sprite; // Note: there is only one sprite being repeatedly drawn here, not a group of several sprites
	uint8_t numSprites;
	uint8_t numSpritesToDisplay;

	Coord2D _dimensionsPerSprite;
	uint8_t _pixelsBetweenSprites;
	Coord2D* _spritePositions;
} LivesDisplay;


void livesDisplayInit(const SpriteSheet* const sheet);
void livesDisplayShutdown();

LivesDisplay* livesDisplayNew(Coord2D pos, Coord2D size, uint8_t numSprites, uint8_t pixelsBetweenSprites);
void livesDisplayDelete(Object* obj);
