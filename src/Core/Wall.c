#include "Wall.h"
#include "Map.h"

#include <stdbool.h>

#define FLOOR_LIGHT_FACTOR 0.70f
#define CEILING_LIGHT_FACTOR 0.45f

static int WrapTextureCoordinate(float coordinate, int textureSize)
{
    int tileOffset = (int)coordinate % TILE_SIZE;
    if (tileOffset < 0)
    {
        tileOffset += TILE_SIZE;
    }

    return (tileOffset * textureSize) / TILE_SIZE;
}

static color_t SampleTexture(int textureIndex, float worldX, float worldY, color_t fallbackColor)
{
    const texture_data_t* texture = GetTextureData(textureIndex);
    if (texture == NULL || texture->pixels == NULL)
    {
        return fallbackColor;
    }

    if (texture->width <= 0 || texture->height <= 0)
    {
        return fallbackColor;
    }

    int textureOffsetX = WrapTextureCoordinate(worldX, texture->width);
    int textureOffsetY = WrapTextureCoordinate(worldY, texture->height);

    return texture->pixels[(texture->width * textureOffsetY) + textureOffsetX];
}

void ChangeColorIntensity(color_t* color, float factor)
{
    unsigned char a = (*color >> 24) & 0xFF;
    unsigned char r = (*color >> 16) & 0xFF;
    unsigned char g = (*color >> 8) & 0xFF;
    unsigned char b = *color & 0xFF;

    r = (unsigned char)(r * factor);
    g = (unsigned char)(g * factor);
    b = (unsigned char)(b * factor);

    *color = ((color_t)a << 24) | ((color_t)r << 16) | ((color_t)g << 8) | b;
}

static void RenderWallSlice(
    int screenX,
    float distance,
    float rayAngle,
    float wallHitX,
    float wallHitY,
    int textureValue,
    bool wasHitVertical,
    int hitMapRow,
    int hitMapCol)
{
    float perpDistance = distance * cosf(rayAngle - player.rotationAngle);
    if (perpDistance <= 0.0f)
    {
        return;
    }

    float wallHeight = ((float)TILE_SIZE / perpDistance) * (float)DIST_PROJ_PLANE;

    int wallTopY = (WINDOW_HEIGHT / 2) - (int)(wallHeight / 2.0f);
    wallTopY = wallTopY < 0 ? 0 : wallTopY;

    int wallBottomY = (WINDOW_HEIGHT / 2) + (int)(wallHeight / 2.0f);
    wallBottomY = wallBottomY > WINDOW_HEIGHT ? WINDOW_HEIGHT : wallBottomY;

    bool isDoor = textureValue == MAP_DOOR_TILE;
    int texNum = isDoor ? GetDoorTextureIndex() : textureValue - 1;
    const texture_data_t* texture = GetTextureData(texNum);
    if (texture == NULL || texture->pixels == NULL)
    {
        return;
    }

    if (texture->width <= 0 || texture->height <= 0)
    {
        return;
    }

    int textureOffsetX;
    if (wasHitVertical)
    {
        textureOffsetX = WrapTextureCoordinate(wallHitY, texture->width);
    }
    else
    {
        textureOffsetX = WrapTextureCoordinate(wallHitX, texture->width);
    }

    if (isDoor)
    {
        int slideOffset = (int)(GetDoorOpenAmount(hitMapRow, hitMapCol) * texture->width);
        textureOffsetX += slideOffset;
        if (textureOffsetX >= texture->width)
        {
            return;
        }
    }

    for (int y = wallTopY; y < wallBottomY; y++)
    {
        int distanceFromTop = y + (int)(wallHeight / 2.0f) - (WINDOW_HEIGHT / 2);
        int textureOffsetY = (int)(distanceFromTop * ((float)texture->height / wallHeight));

        if (textureOffsetY < 0)
        {
            textureOffsetY = 0;
        }
        else if (textureOffsetY >= texture->height)
        {
            textureOffsetY = texture->height - 1;
        }

        color_t texelColor = texture->pixels[(texture->width * textureOffsetY) + textureOffsetX];

        if (wasHitVertical)
        {
            ChangeColorIntensity(&texelColor, 0.70f);
        }

        DrawPixel(screenX, y, texelColor);
    }
}

void RenderFloorAndCeilingProjection(void)
{
    int floorTextureIndex = GetFloorTextureIndex();
    int ceilingTextureIndex = GetCeilingTextureIndex();
    const float halfWindowWidth = WINDOW_WIDTH / 2.0f;
    const float halfWindowHeight = WINDOW_HEIGHT / 2.0f;
    const float playerHeight = TILE_SIZE / 2.0f;
    const float projectionPlaneDistance = (float)DIST_PROJ_PLANE;

    const float forwardX = cosf(player.rotationAngle);
    const float forwardY = sinf(player.rotationAngle);
    const float rightX = -sinf(player.rotationAngle);
    const float rightY = cosf(player.rotationAngle);

    for (int y = WINDOW_HEIGHT / 2; y < WINDOW_HEIGHT; y++)
    {
        float screenY = (float)y - halfWindowHeight + 1.0f;
        float rowDistance = (playerHeight * projectionPlaneDistance) / screenY;
        float stepX = rightX * rowDistance / projectionPlaneDistance;
        float stepY = rightY * rowDistance / projectionPlaneDistance;

        float worldX = player.x + forwardX * rowDistance - stepX * halfWindowWidth;
        float worldY = player.y + forwardY * rowDistance - stepY * halfWindowWidth;
        int ceilingY = WINDOW_HEIGHT - y - 1;

        for (int x = 0; x < WINDOW_WIDTH; x++)
        {
            bool useDefaultFloor = floorTextureIndex == MAP_DEFAULT_SURFACE_TEXTURE_INDEX;
            bool useDefaultCeiling = ceilingTextureIndex == MAP_DEFAULT_SURFACE_TEXTURE_INDEX;
            color_t floorColor = useDefaultFloor ? MEDIUM_GRAY_COLOR : SampleTexture(floorTextureIndex, worldX, worldY, MEDIUM_GRAY_COLOR);
            color_t ceilingColor = useDefaultCeiling ? DARK_GRAY_COLOR : SampleTexture(ceilingTextureIndex, worldX, worldY, DARK_GRAY_COLOR);

            if (!useDefaultFloor)
            {
                ChangeColorIntensity(&floorColor, FLOOR_LIGHT_FACTOR);
            }
            if (!useDefaultCeiling)
            {
                ChangeColorIntensity(&ceilingColor, CEILING_LIGHT_FACTOR);
            }

            DrawPixel(x, y, floorColor);
            DrawPixel(x, ceilingY, ceilingColor);

            worldX += stepX;
            worldY += stepY;
        }
    }
}

void RenderWallProjection(void)
{
    for (int x = 0; x < NUM_RAYS; x++)
    {
        if (rays[x].hasThroughDoorHit)
        {
            RenderWallSlice(
                x,
                rays[x].throughDoorDistance,
                rays[x].rayAngle,
                rays[x].throughDoorHitX,
                rays[x].throughDoorHitY,
                rays[x].throughDoorTexture,
                rays[x].throughDoorWasHitVertical,
                rays[x].throughDoorMapRow,
                rays[x].throughDoorMapCol);
        }

        RenderWallSlice(
            x,
            rays[x].distance,
            rays[x].rayAngle,
            rays[x].wallHitX,
            rays[x].wallHitY,
            rays[x].texture,
            rays[x].wasHitVertical,
            rays[x].hitMapRow,
            rays[x].hitMapCol);
    }
}
