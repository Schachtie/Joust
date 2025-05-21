#pragma once

#include "baseTypes.h"


Coord2D toolLerp(Coord2D startPos, Coord2D endPos, float percentToEnd);
float toolClampFloat(float value, float min, float max);
float toolDistance(Coord2D position1, Coord2D position2);
