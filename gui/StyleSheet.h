#pragma once
#include "imgui.h"

// ─── ParaLog dark theme ───────────────────────────────────────────────────────
// Call once after ImGui::CreateContext() and before the render loop.
// ─────────────────────────────────────────────────────────────────────────────
namespace StyleSheet {

    // Color tokens
    namespace Col {
        static constexpr ImVec4 BgDeep       { 0.035f, 0.035f, 0.043f, 1.0f }; // #09090b
        static constexpr ImVec4 BgCard       { 0.094f, 0.094f, 0.106f, 1.0f }; // #18181b
        static constexpr ImVec4 BgHeader     { 0.153f, 0.153f, 0.169f, 1.0f }; // #27272a
        static constexpr ImVec4 BgConsole    { 0.020f, 0.020f, 0.024f, 1.0f }; // #050506
        static constexpr ImVec4 Border       { 0.247f, 0.247f, 0.275f, 1.0f }; // #3f3f46
        static constexpr ImVec4 TextPrimary  { 1.000f, 1.000f, 1.000f, 1.000f }; // Pure White (#ffffff)
        static constexpr ImVec4 TextMuted    { 0.706f, 0.706f, 0.733f, 1.000f }; // Lighter Zinc (#b5b5bb)

        // Mode accent colors (Vibrant modern tones)
        static constexpr ImVec4 AccentSerial { 0.545f, 0.361f, 0.965f, 1.0f }; // Violet (#8b5cf6)
        static constexpr ImVec4 AccentOpenMP { 0.063f, 0.725f, 0.506f, 1.0f }; // Emerald (#10b981)
        static constexpr ImVec4 AccentMpi    { 0.957f, 0.247f, 0.365f, 1.0f }; // Rose (#f43f5e)

        // Status colors
        static constexpr ImVec4 StatusRun    { 0.961f, 0.620f, 0.043f, 1.0f }; // Amber (#f59e0b)
        static constexpr ImVec4 StatusDone   { 0.063f, 0.725f, 0.506f, 1.0f }; // Emerald (#10b981)
        static constexpr ImVec4 StatusError  { 0.957f, 0.247f, 0.365f, 1.0f }; // Rose (#f43f5e)
        static constexpr ImVec4 StatusNone   { 0.631f, 0.631f, 0.659f, 1.0f }; // Zinc 400

        // Button hover/active variants
        static constexpr ImVec4 BtnSerialHov { 0.655f, 0.545f, 0.980f, 1.0f };
        static constexpr ImVec4 BtnSerialAct { 0.486f, 0.227f, 0.929f, 1.0f };
        static constexpr ImVec4 BtnOmpHov    { 0.204f, 0.827f, 0.600f, 1.0f };
        static constexpr ImVec4 BtnOmpAct    { 0.020f, 0.588f, 0.412f, 1.0f };
        static constexpr ImVec4 BtnMpiHov    { 0.984f, 0.443f, 0.522f, 1.0f };
        static constexpr ImVec4 BtnMpiAct    { 0.882f, 0.114f, 0.282f, 1.0f };
    }

    inline void apply() {
        ImGuiStyle& s = ImGui::GetStyle();

        // Rounding
        s.WindowRounding   = 10.0f;
        s.ChildRounding    = 8.0f;
        s.FrameRounding    = 6.0f;
        s.PopupRounding    = 6.0f;
        s.ScrollbarRounding= 6.0f;
        s.GrabRounding     = 6.0f;
        s.TabRounding      = 6.0f;

        // Sizing / spacing
        s.WindowPadding    = { 24.0f, 24.0f };
        s.FramePadding     = { 14.0f, 10.0f };
        s.ItemSpacing      = { 12.0f, 12.0f };
        s.ItemInnerSpacing = { 10.0f,  8.0f };
        s.IndentSpacing    = 18.0f;
        s.ScrollbarSize    = 12.0f;
        s.GrabMinSize      = 10.0f;
        s.WindowBorderSize = 1.0f;
        s.FrameBorderSize  = 1.0f;

        ImVec4* c = s.Colors;
        c[ImGuiCol_WindowBg]          = Col::BgDeep;
        c[ImGuiCol_ChildBg]           = Col::BgCard;
        c[ImGuiCol_PopupBg]           = Col::BgCard;
        c[ImGuiCol_Border]            = Col::Border;
        c[ImGuiCol_BorderShadow]      = { 0,0,0,0 };

        c[ImGuiCol_FrameBg]           = Col::BgHeader;
        c[ImGuiCol_FrameBgHovered]    = { 0.17f, 0.20f, 0.25f, 1.0f };
        c[ImGuiCol_FrameBgActive]     = { 0.20f, 0.24f, 0.30f, 1.0f };

        c[ImGuiCol_TitleBg]           = Col::BgHeader;
        c[ImGuiCol_TitleBgActive]     = Col::BgHeader;
        c[ImGuiCol_TitleBgCollapsed]  = Col::BgCard;

        c[ImGuiCol_MenuBarBg]         = Col::BgHeader;
        c[ImGuiCol_ScrollbarBg]       = Col::BgCard;
        c[ImGuiCol_ScrollbarGrab]     = Col::Border;
        c[ImGuiCol_ScrollbarGrabHovered] = { 0.27f, 0.32f, 0.40f, 1.0f };
        c[ImGuiCol_ScrollbarGrabActive]  = Col::AccentSerial;

        c[ImGuiCol_CheckMark]         = Col::AccentSerial;
        c[ImGuiCol_SliderGrab]        = Col::AccentSerial;
        c[ImGuiCol_SliderGrabActive]  = Col::BtnSerialAct;

        c[ImGuiCol_Button]            = Col::BgHeader;
        c[ImGuiCol_ButtonHovered]     = { 0.17f, 0.20f, 0.26f, 1.0f };
        c[ImGuiCol_ButtonActive]      = { 0.22f, 0.26f, 0.33f, 1.0f };

        c[ImGuiCol_Header]            = { 0.22f, 0.26f, 0.33f, 1.0f };
        c[ImGuiCol_HeaderHovered]     = { 0.27f, 0.32f, 0.40f, 1.0f };
        c[ImGuiCol_HeaderActive]      = Col::AccentSerial;

        c[ImGuiCol_Separator]         = Col::Border;
        c[ImGuiCol_SeparatorHovered]  = Col::AccentSerial;
        c[ImGuiCol_SeparatorActive]   = Col::AccentSerial;

        c[ImGuiCol_ResizeGrip]        = { 0,0,0,0 };
        c[ImGuiCol_ResizeGripHovered] = Col::Border;
        c[ImGuiCol_ResizeGripActive]  = Col::AccentSerial;

        c[ImGuiCol_Tab]               = Col::BgCard;
        c[ImGuiCol_TabHovered]        = Col::BgHeader;
        c[ImGuiCol_TabSelected]       = Col::BgHeader;
        c[ImGuiCol_TabSelectedOverline] = Col::AccentSerial;
        c[ImGuiCol_TabDimmed]         = Col::BgCard;
        c[ImGuiCol_TabDimmedSelected] = Col::BgCard;

        c[ImGuiCol_PlotLines]         = Col::AccentSerial;
        c[ImGuiCol_PlotLinesHovered]  = Col::BtnSerialHov;
        c[ImGuiCol_PlotHistogram]     = Col::AccentOpenMP;
        c[ImGuiCol_PlotHistogramHovered] = Col::BtnOmpHov;

        c[ImGuiCol_TableHeaderBg]     = Col::BgHeader;
        c[ImGuiCol_TableBorderStrong] = Col::Border;
        c[ImGuiCol_TableBorderLight]  = { 0.15f, 0.18f, 0.22f, 1.0f };
        c[ImGuiCol_TableRowBg]        = { 0,0,0,0 };
        c[ImGuiCol_TableRowBgAlt]     = { 1.0f, 1.0f, 1.0f, 0.03f };

        c[ImGuiCol_TextSelectedBg]    = { 0.22f, 0.54f, 0.99f, 0.35f };
        c[ImGuiCol_DragDropTarget]    = Col::AccentOpenMP;
        c[ImGuiCol_NavHighlight]      = Col::AccentSerial;
        c[ImGuiCol_NavWindowingHighlight] = { 1,1,1,0.7f };
        c[ImGuiCol_NavWindowingDimBg] = { 0.8f,0.8f,0.8f,0.2f };
        c[ImGuiCol_ModalWindowDimBg]  = { 0.08f,0.08f,0.08f,0.45f };

        c[ImGuiCol_Text]              = Col::TextPrimary;
        c[ImGuiCol_TextDisabled]      = Col::TextMuted;
    }

} // namespace StyleSheet
