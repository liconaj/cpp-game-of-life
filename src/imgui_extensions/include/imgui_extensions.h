#pragma once

// Dear ImGui extensions.
// These helpers provide functionality that is not part of the official API.

namespace ImGui {

// Creates a status bar attached to the bottom of the main viewport.
// Must be paired with EndMainStatusBar().
// Mirrors the behavior of BeginMainMenuBar().
bool BeginMainStatusBar();

// Ends a status bar created with BeginMainStatusBar().
void EndMainStatusBar();

}