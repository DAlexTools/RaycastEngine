#include "Player.h"
#include "Utilities/Utils.h"

player_t player = {
    .x = WINDOW_WIDTH / 2.0f,       //
    .y = WINDOW_HEIGHT / 2.0f,      //
    .width = 35,                     //
    .height = 35,                    //
    .turnDirection = 0,             //
    .walkDiretion = 0,              //
    .rotationAngle = PI / 2.0f,     //
    .walkSpeed = 200.0f,            //
    .turnSpeed = 200 * (PI / 180),  //
    .mouseSensitivity = 0.05f,      //
    .sprintMultiply = 150.0f        //
};

void MovePlayer(float deltaTime)
{
    player.rotationAngle += player.turnDirection * player.turnSpeed * deltaTime;
    NormalizeAngle(&player.rotationAngle);
    float moveStep = player.walkDiretion * player.walkSpeed * deltaTime;
    float strafeStep = player.strafeDirection * player.walkSpeed * deltaTime;

    float newPlayerX = player.x + cos(player.rotationAngle) * moveStep;
    float newPlayerY = player.y + sin(player.rotationAngle) * moveStep;

    newPlayerX += cos(player.rotationAngle + PI / 2) * strafeStep;
    newPlayerY += sin(player.rotationAngle + PI / 2) * strafeStep;

    if (!MapHasWallAt(newPlayerX, newPlayerY))
    {
        player.x = newPlayerX;
        player.y = newPlayerY;
    }
}

void RenderMapPlayer(void)
{
    DrawRect(                                  //
        player.x * MINIMAP_SCALE_FACTOR,       //
        player.y * MINIMAP_SCALE_FACTOR,       //
        player.width * MINIMAP_SCALE_FACTOR,   //
        player.height * MINIMAP_SCALE_FACTOR,  //
        WHITE_COLOR                            //
    );
}
