#ifndef MAP_H
#define MAP_H

/**
 * @file Map.h
 * @brief Tile map storage, door state, map serialization, and minimap drawing API.
 */

#include <stdbool.h>
#include "Utilities/Deffinitions.h"
#include "Graphics.h"

/** @brief Default number of rows used by the built-in map. */
#define DEFAULT_MAP_ROWS 20

/** @brief Default number of columns used by the built-in map. */
#define DEFAULT_MAP_COLS 20

/** @brief Minimum map height accepted by resize and load operations. */
#define MAP_MIN_ROWS 4

/** @brief Minimum map width accepted by resize and load operations. */
#define MAP_MIN_COLS 4

/** @brief Maximum map height accepted by resize and load operations. */
#define MAP_MAX_ROWS 128

/** @brief Maximum map width accepted by resize and load operations. */
#define MAP_MAX_COLS 128

/** @brief Backward-compatible alias for the default row count. */
#define MAP_NUMBER_ROWS DEFAULT_MAP_ROWS

/** @brief Backward-compatible alias for the default column count. */
#define MAP_NUMBER_COLS DEFAULT_MAP_COLS

/** @brief Sentinel value meaning floor or ceiling should use the default color. */
#define MAP_DEFAULT_SURFACE_TEXTURE_INDEX -1

/** @brief Map tile value used to mark doors. */
#define MAP_DOOR_TILE -1

/** @brief Default map file path used by the game and editor. */
#define DEFAULT_MAP_FILE_PATH "./resources/maps/map_01.txt"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Checks whether a world-space point is blocked by a wall or closed door.
 *
 * @param x World x coordinate.
 * @param y World y coordinate.
 * @return true when the point is outside the map or inside a blocking tile.
 */
bool MapHasWallAt(float x, float y);

/**
 * @brief Checks whether a world-space point lies inside the map bounds.
 *
 * @param x World x coordinate.
 * @param y World y coordinate.
 * @return true when the point is inside the current map rectangle.
 */
bool IsInsideMap(float x, float y);

/**
 * @brief Reads one tile value from the current map.
 *
 * @param x Map row.
 * @param y Map column.
 * @return Tile value, or 0 when coordinates are invalid.
 */
int GetMap(int x, int y);

/**
 * @brief Writes one tile value into the current map.
 *
 * @param x Map row.
 * @param y Map column.
 * @param value Tile value to store.
 * @return true when coordinates are valid and the tile was written.
 */
bool SetMap(int x, int y, int value);

/**
 * @brief Gets the current map row count.
 *
 * @return Number of rows.
 */
int GetMapRows(void);

/**
 * @brief Gets the current map column count.
 *
 * @return Number of columns.
 */
int GetMapCols(void);

/**
 * @brief Resizes the map while preserving overlapping existing tiles.
 *
 * @param rows Requested row count, clamped to supported limits.
 * @param cols Requested column count, clamped to supported limits.
 * @return true when allocation succeeds.
 */
bool ResizeMap(int rows, int cols);

/**
 * @brief Checks whether a tile value represents a door.
 *
 * @param value Tile value to test.
 * @return true when value is MAP_DOOR_TILE.
 */
bool IsDoorTileValue(int value);

/**
 * @brief Checks whether a map cell currently contains a door.
 *
 * @param row Map row.
 * @param col Map column.
 * @return true when the cell is a door.
 */
bool IsDoorAt(int row, int col);

/**
 * @brief Gets the normalized open amount for a door.
 *
 * @param row Door row.
 * @param col Door column.
 * @return Door open amount in the [0, 1] range.
 */
float GetDoorOpenAmount(int row, int col);

/**
 * @brief Gets the texture index used to render doors.
 *
 * @return Door texture index.
 */
int GetDoorTextureIndex(void);

/**
 * @brief Updates automatic door opening and closing based on player distance.
 *
 * @param playerX Player world x coordinate.
 * @param playerY Player world y coordinate.
 * @param deltaTime Elapsed frame time in seconds.
 */
void UpdateMapDoors(float playerX, float playerY, float deltaTime);

/**
 * @brief Gets the active floor texture index.
 *
 * @return Texture index, or MAP_DEFAULT_SURFACE_TEXTURE_INDEX for flat color.
 */
int GetFloorTextureIndex(void);

/**
 * @brief Gets the active ceiling texture index.
 *
 * @return Texture index, or MAP_DEFAULT_SURFACE_TEXTURE_INDEX for flat color.
 */
int GetCeilingTextureIndex(void);

/**
 * @brief Sets the floor texture index.
 *
 * @param textureIndex Texture index, or MAP_DEFAULT_SURFACE_TEXTURE_INDEX.
 */
void SetFloorTextureIndex(int textureIndex);

/**
 * @brief Sets the ceiling texture index.
 *
 * @param textureIndex Texture index, or MAP_DEFAULT_SURFACE_TEXTURE_INDEX.
 */
void SetCeilingTextureIndex(int textureIndex);

/**
 * @brief Clears the map interior, preserves the border, and removes sprites.
 */
void ClearMap(void);

/**
 * @brief Loads map dimensions, surface textures, tiles, and sprites from a file.
 *
 * @param filePath Path to the map file.
 * @return true when the map was loaded.
 */
bool LoadMapFromFile(const char* filePath);

/**
 * @brief Saves map dimensions, surface textures, tiles, and sprites to a file.
 *
 * @param filePath Path to the map file.
 * @return true when the map was saved.
 */
bool SaveMapToFile(const char* filePath);

/**
 * @brief Draws the map walls into the minimap overlay.
 */
void RenderMapGrid(void);

#ifdef __cplusplus
}
#endif

#endif  // !MAP_H
