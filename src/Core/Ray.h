#ifndef RAY_H
#define RAY_H

/**
 * @file Ray.h
 * @brief Ray casting state and API for wall and door projection.
 */

#include <stdbool.h>
#include "Utilities/Deffinitions.h"
#include "Player.h"

/**
 * @brief Collision data for one screen-column ray.
 */
typedef struct
{
    bool wasHitVertical;            /**< true when the primary hit was on a vertical grid line. */
    float rayAngle;                 /**< Ray direction in radians. */
    float wallHitX;                 /**< Primary hit x coordinate in world space. */
    float wallHitY;                 /**< Primary hit y coordinate in world space. */
    float distance;                 /**< Distance from the player to the primary hit. */
    int texture;                    /**< Map tile value or door marker for the primary hit. */
    int hitMapRow;                  /**< Map row of the primary hit. */
    int hitMapCol;                  /**< Map column of the primary hit. */
    bool hasThroughDoorHit;         /**< true when an open door exposed a farther wall hit. */
    bool throughDoorWasHitVertical; /**< true when the through-door hit was vertical. */
    float throughDoorHitX;          /**< Through-door hit x coordinate in world space. */
    float throughDoorHitY;          /**< Through-door hit y coordinate in world space. */
    float throughDoorDistance;      /**< Distance from the player to the through-door hit. */
    int throughDoorTexture;         /**< Map tile value for the through-door hit. */
    int throughDoorMapRow;          /**< Map row of the through-door hit. */
    int throughDoorMapCol;          /**< Map column of the through-door hit. */
} ray_t;

/** @brief One ray result per screen column. */
extern ray_t rays[NUM_RAYS];

/**
 * @brief Checks whether an angle points upward in map space.
 *
 * @param angle Angle in radians.
 * @return true when the ray faces upward.
 */
bool IsRayFacingUp(float angle);

/**
 * @brief Checks whether an angle points downward in map space.
 *
 * @param angle Angle in radians.
 * @return true when the ray faces downward.
 */
bool IsRayFacingDown(float angle);

/**
 * @brief Checks whether an angle points left in map space.
 *
 * @param angle Angle in radians.
 * @return true when the ray faces left.
 */
bool IsRayFacingLeft(float angle);

/**
 * @brief Checks whether an angle points right in map space.
 *
 * @param angle Angle in radians.
 * @return true when the ray faces right.
 */
bool IsRayFacingRight(float angle);

/**
 * @brief Casts all screen-column rays for the current player view.
 */
void CastAllRays(void);

/**
 * @brief Casts one ray and stores its result in the ray buffer.
 *
 * @param rayAngle Ray direction in radians.
 * @param stripId Screen column and index into rays.
 */
void CastRay(float rayAngle, int stripId);

/**
 * @brief Draws cast rays on the minimap overlay.
 */
void RenderMapRays(void);

#endif  // !RAY_H
