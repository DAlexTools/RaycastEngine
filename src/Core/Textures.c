#include "Textures.h"
#include "SDL2/SDL.h"

#include <stdbool.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#endif

upng_t* _Textures[NUM_BUILTIN_TEXTURES] = {0};

static texture_data_t _TextureData[MAX_TEXTURES] = {0};
static bool _TextureOwnsPixels[MAX_TEXTURES] = {false};
static int _TextureCount = 0;
static texture_data_t _SpriteTextureData[MAX_SPRITE_TEXTURES] = {0};
static bool _SpriteTextureOwnsPixels[MAX_SPRITE_TEXTURES] = {false};
static int _SpriteTextureCount = 0;

#define MAX_DISCOVERED_SPRITE_FILES 1024
#define MAX_ASSET_PATH_LENGTH 512
#define BMP_SPRITE_KEY_MIN_RED 0x80
#define BMP_SPRITE_KEY_MAX_GREEN 0x20
#define BMP_SPRITE_KEY_MIN_MAGENTA_BLUE 0x60
#define BMP_SPRITE_KEY_MAX_RED_BLUE 0x20

typedef struct texture_file_entry_t
{
    char path[MAX_ASSET_PATH_LENGTH];
    char name[64];
} texture_file_entry_t;

typedef enum bmp_sprite_transparency_key_t
{
    BMP_SPRITE_TRANSPARENCY_KEY_NONE,
    BMP_SPRITE_TRANSPARENCY_KEY_MAGENTA,
    BMP_SPRITE_TRANSPARENCY_KEY_RED
} bmp_sprite_transparency_key_t;

static const char* _TextureFileNames[NUM_BUILTIN_TEXTURES] = {
    "./resources/images/redbrick.png",     // [0]
    "./resources/images/purplestone.png",  // [1]
    "./resources/images/mossystone.png",   // [2]
    "./resources/images/graystone.png",    // [3]
    "./resources/images/colorstone.png",   // [4]
    "./resources/images/bluestone.png",    // [5]
    "./resources/images/wood.png",         // [6]
    "./resources/images/eagle.png",        // [7]
    "./resources/images/pikuma.png",       // [8]
    "./resources/images/barrel.png",       // [9]
    "./resources/images/light.png",        // [10]
    "./resources/images/table.png",        // [11]
    "./resources/images/guard.png",        // [12]
    "./resources/images/armor.png"         // [13]
};

static const char* _TextureNames[NUM_BUILTIN_TEXTURES] = {
    "Redbrick",
    "Purplestone",
    "Mossystone",
    "Graystone",
    "Colorstone",
    "Bluestone",
    "Wood",
    "Eagle",
    "Pikuma",
    "Barrel",
    "Light",
    "Table",
    "Guard",
    "Armor"
};

static bool AddTextureData(const char* name, int width, int height, const color_t* pixels, bool ownsPixels)
{
    if (width <= 0 || height <= 0 || pixels == NULL)
    {
        if (ownsPixels)
        {
            free((void*)pixels);
        }
        return false;
    }

    if (_TextureCount >= MAX_TEXTURES)
    {
        PRINT_ERROR_LOADING("Texture limit reached, skipping %s\n", name);
        if (ownsPixels)
        {
            free((void*)pixels);
        }
        return false;
    }

    texture_data_t* texture = &_TextureData[_TextureCount];
    texture->width = width;
    texture->height = height;
    texture->pixels = pixels;
    snprintf(texture->name, sizeof(texture->name), "%s", name);
    _TextureOwnsPixels[_TextureCount] = ownsPixels;
    _TextureCount++;
    return true;
}

static bool AddSpriteTextureData(const char* name, int width, int height, const color_t* pixels, bool ownsPixels)
{
    if (width <= 0 || height <= 0 || pixels == NULL)
    {
        if (ownsPixels)
        {
            free((void*)pixels);
        }
        return false;
    }

    if (_SpriteTextureCount >= MAX_SPRITE_TEXTURES)
    {
        PRINT_ERROR_LOADING("Sprite texture limit reached, skipping %s\n", name);
        if (ownsPixels)
        {
            free((void*)pixels);
        }
        return false;
    }

    texture_data_t* texture = &_SpriteTextureData[_SpriteTextureCount];
    texture->width = width;
    texture->height = height;
    texture->pixels = pixels;
    snprintf(texture->name, sizeof(texture->name), "%s", name);
    _SpriteTextureOwnsPixels[_SpriteTextureCount] = ownsPixels;
    _SpriteTextureCount++;
    return true;
}

static unsigned char GetPackedColorRed(color_t color)
{
    return (unsigned char)(color & 0xFF);
}

static unsigned char GetPackedColorGreen(color_t color)
{
    return (unsigned char)((color >> 8) & 0xFF);
}

static unsigned char GetPackedColorBlue(color_t color)
{
    return (unsigned char)((color >> 16) & 0xFF);
}

static unsigned char GetPackedColorAlpha(color_t color)
{
    return (unsigned char)((color >> 24) & 0xFF);
}

static bool IsBmpSpriteMagentaKeyColor(color_t color)
{
    unsigned char r = GetPackedColorRed(color);
    unsigned char g = GetPackedColorGreen(color);
    unsigned char b = GetPackedColorBlue(color);
    unsigned char a = GetPackedColorAlpha(color);

    return a != 0 && r >= BMP_SPRITE_KEY_MIN_RED && g <= BMP_SPRITE_KEY_MAX_GREEN && b >= BMP_SPRITE_KEY_MIN_MAGENTA_BLUE;
}

static bool IsBmpSpriteRedKeyColor(color_t color)
{
    unsigned char r = GetPackedColorRed(color);
    unsigned char g = GetPackedColorGreen(color);
    unsigned char b = GetPackedColorBlue(color);
    unsigned char a = GetPackedColorAlpha(color);

    return a != 0 && r >= BMP_SPRITE_KEY_MIN_RED && g <= BMP_SPRITE_KEY_MAX_GREEN && b <= BMP_SPRITE_KEY_MAX_RED_BLUE;
}

static color_t GetRgba32SurfacePixel(const SDL_Surface* surface, int x, int y)
{
    const unsigned char* row = (const unsigned char*)surface->pixels + y * surface->pitch;
    return ((const color_t*)row)[x];
}

static bmp_sprite_transparency_key_t GetBmpSpriteTransparencyKey(const SDL_Surface* surface)
{
    color_t key = GetRgba32SurfacePixel(surface, 0, 0);
    if (IsBmpSpriteMagentaKeyColor(key))
    {
        return BMP_SPRITE_TRANSPARENCY_KEY_MAGENTA;
    }
    if (IsBmpSpriteRedKeyColor(key))
    {
        return BMP_SPRITE_TRANSPARENCY_KEY_RED;
    }

    return BMP_SPRITE_TRANSPARENCY_KEY_NONE;
}

static color_t ApplyBmpSpriteTransparencyKey(color_t color, bmp_sprite_transparency_key_t transparencyKey)
{
    if (transparencyKey == BMP_SPRITE_TRANSPARENCY_KEY_MAGENTA && IsBmpSpriteMagentaKeyColor(color))
    {
        return FULLY_TRANSPARENT_BLACK;
    }
    if (transparencyKey == BMP_SPRITE_TRANSPARENCY_KEY_RED && IsBmpSpriteRedKeyColor(color))
    {
        return FULLY_TRANSPARENT_BLACK;
    }

    return color;
}

static bool AddBmpTextureFromFile(const char* path, const char* name)
{
    SDL_Surface* source = SDL_LoadBMP(path);
    if (source == NULL)
    {
        return false;
    }

    SDL_Surface* converted = SDL_ConvertSurfaceFormat(source, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(source);

    if (converted == NULL)
    {
        PRINT_ERROR_LOADING("Error converting texture file %s\n", path);
        return false;
    }

    size_t pixelCount = (size_t)converted->w * (size_t)converted->h;
    color_t* pixels = (color_t*)malloc(pixelCount * sizeof(color_t));
    if (pixels == NULL)
    {
        SDL_FreeSurface(converted);
        PRINT_ERROR_LOADING("Error allocating texture file %s\n", path);
        return false;
    }

    for (int y = 0; y < converted->h; y++)
    {
        const unsigned char* sourceRow = (const unsigned char*)converted->pixels + y * converted->pitch;
        memcpy(&pixels[y * converted->w], sourceRow, (size_t)converted->w * sizeof(color_t));
    }

    bool added = AddTextureData(name, converted->w, converted->h, pixels, true);
    SDL_FreeSurface(converted);
    return added;
}

static bool AddBmpSpriteTextureFromFile(const char* path, const char* name)
{
    SDL_Surface* source = SDL_LoadBMP(path);
    if (source == NULL)
    {
        return false;
    }

    SDL_Surface* converted = SDL_ConvertSurfaceFormat(source, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(source);

    if (converted == NULL)
    {
        PRINT_ERROR_LOADING("Error converting sprite file %s\n", path);
        return false;
    }

    size_t pixelCount = (size_t)converted->w * (size_t)converted->h;
    color_t* pixels = (color_t*)malloc(pixelCount * sizeof(color_t));
    if (pixels == NULL)
    {
        SDL_FreeSurface(converted);
        PRINT_ERROR_LOADING("Error allocating sprite file %s\n", path);
        return false;
    }

    bmp_sprite_transparency_key_t transparencyKey = GetBmpSpriteTransparencyKey(converted);

    for (int y = 0; y < converted->h; y++)
    {
        const color_t* sourceRow = (const color_t*)((const unsigned char*)converted->pixels + y * converted->pitch);
        color_t* destRow = &pixels[y * converted->w];
        for (int x = 0; x < converted->w; x++)
        {
            destRow[x] = ApplyBmpSpriteTransparencyKey(sourceRow[x], transparencyKey);
        }
    }

    bool added = AddSpriteTextureData(name, converted->w, converted->h, pixels, true);
    SDL_FreeSurface(converted);
    return added;
}

static bool AddPngSpriteTextureFromFile(const char* path, const char* name)
{
    upng_t* upng = upng_new_from_file(path);
    if (upng == NULL)
    {
        return false;
    }

    upng_decode(upng);
    if (upng_get_error(upng) != UPNG_EOK)
    {
        PRINT_ERROR_LOADING("Error loading sprite file %s\n", path);
        upng_free(upng);
        return false;
    }

    int width = (int)upng_get_width(upng);
    int height = (int)upng_get_height(upng);
    unsigned components = upng_get_components(upng);
    unsigned bitDepth = upng_get_bitdepth(upng);
    const unsigned char* sourcePixels = upng_get_buffer(upng);
    size_t pixelCount = (size_t)width * (size_t)height;
    color_t* pixels = (color_t*)malloc(pixelCount * sizeof(color_t));
    if (pixels == NULL)
    {
        PRINT_ERROR_LOADING("Error allocating sprite file %s\n", path);
        upng_free(upng);
        return false;
    }

    if (components == 4 && bitDepth == 8)
    {
        memcpy(pixels, sourcePixels, pixelCount * sizeof(color_t));
    }
    else if (components == 3 && bitDepth == 8)
    {
        for (size_t i = 0; i < pixelCount; i++)
        {
            unsigned char r = sourcePixels[i * 3 + 0];
            unsigned char g = sourcePixels[i * 3 + 1];
            unsigned char b = sourcePixels[i * 3 + 2];
            pixels[i] = 0xFF000000 | ((color_t)b << 16) | ((color_t)g << 8) | (color_t)r;
        }
    }
    else
    {
        PRINT_ERROR_LOADING("Unsupported sprite PNG format %s\n", path);
        free(pixels);
        upng_free(upng);
        return false;
    }

    bool added = AddSpriteTextureData(name, width, height, pixels, true);
    upng_free(upng);
    return added;
}

static void AddBuiltinSpriteTexture(int textureIndex)
{
    if (textureIndex < 0 || textureIndex >= NUM_BUILTIN_TEXTURES || _Textures[textureIndex] == NULL)
    {
        return;
    }

    AddSpriteTextureData(
        _TextureNames[textureIndex],
        (int)upng_get_width(_Textures[textureIndex]),
        (int)upng_get_height(_Textures[textureIndex]),
        (const color_t*)upng_get_buffer(_Textures[textureIndex]),
        false);
}

static void LoadBuiltinSpriteTextures(void)
{
    AddBuiltinSpriteTexture(TEXTURE_BARREL);
    AddBuiltinSpriteTexture(TEXTURE_LIGHT);
    AddBuiltinSpriteTexture(TEXTURE_TABLE);
    AddBuiltinSpriteTexture(TEXTURE_GUARD);
    AddBuiltinSpriteTexture(TEXTURE_ARMOR);
}

static int CompareCaseInsensitive(const char* lhs, const char* rhs)
{
    while (*lhs != '\0' && *rhs != '\0')
    {
        int left = tolower((unsigned char)*lhs);
        int right = tolower((unsigned char)*rhs);
        if (left != right)
        {
            return left - right;
        }
        lhs++;
        rhs++;
    }

    return tolower((unsigned char)*lhs) - tolower((unsigned char)*rhs);
}

static int CompareTextureFileEntries(const void* lhs, const void* rhs)
{
    const texture_file_entry_t* left = (const texture_file_entry_t*)lhs;
    const texture_file_entry_t* right = (const texture_file_entry_t*)rhs;
    return CompareCaseInsensitive(left->name, right->name);
}

static bool HasFileExtension(const char* fileName, const char* extension)
{
    const char* dot = strrchr(fileName, '.');
    if (dot == NULL)
    {
        return false;
    }

    return CompareCaseInsensitive(dot, extension) == 0;
}

static bool HasSupportedSpriteExtension(const char* fileName)
{
    return HasFileExtension(fileName, ".bmp") || HasFileExtension(fileName, ".png");
}

static void AddDiscoveredSpriteFile(texture_file_entry_t* entries, int* count, const char* directory, const char* fileName)
{
    if (*count >= MAX_DISCOVERED_SPRITE_FILES || !HasSupportedSpriteExtension(fileName))
    {
        return;
    }

    snprintf(entries[*count].path, sizeof(entries[*count].path), "%s/%s", directory, fileName);
    snprintf(entries[*count].name, sizeof(entries[*count].name), "%s", fileName);
    (*count)++;
}

static int DiscoverSpriteFiles(const char* directory, texture_file_entry_t* entries)
{
    int count = 0;

#ifdef _WIN32
    char searchPath[MAX_ASSET_PATH_LENGTH];
    snprintf(searchPath, sizeof(searchPath), "%s/*", directory);

    WIN32_FIND_DATAA findData;
    HANDLE findHandle = FindFirstFileA(searchPath, &findData);
    if (findHandle == INVALID_HANDLE_VALUE)
    {
        return 0;
    }

    do
    {
        if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
        {
            AddDiscoveredSpriteFile(entries, &count, directory, findData.cFileName);
        }
    } while (FindNextFileA(findHandle, &findData));

    FindClose(findHandle);
#else
    DIR* dir = opendir(directory);
    if (dir == NULL)
    {
        return 0;
    }

    struct dirent* entry = NULL;
    while ((entry = readdir(dir)) != NULL)
    {
        AddDiscoveredSpriteFile(entries, &count, directory, entry->d_name);
    }

    closedir(dir);
#endif

    qsort(entries, (size_t)count, sizeof(texture_file_entry_t), CompareTextureFileEntries);
    return count;
}

static void LoadSpriteTexturesFromDirectory(const char* directory)
{
    texture_file_entry_t entries[MAX_DISCOVERED_SPRITE_FILES];
    int count = DiscoverSpriteFiles(directory, entries);

    for (int i = 0; i < count && _SpriteTextureCount < MAX_SPRITE_TEXTURES; i++)
    {
        if (HasFileExtension(entries[i].name, ".bmp"))
        {
            AddBmpSpriteTextureFromFile(entries[i].path, entries[i].name);
        }
        else if (HasFileExtension(entries[i].name, ".png"))
        {
            AddPngSpriteTextureFromFile(entries[i].path, entries[i].name);
        }
    }
}

static void LoadWallBmpTextures(void)
{
    for (int i = 0; i <= 999 && _TextureCount < MAX_TEXTURES; i++)
    {
        char path[128];
        char name[64];
        snprintf(path, sizeof(path), "./resources/images/walls/%d.bmp", i);
        snprintf(name, sizeof(name), "walls/%d.bmp", i);
        AddBmpTextureFromFile(path, name);
    }
}

void LoadTextures(void)
{
    FreeTextures();

    for (int i = 0; i < NUM_BUILTIN_TEXTURES; i++)
    {
        upng_t* upng = upng_new_from_file(_TextureFileNames[i]);
        if (upng != NULL)
        {
            upng_decode(upng);
            if (upng_get_error(upng) == UPNG_EOK)
            {
                _Textures[i] = upng;
                AddTextureData(
                    _TextureNames[i],
                    (int)upng_get_width(upng),
                    (int)upng_get_height(upng),
                    (const color_t*)upng_get_buffer(upng),
                    false);
            }
            else
            {
                PRINT_ERROR_LOADING("Error decode texture file %s \n", _TextureFileNames[i]);
                upng_free(upng);
            }
        }
        else
        {
            PRINT_ERROR_LOADING("Error loading texture file %s \n", _TextureFileNames[i]);
        }
    }

    LoadWallBmpTextures();
    LoadBuiltinSpriteTextures();
    LoadSpriteTexturesFromDirectory("./resources/sprites");
}

void FreeTextures(void)
{
    for (int i = 0; i < MAX_SPRITE_TEXTURES; i++)
    {
        if (_SpriteTextureOwnsPixels[i] && _SpriteTextureData[i].pixels != NULL)
        {
            free((void*)_SpriteTextureData[i].pixels);
        }

        _SpriteTextureData[i].width = 0;
        _SpriteTextureData[i].height = 0;
        _SpriteTextureData[i].pixels = NULL;
        _SpriteTextureData[i].name[0] = '\0';
        _SpriteTextureOwnsPixels[i] = false;
    }
    _SpriteTextureCount = 0;

    for (int i = 0; i < MAX_TEXTURES; i++)
    {
        if (_TextureOwnsPixels[i] && _TextureData[i].pixels != NULL)
        {
            free((void*)_TextureData[i].pixels);
        }

        _TextureData[i].width = 0;
        _TextureData[i].height = 0;
        _TextureData[i].pixels = NULL;
        _TextureData[i].name[0] = '\0';
        _TextureOwnsPixels[i] = false;
    }
    _TextureCount = 0;

    for (int i = 0; i < NUM_BUILTIN_TEXTURES; i++)
    {
        if (_Textures[i] != NULL)
        {
            upng_free(_Textures[i]);
            _Textures[i] = NULL;
        }
    }
}

int GetTextureCount(void)
{
    return _TextureCount;
}

const texture_data_t* GetTextureData(int textureIndex)
{
    if (textureIndex < 0 || textureIndex >= _TextureCount)
    {
        return NULL;
    }

    return &_TextureData[textureIndex];
}

const char* GetTextureName(int textureIndex)
{
    const texture_data_t* texture = GetTextureData(textureIndex);
    return texture != NULL ? texture->name : "Unknown";
}

int GetSpriteTextureCount(void)
{
    return _SpriteTextureCount;
}

const texture_data_t* GetSpriteTextureData(int textureIndex)
{
    if (textureIndex < 0 || textureIndex >= _SpriteTextureCount)
    {
        return NULL;
    }

    return &_SpriteTextureData[textureIndex];
}

const char* GetSpriteTextureName(int textureIndex)
{
    const texture_data_t* texture = GetSpriteTextureData(textureIndex);
    return texture != NULL ? texture->name : "Unknown";
}
