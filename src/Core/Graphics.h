#ifndef GRAPHICS_H
#define GRAPHICS_H

/**
 * @file Graphics.h
 * @brief SDL window management and software color-buffer drawing API.
 */

#include <stdbool.h>
#include <stdint.h>
#include <SDL2/SDL.h>
#include "Utilities/Deffinitions.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the default fullscreen game window.
 *
 * @return true when SDL, the window, the renderer, and the color buffer were created.
 */
bool InitializeWindow(void);

/**
 * @brief Initializes an SDL window, renderer, color buffer, and backing texture.
 *
 * @param title Window title shown by the operating system.
 * @param fullscreen When true, creates a fullscreen desktop window.
 * @return true when initialization succeeds.
 */
bool InitializeWindowWithTitle(const char* title, bool fullscreen);

/**
 * @brief Gets the active SDL window.
 *
 * @return Window pointer owned by the graphics module, or NULL before initialization.
 */
SDL_Window* GetGraphicsWindow(void);

/**
 * @brief Gets the active SDL renderer.
 *
 * @return Renderer pointer owned by the graphics module, or NULL before initialization.
 */
SDL_Renderer* GetGraphicsRenderer(void);

/**
 * @brief Allocates the software color buffer.
 *
 * @return true when the buffer allocation succeeds.
 */
bool AllocateColorBuffer(void);

/**
 * @brief Creates the SDL texture used to present the software color buffer.
 *
 * @return true when the texture is created.
 */
bool AllocateColorBufferTexture(void);

/**
 * @brief Releases the color buffer, SDL texture, renderer, window, and SDL itself.
 */
void DestroyGraphicsWindow(void);

/**
 * @brief Fills the entire software color buffer with one color.
 *
 * @param color Packed color value written into every pixel.
 */
void ClearColorBuffer(color_t color);

/**
 * @brief Uploads the software color buffer into the SDL texture and renders it.
 */
void RenderColorBuffer(void);

/**
 * @brief Presents the current SDL backbuffer to the window.
 */
void PresentRenderer(void);

/**
 * @brief Draws one pixel into the software color buffer.
 *
 * @param x Pixel x coordinate.
 * @param y Pixel y coordinate.
 * @param color Packed color value.
 */
void DrawPixel(int x, int y, color_t color);

/**
 * @brief Draws a filled rectangle into the software color buffer.
 *
 * @param x Top-left x coordinate.
 * @param y Top-left y coordinate.
 * @param width Rectangle width in pixels.
 * @param height Rectangle height in pixels.
 * @param color Packed fill color.
 */
void DrawRect(int x, int y, int width, int height, color_t color);

/**
 * @brief Draws a line with floating-point interpolation.
 *
 * @param x0 Start x coordinate.
 * @param y0 Start y coordinate.
 * @param x1 End x coordinate.
 * @param y1 End y coordinate.
 * @param color Packed line color.
 */
void DrawLine(int x0, int y0, int x1, int y1, color_t color);

/**
 * @brief Draws a line with Bresenham's integer algorithm.
 *
 * @param x0 Start x coordinate.
 * @param y0 Start y coordinate.
 * @param x1 End x coordinate.
 * @param y1 End y coordinate.
 * @param color Packed line color.
 */
void BresenhamLine(int x0, int y0, int x1, int y1, color_t color);

#ifdef __cplusplus
}
#endif

#endif  // !GRAPHICS_H
