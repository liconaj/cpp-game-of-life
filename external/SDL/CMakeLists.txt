include(FetchContent)


set(SDL_VERSION 3.4.12)

FetchContent_Declare(
    SDL
    URL "${CMAKE_CURRENT_SOURCE_DIR}/SDL3-${SDL_VERSION}.zip"
    URL_HASH MD5=D6DAC2BD7237B70DF3EAEC4E3074C345
)
FetchContent_MakeAvailable(SDL)
