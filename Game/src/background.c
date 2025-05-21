#include <stdlib.h>

#include "background.h"
#include "joustGlobalConstants.h"


// =============== vTable ===============
static void _backgroundDraw(Object* obj);
static ObjVtable _backgroundVtable = {
	backgroundDelete,
	_backgroundDraw,
	NULL,
	NULL
};


/// <summary>
/// Creates a background object. Sets the position to the center of the screen.
/// </summary>
/// <param name="sheet"></param>
/// <param name="spriteBounds"></param>
/// <param name="size"></param>
/// <returns></returns>
Background* backgroundNew(const SpriteSheet* const sheet, Bounds2D spriteBounds, Coord2D size)
{
	Background* background = (Background*)malloc(sizeof(Background));
	if (background != NULL)
	{
		//Need the center position of the screen
		Bounds2D gameBounds = { .topLeft = {0, 0}, .botRight = SCREEN_RESOLUTION };
		objInit(&background->obj, &_backgroundVtable, boundsGetCenter(&gameBounds), false);
		background->obj.size = size;

		Bounds2D spriteUV = { .topLeft = {spriteBounds.topLeft.x / (float)sheet->WIDTH_PIXELS, spriteBounds.topLeft.y / (float)sheet->HEIGHT_PIXELS},
									.botRight = {spriteBounds.botRight.x / (float)sheet->WIDTH_PIXELS, spriteBounds.botRight.y / (float)sheet->HEIGHT_PIXELS} };
		background->sprite = spriteNew(sheet, spriteUV, -0.99f);
	}
	return background;
}

/// <summary>
/// Deletes the passed in background object.
/// </summary>
/// <param name="background"></param>
void backgroundDelete(Object* background)
{
	if (background != NULL)
	{
		Background* backgroundCast = (Background*)background;
		objDeinit(&backgroundCast->obj);
		spriteDelete(backgroundCast->sprite);
	}
	free(background);
}


/// <summary>
/// Draws the background image.
/// </summary>
/// <param name="obj"></param>
static void _backgroundDraw(Object* obj)
{
	Background* background = (Background*)obj;

	spriteDraw(background->sprite, background->obj.position, background->obj.size, false);
}
