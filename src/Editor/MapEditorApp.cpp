#include "Editor.h"
#include "Graphics.h"
#include "Map.h"
#include "Textures.h"
#include "Utilities/MacroFunction.h"

#include <SDL2/SDL.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    if (!InitializeWindowWithTitle("Raycast Map Editor", false))
    {
        return 1;
    }

    LoadTextures();
    LoadMapFromFile(DEFAULT_MAP_FILE_PATH);

    if (!EditorInitialize())
    {
        LOG_ERROR("Error initialize map editor.\n");
        FreeTextures();
        DestroyGraphicsWindow();
        return 1;
    }

    EditorSetOpen(true);

    bool isRunning = true;
    while (isRunning)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            EditorProcessEvent(&event);
            if (event.type == SDL_QUIT)
            {
                isRunning = false;
            }
            else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
            {
                isRunning = false;
            }
        }

        EditorBeginFrame();
        SDL_SetRenderDrawColor(GetGraphicsRenderer(), 18, 19, 23, 255);
        SDL_RenderClear(GetGraphicsRenderer());
        EditorRender();
        PresentRenderer();

        if (!EditorIsOpen())
        {
            isRunning = false;
        }

        SDL_Delay(16);
    }

    EditorShutdown();
    FreeTextures();
    DestroyGraphicsWindow();
    return 0;
}
