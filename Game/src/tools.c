#include "tools.h"
#include "math.h"


/// <summary>
/// Calculates a linear interpolation for the passed in parameters.
/// </summary>
/// <param name="startPos"></param>
/// <param name="endPos"></param>
/// <param name="percentToEnd"></param>
/// <returns>The calculated position based on the LERP.</returns>
Coord2D toolLerp(Coord2D startPos, Coord2D endPos, float percentToEnd)
{
    float newX = startPos.x + percentToEnd * (endPos.x - startPos.x);
    float newY = startPos.y + percentToEnd * (endPos.y - startPos.y);
    //Coord2D newPos = { .x = newX, .y = newY };
    return (Coord2D) { .x = newX, .y = newY };
}

/// <summary>
/// Clamps a float value between a minimum and maximum.
/// </summary>
/// <param name="value"></param>
/// <param name="min"></param>
/// <param name="max"></param>
/// <returns></returns>
float toolClampFloat(float value, float min, float max)
{
	const float temp = value < min ? min : value;
	return temp > max ? max : temp;
}

/// <param name="position1"></param>
/// <param name="position2"></param>
/// <returns>The distance between two 2D points.</returns>
float toolDistance(Coord2D position1, Coord2D position2)
{
    return sqrtf(powf(position2.x - position1.x, 2) + powf(position2.y - position1.y, 2));
}
