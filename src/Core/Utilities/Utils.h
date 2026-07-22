#ifndef UTILS_H
#define UTILS_H

/**
 * @file Utils.h
 * @brief Math helper functions shared by movement, ray casting, and rendering.
 */

/**
 * @brief Wraps an angle into the [0, 2*pi) range.
 *
 * @param angle Angle in radians, modified in place.
 */
void NormalizeAngle(float* angle);

/**
 * @brief Computes the Euclidean distance between two points.
 *
 * @param x1 First point x coordinate.
 * @param y1 First point y coordinate.
 * @param x2 Second point x coordinate.
 * @param y2 Second point y coordinate.
 * @return Distance between the two points.
 */
float DistanceBetweenPoints(float x1, float y1, float x2, float y2);

#endif  // !UTILS_H
