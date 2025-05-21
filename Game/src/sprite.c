#include "sprite.h"


/// <summary>
/// Creates a new sprite object.
/// </summary>
/// <param name="sheet"></param>
/// <param name="spriteUV"></param>
/// <param name="depth"></param>
/// <returns></returns>
Sprite* spriteNew(const SpriteSheet* const sheet, Bounds2D spriteUV, float depth)
{
	Sprite* sprite = (Sprite*)malloc(sizeof(Sprite));
	if (sprite != NULL)
	{
		sprite->spriteSheet = sheet;
		sprite->spriteBounds = spriteUV;
		sprite->depth = depth;
	}
	return sprite;
}

/// <summary>
/// Deletes the sprite object.
/// </summary>
/// <param name="sprite"></param>
void spriteDelete(Sprite* sprite)
{
	free(sprite);
}


/// <summary>
/// Draws a sprite to the screen.
/// </summary>
/// <param name="sprite"></param>
/// <param name="screenPosition"></param>
/// <param name="objDimensions"></param>
/// <param name="horzReflect"> - True will horizontally reflect the image.</param>
void spriteDraw(const Sprite* const sprite, Coord2D screenPosition, Coord2D objDimensions, bool horzReflect)
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, sprite->spriteSheet->textureHandle);
	glBegin(GL_TRIANGLE_STRIP);
	{
		// calculate the bounding box
		GLfloat xPositionLeft = (screenPosition.x - objDimensions.x / 2);
		GLfloat xPositionRight = (screenPosition.x + objDimensions.x / 2);
		GLfloat yPositionTop = (screenPosition.y - objDimensions.y / 2);
		GLfloat yPositionBottom = (screenPosition.y + objDimensions.y / 2);

		const float DEPTH = sprite->depth;

		// draw the textured quad as a tristrip
		glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);

		Bounds2D spriteBounds = sprite->spriteBounds;

		// This is for horizontally reflecting the image
		if (!horzReflect)
		{
			// TL
			glTexCoord2f(spriteBounds.topLeft.x, spriteBounds.topLeft.y);
			glVertex3f(xPositionLeft, yPositionTop, DEPTH);

			// BL
			glTexCoord2f(spriteBounds.topLeft.x, spriteBounds.botRight.y);
			glVertex3f(xPositionLeft, yPositionBottom, DEPTH);

			// TR
			glTexCoord2f(spriteBounds.botRight.x, spriteBounds.topLeft.y);
			glVertex3f(xPositionRight, yPositionTop, DEPTH);

			// BR
			glTexCoord2f(spriteBounds.botRight.x, spriteBounds.botRight.y);
			glVertex3f(xPositionRight, yPositionBottom, DEPTH);
		}
		else
		{
			// TL
			glTexCoord2f(spriteBounds.botRight.x, spriteBounds.topLeft.y);
			glVertex3f(xPositionLeft, yPositionTop, DEPTH);

			// BL
			glTexCoord2f(spriteBounds.botRight.x, spriteBounds.botRight.y);
			glVertex3f(xPositionLeft, yPositionBottom, DEPTH);

			// TR
			glTexCoord2f(spriteBounds.topLeft.x, spriteBounds.topLeft.y);
			glVertex3f(xPositionRight, yPositionTop, DEPTH);

			// BR
			glTexCoord2f(spriteBounds.topLeft.x, spriteBounds.botRight.y);
			glVertex3f(xPositionRight, yPositionBottom, DEPTH);
		}

	}
	glEnd();
}
