add_library(imgui_extensions
    STATIC
        imgui_extensions.cpp
)
add_library(ImGui::Extensions ALIAS imgui_extensions)

target_include_directories(imgui_extensions PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(imgui_extensions PRIVATE ImGui::ImGui)