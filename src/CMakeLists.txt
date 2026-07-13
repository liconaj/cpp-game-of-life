add_executable(gameoflife
    main.cpp
)

target_link_libraries(gameoflife PRIVATE SDL3::SDL3)

if(WIN32)
    add_custom_command(
        TARGET gameoflife POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy $<TARGET_RUNTIME_DLLS:gameoflife> $<TARGET_FILE_DIR:gameoflife>
        VERBATIM
    )
endif()