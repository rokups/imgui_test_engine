#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "imgui.h"
#include "imgui_te_util.h"
#include "imgui_internal.h"
#include <chrono>

// Hash "hello/world" as if it was "helloworld"
// To hash a forward slash we need to use "hello\\/world"
//   IM_ASSERT(ImHashDecoratedPath("Hello/world")   == ImHash("Helloworld", 0));
//   IM_ASSERT(ImHashDecoratedPath("Hello\\/world") == ImHash("Hello/world", 0));
// Adapted from ImHash(). Not particularly fast!
ImGuiID ImHashDecoratedPath(const char* str, ImGuiID seed)
{
    static ImU32 crc32_lut[256] = { 0 };
    if (!crc32_lut[1])
    {
        const ImU32 polynomial = 0xEDB88320;
        for (ImU32 i = 0; i < 256; i++)
        {
            ImU32 crc = i;
            for (ImU32 j = 0; j < 8; j++)
                crc = (crc >> 1) ^ (ImU32(-int(crc & 1)) & polynomial);
            crc32_lut[i] = crc;
        }
    }

    // Prefixing the string with / ignore the seed
    if (str[0] == '/')
        seed = 0;

    seed = ~seed;
    ImU32 crc = seed;

    // Zero-terminated string
    bool inhibit_one = false;
    const unsigned char* current = (const unsigned char*)str;
    while (unsigned char c = *current++)
    {
        if (c == '\\' && !inhibit_one)
        {
            inhibit_one = true;
            continue;
        }

        // Forward slashes are ignored unless prefixed with a backward slash
        if (c == '/' && !inhibit_one)
        {
            inhibit_one = false;
            continue;
        }

        // Reset the hash when encountering ### 
        if (c == '#' && current[0] == '#' && current[1] == '#')
            crc = seed;

        crc = (crc >> 8) ^ crc32_lut[(crc & 0xFF) ^ c];
        inhibit_one = false;
    }
    return ~crc;
}

// Trying std::chrono out of unfettered optimism that it may actually work..
ImU64   ImGetTimeInMicroseconds()
{
    using namespace std::chrono;
    microseconds ms = duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch());
    return ms.count();
}

void    ImPathFixSeparatorsForCurrentOS(char* buf)
{
#ifdef _WIN32
    for (char* p = buf; *p != 0; p++)
        if (*p == '/')
            *p = '\\';
#else
    for (char* p = buf; *p != 0; p++)
        if (*p == '\\')
            *p = '/';
#endif
}

void    ImParseSplitCommandLine(int* out_argc, char*** out_argv, const char* cmd_line)
{
    size_t cmd_line_len = strlen(cmd_line);

    int n = 1;
    {
        const char* p = cmd_line;
        while (*p != 0)
        {
            const char* arg = p;
            while (*arg == ' ')
                arg++;
            const char* arg_end = strchr(arg, ' ');
            if (arg_end == NULL)
                p = arg_end = cmd_line + cmd_line_len;
            else
                p = arg_end + 1;
            n++;
        }
    }

    int argc = n;
    char** argv = (char**)malloc(sizeof(char*) * (argc + 1) + (cmd_line_len + 1));
    char* cmd_line_dup = (char*)argv + sizeof(char*) * (argc + 1);
    strcpy(cmd_line_dup, cmd_line);

    {
        argv[0] = "main.exe";
        argv[argc] = NULL;

        char* p = cmd_line_dup;
        for (n = 1; n < argc; n++)
        {
            char* arg = p;
            char* arg_end = strchr(arg, ' ');
            if (arg_end == NULL)
                p = arg_end = cmd_line_dup + cmd_line_len;
            else
                p = arg_end + 1;
            argv[n] = arg;
            arg_end[0] = 0;
        }
    }

    *out_argc = argc;
    *out_argv = argv;
}

// Turn __DATE__ "Jan 10 2019" into "2019-01-10"
void    ImParseDateFromCompilerIntoYMD(const char* in_date, char* out_buf, size_t out_buf_size)
{
    char month_str[5];
    int year, month, day;
    sscanf(in_date, "%s %d %d", month_str, &day, &year);
    const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
    const char* p = strstr(month_names, month_str);
    month = p ? (int)(1 + (p - month_names) / 3) : 0;
    ImFormatString(out_buf, out_buf_size, "%04d-%02d-%02d", year, month, day);
}

void ImDebugShowInputTextState()
{
    ImGuiContext& g = *GImGui;
    //static MemoryEditor mem_edit;
    using namespace ImGui;

    ImGui::Begin("Debug stb_textedit.h");

    ImGuiInputTextState& imstate = g.InputTextState;
    if (g.ActiveId != 0 && imstate.ID == g.ActiveId)
        ImGui::Text("Active");
    else
        ImGui::Text("Inactive");

    ImGuiStb::StbUndoState& undostate = imstate.StbState.undostate;

    ImGui::Text("undo_point: %d\nredo_point:%d\nundo_char_point: %d\nredo_char_point:%d",
        undostate.undo_point,
        undostate.redo_point,
        undostate.undo_char_point,
        undostate.redo_char_point);

    ImGuiStyle& style = ImGui::GetStyle();
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, -2));
    for (int n = 0; n < STB_TEXTEDIT_UNDOSTATECOUNT; n++)
    {
        char type = ' ';
        if (n < undostate.undo_point)
            type = 'u';
        else if (n >= undostate.redo_point)
            type = 'r';

        ImVec4 col = (type == ' ') ? style.Colors[ImGuiCol_TextDisabled] : style.Colors[ImGuiCol_Text];
        ImGui::TextColored(col, "%c [%02d] where %03d, insert %03d, delete %03d, char_storage %03d",
            type, n, undostate.undo_rec[n].where, undostate.undo_rec[n].insert_length, undostate.undo_rec[n].delete_length, undostate.undo_rec[n].char_storage);
        //if (ImGui::IsItemClicked() && undostate.undo_rec[n].char_storage != -1)
        //    mem_edit.GotoAddrAndHighlight(undostate.undo_rec[n].char_storage, undostate.undo_rec[n].char_storage + undostate.undo_rec[n].insert_length);
    }
    ImGui::PopStyleVar();

    ImGui::End();

    ImGui::Begin("Debug stb_textedit.h char_storage");
    ImVec2 p = ImGui::GetCursorPos();
    for (int n = 0; n < STB_TEXTEDIT_UNDOCHARCOUNT; n++)
    {
        int c = undostate.undo_char[n];
        if (c > 256)
            continue;
        if ((n % 32) == 0)
        {
            ImGui::SetCursorPos(ImVec2(p.x + (n % 32) * 11, p.y + (n / 32) * 13));
            ImGui::Text("%03d:", n);
        }
        ImGui::SetCursorPos(ImVec2(p.x + 40 + (n % 32) * 11, p.y + (n / 32) * 13));
        ImGui::Text("%c", c);
    }
    ImGui::End();
}

void GetImGuiKeyModsPrefixStr(ImGuiKeyModFlags mod_flags, char* out_buf, size_t out_buf_size)
{
    if (mod_flags == 0)
    {
        out_buf[0] = 0;
        return;
    }
    ImFormatString(out_buf, out_buf_size, "%s%s%s%s",
        (mod_flags & ImGuiKeyModFlags_Ctrl) ? "Ctrl+" : "",
        (mod_flags & ImGuiKeyModFlags_Alt) ? "Alt+" : "",
        (mod_flags & ImGuiKeyModFlags_Shift) ? "Shift+" : "",
        (mod_flags & ImGuiKeyModFlags_Super) ? "Super+" : "");
}

const char* GetImGuiKeyName(ImGuiKey key)
{
    // Create switch-case from enum with regexp: ImGuiKey_{.*}, --> case ImGuiKey_\1: return "\1";
    switch (key)
    {
    case ImGuiKey_Tab: return "Tab";
    case ImGuiKey_LeftArrow: return "LeftArrow";
    case ImGuiKey_RightArrow: return "RightArrow";
    case ImGuiKey_UpArrow: return "UpArrow";
    case ImGuiKey_DownArrow: return "DownArrow";
    case ImGuiKey_PageUp: return "PageUp";
    case ImGuiKey_PageDown: return "PageDown";
    case ImGuiKey_Home: return "Home";
    case ImGuiKey_End: return "End";
    case ImGuiKey_Insert: return "Insert";
    case ImGuiKey_Delete: return "Delete";
    case ImGuiKey_Backspace: return "Backspace";
    case ImGuiKey_Space: return "Space";
    case ImGuiKey_Enter: return "Enter";
    case ImGuiKey_Escape: return "Escape";
    case ImGuiKey_A: return "A";
    case ImGuiKey_C: return "C";
    case ImGuiKey_V: return "V";
    case ImGuiKey_X: return "X";
    case ImGuiKey_Y: return "Y";
    case ImGuiKey_Z: return "Z";
    }
    IM_ASSERT(0);
    return "Unknown";
}