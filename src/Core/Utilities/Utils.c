#include "Utils.h"
#include "Deffinitions.h"
#include <math.h>
void NormalizeAngle(float* angle)
{
    *angle = remainder(*angle, DOUBLE_PI);
    if (*angle < 0) *angle = DOUBLE_PI + *angle;
}

float DistanceBetweenPoints(float x1, float y1, float x2, float y2)
{
    return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}
