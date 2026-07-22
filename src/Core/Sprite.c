#include "Sprite.h"
#include "Graphics.h"
#include "Player.h"
#include "Textures.h"
#include "Utilities/Utils.h"
#include "Ray.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

static sprite_t gSprites[MAX_PLACED_SPRITES] = {0};
static int gSpriteCount = 0;

static const sprite_t DEFAULT_SPRITES[] = {
    {.x = 640, .y = 630, .texture = 0},  // barrel
    {.x = 660, .y = 690, .texture = 0},  // barrel
    {.x = 250, .y = 600, .texture = 2},  // table
    {.x = 250, .y = 600, .texture = 1},  // light
    {.x = 300, .y = 400, .texture = 3}   // guard
};

static bool IsSpriteTextureIndexValid(int textureIndex)
{
    return textureIndex >= 0 && textureIndex < GetSpriteTextureCount();
}

static int GetSpriteCellRow(const sprite_t* sprite)
{
    return (int)floor(sprite->y / TILE_SIZE);
}

static int GetSpriteCellCol(const sprite_t* sprite)
{
    return (int)floor(sprite->x / TILE_SIZE);
}

static void RemoveSpriteAtIndex(int index)
{
    if (index < 0 || index >= gSpriteCount)
    {
        return;
    }

    if (index < gSpriteCount - 1)
    {
        memmove(
            &gSprites[index],
            &gSprites[index + 1],
            (size_t)(gSpriteCount - index - 1) * sizeof(sprite_t));
    }

    gSpriteCount--;
}

static bool IsTransparentSpriteColor(color_t color)
{
    return ((color >> 24) & 0xFF) == 0 || color == MAGENTA_COLOR;
}

void InitializeDefaultSprites(void)
{
    ClearSprites();

    int defaultCount = (int)(sizeof(DEFAULT_SPRITES) / sizeof(DEFAULT_SPRITES[0]));
    for (int i = 0; i < defaultCount; i++)
    {
        AddSprite(DEFAULT_SPRITES[i].x, DEFAULT_SPRITES[i].y, DEFAULT_SPRITES[i].texture);
    }
}

void ClearSprites(void)
{
    gSpriteCount = 0;
}

int GetSpriteCount(void)
{
    return gSpriteCount;
}

const sprite_t* GetSprite(int index)
{
    if (index < 0 || index >= gSpriteCount)
    {
        return NULL;
    }

    return &gSprites[index];
}

int FindSpriteAtCell(int row, int col)
{
    for (int i = 0; i < gSpriteCount; i++)
    {
        if (GetSpriteCellRow(&gSprites[i]) == row && GetSpriteCellCol(&gSprites[i]) == col)
        {
            return i;
        }
    }

    return -1;
}

bool AddSprite(float x, float y, int textureIndex)
{
    if (gSpriteCount >= MAX_PLACED_SPRITES || !IsSpriteTextureIndexValid(textureIndex))
    {
        return false;
    }

    gSprites[gSpriteCount].x = x;
    gSprites[gSpriteCount].y = y;
    gSprites[gSpriteCount].texture = textureIndex;
    gSprites[gSpriteCount].angle = 0.0f;
    gSprites[gSpriteCount].distance = 0.0f;
    gSprites[gSpriteCount].visible = false;
    gSpriteCount++;
    return true;
}

bool AddSpriteAtCell(int row, int col, int textureIndex)
{
    if (row < 0 || col < 0)
    {
        return false;
    }

    RemoveSpriteAtCell(row, col);
    return AddSprite((col + 0.5f) * TILE_SIZE, (row + 0.5f) * TILE_SIZE, textureIndex);
}

bool RemoveSpriteAtCell(int row, int col)
{
    int index = FindSpriteAtCell(row, col);
    if (index < 0)
    {
        return false;
    }

    RemoveSpriteAtIndex(index);
    return true;
}

void RemoveSpritesOutsideMap(int rows, int cols)
{
    for (int i = gSpriteCount - 1; i >= 0; i--)
    {
        int row = GetSpriteCellRow(&gSprites[i]);
        int col = GetSpriteCellCol(&gSprites[i]);
        if (row < 0 || row >= rows || col < 0 || col >= cols || !IsSpriteTextureIndexValid(gSprites[i].texture))
        {
            RemoveSpriteAtIndex(i);
        }
    }
}

bool LoadSpritesFromMapFile(const char* filePath)
{
    FILE* file = fopen(filePath, "r");
    if (file == NULL)
    {
        return false;
    }

    char line[256];
    int spriteCount = 0;
    bool foundSpriteSection = false;

    while (fgets(line, sizeof(line), file) != NULL)
    {
        if (sscanf(line, " sprites %d", &spriteCount) == 1 || sscanf(line, " SPRITES %d", &spriteCount) == 1)
        {
            foundSpriteSection = true;
            break;
        }
    }

    if (!foundSpriteSection)
    {
        fclose(file);
        return false;
    }

    ClearSprites();
    if (spriteCount < 0)
    {
        spriteCount = 0;
    }

    int loadedSprites = 0;
    while (loadedSprites < spriteCount && fgets(line, sizeof(line), file) != NULL)
    {
        int row = 0;
        int col = 0;
        int textureTile = 0;
        if (sscanf(line, " %d %d %d", &row, &col, &textureTile) == 3 && textureTile > 0)
        {
            AddSpriteAtCell(row, col, textureTile - 1);
        }
        loadedSprites++;
    }

    fclose(file);
    return true;
}

void SaveSpritesToFile(FILE* file)
{
    if (file == NULL)
    {
        return;
    }

    int validSpriteCount = 0;
    for (int i = 0; i < gSpriteCount; i++)
    {
        if (IsSpriteTextureIndexValid(gSprites[i].texture))
        {
            validSpriteCount++;
        }
    }

    fprintf(file, "sprites %d\n", validSpriteCount);
    for (int i = 0; i < gSpriteCount; i++)
    {
        if (!IsSpriteTextureIndexValid(gSprites[i].texture))
        {
            continue;
        }

        fprintf(
            file,
            "%d %d %d\n",
            GetSpriteCellRow(&gSprites[i]),
            GetSpriteCellCol(&gSprites[i]),
            gSprites[i].texture + 1);
    }
}

void RenderSpriteProjection(void)
{
    sprite_t visibleSprites[MAX_PLACED_SPRITES];
    int numVisibleSprites = 0;

    for (int i = 0; i < gSpriteCount; i++)
    {
        if (!IsSpriteTextureIndexValid(gSprites[i].texture))
        {
            continue;
        }

        float angleSpritePlayer = player.rotationAngle - atan2f(gSprites[i].y - player.y, gSprites[i].x - player.x);

        if (angleSpritePlayer > PI) angleSpritePlayer -= DOUBLE_PI;
        if (angleSpritePlayer < -PI) angleSpritePlayer += DOUBLE_PI;
        angleSpritePlayer = fabsf(angleSpritePlayer);

        const float EPSILON = 0.2f;
        if (angleSpritePlayer < (FOV_ANGLE / 2.0f) + EPSILON)
        {
            gSprites[i].visible = true;
            gSprites[i].angle = angleSpritePlayer;
            gSprites[i].distance = DistanceBetweenPoints(gSprites[i].x, gSprites[i].y, player.x, player.y);
            visibleSprites[numVisibleSprites] = gSprites[i];
            numVisibleSprites++;
        }
        else
        {
            gSprites[i].visible = false;
        }
    }

    for (int i = 0; i < numVisibleSprites - 1; i++)
    {
        for (int j = i + 1; j < numVisibleSprites; j++)
        {
            if (visibleSprites[i].distance < visibleSprites[j].distance)
            {
                sprite_t temp = visibleSprites[i];
                visibleSprites[i] = visibleSprites[j];
                visibleSprites[j] = temp;
            }
        }
    }

    for (int i = 0; i < numVisibleSprites; i++)
    {
        sprite_t sprite = visibleSprites[i];
        const texture_data_t* texture = GetSpriteTextureData(sprite.texture);
        if (texture == NULL || texture->pixels == NULL || texture->width <= 0 || texture->height <= 0)
        {
            continue;
        }

        float perpDistance = sprite.distance * cosf(sprite.angle);
        if (perpDistance <= 0.0f)
        {
            continue;
        }

        float spriteHeight = ((float)TILE_SIZE / perpDistance) * (float)DIST_PROJ_PLANE;
        float spriteWidth = spriteHeight;

        float spriteTopY = (WINDOW_HEIGHT / 2.0f) - (spriteHeight / 2.0f);
        spriteTopY = (spriteTopY < 0.0f) ? 0.0f : spriteTopY;

        float spriteBottomY = (WINDOW_HEIGHT / 2.0f) + (spriteHeight / 2.0f);
        spriteBottomY = (spriteBottomY > WINDOW_HEIGHT) ? WINDOW_HEIGHT : spriteBottomY;

        float spriteAngle = atan2f(sprite.y - player.y, sprite.x - player.x) - player.rotationAngle;
        if (spriteAngle > PI) spriteAngle -= DOUBLE_PI;
        if (spriteAngle < -PI) spriteAngle += DOUBLE_PI;

        float spriteScreenPosX = tanf(spriteAngle) * (float)DIST_PROJ_PLANE;
        float spriteLeftX = (WINDOW_WIDTH / 2.0f) + spriteScreenPosX - (spriteWidth / 2.0f);
        float spriteRightX = spriteLeftX + spriteWidth;

        int startX = (int)spriteLeftX;
        int endX = (int)spriteRightX;
        if (startX < 0)
        {
            startX = 0;
        }
        if (endX > WINDOW_WIDTH)
        {
            endX = WINDOW_WIDTH;
        }

        for (int x = startX; x < endX; x++)
        {
            float texelWidth = texture->width / spriteWidth;
            int textureOffsetX = (int)((x - spriteLeftX) * texelWidth);
            if (textureOffsetX < 0 || textureOffsetX >= texture->width)
            {
                continue;
            }

            for (int y = (int)spriteTopY; y < (int)spriteBottomY; y++)
            {
                if (x <= 0 || x >= WINDOW_WIDTH || y <= 0 || y >= WINDOW_HEIGHT)
                {
                    continue;
                }

                int distanceFromTop = y + (int)(spriteHeight / 2.0f) - (WINDOW_HEIGHT / 2);
                int textureOffsetY = (int)(distanceFromTop * ((float)texture->height / spriteHeight));
                if (textureOffsetY < 0 || textureOffsetY >= texture->height)
                {
                    continue;
                }

                color_t texelColor = texture->pixels[(texture->width * textureOffsetY) + textureOffsetX];
                if (sprite.distance < rays[x].distance && !IsTransparentSpriteColor(texelColor))
                {
                    DrawPixel(x, y, texelColor);
                }
            }
        }
    }
}

void RenderMapSprites(void)
{
    for (int i = 0; i < gSpriteCount; i++)
    {
        DrawRect(
            (int)(gSprites[i].x * MINIMAP_SCALE_FACTOR),
            (int)(gSprites[i].y * MINIMAP_SCALE_FACTOR),
            2,
            2,
            gSprites[i].visible ? CYAN_COLOR : DARK_GRAY_COLOR);
    }
}
