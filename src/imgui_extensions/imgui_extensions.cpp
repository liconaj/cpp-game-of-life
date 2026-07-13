#include "imgui_extensions.h"

#include "imgui.h"
#include "imgui_internal.h"

// Based on Dear ImGui v1.92.8-docking BeginMainMenuBar() implementation.
//
// This function intentionally mirrors the upstream implementation to provide
// a bottom status bar, with only the following changes:
// - Uses ImGuiDir_Down instead of ImGuiDir_Up.
// - Uses the internal window name "##MainStatusBar".
//
// Keep this implementation in sync with Dear ImGui when upgrading versions
// until a public api is provided for BeginViewportSideBar or for a status bar.
bool ImGui::BeginMainStatusBar()
{
    ImGuiContext& g = *GImGui;
    ImGuiViewportP* viewport = (ImGuiViewportP*)(void*)GetMainViewport();

    // Notify of viewport change so GetFrameHeight() can be accurate in case of DPI change
    SetCurrentViewport(NULL, viewport);

    g.NextWindowData.MenuBarOffsetMinVal = ImVec2(g.Style.DisplaySafeAreaPadding.x, ImMax(g.Style.DisplaySafeAreaPadding.y - g.Style.FramePadding.y, 0.0f));
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
    float height = GetFrameHeight();
    bool is_open = BeginViewportSideBar("##MainStatusBar", viewport, ImGuiDir_Down, height, window_flags);
    g.NextWindowData.MenuBarOffsetMinVal = ImVec2(0.0f, 0.0f);
    if (!is_open)
    {
        End();
        return false;
    }

    // Temporarily disable _NoSavedSettings, in the off-chance that tables or child windows submitted within the menu-bar may want to use settings.
    g.CurrentWindow->Flags &= ~ImGuiWindowFlags_NoSavedSettings;
    BeginMenuBar();
    return is_open;
}

// Based on Dear ImGui v1.92.-docking ImGui::EndMainMenuBar().
//
// This function intentionally mirrors the upstream implementation to provide
// a bottom status bar, with only the change of the function name in the error message.
//
// Keep this implementation in sync with Dear ImGui when upgrading versions.
void ImGui::EndMainStatusBar()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT_USER_ERROR_RET(g.CurrentWindow->DC.MenuBarAppending, "Calling EndMainStatusBar() not from a menu-bar!"); // Not technically testing that it is the main menu bar

    EndMenuBar();
    g.CurrentWindow->Flags |= ImGuiWindowFlags_NoSavedSettings; // Restore _NoSavedSettings

    // When the user has left the menu layer (typically: closed menus through activation of an item), we restore focus to the previous window
    if (g.CurrentWindow == g.NavWindow && g.NavLayer == ImGuiNavLayer_Main && !g.NavAnyRequest && g.ActiveId == 0)
        FocusTopMostWindowUnderOne(g.NavWindow, NULL, NULL, ImGuiFocusRequestFlags_UnlessBelowModal | ImGuiFocusRequestFlags_RestoreFocusedChild);

    End();
}
