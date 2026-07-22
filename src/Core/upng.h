/*
uPNG -- derived from LodePNG version 20100808

Copyright (c) 2005-2010 Lode Vandevenne
Copyright (c) 2010 Sean Middleditch

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented; you must not
		claim that you wrote the original software. If you use this software
		in a product, an acknowledgment in the product documentation would be
		appreciated but is not required.

		2. Altered source versions must be plainly marked as such, and must not be
		misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		distribution.
*/

#if !defined(UPNG_H)
#define UPNG_H

/**
 * @file upng.h
 * @brief Minimal PNG decoder API used by the texture loader.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Error codes returned by uPNG operations.
 */
typedef enum upng_error {
	UPNG_EOK			= 0, /**< Operation succeeded. */
	UPNG_ENOMEM			= 1, /**< Memory allocation failed. */
	UPNG_ENOTFOUND		= 2, /**< Resource was not found. */
	UPNG_ENOTPNG		= 3, /**< Input data does not contain a PNG header. */
	UPNG_EMALFORMED		= 4, /**< Input data is not a valid PNG image. */
	UPNG_EUNSUPPORTED	= 5, /**< PNG uses an unsupported critical chunk. */
	UPNG_EUNINTERLACED	= 6, /**< PNG interlacing is not supported. */
	UPNG_EUNFORMAT		= 7, /**< PNG color format is not supported. */
	UPNG_EPARAM			= 8  /**< Invalid parameter passed to the API. */
} upng_error;

/**
 * @brief Decoded PNG pixel formats supported by uPNG.
 */
typedef enum upng_format {
	UPNG_BADFORMAT,        /**< Unknown or unsupported format. */
	UPNG_RGB8,             /**< 8-bit RGB. */
	UPNG_RGB16,            /**< 16-bit RGB. */
	UPNG_RGBA8,            /**< 8-bit RGBA. */
	UPNG_RGBA16,           /**< 16-bit RGBA. */
	UPNG_LUMINANCE1,       /**< 1-bit luminance. */
	UPNG_LUMINANCE2,       /**< 2-bit luminance. */
	UPNG_LUMINANCE4,       /**< 4-bit luminance. */
	UPNG_LUMINANCE8,       /**< 8-bit luminance. */
	UPNG_LUMINANCE_ALPHA1, /**< 1-bit luminance with alpha. */
	UPNG_LUMINANCE_ALPHA2, /**< 2-bit luminance with alpha. */
	UPNG_LUMINANCE_ALPHA4, /**< 4-bit luminance with alpha. */
	UPNG_LUMINANCE_ALPHA8  /**< 8-bit luminance with alpha. */
} upng_format;

/**
 * @brief Opaque PNG decoder instance.
 */
typedef struct upng_t upng_t;

/**
 * @brief Creates a PNG decoder from an in-memory byte buffer.
 *
 * @param buffer PNG file bytes. The caller owns the buffer.
 * @param size Buffer size in bytes.
 * @return New decoder instance, or NULL on allocation failure.
 */
upng_t*		upng_new_from_bytes	(const unsigned char* buffer, unsigned long size);

/**
 * @brief Creates a PNG decoder by reading a PNG file from disk.
 *
 * @param path Path to a PNG file.
 * @return New decoder instance, or NULL when allocation fails.
 */
upng_t*		upng_new_from_file	(const char* path);

/**
 * @brief Releases a PNG decoder and its owned buffers.
 *
 * @param upng Decoder instance, or NULL.
 */
void		upng_free			(upng_t* upng);

/**
 * @brief Parses the PNG header and metadata.
 *
 * @param upng Decoder instance.
 * @return Current decoder error code.
 */
upng_error	upng_header			(upng_t* upng);

/**
 * @brief Decodes the PNG image into an internal pixel buffer.
 *
 * @param upng Decoder instance.
 * @return Current decoder error code.
 */
upng_error	upng_decode			(upng_t* upng);

/**
 * @brief Gets the last decoder error code.
 *
 * @param upng Decoder instance.
 * @return Last error code.
 */
upng_error	upng_get_error		(const upng_t* upng);

/**
 * @brief Gets the source line that set the last decoder error.
 *
 * @param upng Decoder instance.
 * @return Source line number, or 0 when no error line was recorded.
 */
unsigned	upng_get_error_line	(const upng_t* upng);

/**
 * @brief Gets decoded image width.
 *
 * @param upng Decoder instance.
 * @return Width in pixels.
 */
unsigned	upng_get_width		(const upng_t* upng);

/**
 * @brief Gets decoded image height.
 *
 * @param upng Decoder instance.
 * @return Height in pixels.
 */
unsigned	upng_get_height		(const upng_t* upng);

/**
 * @brief Gets decoded bits per pixel.
 *
 * @param upng Decoder instance.
 * @return Bits per pixel.
 */
unsigned	upng_get_bpp		(const upng_t* upng);

/**
 * @brief Gets decoded bit depth per channel.
 *
 * @param upng Decoder instance.
 * @return Bit depth per channel.
 */
unsigned	upng_get_bitdepth	(const upng_t* upng);

/**
 * @brief Gets decoded component count.
 *
 * @param upng Decoder instance.
 * @return Number of color components per pixel.
 */
unsigned	upng_get_components	(const upng_t* upng);

/**
 * @brief Gets decoded pixel size in bytes.
 *
 * @param upng Decoder instance.
 * @return Bytes per decoded pixel.
 */
unsigned	upng_get_pixelsize	(const upng_t* upng);

/**
 * @brief Gets decoded pixel format.
 *
 * @param upng Decoder instance.
 * @return Pixel format enum value.
 */
upng_format	upng_get_format		(const upng_t* upng);

/**
 * @brief Gets the decoded pixel buffer.
 *
 * @param upng Decoder instance.
 * @return Pointer to decoder-owned pixel data.
 */
const unsigned char*	upng_get_buffer		(const upng_t* upng);

/**
 * @brief Gets decoded pixel buffer size.
 *
 * @param upng Decoder instance.
 * @return Buffer size in bytes.
 */
unsigned				upng_get_size		(const upng_t* upng);

#ifdef __cplusplus
}
#endif

#endif /*defined(UPNG_H)*/
