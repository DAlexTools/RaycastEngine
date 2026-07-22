#ifndef WALL_H
#define WALL_H

/**
 * @file Wall.h
 * @brief Wall, floor, and ceiling projection rendering API.
 */

#include "Utilities/Deffinitions.h"
#include "Ray.h"
#include "Player.h"
#include "Graphics.h"
#include "Textures.h"
#include "upng.h"

/**
 * @brief Scales RGB channels of a color while preserving alpha.
 *
 * @param color Packed color value modified in place.
 * @param factor Light intensity multiplier.
 */
void ChangeColorIntensity(color_t* color, float factor);

/**
 * @brief Renders floor and ceiling pixels for the current camera view.
 */
void RenderFloorAndCeilingProjection(void);

/**
 * @brief Renders wall and door slices from the current ray buffer.
 */
void RenderWallProjection(void);

#endif  // !WALL_H
