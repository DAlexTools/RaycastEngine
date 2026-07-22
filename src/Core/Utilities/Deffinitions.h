#pragma once

/**
 * @file Deffinitions.h
 * @brief Shared math, rendering, texture, and color constants.
 */

#include <stdint.h>
#include <math.h>

/** @brief Pi constant used by angle and ray calculations. */
#define PI 3.14159265

/** @brief Two pi radians, used for angle normalization. */
#define DOUBLE_PI 6.28318530

/** @brief Target simulation frame rate. */
#define FPS 30

/** @brief Target frame duration in milliseconds. */
#define FRAME_TIME_LENGTH (1000 / FPS)

/** @brief Horizontal field of view angle in radians. */
#define FOV_ANGLE (60 * (PI / 120))

/** @brief Width and height of one map tile in world pixels. */
#define TILE_SIZE 64

/** @brief Number of texture slots loaded from the built-in texture list. */
#define NUM_BUILTIN_TEXTURES 14

/** @brief Maximum number of wall/floor/ceiling textures available at runtime. */
#define MAX_TEXTURES 256

/** @brief Maximum number of sprite textures available at runtime. */
#define MAX_SPRITE_TEXTURES 256

/** @brief Backward-compatible alias for the runtime texture limit. */
#define NUM_TEXTURES MAX_TEXTURES

/** @brief Scale applied when drawing the minimap overlay. */
#define MINIMAP_SCALE_FACTOR 0.25

/** @brief Logical render width in pixels. */
#define WINDOW_WIDTH 1280

/** @brief Logical render height in pixels. */
#define WINDOW_HEIGHT 800

/** @brief Number of rays cast per frame, one per screen column. */
#define NUM_RAYS WINDOW_WIDTH

/** @brief Distance from the player to the projection plane in screen space. */
#define DIST_PROJ_PLANE ((WINDOW_WIDTH / 2) / tan(FOV_ANGLE / 2))

/**
 * @brief Packed 32-bit RGBA color value used by the software color buffer.
 */
typedef uint32_t color_t;

/** @name Opaque colors in the renderer's packed color format. */
/** @{ */
#define BLACK_COLOR 0xFF000000
#define WHITE_COLOR 0xFFFFFFFF
#define RED_COLOR 0xFF0000FF
#define DARK_GRAY_COLOR 0xFF444444
#define MEDIUM_GRAY_COLOR 0xFF888888
#define GREEN_COLOR 0xFF00FF00
#define BLUE_COLOR 0xFFFF0000
#define YELLOW_COLOR 0xFF00FFFF
#define CYAN_COLOR 0xFFFFFF00
#define MAGENTA_COLOR 0xFFFF00FF
#define ORANGE_COLOR 0xFF007FFF
#define PURPLE_COLOR 0xFF800080
#define PINK_COLOR 0xFFFFC0CB
#define BROWN_COLOR 0xFF8B4513
#define LIGHT_GRAY_COLOR 0xFFBBBBBB
#define DARK_RED_COLOR 0xFF8B0000
#define DARK_GREEN_COLOR 0xFF006400
#define DARK_BLUE_COLOR 0xFF00008B
#define LIGHT_BLUE_COLOR 0xFFFFE0B2
#define LIGHT_GREEN_COLOR 0xFF90EE90
#define LIGHT_YELLOW_COLOR 0xFFFFFFE0
#define LIGHT_CYAN_COLOR 0xFFE0FFFF
#define LIGHT_MAGENTA_COLOR 0xFFFFE0FF
#define LIGHT_ORANGE_COLOR 0xFFFFE4B5
#define LIGHT_PURPLE_COLOR 0xFFE6E6FA
#define LIGHT_PINK_COLOR 0xFFFFB6C1
#define LIGHT_BROWN_COLOR 0xFFD2B48C
/** @} */

/** @name Fully transparent colors. */
/** @{ */
#define FULLY_TRANSPARENT_RED_COLOR 0x00FF0000
#define FULLY_TRANSPARENT_GREEN_COLOR 0x0000FF00
#define FULLY_TRANSPARENT_BLUE_COLOR 0x000000FF
#define FULLY_TRANSPARENT_BLACK 0x00000000
/** @} */

/** @name Semi-transparent colors. */
/** @{ */
#define TRANSPARENT_BLACK_COLOR 0x80000000
#define TRANSPARENT_WHITE_COLOR 0x80FFFFFF
#define TRANSPARENT_RED_COLOR 0x80FF0000
#define TRANSPARENT_GREEN_COLOR 0x8000FF00
#define TRANSPARENT_BLUE_COLOR 0x800000FF
#define TRANSPARENT_YELLOW_COLOR 0x80FFFF00
#define TRANSPARENT_CYAN_COLOR 0x80FFFFFF
#define TRANSPARENT_MAGENTA_COLOR 0x80FF00FF
#define TRANSPARENT_ORANGE_COLOR 0x80FFA500
#define TRANSPARENT_PURPLE_COLOR 0x80800080
#define TRANSPARENT_PINK_COLOR 0x80FFC0CB
#define TRANSPARENT_BROWN_COLOR 0x808B4513
/** @} */
