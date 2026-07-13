add_executable(gameoflife
    main.cpp
)

target_link_libraries(gameoflife
    PRIVATE
        SDL3::SDL3
        ImGui::ImGui
        ImGui::ImplSDL3
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