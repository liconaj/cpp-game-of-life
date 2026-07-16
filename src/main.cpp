#include "SDL3/SDL_video.h"
#define SDL_MAIN_USE_CALLBACKS 1

#include "imgui.h"
#include "imgui_extensions.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl3.h"
#include "imgui_internal.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL3/SDL_opengles2.h>
#else
#include <SDL3/SDL_opengl.h>
#endif
#include <string>

namespace {

constexpr int WindowWidth = 900;
constexpr int WindowHeight = 600;
constexpr int MinimumWindowWidth = 600;
constexpr int MinimumWindowHeight = 400;

struct AppState
{
    SDL_Window* window = nullptr;
    SDL_GLContext glContext = nullptr;

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
    int vsync = state.vsyncEnabled ? 1 : 0;
    if (SDL_GL_SetSwapInterval(vsync) == false) {
        SDL_Log("Couldn't set GL with vsync %d: %s", vsync, SDL_GetError());
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

    // Select GL version and let the backend select a GLSL version
    const char* glslVersion = nullptr;
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100 (WebGL 1.0)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(IMGUI_IMPL_OPENGL_ES3)
    // GL ES 3.0 + GLSL 300 es (WebGL 2.0)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
    // GL 3.2 Core + generally GLSL 150
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + generally GLSL 130
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // Set OpenGL graphics context and create window
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    float displayScale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());

    state.window = SDL_CreateWindow(
        appName.c_str(), WindowWidth, WindowHeight,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY
    );
    SDL_SetWindowMinimumSize(state.window, MinimumWindowWidth, MinimumWindowHeight);
    if (state.window == nullptr) {
        SDL_Log("Couldn't create window: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    state.glContext = SDL_GL_CreateContext(state.window);
    if (state.glContext == nullptr) {
        SDL_Log("Couldn't create GL context: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_GL_MakeCurrent(state.window, state.glContext);

    updateRenderVsync(state);

    // Show window in the center of the screen
    SDL_SetWindowPosition(state.window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(state.window);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    // Style
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.FontSizeBase = 18.0f;
    style.FontScaleDpi = displayScale;
    style.Colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);

    // Setup Platform / Renderer backends
    if (!ImGui_ImplSDL3_InitForOpenGL(state.window, state.glContext)) {
        SDL_Log("Couldn't initialize imgui");
        return SDL_APP_FAILURE;
    }
    if (!ImGui_ImplOpenGL3_Init(glslVersion)) {
        SDL_Log("Couldn't initialize sdl renderer");
        return SDL_APP_FAILURE;
    }

    // Load font
    io.FontDefault = io.Fonts->AddFontFromFileTTF("assets/fonts/Inter/Inter_18pt-Regular.ttf");
    io.ConfigDpiScaleFonts = true;
    io.ConfigDpiScaleViewports = true;

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

    // Star the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
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
    ImGuiIO& io = ImGui::GetIO();

    // Clear GL viewport
    ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    glViewport(0, 0, io.DisplaySize.x, io.DisplaySize.y);
    glClearColor(clearColor.x * clearColor.w, clearColor.y * clearColor.w, clearColor.z * clearColor.w, clearColor.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update multi-viewports
    if ((io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) != 0) {
        SDL_Window* backupCurrentWindow = SDL_GL_GetCurrentWindow();
        SDL_GLContext backupCurrentContext = SDL_GL_GetCurrentContext();

        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();

        SDL_GL_MakeCurrent(backupCurrentWindow, backupCurrentContext);
    }

    SDL_GL_SwapWindow(state.window);

    state.framerate = ImGui::GetIO().Framerate;
    state.frameTimeMs = 1000.0 / state.framerate;

    if (state.running == false) {
        return SDL_APP_SUCCESS;
    }

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    auto const* state = static_cast<AppState*>(appstate);
    SDL_GL_DestroyContext(state->glContext);
    SDL_DestroyWindow(state->window);
    delete state;

    SDL_Quit();
}
}
