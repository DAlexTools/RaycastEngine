#ifndef PLAYER_H
#define PLAYER_H

/**
 * @file Player.h
 * @brief Player state and minimap/player movement API.
 */

#include "Utilities/Deffinitions.h"
#include "Map.h"
#include "Graphics.h"
#include <float.h>

/**
 * @brief Runtime player state used by movement, ray casting, and rendering.
 */
typedef struct 
{
    int turnDirection;       /**< -1 turns left, +1 turns right, 0 stops turning. */
    int walkDiretion;        /**< -1 moves backward, +1 moves forward, 0 stops walking. */
    int strafeDirection;     /**< -1 strafes left, +1 strafes right, 0 stops strafing. */

    float x;                 /**< Player center x coordinate in world space. */
    float y;                 /**< Player center y coordinate in world space. */
    float width;             /**< Player marker width in world pixels. */
    float height;            /**< Player marker height in world pixels. */
    float rotationAngle;     /**< Facing direction in radians. */
    float walkSpeed;         /**< Movement speed in world pixels per second. */
    float turnSpeed;         /**< Rotation speed in radians per second. */
    float mouseSensitivity;  /**< Multiplier applied to relative mouse movement. */
    float sprintMultiply;    /**< Extra movement speed applied while sprinting. */

} player_t;

/** @brief Global player instance used by the game loop and renderers. */
extern player_t player;

/**
 * @brief Updates player rotation and position using current input directions.
 *
 * @param deltaTime Elapsed frame time in seconds.
 */
void MovePlayer(float deltaTime);

/**
 * @brief Draws the player marker on the minimap.
 */
void RenderMapPlayer(void);

#endif  // !PLAYER_H
