#include "GuiRenderer.h"
#include "StyleSheet.h"
#include "AnalysisJob.h"
#include "ResultsParser.h"
#include "imgui.h"

#include <windows.h>
#include <commdlg.h>
#include <string>
#include <sstream>
#include <filesystem>
#include <cstdio>
#include <cmath>

namespace fs = std::filesystem;

// ─── Font handles (set in loadFonts) ─────────────────────────────────────────
static ImFont* g_fontUI      = nullptr;
static ImFont* g_fontConsole = nullptr;
static ImFont* g_fontLarge   = nullptr;

// ─── Helpers ──────────────────────────────────────────────────────────────────

static std::string getExeDir() {
    char buf[MAX_PATH] = {};
    GetModuleFileNameA(nullptr, buf, MAX_PATH);
    std::string path(buf);
    size_t pos = path.find_last_of("\\/");
    return (pos != std::string::npos) ? path.substr(0, pos) : ".";
}

static std::string getParalogExe() {
    return getExeDir() + "\\paralog.exe";
}

static std::string getCachePath() {
    // cache lives one level up from bin/, in the build root
    std::string exeDir = getExeDir();
    // Try: <exeDir>\comparison_cache.json (same folder as GUI)
    std::string p1 = exeDir + "\\comparison_cache.json";
    if (fs::exists(p1)) return p1;
    // Try: <exeDir>\..\comparison_cache.json (build root)
    std::string p2 = exeDir + "\\..\\comparison_cache.json";
    if (fs::exists(p2)) return p2;
    return p1; // fallback — parser will produce friendly error
}

// Open native Windows file picker, returns selected path or ""
static std::string browseFile() {
    char buf[MAX_PATH] = {};
    OPENFILENAMEA ofn{};
    ofn.lStructSize  = sizeof(ofn);
    ofn.hwndOwner    = nullptr;
    ofn.lpstrFilter  = "Log Files (*.log)\0*.log\0All Files (*.*)\0*.*\0\0";
    ofn.lpstrFile    = buf;
    ofn.nMaxFile     = MAX_PATH;
    ofn.Flags        = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    ofn.lpstrTitle   = "Select Log File";
    if (GetOpenFileNameA(&ofn)) {
        std::string s(buf);
        for (char& c : s) { if (c == '\\') c = '/'; }
        return s;
    }
    return "";
}

static ImVec4 statusColor(JobState::Status s) {
    switch (s) {
        case JobState::Status::Running: return StyleSheet::Col::StatusRun;
        case JobState::Status::Done:    return StyleSheet::Col::StatusDone;
        case JobState::Status::Error:   return StyleSheet::Col::StatusError;
        default:                         return StyleSheet::Col::StatusNone;
    }
}

static const char* statusText(JobState::Status s) {
    switch (s) {
        case JobState::Status::Running: return "Running...";
        case JobState::Status::Done:    return "Completed";
        case JobState::Status::Error:   return "Error";
        default:                         return "Not Run";
    }
}

static void drawStatusBadge(JobState::Status s) {
    ImGui::PushStyleColor(ImGuiCol_Text, statusColor(s));
    ImGui::TextUnformatted(statusText(s));
    ImGui::PopStyleColor();
}

// Spinning animation character
static const char* spinner() {
    static const char* frames[] = { "\xEF\x84\x91", "\xEF\x86\x83", "\xEF\x84\x90", "\xEF\x86\x82" }; // Dots if font supports, fallback to dots
    static const char* fallback[] = { ".", "..", "...", "...." };
    return fallback[(int)(ImGui::GetTime() * 4.0) % 4];
}

// ─── Font loading ─────────────────────────────────────────────────────────────
void GuiRenderer::loadFonts(float uiSize, float consoleSize) {
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();
    
    std::string windir = "C:\\Windows\\Fonts\\";
    ImFontConfig cfg;
    cfg.OversampleH = 3;
    cfg.OversampleV = 3;
    cfg.PixelSnapH = true; // Snap characters to pixel grid
    
    // UI Font (Segoe UI Semibold for better weight)
    std::string semibold = windir + "seguisb.ttf";
    std::string bold = windir + "segoeuib.ttf";
    std::string sans = windir + "segoeui.ttf";
    
    if (fs::exists(semibold)) {
        g_fontUI = io.Fonts->AddFontFromFileTTF(semibold.c_str(), 20.0f, &cfg);
    } else if (fs::exists(sans)) {
        g_fontUI = io.Fonts->AddFontFromFileTTF(sans.c_str(), 20.0f, &cfg);
    } else {
        g_fontUI = io.Fonts->AddFontDefault();
    }

    if (fs::exists(bold)) {
        g_fontLarge = io.Fonts->AddFontFromFileTTF(bold.c_str(), 36.0f, &cfg);
    } else {
        g_fontLarge = io.Fonts->AddFontDefault();
    }
    
    // Console Font (Consolas)
    std::string mono = windir + "consola.ttf";
    if (fs::exists(mono)) {
        g_fontConsole = io.Fonts->AddFontFromFileTTF(mono.c_str(), 18.0f, &cfg);
    } else {
        g_fontConsole = io.Fonts->AddFontDefault();
    }
    
    io.Fonts->Build();
    (void)uiSize; (void)consoleSize;
}

// ─── Mode card rendering ──────────────────────────────────────────────────────
struct ModeCard {
    const char* label;
    const char* icon;
    ImVec4      accent;
    ImVec4      accentHov;
    ImVec4      accentAct;
    JobState&   job;
    AnalysisJob& runner;
    int         tabIdx;
};

static void drawModeCard(AppState& state, ModeCard& card,
                          const std::string& file, float cardWidth, bool anyRunning) {
    // Poll finished flag from reader thread
    if (card.job.finished.load() &&
        card.job.status == JobState::Status::Running) {
        card.job.status = card.job.success ? JobState::Status::Done
                                            : JobState::Status::Error;
    }

    ImGui::PushID(card.label);
    
    if (card.job.status == JobState::Status::Running) {
        float pulse = (sin(ImGui::GetTime() * 6.0f) + 1.0f) * 0.5f;
        ImVec4 borderCol = card.accent;
        borderCol.w = 0.3f + 0.7f * pulse;
        ImGui::PushStyleColor(ImGuiCol_Border, borderCol);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Border, StyleSheet::Col::Border);
    }
    
    ImGui::PushStyleColor(ImGuiCol_ChildBg, StyleSheet::Col::BgCard);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
    ImGui::BeginChild("##card", {cardWidth, 0}, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY);
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);

    // Mode header (Centered)
    ImGui::PushStyleColor(ImGuiCol_Text, card.accent);
    float textWidth = ImGui::CalcTextSize(card.label).x;
    ImGui::SetCursorPosX((cardWidth - textWidth) * 0.5f);
    ImGui::TextUnformatted(card.label);
    ImGui::PopStyleColor();
    ImGui::Separator();
    ImGui::Spacing();

    // Status badge (Centered)
    float statusTotalWidth = ImGui::CalcTextSize("Status: ").x + ImGui::CalcTextSize(statusText(card.job.status)).x + 20.0f;
    ImGui::SetCursorPosX((cardWidth - statusTotalWidth) * 0.5f);
    ImGui::TextUnformatted("Status: ");
    ImGui::SameLine();
    if (card.job.status == JobState::Status::Running) {
        float pulse = (sin(ImGui::GetTime() * 8.0f) + 1.0f) * 0.5f;
        ImVec4 runningCol = StyleSheet::Col::StatusRun;
        runningCol.x += (1.0f - runningCol.x) * pulse * 0.3f;
        runningCol.y += (1.0f - runningCol.y) * pulse * 0.3f;
        runningCol.z += (1.0f - runningCol.z) * pulse * 0.3f;
        
        ImGui::PushStyleColor(ImGuiCol_Text, runningCol);
        ImGui::Text("Running %s", spinner());
        ImGui::PopStyleColor();
    } else {
        drawStatusBadge(card.job.status);
    }
    ImGui::Spacing();

    // MPI-specific config
    if (card.tabIdx == 2) {
        ImGui::TextUnformatted("Processes:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(cardWidth - 110.0f);
        ImGui::InputInt("##nproc", &state.mpiProcesses, 1, 4);
        if (state.mpiProcesses < 1) state.mpiProcesses = 1;
        if (state.mpiProcesses > 256) state.mpiProcesses = 256;

        ImGui::TextUnformatted("mpiexec: ");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(cardWidth - 90.0f);
        ImGui::InputText("##mpiexec", state.mpiexecPath, sizeof(state.mpiexecPath));
        ImGui::Spacing();
    }

    // Run button
    bool running = card.job.status == JobState::Status::Running;
    bool noFile  = file.empty();

    if (running) {
        ImGui::PushStyleColor(ImGuiCol_Button,        StyleSheet::Col::StatusError);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 1.0f, 0.4f, 0.35f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  { 0.8f, 0.25f, 0.22f, 1.0f });
        ImGui::SetCursorPosX(14.0f);
        if (ImGui::Button("CANCEL ANALYSIS", { cardWidth - 28.0f, 40.0f })) {
            card.runner.cancel();
            card.job.status = JobState::Status::NotRun;
            card.job.pushLine("[GUI] Analysis cancelled by user.");
        }
        ImGui::PopStyleColor(3);
    } else {
        bool disabled = noFile || anyRunning;
        ImGui::BeginDisabled(disabled);
        ImGui::PushStyleColor(ImGuiCol_Button,        card.accent);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, card.accentHov);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  card.accentAct);
        std::string btnLabel = std::string("RUN ") + card.label;
        ImGui::SetCursorPosX(14.0f);
        if (ImGui::Button(btnLabel.c_str(), { cardWidth - 28.0f, 40.0f })) {
            state.activeConsoleTab = card.tabIdx;
            std::string exe = getParalogExe();
            if (card.tabIdx == 0)
                card.runner.runSerial(card.job, exe, file);
            else if (card.tabIdx == 1)
                card.runner.runOpenMP(card.job, exe, file);
            else
                card.runner.runMpi(card.job, state.mpiexecPath, exe, file, state.mpiProcesses);
        }
        ImGui::PopStyleColor(3);
        ImGui::EndDisabled();
    }

    ImGui::Spacing();
    ImGui::EndChild();
    ImGui::PopID();
}

// ─── Console panel ────────────────────────────────────────────────────────────
static void drawConsolePanel(AppState& state, float panelHeight) {
    const char* tabNames[] = { "Serial", "OpenMP", "MPI" };

    ImGui::PushStyleColor(ImGuiCol_ChildBg, StyleSheet::Col::BgConsole);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);

    // Tab header (sits above the console child)
    if (ImGui::BeginTabBar("##consoleTabs")) {
        for (int i = 0; i < 3; i++) {
            JobState& job = state.jobFor(i);
            ImVec4 tabCol = (job.status == JobState::Status::Running)
                          ? StyleSheet::Col::StatusRun : StyleSheet::Col::TextMuted;
            ImGui::PushStyleColor(ImGuiCol_Text, tabCol);
            if (ImGui::BeginTabItem(tabNames[i])) {
                state.activeConsoleTab = i;
                ImGui::EndTabItem();
            }
            ImGui::PopStyleColor();
        }
        ImGui::EndTabBar();
    }

    // Console output
    JobState& activeJob = state.jobFor(state.activeConsoleTab);
    if (ImGui::BeginChild("##console", { 0.0f, panelHeight - 38.0f },
                           ImGuiChildFlags_Border, ImGuiWindowFlags_HorizontalScrollbar)) {
        if (g_fontConsole) ImGui::PushFont(g_fontConsole);

        // Copy lines under lock
        std::vector<std::string> linesCopy;
        {
            std::lock_guard<std::mutex> lk(activeJob.linesMutex);
            linesCopy = activeJob.lines;
        }

        for (const auto& line : linesCopy) {
            // Color-code by prefix
            if      (line.find("[FATAL]")  != std::string::npos ||
                     line.find("[ERROR]")  != std::string::npos)
                ImGui::PushStyleColor(ImGuiCol_Text, StyleSheet::Col::StatusError);
            else if (line.find("[WARN]")   != std::string::npos)
                ImGui::PushStyleColor(ImGuiCol_Text, StyleSheet::Col::StatusRun);
            else if (line.find("[INFO]")   != std::string::npos ||
                     line.find("[SUCCESS]")!= std::string::npos)
                ImGui::PushStyleColor(ImGuiCol_Text, StyleSheet::Col::AccentOpenMP);
            else if (line.find("[GUI")     != std::string::npos)
                ImGui::PushStyleColor(ImGuiCol_Text, StyleSheet::Col::TextMuted);
            else
                ImGui::PushStyleColor(ImGuiCol_Text, StyleSheet::Col::TextPrimary);

            ImGui::TextUnformatted(line.c_str());
            ImGui::PopStyleColor();
        }

        // Auto-scroll to bottom while running
        if (activeJob.status == JobState::Status::Running)
            ImGui::SetScrollHereY(1.0f);

        if (g_fontConsole) ImGui::PopFont();
    }
    ImGui::EndChild();

    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

// ─── Compare panel ────────────────────────────────────────────────────────────
static void drawComparePanel(AppState& state) {
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // "COMPARE RESULTS" button (Centered)
    ImGui::PushStyleColor(ImGuiCol_Button,        StyleSheet::Col::BgHeader);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.20f, 0.25f, 0.32f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  StyleSheet::Col::AccentSerial);
    
    float compareBtnW = 220.0f;
    ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - compareBtnW) * 0.5f);
    bool load = ImGui::Button("COMPARE RESULTS", { compareBtnW, 40.0f });
    ImGui::PopStyleColor(3);

    if (load && !state.selectedFile.empty()) {
        state.compareError.clear();
        state.compareResults = ResultsParser::load(
            getCachePath(), state.selectedFile, state.compareError);
        state.compareLoaded = true;
    }

    if (state.compareLoaded) {
        ImGui::Spacing();

        if (!state.compareError.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, StyleSheet::Col::StatusError);
            ImGui::TextWrapped("%s", state.compareError.c_str());
            ImGui::PopStyleColor();
            return;
        }

        // Results table
        const ImGuiTableFlags flags =
            ImGuiTableFlags_Borders       |
            ImGuiTableFlags_RowBg         |
            ImGuiTableFlags_SizingStretchSame |
            ImGuiTableFlags_Resizable;

        if (ImGui::BeginTable("##compare", 7, flags)) {
            // Headers
            ImGui::TableSetupColumn("Mode",       ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableSetupColumn("Total Lines",ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Errors",     ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Warnings",   ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Info",       ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Time (ms)",  ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Speedup",    ImGuiTableColumnFlags_WidthFixed, 90.0f);
            ImGui::TableHeadersRow();

            for (const auto& r : state.compareResults) {
                ImGui::TableNextRow();

                // Mode column (colored)
                ImGui::TableSetColumnIndex(0);
                ImVec4 modeCol = (r.mode == "serial")  ? StyleSheet::Col::AccentSerial
                               : (r.mode == "openmp")  ? StyleSheet::Col::AccentOpenMP
                                                        : StyleSheet::Col::AccentMpi;
                ImGui::PushStyleColor(ImGuiCol_Text, modeCol);
                ImGui::TextUnformatted(r.mode.c_str());
                ImGui::PopStyleColor();

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%zu", r.totalLines);

                ImGui::TableSetColumnIndex(2);
                ImGui::PushStyleColor(ImGuiCol_Text, StyleSheet::Col::StatusError);
                ImGui::Text("%zu", r.errorCount);
                ImGui::PopStyleColor();

                ImGui::TableSetColumnIndex(3);
                ImGui::PushStyleColor(ImGuiCol_Text, StyleSheet::Col::StatusRun);
                ImGui::Text("%zu", r.warningCount);
                ImGui::PopStyleColor();

                ImGui::TableSetColumnIndex(4);
                ImGui::Text("%zu", r.infoCount);

                ImGui::TableSetColumnIndex(5);
                ImGui::Text("%.1f", r.processingTimeMs);

                ImGui::TableSetColumnIndex(6);
                if (r.speedup > 0.0) {
                    ImGui::PushStyleColor(ImGuiCol_Text, StyleSheet::Col::AccentOpenMP);
                    ImGui::Text("%.2fx", r.speedup);
                    ImGui::PopStyleColor();
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Text, StyleSheet::Col::TextMuted);
                    ImGui::TextUnformatted("—");
                    ImGui::PopStyleColor();
                }
            }
            ImGui::EndTable();
        }

        // Speedup summary
        double omSpeedup = 0.0, mpiSpeedup = 0.0;
        for (const auto& r : state.compareResults) {
            if (r.mode == "openmp") omSpeedup  = r.speedup;
            if (r.mode == "mpi")   mpiSpeedup = r.speedup;
        }
        if (omSpeedup > 0.0 || mpiSpeedup > 0.0) {
            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_ChildBg, StyleSheet::Col::BgHeader);
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
            ImGui::BeginChild("##summary", {0, 60}, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY);
            ImGui::Spacing();
            ImGui::Text("  Performance Summary:");
            ImGui::SameLine(220);
            if (omSpeedup > 0.0) {
                ImGui::PushStyleColor(ImGuiCol_Text, StyleSheet::Col::AccentOpenMP);
                ImGui::Text("OpenMP: %.2fx faster", omSpeedup);
                ImGui::PopStyleColor();
                ImGui::SameLine();
                ImGui::TextUnformatted("   ");
                ImGui::SameLine();
            }
            if (mpiSpeedup > 0.0) {
                ImGui::PushStyleColor(ImGuiCol_Text, StyleSheet::Col::AccentMpi);
                ImGui::Text("MPI: %.2fx faster", mpiSpeedup);
                ImGui::PopStyleColor();
            }
            ImGui::Spacing();
            ImGui::EndChild();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
        }
    }
}

// ─── Main draw ────────────────────────────────────────────────────────────────
void GuiRenderer::draw(AppState& state, float windowW, float windowH) {
    // Full-screen window (no decorations — the OS window is the frame)
    ImGui::SetNextWindowPos({ 0, 0 });
    ImGui::SetNextWindowSize({ windowW, windowH });
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGuiWindowFlags mainFlags =
        ImGuiWindowFlags_NoDecoration    |
        ImGuiWindowFlags_NoMove          |
        ImGuiWindowFlags_NoResize        |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::Begin("##main", nullptr, mainFlags);
    ImGui::PopStyleVar(2);

    // ── App title bar ──────────────────────────────────────────────────────
    if (g_fontLarge) ImGui::PushFont(g_fontLarge);
    ImGui::PushStyleColor(ImGuiCol_Text, StyleSheet::Col::AccentSerial);
    ImGui::Text("  ParaLog");
    ImGui::PopStyleColor();
    if (g_fontLarge) ImGui::PopFont();
    
    ImGui::SameLine();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6.0f); // align baseline
    ImGui::PushStyleColor(ImGuiCol_Text, StyleSheet::Col::TextMuted);
    ImGui::TextUnformatted("Parallel Log File Analyzer");
    ImGui::PopStyleColor();
    ImGui::Separator();
    ImGui::Spacing();

    // ── File selection ─────────────────────────────────────────────────────
    float browseW  = 110.0f;
    float pathW    = windowW - browseW - 50.0f;

    ImGui::PushStyleColor(ImGuiCol_FrameBg, StyleSheet::Col::BgConsole);
    ImGui::SetNextItemWidth(pathW);
    ImGui::InputText("##filepath",
                     state.selectedFile.data(),
                     state.selectedFile.capacity() + 1,
                     ImGuiInputTextFlags_ReadOnly);
    ImGui::PopStyleColor();
    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button,        StyleSheet::Col::AccentSerial);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, StyleSheet::Col::BtnSerialHov);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  StyleSheet::Col::BtnSerialAct);
    if (ImGui::Button("Browse...", { browseW, 0 })) {
        std::string picked = browseFile();
        if (!picked.empty()) {
            state.selectedFile = picked;
            // Reserve enough capacity for InputText to read it
            state.selectedFile.reserve(MAX_PATH);
            // Reset compare results when a new file is selected
            state.compareLoaded  = false;
            state.compareResults.clear();
            state.compareError.clear();
        }
    }
    ImGui::PopStyleColor(3);

    if (state.selectedFile.empty()) {
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, StyleSheet::Col::TextMuted);
        ImGui::TextUnformatted("  ← Select a log file to begin");
        ImGui::PopStyleColor();
    }
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // ── Mode cards ─────────────────────────────────────────────────────────
    float cardW = (windowW - 60.0f) / 3.0f;
    const std::string& file = state.selectedFile;

    ModeCard cards[3] = {
        { "Serial",  "[S]", StyleSheet::Col::AccentSerial,
          StyleSheet::Col::BtnSerialHov, StyleSheet::Col::BtnSerialAct,
          state.serial,  g_jobSerial,  0 },
        { "OpenMP",  "[O]", StyleSheet::Col::AccentOpenMP,
          StyleSheet::Col::BtnOmpHov,   StyleSheet::Col::BtnOmpAct,
          state.openmp,  g_jobOpenMP,  1 },
        { "MPI",     "[M]", StyleSheet::Col::AccentMpi,
          StyleSheet::Col::BtnMpiHov,   StyleSheet::Col::BtnMpiAct,
          state.mpi,     g_jobMpi,     2 },
    };

    bool anyRunning = (state.serial.status == JobState::Status::Running) ||
                      (state.openmp.status == JobState::Status::Running) ||
                      (state.mpi.status == JobState::Status::Running);

    for (int i = 0; i < 3; i++) {
        if (i > 0) ImGui::SameLine(0, 8.0f);
        drawModeCard(state, cards[i], file, cardW, anyRunning);
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // ── Console panel ──────────────────────────────────────────────────────
    float compareH    = 220.0f; // reserved height for compare at bottom
    float headerUsed  = 160.0f; // title + file + cards + separators (approx)
    float consoleH    = windowH - headerUsed - compareH - 80.0f;
    if (consoleH < 100.0f) consoleH = 100.0f;

    drawConsolePanel(state, consoleH);

    // ── Compare panel ──────────────────────────────────────────────────────
    drawComparePanel(state);

    ImGui::End();
}
