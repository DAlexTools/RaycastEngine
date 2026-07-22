#include "Ray.h"
#include "Utilities/Utils.h"

#include <float.h>

ray_t rays[NUM_RAYS];

#define BRENSEHEM_LINE 1

typedef struct cast_hit_t
{
    bool found;
    bool wasHitVertical;
    float wallHitX;
    float wallHitY;
    float distance;
    int texture;
    int hitMapRow;
    int hitMapCol;
} cast_hit_t;

bool IsRayFacingUp(float angle)
{
    return !IsRayFacingDown(angle);
}

bool IsRayFacingDown(float angle)
{
    return angle > 0 && angle < PI;
}

bool IsRayFacingLeft(float angle)
{
    return !IsRayFacingRight(angle);
}

bool IsRayFacingRight(float angle)
{
    return angle < 0.5 * PI || angle > 1.5 * PI;
}

static bool ShouldHitTile(int row, int col, int ignoreDoorRow, int ignoreDoorCol)
{
    if (row < 0 || row >= GetMapRows() || col < 0 || col >= GetMapCols())
    {
        return true;
    }

    int tile = GetMap(row, col);
    if (tile == 0)
    {
        return false;
    }

    return !(tile == MAP_DOOR_TILE && row == ignoreDoorRow && col == ignoreDoorCol);
}

static cast_hit_t MakeEmptyHit(void)
{
    cast_hit_t hit;
    hit.found = false;
    hit.wasHitVertical = false;
    hit.wallHitX = 0.0f;
    hit.wallHitY = 0.0f;
    hit.distance = FLT_MAX;
    hit.texture = 0;
    hit.hitMapRow = -1;
    hit.hitMapCol = -1;
    return hit;
}

static int GetHitTexture(int row, int col)
{
    int tile = GetMap(row, col);
    return tile != 0 ? tile : 1;
}

static cast_hit_t CastRayHit(float rayAngle, int ignoreDoorRow, int ignoreDoorCol)
{
    float xintercept, yintercept;
    float xstep, ystep;
    cast_hit_t result = MakeEmptyHit();

    ///////////////////////////////////////////
    // HORIZONTAL RAY-GRID INTERSECTION CODE
    ///////////////////////////////////////////
    bool foundHorzWallHit = false;
    float horzWallHitX = 0;
    float horzWallHitY = 0;
    int horzWallTexture = 0;
    int horzWallHitRow = -1;
    int horzWallHitCol = -1;

    // Find the y-coordinate of the closest horizontal grid intersection
    yintercept = floor(player.y / TILE_SIZE) * TILE_SIZE;
    yintercept += IsRayFacingDown(rayAngle) ? TILE_SIZE : 0;

    // Find the x-coordinate of the closest horizontal grid intersection
    xintercept = player.x + (yintercept - player.y) / tan(rayAngle);

    // Calculate the increment xstep and ystep
    ystep = TILE_SIZE;
    ystep *= IsRayFacingUp(rayAngle) ? -1 : 1;

    xstep = TILE_SIZE / tan(rayAngle);
    xstep *= (IsRayFacingLeft(rayAngle) && xstep > 0) ? -1 : 1;
    xstep *= (IsRayFacingRight(rayAngle) && xstep < 0) ? -1 : 1;

    float nextHorzTouchX = xintercept;
    float nextHorzTouchY = yintercept;

    // Increment xstep and ystep until we find a wall
    while (IsInsideMap(nextHorzTouchX, nextHorzTouchY))
    {
        float xToCheck = nextHorzTouchX;
        float yToCheck = nextHorzTouchY + (IsRayFacingUp(rayAngle) ? -1 : 0);
        int rowToCheck = (int)floor(yToCheck / TILE_SIZE);
        int colToCheck = (int)floor(xToCheck / TILE_SIZE);

        if (ShouldHitTile(rowToCheck, colToCheck, ignoreDoorRow, ignoreDoorCol))
        {
            // found a wall hit
            horzWallHitRow = rowToCheck;
            horzWallHitCol = colToCheck;
            horzWallHitX = nextHorzTouchX;
            horzWallHitY = nextHorzTouchY;
            horzWallTexture = GetHitTexture(horzWallHitRow, horzWallHitCol);
            foundHorzWallHit = true;
            break;
        }
        else
        {
            nextHorzTouchX += xstep;
            nextHorzTouchY += ystep;
        }
    }

    ///////////////////////////////////////////
    // VERTICAL RAY-GRID INTERSECTION CODE
    ///////////////////////////////////////////
    bool foundVertWallHit = false;
    float vertWallHitX = 0;
    float vertWallHitY = 0;
    int vertWallTexture = 0;
    int vertWallHitRow = -1;
    int vertWallHitCol = -1;

    // Find the x-coordinate of the closest vertical grid intersection
    xintercept = floor(player.x / TILE_SIZE) * TILE_SIZE;
    xintercept += IsRayFacingRight(rayAngle) ? TILE_SIZE : 0;

    // Find the y-coordinate of the closest vertical grid intersection
    yintercept = player.y + (xintercept - player.x) * tan(rayAngle);

    // Calculate the increment xstep and ystep
    xstep = TILE_SIZE;
    xstep *= IsRayFacingLeft(rayAngle) ? -1 : 1;

    ystep = TILE_SIZE * tan(rayAngle);
    ystep *= (IsRayFacingUp(rayAngle) && ystep > 0) ? -1 : 1;
    ystep *= (IsRayFacingDown(rayAngle) && ystep < 0) ? -1 : 1;

    float nextVertTouchX = xintercept;
    float nextVertTouchY = yintercept;

    // Increment xstep and ystep until we find a wall
    while (IsInsideMap(nextVertTouchX, nextVertTouchY))
    {
        float xToCheck = nextVertTouchX + (IsRayFacingLeft(rayAngle) ? -1 : 0);
        float yToCheck = nextVertTouchY;
        int rowToCheck = (int)floor(yToCheck / TILE_SIZE);
        int colToCheck = (int)floor(xToCheck / TILE_SIZE);

        if (ShouldHitTile(rowToCheck, colToCheck, ignoreDoorRow, ignoreDoorCol))
        {
            // found a wall hit
            vertWallHitRow = rowToCheck;
            vertWallHitCol = colToCheck;
            vertWallHitX = nextVertTouchX;
            vertWallHitY = nextVertTouchY;
            vertWallTexture = GetHitTexture(vertWallHitRow, vertWallHitCol);
            foundVertWallHit = true;
            break;
        }
        else
        {
            nextVertTouchX += xstep;
            nextVertTouchY += ystep;
        }
    }

    // Calculate both horizontal and vertical hit distances and choose the smallest one
    float horzHitDistance = foundHorzWallHit ? DistanceBetweenPoints(player.x, player.y, horzWallHitX, horzWallHitY) : FLT_MAX;
    float vertHitDistance = foundVertWallHit ? DistanceBetweenPoints(player.x, player.y, vertWallHitX, vertWallHitY) : FLT_MAX;

    if (foundVertWallHit && vertHitDistance < horzHitDistance)
    {
        result.found = true;
        result.distance = vertHitDistance;
        result.wallHitX = vertWallHitX;
        result.wallHitY = vertWallHitY;
        result.texture = vertWallTexture;
        result.hitMapRow = vertWallHitRow;
        result.hitMapCol = vertWallHitCol;
        result.wasHitVertical = true;
    }
    else if (foundHorzWallHit)
    {
        result.found = true;
        result.distance = horzHitDistance;
        result.wallHitX = horzWallHitX;
        result.wallHitY = horzWallHitY;
        result.texture = horzWallTexture;
        result.hitMapRow = horzWallHitRow;
        result.hitMapCol = horzWallHitCol;
        result.wasHitVertical = false;
    }
    else if (foundVertWallHit)
    {
        result.found = true;
        result.distance = vertHitDistance;
        result.wallHitX = vertWallHitX;
        result.wallHitY = vertWallHitY;
        result.texture = vertWallTexture;
        result.hitMapRow = vertWallHitRow;
        result.hitMapCol = vertWallHitCol;
        result.wasHitVertical = true;
    }

    return result;
}

static void ApplyPrimaryHit(ray_t* ray, const cast_hit_t* hit, float rayAngle)
{
    ray->distance = hit->distance;
    ray->wallHitX = hit->wallHitX;
    ray->wallHitY = hit->wallHitY;
    ray->texture = hit->texture;
    ray->hitMapRow = hit->hitMapRow;
    ray->hitMapCol = hit->hitMapCol;
    ray->wasHitVertical = hit->wasHitVertical;
    ray->rayAngle = rayAngle;
    ray->hasThroughDoorHit = false;
    ray->throughDoorWasHitVertical = false;
    ray->throughDoorHitX = 0.0f;
    ray->throughDoorHitY = 0.0f;
    ray->throughDoorDistance = FLT_MAX;
    ray->throughDoorTexture = 0;
    ray->throughDoorMapRow = -1;
    ray->throughDoorMapCol = -1;
}

static void ApplyThroughDoorHit(ray_t* ray, const cast_hit_t* hit)
{
    ray->hasThroughDoorHit = hit->found;
    ray->throughDoorWasHitVertical = hit->wasHitVertical;
    ray->throughDoorHitX = hit->wallHitX;
    ray->throughDoorHitY = hit->wallHitY;
    ray->throughDoorDistance = hit->distance;
    ray->throughDoorTexture = hit->texture;
    ray->throughDoorMapRow = hit->hitMapRow;
    ray->throughDoorMapCol = hit->hitMapCol;
}

void CastRay(float rayAngle, int stripId)
{
    NormalizeAngle(&rayAngle);

    cast_hit_t hit = CastRayHit(rayAngle, -1, -1);
    ApplyPrimaryHit(&rays[stripId], &hit, rayAngle);

    if (hit.found && hit.texture == MAP_DOOR_TILE)
    {
        cast_hit_t throughDoorHit = CastRayHit(rayAngle, hit.hitMapRow, hit.hitMapCol);
        ApplyThroughDoorHit(&rays[stripId], &throughDoorHit);
    }
}

void CastAllRays(void)
{
    for (int col = 0; col < NUM_RAYS; col++)
    {
        float rayAngle = player.rotationAngle + atan((col - NUM_RAYS / 2) / DIST_PROJ_PLANE);
        CastRay(rayAngle, col);
    }
}

void RenderMapRays(void)
{
    for (int i = 0; i < NUM_RAYS; i++)
    {
#if BRENSEHEM_LINE
        // bresengam algorithm for more optimization draw line 
        BresenhamLine(player.x * MINIMAP_SCALE_FACTOR,      //
            player.y * MINIMAP_SCALE_FACTOR,                //
            rays[i].wallHitX * MINIMAP_SCALE_FACTOR,        //
            rays[i].wallHitY * MINIMAP_SCALE_FACTOR,        //
            RED_COLOR                                       //
        );

#else
        // DDA Algorithm standart draw line algorithm 
        DrawLine(player.x * MINIMAP_SCALE_FACTOR,     //
            player.y * MINIMAP_SCALE_FACTOR,          //
            rays[i].wallHitX * MINIMAP_SCALE_FACTOR,  //
            rays[i].wallHitY * MINIMAP_SCALE_FACTOR,  //
            RED_COLOR                                 //
        );
#endif
    }
}
