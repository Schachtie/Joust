#pragma once

#include "object.h"
#include "sprite.h"


typedef enum numberColor_t {
	NUMBERCOLOR_YELLOW,
	NUMBERCOLOR_BLUE,
	NUMBERCOLOR_WHITE
	// NOTE: Green & Red don't have full number sets rn
} NumberColor;


typedef struct numberDisplay_t {
	Object obj; // Note: the internal object's position should be TL position of highest digit to print. Size will be used to determine the other values in this object.

	NumberColor color;
	uint32_t numberToDisplay;
	uint8_t numDigits;

	Coord2D _dimensionsPerNumber;
	uint8_t _pixelsBetweenNumbers;
	Coord2D* _numberPositions;
} NumberDisplay;


void numberDisplayInit(const SpriteSheet* const sheet);
void numberDisplayShutdown();

NumberDisplay* numberDisplayNew(NumberColor color, Coord2D pos, Coord2D size, uint8_t numDigits, uint8_t pixelsBetweenNumbers);
void numberDisplayDelete(Object* obj);

void numberDisplayChangePosition(NumberDisplay* numberDisplay, Coord2D newPosition);
