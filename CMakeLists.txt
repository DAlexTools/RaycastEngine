cmake_minimum_required(VERSION 3.16)

project(RaycastEngine LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(RAYCAST_ENGINE_TARGET RaycastEngineApp)
set(RAYCAST_EDITOR_TARGET RaycastMapEditor)

include(FetchContent)

set(RAYCAST_CORE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Core/Graphics.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Core/Map.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Core/Player.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Core/Ray.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Core/Sprite.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Core/Textures.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Core/Utilities/Utils.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Core/upng.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Core/Wall.c"
)

set(RAYCAST_EDITOR_MODULE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Editor/Editor.cpp"
)

set(RAYCAST_GAME_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Game/Main.c"
)

set(RAYCAST_EDITOR_APP_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Editor/MapEditorApp.cpp"
)

add_executable(${RAYCAST_ENGINE_TARGET}
    ${RAYCAST_CORE_SOURCES}
    ${RAYCAST_EDITOR_MODULE_SOURCES}
    ${RAYCAST_GAME_SOURCES}
)

add_executable(${RAYCAST_EDITOR_TARGET}
    ${RAYCAST_CORE_SOURCES}
    ${RAYCAST_EDITOR_MODULE_SOURCES}
    ${RAYCAST_EDITOR_APP_SOURCES}
)

foreach(RAYCAST_TARGET IN ITEMS ${RAYCAST_ENGINE_TARGET} ${RAYCAST_EDITOR_TARGET})
    target_include_directories(${RAYCAST_TARGET} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/Core"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/Core/Utilities"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/Editor"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/Game"
    )
endforeach()

find_package(SDL2 CONFIG QUIET)
if(NOT TARGET SDL2::SDL2 AND NOT TARGET SDL2::SDL2-static)
    set(SDL_TEST OFF CACHE BOOL "" FORCE)
    set(SDL_SHARED OFF CACHE BOOL "" FORCE)
    set(SDL_STATIC ON CACHE BOOL "" FORCE)

    FetchContent_Declare(
        SDL2
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG release-2.32.10
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(SDL2)
endif()

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.92.8
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(imgui)
FetchContent_GetProperties(imgui)

set(IMGUI_DIR "${imgui_SOURCE_DIR}")
set(IMGUI_SOURCES
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_sdlrenderer2.cpp"
)

foreach(RAYCAST_TARGET IN ITEMS ${RAYCAST_ENGINE_TARGET} ${RAYCAST_EDITOR_TARGET})
    target_sources(${RAYCAST_TARGET} PRIVATE
        ${IMGUI_SOURCES}
    )

    target_include_directories(${RAYCAST_TARGET} PRIVATE
        "${IMGUI_DIR}"
        "${IMGUI_DIR}/backends"
    )

    if(TARGET SDL2::SDL2main)
        target_link_libraries(${RAYCAST_TARGET} PRIVATE SDL2::SDL2main)
    endif()

    if(TARGET SDL2::SDL2-static)
        target_link_libraries(${RAYCAST_TARGET} PRIVATE SDL2::SDL2-static)
    elseif(TARGET SDL2::SDL2)
        target_link_libraries(${RAYCAST_TARGET} PRIVATE SDL2::SDL2)
    elseif(TARGET SDL2-static)
        target_link_libraries(${RAYCAST_TARGET} PRIVATE SDL2-static)
    elseif(TARGET SDL2)
        target_link_libraries(${RAYCAST_TARGET} PRIVATE SDL2)
    else()
        message(FATAL_ERROR "SDL2 target was not created by find_package() or FetchContent.")
    endif()

    if(MSVC)
        target_compile_options(${RAYCAST_TARGET} PRIVATE /W4)
        target_compile_definitions(${RAYCAST_TARGET} PRIVATE _CRT_SECURE_NO_WARNINGS)
    else()
        target_compile_options(${RAYCAST_TARGET} PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    if(UNIX AND NOT APPLE)
        target_link_libraries(${RAYCAST_TARGET} PRIVATE m)
    endif()

endforeach()

set(RAYCAST_RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources")
if(NOT EXISTS "${RAYCAST_RESOURCE_DIR}")
    message(FATAL_ERROR "Resource directory not found: ${RAYCAST_RESOURCE_DIR}")
endif()

add_custom_target(RaycastCopyAssets
    COMMAND "${CMAKE_COMMAND}" -E remove_directory
            "${CMAKE_BINARY_DIR}/$<CONFIG>/images"
    COMMAND "${CMAKE_COMMAND}" -E remove_directory
            "${CMAKE_BINARY_DIR}/$<CONFIG>/maps"
    COMMAND "${CMAKE_COMMAND}" -E remove_directory
            "${CMAKE_BINARY_DIR}/$<CONFIG>/sprites"
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
            "${RAYCAST_RESOURCE_DIR}"
            "${CMAKE_BINARY_DIR}/$<CONFIG>/resources"
)

add_dependencies(${RAYCAST_ENGINE_TARGET} RaycastCopyAssets)
add_dependencies(${RAYCAST_EDITOR_TARGET} RaycastCopyAssets)
