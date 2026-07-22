#pragma once

/**
 * @file MacroFunction.h
 * @brief Logging, assertion-style checks, and frame pacing helper macros.
 */

#include "Deffinitions.h"

/**
 * @brief Writes a formatted message to the specified C stream.
 *
 * @param stream Output stream such as stdout or stderr.
 * @param ... printf-compatible format string and values.
 */
#define LOG(stream, ...) fprintf(stream, __VA_ARGS__)

/**
 * @brief Writes a formatted error message to stderr.
 *
 * @param ... printf-compatible format string and values.
 */
#define LOG_ERROR(...) LOG(stderr, __VA_ARGS__)

/**
 * @brief Logs an asset loading error with one formatted value.
 *
 * @param format printf-compatible format string.
 * @param ptr Value inserted into the format string.
 */
#define PRINT_ERROR_LOADING(format, ptr) LOG_ERROR(format, ptr)

/**
 * @brief Returns false from the current function when a condition fails.
 *
 * @param condition Boolean expression that must evaluate to true.
 * @param msg Error message passed to LOG_ERROR when the check fails.
 */
#define CHECK(condition, msg)       \
    if (!(condition))               \
    {                               \
        LOG_ERROR(msg);             \
        return false;               \
    }

/**
 * @brief Busy-waits until the next target frame time has elapsed.
 *
 * @param ticksLastFrame SDL tick value captured for the previous frame.
 * @param FRAME_TIME_LENGTH Target frame length in milliseconds.
 */
#define WAIT_FOR_NEXT_FRAME(ticksLastFrame, FRAME_TIME_LENGTH) \
    while (!SDL_TICKS_PASSED(SDL_GetTicks(), (ticksLastFrame) + (FRAME_TIME_LENGTH)))
