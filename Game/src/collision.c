#include <assert.h>
#include <math.h>

#include "collision.h"


/// <summary>
/// Detects whether two Bounds2Ds are colliding/overlapping.
/// </summary>
/// <param name="thisBounds"></param>
/// <param name="otherBounds"></param>
/// <returns>Information relating to the collision that may have occurred.</returns>
Collision detectCollision(const Bounds2D* const thisBounds, const Bounds2D* const otherBounds)
{
	assert(thisBounds != NULL && otherBounds != NULL);

	Collision resultingCollision = { .isColliding = false, .delta = {1.0f, 1.0f}, .intersect = {1.0f, 1.0f} };

	Coord2D thisSize = boundsGetDimensions(thisBounds);
	Coord2D thisCenter = boundsGetCenter(thisBounds);
	Coord2D thisHalfSize = { .x = thisSize.x / 2, .y = thisSize.y / 2 };

	Coord2D otherSize = boundsGetDimensions(otherBounds);
	Coord2D otherCenter = boundsGetCenter(otherBounds);
	Coord2D otherHalfSize = { .x = otherSize.x / 2, .y = otherSize.y / 2 };

	float deltaX = thisCenter.x - otherCenter.x;
	float deltaY = thisCenter.y - otherCenter.y;

	float intersectX = (float)fabs(deltaX) - (thisHalfSize.x + otherHalfSize.x);
	float intersectY = (float)fabs(deltaY) - (thisHalfSize.y + otherHalfSize.y);

	resultingCollision.delta.x = deltaX;
	resultingCollision.delta.y = deltaY;
	resultingCollision.intersect.x = intersectX;
	resultingCollision.intersect.y = intersectY;

	if (intersectX < 0.0f && intersectY < 0.0f)
	{
		resultingCollision.isColliding = true;
	}
	return resultingCollision;
}
