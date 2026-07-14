include(FetchContent)

set(IMGUI_VERSION 1.92.8-docking)

FetchContent_Declare(
    imgui
    URL "${CMAKE_CURRENT_SOURCE_DIR}/imgui-${IMGUI_VERSION}.tar.gz"
    URL_HASH SHA256=CA0653454ED371B7A87E9B0BC29A5D15C9BE7F7C0FBE778042FC48C71DF1D3D8
)
FetchContent_MakeAvailable(imgui)

add_library(imgui
    STATIC
        "${imgui_SOURCE_DIR}/imgui.cpp"
        "${imgui_SOURCE_DIR}/imgui_draw.cpp"
        "${imgui_SOURCE_DIR}/imgui_tables.cpp"
        "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
        "${imgui_SOURCE_DIR}/imgui_demo.cpp"
)
add_library(ImGui::ImGui ALIAS imgui)

target_include_directories(imgui
    PUBLIC "${imgui_SOURCE_DIR}"
)

add_library(imgui_implsdl3
    STATIC
        "${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp"
        "${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer3.cpp"
)
add_library(ImGui::ImplSDL3 ALIAS imgui_implsdl3)

target_include_directories(imgui_implsdl3
    PUBLIC "${imgui_SOURCE_DIR}/backends"
)

target_link_libraries(imgui_implsdl3
    PUBLIC
        ImGui::ImGui
        SDL3::SDL3
)
