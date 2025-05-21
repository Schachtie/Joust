#include "livesDisplay.h"
#include "player.h"


static const Coord2D _LIFE_SPRITE_PLAYER1_TL = { .x = 195.5f, .y = 371 };
static const Coord2D _LIFE_SPRITE_PLAYER1_BR = { .x = 209, .y = 352 };
static Sprite* _player1LifeSprite = NULL;

	// These should not be used at all currently, no sprite has been made for player 2. This is only for future development.
static const Coord2D _LIFE_SPRITE_PLAYER2_TL;
static const Coord2D _LIFE_SPRITE_PLAYER2_BR;
static Sprite* _player2LifeSprite = NULL;


// =============== vTable ===============
static void _livesDisplayDraw(Object* obj);
static ObjVtable _livesDisplayVtable = {
	livesDisplayDelete,
	_livesDisplayDraw,
	NULL,
	NULL
};


/// <summary>
/// Initializes the internal life display sprites.
/// </summary>
/// <param name="sheet"></param>
void livesDisplayInit(const SpriteSheet* const sheet)
{
	// This should create the sprites used for displaying lives
	Bounds2D spriteUV;
		// Player 1
	spriteUV.topLeft.x = _LIFE_SPRITE_PLAYER1_TL.x / sheet->WIDTH_PIXELS;
	spriteUV.topLeft.y = _LIFE_SPRITE_PLAYER1_TL.y / sheet->HEIGHT_PIXELS;
	spriteUV.botRight.x = _LIFE_SPRITE_PLAYER1_BR.x / sheet->WIDTH_PIXELS;
	spriteUV.botRight.y = _LIFE_SPRITE_PLAYER1_BR.y / sheet->HEIGHT_PIXELS;
	_player1LifeSprite = spriteNew(sheet, spriteUV, 0);
	assert(_player1LifeSprite != NULL);

		// Player 2 - Not currently implemented (out of scope)
}

/// <summary>
/// Deletes the internal life display sprites.
/// </summary>
void livesDisplayShutdown()
{
	// This should destroy the sprites used for displaying lives
	spriteDelete(_player1LifeSprite);
}


/// <summary>
/// Creates a new life display object.
///		<para>
/// Note: Position is the top left pixel of the whole display.
///		</para>
/// </summary>
/// <param name="pos"></param>
/// <param name="size"></param>
/// <param name="numSprites"></param>
/// <param name="pixelsBetweenSprites"></param>
/// <returns></returns>
LivesDisplay* livesDisplayNew(Coord2D pos, Coord2D size, uint8_t numSprites, uint8_t pixelsBetweenSprites)
{
	LivesDisplay* livesDisplay = (LivesDisplay*)malloc(sizeof(LivesDisplay));
	if (livesDisplay != NULL)
	{
		objInit(&livesDisplay->obj, &_livesDisplayVtable, pos, false);
		livesDisplay->obj.size = size;
		livesDisplay->numSprites = numSprites;
		livesDisplay->numSpritesToDisplay = numSprites;
		livesDisplay->_pixelsBetweenSprites = pixelsBetweenSprites;

		// Set the necessary sprite
		livesDisplay->sprite = _player1LifeSprite;

		// Calculate the dimensions for each number based on the passed in params
			// Y - just is the height of the size
		livesDisplay->_dimensionsPerSprite.y = size.y;
			// X - Needs to be calculated
		assert(pixelsBetweenSprites * (numSprites - 1) < size.x); // this is not a good enough assert, consider this formula more
		livesDisplay->_dimensionsPerSprite.x = (size.x - (pixelsBetweenSprites * (numSprites - 1))) / numSprites;

		// Allocate space for and determine the sprites' positions in world space
		livesDisplay->_spritePositions = (Coord2D*)malloc(sizeof(Coord2D) * numSprites);
		if (livesDisplay->_spritePositions != NULL)
		{
			// Set the first one (left most)
			livesDisplay->_spritePositions[numSprites - 1].x = pos.x + livesDisplay->_dimensionsPerSprite.x / 2;
			livesDisplay->_spritePositions[numSprites - 1].y = pos.y + livesDisplay->_dimensionsPerSprite.y / 2;

			for (uint8_t i = 1; i < numSprites; ++i)
			{
				livesDisplay->_spritePositions[numSprites - 1 - i].x = pos.x + (i * (livesDisplay->_dimensionsPerSprite.x + pixelsBetweenSprites)) + livesDisplay->_dimensionsPerSprite.x / 2;
				livesDisplay->_spritePositions[numSprites - 1 - i].y = livesDisplay->_spritePositions[numSprites - 1].y;
			}
		}
	}
	return livesDisplay;
}

/// <summary>
/// Deletes the passed in life display object.
/// </summary>
/// <param name="obj"></param>
void livesDisplayDelete(Object* obj)
{
	LivesDisplay* livesDisplay = (LivesDisplay*)obj;

	free(livesDisplay->_spritePositions);
	objDeinit(&livesDisplay->obj);
	free(livesDisplay);
}


/// <summary>
/// Draws the life display object to the screen.
/// </summary>
/// <param name="obj"></param>
static void _livesDisplayDraw(Object* obj)
{
	LivesDisplay* livesDisplay = (LivesDisplay*)obj;

	// Display the number of lives left starting from the left
	uint8_t numSprites = livesDisplay->numSprites;
	uint8_t numSpritesToDisplay = livesDisplay->numSpritesToDisplay;
	for (uint8_t i = 0; i < numSpritesToDisplay; ++i)
	{
		spriteDraw(livesDisplay->sprite, livesDisplay->_spritePositions[numSprites - 1 - i], livesDisplay->_dimensionsPerSprite, false);
	}
}
