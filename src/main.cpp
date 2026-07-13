#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <string>

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

namespace {

constexpr int WindowWidth = 900;
constexpr int WindowHeight = 640;

struct AppState
{
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
};

} // namespace

extern "C" {
SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
    const std::string appName = "Game of Life";
    const std::string appVersion = "0.1.0";
    const std::string appIdentifier = "com.liconaj.cpp-game-of-life";
    SDL_SetAppMetadata(appName.c_str(), appVersion.c_str(), appIdentifier.c_str());

    *appstate = new AppState();
    auto& state = *static_cast<AppState*>(*appstate);

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    state.window = SDL_CreateWindow(appName.c_str(), WindowWidth, WindowHeight, SDL_WINDOW_RESIZABLE);
    if (state.window == nullptr) {
        SDL_Log("Couldn't create window: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    state.renderer = SDL_CreateRenderer(state.window, nullptr);
    if (state.renderer == nullptr) {
        SDL_Log("Couldn't create renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Load font
    io.FontDefault = io.Fonts->AddFontFromFileTTF("assets/fonts/Inter/Inter_18pt-Regular.ttf", 18.0);

    // Setup Platform / Renderer backends

    if (!ImGui_ImplSDL3_InitForSDLRenderer(state.window, state.renderer)) {
        SDL_Log("Couldn't initialize imgui");
        return SDL_APP_FAILURE;
    }
    if (!ImGui_ImplSDLRenderer3_Init(state.renderer)) {
        SDL_Log("Couldn't initialize sdl renderer");
        return SDL_APP_FAILURE;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    ImGui_ImplSDL3_ProcessEvent(event);

    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
    auto& state = *static_cast<AppState*>(appstate);

    // Star the Dear ImGui frame
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport();

    ImGui::ShowDemoWindow();

    // Rendering
    SDL_SetRenderDrawColor(state.renderer, 100, 100, 100, 255);
    SDL_RenderClear(state.renderer);
    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), state.renderer);
    SDL_RenderPresent(state.renderer);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    auto const* state = static_cast<AppState*>(appstate);
    SDL_DestroyRenderer(state->renderer);
    SDL_DestroyWindow(state->window);
    delete state;

    SDL_Quit();
}

}
