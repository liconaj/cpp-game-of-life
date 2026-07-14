add_executable(gameoflife
    main.cpp
)

add_subdirectory(imgui_extensions)

target_link_libraries(gameoflife
    PRIVATE
        SDL3::SDL3
        ImGui::ImplSDL3
        ImGui::Extensions
)

add_custom_command(
    TARGET gameoflife POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_directory "${PROJECT_SOURCE_DIR}/assets" $<TARGET_FILE_DIR:gameoflife>/assets
)

if(WIN32)
    add_custom_command(
        TARGET gameoflife POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy $<TARGET_RUNTIME_DLLS:gameoflife> $<TARGET_FILE_DIR:gameoflife>
        VERBATIM
    )
endif()
