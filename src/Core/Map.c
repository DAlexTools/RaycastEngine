#include "Map.h"
#include "Textures.h"
#include "Sprite.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#define MAKE_DIRECTORY(path) _mkdir(path)
#else
#include <sys/stat.h>
#define MAKE_DIRECTORY(path) mkdir(path, 0755)
#endif

static const int DEFAULT_MAP[DEFAULT_MAP_ROWS][DEFAULT_MAP_COLS] = {
    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 7, 7, 7, 7},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 7, 7, 7},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7},
    {1, 0, 0, 0, 1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 7, 7, 0, 7},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 7, 7},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 8, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
    {1, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 2, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 0, 3, 3, 3, 3, 3, 3, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 6, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};

static int* map_01 = NULL;
static float* gDoorOpenAmounts = NULL;
static int gMapRows = 0;
static int gMapCols = 0;
static int gFloorTextureIndex = FLOOR_TEXTURE_INDEX;
static int gCeilingTextureIndex = CEILING_TEXTURE_INDEX;
static int gDoorTextureIndex = TEXTURE_WOOD;

#define DOOR_OPEN_DISTANCE (TILE_SIZE * 1.8f)
#define DOOR_OPEN_SPEED 2.4f
#define DOOR_CLOSE_SPEED 1.8f
#define DOOR_PASSABLE_OPEN_AMOUNT 0.85f

static int ClampInt(int value, int minValue, int maxValue)
{
    if (value < minValue)
    {
        return minValue;
    }
    if (value > maxValue)
    {
        return maxValue;
    }
    return value;
}

static int GetMapIndex(int row, int col, int cols)
{
    return row * cols + col;
}

static bool IsBorderCell(int row, int col, int rows, int cols)
{
    return row == 0 || row == rows - 1 || col == 0 || col == cols - 1;
}

static int ClampTileValue(int value)
{
    if (value == MAP_DOOR_TILE)
    {
        return MAP_DOOR_TILE;
    }

    if (value < 0)
    {
        return 0;
    }

    int textureCount = GetTextureCount();
    if (textureCount <= 0)
    {
        textureCount = NUM_BUILTIN_TEXTURES;
    }

    if (value > textureCount)
    {
        return textureCount;
    }

    return value;
}

static int ClampSurfaceTextureIndex(int textureIndex)
{
    if (textureIndex < 0)
    {
        return MAP_DEFAULT_SURFACE_TEXTURE_INDEX;
    }

    int textureCount = GetTextureCount();
    if (textureCount <= 0)
    {
        textureCount = NUM_BUILTIN_TEXTURES;
    }

    if (textureIndex >= textureCount)
    {
        return textureCount - 1;
    }

    return textureIndex;
}

static bool EnsureDoorStateAllocated(void)
{
    if (map_01 == NULL)
    {
        return false;
    }

    if (gDoorOpenAmounts != NULL)
    {
        return true;
    }

    gDoorOpenAmounts = (float*)calloc((size_t)gMapRows * (size_t)gMapCols, sizeof(float));
    return gDoorOpenAmounts != NULL;
}

static void ResetDoorStatesForNonDoors(void)
{
    if (map_01 == NULL || gDoorOpenAmounts == NULL)
    {
        return;
    }

    for (int row = 0; row < gMapRows; row++)
    {
        for (int col = 0; col < gMapCols; col++)
        {
            int index = GetMapIndex(row, col, gMapCols);
            if (map_01[index] != MAP_DOOR_TILE)
            {
                gDoorOpenAmounts[index] = 0.0f;
            }
        }
    }
}

static void FillMapWithBorder(int* mapData, int rows, int cols)
{
    for (int row = 0; row < rows; row++)
    {
        for (int col = 0; col < cols; col++)
        {
            mapData[GetMapIndex(row, col, cols)] = IsBorderCell(row, col, rows, cols) ? 1 : 0;
        }
    }
}

static void KeepBoundaryClosed(void)
{
    if (map_01 == NULL)
    {
        return;
    }

    for (int row = 0; row < gMapRows; row++)
    {
        for (int col = 0; col < gMapCols; col++)
        {
            if (IsBorderCell(row, col, gMapRows, gMapCols))
            {
                map_01[GetMapIndex(row, col, gMapCols)] = 1;
            }
        }
    }
}

static bool InitializeDefaultMap(void)
{
    int* newMap = (int*)malloc((size_t)DEFAULT_MAP_ROWS * (size_t)DEFAULT_MAP_COLS * sizeof(int));
    if (newMap == NULL)
    {
        return false;
    }

    float* newDoorOpenAmounts = (float*)calloc((size_t)DEFAULT_MAP_ROWS * (size_t)DEFAULT_MAP_COLS, sizeof(float));
    if (newDoorOpenAmounts == NULL)
    {
        free(newMap);
        return false;
    }

    for (int row = 0; row < DEFAULT_MAP_ROWS; row++)
    {
        for (int col = 0; col < DEFAULT_MAP_COLS; col++)
        {
            newMap[GetMapIndex(row, col, DEFAULT_MAP_COLS)] = DEFAULT_MAP[row][col];
        }
    }

    free(map_01);
    free(gDoorOpenAmounts);
    map_01 = newMap;
    gDoorOpenAmounts = newDoorOpenAmounts;
    gMapRows = DEFAULT_MAP_ROWS;
    gMapCols = DEFAULT_MAP_COLS;
    return true;
}

static bool EnsureMapAllocated(void)
{
    if (map_01 != NULL)
    {
        return true;
    }

    return InitializeDefaultMap();
}

static bool ReadAllMapValues(FILE* file, int** values, int* count)
{
    int capacity = 512;
    int valueCount = 0;
    int* buffer = (int*)malloc((size_t)capacity * sizeof(int));
    if (buffer == NULL)
    {
        return false;
    }

    int value = 0;
    while (fscanf(file, "%d", &value) == 1)
    {
        if (valueCount >= capacity)
        {
            capacity *= 2;
            int* resized = (int*)realloc(buffer, (size_t)capacity * sizeof(int));
            if (resized == NULL)
            {
                free(buffer);
                return false;
            }
            buffer = resized;
        }

        buffer[valueCount++] = value;
    }

    *values = buffer;
    *count = valueCount;
    return true;
}

bool MapHasWallAt(float x, float y)
{
    if (!EnsureMapAllocated())
    {
        return true;
    }

    if (x < 0 || x >= gMapCols * TILE_SIZE || y < 0 || y >= gMapRows * TILE_SIZE)
    {
        return true;
    }

    int mapGridIndexX = (int)floor(x / TILE_SIZE);
    int mapGridIndexY = (int)floor(y / TILE_SIZE);

    int tile = GetMap(mapGridIndexY, mapGridIndexX);
    if (tile == MAP_DOOR_TILE)
    {
        int index = GetMapIndex(mapGridIndexY, mapGridIndexX, gMapCols);
        float openAmount = gDoorOpenAmounts != NULL ? gDoorOpenAmounts[index] : 0.0f;
        return openAmount < DOOR_PASSABLE_OPEN_AMOUNT;
    }

    return tile != 0;
}

bool IsInsideMap(float x, float y)
{
    if (!EnsureMapAllocated())
    {
        return false;
    }

    return (x >= 0 && x <= gMapCols * TILE_SIZE && y >= 0 && y <= gMapRows * TILE_SIZE);
}

void RenderMapGrid(void)
{
    if (!EnsureMapAllocated())
    {
        return;
    }

    for (int row = 0; row < gMapRows; row++)
    {
        for (int col = 0; col < gMapCols; col++)
        {
            int tileX = col * TILE_SIZE;
            int tileY = row * TILE_SIZE;
            color_t tileColor = GetMap(row, col) != 0 ? WHITE_COLOR : FULLY_TRANSPARENT_BLACK;
            int minimapX = (int)(tileX * MINIMAP_SCALE_FACTOR);
            int minimapY = (int)(tileY * MINIMAP_SCALE_FACTOR);
            int minimapTileSize = (int)(TILE_SIZE * MINIMAP_SCALE_FACTOR);

            DrawRect(
                minimapX,
                minimapY,
                minimapTileSize,
                minimapTileSize,
                tileColor);
        }
    }
}

int GetMap(int x, int y)
{
    if (!EnsureMapAllocated())
    {
        return 0;
    }

    if (x < 0 || x >= gMapRows || y < 0 || y >= gMapCols)
    {
        return 0;
    }

    return map_01[GetMapIndex(x, y, gMapCols)];
}

bool SetMap(int x, int y, int value)
{
    if (!EnsureMapAllocated())
    {
        return false;
    }

    if (x < 0 || x >= gMapRows || y < 0 || y >= gMapCols)
    {
        return false;
    }

    EnsureDoorStateAllocated();

    int index = GetMapIndex(x, y, gMapCols);
    int previousTile = map_01[index];
    int newTile = ClampTileValue(value);
    map_01[index] = newTile;

    if (newTile != 0)
    {
        RemoveSpriteAtCell(x, y);
    }

    if (gDoorOpenAmounts != NULL && (newTile != MAP_DOOR_TILE || previousTile != MAP_DOOR_TILE))
    {
        gDoorOpenAmounts[index] = 0.0f;
    }

    return true;
}

int GetMapRows(void)
{
    EnsureMapAllocated();
    return gMapRows;
}

int GetMapCols(void)
{
    EnsureMapAllocated();
    return gMapCols;
}

int GetFloorTextureIndex(void)
{
    gFloorTextureIndex = ClampSurfaceTextureIndex(gFloorTextureIndex);
    return gFloorTextureIndex;
}

int GetCeilingTextureIndex(void)
{
    gCeilingTextureIndex = ClampSurfaceTextureIndex(gCeilingTextureIndex);
    return gCeilingTextureIndex;
}

void SetFloorTextureIndex(int textureIndex)
{
    gFloorTextureIndex = ClampSurfaceTextureIndex(textureIndex);
}

void SetCeilingTextureIndex(int textureIndex)
{
    gCeilingTextureIndex = ClampSurfaceTextureIndex(textureIndex);
}

bool ResizeMap(int rows, int cols)
{
    rows = ClampInt(rows, MAP_MIN_ROWS, MAP_MAX_ROWS);
    cols = ClampInt(cols, MAP_MIN_COLS, MAP_MAX_COLS);

    int* newMap = (int*)malloc((size_t)rows * (size_t)cols * sizeof(int));
    if (newMap == NULL)
    {
        return false;
    }

    float* newDoorOpenAmounts = (float*)calloc((size_t)rows * (size_t)cols, sizeof(float));
    if (newDoorOpenAmounts == NULL)
    {
        free(newMap);
        return false;
    }

    FillMapWithBorder(newMap, rows, cols);

    if (map_01 != NULL)
    {
        int copyRows = rows < gMapRows ? rows : gMapRows;
        int copyCols = cols < gMapCols ? cols : gMapCols;

        for (int row = 0; row < copyRows; row++)
        {
            for (int col = 0; col < copyCols; col++)
            {
                int oldIndex = GetMapIndex(row, col, gMapCols);
                int newIndex = GetMapIndex(row, col, cols);
                newMap[newIndex] = map_01[oldIndex];
                if (map_01[oldIndex] == MAP_DOOR_TILE && gDoorOpenAmounts != NULL)
                {
                    newDoorOpenAmounts[newIndex] = gDoorOpenAmounts[oldIndex];
                }
            }
        }
    }

    free(map_01);
    free(gDoorOpenAmounts);
    map_01 = newMap;
    gDoorOpenAmounts = newDoorOpenAmounts;
    gMapRows = rows;
    gMapCols = cols;
    KeepBoundaryClosed();
    ResetDoorStatesForNonDoors();
    RemoveSpritesOutsideMap(gMapRows, gMapCols);
    return true;
}

void ClearMap(void)
{
    if (!EnsureMapAllocated())
    {
        return;
    }

    FillMapWithBorder(map_01, gMapRows, gMapCols);
    if (gDoorOpenAmounts != NULL)
    {
        memset(gDoorOpenAmounts, 0, (size_t)gMapRows * (size_t)gMapCols * sizeof(float));
    }
    ClearSprites();
}

bool IsDoorTileValue(int value)
{
    return value == MAP_DOOR_TILE;
}

bool IsDoorAt(int row, int col)
{
    if (!EnsureMapAllocated())
    {
        return false;
    }

    if (row < 0 || row >= gMapRows || col < 0 || col >= gMapCols)
    {
        return false;
    }

    return map_01[GetMapIndex(row, col, gMapCols)] == MAP_DOOR_TILE;
}

float GetDoorOpenAmount(int row, int col)
{
    if (!IsDoorAt(row, col) || gDoorOpenAmounts == NULL)
    {
        return 0.0f;
    }

    return gDoorOpenAmounts[GetMapIndex(row, col, gMapCols)];
}

int GetDoorTextureIndex(void)
{
    gDoorTextureIndex = ClampSurfaceTextureIndex(gDoorTextureIndex);
    if (gDoorTextureIndex == MAP_DEFAULT_SURFACE_TEXTURE_INDEX)
    {
        return TEXTURE_WOOD;
    }
    return gDoorTextureIndex;
}

void UpdateMapDoors(float playerX, float playerY, float deltaTime)
{
    if (!EnsureMapAllocated() || !EnsureDoorStateAllocated())
    {
        return;
    }

    if (deltaTime < 0.0f)
    {
        deltaTime = 0.0f;
    }

    float openDistanceSquared = DOOR_OPEN_DISTANCE * DOOR_OPEN_DISTANCE;
    for (int row = 0; row < gMapRows; row++)
    {
        for (int col = 0; col < gMapCols; col++)
        {
            int index = GetMapIndex(row, col, gMapCols);
            if (map_01[index] != MAP_DOOR_TILE)
            {
                gDoorOpenAmounts[index] = 0.0f;
                continue;
            }

            float doorCenterX = (col + 0.5f) * TILE_SIZE;
            float doorCenterY = (row + 0.5f) * TILE_SIZE;
            float dx = doorCenterX - playerX;
            float dy = doorCenterY - playerY;
            bool shouldOpen = dx * dx + dy * dy <= openDistanceSquared;
            float step = (shouldOpen ? DOOR_OPEN_SPEED : DOOR_CLOSE_SPEED) * deltaTime;

            if (shouldOpen)
            {
                gDoorOpenAmounts[index] += step;
                if (gDoorOpenAmounts[index] > 1.0f)
                {
                    gDoorOpenAmounts[index] = 1.0f;
                }
            }
            else
            {
                gDoorOpenAmounts[index] -= step;
                if (gDoorOpenAmounts[index] < 0.0f)
                {
                    gDoorOpenAmounts[index] = 0.0f;
                }
            }
        }
    }
}

bool LoadMapFromFile(const char* filePath)
{
    FILE* file = fopen(filePath, "r");
    if (file == NULL)
    {
        EnsureMapAllocated();
        InitializeDefaultSprites();
        return false;
    }

    int* values = NULL;
    int valueCount = 0;
    bool readOk = ReadAllMapValues(file, &values, &valueCount);
    fclose(file);

    if (!readOk || values == NULL)
    {
        free(values);
        EnsureMapAllocated();
        return false;
    }

    int rows = DEFAULT_MAP_ROWS;
    int cols = DEFAULT_MAP_COLS;
    int floorTextureTile = FLOOR_TEXTURE_INDEX + 1;
    int ceilingTextureTile = CEILING_TEXTURE_INDEX + 1;
    int firstTileIndex = 0;

    if (valueCount >= 2)
    {
        int headerRows = values[0];
        int headerCols = values[1];
        bool headerSizeIsValid =
            headerRows >= MAP_MIN_ROWS &&
            headerRows <= MAP_MAX_ROWS &&
            headerCols >= MAP_MIN_COLS &&
            headerCols <= MAP_MAX_COLS;

        if (headerSizeIsValid && valueCount - 4 == headerRows * headerCols)
        {
            rows = headerRows;
            cols = headerCols;
            floorTextureTile = values[2];
            ceilingTextureTile = values[3];
            firstTileIndex = 4;
        }
        else if (headerSizeIsValid && valueCount - 2 == headerRows * headerCols)
        {
            rows = headerRows;
            cols = headerCols;
            firstTileIndex = 2;
        }
        else if (valueCount == DEFAULT_MAP_ROWS * DEFAULT_MAP_COLS)
        {
            rows = DEFAULT_MAP_ROWS;
            cols = DEFAULT_MAP_COLS;
            firstTileIndex = 0;
        }
        else
        {
            free(values);
            EnsureMapAllocated();
            return false;
        }
    }

    if (firstTileIndex + rows * cols != valueCount)
    {
        free(values);
        EnsureMapAllocated();
        return false;
    }

    if (!ResizeMap(rows, cols))
    {
        free(values);
        EnsureMapAllocated();
        return false;
    }

    SetFloorTextureIndex(floorTextureTile <= 0 ? MAP_DEFAULT_SURFACE_TEXTURE_INDEX : floorTextureTile - 1);
    SetCeilingTextureIndex(ceilingTextureTile <= 0 ? MAP_DEFAULT_SURFACE_TEXTURE_INDEX : ceilingTextureTile - 1);

    for (int row = 0; row < rows; row++)
    {
        for (int col = 0; col < cols; col++)
        {
            SetMap(row, col, values[firstTileIndex + GetMapIndex(row, col, cols)]);
        }
    }

    free(values);
    if (!LoadSpritesFromMapFile(filePath))
    {
        InitializeDefaultSprites();
    }
    return true;
}

bool SaveMapToFile(const char* filePath)
{
    if (!EnsureMapAllocated())
    {
        return false;
    }

    MAKE_DIRECTORY("resources");
    MAKE_DIRECTORY("resources/maps");

    FILE* file = fopen(filePath, "w");
    if (file == NULL)
    {
        return false;
    }

    int floorTextureTile = GetFloorTextureIndex() < 0 ? 0 : GetFloorTextureIndex() + 1;
    int ceilingTextureTile = GetCeilingTextureIndex() < 0 ? 0 : GetCeilingTextureIndex() + 1;
    fprintf(file, "%d %d %d %d\n", gMapRows, gMapCols, floorTextureTile, ceilingTextureTile);
    for (int row = 0; row < gMapRows; row++)
    {
        for (int col = 0; col < gMapCols; col++)
        {
            fprintf(file, "%d%s", GetMap(row, col), col == gMapCols - 1 ? "" : " ");
        }
        fprintf(file, "\n");
    }

    SaveSpritesToFile(file);

    fclose(file);
    return true;
}
