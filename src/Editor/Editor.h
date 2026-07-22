#ifndef EDITOR_H
#define EDITOR_H

/**
 * @file Editor.h
 * @brief ImGui-powered map editor lifecycle and rendering API.
 */

#include <stdbool.h>
#include <SDL2/SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes ImGui and editor preview resources.
 *
 * @return true when the editor context and backends are initialized.
 */
bool EditorInitialize(void);

/**
 * @brief Releases editor textures, ImGui backends, and ImGui context.
 */
void EditorShutdown(void);

/**
 * @brief Passes an SDL event to the editor UI.
 *
 * @param event Event received from SDL_PollEvent.
 */
void EditorProcessEvent(const SDL_Event* event);

/**
 * @brief Starts a new ImGui frame for the editor.
 */
void EditorBeginFrame(void);

/**
 * @brief Draws the editor window when open and submits ImGui draw data.
 */
void EditorRender(void);

/**
 * @brief Toggles the editor open state.
 */
void EditorToggle(void);

/**
 * @brief Sets the editor open state explicitly.
 *
 * @param open true to open the editor, false to close it.
 */
void EditorSetOpen(bool open);

/**
 * @brief Checks whether the editor is currently open.
 *
 * @return true when the editor window is open.
 */
bool EditorIsOpen(void);

#ifdef __cplusplus
}
#endif

#endif  // EDITOR_H
