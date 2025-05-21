#include <assert.h>

#include "animation.h"


/// <summary>
/// Creates an animation based on a sequence of sprites in the passed in sprite sheet.
///		<para>
/// Assumes the sprites are ordered from left to right.
///		</para>
/// </summary>
/// <param name="numFrames"></param>
/// <param name="sheet"></param>
/// <param name="firstSpriteBounds"></param>
/// <param name="pixelsBetweenFrames"></param>
/// <param name="depth"></param>
/// <returns></returns>
Animation* animationNew(uint8_t numFrames, const SpriteSheet* const sheet, Bounds2D firstSpriteBounds, float pixelsBetweenFrames, float depth)
{
	assert(numFrames > 0);
	assert(sheet != NULL);

	Animation* animation = (Animation*)malloc(sizeof(Animation));
	if (animation != NULL)
	{
		animation->numFrames = numFrames;
		animation->spriteFrames = (Sprite**)malloc(numFrames * sizeof(Sprite*));
		if (animation->spriteFrames != NULL)
		{
			// Determine the uv of the first frame
			// Will need the width of the spriteFrame
			// Will need the uv cost of x pixel(s) width
			float uPixelsBetweenFrames = (float)pixelsBetweenFrames / (float)sheet->WIDTH_PIXELS;
			Bounds2D spriteUV = {	.topLeft = {.x = firstSpriteBounds.topLeft.x / (float)sheet->WIDTH_PIXELS, .y = firstSpriteBounds.topLeft.y / (float)sheet->HEIGHT_PIXELS},
										.botRight = {.x = firstSpriteBounds.botRight.x / (float)sheet->WIDTH_PIXELS, .y = firstSpriteBounds.botRight.y / (float)sheet->HEIGHT_PIXELS} };
			float uCostPerFrame = spriteUV.botRight.x - spriteUV.topLeft.x;

			animation->spriteFrames[0] = spriteNew(sheet, spriteUV, depth);
			for (uint8_t i = 1; i < numFrames; ++i)
			{
				spriteUV.topLeft.x += uCostPerFrame + uPixelsBetweenFrames;
				spriteUV.botRight.x += uCostPerFrame + uPixelsBetweenFrames;

				animation->spriteFrames[i] = spriteNew(sheet, spriteUV, depth);
			}
		}
	}
	return animation;
}

/// <summary>
/// Deletes the passed in animation.
/// </summary>
/// <param name="animation"></param>
void animationDelete(Animation* animation)
{
	for (uint8_t i = 0; i < animation->numFrames; ++i)
	{
		spriteDelete(animation->spriteFrames[i]);
	}

	free(animation);
}


/// <summary>
/// Draws a sprite from the passed in animation based on the specified frameNumber.
/// </summary>
/// <param name="animation"></param>
/// <param name="frameNumber"></param>
/// <param name="screenPosition"></param>
/// <param name="objDimensions"></param>
/// <param name="horzReflect"></param>
void animationDraw(const Animation* const animation, uint8_t frameNumber, Coord2D screenPosition, Coord2D objDimensions, bool horzReflect)
{
	assert(frameNumber < animation->numFrames);

	spriteDraw(animation->spriteFrames[frameNumber], screenPosition, objDimensions, horzReflect);
}
