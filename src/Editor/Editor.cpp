#include "Editor.h"

#include "Graphics.h"
#include "Map.h"
#include "Sprite.h"
#include "Textures.h"

#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_sdlrenderer2.h"

#include <cstdio>
#include <cstdint>
#include <cstring>

static bool gEditorInitialized = false;
static bool gEditorOpen = false;
static int gSelectedTile = 1;
static int gSelectedSpriteTexture = 0;
static bool gAllowBorderPaint = false;
static float gCellSize = 24.0f;
static float gThumbnailSize = 56.0f;
static int gResizeRows = DEFAULT_MAP_ROWS;
static int gResizeCols = DEFAULT_MAP_COLS;
static int gLastMapRows = 0;
static int gLastMapCols = 0;
static char gStatusText[128] = "F1 toggles the map editor.";
static SDL_Texture* gTexturePreviews[MAX_TEXTURES] = {};
static SDL_Texture* gSpriteTexturePreviews[MAX_SPRITE_TEXTURES] = {};

typedef enum editor_paint_mode_t
{
    EDITOR_PAINT_TILES,
    EDITOR_PAINT_SPRITES
} editor_paint_mode_t;

static editor_paint_mode_t gPaintMode = EDITOR_PAINT_TILES;

static const char* GetTileName(int tile)
{
    if (tile == MAP_DOOR_TILE)
    {
        return "Door";
    }

    if (tile == 0)
    {
        return "Empty";
    }

    return GetTextureName(tile - 1);
}

static ImU32 GetTileColor(int tile)
{
    switch (tile)
    {
        case MAP_DOOR_TILE: return IM_COL32(109, 70, 38, 255);
        case 0: return IM_COL32(24, 26, 31, 255);
        case 1: return IM_COL32(150, 45, 36, 255);
        case 2: return IM_COL32(116, 77, 145, 255);
        case 3: return IM_COL32(83, 120, 75, 255);
        case 4: return IM_COL32(116, 116, 116, 255);
        case 5: return IM_COL32(180, 115, 60, 255);
        case 6: return IM_COL32(63, 91, 150, 255);
        case 7: return IM_COL32(132, 94, 54, 255);
        case 8: return IM_COL32(214, 202, 150, 255);
        case 9: return IM_COL32(94, 141, 202, 255);
        case 10: return IM_COL32(130, 86, 54, 255);
        case 11: return IM_COL32(245, 211, 88, 255);
        case 12: return IM_COL32(95, 72, 49, 255);
        case 13: return IM_COL32(95, 130, 88, 255);
        case 14: return IM_COL32(138, 138, 174, 255);
        default:
        {
            int value = tile * 73;
            int r = 70 + (value % 145);
            int g = 70 + ((value / 3) % 145);
            int b = 70 + ((value / 7) % 145);
            return IM_COL32(r, g, b, 255);
        }
    }
}

static void FormatTileLabel(int tile, char* buffer, size_t bufferSize)
{
    if (tile == MAP_DOOR_TILE)
    {
        std::snprintf(buffer, bufferSize, "Door");
        return;
    }

    std::snprintf(buffer, bufferSize, "%d - %s", tile, GetTileName(tile));
}

static void FormatSpriteTextureLabel(int textureIndex, char* buffer, size_t bufferSize)
{
    std::snprintf(buffer, bufferSize, "%d - %s", textureIndex + 1, GetSpriteTextureName(textureIndex));
}

static bool IsBorderTile(int row, int col)
{
    int rows = GetMapRows();
    int cols = GetMapCols();
    return row == 0 || row == rows - 1 || col == 0 || col == cols - 1;
}

static void SetStatus(const char* text)
{
    std::snprintf(gStatusText, sizeof(gStatusText), "%s", text);
}

static ImTextureRef GetPreviewTextureRef(SDL_Texture* texture)
{
    return ImTextureRef((ImTextureID)(uintptr_t)texture);
}

static void DestroyTexturePreviews(void)
{
    for (int i = 0; i < MAX_TEXTURES; i++)
    {
        if (gTexturePreviews[i] != NULL)
        {
            SDL_DestroyTexture(gTexturePreviews[i]);
            gTexturePreviews[i] = NULL;
        }
    }

    for (int i = 0; i < MAX_SPRITE_TEXTURES; i++)
    {
        if (gSpriteTexturePreviews[i] != NULL)
        {
            SDL_DestroyTexture(gSpriteTexturePreviews[i]);
            gSpriteTexturePreviews[i] = NULL;
        }
    }
}

static void CreateTexturePreviews(void)
{
    SDL_Renderer* renderer = GetGraphicsRenderer();
    if (renderer == NULL)
    {
        return;
    }

    DestroyTexturePreviews();

    int textureCount = GetTextureCount();
    if (textureCount > MAX_TEXTURES)
    {
        textureCount = MAX_TEXTURES;
    }

    for (int i = 0; i < textureCount; i++)
    {
        const texture_data_t* textureData = GetTextureData(i);
        if (textureData == NULL || textureData->width <= 0 || textureData->height <= 0 || textureData->pixels == NULL)
        {
            continue;
        }

        SDL_Texture* texture = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_RGBA32,
            SDL_TEXTUREACCESS_STATIC,
            textureData->width,
            textureData->height);
        if (texture == NULL)
        {
            continue;
        }

        SDL_UpdateTexture(texture, NULL, textureData->pixels, textureData->width * (int)sizeof(color_t));
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        gTexturePreviews[i] = texture;
    }

    int spriteTextureCount = GetSpriteTextureCount();
    if (spriteTextureCount > MAX_SPRITE_TEXTURES)
    {
        spriteTextureCount = MAX_SPRITE_TEXTURES;
    }

    for (int i = 0; i < spriteTextureCount; i++)
    {
        const texture_data_t* textureData = GetSpriteTextureData(i);
        if (textureData == NULL || textureData->width <= 0 || textureData->height <= 0 || textureData->pixels == NULL)
        {
            continue;
        }

        SDL_Texture* texture = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_RGBA32,
            SDL_TEXTUREACCESS_STATIC,
            textureData->width,
            textureData->height);
        if (texture == NULL)
        {
            continue;
        }

        SDL_UpdateTexture(texture, NULL, textureData->pixels, textureData->width * (int)sizeof(color_t));
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        gSpriteTexturePreviews[i] = texture;
    }
}

static void ApplyEditorInputMode(void)
{
    SDL_SetRelativeMouseMode(gEditorOpen ? SDL_FALSE : SDL_TRUE);
    SDL_ShowCursor(gEditorOpen ? SDL_ENABLE : SDL_DISABLE);
}

static int GetEditorTextureCount(void)
{
    int textureCount = GetTextureCount();
    if (textureCount < 0)
    {
        return 0;
    }
    if (textureCount > MAX_TEXTURES)
    {
        return MAX_TEXTURES;
    }
    return textureCount;
}

static bool DrawPaletteTileButton(int tile, float thumbSize)
{
    bool selected = gSelectedTile == tile;
    ImGui::InvisibleButton("preview", ImVec2(thumbSize, thumbSize));
    bool clicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);
    bool hovered = ImGui::IsItemHovered();

    ImVec2 min = ImGui::GetItemRectMin();
    ImVec2 max = ImGui::GetItemRectMax();
    ImVec2 imageMin(min.x + 2.0f, min.y + 2.0f);
    ImVec2 imageMax(max.x - 2.0f, max.y - 2.0f);
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    ImU32 background = selected ? IM_COL32(128, 94, 38, 255) : IM_COL32(32, 34, 40, 255);
    ImU32 border = selected ? IM_COL32(255, 190, 88, 255) : IM_COL32(82, 86, 96, 255);
    if (hovered && !selected)
    {
        border = IM_COL32(155, 162, 180, 255);
    }

    drawList->AddRectFilled(min, max, background, 4.0f);

    if (tile == MAP_DOOR_TILE)
    {
        int doorTextureIndex = GetDoorTextureIndex();
        if (doorTextureIndex >= 0 && doorTextureIndex < MAX_TEXTURES && gTexturePreviews[doorTextureIndex] != NULL)
        {
            drawList->AddImage(GetPreviewTextureRef(gTexturePreviews[doorTextureIndex]), imageMin, imageMax);
        }
        else
        {
            drawList->AddRectFilled(imageMin, imageMax, GetTileColor(tile), 2.0f);
        }

        float splitX = imageMin.x + (imageMax.x - imageMin.x) * 0.58f;
        drawList->AddLine(ImVec2(splitX, imageMin.y + 4.0f), ImVec2(splitX, imageMax.y - 4.0f), IM_COL32(32, 20, 12, 230), 2.0f);
        drawList->AddCircleFilled(ImVec2(splitX - 5.0f, (imageMin.y + imageMax.y) * 0.5f), 2.2f, IM_COL32(230, 190, 96, 255));
    }
    else if (tile == 0)
    {
        float pad = thumbSize * 0.22f;
        ImVec2 a(min.x + pad, min.y + pad);
        ImVec2 b(max.x - pad, max.y - pad);
        drawList->AddLine(a, b, IM_COL32(170, 176, 190, 255), 2.0f);
        drawList->AddLine(ImVec2(a.x, b.y), ImVec2(b.x, a.y), IM_COL32(170, 176, 190, 255), 2.0f);
    }
    else if (tile - 1 < MAX_TEXTURES && gTexturePreviews[tile - 1] != NULL)
    {
        drawList->AddImage(GetPreviewTextureRef(gTexturePreviews[tile - 1]), imageMin, imageMax);
    }
    else
    {
        drawList->AddRectFilled(imageMin, imageMax, GetTileColor(tile), 2.0f);
    }

    drawList->AddRect(min, max, border, 4.0f, 0, selected ? 3.0f : 1.0f);

    if (hovered)
    {
        if (tile == MAP_DOOR_TILE)
        {
            ImGui::SetTooltip("Door\nSaved as -1");
        }
        else if (tile > 0)
        {
            const texture_data_t* texture = GetTextureData(tile - 1);
            if (texture != NULL)
            {
                ImGui::SetTooltip("%d - %s\n%d x %d", tile, GetTileName(tile), texture->width, texture->height);
            }
            else
            {
                ImGui::SetTooltip("%d - %s", tile, GetTileName(tile));
            }
        }
        else
        {
            ImGui::SetTooltip("0 - Empty");
        }
    }

    return clicked;
}

static bool DrawSpritePaletteButton(int textureIndex, float thumbSize)
{
    bool selected = gSelectedSpriteTexture == textureIndex;
    ImGui::InvisibleButton("sprite_preview", ImVec2(thumbSize, thumbSize));
    bool clicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);
    bool hovered = ImGui::IsItemHovered();

    ImVec2 min = ImGui::GetItemRectMin();
    ImVec2 max = ImGui::GetItemRectMax();
    ImVec2 imageMin(min.x + 2.0f, min.y + 2.0f);
    ImVec2 imageMax(max.x - 2.0f, max.y - 2.0f);
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    ImU32 background = selected ? IM_COL32(48, 92, 76, 255) : IM_COL32(32, 34, 40, 255);
    ImU32 border = selected ? IM_COL32(118, 220, 170, 255) : IM_COL32(82, 86, 96, 255);
    if (hovered && !selected)
    {
        border = IM_COL32(155, 162, 180, 255);
    }

    drawList->AddRectFilled(min, max, background, 4.0f);
    if (textureIndex >= 0 && textureIndex < MAX_SPRITE_TEXTURES && gSpriteTexturePreviews[textureIndex] != NULL)
    {
        drawList->AddImage(GetPreviewTextureRef(gSpriteTexturePreviews[textureIndex]), imageMin, imageMax);
    }
    else
    {
        drawList->AddRectFilled(imageMin, imageMax, IM_COL32(70, 120, 100, 255), 2.0f);
    }
    drawList->AddRect(min, max, border, 4.0f, 0, selected ? 3.0f : 1.0f);

    if (hovered)
    {
        const texture_data_t* texture = GetSpriteTextureData(textureIndex);
        if (texture != NULL)
        {
            ImGui::SetTooltip("%d - %s\n%d x %d", textureIndex + 1, GetSpriteTextureName(textureIndex), texture->width, texture->height);
        }
        else
        {
            ImGui::SetTooltip("%d - %s", textureIndex + 1, GetSpriteTextureName(textureIndex));
        }
    }

    return clicked;
}

static void DrawCenteredTileLabel(int tile, float width)
{
    char label[16];
    if (tile == MAP_DOOR_TILE)
    {
        std::snprintf(label, sizeof(label), "D");
    }
    else
    {
        std::snprintf(label, sizeof(label), "%d", tile);
    }
    ImVec2 textSize = ImGui::CalcTextSize(label);
    float cursorX = ImGui::GetCursorPosX();
    float offset = (width - textSize.x) * 0.5f;
    if (offset > 0.0f)
    {
        ImGui::SetCursorPosX(cursorX + offset);
    }
    ImGui::TextUnformatted(label);
}

static void DrawCenteredSpriteLabel(int textureIndex, float width)
{
    char label[16];
    std::snprintf(label, sizeof(label), "S%d", textureIndex + 1);
    ImVec2 textSize = ImGui::CalcTextSize(label);
    float cursorX = ImGui::GetCursorPosX();
    float offset = (width - textSize.x) * 0.5f;
    if (offset > 0.0f)
    {
        ImGui::SetCursorPosX(cursorX + offset);
    }
    ImGui::TextUnformatted(label);
}

static void FormatSurfaceTextureLabel(int textureIndex, char* buffer, size_t bufferSize)
{
    if (textureIndex == MAP_DEFAULT_SURFACE_TEXTURE_INDEX)
    {
        std::snprintf(buffer, bufferSize, "0 - Default gray");
        return;
    }

    std::snprintf(buffer, bufferSize, "%d - %s", textureIndex + 1, GetTextureName(textureIndex));
}

static void DrawTextureSwatch(const char* id, int textureIndex, float size, ImU32 defaultColor)
{
    ImGui::InvisibleButton(id, ImVec2(size, size));
    bool hovered = ImGui::IsItemHovered();

    ImVec2 min = ImGui::GetItemRectMin();
    ImVec2 max = ImGui::GetItemRectMax();
    ImVec2 imageMin(min.x + 2.0f, min.y + 2.0f);
    ImVec2 imageMax(max.x - 2.0f, max.y - 2.0f);
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    drawList->AddRectFilled(min, max, IM_COL32(32, 34, 40, 255), 4.0f);
    if (textureIndex == MAP_DEFAULT_SURFACE_TEXTURE_INDEX)
    {
        drawList->AddRectFilled(imageMin, imageMax, defaultColor, 2.0f);
    }
    else if (textureIndex >= 0 && textureIndex < MAX_TEXTURES && gTexturePreviews[textureIndex] != NULL)
    {
        drawList->AddImage(GetPreviewTextureRef(gTexturePreviews[textureIndex]), imageMin, imageMax);
    }
    else
    {
        drawList->AddRectFilled(imageMin, imageMax, GetTileColor(textureIndex + 1), 2.0f);
    }
    drawList->AddRect(min, max, hovered ? IM_COL32(155, 162, 180, 255) : IM_COL32(82, 86, 96, 255), 4.0f);

    if (hovered)
    {
        if (textureIndex == MAP_DEFAULT_SURFACE_TEXTURE_INDEX)
        {
            ImGui::SetTooltip("0 - Default gray");
        }
        else
        {
            const texture_data_t* texture = GetTextureData(textureIndex);
            if (texture != NULL)
            {
                ImGui::SetTooltip("%d - %s\n%d x %d", textureIndex + 1, GetTextureName(textureIndex), texture->width, texture->height);
            }
        }
    }
}

static bool DrawTexturePicker(const char* label, int* textureIndex, ImU32 defaultColor)
{
    int textureCount = GetEditorTextureCount();
    if (textureCount <= 0)
    {
        *textureIndex = MAP_DEFAULT_SURFACE_TEXTURE_INDEX;
        ImGui::Text("%s: Default gray", label);
        return false;
    }

    if (*textureIndex < MAP_DEFAULT_SURFACE_TEXTURE_INDEX)
    {
        *textureIndex = MAP_DEFAULT_SURFACE_TEXTURE_INDEX;
    }
    else if (*textureIndex >= textureCount)
    {
        *textureIndex = textureCount - 1;
    }

    bool changed = false;
    char selectedLabel[96];
    FormatSurfaceTextureLabel(*textureIndex, selectedLabel, sizeof(selectedLabel));

    ImGui::PushID(label);
    ImGui::TextUnformatted(label);
    ImGui::SameLine();
    DrawTextureSwatch("swatch", *textureIndex, 38.0f, defaultColor);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(250.0f);
    if (ImGui::BeginCombo("##texture", selectedLabel))
    {
        bool defaultSelected = *textureIndex == MAP_DEFAULT_SURFACE_TEXTURE_INDEX;
        if (ImGui::Selectable("0 - Default gray", defaultSelected))
        {
            *textureIndex = MAP_DEFAULT_SURFACE_TEXTURE_INDEX;
            changed = true;
        }
        if (defaultSelected)
        {
            ImGui::SetItemDefaultFocus();
        }

        for (int index = 0; index < textureCount; index++)
        {
            char itemLabel[96];
            std::snprintf(itemLabel, sizeof(itemLabel), "%d - %s", index + 1, GetTextureName(index));

            bool selected = *textureIndex == index;
            if (ImGui::Selectable(itemLabel, selected))
            {
                *textureIndex = index;
                changed = true;
            }
            if (selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    ImGui::PopID();

    return changed;
}

static void DrawEnvironmentTextureControls(void)
{
    ImGui::SeparatorText("Environment");

    int floorTextureIndex = GetFloorTextureIndex();
    int ceilingTextureIndex = GetCeilingTextureIndex();

    if (ImGui::BeginTable("EnvironmentTextureControls", 2, ImGuiTableFlags_SizingStretchSame))
    {
        ImGui::TableNextColumn();
        if (DrawTexturePicker("Floor", &floorTextureIndex, IM_COL32(136, 136, 136, 255)))
        {
            SetFloorTextureIndex(floorTextureIndex);
            char label[96];
            FormatSurfaceTextureLabel(GetFloorTextureIndex(), label, sizeof(label));
            std::snprintf(gStatusText, sizeof(gStatusText), "Floor texture set to %s.", label);
        }

        ImGui::TableNextColumn();
        if (DrawTexturePicker("Ceiling", &ceilingTextureIndex, IM_COL32(68, 68, 68, 255)))
        {
            SetCeilingTextureIndex(ceilingTextureIndex);
            char label[96];
            FormatSurfaceTextureLabel(GetCeilingTextureIndex(), label, sizeof(label));
            std::snprintf(gStatusText, sizeof(gStatusText), "Ceiling texture set to %s.", label);
        }

        ImGui::EndTable();
    }
}

static void DrawTilePalette(void)
{
    int textureCount = GetEditorTextureCount();
    if (gSelectedTile != MAP_DOOR_TILE && gSelectedTile > textureCount)
    {
        gSelectedTile = textureCount;
    }

    char selectedLabel[96];
    FormatTileLabel(gSelectedTile, selectedLabel, sizeof(selectedLabel));

    ImGui::Text("Selected: %s", selectedLabel);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(190.0f);
    ImGui::SliderFloat("Thumbnail size", &gThumbnailSize, 28.0f, 96.0f, "%.0f px");

    ImGui::SetNextItemWidth(300.0f);
    if (ImGui::BeginCombo("Tile", selectedLabel))
    {
        char doorLabel[96];
        FormatTileLabel(MAP_DOOR_TILE, doorLabel, sizeof(doorLabel));
        bool doorSelected = gSelectedTile == MAP_DOOR_TILE;
        if (ImGui::Selectable(doorLabel, doorSelected))
        {
            gSelectedTile = MAP_DOOR_TILE;
        }
        if (doorSelected)
        {
            ImGui::SetItemDefaultFocus();
        }
        ImGui::Separator();

        for (int tile = 0; tile <= textureCount; tile++)
        {
            char label[96];
            FormatTileLabel(tile, label, sizeof(label));

            bool selected = gSelectedTile == tile;
            if (ImGui::Selectable(label, selected))
            {
                gSelectedTile = tile;
            }
            if (selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndCombo();
    }

    ImGui::SeparatorText("Textures");

    ImGui::BeginChild("TexturePalette", ImVec2(0.0f, 260.0f), true);

    float availableWidth = ImGui::GetContentRegionAvail().x;
    float columnWidth = gThumbnailSize + ImGui::GetStyle().ItemSpacing.x + 12.0f;
    int columns = (int)(availableWidth / columnWidth);
    if (columns < 1)
    {
        columns = 1;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4.0f, 4.0f));
    if (ImGui::BeginTable("TextureGrid", columns, ImGuiTableFlags_SizingFixedFit))
    {
        ImGui::TableNextColumn();
        ImGui::PushID(MAP_DOOR_TILE);
        if (DrawPaletteTileButton(MAP_DOOR_TILE, gThumbnailSize))
        {
            gSelectedTile = MAP_DOOR_TILE;
        }
        DrawCenteredTileLabel(MAP_DOOR_TILE, gThumbnailSize);
        ImGui::PopID();

        for (int tile = 0; tile <= textureCount; tile++)
        {
            ImGui::TableNextColumn();
            ImGui::PushID(tile);

            if (DrawPaletteTileButton(tile, gThumbnailSize))
            {
                gSelectedTile = tile;
            }
            DrawCenteredTileLabel(tile, gThumbnailSize);

            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    ImGui::PopStyleVar();
    ImGui::EndChild();
}

static void DrawSpritePalette(void)
{
    int spriteTextureCount = GetSpriteTextureCount();
    if (spriteTextureCount <= 0)
    {
        ImGui::TextUnformatted("No sprite textures loaded.");
        return;
    }

    if (gSelectedSpriteTexture < 0)
    {
        gSelectedSpriteTexture = 0;
    }
    else if (gSelectedSpriteTexture >= spriteTextureCount)
    {
        gSelectedSpriteTexture = spriteTextureCount - 1;
    }

    char selectedLabel[96];
    FormatSpriteTextureLabel(gSelectedSpriteTexture, selectedLabel, sizeof(selectedLabel));

    ImGui::Text("Selected sprite: %s", selectedLabel);
    ImGui::SameLine();
    if (ImGui::Button("Clear sprites"))
    {
        ClearSprites();
        SetStatus("Sprites cleared.");
    }

    ImGui::SetNextItemWidth(300.0f);
    if (ImGui::BeginCombo("Sprite", selectedLabel))
    {
        for (int textureIndex = 0; textureIndex < spriteTextureCount; textureIndex++)
        {
            char label[96];
            FormatSpriteTextureLabel(textureIndex, label, sizeof(label));

            bool selected = gSelectedSpriteTexture == textureIndex;
            if (ImGui::Selectable(label, selected))
            {
                gSelectedSpriteTexture = textureIndex;
            }
            if (selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndCombo();
    }

    ImGui::SeparatorText("Sprites");
    ImGui::BeginChild("SpritePalette", ImVec2(0.0f, 190.0f), true);

    float availableWidth = ImGui::GetContentRegionAvail().x;
    float columnWidth = gThumbnailSize + ImGui::GetStyle().ItemSpacing.x + 12.0f;
    int columns = (int)(availableWidth / columnWidth);
    if (columns < 1)
    {
        columns = 1;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4.0f, 4.0f));
    if (ImGui::BeginTable("SpriteGrid", columns, ImGuiTableFlags_SizingFixedFit))
    {
        for (int textureIndex = 0; textureIndex < spriteTextureCount; textureIndex++)
        {
            ImGui::TableNextColumn();
            ImGui::PushID(textureIndex);

            if (DrawSpritePaletteButton(textureIndex, gThumbnailSize))
            {
                gSelectedSpriteTexture = textureIndex;
            }
            DrawCenteredSpriteLabel(textureIndex, gThumbnailSize);

            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    ImGui::PopStyleVar();
    ImGui::EndChild();
}

static void SyncResizeFieldsToMap(void)
{
    int rows = GetMapRows();
    int cols = GetMapCols();
    if (rows != gLastMapRows || cols != gLastMapCols)
    {
        gResizeRows = rows;
        gResizeCols = cols;
        gLastMapRows = rows;
        gLastMapCols = cols;
    }
}

static void DrawMapSizeControls(void)
{
    SyncResizeFieldsToMap();

    ImGui::SetNextItemWidth(76.0f);
    ImGui::InputInt("Rows", &gResizeRows, 1, 4);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(76.0f);
    ImGui::InputInt("Cols", &gResizeCols, 1, 4);
    ImGui::SameLine();

    if (ImGui::Button("Resize"))
    {
        if (ResizeMap(gResizeRows, gResizeCols))
        {
            SyncResizeFieldsToMap();
            std::snprintf(gStatusText, sizeof(gStatusText), "Map resized to %d x %d.", GetMapRows(), GetMapCols());
        }
        else
        {
            SetStatus("Resize failed.");
        }
    }
}

static void DrawMapCanvas(void)
{
    ImGuiIO& io = ImGui::GetIO();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const ImVec2 origin = ImGui::GetCursorScreenPos();
    const int rows = GetMapRows();
    const int cols = GetMapCols();
    const ImVec2 canvasSize(cols * gCellSize, rows * gCellSize);

    ImGui::InvisibleButton(
        "map_canvas",
        canvasSize,
        ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);

    bool canvasHovered = ImGui::IsItemHovered();
    int hoverRow = -1;
    int hoverCol = -1;

    if (canvasHovered)
    {
        hoverCol = (int)((io.MousePos.x - origin.x) / gCellSize);
        hoverRow = (int)((io.MousePos.y - origin.y) / gCellSize);

        if (hoverRow >= 0 && hoverRow < rows && hoverCol >= 0 && hoverCol < cols)
        {
            bool canEdit = gAllowBorderPaint || !IsBorderTile(hoverRow, hoverCol);
            if (gPaintMode == EDITOR_PAINT_SPRITES)
            {
                if (canEdit && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                {
                    if (GetMap(hoverRow, hoverCol) == 0 && AddSpriteAtCell(hoverRow, hoverCol, gSelectedSpriteTexture))
                    {
                        char label[96];
                        FormatSpriteTextureLabel(gSelectedSpriteTexture, label, sizeof(label));
                        std::snprintf(gStatusText, sizeof(gStatusText), "Placed sprite %s at %d, %d.", label, hoverRow, hoverCol);
                    }
                    else
                    {
                        SetStatus("Sprite placement failed.");
                    }
                }
                if (canEdit && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
                {
                    if (RemoveSpriteAtCell(hoverRow, hoverCol))
                    {
                        std::snprintf(gStatusText, sizeof(gStatusText), "Removed sprite at %d, %d.", hoverRow, hoverCol);
                    }
                }
            }
            else
            {
                if (canEdit && ImGui::IsMouseDown(ImGuiMouseButton_Left))
                {
                    SetMap(hoverRow, hoverCol, gSelectedTile);
                }
                if (canEdit && ImGui::IsMouseDown(ImGuiMouseButton_Right))
                {
                    SetMap(hoverRow, hoverCol, 0);
                }
            }
        }
    }

    for (int row = 0; row < rows; row++)
    {
        for (int col = 0; col < cols; col++)
        {
            int tile = GetMap(row, col);
            ImVec2 min(origin.x + col * gCellSize, origin.y + row * gCellSize);
            ImVec2 max(min.x + gCellSize, min.y + gCellSize);
            ImU32 fillColor = GetTileColor(tile);
            ImU32 borderColor = IM_COL32(72, 76, 86, 255);

            if (row == hoverRow && col == hoverCol)
            {
                borderColor = IM_COL32(255, 220, 120, 255);
            }
            else if (IsBorderTile(row, col))
            {
                borderColor = IM_COL32(128, 132, 142, 255);
            }

            drawList->AddRectFilled(min, max, fillColor);
            drawList->AddRect(min, max, borderColor);

            if (tile != 0 && gCellSize >= 18.0f)
            {
                char text[8];
                if (tile == MAP_DOOR_TILE)
                {
                    std::snprintf(text, sizeof(text), "D");
                }
                else
                {
                    std::snprintf(text, sizeof(text), "%d", tile);
                }
                ImVec2 textSize = ImGui::CalcTextSize(text);
                ImVec2 textPos(
                    min.x + (gCellSize - textSize.x) * 0.5f,
                    min.y + (gCellSize - textSize.y) * 0.5f);
                drawList->AddText(textPos, IM_COL32(245, 245, 245, 255), text);
            }
        }
    }

    for (int i = 0; i < GetSpriteCount(); i++)
    {
        const sprite_t* sprite = GetSprite(i);
        if (sprite == NULL)
        {
            continue;
        }

        int row = (int)(sprite->y / TILE_SIZE);
        int col = (int)(sprite->x / TILE_SIZE);
        if (row < 0 || row >= rows || col < 0 || col >= cols)
        {
            continue;
        }

        ImVec2 cellMin(origin.x + col * gCellSize, origin.y + row * gCellSize);
        ImVec2 cellMax(cellMin.x + gCellSize, cellMin.y + gCellSize);
        float pad = gCellSize * 0.16f;
        ImVec2 imageMin(cellMin.x + pad, cellMin.y + pad);
        ImVec2 imageMax(cellMax.x - pad, cellMax.y - pad);

        if (sprite->texture >= 0 && sprite->texture < MAX_SPRITE_TEXTURES && gSpriteTexturePreviews[sprite->texture] != NULL)
        {
            drawList->AddImage(GetPreviewTextureRef(gSpriteTexturePreviews[sprite->texture]), imageMin, imageMax);
        }
        else
        {
            drawList->AddCircleFilled(
                ImVec2((cellMin.x + cellMax.x) * 0.5f, (cellMin.y + cellMax.y) * 0.5f),
                gCellSize * 0.24f,
                IM_COL32(118, 220, 170, 255));
        }

        drawList->AddRect(imageMin, imageMax, IM_COL32(118, 220, 170, 255), 3.0f, 0, 1.5f);
    }

    if (hoverRow >= 0 && hoverRow < rows && hoverCol >= 0 && hoverCol < cols)
    {
        char tileLabel[96];
        FormatTileLabel(GetMap(hoverRow, hoverCol), tileLabel, sizeof(tileLabel));
        ImGui::Text("Hover: row %d, col %d, tile %s", hoverRow, hoverCol, tileLabel);

        int spriteIndex = FindSpriteAtCell(hoverRow, hoverCol);
        if (spriteIndex >= 0)
        {
            const sprite_t* sprite = GetSprite(spriteIndex);
            if (sprite != NULL)
            {
                char spriteLabel[96];
                FormatSpriteTextureLabel(sprite->texture, spriteLabel, sizeof(spriteLabel));
                ImGui::Text("Sprite: %s", spriteLabel);
            }
        }
    }
}

extern "C" bool EditorInitialize(void)
{
    if (gEditorInitialized)
    {
        return true;
    }

    SDL_Window* window = GetGraphicsWindow();
    SDL_Renderer* renderer = GetGraphicsRenderer();
    if (window == NULL || renderer == NULL)
    {
        return false;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = "imgui.ini";

    ImGui::StyleColorsDark();

    if (!ImGui_ImplSDL2_InitForSDLRenderer(window, renderer))
    {
        return false;
    }
    if (!ImGui_ImplSDLRenderer2_Init(renderer))
    {
        ImGui_ImplSDL2_Shutdown();
        return false;
    }

    CreateTexturePreviews();

    gEditorInitialized = true;
    return true;
}

extern "C" void EditorShutdown(void)
{
    if (!gEditorInitialized)
    {
        return;
    }

    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    DestroyTexturePreviews();
    ImGui::DestroyContext();
    gEditorInitialized = false;
}

extern "C" void EditorProcessEvent(const SDL_Event* event)
{
    if (gEditorInitialized && event != NULL)
    {
        ImGui_ImplSDL2_ProcessEvent(event);
    }
}

extern "C" void EditorBeginFrame(void)
{
    if (!gEditorInitialized)
    {
        return;
    }

    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

extern "C" void EditorRender(void)
{
    if (!gEditorInitialized)
    {
        return;
    }

    bool wasOpen = gEditorOpen;
    if (gEditorOpen)
    {
        ImGui::SetNextWindowSize(ImVec2(820, 720), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Raycast Map Editor", &gEditorOpen, ImGuiWindowFlags_NoCollapse))
        {
            ImGui::Checkbox("Paint borders", &gAllowBorderPaint);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(170.0f);
            ImGui::SliderFloat("Cell size", &gCellSize, 14.0f, 36.0f, "%.0f");
            ImGui::SameLine();
            DrawMapSizeControls();
            DrawEnvironmentTextureControls();
            if (ImGui::BeginTabBar("PaintModeTabs"))
            {
                if (ImGui::BeginTabItem("Tiles"))
                {
                    gPaintMode = EDITOR_PAINT_TILES;
                    DrawTilePalette();
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Sprites"))
                {
                    gPaintMode = EDITOR_PAINT_SPRITES;
                    DrawSpritePalette();
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }

            if (ImGui::Button("Load"))
            {
                if (LoadMapFromFile(DEFAULT_MAP_FILE_PATH))
                {
                    SyncResizeFieldsToMap();
                    std::snprintf(
                        gStatusText,
                        sizeof(gStatusText),
                        "Loaded resources/maps/map_01.txt (%d x %d, %d sprites).",
                        GetMapRows(),
                        GetMapCols(),
                        GetSpriteCount());
                }
                else
                {
                    SetStatus("Load failed.");
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Save"))
            {
                if (SaveMapToFile(DEFAULT_MAP_FILE_PATH))
                {
                    std::snprintf(
                        gStatusText,
                        sizeof(gStatusText),
                        "Saved resources/maps/map_01.txt (%d x %d, %d sprites).",
                        GetMapRows(),
                        GetMapCols(),
                        GetSpriteCount());
                }
                else
                {
                    SetStatus("Save failed.");
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Clear"))
            {
                ClearMap();
                std::snprintf(gStatusText, sizeof(gStatusText), "Map cleared (%d x %d), border kept, sprites removed.", GetMapRows(), GetMapCols());
            }

            ImGui::TextUnformatted(gStatusText);
            ImGui::Separator();
            ImGui::BeginChild("MapCanvasRegion", ImVec2(0.0f, 0.0f), false, ImGuiWindowFlags_HorizontalScrollbar);
            DrawMapCanvas();
            ImGui::EndChild();
        }
        ImGui::End();
    }

    if (wasOpen != gEditorOpen)
    {
        ApplyEditorInputMode();
    }

    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), GetGraphicsRenderer());
}

extern "C" void EditorToggle(void)
{
    gEditorOpen = !gEditorOpen;
    ApplyEditorInputMode();
}

extern "C" void EditorSetOpen(bool open)
{
    if (gEditorOpen == open)
    {
        return;
    }

    gEditorOpen = open;
    ApplyEditorInputMode();
}

extern "C" bool EditorIsOpen(void)
{
    return gEditorOpen;
}
