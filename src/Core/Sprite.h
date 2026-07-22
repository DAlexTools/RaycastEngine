#ifndef SPRITE_H
#define SPRITE_H

/**
 * @file Sprite.h
 * @brief Placed sprite storage, map serialization, and sprite rendering API.
 */

#include <stdbool.h>
#include <stdio.h>

#include "Utilities/Deffinitions.h"

/** @brief Maximum number of sprite instances that can be placed in a map. */
#define MAX_PLACED_SPRITES 1024

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Runtime data for one placed sprite instance.
 */
typedef struct
{
    float x;        /**< Sprite center x coordinate in world space. */
    float y;        /**< Sprite center y coordinate in world space. */
    int texture;    /**< Index into the sprite texture list. */
    float angle;    /**< Angle between the player view direction and the sprite. */
    float distance; /**< Distance from the player, used for depth sorting. */
    bool visible;   /**< true when the sprite is inside the current field of view. */
} sprite_t;

/**
 * @brief Clears placed sprites and restores the built-in default placements.
 */
void InitializeDefaultSprites(void);

/**
 * @brief Removes every placed sprite.
 */
void ClearSprites(void);

/**
 * @brief Gets the number of currently placed sprites.
 *
 * @return Active sprite count.
 */
int GetSpriteCount(void);

/**
 * @brief Gets one sprite by index.
 *
 * @param index Zero-based sprite index.
 * @return Pointer to sprite data, or NULL when index is invalid.
 */
const sprite_t* GetSprite(int index);

/**
 * @brief Finds a sprite occupying a map cell.
 *
 * @param row Map row.
 * @param col Map column.
 * @return Sprite index, or -1 when the cell is empty.
 */
int FindSpriteAtCell(int row, int col);

/**
 * @brief Adds a sprite at a world-space position.
 *
 * @param x Sprite center x coordinate.
 * @param y Sprite center y coordinate.
 * @param textureIndex Index into the sprite texture list.
 * @return true when the sprite was added.
 */
bool AddSprite(float x, float y, int textureIndex);

/**
 * @brief Adds a sprite centered in a map cell.
 *
 * @param row Map row.
 * @param col Map column.
 * @param textureIndex Index into the sprite texture list.
 * @return true when placement succeeds.
 */
bool AddSpriteAtCell(int row, int col, int textureIndex);

/**
 * @brief Removes the sprite occupying a map cell.
 *
 * @param row Map row.
 * @param col Map column.
 * @return true when a sprite was removed.
 */
bool RemoveSpriteAtCell(int row, int col);

/**
 * @brief Removes sprites outside the current map bounds or using invalid textures.
 *
 * @param rows Current map row count.
 * @param cols Current map column count.
 */
void RemoveSpritesOutsideMap(int rows, int cols);

/**
 * @brief Loads sprite placements from the sprite section of a map file.
 *
 * @param filePath Path to the map file.
 * @return true when a sprite section was found and loaded.
 */
bool LoadSpritesFromMapFile(const char* filePath);

/**
 * @brief Writes sprite placements to an already-open map file.
 *
 * @param file Output file handle.
 */
void SaveSpritesToFile(FILE* file);

/**
 * @brief Projects and draws visible sprites into the 3D view.
 */
void RenderSpriteProjection(void);

/**
 * @brief Draws sprite markers on the minimap overlay.
 */
void RenderMapSprites(void);

#ifdef __cplusplus
}
#endif

#endif  // !SPRITE_H
