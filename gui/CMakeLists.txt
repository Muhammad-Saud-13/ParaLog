cmake_minimum_required(VERSION 3.20)

include(FetchContent)

# ─── Dear ImGui ───────────────────────────────────────────────────────────────
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.91.6
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(imgui)

# ─── nlohmann/json (single header) ───────────────────────────────────────────
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

# ─── GUI source files ─────────────────────────────────────────────────────────
set(GUI_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/main_gui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/AnalysisJob.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ResultsParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/GuiRenderer.cpp

    # ImGui core
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp

    # ImGui backends (Win32 + DirectX 11)
    ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp
)

# ─── WIN32 gui executable (no console window) ─────────────────────────────────
add_executable(paralog_gui WIN32 ${GUI_SOURCES})

target_include_directories(paralog_gui PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${nlohmann_json_SOURCE_DIR}/include
)

# ─── Link DirectX 11 + Windows libraries ─────────────────────────────────────
target_link_libraries(paralog_gui PRIVATE
    d3d11
    d3dcompiler
    dxgi
    dwmapi
    comctl32
    comdlg32
)

# ─── Compiler flags ───────────────────────────────────────────────────────────
target_compile_options(paralog_gui PRIVATE -Wall -Wextra)
target_compile_options(paralog_gui PRIVATE $<$<CONFIG:Release>:-O2>)
target_compile_definitions(paralog_gui PRIVATE
    UNICODE
    _UNICODE
    NOMINMAX
    WIN32_LEAN_AND_MEAN
    _WIN32_WINNT=0x0601   # Windows 7+
    IMGUI_IMPL_DX11_DISABLE_D3D11_COMPILER=1
)

if(MINGW)
    target_link_options(paralog_gui PRIVATE "-mwindows" "-static" "-static-libgcc" "-static-libstdc++")
endif()

# ─── Output to same bin/ folder as paralog.exe ────────────────────────────────
set_target_properties(paralog_gui PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

message(STATUS "GUI target: paralog_gui (Dear ImGui + Win32 + DX11)")
