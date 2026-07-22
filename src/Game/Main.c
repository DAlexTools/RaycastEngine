// RaycastEngine.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "SDL2/SDL.h"
#include <float.h>
#include <stdbool.h>
#include <stdio.h>
#include "Graphics.h"
#include "Utilities/MacroFunction.h"
#include "Textures.h"
#include "Map.h"
#include "Ray.h"
#include "Player.h"
#include "Sprite.h"
#include "Wall.h"
#include "Editor.h"




// on/off for debug minimap 
#define DEBUG_MAP 0

/* Global variable  */ 
int IsGameRunning = false;
int ticksLastFrame = 0;

static void ResetPlayerInput(void)
{
    player.walkDiretion = 0;
    player.strafeDirection = 0;
    player.turnDirection = 0;
    player.walkSpeed = 200.0f;
}

static void EnsurePlayerInsidePlayableMap(void)
{
    if (!MapHasWallAt(player.x, player.y))
    {
        return;
    }

    for (int row = 1; row < GetMapRows() - 1; row++)
    {
        for (int col = 1; col < GetMapCols() - 1; col++)
        {
            if (GetMap(row, col) == 0)
            {
                player.x = (col + 0.5f) * TILE_SIZE;
                player.y = (row + 0.5f) * TILE_SIZE;
                return;
            }
        }
    }

    player.x = GetMapCols() * TILE_SIZE * 0.5f;
    player.y = GetMapRows() * TILE_SIZE * 0.5f;
}

/*
 * Setup 
 */
void Setup()
{
    SDL_SetRelativeMouseMode(SDL_TRUE);
    SDL_ShowCursor(SDL_DISABLE);

    LoadTextures();
    LoadMapFromFile(DEFAULT_MAP_FILE_PATH);
    EnsurePlayerInsidePlayableMap();
    if (!EditorInitialize())
    {
        LOG_ERROR("Error initialize map editor.\n");
        IsGameRunning = false;
    }
}

/*
 * Input Proccessing 
 */
void ProcessInput()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        EditorProcessEvent(&event);
        switch (event.type)
        {
            case SDL_QUIT:
            {
                IsGameRunning = false;
                break;
            }
            case SDL_KEYDOWN:
            {
                if (event.key.keysym.sym == SDLK_F1)
                {
                    EditorToggle();
                    ResetPlayerInput();
                    break;
                }
                if (EditorIsOpen())
                {
                    break;
                }
                if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    IsGameRunning = false;
                }
                if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w)
                {
                    player.walkDiretion = +1;
                }
                if (event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_s)
                {
                    player.walkDiretion = -1;
                }
                if (event.key.keysym.sym == SDLK_a)
                {
                    player.strafeDirection = -1;
                }
                if (event.key.keysym.sym == SDLK_d)
                {
                    player.strafeDirection = +1;
                }
                if (event.key.keysym.sym == SDLK_LEFT)
                {
                    player.turnDirection = -1;
                }
                if (event.key.keysym.sym == SDLK_RIGHT)
                {
                    player.turnDirection = +1;
                }
                if (event.key.keysym.sym == SDLK_LSHIFT && !event.key.repeat)
                {
                    player.walkSpeed += player.sprintMultiply;
                }
                break;
            }
            case SDL_KEYUP:
            {
                if (EditorIsOpen())
                {
                    break;
                }
                if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w)
                {
                    player.walkDiretion = 0;
                }
                if (event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_s)
                {
                    player.walkDiretion = 0;
                }
                if (event.key.keysym.sym == SDLK_a)
                {
                    player.strafeDirection = 0;
                }
                if (event.key.keysym.sym == SDLK_d)
                {
                    player.strafeDirection = 0;
                }
                if (event.key.keysym.sym == SDLK_LEFT)
                {
                    player.turnDirection = 0;
                }
                if (event.key.keysym.sym == SDLK_RIGHT)
                {
                    player.turnDirection = 0;
                }
                if (event.key.keysym.sym == SDLK_LSHIFT)
                {
                    player.walkSpeed -= player.sprintMultiply;
                }
                break;
            }

            //case SDL_MOUSEMOTION: 
            //    player.rotationAngle += event.motion.xrel * player.mouseSensitivity;
            default: break;
        }
    }
}

/*
 * Update Implementation
 */
void Update()
{
    WAIT_FOR_NEXT_FRAME(ticksLastFrame, FRAME_TIME_LENGTH);

    // Waste some time until we reach the target frame time lenght
    float deltaTime = (float)(SDL_GetTicks() - ticksLastFrame) / 1000.0f;

    ticksLastFrame = (float)SDL_GetTicks();

    EnsurePlayerInsidePlayableMap();
    UpdateMapDoors(player.x, player.y, deltaTime);
    MovePlayer(deltaTime);
    CastAllRays();
}

/*
 * Render Implementaion 
 */
void Render()
{
    EditorBeginFrame();
    ClearColorBuffer(BLACK_COLOR);
    RenderFloorAndCeilingProjection();
    RenderWallProjection();
    RenderSpriteProjection();


    // DEBUG Display the minimap
#if DEBUG_MAP
    RenderMapGrid();
    RenderMapRays();
    RenderMapPlayer();
    RenderMapSprites();
#endif  // DEBUG_MAP

    RenderColorBuffer();
    EditorRender();
    PresentRenderer();
}

/*
 * Destruction Resource 
 */
void ReleaseResources(void)
{
    EditorShutdown();
    FreeTextures();
    DestroyGraphicsWindow();
}



/*
 * Main Function 
 */
int main(int argc, char* arga[])
{
    IsGameRunning = InitializeWindow();

    Setup();

    while (IsGameRunning)
    {
        ProcessInput();
        Update();
        Render();
    }

    ReleaseResources();
    return false;
}
///////////////////////////////////////////////////////////////////////////////////////////
