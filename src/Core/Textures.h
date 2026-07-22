#ifndef TEXTURES_H
#define TEXTURES_H

/**
 * @file Textures.h
 * @brief Texture loading, storage, and lookup API for walls and sprites.
 */

#include "Utilities/MacroFunction.h"
#include "upng.h"

#include <stdint.h>

/**
 * @brief Built-in texture indices used by map tile values.
 */
typedef enum
{
    TEXTURE_REDBRICK = 0, /**< Red brick wall texture. */
    TEXTURE_PURPLESTONE,  /**< Purple stone wall texture. */
    TEXTURE_MOSSYSTONE,   /**< Mossy stone wall texture. */
    TEXTURE_GRAYSTONE,    /**< Gray stone wall texture. */
    TEXTURE_COLORSTONE,   /**< Color stone wall texture. */
    TEXTURE_BLUESTONE,    /**< Blue stone wall texture. */
    TEXTURE_WOOD,         /**< Wood wall and default floor texture. */
    TEXTURE_EAGLE,        /**< Eagle banner wall texture. */
    TEXTURE_PIKUMA,       /**< Pikuma decorative wall texture. */
    TEXTURE_BARREL,       /**< Built-in barrel sprite texture. */
    TEXTURE_LIGHT,        /**< Built-in light sprite texture. */
    TEXTURE_TABLE,        /**< Built-in table sprite texture. */
    TEXTURE_GUARD,        /**< Built-in guard sprite texture. */
    TEXTURE_ARMOR         /**< Built-in armor sprite texture. */
} texture_index_t;

/** @brief Default floor texture index used when maps do not override it. */
#define FLOOR_TEXTURE_INDEX TEXTURE_WOOD

/** @brief Default ceiling texture index used when maps do not override it. */
#define CEILING_TEXTURE_INDEX TEXTURE_GRAYSTONE

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief CPU-side texture pixels and metadata.
 */
typedef struct texture_data_t
{
    int width;             /**< Texture width in pixels. */
    int height;            /**< Texture height in pixels. */
    const color_t* pixels; /**< Packed RGBA pixel buffer. */
    char name[64];         /**< Display name used by the editor. */
} texture_data_t;

/** @brief Decoded built-in PNG textures, indexed by texture_index_t. */
extern upng_t* _Textures[NUM_BUILTIN_TEXTURES];

/**
 * @brief Loads built-in textures, wall BMP textures, and sprite textures.
 */
void LoadTextures(void);

/**
 * @brief Releases all texture buffers owned by the texture module.
 */
void FreeTextures(void);

/**
 * @brief Gets the total number of loaded wall/floor/ceiling textures.
 *
 * @return Runtime texture count.
 */
int GetTextureCount(void);

/**
 * @brief Gets texture data by runtime texture index.
 *
 * @param textureIndex Zero-based texture index.
 * @return Texture data pointer, or NULL when index is invalid.
 */
const texture_data_t* GetTextureData(int textureIndex);

/**
 * @brief Gets the display name for a runtime texture.
 *
 * @param textureIndex Zero-based texture index.
 * @return Texture display name, or "Unknown" when index is invalid.
 */
const char* GetTextureName(int textureIndex);

/**
 * @brief Gets the total number of loaded sprite textures.
 *
 * @return Runtime sprite texture count.
 */
int GetSpriteTextureCount(void);

/**
 * @brief Gets sprite texture data by runtime texture index.
 *
 * @param textureIndex Zero-based sprite texture index.
 * @return Texture data pointer, or NULL when index is invalid.
 */
const texture_data_t* GetSpriteTextureData(int textureIndex);

/**
 * @brief Gets the display name for a sprite texture.
 *
 * @param textureIndex Zero-based sprite texture index.
 * @return Sprite texture display name, or "Unknown" when index is invalid.
 */
const char* GetSpriteTextureName(int textureIndex);

#ifdef __cplusplus
}
#endif

#endif  // TEXTURES_H
