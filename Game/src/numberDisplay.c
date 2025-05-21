#include <assert.h>
#include <math.h>

#include "numberDisplay.h"
#include "joustGlobalConstants.h"


static Sprite* _numbersYellow[10];
static Sprite* _numbersBlue[10];
static Sprite* _numbersWhite[10];


// =============== vTable ===============
static void _numberDisplayDraw(Object* obj);
static ObjVtable _numberDisplayVtable = {
	numberDisplayDelete,
	_numberDisplayDraw,
	NULL,
	NULL
};


/// <summary>
/// Initializes all the internal sprites for various sets of colored numbers.
/// </summary>
/// <param name="sheet"></param>
void numberDisplayInit(const SpriteSheet* const sheet)
{
	// Set all the values of the number sprite arrays
	
	Bounds2D spriteUV;

		// Yellow
	spriteUV.topLeft.y = NUMBER_SPRITE_YELLOW_0_TL.y / sheet->HEIGHT_PIXELS;
	spriteUV.botRight.y = NUMBER_SPRITE_YELLOW_0_BR.y / sheet->HEIGHT_PIXELS;
	for (uint8_t i = 0; i < 10; ++i)
	{
		spriteUV.topLeft.x = (NUMBER_SPRITE_YELLOW_0_TL.x + (i * NUMBER_SPRITE_PIXELS_BETWEEN_SPRITES)) / sheet->WIDTH_PIXELS;
		spriteUV.botRight.x = (NUMBER_SPRITE_YELLOW_0_BR.x + (i * NUMBER_SPRITE_PIXELS_BETWEEN_SPRITES)) / sheet->WIDTH_PIXELS;
		_numbersYellow[i] = spriteNew(sheet, spriteUV, 0);
	}
		// Blue
	spriteUV.topLeft.y = NUMBER_SPRITE_BLUE_0_TL.y / sheet->HEIGHT_PIXELS;
	spriteUV.botRight.y = NUMBER_SPRITE_BLUE_0_BR.y / sheet->HEIGHT_PIXELS;
	for (uint8_t i = 0; i < 10; ++i)
	{
		spriteUV.topLeft.x = (NUMBER_SPRITE_BLUE_0_TL.x + (i * NUMBER_SPRITE_PIXELS_BETWEEN_SPRITES)) / sheet->WIDTH_PIXELS;
		spriteUV.botRight.x = (NUMBER_SPRITE_BLUE_0_BR.x + (i * NUMBER_SPRITE_PIXELS_BETWEEN_SPRITES)) / sheet->WIDTH_PIXELS;
		_numbersBlue[i] = spriteNew(sheet, spriteUV, 0);
	}
		// White
	spriteUV.topLeft.y = NUMBER_SPRITE_WHITE_0_TL.y / sheet->HEIGHT_PIXELS;
	spriteUV.botRight.y = NUMBER_SPRITE_WHITE_0_BR.y / sheet->HEIGHT_PIXELS;
	for (uint8_t i = 0; i < 10; ++i)
	{
		spriteUV.topLeft.x = (NUMBER_SPRITE_WHITE_0_TL.x + (i * NUMBER_SPRITE_PIXELS_BETWEEN_SPRITES)) / sheet->WIDTH_PIXELS;
		spriteUV.botRight.x = (NUMBER_SPRITE_WHITE_0_BR.x + (i * NUMBER_SPRITE_PIXELS_BETWEEN_SPRITES)) / sheet->WIDTH_PIXELS;
		_numbersWhite[i] = spriteNew(sheet, spriteUV, 0);
	}
}

/// <summary>
/// Deletes the internal sprites for all number sets.
/// </summary>
void numberDisplayShutdown()
{
	for (uint8_t i = 0; i < 10; ++i)
	{
		spriteDelete(_numbersWhite[i]);
		spriteDelete(_numbersBlue[i]);
		spriteDelete(_numbersYellow[i]);
	}
}


/// <summary>
/// Creates a new number display object.
///		<para>
/// Note: Position is the top left pixel of the whole display.
///		</para>
/// </summary>
/// <param name="color"></param>
/// <param name="pos"></param>
/// <param name="size"></param>
/// <param name="numDigits"></param>
/// <param name="pixelsBetweenNumbers"></param>
/// <returns></returns>
NumberDisplay* numberDisplayNew(NumberColor color, Coord2D pos, Coord2D size, uint8_t numDigits, uint8_t pixelsBetweenNumbers)
{
	NumberDisplay* numberDisplay = (NumberDisplay*)malloc(sizeof(NumberDisplay));
	if (numberDisplay != NULL)
	{
		objInit(&numberDisplay->obj, &_numberDisplayVtable, pos, false);
		numberDisplay->obj.size = size;
		numberDisplay->color = color;
		numberDisplay->numberToDisplay = 0;
		numberDisplay->numDigits = numDigits;
		numberDisplay->_pixelsBetweenNumbers = pixelsBetweenNumbers;
		
		// Calculate the dimensions for each number based on the passed in params
			// Y - Just is the height of the size
		numberDisplay->_dimensionsPerNumber.y = size.y;
			// X - Needs to be calculated
		assert(pixelsBetweenNumbers * (numDigits - 1) < size.x); // this is not a good enough assert, consider this formula more
		numberDisplay->_dimensionsPerNumber.x = (size.x - (pixelsBetweenNumbers * (numDigits - 1))) / numDigits;

		// Allocate space for and determine the numbers' positions in world space
		numberDisplay->_numberPositions = (Coord2D*)malloc(sizeof(Coord2D) * numDigits);
		if (numberDisplay->_numberPositions != NULL)
		{
			// Set the first one (left most)
			numberDisplay->_numberPositions[numDigits - 1].x = pos.x + numberDisplay->_dimensionsPerNumber.x / 2;
			numberDisplay->_numberPositions[numDigits - 1].y = pos.y + numberDisplay->_dimensionsPerNumber.y / 2;

			for (uint8_t i = 1; i < numDigits; ++i)
			{
				numberDisplay->_numberPositions[numDigits - 1 - i].x = pos.x + (i * (numberDisplay->_dimensionsPerNumber.x + pixelsBetweenNumbers)) + numberDisplay->_dimensionsPerNumber.x / 2;
				numberDisplay->_numberPositions[numDigits - 1 - i].y = numberDisplay->_numberPositions[numDigits - 1].y;
			}
		}
	}
	return numberDisplay;
}

/// <summary>
/// Deletes the number display object.
/// </summary>
/// <param name="obj"></param>
void numberDisplayDelete(Object* obj)
{
	NumberDisplay* numberDisplay = (NumberDisplay*)obj;

	free(numberDisplay->_numberPositions);
	objDeinit(&numberDisplay->obj);
	free(numberDisplay);
}


/// <summary>
/// Draws the number display object to the screen.
/// </summary>
/// <param name="obj"></param>
static void _numberDisplayDraw(Object* obj)
{
	NumberDisplay* numberDisplay = (NumberDisplay*)obj;

	// Get the proper color array of numbers
	const Sprite** numberArray = NULL;
	switch (numberDisplay->color)
	{
	case NUMBERCOLOR_YELLOW:
	{
		numberArray = _numbersYellow;
		break;
	}
	case NUMBERCOLOR_BLUE:
	{
		numberArray = _numbersBlue;
		break;
	}
	case NUMBERCOLOR_WHITE:
	{
		numberArray = _numbersWhite;
		break;
	}
	default:
	{
		break;
	}
	}
	assert(numberArray != NULL);


	// Pull off each of the digits and display them in the proper location based on the world space
	bool isFirstDigitPrinted = false;
	uint32_t remainder = numberDisplay->numberToDisplay;
	for (int8_t i = numberDisplay->numDigits - 1; i >= 0; --i)
	{
		if (remainder / (uint32_t)pow(10, i) != 0) { isFirstDigitPrinted = true; }
		if (isFirstDigitPrinted || i == 0) { spriteDraw(numberArray[remainder / (uint32_t)pow(10, i)], numberDisplay->_numberPositions[i], numberDisplay->_dimensionsPerNumber, false); }
		remainder %= (uint32_t)pow(10, i);
	}
}


/// <summary>
/// Moves the entire number display to the new position.
/// </summary>
/// <param name="numberDisplay"></param>
/// <param name="newPosition"></param>
void numberDisplayChangePosition(NumberDisplay* numberDisplay, Coord2D newPosition)
{
	numberDisplay->obj.position = newPosition;

	// Set the first one (left most)
	numberDisplay->_numberPositions[numberDisplay->numDigits - 1].x = newPosition.x + numberDisplay->_dimensionsPerNumber.x / 2;
	numberDisplay->_numberPositions[numberDisplay->numDigits - 1].y = newPosition.y + numberDisplay->_dimensionsPerNumber.y / 2;

	for (uint8_t i = 1; i < numberDisplay->numDigits; ++i)
	{
		numberDisplay->_numberPositions[numberDisplay->numDigits - 1 - i].x = newPosition.x + (i * (numberDisplay->_dimensionsPerNumber.x + numberDisplay->_pixelsBetweenNumbers)) + numberDisplay->_dimensionsPerNumber.x / 2;
		numberDisplay->_numberPositions[numberDisplay->numDigits - 1 - i].y = numberDisplay->_numberPositions[numberDisplay->numDigits - 1].y;
	}
}
