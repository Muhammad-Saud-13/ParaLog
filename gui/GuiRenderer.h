#pragma once

#include "AppState.h"
#include "imgui.h"

// Forward declarations
struct ID3D11ShaderResourceView;

// ─── GuiRenderer ──────────────────────────────────────────────────────────────
// Draws the entire application UI each frame using Dear ImGui.
// All panels are stateless — all data lives in AppState.
// ─────────────────────────────────────────────────────────────────────────────
namespace GuiRenderer {

    // Call once after ImGui init to load fonts
    void loadFonts(float uiSize = 15.0f, float consoleSize = 13.0f);

    // Draw the full UI — call every frame between NewFrame() and Render()
    void draw(AppState& state, float windowW, float windowH);

} // namespace GuiRenderer
