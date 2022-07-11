// dear imgui
// (test engine, core)
// This is the interface that your initial setup (app init, main loop) will mostly be using.
// Actual tests will mostly use the interface of imgui_te_context.h

#pragma once

#include "imgui.h"
#include "imgui_internal.h"         // ImPool<>, ImRect, ImGuiItemStatusFlags, ImFormatString
#include "imgui_te_utils.h"         // ImFuncPtr
#include "imgui_capture_tool.h"     // ImGuiScreenCaptureFunc

//-------------------------------------------------------------------------
// Forward Declarations
//-------------------------------------------------------------------------

struct ImGuiTest;                   // Data for a test registered with IM_REGISTER_TEST()
struct ImGuiTestContext;            // Context while a test is running
struct ImGuiTestCoroutineInterface; // Interface to expose coroutine functions (imgui_te_coroutine provides a default implementation for C++11 using std::thread, but you may use your own)
struct ImGuiTestEngine;             // Test engine instance
struct ImGuiTestEngineIO;           // Test engine public I/O
struct ImGuiTestItemInfo;           // Info queried from item (id, geometry, status flags, debug label)
struct ImGuiTestItemList;           // A list of items
struct ImGuiTestInputs;             // Simulated user inputs (will be fed into ImGuiIO by the test engine)

struct ImGuiCaptureArgs;            // Parameters for ImGuiTestContext::CaptureXXX functions.
struct ImGuiPerfTool;               // Performance tool instance

typedef int ImGuiTestFlags;         // Flags: See ImGuiTestFlags_
typedef int ImGuiTestCheckFlags;    // Flags: See ImGuiTestCheckFlags_
typedef int ImGuiTestLogFlags;      // Flags: See ImGuiTestLogFlags_
typedef int ImGuiTestOpFlags;       // Flags: See ImGuiTestOpFlags_
typedef int ImGuiTestRunFlags;      // Flags: See ImGuiTestRunFlags_

enum ImGuiTestEngineExportFormat : int;

//-------------------------------------------------------------------------
// Types
//-------------------------------------------------------------------------

enum ImGuiTestRunSpeed
{
    ImGuiTestRunSpeed_Fast          = 0,    // Run tests as fast as possible (teleport mouse, skip delays, etc.)
    ImGuiTestRunSpeed_Normal        = 1,    // Run tests at human watchable speed (for debugging)
    ImGuiTestRunSpeed_Cinematic     = 2,    // Run tests with pauses between actions (for e.g. tutorials)
    ImGuiTestRunSpeed_COUNT
};

enum ImGuiTestVerboseLevel
{
    ImGuiTestVerboseLevel_Silent    = 0,    // -v0
    ImGuiTestVerboseLevel_Error     = 1,    // -v1
    ImGuiTestVerboseLevel_Warning   = 2,    // -v2
    ImGuiTestVerboseLevel_Info      = 3,    // -v3
    ImGuiTestVerboseLevel_Debug     = 4,    // -v4
    ImGuiTestVerboseLevel_Trace     = 5,
    ImGuiTestVerboseLevel_COUNT     = 6
};

enum ImGuiTestStatus
{
    ImGuiTestStatus_Unknown     = -1,
    ImGuiTestStatus_Success     = 0,
    ImGuiTestStatus_Queued      = 1,
    ImGuiTestStatus_Running     = 2,
    ImGuiTestStatus_Error       = 3,
    ImGuiTestStatus_Suspended   = 4,
};

// Test group (this is mostly used to categorize tests in our testing UI)
enum ImGuiTestGroup
{
    ImGuiTestGroup_Unknown      = -1,
    ImGuiTestGroup_Tests        = 0,
    ImGuiTestGroup_Perfs        = 1,
    ImGuiTestGroup_COUNT
};

enum ImGuiTestFlags_
{
    ImGuiTestFlags_None                 = 0,
    ImGuiTestFlags_NoWarmUp             = 1 << 0,   // By default, we run the GUI func twice before starting the test code
    ImGuiTestFlags_NoAutoFinish         = 1 << 1,   // By default, tests with no test func end on Frame 0 (after the warm up). Setting this require test to call ctx->Finish().
    ImGuiTestFlags_NoRecoverWarnings    = 1 << 2    // Disable state recovery warnings (missing End/Pop calls etc.) for tests which may rely on those.
    //ImGuiTestFlags_RequireViewports   = 1 << 10
};

// Flags for IM_CHECK* macros.
enum ImGuiTestCheckFlags_
{
    ImGuiTestCheckFlags_None            = 0,
    ImGuiTestCheckFlags_SilentSuccess   = 1 << 0
};

// Flags for ImGuiTestContext::Log* functions.
enum ImGuiTestLogFlags_
{
    ImGuiTestLogFlags_None              = 0,
    ImGuiTestLogFlags_NoHeader          = 1 << 0    // Do not display frame count and depth padding
};

enum ImGuiTestRunFlags_
{
    ImGuiTestRunFlags_None              = 0,
    ImGuiTestRunFlags_GuiFuncDisable    = 1 << 0,   // Used internally to temporarily disable the GUI func (at the end of a test, etc)
    ImGuiTestRunFlags_GuiFuncOnly       = 1 << 1,   // Set when user selects "Run GUI func"
    ImGuiTestRunFlags_NoSuccessMsg      = 1 << 2,
    ImGuiTestRunFlags_NoStopOnError     = 1 << 3,
    ImGuiTestRunFlags_NoBreakOnError    = 1 << 4,
    ImGuiTestRunFlags_EnableRawInputs   = 1 << 5,   // Disable input submission to let test submission raw input event (in order to test e.g. IO queue)
    ImGuiTestRunFlags_ManualRun         = 1 << 6,
    ImGuiTestRunFlags_CommandLine       = 1 << 7
};

//-------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------

// Hooks for core imgui/ library
extern void         ImGuiTestEngineHook_ItemAdd(ImGuiContext* ui_ctx, const ImRect& bb, ImGuiID id);
#ifdef IMGUI_HAS_IMSTR
extern void         ImGuiTestEngineHook_ItemInfo(ImGuiContext* ui_ctx, ImGuiID id, ImStrv label, ImGuiItemStatusFlags flags);
#else
extern void         ImGuiTestEngineHook_ItemInfo(ImGuiContext* ui_ctx, ImGuiID id, const char* label, ImGuiItemStatusFlags flags);
static inline int   ImStrcmp(const char* str1, const char* str2) { return strcmp(str1, str2); } // FIXME: to remove once this gets added in core library
#endif
extern void         ImGuiTestEngineHook_Log(ImGuiContext* ui_ctx, const char* fmt, ...);
extern const char*  ImGuiTestEngine_FindItemDebugLabel(ImGuiContext* ui_ctx, ImGuiID id);

// Functions
IMGUI_API bool      ImGuiTestEngine_Check(const char* file, const char* func, int line, ImGuiTestCheckFlags flags, bool result, const char* expr);
IMGUI_API bool      ImGuiTestEngine_CheckStrOp(const char* file, const char* func, int line, ImGuiTestCheckFlags flags, const char* op, const char* lhs_var, const char* lhs_value, const char* rhs_var, const char* rhs_value);
IMGUI_API bool      ImGuiTestEngine_Error(const char* file, const char* func, int line, ImGuiTestCheckFlags flags, const char* fmt, ...);
IMGUI_API void      ImGuiTestEngine_Assert(const char* expr, const char* file, const char* function, int line);

//-------------------------------------------------------------------------
// Macros (the IM_CHECK_xxx macros are at the bottom of this file)
//-------------------------------------------------------------------------

// Register a new test
#define IM_REGISTER_TEST(_ENGINE, _CAT, _NAME)  ImGuiTestEngine_RegisterTest(_ENGINE, _CAT, _NAME, __FILE__, __LINE__)

//-------------------------------------------------------------------------
// ImGuiTestEngine API
//-------------------------------------------------------------------------

// Functions: Initialization
IMGUI_API ImGuiTestEngine*    ImGuiTestEngine_CreateContext();                                      // Create test engine
IMGUI_API void                ImGuiTestEngine_DestroyContext(ImGuiTestEngine* engine);              // Destroy test engine. Call after ImGui::DestroyContext() so test engine specific ini data gets saved.
IMGUI_API void                ImGuiTestEngine_Start(ImGuiTestEngine* engine, ImGuiContext* ui_ctx); // Bind to a dear imgui context. Start coroutine.
IMGUI_API void                ImGuiTestEngine_Stop(ImGuiTestEngine* engine);                        // Stop coroutine and export if any. (Unbind will lazily happen on context shutdown)
IMGUI_API void                ImGuiTestEngine_PostSwap(ImGuiTestEngine* engine);                    // Call every frame after framebuffer swap, will process screen capture and call test_io.ScreenCaptureFunc()
IMGUI_API ImGuiTestEngineIO&  ImGuiTestEngine_GetIO(ImGuiTestEngine* engine);

// Functions: Main
IMGUI_API ImGuiTest*          ImGuiTestEngine_RegisterTest(ImGuiTestEngine* engine, const char* category, const char* name, const char* src_file = NULL, int src_line = 0);
IMGUI_API void                ImGuiTestEngine_QueueTests(ImGuiTestEngine* engine, ImGuiTestGroup group, const char* filter = NULL, ImGuiTestRunFlags run_flags = 0);
IMGUI_API void                ImGuiTestEngine_QueueTest(ImGuiTestEngine* engine, ImGuiTest* test, ImGuiTestRunFlags run_flags = 0);
IMGUI_API void                ImGuiTestEngine_AbortCurrentTest(ImGuiTestEngine* engine);
IMGUI_API bool                ImGuiTestEngine_TryAbortEngine(ImGuiTestEngine* engine);
IMGUI_API bool                ImGuiTestEngine_IsTestQueueEmpty(ImGuiTestEngine* engine);      // FIXME: Clarify difference between this and io.iSRunningTests
IMGUI_API void                ImGuiTestEngine_CoroutineStopRequest(ImGuiTestEngine* engine);
IMGUI_API void                ImGuiTestEngine_GetResult(ImGuiTestEngine* engine, int& count_tested, int& success_count);
IMGUI_API ImGuiPerfTool*      ImGuiTestEngine_GetPerfTool(ImGuiTestEngine* engine);
IMGUI_API bool                ImGuiTestEngine_UseSimulatedInputs(ImGuiTestEngine* engine);

// Functions: Crash Handling
IMGUI_API void                ImGuiTestEngine_InstallDefaultCrashHandler();                         // Install default crash handler
IMGUI_API void                ImGuiTestEngine_CrashHandler();                                       // Default crash handler, should be called from a custom crash handler if such exists

// Functions: Internal/Experimental
IMGUI_API void                ImGuiTestEngine_RebootUiContext(ImGuiTestEngine* engine);

// Function pointers for IO structure
// (also see imgui_te_coroutine.h for coroutine functions)
typedef void        (ImGuiTestEngineSrcFileOpenFunc)(const char* filename, int line, void* user_data);

//-----------------------------------------------------------------------------
// IO structure to configure the test engine
//-----------------------------------------------------------------------------

struct IMGUI_API ImGuiTestEngineIO
{
    //-------------------------------------------------------------------------
    // Functions
    //-------------------------------------------------------------------------

    // Inputs: Functions
    ImGuiTestCoroutineInterface*                CoroutineFuncs = NULL;          // (Required) Coroutine functions (see imgui_te_coroutines.h)
    ImFuncPtr(ImGuiTestEngineSrcFileOpenFunc)   SrcFileOpenFunc = NULL;         // (Optional) To open source files from test engine UI
    ImFuncPtr(ImGuiScreenCaptureFunc)           ScreenCaptureFunc = NULL;       // (Optional) To capture graphics output (application _MUST_ call ImGuiTestEngine_PostSwap() function after swapping is framebuffer)
    void*                                       SrcFileOpenUserData = NULL;     // (Optional) User data for SrcFileOpenFunc
    void*                                       ScreenCaptureUserData = NULL;   // (Optional) User data for ScreenCaptureFunc

    // Options: Main
    bool                        ConfigSavedSettings = true;                     // Load/Save settings in main context .ini file.
    ImGuiTestRunSpeed           ConfigRunSpeed = ImGuiTestRunSpeed_Fast;        // Run tests in fast/normal/cinematic mode
    bool                        ConfigStopOnError = false;                      // Stop queued tests on test error
    bool                        ConfigBreakOnError = false;                     // Break debugger on test error
    bool                        ConfigKeepGuiFunc = false;                      // Keep test GUI running at the end of the test
    ImGuiTestVerboseLevel       ConfigVerboseLevel = ImGuiTestVerboseLevel_Warning;
    ImGuiTestVerboseLevel       ConfigVerboseLevelOnError = ImGuiTestVerboseLevel_Info;
    bool                        ConfigLogToTTY = false;
    bool                        ConfigLogToDebugger = false;
    bool                        ConfigTakeFocusBackAfterTests = true;
    bool                        ConfigCaptureEnabled = true;        // Master enable flags for capturing and saving captures. Disable to avoid e.g. lengthy saving of large PNG files.
    bool                        ConfigCaptureTempDisabled = false;  // Suppress capture functions temporarily (not saved to .ini), do not produce capture warnings. Set by -nocapture, useful for CI.
    bool                        ConfigCaptureOnError = false;
    bool                        ConfigNoThrottle = false;           // Disable vsync for performance measurement or fast test running
    bool                        ConfigMouseDrawCursor = true;       // Enable drawing of Dear ImGui software mouse cursor when running tests
    float                       ConfigFixedDeltaTime = 0.0f;        // Use fixed delta time instead of calculating it from wall clock
    int                         PerfStressAmount = 1;               // Integer to scale the amount of items submitted in test
    char                        GitBranchName[64] = "";             // e.g. fill in branch name

    // Options: Speed of user simulation
    float                       MouseSpeed = 600.0f;                // Mouse speed (pixel/second) when not running in fast mode
    float                       MouseWobble = 0.25f;                // (0.0f..1.0f) How much wobble to apply to the mouse (pixels per pixel of move distance) when not running in fast mode
    float                       ScrollSpeed = 1400.0f;              // Scroll speed (pixel/second) when not running in fast mode
    float                       TypingSpeed = 20.0f;                // Char input speed (characters/second) when not running in fast mode
    float                       ActionDelayShort = 0.15f;           // Time between short actions
    float                       ActionDelayStandard = 0.40f;        // Time between most actions

    // Options: Screen/video capture
    char                        VideoCaptureEncoderPath[256] = "";  // Video encoder executable path, e.g. "path/to/ffmpeg.exe".
    char                        VideoCaptureEncoderParams[256] = "";// Video encoder parameters for .MP4 captures, e.g. see IMGUI_CAPTURE_DEFAULT_VIDEO_PARAMS_FOR_FFMPEG
    char                        GifCaptureEncoderParams[512] = "";  // Video encoder parameters for .GIF captures, e.g. see IMGUI_CAPTURE_DEFAULT_GIF_PARAMS_FOR_FFMPEG
    char                        VideoCaptureExtension[8] = ".mp4";  // Video file extension (default, may be overriden by test).

    // Options: Watchdog. Set values to FLT_MAX to disable.
    // Interactive GUI applications that may be slower tend to use higher values.
    float                       ConfigWatchdogWarning = 30.0f;      // Warn when a test exceed this time (in second)
    float                       ConfigWatchdogKillTest = 60.0f;     // Attempt to stop running a test when exceeding this time (in second)
    float                       ConfigWatchdogKillApp = FLT_MAX;    // Stop application when exceeding this time (in second)

    // Options: Export
    const char*                 ExportResultsFilename = NULL;
    const char*                 ExportResultsDescription = NULL;
    ImGuiTestEngineExportFormat ExportResultsFormat = (ImGuiTestEngineExportFormat)0;

    //-------------------------------------------------------------------------
    // Outputs
    //-------------------------------------------------------------------------

    // State of test engine
    bool                        IsRunningTests = false;
    bool                        IsRequestingMaxAppSpeed = false;    // When running in fast mode: request app to skip vsync or even skip rendering if it wants
    bool                        IsCapturing = false;                // Capture is in progress
};

// Information about a given item, result of an ItemInfo() query
struct ImGuiTestItemInfo
{
    int                         RefCount : 8;               // User can increment this if they want to hold on the result pointer across frames, otherwise the task will be GC-ed.
    unsigned int                NavLayer : 1;               // Nav layer of the item
    int                         Depth : 16;                 // Depth from requested parent id. 0 == ID is immediate child of requested parent id.
    int                         TimestampMain = -1;         // Timestamp of main result (all fields)
    int                         TimestampStatus = -1;       // Timestamp of StatusFlags
    ImGuiID                     ID = 0;                     // Item ID
    ImGuiID                     ParentID = 0;               // Item Parent ID (value at top of the ID stack)
    ImGuiWindow*                Window = NULL;              // Item Window
    ImRect                      RectFull = ImRect();        // Item Rectangle
    ImRect                      RectClipped = ImRect();     // Item Rectangle (clipped with window->ClipRect at time of item submission)
    ImGuiItemStatusFlags        StatusFlags = 0;            // Item Status flags (fully updated for some items only, compare TimestampStatus to FrameCount)
    char                        DebugLabel[32] = {};        // Shortened label for debugging purpose

    ImGuiTestItemInfo()
    {
        RefCount = 0;
        NavLayer = 0;
        Depth = 0;
    }
};

// Result of an ItemGather query
struct IMGUI_API ImGuiTestItemList
{
    ImPool<ImGuiTestItemInfo>   Pool;

    void                        Clear()                 { Pool.Clear(); }
    void                        Reserve(int capacity)   { Pool.Reserve(capacity); }
    int                         GetSize() const         { return Pool.GetMapSize(); }
    const ImGuiTestItemInfo*    GetByIndex(int n)       { return Pool.GetByIndex(n); }
    const ImGuiTestItemInfo*    GetByID(ImGuiID id)     { return Pool.GetByKey(id); }

    // For range-for
    size_t                      size() const            { return (size_t)Pool.GetMapSize(); }
    const ImGuiTestItemInfo*    begin() const           { return Pool.Buf.begin(); }
    const ImGuiTestItemInfo*    end() const             { return Pool.Buf.end(); }
    const ImGuiTestItemInfo*    operator[] (size_t n)   { return &Pool.Buf[(int)n]; }
};

//-------------------------------------------------------------------------
// ImGuiTestLog
//-------------------------------------------------------------------------

struct IMGUI_API ImGuiTestLogLineInfo
{
    ImGuiTestVerboseLevel           Level;
    int                             LineOffset;
};

struct IMGUI_API ImGuiTestLog
{
    ImGuiTextBuffer                 Buffer;
    ImVector<ImGuiTestLogLineInfo>  LineInfo;
    int                             CountPerLevel[ImGuiTestVerboseLevel_COUNT] = {};

    // Functions
    ImGuiTestLog() {}
    bool    IsEmpty() const         { return Buffer.empty(); }
    void    Clear();

    // Extract log contents filtered per log-level.
    // Output:
    // - If 'buffer != NULL': all extracted lines are appended to 'buffer'. Use 'buffer->c_str()' on your side to obtain the text.
    // - Return value: number of lines extracted (should be equivalent to number of '\n' inside buffer->c_str()).
    // - You may call the function with buffer == NULL to only obtain a count without getting the data.
    // Verbose levels are inclusive:
    // - To get ONLY Error:                     Use level_min == ImGuiTestVerboseLevel_Error, level_max = ImGuiTestVerboseLevel_Error
    // - To get ONLY Error and Warnings:        Use level_min == ImGuiTestVerboseLevel_Error, level_max = ImGuiTestVerboseLevel_Warning
    // - To get All Errors, Warnings, Debug...  Use level_min == ImGuiTestVerboseLevel_Error, level_max = ImGuiTestVerboseLevel_Trace
    int     ExtractLinesForVerboseLevels(ImGuiTestVerboseLevel level_min, ImGuiTestVerboseLevel level_max, ImGuiTextBuffer* out_buffer);

    // [Internal]
    void    UpdateLineOffsets(ImGuiTestEngineIO* engine_io, ImGuiTestVerboseLevel level, const char* start);
};

//-------------------------------------------------------------------------
// ImGuiTest
//-------------------------------------------------------------------------

typedef void    (ImGuiTestGuiFunc)(ImGuiTestContext* ctx);
typedef void    (ImGuiTestTestFunc)(ImGuiTestContext* ctx);

// Wraps a placement new of a given type (where 'buffer' is the allocated memory)
typedef void    (ImGuiTestVarsConstructor)(void* buffer);
typedef void    (ImGuiTestVarsPostConstructor)(void* ptr, void* fn);
typedef void    (ImGuiTestVarsDestructor)(void* ptr);

// Storage for one test
struct IMGUI_API ImGuiTest
{
    // Test Definition
    const char*                     Category = NULL;                // Literal, not owned
    const char*                     Name = NULL;                    // Literal, generally not owned unless NameOwned=true
    ImGuiTestGroup                  Group = ImGuiTestGroup_Unknown; // Coarse groups: 'Tests' or 'Perf'
    bool                            NameOwned = false;              //
    const char*                     SourceFile = NULL;              // __FILE__
    int                             SourceLine = 0;                 // __LINE__
    int                             SourceLineEnd = 0;              // Calculated by ImGuiTestEngine_StartCalcSourceLineEnds()
    int                             ArgVariant = 0;                 // User parameter. Generally we use it to run variations of a same test by sharing GuiFunc/TestFunc
    ImGuiTestFlags                  Flags = ImGuiTestFlags_None;    // See ImGuiTestFlags_
    ImFuncPtr(ImGuiTestGuiFunc)     GuiFunc = NULL;                 // GUI function (optional if your test are running over an existing GUI application)
    ImFuncPtr(ImGuiTestTestFunc)    TestFunc = NULL;                // Test function
    void*                           UserData = NULL;                // General purpose user data (if assigning capturing lambdas on GuiFunc/TestFunc you may not need to se this)

    // Test Status
    ImGuiTestStatus                 Status = ImGuiTestStatus_Unknown;
    ImGuiTestLog                    TestLog;
    ImU64                           StartTime = 0;                  // Microseconds
    ImU64                           EndTime = 0;                    // Microseconds
    int                             GuiFuncLastFrame = -1;

    // User variables (which are instantiated when running the test)
    // Setup after test registration with SetVarsDataType<>(), access instance during test with GetVars<>().
    // This is mostly useful to communicate between GuiFunc and TestFunc. If you don't use both you may not want to use it!
    size_t                          VarsSize = 0;
    ImGuiTestVarsConstructor*       VarsConstructor = NULL;
    ImGuiTestVarsPostConstructor*   VarsPostConstructor = NULL;     // To override constructor default (in case the default are problematic on the first GuiFunc frame)
    void*                           VarsPostConstructorUserFn = NULL;
    ImGuiTestVarsDestructor*        VarsDestructor = NULL;

    // Functions
    ImGuiTest() {}
    ~ImGuiTest();

    void SetOwnedName(const char* name);

    template <typename T>
    void SetVarsDataType(void(*post_initialize)(T& vars) = NULL)
    {
        VarsSize = sizeof(T);
        VarsConstructor = [](void* ptr) { IM_PLACEMENT_NEW(ptr) T; };
        VarsDestructor = [](void* ptr) { IM_UNUSED(ptr); reinterpret_cast<T*>(ptr)->~T(); };
        if (post_initialize != NULL)
        {
            VarsPostConstructorUserFn = (void*)post_initialize;
            VarsPostConstructor = [](void* ptr, void* fn) { ((void (*)(T&))(fn))(*(T*)ptr); };
        }
    }
};

//-------------------------------------------------------------------------
