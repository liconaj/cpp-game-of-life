#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>


namespace {

struct AppState
{
    SDL_Window *window;
    SDL_Renderer *renderer;
};

}

extern "C" {

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv)
{
    SDL_SetAppMetadata("Example Renderer Clear", "1.0", "com.example.renderer-clear");

    *appstate = new AppState();
    auto* state = static_cast<AppState*>(*appstate);

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("examples/renderer/clear", 640, 480, SDL_WINDOW_RESIZABLE, &state->window, &state->renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_SetRenderLogicalPresentation(state->renderer, 640, 480, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
    AppState& state = *static_cast<AppState*>(appstate);

    const auto now = static_cast<double>(SDL_GetTicks()) / 1000.0;

    const auto red = static_cast<float>(0.5 + 0.5 * SDL_sin(now));
    const auto green = static_cast<float>(0.5 + 0.5 * SDL_sin(now + SDL_PI_D * 2 / 3));
    const auto blue = static_cast<float>(0.5 + 0.5 * SDL_sin(now + SDL_PI_D * 4 / 2));

    SDL_SetRenderDrawColorFloat(state.renderer, red, green, blue, SDL_ALPHA_OPAQUE_FLOAT);
    SDL_RenderClear(state.renderer);
    SDL_RenderPresent(state.renderer);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    /* SDL will clean up the window/renderer for us. */
}

}
