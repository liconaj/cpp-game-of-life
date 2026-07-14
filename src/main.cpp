#define SDL_MAIN_USE_CALLBACKS 1

#include "imgui.h"
#include "imgui_extensions.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include "imgui_internal.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <string>

namespace {

constexpr int WindowWidth = 900;
constexpr int WindowHeight = 600;
constexpr int MinimumWindowWidth = 600;
constexpr int MinimumWindowHeight = 400;

struct AppState
{
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    bool vsyncEnabled = true;
    double framerate = 0.0;
    double frameTimeMs = 0.0;

    bool minimized = false;

    bool showDearImGuiDemo = false;
    bool showStatusBar = true;

    bool layoutInitalized = false;
    bool running = true;

    float lastScaleX = 0;
    float lastScaleY = 0;
};

void updateRenderVsync(AppState& state)
{
    int vsync = state.vsyncEnabled ? 1 : SDL_RENDERER_VSYNC_DISABLED;
    if (SDL_SetRenderVSync(state.renderer, vsync) == false) {
        SDL_Log("Couldn't set renderer with vsync %d: %s", vsync, SDL_GetError());
        state.vsyncEnabled = false;
    }
}

void buildDockLayout(ImGuiID dockspaceId, ImGuiViewport* viewport)
{
    if (ImGui::DockBuilderGetNode(dockspaceId) != nullptr) {
        return;
    }

    ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspaceId, viewport->WorkSize);
    ImGui::DockBuilderDockWindow("World Grid", dockspaceId);
    ImGui::DockBuilderFinish(dockspaceId);
}

void handleHighDpi(AppState& state)
{
    int width{};
    int height{};
    int widthPixels{};
    int heightPixels{};
    SDL_GetWindowSize(state.window, &width, &height);
    SDL_GetWindowSizeInPixels(state.window, &widthPixels, &heightPixels);
    if (width == 0 || height == 0) {
        return;
    }
    float scaleX = static_cast<float>(widthPixels) / width;
    float scaleY = static_cast<float>(heightPixels) / height;
    if (scaleX != state.lastScaleX && scaleY != state.lastScaleY) {
        SDL_SetRenderScale(state.renderer, scaleX, scaleY);
        state.lastScaleX = scaleX;
        state.lastScaleY = scaleY;
    }
}

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

    state.window = SDL_CreateWindow(
        appName.c_str(), WindowWidth, WindowHeight, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY
    );
    SDL_SetWindowMinimumSize(state.window, MinimumWindowWidth, MinimumWindowHeight);
    if (state.window == nullptr) {
        SDL_Log("Couldn't create window: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    state.renderer = SDL_CreateRenderer(state.window, nullptr);
    if (state.renderer == nullptr) {
        SDL_Log("Couldn't create renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    updateRenderVsync(state);
    handleHighDpi(state);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Load font
    io.FontDefault = io.Fonts->AddFontFromFileTTF("assets/fonts/Inter/Inter_18pt-Regular.ttf");
    io.ConfigDpiScaleFonts = true;
    io.ConfigDpiScaleViewports = true;

    // Style
    float displayScale = SDL_GetWindowDisplayScale(state.window);
    ImGuiStyle& style = ImGui::GetStyle();
    style.FontSizeBase = 18.0f;
    style.FontScaleDpi = displayScale;
    style.Colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);

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
    auto& state = *static_cast<AppState*>(appstate);

    ImGui_ImplSDL3_ProcessEvent(event);

    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }

    if (event->type == SDL_EVENT_WINDOW_MINIMIZED) {
        state.minimized = true;
    } else if (event->type == SDL_EVENT_WINDOW_SHOWN) {
        state.minimized = false;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
    auto& state = *static_cast<AppState*>(appstate);

    handleHighDpi(state);

    // Star the Dear ImGui frame
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    if (!state.minimized) {
        ImGuiID dockspaceId = ImGui::GetID("My DockSpace");
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        buildDockLayout(dockspaceId, viewport);
        ImGui::DockSpaceOverViewport(
            dockspaceId, viewport, ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoDockingOverCentralNode
        );

        // --------------------------------------------------------------------------
        // Start menu and status bar configuration
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

        // Main menu bar
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("Menu")) {
                if (ImGui::BeginMenu("Settings")) {
                    if (ImGui::Checkbox("Enable VSync", &state.vsyncEnabled)) {
                        updateRenderVsync(state);
                    }
                    ImGui::EndMenu();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Quit", "Alt+F4")) {
                    state.running = false;
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View")) {
                ImGui::MenuItem("Status bar", nullptr, &state.showStatusBar);
                ImGui::Separator();
                ImGui::MenuItem("Dear ImGui Demo", nullptr, &state.showDearImGuiDemo);
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        // Main Status bar
        if (state.showStatusBar) {
            ImGui::PushFont(nullptr, 18.0);
            if (ImGui::BeginMainStatusBar()) {
                if (ImGui::BeginTable("Status", 1)) {
                    ImGui::TableNextColumn();
                    ImGui::Text("FPS: %3.1f (%2.1f ms)", state.framerate, state.frameTimeMs);
                    ImGui::EndTable();
                }
                ImGui::EndMainStatusBar();
            }
            ImGui::PopFont();
        }

        ImGui::PopStyleVar();
        // End menu and status bar configuration
        // --------------------------------------------------------------------------

        if (state.showDearImGuiDemo) {
            ImGui::ShowDemoWindow(&state.showDearImGuiDemo);
        }

        // For some reason the settings of the window must be saved, otherwise it will not be docked as
        // configured in buildDockLayout(). This window must no be moved so it always stay in the central node
        ImGui::Begin("World Grid", nullptr, ImGuiWindowFlags_NoMove);
        ImGui::End();
    }

    // Rendering
    ImGui::Render();
    SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(state.renderer);

    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), state.renderer);
    SDL_RenderPresent(state.renderer);

    state.framerate = ImGui::GetIO().Framerate;
    state.frameTimeMs = 1000.0 / state.framerate;

    if (state.running == false) {
        return SDL_APP_SUCCESS;
    }

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
