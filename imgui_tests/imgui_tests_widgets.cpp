// dear imgui
// (tests: widgets)

#define _CRT_SECURE_NO_WARNINGS
#include <limits.h>
#include "imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#include "imgui_tests.h"
#include "imgui_test_engine/imgui_te_engine.h"      // IM_REGISTER_TEST()
#include "imgui_test_engine/imgui_te_context.h"
#include "imgui_test_engine/imgui_te_utils.h"       // InputText() with Str
#include "imgui_test_engine/thirdparty/Str/Str.h"

// Warnings
#ifdef _MSC_VER
#pragma warning (disable: 4100) // unreferenced formal parameter
#pragma warning (disable: 4127) // conditional expression is constant
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

//-------------------------------------------------------------------------
// Ideas/Specs for future tests
// It is important we take the habit to write those down.
// - Even if we don't implement the test right away: they allow us to remember edge cases and interesting things to test.
// - Even if they will be hard to actually implement/automate, they still allow us to manually check things.
//-------------------------------------------------------------------------
// TODO: Tests: InputText: read-only + callback (#4762)
// TODO: Tests: InputTextMultiline(): (ImDrawList) position of text (#4794)
// TODO: Tests: MenuItemEx() with icon (what to test?)
// TODO: Tests: TabBar: test shrinking large number of tabs, that right-most tab edge touches exactly the edge of the tab bar.
// TODO: Tests: test SetColorEditOptions(0) then restore value
//-------------------------------------------------------------------------


// Helpers
static inline bool operator==(const ImVec2& lhs, const ImVec2& rhs)     { return lhs.x == rhs.x && lhs.y == rhs.y; }    // for IM_CHECK_EQ()
static inline bool operator==(const ImVec4& lhs, const ImVec4& rhs)     { return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.w == rhs.w; }
static inline bool operator!=(const ImVec4& lhs, const ImVec4& rhs)     { return lhs.x != rhs.x || lhs.y != rhs.y || lhs.z != rhs.z || lhs.w != rhs.w; }

typedef ImGuiDataTypeTempStorage ImGuiDataTypeStorage; // Will rename in imgui/ later
void GetSliderTestRanges(ImGuiDataType data_type, ImGuiDataTypeStorage* min_p, ImGuiDataTypeStorage* max_p)
{
    switch (data_type)
    {
    case ImGuiDataType_S8:
        *(int8_t*)(void*)min_p = INT8_MIN;
        *(int8_t*)(void*)max_p = INT8_MAX;
        break;
    case ImGuiDataType_U8:
        *(uint8_t*)(void*)min_p = 0;
        *(uint8_t*)(void*)max_p = UINT8_MAX;
        break;
    case ImGuiDataType_S16:
        *(int16_t*)(void*)min_p = INT16_MIN;
        *(int16_t*)(void*)max_p = INT16_MAX;
        break;
    case ImGuiDataType_U16:
        *(uint16_t*)(void*)min_p = 0;
        *(uint16_t*)(void*)max_p = UINT16_MAX;
        break;
    case ImGuiDataType_S32:
        *(int32_t*)(void*)min_p = INT32_MIN / 2;
        *(int32_t*)(void*)max_p = INT32_MAX / 2;
        break;
    case ImGuiDataType_U32:
        *(uint32_t*)(void*)min_p = 0;
        *(uint32_t*)(void*)max_p = UINT32_MAX / 2;
        break;
    case ImGuiDataType_S64:
        *(int64_t*)(void*)min_p = INT64_MIN / 2;
        *(int64_t*)(void*)max_p = INT64_MAX / 2;
        break;
    case ImGuiDataType_U64:
        *(uint64_t*)(void*)min_p = 0;
        *(uint64_t*)(void*)max_p = UINT64_MAX / 2;
        break;
    case ImGuiDataType_Float:
        *(float*)(void*)min_p = -1000000000.0f; // Floating point types do not use their min/max supported values because widgets
        *(float*)(void*)max_p = +1000000000.0f; // may not be able to display them due to lossy RoundScalarWithFormatT().
        break;
    case ImGuiDataType_Double:
        *(double*)(void*)min_p = -1000000000.0;
        *(double*)(void*)max_p = +1000000000.0;
        break;
    default:
        IM_ASSERT(0);
    }
}

//-------------------------------------------------------------------------
// Tests: Widgets
//-------------------------------------------------------------------------

struct StrVars { Str str; };

void RegisterTests_Widgets(ImGuiTestEngine* e)
{
    ImGuiTest* t = NULL;

    // ## Test basic button presses
    t = IM_REGISTER_TEST(e, "widgets", "widgets_button_press");
    struct ButtonPressTestVars { int ButtonPressCount[6] = { 0 }; };
    t->SetVarsDataType<ButtonPressTestVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ButtonPressTestVars& vars = ctx->GetVars<ButtonPressTestVars>();

        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        if (ImGui::Button("Button0"))
            vars.ButtonPressCount[0]++;
        if (ImGui::ButtonEx("Button1", ImVec2(0, 0), ImGuiButtonFlags_PressedOnDoubleClick))
            vars.ButtonPressCount[1]++;
        if (ImGui::ButtonEx("Button2", ImVec2(0, 0), ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_PressedOnDoubleClick))
            vars.ButtonPressCount[2]++;
        if (ImGui::ButtonEx("Button3", ImVec2(0, 0), ImGuiButtonFlags_PressedOnClickReleaseAnywhere))
            vars.ButtonPressCount[3]++;
        if (ImGui::ButtonEx("Button4", ImVec2(0, 0), ImGuiButtonFlags_Repeat))
            vars.ButtonPressCount[4]++;
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ButtonPressTestVars& vars = ctx->GetVars<ButtonPressTestVars>();
        ImGuiContext& g = *ctx->UiContext;

        ctx->SetRef("Test Window");
        ctx->ItemClick("Button0");
        IM_CHECK_EQ(vars.ButtonPressCount[0], 1);
        ctx->ItemDoubleClick("Button1");
        IM_CHECK_EQ(vars.ButtonPressCount[1], 1);
        ctx->ItemDoubleClick("Button2");
        IM_CHECK_EQ(vars.ButtonPressCount[2], 2);

        // Test ImGuiButtonFlags_PressedOnClickRelease vs ImGuiButtonFlags_PressedOnClickReleaseAnywhere
        vars.ButtonPressCount[2] = 0;
        ctx->MouseMove("Button2");
        ctx->MouseDown(0);
        ctx->MouseMove("Button0", ImGuiTestOpFlags_NoCheckHoveredId);
        ctx->MouseUp(0);
        IM_CHECK_EQ(vars.ButtonPressCount[2], 0);
        ctx->MouseMove("Button3");
        ctx->MouseDown(0);
        ctx->MouseMove("Button0", ImGuiTestOpFlags_NoCheckHoveredId);
        ctx->MouseUp(0);
        IM_CHECK_EQ(vars.ButtonPressCount[3], 1);

        // Test ImGuiButtonFlags_Repeat
        const float step = ImMin(g.IO.KeyRepeatDelay, g.IO.KeyRepeatRate) * 0.50f;
        ctx->ItemClick("Button4");
        IM_CHECK_EQ(vars.ButtonPressCount[4], 1);
        ctx->MouseDown(0);
        IM_CHECK_EQ(vars.ButtonPressCount[4], 1);
        ctx->SleepNoSkip(g.IO.KeyRepeatDelay, step);
        ctx->SleepNoSkip(g.IO.KeyRepeatRate, step);
        ctx->SleepNoSkip(g.IO.KeyRepeatRate, step);
        ctx->SleepNoSkip(g.IO.KeyRepeatRate, step);
#if IMGUI_VERSION_NUM >= 18705
        IM_CHECK_EQ(vars.ButtonPressCount[4], 1 + 1 + 3);
#else
        IM_CHECK_EQ(vars.ButtonPressCount[4], 1 + 1 + 3 * 2); // FIXME: MouseRepeatRate is double KeyRepeatRate, that's not documented / or that's a bug
#endif
        ctx->MouseUp(0);

        // Test ImGuiButtonFlags_Repeat with Nav
        ctx->NavEnableForWindow();
        ctx->NavMoveTo("Button4");
        vars.ButtonPressCount[4] = 0;
        ctx->KeyDown(ImGuiKey_Space);
        IM_CHECK_EQ(vars.ButtonPressCount[4], 1);
        ctx->SleepNoSkip(g.IO.KeyRepeatDelay, step);
        ctx->SleepNoSkip(g.IO.KeyRepeatRate, step);
        ctx->SleepNoSkip(g.IO.KeyRepeatRate, step);
        ctx->SleepNoSkip(g.IO.KeyRepeatRate, step);
        ctx->KeyUp(ImGuiKey_Space);
#if IMGUI_VERSION_NUM >= 18804
        IM_CHECK_EQ(vars.ButtonPressCount[4], 1 + 1 + 3);
#else
        IM_CHECK_EQ(vars.ButtonPressCount[4], 1 + 3 * 2);
#endif
    };

    // ## Test basic button presses
    t = IM_REGISTER_TEST(e, "widgets", "widgets_button_mouse_buttons");
    t->SetVarsDataType<ButtonPressTestVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ButtonPressTestVars& vars = ctx->GetVars<ButtonPressTestVars>();

        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        if (ImGui::ButtonEx("ButtonL", ImVec2(0, 0), ImGuiButtonFlags_MouseButtonLeft))
            vars.ButtonPressCount[0]++;
        if (ImGui::ButtonEx("ButtonR", ImVec2(0, 0), ImGuiButtonFlags_MouseButtonRight))
            vars.ButtonPressCount[1]++;
        if (ImGui::ButtonEx("ButtonM", ImVec2(0, 0), ImGuiButtonFlags_MouseButtonMiddle))
            vars.ButtonPressCount[2]++;
        if (ImGui::ButtonEx("ButtonLR", ImVec2(0, 0), ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight))
            vars.ButtonPressCount[3]++;

        if (ImGui::ButtonEx("ButtonL-release", ImVec2(0, 0), ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_PressedOnRelease))
            vars.ButtonPressCount[4]++;
        if (ImGui::ButtonEx("ButtonR-release", ImVec2(0, 0), ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_PressedOnRelease))
        {
            ctx->LogDebug("Pressed!");
            vars.ButtonPressCount[5]++;
        }
        for (int n = 0; n < IM_ARRAYSIZE(vars.ButtonPressCount); n++)
            ImGui::Text("%d: %d", n, vars.ButtonPressCount[n]);

        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ButtonPressTestVars& vars = ctx->GetVars<ButtonPressTestVars>();

        ctx->SetRef("Test Window");
        ctx->ItemClick("ButtonL", 0);
        IM_CHECK_EQ(vars.ButtonPressCount[0], 1);
        ctx->ItemClick("ButtonR", 1);
        IM_CHECK_EQ(vars.ButtonPressCount[1], 1);
        ctx->ItemClick("ButtonM", 2);
        IM_CHECK_EQ(vars.ButtonPressCount[2], 1);
        ctx->ItemClick("ButtonLR", 0);
        ctx->ItemClick("ButtonLR", 1);
        IM_CHECK_EQ(vars.ButtonPressCount[3], 2);

        vars.ButtonPressCount[3] = 0;
        ctx->MouseMove("ButtonLR");
        ctx->MouseDown(0);
        ctx->MouseDown(1);
        ctx->MouseUp(0);
        ctx->MouseUp(1);
        IM_CHECK_EQ(vars.ButtonPressCount[3], 1);

        vars.ButtonPressCount[3] = 0;
        ctx->MouseMove("ButtonLR");
        ctx->MouseDown(0);
        ctx->MouseMove("ButtonR", ImGuiTestOpFlags_NoCheckHoveredId);
        ctx->MouseDown(1);
        ctx->MouseUp(0);
        ctx->MouseMove("ButtonLR");
        ctx->MouseUp(1);
        IM_CHECK_EQ(vars.ButtonPressCount[3], 0);
    };

    // ## Test ButtonBehavior frame by frame behaviors (see comments at the top of the ButtonBehavior() function)
    enum ButtonStateMachineTestStep
    {
        ButtonStateMachineTestStep_None,
        ButtonStateMachineTestStep_Init,
        ButtonStateMachineTestStep_MovedOver,
        ButtonStateMachineTestStep_MouseDown,
        ButtonStateMachineTestStep_MovedAway,
        ButtonStateMachineTestStep_MovedOverAgain,
        ButtonStateMachineTestStep_MouseUp,
        ButtonStateMachineTestStep_Done
    };
    t = IM_REGISTER_TEST(e, "widgets", "widgets_button_status");
    struct ButtonStateTestVars
    {
        ButtonStateMachineTestStep NextStep;
        ImGuiTestGenericItemStatus Status;
        ButtonStateTestVars() { NextStep = ButtonStateMachineTestStep_None; }
    };
    t->SetVarsDataType<ButtonStateTestVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ButtonStateTestVars& vars = ctx->GetVars<ButtonStateTestVars>();
        ImGuiTestGenericItemStatus& status = vars.Status;

        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);

        const bool pressed = ImGui::Button("Test");
        status.QuerySet();
        switch (vars.NextStep)
        {
        case ButtonStateMachineTestStep_Init:
            IM_CHECK(0 == pressed);
            IM_CHECK(0 == status.Hovered);
            IM_CHECK(0 == status.Active);
            IM_CHECK(0 == status.Activated);
            IM_CHECK(0 == status.Deactivated);
            break;
        case ButtonStateMachineTestStep_MovedOver:
            IM_CHECK(0 == pressed);
            IM_CHECK(1 == status.Hovered);
            IM_CHECK(0 == status.Active);
            IM_CHECK(0 == status.Activated);
            IM_CHECK(0 == status.Deactivated);
            break;
        case ButtonStateMachineTestStep_MouseDown:
            IM_CHECK(0 == pressed);
            IM_CHECK(1 == status.Hovered);
            IM_CHECK(1 == status.Active);
            IM_CHECK(1 == status.Activated);
            IM_CHECK(0 == status.Deactivated);
            break;
        case ButtonStateMachineTestStep_MovedAway:
            IM_CHECK(0 == pressed);
            IM_CHECK(0 == status.Hovered);
            IM_CHECK(1 == status.Active);
            IM_CHECK(0 == status.Activated);
            IM_CHECK(0 == status.Deactivated);
            break;
        case ButtonStateMachineTestStep_MovedOverAgain:
            IM_CHECK(0 == pressed);
            IM_CHECK(1 == status.Hovered);
            IM_CHECK(1 == status.Active);
            IM_CHECK(0 == status.Activated);
            IM_CHECK(0 == status.Deactivated);
            break;
        case ButtonStateMachineTestStep_MouseUp:
            IM_CHECK(1 == pressed);
            IM_CHECK(1 == status.Hovered);
            IM_CHECK(0 == status.Active);
            IM_CHECK(0 == status.Activated);
            IM_CHECK(1 == status.Deactivated);
            break;
        case ButtonStateMachineTestStep_Done:
            IM_CHECK(0 == pressed);
            IM_CHECK(0 == status.Hovered);
            IM_CHECK(0 == status.Active);
            IM_CHECK(0 == status.Activated);
            IM_CHECK(0 == status.Deactivated);
            break;
        default:
            break;
        }
        vars.NextStep = ButtonStateMachineTestStep_None;

        // The "Unused" button allows to move the mouse away from the "Test" button
        ImGui::Button("Unused");

        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ButtonStateTestVars& vars = ctx->GetVars<ButtonStateTestVars>();
        vars.NextStep = ButtonStateMachineTestStep_None;

        ctx->SetRef("Test Window");

        // Move mouse away from "Test" button
        ctx->MouseMove("Unused");
        vars.NextStep = ButtonStateMachineTestStep_Init;
        ctx->Yield();

        ctx->MouseMove("Test");
        vars.NextStep = ButtonStateMachineTestStep_MovedOver;
        ctx->Yield();

        vars.NextStep = ButtonStateMachineTestStep_MouseDown;
        ctx->MouseDown();

        ctx->MouseMove("Unused", ImGuiTestOpFlags_NoCheckHoveredId);
        vars.NextStep = ButtonStateMachineTestStep_MovedAway;
        ctx->Yield();

        ctx->MouseMove("Test");
        vars.NextStep = ButtonStateMachineTestStep_MovedOverAgain;
        ctx->Yield();

        vars.NextStep = ButtonStateMachineTestStep_MouseUp;
        ctx->MouseUp();

        ctx->MouseMove("Unused");
        vars.NextStep = ButtonStateMachineTestStep_Done;
        ctx->Yield();
    };

    // ## Test checkbox click
    t = IM_REGISTER_TEST(e, "widgets", "widgets_checkbox_001");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGui::Begin("Window1", NULL, ImGuiWindowFlags_NoSavedSettings);
        ImGui::Checkbox("Checkbox", &vars.Bool1);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        // We use WindowRef() to ensure the window is uncollapsed.
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        IM_CHECK(vars.Bool1 == false);
        ctx->SetRef("Window1");
        ctx->ItemClick("Checkbox");
        IM_CHECK(vars.Bool1 == true);
    };

    // ## Test all types with DragScalar().
    t = IM_REGISTER_TEST(e, "widgets", "widgets_datatype_1");
    struct DragDatatypeVars { int widget_type = 0; ImGuiDataType data_type = 0; char data_storage[10] = ""; char data_zero[8] = ""; ImGuiTestGenericItemStatus Status; };
    t->SetVarsDataType<DragDatatypeVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        DragDatatypeVars& vars = ctx->GetVars<DragDatatypeVars>();
        ImGui::SetNextWindowSize(ImVec2(200, 200));
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        bool ret;
        if (vars.widget_type == 0)
            ret = ImGui::DragScalar("Drag", vars.data_type, &vars.data_storage[1], 0.5f);
        else
            ret = ImGui::SliderScalar("Slider", vars.data_type, &vars.data_storage[1], &vars.data_zero, &vars.data_zero);
        vars.Status.QueryInc(ret);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiContext& g = *ctx->UiContext;
        DragDatatypeVars& vars = ctx->GetVars<DragDatatypeVars>();

        ctx->SetRef("Test Window");
        for (int widget_type = 0; widget_type < 2; widget_type++)
        {
            for (int data_type = 0; data_type < ImGuiDataType_COUNT; data_type++)
            {
                size_t data_size = ImGui::DataTypeGetInfo(data_type)->Size;
                IM_ASSERT(data_size + 2 <= sizeof(vars.data_storage));
                memset(vars.data_storage, 0, sizeof(vars.data_storage));
                memset(vars.data_zero, 0, sizeof(vars.data_zero));
                vars.widget_type = widget_type;
                vars.data_type = data_type;
                vars.data_storage[0] = vars.data_storage[1 + data_size] = 42; // Sentinel values
                const char* widget_name = widget_type == 0 ? "Drag" : "Slider";

                if (widget_type == 0)
                {
                    vars.Status.Clear();
                    ctx->MouseMove(widget_name);
                    ctx->MouseDown();
                    ctx->MouseMoveToPos(g.IO.MousePos + ImVec2(30, 0));
                    IM_CHECK(vars.Status.Edited >= 1);
                    vars.Status.Clear();
                    ctx->MouseMoveToPos(g.IO.MousePos + ImVec2(-40, 0));
                    IM_CHECK(vars.Status.Edited >= 1);
                    ctx->MouseUp();
                }

                vars.Status.Clear();
                ctx->ItemInput(widget_name);
                ctx->KeyChars("123");                               // Case fixed by PR #3231
                IM_CHECK_GE(vars.Status.Ret, 1);
                IM_CHECK_GE(vars.Status.Edited, 1);
                vars.Status.Clear();
                ctx->Yield();
                IM_CHECK_EQ(vars.Status.Ret, 0);        // Verify it doesn't keep returning as edited.
                IM_CHECK_EQ(vars.Status.Edited, 0);

                vars.Status.Clear();
                ctx->KeyPress(ImGuiKey_Enter);
                IM_CHECK(vars.data_storage[0] == 42);               // Ensure there were no oob writes.
                IM_CHECK(vars.data_storage[1 + data_size] == 42);
            }
        }
    };

    // ## Test DragInt() as InputText
    // ## Test ColorEdit4() as InputText (#2557)
    t = IM_REGISTER_TEST(e, "widgets", "widgets_dragslider_as_input");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        ImGui::DragInt("Drag", &vars.Int1);
        ImGui::ColorEdit4("Color", &vars.Color1.x);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ctx->SetRef("Test Window");

        IM_CHECK_EQ(vars.Int1, 0);
        ctx->ItemInput("Drag");
        IM_CHECK_EQ(ctx->UiContext->ActiveId, ctx->GetID("Drag"));
        ctx->KeyCharsAppendEnter("123");
        IM_CHECK_EQ(vars.Int1, 123);

        ctx->ItemInput("Color##Y");
        IM_CHECK_EQ(ctx->UiContext->ActiveId, ctx->GetID("Color##Y"));
        ctx->KeyCharsAppend("123");
        IM_CHECK_FLOAT_EQ_EPS(vars.Color1.y, 123.0f / 255.0f);
        ctx->KeyPress(ImGuiKey_Tab);
        ctx->KeyCharsAppendEnter("200");
        IM_CHECK_FLOAT_EQ_EPS(vars.Color1.x,   0.0f / 255.0f);
        IM_CHECK_FLOAT_EQ_EPS(vars.Color1.y, 123.0f / 255.0f);
        IM_CHECK_FLOAT_EQ_EPS(vars.Color1.z, 200.0f / 255.0f);
    };

    // ## Test Sliders and Drags clamping values
    t = IM_REGISTER_TEST(e, "widgets", "widgets_dragslider_clamping");
    struct ImGuiDragSliderVars { float DragValue = 0.0f; float DragMin = 0.0f; float DragMax = 1.0f; float SliderValue = 0.0f; float SliderMin = 0.0f; float SliderMax = 0.0f; float ScalarValue = 0.0f; void* ScalarMinP = NULL; void* ScalarMaxP = NULL; ImGuiSliderFlags Flags = ImGuiSliderFlags_None; };
    t->SetVarsDataType<ImGuiDragSliderVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiDragSliderVars& vars = ctx->GetVars<ImGuiDragSliderVars>();
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        const char* format = "%.3f";
        ImGui::SliderFloat("Slider", &vars.SliderValue, vars.SliderMin, vars.SliderMax, format, vars.Flags);
        ImGui::DragFloat("Drag", &vars.DragValue, 1.0f, vars.DragMin, vars.DragMax, format, vars.Flags);
        ImGui::DragScalar("Scalar", ImGuiDataType_Float, &vars.ScalarValue, 1.0f, vars.ScalarMinP, vars.ScalarMaxP, NULL, vars.Flags);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiContext& g = *ImGui::GetCurrentContext();
        ImGuiDragSliderVars& vars = ctx->GetVars<ImGuiDragSliderVars>();
        ctx->SetRef("Test Window");
        ImGuiSliderFlags flags[] = { ImGuiSliderFlags_None, ImGuiSliderFlags_AlwaysClamp };
        for (int i = 0; i < IM_ARRAYSIZE(flags); ++i)
        {
            bool clamp_on_input = flags[i] == ImGuiSliderFlags_AlwaysClamp;
            vars.Flags = flags[i];

            float slider_min_max[][2] = { {0.0f, 1.0f}, {0.0f, 0.0f} };
            for (int j = 0; j < IM_ARRAYSIZE(slider_min_max); ++j)
            {
                ctx->LogInfo("## Slider %d with Flags = 0x%08X", j, vars.Flags);

                vars.SliderValue = 0.0f;
                vars.SliderMin = slider_min_max[j][0];
                vars.SliderMax = slider_min_max[j][1];

                ctx->ItemInputValue("Slider", 2);
                IM_CHECK_EQ(vars.SliderValue, clamp_on_input ? vars.SliderMax : 2.0f);

                // Check higher bound
                ctx->MouseMove("Slider", ImGuiTestOpFlags_MoveToEdgeR);
                ctx->MouseDown(); // Click will update clamping
                IM_CHECK_EQ(vars.SliderValue, vars.SliderMax);
                ctx->MouseMoveToPos(g.IO.MousePos + ImVec2(100, 0));
                ctx->MouseUp();
                IM_CHECK_EQ(vars.SliderValue, vars.SliderMax);

                ctx->ItemInputValue("Slider", -2);
                IM_CHECK_EQ(vars.SliderValue, clamp_on_input ? vars.SliderMin : -2.0f);

                // Check lower bound
                ctx->MouseMove("Slider", ImGuiTestOpFlags_MoveToEdgeL);
                ctx->MouseDown(); // Click will update clamping
                IM_CHECK_EQ(vars.SliderValue, vars.SliderMin);
                ctx->MouseMoveToPos(g.IO.MousePos - ImVec2(100, 0));
                ctx->MouseUp();
                IM_CHECK_EQ(vars.SliderValue, vars.SliderMin);
            }

            float drag_min_max[][2] = { {0.0f, 1.0f}, {0.0f, 0.0f}, {-FLT_MAX, FLT_MAX} };
            for (int j = 0; j < IM_ARRAYSIZE(drag_min_max); ++j)
            {
                ctx->LogDebug("Drag %d with flags = 0x%08X", j, vars.Flags);

                vars.DragValue = 0.0f;
                vars.DragMin = drag_min_max[j][0];
                vars.DragMax = drag_min_max[j][1];

                // [0,0] is equivalent to [-FLT_MAX, FLT_MAX] range
                bool unbound = (vars.DragMin == 0.0f && vars.DragMax == 0.0f) || (vars.DragMin == -FLT_MAX && vars.DragMax == FLT_MAX);
                float value_before_click = 0.0f;

                ctx->ItemInputValue("Drag", -3);
                IM_CHECK_EQ(vars.DragValue, clamp_on_input && !unbound ? vars.DragMin : -3.0f);

                ctx->ItemInputValue("Drag", 2);
                IM_CHECK_EQ(vars.DragValue, clamp_on_input && !unbound ? vars.DragMax : 2.0f);

                // Check higher bound
                ctx->MouseMove("Drag");
                value_before_click = vars.DragValue;
                ctx->MouseDown(); // Click will not update clamping value
                IM_CHECK_EQ(vars.DragValue, value_before_click);
                ctx->MouseMoveToPos(g.IO.MousePos + ImVec2(100, 0));
                ctx->MouseUp();
                if (unbound)
                    IM_CHECK_GT(vars.DragValue, value_before_click);
                else
                    IM_CHECK_EQ(vars.DragValue, value_before_click);

                // Check higher to lower bound
                value_before_click = vars.DragValue;
                ctx->MouseMove("Drag");
                ctx->MouseDragWithDelta(ImVec2(-100, 0));
                if (unbound)
                    IM_CHECK_LT(vars.DragValue, value_before_click);
                else
                    IM_CHECK_EQ(vars.DragValue, vars.DragMin);

                // Check low to high bound
                value_before_click = vars.DragValue;
                ctx->MouseMove("Drag");
                ctx->MouseDragWithDelta(ImVec2(100, 0));
                if (unbound)
                    IM_CHECK_GT(vars.DragValue, value_before_click);
                else
                    IM_CHECK_EQ(vars.DragValue, vars.DragMax);
            }

            float scalar_min_max[][2] = { {-FLT_MAX, 1.0f}, {0.0f, FLT_MAX} };
            for (int j = 0; j < IM_ARRAYSIZE(scalar_min_max); ++j)
            {
                ctx->LogDebug("Scalar %d with flags = 0x%08X", j, vars.Flags);

                vars.ScalarValue = 0.0f;
                vars.DragMin = scalar_min_max[j][0];    // No harm in reusing DragMin/DragMax.
                vars.DragMax = scalar_min_max[j][1];
                vars.ScalarMinP = (vars.DragMin == -FLT_MAX || vars.DragMin == +FLT_MAX) ? NULL : &vars.DragMin;
                vars.ScalarMaxP = (vars.DragMax == -FLT_MAX || vars.DragMax == +FLT_MAX) ? NULL : &vars.DragMax;

                bool unbound_min = vars.ScalarMinP == NULL;
                bool unbound_max = vars.ScalarMaxP == NULL;
                float value_before_click = 0.0f;

                ctx->ItemInputValue("Scalar", -3);
                IM_CHECK_EQ(vars.ScalarValue, clamp_on_input && !unbound_min ? vars.DragMin : -3.0f);

                ctx->ItemInputValue("Scalar", 2);
                IM_CHECK_EQ(vars.ScalarValue, clamp_on_input && !unbound_max ? vars.DragMax : 2.0f);

                // Check higher bound
                ctx->MouseMove("Scalar");
                value_before_click = vars.ScalarValue;
                ctx->MouseDown(); // Click will not update clamping value
                IM_CHECK_EQ(vars.ScalarValue, value_before_click);
                ctx->MouseMoveToPos(g.IO.MousePos + ImVec2(100, 0));
                ctx->MouseUp();
                if (unbound_max)
                    IM_CHECK_GT(vars.ScalarValue, value_before_click);
                else
                    IM_CHECK_EQ(vars.ScalarValue, value_before_click);

                // Check higher to lower bound
                value_before_click = vars.ScalarValue = 50.0f;
                ctx->MouseMove("Scalar");
                ctx->MouseDragWithDelta(ImVec2(-100, 0));
                if (unbound_min)
                    IM_CHECK_LT(vars.ScalarValue, value_before_click);
                else
                    IM_CHECK_EQ(vars.ScalarValue, vars.DragMin);

                // Check low to high bound
                value_before_click = vars.ScalarValue = 0.0f;
                ctx->MouseMove("Scalar");
                ctx->MouseDragWithDelta(ImVec2(100, 0));
                if (unbound_max)
                    IM_CHECK_GT(vars.ScalarValue, value_before_click);
                else
                    IM_CHECK_EQ(vars.ScalarValue, vars.DragMax);
            }
        }
    };

#if 0
    // ## Testing Scroll on Window with Test Engine helpers
    t = IM_REGISTER_TEST(e, "widgets", "widgets_scrollbar");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGui::SetNextWindowSize(ImVec2(200, 200));
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysHorizontalScrollbar);
        ImGui::BeginChild("##Child", ImVec2(800, 800), true);
        ImGui::Text("This is some text");
        ImGui::EndChild();
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ctx->SetRef("Test Window");
        ctx->ScrollTo(ImGuiAxis_Y, 60);
        ctx->ScrollTo(ImGuiAxis_X, 80);
    };
#endif

    // ## Test for IsItemHovered() on BeginChild() and InputTextMultiLine() (#1370, #3851)
    t = IM_REGISTER_TEST(e, "widgets", "widgets_hover");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);

        ImGui::BeginChild("Child", ImVec2(100, 100), true);
        vars.Pos = ImGui::GetCursorScreenPos();
        ImGui::Dummy(ImVec2(10, 10));
        ImGui::Button("button");
        vars.Id = ImGui::GetItemID();
        ImGui::EndChild();
        vars.Bool1 = ImGui::IsItemHovered();
        ImGui::Text("hovered: %d", vars.Bool1);

        ImGui::InputTextMultiline("##Field", vars.Str1, IM_ARRAYSIZE(vars.Str1), ImVec2(-FLT_MIN, 0.0f));
        vars.Bool2 = ImGui::IsItemHovered();
        ImGui::Text("hovered: %d", vars.Bool2);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ctx->SetRef("Test Window");

        // move on child's void
        ctx->MouseMoveToPos(vars.Pos);
        IM_CHECK(vars.Bool1 == true);
        IM_CHECK(vars.Bool2 == false);

        // move on child's item
        ctx->MouseMove(vars.Id);
        IM_CHECK(vars.Bool1 == true);
        IM_CHECK(vars.Bool2 == false);

        // move on InputTextMultiline() = child embedded in group
        ctx->MouseMove("##Field");
        IM_CHECK(vars.Bool1 == false);
        IM_CHECK(vars.Bool2 == true);
    };

    // ## Test InputText widget
    t = IM_REGISTER_TEST(e, "widgets", "widgets_inputtext_basic");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGui::SetNextWindowSize(ImVec2(200, 200));
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        ImGui::InputText("InputText", vars.Str1, IM_ARRAYSIZE(vars.Str1), vars.Bool1 ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_None);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGuiInputTextState& state = ctx->UiContext->InputTextState;
        char* buf = vars.Str1;

        ctx->SetRef("Test Window");

        // Insert
        strcpy(buf, "Hello");
        ctx->ItemClick("InputText");
        ctx->KeyCharsAppendEnter(u8"World123\u00A9");
        IM_CHECK_STR_EQ(buf, u8"HelloWorld123\u00A9");
        IM_CHECK_EQ(state.CurLenA, 15);
        IM_CHECK_EQ(state.CurLenW, 14);

        // Delete
        ctx->ItemClick("InputText");
        ctx->KeyPress(ImGuiKey_End);
        ctx->KeyPress(ImGuiKey_LeftArrow, ImGuiModFlags_Shift, 2);    // Select last two characters
        ctx->KeyPress(ImGuiKey_Backspace, ImGuiModFlags_None, 3);     // Delete selection and two more characters
        ctx->KeyPress(ImGuiKey_Enter);
        IM_CHECK_STR_EQ(buf, "HelloWorld");
        IM_CHECK_EQ(state.CurLenA, 10);
        IM_CHECK_EQ(state.CurLenW, 10);

        // Insert, Cancel
        ctx->ItemClick("InputText");
        ctx->KeyPress(ImGuiKey_End);
        ctx->KeyChars("XXXXX");
        ctx->KeyPress(ImGuiKey_Escape);
        IM_CHECK_STR_EQ(buf, "HelloWorld");
        IM_CHECK_EQ(state.CurLenA, 10);
        IM_CHECK_EQ(state.CurLenW, 10);

        // Delete, Cancel
        ctx->ItemClick("InputText");
        ctx->KeyPress(ImGuiKey_End);
        ctx->KeyPress(ImGuiKey_Backspace, ImGuiModFlags_None, 5);
        ctx->KeyPress(ImGuiKey_Escape);
        IM_CHECK_STR_EQ(buf, "HelloWorld");
        IM_CHECK_EQ(state.CurLenA, 10);
        IM_CHECK_EQ(state.CurLenW, 10);

        // Read-only mode
        strcpy(buf, "Some read-only text.");
        vars.Bool1 = true;
        ctx->Yield();
        ctx->ItemClick("InputText");
        ctx->KeyCharsAppendEnter("World123");
        IM_CHECK_STR_EQ(buf, vars.Str1);
        IM_CHECK_EQ(state.CurLenA, 20);
        IM_CHECK_EQ(state.CurLenW, 20);

        // Space as key (instead of Space as character) -> check not conflicting with Nav Activate (#4552)
        vars.Bool1 = false;
        ctx->ItemClick("InputText");
        ctx->KeyCharsReplace("Hello");
        IM_CHECK(ImGui::GetActiveID() == ctx->GetID("InputText"));
        ctx->KeyPress(ImGuiKey_Space); // Should not add text, should not validate
        IM_CHECK(ImGui::GetActiveID() == ctx->GetID("InputText"));
        IM_CHECK_STR_EQ(vars.Str1, "Hello");
    };

    // ## Test InputText undo/redo ops, in particular related to issue we had with stb_textedit undo/redo buffers
    t = IM_REGISTER_TEST(e, "widgets", "widgets_inputtext_undo_redo");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        if (vars.StrLarge.empty())
            vars.StrLarge.resize(10000, 0);
        ImGui::SetNextWindowSize(ImVec2(ImGui::GetFontSize() * 50, 0.0f));
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("strlen() = %d", (int)strlen(vars.StrLarge.Data));
        ImGui::InputText("Other", vars.Str1, IM_ARRAYSIZE(vars.Str1), ImGuiInputTextFlags_None);
        ImGui::InputTextMultiline("InputText", vars.StrLarge.Data, (size_t)vars.StrLarge.Size, ImVec2(-1, ImGui::GetFontSize() * 20), ImGuiInputTextFlags_None);
        ImGui::End();
        //DebugInputTextState();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        // https://github.com/nothings/stb/issues/321
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGuiContext& g = *ctx->UiContext;

        // Start with a 350 characters buffer.
        // For this test we don't inject the characters via pasting or key-by-key in order to precisely control the undo/redo state.
        char* buf = vars.StrLarge.Data;
        IM_CHECK_EQ((int)strlen(buf), 0);
        for (int n = 0; n < 10; n++)
            strcat(buf, "xxxxxxx abcdefghijklmnopqrstuvwxyz\n");
        IM_CHECK_EQ((int)strlen(buf), 350);

        ctx->SetRef("Test Window");
        ctx->ItemClick("Other"); // This is to ensure stb_textedit_clear_state() gets called (clear the undo buffer, etc.)
        ctx->ItemClick("InputText");

        ImGuiInputTextState& input_text_state = g.InputTextState;
        ImStb::StbUndoState& undo_state = input_text_state.Stb.undostate;
        IM_CHECK_EQ(input_text_state.ID, g.ActiveId);
        IM_CHECK_EQ(undo_state.undo_point, 0);
        IM_CHECK_EQ(undo_state.undo_char_point, 0);
        IM_CHECK_EQ(undo_state.redo_point, STB_TEXTEDIT_UNDOSTATECOUNT);
        IM_CHECK_EQ(undo_state.redo_char_point, STB_TEXTEDIT_UNDOCHARCOUNT);
        IM_CHECK_EQ(STB_TEXTEDIT_UNDOCHARCOUNT, 999); // Test designed for this value

        // Insert 350 characters via 10 paste operations
        // We use paste operations instead of key-by-key insertion so we know our undo buffer will contains 10 undo points.
        //const char line_buf[26+8+1+1] = "xxxxxxx abcdefghijklmnopqrstuvwxyz\n"; // 8+26+1 = 35
        //ImGui::SetClipboardText(line_buf);
        //IM_CHECK(strlen(line_buf) == 35);
        //ctx->KeyPress(ImGuiKey_V, ImGuiModFlags_Shortcut, 10);

        // Select all, copy, paste 3 times
        ctx->KeyPress(ImGuiKey_A, ImGuiModFlags_Shortcut);    // Select all
        ctx->KeyPress(ImGuiKey_C, ImGuiModFlags_Shortcut);    // Copy
        ctx->KeyPress(ImGuiKey_End, ImGuiModFlags_Shortcut);  // Go to end, clear selection
        ctx->SleepStandard();
        for (int n = 0; n < 3; n++)
        {
            ctx->KeyPress(ImGuiKey_V, ImGuiModFlags_Shortcut);// Paste append three times
            ctx->SleepStandard();
        }
        int len = (int)strlen(vars.StrLarge.Data);
        IM_CHECK_EQ(len, 350 * 4);
        IM_CHECK_EQ(undo_state.undo_point, 3);
        IM_CHECK_EQ(undo_state.undo_char_point, 0);

        // Undo x2
        IM_CHECK(undo_state.redo_point == STB_TEXTEDIT_UNDOSTATECOUNT);
        ctx->KeyPress(ImGuiKey_Z, ImGuiModFlags_Shortcut);
        ctx->KeyPress(ImGuiKey_Z, ImGuiModFlags_Shortcut);
        len = (int)strlen(vars.StrLarge.Data);
        IM_CHECK_EQ(len, 350 * 2);
        IM_CHECK_EQ(undo_state.undo_point, 1);
        IM_CHECK_EQ(undo_state.redo_point, STB_TEXTEDIT_UNDOSTATECOUNT - 2);
        IM_CHECK_EQ(undo_state.redo_char_point, STB_TEXTEDIT_UNDOCHARCOUNT - 350 * 2);

        // Undo x1 should call stb_textedit_discard_redo()
        ctx->KeyPress(ImGuiKey_Z, ImGuiModFlags_Shortcut);
        len = (int)strlen(vars.StrLarge.Data);
        IM_CHECK_EQ(len, 350 * 1);
    };

#if IMGUI_VERSION_NUM >= 18727
    // ## Test undo stack reset when contents change while widget is inactive. (#4947's second bug, #2890)
    t = IM_REGISTER_TEST(e, "widgets", "widgets_inputtext_undo_reset");
    t->SetVarsDataType<StrVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        StrVars& vars = ctx->GetVars<StrVars>();
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        ImGui::InputText("Field1", &vars.str, ImGuiInputTextFlags_CallbackHistory);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiContext& g = *ctx->UiContext;
        StrVars& vars = ctx->GetVars<StrVars>();
        ctx->SetRef("Test Window");
        ctx->ItemClick("Field1");
        ctx->KeyCharsAppend("Hello, world!");
        IM_CHECK_GT(g.InputTextState.Stb.undostate.undo_point, 0);
        ctx->KeyPress(ImGuiKey_Escape);
        vars.str = "Foobar";
        ctx->ItemClick("Field1");
        IM_CHECK_EQ(g.InputTextState.Stb.undostate.undo_point, 0);
    };
#endif

#if IMGUI_VERSION_NUM >= 18727
    // ## Test undo/redo operations with modifications from callback. (#4947, #4949)
    t = IM_REGISTER_TEST(e, "widgets", "widgets_inputtext_undo_callback");
    t->SetVarsDataType<StrVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        auto callback = [](ImGuiInputTextCallbackData* data)
        {
            if (data->EventFlag == ImGuiInputTextFlags_CallbackHistory)
                data->InsertChars(data->CursorPos, ", world!");
            return 0;
        };

        StrVars& vars = ctx->GetVars<StrVars>();
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        ImGui::InputText("Field1", &vars.str, ImGuiInputTextFlags_CallbackHistory, callback);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        StrVars& vars = ctx->GetVars<StrVars>();
        ctx->SetRef("Test Window");
        ctx->ItemClick("Field1");
        ctx->KeyCharsAppend("Hello");
        ctx->KeyPress(ImGuiKey_DownArrow);                      // Trigger modification from callback.
        IM_CHECK_STR_EQ(vars.str.c_str(), "Hello, world!");
        ctx->KeyPress(ImGuiKey_Z, ImGuiModFlags_Shortcut);
        IM_CHECK_STR_EQ(vars.str.c_str(), "Hello");
        ctx->KeyPress(ImGuiKey_Y, ImGuiModFlags_Shortcut);
        IM_CHECK_STR_EQ(vars.str.c_str(), "Hello, world!");
    };
#endif

    // ## Test InputText vs user ownership of data
    t = IM_REGISTER_TEST(e, "widgets", "widgets_inputtext_text_ownership");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::LogToBuffer();
        ImGui::InputText("##InputText", vars.Str1, IM_ARRAYSIZE(vars.Str1)); // Remove label to simplify the capture/comparison
        ImStrncpy(vars.Str2, ctx->UiContext->LogBuffer.c_str(), IM_ARRAYSIZE(vars.Str2));
        ImGui::LogFinish();
        ImGui::Text("Captured: \"%s\"", vars.Str2);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        char* buf_user = vars.Str1;
        char* buf_visible = vars.Str2;
        ctx->SetRef("Test Window");

        IM_CHECK_STR_EQ(buf_visible, "{ }");
        strcpy(buf_user, "Hello");
        ctx->Yield();
        IM_CHECK_STR_EQ(buf_visible, "{ Hello }");
        ctx->ItemClick("##InputText");
        ctx->KeyCharsAppend("1");
        ctx->Yield();
        IM_CHECK_STR_EQ(buf_user, "Hello1");
        IM_CHECK_STR_EQ(buf_visible, "{ Hello1 }");

        // Because the item is active, it owns the source data, so:
        strcpy(buf_user, "Overwritten");
        ctx->Yield();
        IM_CHECK_STR_EQ(buf_user, "Hello1");
        IM_CHECK_STR_EQ(buf_visible, "{ Hello1 }");

        // Lose focus, at this point the InputTextState->ID should be holding on the last active state,
        // so we verify that InputText() is picking up external changes.
        ctx->KeyPress(ImGuiKey_Escape);
        IM_CHECK_EQ(ctx->UiContext->ActiveId, (unsigned)0);
        strcpy(buf_user, "Hello2");
        ctx->Yield();
        IM_CHECK_STR_EQ(buf_user, "Hello2");
        IM_CHECK_STR_EQ(buf_visible, "{ Hello2 }");
    };

    // ## Test that InputText doesn't go havoc when activated via another item
    t = IM_REGISTER_TEST(e, "widgets", "widgets_inputtext_id_conflict");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGui::SetNextWindowSize(ImVec2(ImGui::GetFontSize() * 50, 0.0f));
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        if (vars.Step == 0)
        {
            if (ctx->FrameCount < 50)
                ImGui::Button("Hello");
            else
                ImGui::InputText("Hello", vars.Str1, IM_ARRAYSIZE(vars.Str1));
        }
        else
        {
            ImGui::InputTextMultiline("Hello", vars.Str1, IM_ARRAYSIZE(vars.Str1));
        }
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;

        ctx->SetRef("Test Window");
        ctx->ItemHoldForFrames("Hello", 100);
        ctx->ItemClick("Hello");
        ImGuiInputTextState* state = ImGui::GetInputTextState(ctx->GetID("Hello"));
        IM_CHECK(state != NULL);
        IM_CHECK(state->Stb.single_line == 1);

        // Toggling from single to multiline is a little bit ill-defined
        vars.Step = 1;
        ctx->Yield();
        ctx->ItemClick("Hello");
        state = ImGui::GetInputTextState(ctx->GetID("Hello"));
        IM_CHECK(state != NULL);
        IM_CHECK(state->Stb.single_line == 0);
    };

    // ## Test that InputText doesn't append two tab characters if the backend supplies both tab key and character
    t = IM_REGISTER_TEST(e, "widgets", "widgets_inputtext_tab_double_insertion");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        ImGui::InputText("Field", vars.Str1, IM_ARRAYSIZE(vars.Str1), ImGuiInputTextFlags_AllowTabInput);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ctx->SetRef("Test Window");
        ctx->ItemClick("Field");
        ctx->UiContext->IO.AddInputCharacter((ImWchar)'\t');
        ctx->KeyPress(ImGuiKey_Tab);
        IM_CHECK_STR_EQ(vars.Str1, "\t");
    };

    // ## Test input clearing action (ESC key) being undoable (#3008).
    t = IM_REGISTER_TEST(e, "widgets", "widgets_inputtext_esc_undo");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        ImGui::InputText("Field", vars.Str1, IM_ARRAYSIZE(vars.Str1));
        ImGui::End();

    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        // FIXME-TESTS: Facilitate usage of variants
        const int test_count = ctx->HasDock ? 2 : 1;
        for (int test_n = 0; test_n < test_count; test_n++)
        {
            ctx->LogDebug("TEST CASE %d", test_n);
            const char* initial_value = (test_n == 0) ? "" : "initial";
            strcpy(vars.Str1, initial_value);
            ctx->SetRef("Test Window");
            ctx->ItemInput("Field");
            ctx->KeyCharsReplace("text");
            IM_CHECK_STR_EQ(vars.Str1, "text");
            ctx->KeyPress(ImGuiKey_Escape);                      // Reset input to initial value.
            IM_CHECK_STR_EQ(vars.Str1, initial_value);
            ctx->ItemInput("Field");
            ctx->KeyPress(ImGuiKey_Z, ImGuiModFlags_Shortcut);    // Undo
            IM_CHECK_STR_EQ(vars.Str1, "text");
            ctx->KeyPress(ImGuiKey_Enter);                       // Unfocus otherwise test_n==1 strcpy will fail
        }
    };

    // ## Test input text multiline cursor movement: left, up, right, down, origin, end, ctrl+origin, ctrl+end, page up, page down
    // ## Verify that text selection does not leak spaces in password fields. (#4155)
    // TODO ## Test input text multiline cursor with selection: left, up, right, down, origin, end, ctrl+origin, ctrl+end, page up, page down
    // TODO ## Test input text multiline scroll movement only: ctrl + (left, up, right, down)
    // TODO ## Test input text multiline page up/page down history ?
    t = IM_REGISTER_TEST(e, "widgets", "widgets_inputtext_cursor");
    struct InputTextCursorVars { Str str; int Cursor = 0; int LineCount = 10; Str64 Password; };
    t->SetVarsDataType<InputTextCursorVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        InputTextCursorVars& vars = ctx->GetVars<InputTextCursorVars>();

        float height = vars.LineCount * 0.5f * ImGui::GetFontSize();
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::InputTextMultiline("Field", &vars.str, ImVec2(300, height), ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::InputText("Password", &vars.Password, ImGuiInputTextFlags_Password);
        if (ImGuiInputTextState* state = ImGui::GetInputTextState(ctx->GetID("/Test Window/Field")))
            ImGui::Text("Stb Cursor: %d", state->Stb.cursor);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        InputTextCursorVars& vars = ctx->GetVars<InputTextCursorVars>();
        ctx->SetRef("Test Window");

        vars.str.clear();
        const int char_count_per_line = 10;
        for (int n = 0; n < vars.LineCount; n++)
        {
            for (int c = 0; c < char_count_per_line - 1; c++) // \n is part of our char_count_per_line
                vars.str.appendf("%d", n);
            if (n < vars.LineCount - 1)
                vars.str.append("\n");
        }
        ctx->ItemInput("Field");

        ImGuiInputTextState* state = ImGui::GetInputTextState(ctx->GetID("Field"));
        IM_CHECK(state != NULL);
        ImStb::STB_TexteditState& stb = state->Stb;
        vars.Cursor = stb.cursor;

        const int page_size = (vars.LineCount / 2) - 1;

        const int cursor_pos_begin_of_first_line = 0;
        const int cursor_pos_end_of_first_line = char_count_per_line - 1;
        const int cursor_pos_middle_of_first_line = char_count_per_line / 2;
        const int cursor_pos_end_of_last_line = vars.str.length();
        const int cursor_pos_begin_of_last_line = cursor_pos_end_of_last_line - char_count_per_line + 1;
        //const int cursor_pos_middle_of_last_line = cursor_pos_end_of_last_line - char_count_per_line / 2;
        const int cursor_pos_middle = vars.str.length() / 2;

        auto SetCursorPosition = [&stb](int cursor) { stb.cursor = cursor; stb.has_preferred_x = 0; };

        // Do all the test twice: with no trailing \n, and with.
        for (int i = 0; i < 2; i++)
        {
            bool has_trailing_line_feed = (i == 1);
            if (has_trailing_line_feed)
            {
                SetCursorPosition(cursor_pos_end_of_last_line);
                ctx->KeyCharsAppend("\n");
            }
            int eof = vars.str.length();

            // Begin of File
            SetCursorPosition(0); ctx->KeyPress(ImGuiKey_UpArrow);
            IM_CHECK_EQ(stb.cursor, 0);
            SetCursorPosition(0); ctx->KeyPress(ImGuiKey_LeftArrow);
            IM_CHECK_EQ(stb.cursor, 0);
            SetCursorPosition(0); ctx->KeyPress(ImGuiKey_DownArrow);
            IM_CHECK_EQ(stb.cursor, char_count_per_line);
            SetCursorPosition(0); ctx->KeyPress(ImGuiKey_RightArrow);
            IM_CHECK_EQ(stb.cursor, 1);

            // End of first line
            SetCursorPosition(cursor_pos_end_of_first_line); ctx->KeyPress(ImGuiKey_UpArrow);
            IM_CHECK_EQ(stb.cursor, cursor_pos_end_of_first_line);
            SetCursorPosition(cursor_pos_end_of_first_line); ctx->KeyPress(ImGuiKey_LeftArrow);
            IM_CHECK_EQ(stb.cursor, cursor_pos_end_of_first_line - 1);
            SetCursorPosition(cursor_pos_end_of_first_line); ctx->KeyPress(ImGuiKey_DownArrow);
            IM_CHECK_EQ(stb.cursor, cursor_pos_end_of_first_line + char_count_per_line);
            SetCursorPosition(cursor_pos_end_of_first_line); ctx->KeyPress(ImGuiKey_RightArrow);
            IM_CHECK_EQ(stb.cursor, cursor_pos_end_of_first_line + 1);

            // Begin of last line
            SetCursorPosition(cursor_pos_begin_of_last_line); ctx->KeyPress(ImGuiKey_UpArrow);
            IM_CHECK_EQ(stb.cursor, cursor_pos_begin_of_last_line - char_count_per_line);
            SetCursorPosition(cursor_pos_begin_of_last_line); ctx->KeyPress(ImGuiKey_LeftArrow);
            IM_CHECK_EQ(stb.cursor, cursor_pos_begin_of_last_line - 1);
            SetCursorPosition(cursor_pos_begin_of_last_line); ctx->KeyPress(ImGuiKey_DownArrow);
            IM_CHECK_EQ(stb.cursor, has_trailing_line_feed ? eof : cursor_pos_begin_of_last_line);
            SetCursorPosition(cursor_pos_begin_of_last_line); ctx->KeyPress(ImGuiKey_RightArrow);
            IM_CHECK_EQ(stb.cursor, cursor_pos_begin_of_last_line + 1);

            // End of last line
            SetCursorPosition(cursor_pos_end_of_last_line); ctx->KeyPress(ImGuiKey_UpArrow);
            //IM_CHECK_EQ(stb.cursor, cursor_pos_end_of_last_line - char_count_per_line); // FIXME: This one is broken even on master
            SetCursorPosition(cursor_pos_end_of_last_line); ctx->KeyPress(ImGuiKey_LeftArrow);
            IM_CHECK_EQ(stb.cursor, cursor_pos_end_of_last_line - 1);
            SetCursorPosition(cursor_pos_end_of_last_line); ctx->KeyPress(ImGuiKey_DownArrow);
            IM_CHECK_EQ(stb.cursor, has_trailing_line_feed ? eof : cursor_pos_end_of_last_line);
            SetCursorPosition(cursor_pos_end_of_last_line); ctx->KeyPress(ImGuiKey_RightArrow);
            IM_CHECK_EQ(stb.cursor, cursor_pos_end_of_last_line + (has_trailing_line_feed ? 1 : 0));

            // In the middle of the content
            SetCursorPosition(cursor_pos_middle); ctx->KeyPress(ImGuiKey_UpArrow);
            IM_CHECK_EQ(stb.cursor, cursor_pos_middle - char_count_per_line);
            SetCursorPosition(cursor_pos_middle); ctx->KeyPress(ImGuiKey_LeftArrow);
            IM_CHECK_EQ(stb.cursor, cursor_pos_middle - 1);
            SetCursorPosition(cursor_pos_middle); ctx->KeyPress(ImGuiKey_DownArrow);
            IM_CHECK_EQ(stb.cursor, cursor_pos_middle + char_count_per_line);
            SetCursorPosition(cursor_pos_middle); ctx->KeyPress(ImGuiKey_RightArrow);
            IM_CHECK_EQ(stb.cursor, cursor_pos_middle + 1);

            // Home/End to go to beginning/end of the line
            SetCursorPosition(cursor_pos_middle); ctx->KeyPress(ImGuiKey_Home);
            IM_CHECK_EQ(stb.cursor, ((vars.LineCount / 2) - 1) * char_count_per_line);
            SetCursorPosition(cursor_pos_middle); ctx->KeyPress(ImGuiKey_End);
            IM_CHECK_EQ(stb.cursor, (vars.LineCount / 2) * char_count_per_line - 1);

            // Ctrl+Home/End to go to beginning/end of the text
            SetCursorPosition(cursor_pos_middle); ctx->KeyPress(ImGuiKey_Home, ImGuiModFlags_Ctrl);
            IM_CHECK_EQ(stb.cursor, 0);
            SetCursorPosition(cursor_pos_middle); ctx->KeyPress(ImGuiKey_End, ImGuiModFlags_Ctrl);
            IM_CHECK_EQ(stb.cursor, cursor_pos_end_of_last_line + (has_trailing_line_feed ? 1 : 0));

            // PageUp/PageDown
            SetCursorPosition(cursor_pos_begin_of_first_line); ctx->KeyPress(ImGuiKey_PageDown);
            IM_CHECK_EQ(stb.cursor, cursor_pos_begin_of_first_line + char_count_per_line * page_size);
            ctx->KeyPress(ImGuiKey_PageUp);
            IM_CHECK_EQ(stb.cursor, cursor_pos_begin_of_first_line);

            SetCursorPosition(cursor_pos_middle_of_first_line);
            ctx->KeyPress(ImGuiKey_PageDown);
            IM_CHECK_EQ(stb.cursor, cursor_pos_middle_of_first_line + char_count_per_line * page_size);
            ctx->KeyPress(ImGuiKey_PageDown);
            IM_CHECK_EQ(stb.cursor, cursor_pos_middle_of_first_line + char_count_per_line * page_size * 2);
            ctx->KeyPress(ImGuiKey_PageDown);
            IM_CHECK_EQ(stb.cursor, has_trailing_line_feed ? eof : eof - (char_count_per_line / 2) + 1);

            // We started PageDown from the middle of a line, so even if we're at the end (with X = 0),
            // PageUp should bring us one page up to the middle of the line
            int cursor_pos_begin_current_line = (stb.cursor / char_count_per_line) * char_count_per_line; // Round up cursor position to decimal only
            ctx->KeyPress(ImGuiKey_PageUp);
            IM_CHECK_EQ(stb.cursor, cursor_pos_begin_current_line - (page_size * char_count_per_line) + (char_count_per_line / 2));
                //eof - (char_count_per_line * page_size) + (char_count_per_line / 2) + (has_trailing_line_feed ? 0 : 1));
        }

        // Cursor positioning after new line. Broken line indexing may produce incorrect results in such case.
        ctx->KeyCharsReplaceEnter("foo");
        IM_CHECK_EQ(stb.cursor, 4);

        // Verify that cursor placement does not leak spaces in password field. (#4155)
        ctx->ItemClick("Password");
        ctx->KeyCharsAppendEnter("Totally not Password123");
        ctx->ItemDoubleClick("Password");
        IM_CHECK_EQ(stb.select_start, 0);
        IM_CHECK_EQ(stb.select_end, 23);
        IM_CHECK_EQ(stb.cursor, 23);
        ctx->KeyPress(ImGuiKey_LeftArrow, ImGuiModFlags_Shortcut);
        IM_CHECK_EQ(stb.cursor, 0);
        ctx->KeyPress(ImGuiKey_RightArrow, ImGuiModFlags_Shortcut);
        IM_CHECK_EQ(stb.cursor, 23);
    };

    // ## Test that scrolling preserve cursor and selection
    t = IM_REGISTER_TEST(e, "widgets", "widgets_inputtext_scrolling");
    t->SetVarsDataType<InputTextCursorVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        InputTextCursorVars& vars = ctx->GetVars<InputTextCursorVars>();

        float height = 5 * ImGui::GetFontSize();
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::InputTextMultiline("Field", &vars.str, ImVec2(300, height), ImGuiInputTextFlags_EnterReturnsTrue);
        if (ImGuiInputTextState* state = ImGui::GetInputTextState(ctx->GetID("/Test Window/Field")))
            ImGui::Text("Stb Cursor: %d", state->Stb.cursor);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ctx->SetRef("Test Window");
        ctx->ItemInput("Field");
        for (int n = 0; n < 10; n++)
            ctx->KeyCharsAppendEnter(Str16f("Line %d", n).c_str());
        ctx->KeyModDown(ImGuiModFlags_Shift);
        ctx->KeyPress(ImGuiKey_UpArrow);
        ctx->KeyModUp(ImGuiModFlags_Shift);

        ImGuiID child_id = ctx->GetChildWindowID("/Test Window", "Field");
        ImGuiWindow* child_window = ctx->GetWindowByRef(child_id);
        IM_CHECK(child_window != NULL);
        const int selection_len = (int)strlen("Line 9\n");

        for (int n = 0; n < 3; n++)
        {
            ImGuiInputTextState* state = ImGui::GetInputTextState(ctx->GetID("Field"));
            IM_CHECK(state != NULL);
            IM_CHECK(state->HasSelection());
            IM_CHECK(ImAbs(state->Stb.select_end - state->Stb.select_start) == selection_len);
            IM_CHECK(state->Stb.select_end == state->Stb.cursor);
            IM_CHECK(state->Stb.cursor == state->CurLenW - selection_len);
            if (n == 1)
                ctx->ScrollToBottom(child_id);
            else
                ctx->ScrollToTop(child_id);
        }

        ImGuiInputTextState* state = ImGui::GetInputTextState(ctx->GetID("Field"));
        IM_CHECK(state != NULL);
        IM_CHECK(child_window->Scroll.y == 0.0f);
        ctx->KeyPress(ImGuiKey_RightArrow);
        IM_CHECK_EQ(state->Stb.cursor, state->CurLenW);
        IM_CHECK_EQ(child_window->Scroll.y, child_window->ScrollMax.y);
    };

    // ## Test named filters
    t = IM_REGISTER_TEST(e, "widgets", "widgets_inputtext_filters");
    struct InputTextFilterVars { Str64 Default; Str64 Decimal; Str64 Scientific;  Str64 Hex; Str64 Uppercase; Str64 NoBlank; Str64 Custom; };
    t->SetVarsDataType<InputTextFilterVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        InputTextFilterVars& vars = ctx->GetVars<InputTextFilterVars>();
        struct TextFilters
        {
            // Return 0 (pass) if the character is 'i' or 'm' or 'g' or 'u' or 'i'
            static int FilterImGuiLetters(ImGuiInputTextCallbackData* data)
            {
                if (data->EventChar < 256 && strchr("imgui", (char)data->EventChar))
                    return 0;
                return 1;
            }
        };

        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        ImGui::InputText("default", &vars.Default);
        ImGui::InputText("decimal", &vars.Decimal, ImGuiInputTextFlags_CharsDecimal);
        ImGui::InputText("scientific", &vars.Scientific, ImGuiInputTextFlags_CharsScientific);
        ImGui::InputText("hexadecimal", &vars.Hex, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase);
        ImGui::InputText("uppercase", &vars.Uppercase, ImGuiInputTextFlags_CharsUppercase);
        ImGui::InputText("no blank", &vars.NoBlank, ImGuiInputTextFlags_CharsNoBlank);
        ImGui::InputText("\"imgui\" letters", &vars.Custom, ImGuiInputTextFlags_CallbackCharFilter, TextFilters::FilterImGuiLetters);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        InputTextFilterVars& vars = ctx->GetVars<InputTextFilterVars>();
        const char* input_text = "Some fancy Input Text in 0.., 1.., 2.., 3!";
        ctx->SetRef("Test Window");
        ctx->ItemClick("default");
        ctx->KeyCharsAppendEnter(input_text);
        IM_CHECK_STR_EQ(vars.Default.c_str(), input_text);

        ctx->ItemClick("decimal");
        ctx->KeyCharsAppendEnter(input_text);
        IM_CHECK_STR_EQ(vars.Decimal.c_str(), "0..1..2..3");

        ctx->ItemClick("scientific");
        ctx->KeyCharsAppendEnter(input_text);
        IM_CHECK_STR_EQ(vars.Scientific.c_str(), "ee0..1..2..3");

        ctx->ItemClick("hexadecimal");
        ctx->KeyCharsAppendEnter(input_text);
        IM_CHECK_STR_EQ(vars.Hex.c_str(), "EFACE0123");

        ctx->ItemClick("uppercase");
        ctx->KeyCharsAppendEnter(input_text);
        IM_CHECK_STR_EQ(vars.Uppercase.c_str(), "SOME FANCY INPUT TEXT IN 0.., 1.., 2.., 3!");

        ctx->ItemClick("no blank");
        ctx->KeyCharsAppendEnter(input_text);
        IM_CHECK_STR_EQ(vars.NoBlank.c_str(), "SomefancyInputTextin0..,1..,2..,3!");

        ctx->ItemClick("\"imgui\" letters");
        ctx->KeyCharsAppendEnter(input_text);
        IM_CHECK_STR_EQ(vars.Custom.c_str(), "mui");
    };

    // ## Test completion and history
    t = IM_REGISTER_TEST(e, "widgets", "widgets_inputtext_callback_misc");
    struct InputTextCallbackHistoryVars { Str CompletionBuffer; Str HistoryBuffer; Str EditBuffer; int EditCount = 0; };
    t->SetVarsDataType<InputTextCallbackHistoryVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        struct Funcs
        {
            static int MyCallback(ImGuiInputTextCallbackData* data)
            {
                if (data->EventFlag == ImGuiInputTextFlags_CallbackCompletion)
                {
                    data->InsertChars(data->CursorPos, "..");
                }
                else if (data->EventFlag == ImGuiInputTextFlags_CallbackHistory)
                {
                    if (data->EventKey == ImGuiKey_UpArrow)
                    {
                        data->DeleteChars(0, data->BufTextLen);
                        data->InsertChars(0, "Pressed Up!");
                        data->SelectAll();
                    }
                    else if (data->EventKey == ImGuiKey_DownArrow)
                    {
                        data->DeleteChars(0, data->BufTextLen);
                        data->InsertChars(0, "Pressed Down!");
                        data->SelectAll();
                    }
                }
                else if (data->EventFlag == ImGuiInputTextFlags_CallbackEdit)
                {
                    // Toggle casing of first character
                    char c = data->Buf[0];
                    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) data->Buf[0] ^= 32;
                    data->BufDirty = true;

                    // Increment a counter
                    int* p_edit_count = (int*)data->UserData;
                    *p_edit_count = *p_edit_count + 1;
                }
                return 0;
            }
        };

        InputTextCallbackHistoryVars& vars = ctx->GetVars<InputTextCallbackHistoryVars>();
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::InputText("Completion", &vars.CompletionBuffer, ImGuiInputTextFlags_CallbackCompletion, Funcs::MyCallback);
        ImGui::InputText("History", &vars.HistoryBuffer, ImGuiInputTextFlags_CallbackHistory, Funcs::MyCallback);
        ImGui::InputText("Edit", &vars.EditBuffer, ImGuiInputTextFlags_CallbackEdit, Funcs::MyCallback, (void*)&vars.EditCount);
        ImGui::SameLine(); ImGui::Text("(%d)", vars.EditCount);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        InputTextCallbackHistoryVars& vars = ctx->GetVars<InputTextCallbackHistoryVars>();

        ctx->SetRef("Test Window");
        ctx->ItemClick("Completion");
        ctx->KeyCharsAppend("Hello World");
        IM_CHECK_STR_EQ(vars.CompletionBuffer.c_str(), "Hello World");
        ctx->KeyPress(ImGuiKey_Tab);
        IM_CHECK_STR_EQ(vars.CompletionBuffer.c_str(), "Hello World..");

        // FIXME: Not testing History callback :)
        ctx->ItemClick("History");
        ctx->KeyCharsAppend("ABCDEF");
        ctx->KeyPress(ImGuiKey_Z, ImGuiModFlags_Shortcut);
        IM_CHECK_STR_EQ(vars.HistoryBuffer.c_str(), "ABCDE");
        ctx->KeyPress(ImGuiKey_Z, ImGuiModFlags_Shortcut);
        ctx->KeyPress(ImGuiKey_Z, ImGuiModFlags_Shortcut);
        IM_CHECK_STR_EQ(vars.HistoryBuffer.c_str(), "ABC");
        ctx->KeyPress(ImGuiKey_Y, ImGuiModFlags_Shortcut);
        IM_CHECK_STR_EQ(vars.HistoryBuffer.c_str(), "ABCD");
        ctx->KeyPress(ImGuiKey_UpArrow);
        IM_CHECK_STR_EQ(vars.HistoryBuffer.c_str(), "Pressed Up!");
        ctx->KeyPress(ImGuiKey_DownArrow);
        IM_CHECK_STR_EQ(vars.HistoryBuffer.c_str(), "Pressed Down!");

        ctx->ItemClick("Edit");
        IM_CHECK_STR_EQ(vars.EditBuffer.c_str(), "");
        IM_CHECK_EQ(vars.EditCount, 0);
        ctx->KeyCharsAppend("h");
        IM_CHECK_STR_EQ(vars.EditBuffer.c_str(), "H");
        IM_CHECK_EQ(vars.EditCount, 1);
        ctx->KeyCharsAppend("e");
        IM_CHECK_STR_EQ(vars.EditBuffer.c_str(), "he");
        IM_CHECK_EQ(vars.EditCount, 2);
        ctx->KeyCharsAppend("llo");
        IM_CHECK_STR_EQ(vars.EditBuffer.c_str(), "Hello");
        IM_CHECK_LE(vars.EditCount, (ctx->EngineIO->ConfigRunSpeed == ImGuiTestRunSpeed_Fast) ? 3 : 5); // If running fast, "llo" will be considered as one edit only
    };

    // ## Test character replacement in callback (inspired by https://github.com/ocornut/imgui/pull/3587)
    t = IM_REGISTER_TEST(e, "widgets", "widgets_inputtext_callback_replace");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        auto callback = [](ImGuiInputTextCallbackData* data)
        {
            if (data->CursorPos >= 3 && strcmp(data->Buf + data->CursorPos - 3, "abc") == 0)
            {
                data->DeleteChars(data->CursorPos - 3, 3);
                data->InsertChars(data->CursorPos, "\xE5\xA5\xBD"); // HAO
                data->SelectionStart = data->CursorPos - 3;
                data->SelectionEnd = data->CursorPos;
                return 1;
            }
            return 0;
        };
        ImGui::InputText("Hello", vars.Str1, IM_ARRAYSIZE(vars.Str1), ImGuiInputTextFlags_CallbackAlways, callback);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ctx->SetRef("Test Window");
        ctx->ItemInput("Hello");
        ImGuiInputTextState* state = &ctx->UiContext->InputTextState;
        IM_CHECK(state && state->ID == ctx->GetID("Hello"));
        ctx->KeyCharsAppend("ab");
        IM_CHECK(state->CurLenA == 2);
        IM_CHECK(state->CurLenW == 2);
        IM_CHECK(strcmp(state->TextA.Data, "ab") == 0);
        IM_CHECK(state->Stb.cursor == 2);
        ctx->KeyCharsAppend("c");
        IM_CHECK(state->CurLenA == 3);
        IM_CHECK(state->CurLenW == 1);
        IM_CHECK(strcmp(state->TextA.Data, "\xE5\xA5\xBD") == 0);
        IM_CHECK(state->TextW.Data[0] == 0x597D);
        IM_CHECK(state->TextW.Data[1] == 0);
        IM_CHECK(state->Stb.cursor == 1);
        IM_CHECK(state->Stb.select_start == 0 && state->Stb.select_end == 1);
    };

    // ## Test resize callback (#3009, #2006, #1443, #1008)
    t = IM_REGISTER_TEST(e, "widgets", "widgets_inputtext_callback_resize");
    t->SetVarsDataType<StrVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        StrVars& vars = ctx->GetVars<StrVars>();
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        if (ImGui::InputText("Field1", &vars.str, ImGuiInputTextFlags_EnterReturnsTrue))
        {
            IM_CHECK_EQ(vars.str.capacity(), 4 + 5 + 1);
            IM_CHECK_STR_EQ(vars.str.c_str(), "abcdhello");
        }
        Str str_local_unsaved = "abcd";
        if (ImGui::InputText("Field2", &str_local_unsaved, ImGuiInputTextFlags_EnterReturnsTrue))
        {
            IM_CHECK_EQ(str_local_unsaved.capacity(), 4 + 5 + 1);
            IM_CHECK_STR_EQ(str_local_unsaved.c_str(), "abcdhello");
        }
        ImGui::End();

    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        StrVars& vars = ctx->GetVars<StrVars>();
        vars.str.set("abcd");
        IM_CHECK_EQ(vars.str.capacity(), 4 + 1);
        ctx->SetRef("Test Window");
        ctx->ItemInput("Field1");
        ctx->KeyCharsAppendEnter("hello");
        ctx->ItemInput("Field2");
        ctx->KeyCharsAppendEnter("hello");
    };

    // ## Test resize callback being triggered from within callback (#4784)
    t = IM_REGISTER_TEST(e, "widgets", "widgets_inputtext_callback_resize2");
    t->SetVarsDataType<StrVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        auto callback = [](ImGuiInputTextCallbackData* data)
        {
            Str* str = (Str*)data->UserData;
            if (data->EventFlag == ImGuiInputTextFlags_CallbackHistory)
            {
                data->InsertChars(0, "foo");
            }
            else if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
            {
                IM_ASSERT(data->Buf == str->c_str());
                str->reserve(data->BufTextLen + 1);
                data->Buf = (char*)str->c_str();
            }
            return 0;
        };

        StrVars& vars = ctx->GetVars<StrVars>();
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        ImGui::InputText("Field1", vars.str.c_str(), vars.str.capacity(), ImGuiInputTextFlags_CallbackHistory | ImGuiInputTextFlags_CallbackResize, callback, (void*)&vars.str);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        StrVars& vars = ctx->GetVars<StrVars>();
        ctx->SetRef("Test Window");
        ctx->ItemClick("Field1");
        ctx->KeyPress(ImGuiKey_DownArrow);
        IM_CHECK_STR_EQ(vars.str.c_str(), "foo");
    };

    // ## Test for Nav interference
    t = IM_REGISTER_TEST(e, "widgets", "widgets_inputtext_nav");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        ImVec2 sz(50, 0);
        ImGui::Button("UL", sz); ImGui::SameLine();
        ImGui::Button("U",  sz); ImGui::SameLine();
        ImGui::Button("UR", sz);
        ImGui::Button("L",  sz); ImGui::SameLine();
        ImGui::SetNextItemWidth(sz.x);
        ImGui::InputText("##Field", vars.Str1, IM_ARRAYSIZE(vars.Str1), ImGuiInputTextFlags_AllowTabInput);
        ImGui::SameLine();
        ImGui::Button("R", sz);
        ImGui::Button("DL", sz); ImGui::SameLine();
        ImGui::Button("D", sz); ImGui::SameLine();
        ImGui::Button("DR", sz);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ctx->SetRef("Test Window");
        ctx->ItemClick("##Field");
        ctx->KeyPress(ImGuiKey_LeftArrow);
        IM_CHECK_EQ(ctx->UiContext->NavId, ctx->GetID("##Field"));
        ctx->KeyPress(ImGuiKey_RightArrow);
        IM_CHECK_EQ(ctx->UiContext->NavId, ctx->GetID("##Field"));
        ctx->KeyPress(ImGuiKey_UpArrow);
        IM_CHECK_EQ(ctx->UiContext->NavId, ctx->GetID("U"));
        ctx->KeyPress(ImGuiKey_DownArrow);
        ctx->KeyPress(ImGuiKey_DownArrow);
        IM_CHECK_EQ(ctx->UiContext->NavId, ctx->GetID("D"));
    };

    // ## Test InputText widget with user a buffer on stack, reading/writing past end of buffer.
    t = IM_REGISTER_TEST(e, "widgets", "widgets_inputtext_temp_buffer");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        char buf[10] = "foo\0xxxxx";
        if (ImGui::InputText("Field", buf, 7, ImGuiInputTextFlags_EnterReturnsTrue))
        {
            IM_CHECK_STR_EQ(buf, "foobar");
            IM_CHECK_STR_EQ(buf + 7, "xx"); // Buffer padding not overwritten
        }
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ctx->SetRef("Test Window");
        ctx->ItemClick("Field");
        ctx->KeyCharsAppendEnter("barrr");
    };

    // ## Test InputText clipboard functions.
    t = IM_REGISTER_TEST(e, "widgets", "widgets_inputtext_clipboard");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        ImGui::InputText("Field", vars.Str1, IM_ARRAYSIZE(vars.Str1), ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        char* text = vars.Str1;
        const char* clipboard_text = ImGui::GetClipboardText();
        IM_CHECK_STR_EQ(clipboard_text, "");
        ctx->SetRef("Test Window");

        for (int variant = 0; variant < 2; variant++)
        {
            // State reset.
            ImGui::ClearActiveID();
            strcpy(text, "Hello, world!");

            // Copying without selection.
            ctx->ItemClick("Field");
            if (variant == 0)
                ctx->KeyPress(ImGuiKey_C, ImGuiModFlags_Shortcut);
            else
                ctx->KeyPress(ImGuiKey_Insert, ImGuiModFlags_Ctrl);
            clipboard_text = ImGui::GetClipboardText();
            IM_CHECK_STR_EQ(clipboard_text, "Hello, world!");

            // Copying with selection.
            ctx->ItemClick("Field");
            ctx->KeyPress(ImGuiKey_Home);
            for (int i = 0; i < 5; i++) // Seek to and select first word
                ctx->KeyPress(ImGuiKey_RightArrow, ImGuiModFlags_Shift);
            if (variant == 0)
                ctx->KeyPress(ImGuiKey_C, ImGuiModFlags_Shortcut);
            else
                ctx->KeyPress(ImGuiKey_Insert, ImGuiModFlags_Ctrl);
            clipboard_text = ImGui::GetClipboardText();
            IM_CHECK_STR_EQ(clipboard_text, "Hello");

            // Cut a selection.
            ctx->ItemClick("Field");
            ctx->KeyPress(ImGuiKey_Home);
            for (int i = 0; i < 5; i++) // Seek to and select first word
                ctx->KeyPress(ImGuiKey_RightArrow, ImGuiModFlags_Shift);
            if (variant == 0)
                ctx->KeyPress(ImGuiKey_X, ImGuiModFlags_Shortcut);
            else
                ctx->KeyPress(ImGuiKey_Delete, ImGuiModFlags_Shift);
            clipboard_text = ImGui::GetClipboardText();
            IM_CHECK_STR_EQ(clipboard_text, "Hello");
            IM_CHECK_STR_EQ(text, ", world!");

            // Paste over selection.
            ctx->ItemClick("Field");
            ImGui::SetClipboardText("h\xc9\x99\xcb\x88l\xc5\x8d");  // həˈlō
            ctx->KeyPress(ImGuiKey_Home);
            if (variant == 0)
                ctx->KeyPress(ImGuiKey_V, ImGuiModFlags_Shortcut);
            else
                ctx->KeyPress(ImGuiKey_Insert, ImGuiModFlags_Shift);
            IM_CHECK_STR_EQ(text, "h\xc9\x99\xcb\x88l\xc5\x8d, world!");
        }
    };

    // ## Test for IsItemHovered() on InputTextMultiline() (#1370, #3851)
    t = IM_REGISTER_TEST(e, "widgets", "widgets_inputtext_hover");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        ImGui::InputTextMultiline("Field", vars.Str1, IM_ARRAYSIZE(vars.Str1));
#if IMGUI_VERSION_NUM >= 18511
        IM_CHECK_EQ(ImGui::GetItemID(), ImGui::GetID("Field"));
#endif
        vars.Bool1 = ImGui::IsItemHovered();
        ImGui::Text("hovered: %d", vars.Bool1);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ctx->SetRef("Test Window");
        ctx->MouseMove("Field");
        IM_CHECK(vars.Bool1 == true);
    };

    // ## Test for IsItemXXX() calls on InputTextMultiline()
#if IMGUI_VERSION_NUM >= 18512
    t = IM_REGISTER_TEST(e, "widgets", "widgets_inputtext_multiline_status");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        ImGui::InputTextMultiline("Field", vars.Str1, IM_ARRAYSIZE(vars.Str1));
        //IM_CHECK_EQ(ImGui::GetItemID(), ImGui::GetID("Field"));
        vars.Status.QuerySet();
        ImGui::Text("IsItemActive: %d", vars.Status.Active);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiContext& g = *ctx->UiContext;
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ctx->SetRef("Test Window");
        IM_CHECK(vars.Bool1 == false);
        ctx->ItemClick("Field");
        ImGuiID input_id = ctx->GetID("Field");
        IM_CHECK_EQ(g.ActiveId, input_id);
        IM_CHECK(vars.Status.Active == 1);
        ctx->KeyCharsReplace("1\n2\n3\n4\n5\n6\n\7\n8\n9\n10\n11\n12\n13\n14\n15\n");
        ImGuiWindow* window = ImGui::FindWindowByID(ctx->GetChildWindowID("", "Field"));
        IM_CHECK(window != NULL);
        ImGuiID scrollbar_id = ImGui::GetWindowScrollbarID(window, ImGuiAxis_Y);
        ctx->MouseMove(scrollbar_id);
        ctx->MouseDown(ImGuiMouseButton_Left);
        IM_CHECK_EQ(g.ActiveId, scrollbar_id);
        IM_CHECK(vars.Status.Active == 1);
        ctx->MouseUp(ImGuiMouseButton_Left);
        IM_CHECK_EQ(g.ActiveId, input_id);
        IM_CHECK(vars.Status.Active == 1);
    };
#endif

    // ## Test handling of Tab/Enter/Space keys events also emitting text events. (#2467, #1336)
    // Backends are inconsistent in behavior: some don't send a text event for Tab and Enter (but still send it for Space)
#if IMGUI_VERSION_NUM >= 18711
    t = IM_REGISTER_TEST(e, "widgets", "widgets_inputtext_special_key_chars");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        ImGui::InputTextMultiline("Field", ctx->GenericVars.Str1, IM_ARRAYSIZE(ctx->GenericVars.Str1), ImVec2(), ImGuiInputTextFlags_AllowTabInput);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiContext& g = *ctx->UiContext;

        char* field_text = ctx->GenericVars.Str1;
        ctx->SetRef("Test Window");
        ctx->ItemClick("Field");
        ctx->RunFlags |= ImGuiTestRunFlags_EnableRawInputs; // Disable TestEngine submitting inputs events
        ctx->Yield();

        g.IO.AddKeyEvent(ImGuiKey_Tab, true);
        g.IO.AddInputCharacter('\t');
        ctx->Yield();
        g.IO.AddKeyEvent(ImGuiKey_Tab, false);
        ctx->Yield();
        IM_CHECK_STR_EQ(field_text, "\t");
        g.IO.AddKeyEvent(ImGuiKey_Backspace, true);
        ctx->Yield();
        g.IO.AddKeyEvent(ImGuiKey_Backspace, false);
        ctx->Yield();

        g.IO.AddKeyEvent(ImGuiKey_Enter, true);
        g.IO.AddInputCharacter('\r');
        ctx->Yield();
        g.IO.AddKeyEvent(ImGuiKey_Enter, false);
        ctx->Yield();
        IM_CHECK_STR_EQ(field_text, "\n");
        g.IO.AddKeyEvent(ImGuiKey_Backspace, true);
        ctx->Yield();
        g.IO.AddKeyEvent(ImGuiKey_Backspace, false);
        ctx->Yield();

        g.IO.AddKeyEvent(ImGuiKey_Space, true);
        g.IO.AddInputCharacter(' ');
        ctx->Yield();
        g.IO.AddKeyEvent(ImGuiKey_Space, false);
        ctx->Yield();
        IM_CHECK_STR_EQ(field_text, " ");
    };
#endif

    // ## Test inheritance of ItemFlags
    t = IM_REGISTER_TEST(e, "widgets", "widgets_item_flags_stack");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiContext& g = *ctx->UiContext;
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        IM_CHECK_EQ(g.CurrentItemFlags, 0);

        ImGui::BeginChild("child1", ImVec2(100, 100));
        IM_CHECK_EQ(g.CurrentItemFlags, 0);
        ImGui::Button("enable button in child1");
        ImGui::EndChild();

        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::Button("disabled button in parent");

        IM_CHECK_EQ(g.CurrentItemFlags, ImGuiItemFlags_Disabled);
        ImGui::BeginChild("child1"); // Append
        IM_CHECK_EQ(g.CurrentItemFlags, ImGuiItemFlags_Disabled);
        ImGui::Button("disabled button in child1");
        ImGui::EndChild();

        ImGui::BeginChild("child2", ImVec2(100, 100)); // New
        IM_CHECK_EQ(g.CurrentItemFlags, ImGuiItemFlags_Disabled);
        ImGui::Button("disabled button in child2");
        ImGui::EndChild();

        ImGui::PopItemFlag();

#if IMGUI_VERSION_NUM >= 18207 && IMGUI_VERSION_NUM < 18415
        IM_CHECK_EQ(g.CurrentItemFlags, 0);
        ImGui::Begin("Test Window 2", NULL, ImGuiWindowFlags_NoSavedSettings);
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::End();
        IM_CHECK_EQ(g.CurrentItemFlags, ImGuiItemFlags_Disabled);
        ImGui::PopItemFlag();
        IM_CHECK_EQ(g.CurrentItemFlags, 0);
#endif

        ImGui::End();
    };

    // ## Test ColorEdit4() and IsItemDeactivatedXXX() functions
    // ## Test that IsItemActivated() doesn't trigger when clicking the color button to open picker
    t = IM_REGISTER_TEST(e, "widgets", "widgets_status_coloredit");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        bool ret = ImGui::ColorEdit4("Field", &vars.Color1.x, ImGuiColorEditFlags_None);
        vars.Status.QueryInc(ret);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        // Accumulate return values over several frames/action into each bool
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGuiTestGenericItemStatus& status = vars.Status;

        // Testing activation flag being set
        ctx->SetRef("Test Window");
        ctx->ItemClick("Field/##ColorButton");
        IM_CHECK(status.Ret == 0 && status.Activated == 1 && status.Deactivated == 1 && status.DeactivatedAfterEdit == 0 && status.Edited == 0);
        status.Clear();

        ctx->KeyPress(ImGuiKey_Escape);
        IM_CHECK(status.Ret == 0 && status.Activated == 0 && status.Deactivated == 0 && status.DeactivatedAfterEdit == 0 && status.Edited == 0);
        status.Clear();
    };

    // ## Test InputText() and IsItemDeactivatedXXX() functions (mentioned in #2215)
    t = IM_REGISTER_TEST(e, "widgets", "widgets_status_inputtext");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        bool ret = ImGui::InputText("Field", vars.Str1, IM_ARRAYSIZE(vars.Str1));
        vars.Status.QueryInc(ret);
        ImGui::InputText("Sibling", vars.Str2, IM_ARRAYSIZE(vars.Str2));
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        // Accumulate return values over several frames/action into each bool
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGuiTestGenericItemStatus& status = vars.Status;

        // Testing activation flag being set
        ctx->SetRef("Test Window");
        ctx->ItemClick("Field");
        IM_CHECK(status.Ret == 0 && status.Activated == 1 && status.Deactivated == 0 && status.DeactivatedAfterEdit == 0 && status.Edited == 0);
        status.Clear();

        // Testing deactivated flag being set when canceling with Escape
        ctx->KeyPress(ImGuiKey_Escape);
        IM_CHECK(status.Ret == 0 && status.Activated == 0 && status.Deactivated == 1 && status.DeactivatedAfterEdit == 0 && status.Edited == 0);
        status.Clear();

        // Testing validation with Return after editing
        ctx->ItemClick("Field");
        IM_CHECK(!status.Ret && status.Activated && !status.Deactivated && !status.DeactivatedAfterEdit && status.Edited == 0);
        status.Clear();
        ctx->KeyCharsAppend("Hello");
        IM_CHECK(status.Ret && !status.Activated && !status.Deactivated && !status.DeactivatedAfterEdit && status.Edited >= 1);
        status.Clear();
        ctx->KeyPress(ImGuiKey_Enter);
        IM_CHECK(!status.Ret && !status.Activated && status.Deactivated && status.DeactivatedAfterEdit && status.Edited == 0);
        status.Clear();

        // Testing validation with Tab after editing
        ctx->ItemClick("Field");
        ctx->KeyCharsAppend(" World");
        IM_CHECK(status.Ret && status.Activated && !status.Deactivated && !status.DeactivatedAfterEdit && status.Edited >= 1);
        status.Clear();
        ctx->KeyPress(ImGuiKey_Tab);
        IM_CHECK(!status.Ret && !status.Activated && status.Deactivated && status.DeactivatedAfterEdit && status.Edited == 0);
        status.Clear();
    };

    // ## Test the IsItemDeactivatedXXX() functions (e.g. #2550, #1875)
    t = IM_REGISTER_TEST(e, "widgets", "widgets_status_multicomponent");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        bool ret = ImGui::InputFloat4("Field", &vars.FloatArray[0]);
        vars.Status.QueryInc(ret);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        // Accumulate return values over several frames/action into each bool
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGuiTestGenericItemStatus& status = vars.Status;

        // FIXME-TESTS: Better helper to build ids out of various type of data
        ctx->SetRef("Test Window");
        int n;
        n = 0; ImGuiID field_0 = ImHashData(&n, sizeof(n), ctx->GetID("Field"));
        n = 1; ImGuiID field_1 = ImHashData(&n, sizeof(n), ctx->GetID("Field"));
        //n = 2; ImGuiID field_2 = ImHashData(&n, sizeof(n), ctx->GetID("Field"));

        // Testing activation/deactivation flags
        ctx->ItemClick(field_0);
        IM_CHECK(status.Ret == 0 && status.Activated == 1 && status.Deactivated == 0 && status.DeactivatedAfterEdit == 0);
        status.Clear();
        ctx->KeyPress(ImGuiKey_Enter);
        IM_CHECK(status.Ret == 0 && status.Activated == 0 && status.Deactivated == 1 && status.DeactivatedAfterEdit == 0);
        status.Clear();

        // Testing validation with Return after editing
        ctx->ItemClick(field_0);
        status.Clear();
        ctx->KeyCharsAppend("123");
        IM_CHECK(status.Ret >= 1 && status.Activated == 0 && status.Deactivated == 0);
        status.Clear();
        ctx->KeyPress(ImGuiKey_Enter);
        IM_CHECK(status.Ret == 0 && status.Activated == 0 && status.Deactivated == 1);
        status.Clear();

        // Testing validation with Tab after editing
        ctx->ItemClick(field_0);
        ctx->KeyCharsAppend("456");
        status.Clear();
        ctx->KeyPress(ImGuiKey_Tab);
        IM_CHECK(status.Ret == 0 && status.Activated == 1 && status.Deactivated == 1 && status.DeactivatedAfterEdit == 1);

        // Testing Edited flag on all components
        ctx->ItemClick(field_1); // FIXME-TESTS: Should not be necessary!
        ctx->ItemClick(field_0);
        ctx->KeyCharsAppend("111");
        IM_CHECK(status.Edited >= 1);
        ctx->KeyPress(ImGuiKey_Tab);
        status.Clear();
        ctx->KeyCharsAppend("222");
        IM_CHECK(status.Edited >= 1);
        ctx->KeyPress(ImGuiKey_Tab);
        status.Clear();
        ctx->KeyCharsAppend("333");
        IM_CHECK(status.Edited >= 1);
    };

    // ## Test the IsItemEdited() function when input vs output format are not matching
    t = IM_REGISTER_TEST(e, "widgets", "widgets_status_inputfloat_format_mismatch");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        bool ret = ImGui::InputFloat("Field", &vars.Float1);
        vars.Status.QueryInc(ret);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGuiTestGenericItemStatus& status = vars.Status;

        // Input "1" which will be formatted as "1.000", make sure we don't report IsItemEdited() multiple times!
        ctx->SetRef("Test Window");
        ctx->ItemClick("Field");
        ctx->KeyCharsAppend("1");
        IM_CHECK(status.Ret == 1 && status.Edited == 1 && status.Activated == 1 && status.Deactivated == 0 && status.DeactivatedAfterEdit == 0);
        ctx->Yield();
        ctx->Yield();
        IM_CHECK(status.Edited == 1);
    };

    // ## Check input of InputScalar().
    t = IM_REGISTER_TEST(e, "widgets", "widgets_inputscalar_input");
    struct InputScalarStepVars { int Int = 0; float Float = 0.0f; double Double = 0.0; };
    t->SetVarsDataType<InputScalarStepVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        InputScalarStepVars& vars = ctx->GetVars<InputScalarStepVars>();
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
#if IMGUI_VERSION_NUM >= 18717 // GetID() used to cause interferences by setting g.ActiveIdIsAlive (#5181)
        ImGui::GetID("Int");
        ImGui::GetID("Float");
        ImGui::GetID("Double");
#endif
        ImGui::InputInt("Int", &vars.Int, 2);
        ImGui::InputFloat("Float", &vars.Float, 1.5f);
        ImGui::InputDouble("Double", &vars.Double, 1.5);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        InputScalarStepVars& vars = ctx->GetVars<InputScalarStepVars>();
        ctx->SetRef("Test Window");
        ctx->ItemInputValue("Int", 42);
        IM_CHECK(vars.Int == 42);
        ctx->ItemInputValue("Float", 42.1f);
        IM_CHECK(vars.Float == 42.1f);
        ctx->ItemInputValue("Double", "123.456789");
        IM_CHECK(vars.Double == 123.456789);
    };

    // ## Check step buttons of InputScalar().
    t = IM_REGISTER_TEST(e, "widgets", "widgets_inputscalar_step");
    t->SetVarsDataType<InputScalarStepVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        InputScalarStepVars& vars = ctx->GetVars<InputScalarStepVars>();
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::InputInt("Int", &vars.Int, 2);
        ImGui::InputFloat("Float", &vars.Float, 1.5f);
        ImGui::InputDouble("Double", &vars.Double, 1.5);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        InputScalarStepVars& vars = ctx->GetVars<InputScalarStepVars>();
        ctx->SetRef("Test Window");

        ctx->ItemClick("Int/+");
        IM_CHECK(vars.Int == +2);
        ctx->ItemClick("Int/-");
        ctx->ItemClick("Int/-");
        IM_CHECK(vars.Int == -2);

        ctx->ItemClick("Float/+");
        IM_CHECK(vars.Float == +1.5f);
        ctx->ItemClick("Float/-");
        ctx->ItemClick("Float/-");
        IM_CHECK(vars.Float == -1.5f);

        ctx->ItemClick("Double/+");
        IM_CHECK(vars.Double == +1.5);
        ctx->ItemClick("Double/-");
        ctx->ItemClick("Double/-");
        IM_CHECK(vars.Double == -1.5);
    };

    // ## Test ImGui::InputScalar() handling overflow for different data types
    t = IM_REGISTER_TEST(e, "widgets", "widgets_inputscalar_overflow");
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        {
            ImS8 one = 1;
            ImS8 value = 2;
            ImGui::DataTypeApplyOp(ImGuiDataType_S8, '+', &value, &value, &one);
            IM_CHECK(value == 3);
            value = SCHAR_MAX;
            ImGui::DataTypeApplyOp(ImGuiDataType_S8, '+', &value, &value, &one);
            IM_CHECK(value == SCHAR_MAX);
            value = SCHAR_MIN;
            ImGui::DataTypeApplyOp(ImGuiDataType_S8, '-', &value, &value, &one);
            IM_CHECK(value == SCHAR_MIN);
        }
        {
            ImU8 one = 1;
            ImU8 value = 2;
            ImGui::DataTypeApplyOp(ImGuiDataType_U8, '+', &value, &value, &one);
            IM_CHECK(value == 3);
            value = UCHAR_MAX;
            ImGui::DataTypeApplyOp(ImGuiDataType_U8, '+', &value, &value, &one);
            IM_CHECK(value == UCHAR_MAX);
            value = 0;
            ImGui::DataTypeApplyOp(ImGuiDataType_U8, '-', &value, &value, &one);
            IM_CHECK(value == 0);
        }
        {
            ImS16 one = 1;
            ImS16 value = 2;
            ImGui::DataTypeApplyOp(ImGuiDataType_S16, '+', &value, &value, &one);
            IM_CHECK(value == 3);
            value = SHRT_MAX;
            ImGui::DataTypeApplyOp(ImGuiDataType_S16, '+', &value, &value, &one);
            IM_CHECK(value == SHRT_MAX);
            value = SHRT_MIN;
            ImGui::DataTypeApplyOp(ImGuiDataType_S16, '-', &value, &value, &one);
            IM_CHECK(value == SHRT_MIN);
        }
        {
            ImU16 one = 1;
            ImU16 value = 2;
            ImGui::DataTypeApplyOp(ImGuiDataType_U16, '+', &value, &value, &one);
            IM_CHECK(value == 3);
            value = USHRT_MAX;
            ImGui::DataTypeApplyOp(ImGuiDataType_U16, '+', &value, &value, &one);
            IM_CHECK(value == USHRT_MAX);
            value = 0;
            ImGui::DataTypeApplyOp(ImGuiDataType_U16, '-', &value, &value, &one);
            IM_CHECK(value == 0);
        }
        {
            ImS32 one = 1;
            ImS32 value = 2;
            ImGui::DataTypeApplyOp(ImGuiDataType_S32, '+', &value, &value, &one);
            IM_CHECK(value == 3);
            value = INT_MAX;
            ImGui::DataTypeApplyOp(ImGuiDataType_S32, '+', &value, &value, &one);
            IM_CHECK(value == INT_MAX);
            value = INT_MIN;
            ImGui::DataTypeApplyOp(ImGuiDataType_S32, '-', &value, &value, &one);
            IM_CHECK(value == INT_MIN);
        }
        {
            ImU32 one = 1;
            ImU32 value = 2;
            ImGui::DataTypeApplyOp(ImGuiDataType_U32, '+', &value, &value, &one);
            IM_CHECK(value == 3);
            value = UINT_MAX;
            ImGui::DataTypeApplyOp(ImGuiDataType_U32, '+', &value, &value, &one);
            IM_CHECK(value == UINT_MAX);
            value = 0;
            ImGui::DataTypeApplyOp(ImGuiDataType_U32, '-', &value, &value, &one);
            IM_CHECK(value == 0);
        }
        {
            ImS64 one = 1;
            ImS64 value = 2;
            ImGui::DataTypeApplyOp(ImGuiDataType_S64, '+', &value, &value, &one);
            IM_CHECK(value == 3);
            value = LLONG_MAX;
            ImGui::DataTypeApplyOp(ImGuiDataType_S64, '+', &value, &value, &one);
            IM_CHECK(value == LLONG_MAX);
            value = LLONG_MIN;
            ImGui::DataTypeApplyOp(ImGuiDataType_S64, '-', &value, &value, &one);
            IM_CHECK(value == LLONG_MIN);
        }
        {
            ImU64 one = 1;
            ImU64 value = 2;
            ImGui::DataTypeApplyOp(ImGuiDataType_U64, '+', &value, &value, &one);
            IM_CHECK(value == 3);
            value = ULLONG_MAX;
            ImGui::DataTypeApplyOp(ImGuiDataType_U64, '+', &value, &value, &one);
            IM_CHECK(value == ULLONG_MAX);
            value = 0;
            ImGui::DataTypeApplyOp(ImGuiDataType_U64, '-', &value, &value, &one);
            IM_CHECK(value == 0);
        }
    };

    // ## Test ImGui::InputScalar() formatting edgecases.
    t = IM_REGISTER_TEST(e, "widgets", "widgets_inputscalar_formats");
    struct InputScalarFormatsVars { Str30 Format; int Int = 0; Str30 Text; int CaptureWidget = -1; };
    t->SetVarsDataType<InputScalarFormatsVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiContext& g = *ctx->UiContext;
        InputScalarFormatsVars& vars = ctx->GetVars<InputScalarFormatsVars>();
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        if (ctx->IsGuiFuncOnly())
            ImGui::InputText("Format", &vars.Format);

        for (int i = 0; i < 3; i++)
        {
            ImGui::LogToBuffer();
            if (i == 0)
                ImGui::DragInt("Drag", &vars.Int, 1.0f, 0, 0, vars.Format.c_str());
            else if (i == 1)
                ImGui::InputScalar("Input", ImGuiDataType_S32, &vars.Int, NULL, NULL, vars.Format.c_str());
            else if (i == 2)
                ImGui::SliderInt("Slider", &vars.Int, -10000, +10000, vars.Format.c_str());
            if (vars.CaptureWidget == i)
            {
                vars.Text = g.LogBuffer.c_str();
                vars.CaptureWidget = -1;
            }
            ImGui::LogFinish();
        }

        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        InputScalarFormatsVars& vars = ctx->GetVars<InputScalarFormatsVars>();
        ctx->SetRef("Test Window");
        auto capture_widget_value = [ctx, &vars](int i)
        {
            vars.Text.clear();
            vars.CaptureWidget = i;
            ctx->Yield();
            if (char* curly = strstr(vars.Text.c_str(), "}"))
                curly[1] = 0;   // Strip widget name
            return vars.Text.c_str();
        };

        const char* widget_names[] = { "Drag", "Input", "Slider" };
        for (int i = 0; i < 3; i++)
        {
            const char* widget_name = widget_names[i];
            ctx->LogDebug("Widget: %s", widget_name);

#if IMGUI_VERSION_NUM >= 18714
            vars.Int = 0;
            vars.Format = "%03X";
            ctx->ItemInput(widget_name);
            ctx->KeyCharsReplaceEnter("7b");
            IM_CHECK_STR_EQ(capture_widget_value(i), "{ 07B }");
            IM_CHECK_EQ(vars.Int, 0x7B);
#endif

#if IMGUI_VERSION_NUM >= 18715
            vars.Int = 0;
            ctx->ItemInput(widget_name);
            ctx->KeyCharsReplaceEnter("FF");
            IM_CHECK_STR_EQ(capture_widget_value(i), "{ 0FF }");
            IM_CHECK_EQ(vars.Int, 0xFF);

            vars.Int = 0;
            ctx->ItemInput(widget_name);
            ctx->KeyCharsReplaceEnter("1FF");
            IM_CHECK_STR_EQ(capture_widget_value(i), "{ 1FF }");
            IM_CHECK_EQ(vars.Int, 0x1FF);
#endif

            vars.Int = 0;
            vars.Format = "%03d";
            ctx->ItemInput(widget_name);
            ctx->KeyCharsReplaceEnter("1234");
            IM_CHECK_STR_EQ(capture_widget_value(i), "{ 1234 }");
            IM_CHECK_EQ(vars.Int, 1234);

#if IMGUI_VERSION_NUM >= 18715
            vars.Int = 0;
            vars.Format = "%03d";
            ctx->ItemInput(widget_name);
            ctx->KeyCharsReplaceEnter("1235");
            IM_CHECK_STR_EQ(capture_widget_value(i), "{ 1235 }");
            IM_CHECK_EQ(vars.Int, 1235);

            vars.Int = 0;
            vars.Format = "%.03d";
            ctx->ItemInput(widget_name);
            ctx->KeyCharsReplaceEnter("1236");
            IM_CHECK_STR_EQ(capture_widget_value(i), "{ 1236 }");
            IM_CHECK_EQ(vars.Int, 1236);
#endif
        }
    };

    // ## Test that tight tab bar does not create extra drawcalls
    t = IM_REGISTER_TEST(e, "widgets", "widgets_tabbar_drawcalls");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        if (ImGui::BeginTabBar("Tab Drawcalls"))
        {
            for (int i = 0; i < 20; i++)
                if (ImGui::BeginTabItem(Str30f("Tab %d", i).c_str()))
                    ImGui::EndTabItem();
            ImGui::EndTabBar();
        }
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiWindow* window = ImGui::FindWindowByName("Test Window");
        ctx->WindowResize("Test Window", ImVec2(300, 300));
        int draw_calls = window->DrawList->CmdBuffer.Size;
        ctx->WindowResize("Test Window", ImVec2(1, 1));
        IM_CHECK(draw_calls == window->DrawList->CmdBuffer.Size);
    };

    // ## Test order of tabs in a tab bar
    t = IM_REGISTER_TEST(e, "widgets", "widgets_tabbar_order");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        for (int n = 0; n < 4; n++)
            ImGui::Checkbox(Str30f("Open Tab %d", n).c_str(), &vars.BoolArray[n]);
        if (ImGui::BeginTabBar("TabBar", ImGuiTabBarFlags_Reorderable))
        {
            for (int n = 0; n < 4; n++)
                if (vars.BoolArray[n] && ImGui::BeginTabItem(Str30f("Tab %d", n).c_str(), &vars.BoolArray[n]))
                    ImGui::EndTabItem();
            ImGui::EndTabBar();
        }
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiContext& g = *ctx->UiContext;
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ctx->SetRef("Test Window");
        ImGuiTabBar* tab_bar = g.TabBars.GetOrAddByKey(ctx->GetID("TabBar")); // FIXME-TESTS: Helper function?
        IM_CHECK(tab_bar != NULL);
        IM_CHECK(tab_bar->Tabs.Size == 0);

        vars.BoolArray[0] = vars.BoolArray[1] = vars.BoolArray[2] = true;
        ctx->Yield();
        ctx->Yield(); // Important: so tab layout are correct for TabClose()
        IM_CHECK(tab_bar->Tabs.Size == 3);
        IM_CHECK_STR_EQ(tab_bar->GetTabName(&tab_bar->Tabs[0]), "Tab 0");
        IM_CHECK_STR_EQ(tab_bar->GetTabName(&tab_bar->Tabs[1]), "Tab 1");
        IM_CHECK_STR_EQ(tab_bar->GetTabName(&tab_bar->Tabs[2]), "Tab 2");

        ctx->TabClose("TabBar/Tab 1");
        ctx->Yield();
        ctx->Yield();
        IM_CHECK(vars.BoolArray[1] == false);
        IM_CHECK(tab_bar->Tabs.Size == 2);
        IM_CHECK_STR_EQ(tab_bar->GetTabName(&tab_bar->Tabs[0]), "Tab 0");
        IM_CHECK_STR_EQ(tab_bar->GetTabName(&tab_bar->Tabs[1]), "Tab 2");

        vars.BoolArray[1] = true;
        ctx->Yield();
        IM_CHECK(tab_bar->Tabs.Size == 3);
        IM_CHECK_STR_EQ(tab_bar->GetTabName(&tab_bar->Tabs[0]), "Tab 0");
        IM_CHECK_STR_EQ(tab_bar->GetTabName(&tab_bar->Tabs[1]), "Tab 2");
        IM_CHECK_STR_EQ(tab_bar->GetTabName(&tab_bar->Tabs[2]), "Tab 1");
    };

    // ## (Attempt to) Test that tab bar declares its unclipped size.
    t = IM_REGISTER_TEST(e, "widgets", "widgets_tabbar_size");
    struct TabBarVars { bool HasCloseButton = false; float ExpectedWidth = 0.0f; };
    t->SetVarsDataType<TabBarVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiContext& g = *ctx->UiContext;
        auto& vars = ctx->GetVars<TabBarVars>();

        // FIXME-TESTS: Ideally we would test variation of with/without ImGuiTabBarFlags_TabListPopupButton, but we'd need to know its width...
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        ImGui::Checkbox("HasCloseButton", &vars.HasCloseButton);
        if (ImGui::BeginTabBar("TabBar"))
        {
            vars.ExpectedWidth = 0.0f;
            for (int i = 0; i < 3; i++)
            {
                Str30f label("TabItem %d", i);
                bool tab_open = true;
                if (ImGui::BeginTabItem(label.c_str(), vars.HasCloseButton ? &tab_open : NULL))
                    ImGui::EndTabItem();
                if (i > 0)
                    vars.ExpectedWidth += g.Style.ItemInnerSpacing.x;
                vars.ExpectedWidth += ImGui::TabItemCalcSize(label.c_str(), vars.HasCloseButton).x;
            }
            ImGui::EndTabBar();
        }
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiWindow* window = ImGui::FindWindowByName("Test Window");
        auto& vars = ctx->GetVars<TabBarVars>();

        vars.HasCloseButton = false;
        ctx->Yield();
        IM_CHECK_EQ(window->DC.CursorStartPos.x + vars.ExpectedWidth, window->DC.IdealMaxPos.x);

        vars.HasCloseButton = true;
        ctx->Yield(); // BeginTabBar() will submit old size --> TabBarLayout update sizes
        ctx->Yield(); // BeginTabBar() will submit new size
        IM_CHECK_EQ(window->DC.CursorStartPos.x + vars.ExpectedWidth, window->DC.IdealMaxPos.x);
    };

    // ## Test TabItemButton behavior
    t = IM_REGISTER_TEST(e, "widgets", "widgets_tabbar_tabitem_button");
    struct TabBarButtonVars { int LastClickedButton = -1; };
    t->SetVarsDataType<TabBarButtonVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        TabBarButtonVars& vars = ctx->GetVars<TabBarButtonVars>();
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings);
        if (ImGui::BeginTabBar("TabBar"))
        {
            if (ImGui::TabItemButton("1", ImGuiTabItemFlags_None))        { vars.LastClickedButton = 1; }
            if (ImGui::TabItemButton("0", ImGuiTabItemFlags_None))        { vars.LastClickedButton = 0; }
            if (ImGui::BeginTabItem("Tab", NULL, ImGuiTabItemFlags_None)) { ImGui::EndTabItem(); }
            ImGui::EndTabBar();
        }
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        TabBarButtonVars& vars = ctx->GetVars<TabBarButtonVars>();
        ctx->SetRef("Test Window/TabBar");

        IM_CHECK_EQ(vars.LastClickedButton, -1);
        ctx->ItemClick("1");
        IM_CHECK_EQ(vars.LastClickedButton, 1);
        ctx->ItemClick("Tab");
        IM_CHECK_EQ(vars.LastClickedButton, 1);
        ctx->MouseMove("0");
        ctx->MouseDown();
        IM_CHECK_EQ(vars.LastClickedButton, 1);
        ctx->MouseUp();
        IM_CHECK_EQ(vars.LastClickedButton, 0);
    };

    // ## Test that tab items respects their Leading/Trailing position
    t = IM_REGISTER_TEST(e, "widgets", "widgets_tabbar_tabitem_leading_trailing");
    struct TabBarLeadingTrailingVars { bool WindowAutoResize = true; ImGuiTabBarFlags TabBarFlags = 0; ImGuiTabBar* TabBar = NULL; };
    t->SetVarsDataType<TabBarLeadingTrailingVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiContext& g = *ctx->UiContext;
        TabBarLeadingTrailingVars& vars = ctx->GetVars<TabBarLeadingTrailingVars>();
        ImGui::Begin("Test Window", NULL, (vars.WindowAutoResize ? ImGuiWindowFlags_AlwaysAutoResize : 0) | ImGuiWindowFlags_NoSavedSettings);
        ImGui::Checkbox("ImGuiWindowFlags_AlwaysAutoResize", &vars.WindowAutoResize);
        if (ImGui::BeginTabBar("TabBar", vars.TabBarFlags))
        {
            vars.TabBar = g.CurrentTabBar;
            if (ImGui::BeginTabItem("Trailing", NULL, ImGuiTabItemFlags_Trailing)) { ImGui::EndTabItem(); } // Intentionally submit Trailing tab early and Leading tabs at the end
            if (ImGui::BeginTabItem("Tab 0", NULL, ImGuiTabItemFlags_None))        { ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Tab 1", NULL, ImGuiTabItemFlags_None))        { ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Tab 2", NULL, ImGuiTabItemFlags_None))        { ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Leading", NULL, ImGuiTabItemFlags_Leading))   { ImGui::EndTabItem(); }
            ImGui::EndTabBar();
        }
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiContext& g = *ctx->UiContext;
        auto& vars = ctx->GetVars<TabBarLeadingTrailingVars>();

        vars.TabBarFlags = ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_FittingPolicyResizeDown;
        ctx->Yield();

        ctx->SetRef("Test Window/TabBar");
        const char* tabs[] = { "Leading", "Tab 0", "Tab 1", "Tab 2", "Trailing" };

        // Check that tabs relative order matches what we expect (which is not the same as submission order above)
        float offset_x = -FLT_MAX;
        for (int i = 0; i < IM_ARRAYSIZE(tabs); ++i)
        {
            ctx->MouseMove(tabs[i]);
            IM_CHECK_GT(g.IO.MousePos.x, offset_x);
            offset_x = g.IO.MousePos.x;
        }

        // Test that "Leading" cannot be reordered over "Tab 0" and vice-versa
        ctx->ItemDragAndDrop("Leading", "Tab 0");
        IM_CHECK_EQ(vars.TabBar->Tabs[0].ID, ctx->GetID("Leading"));
        IM_CHECK_EQ(vars.TabBar->Tabs[1].ID, ctx->GetID("Tab 0"));
        ctx->ItemDragAndDrop("Tab 0", "Leading");
        IM_CHECK_EQ(vars.TabBar->Tabs[0].ID, ctx->GetID("Leading"));
        IM_CHECK_EQ(vars.TabBar->Tabs[1].ID, ctx->GetID("Tab 0"));

        // Test that "Trailing" cannot be reordered over "Tab 2" and vice-versa
        ctx->ItemDragAndDrop("Trailing", "Tab 2");
        IM_CHECK_EQ(vars.TabBar->Tabs[4].ID, ctx->GetID("Trailing"));
        IM_CHECK_EQ(vars.TabBar->Tabs[3].ID, ctx->GetID("Tab 2"));
        ctx->ItemDragAndDrop("Tab 2", "Trailing");
        IM_CHECK_EQ(vars.TabBar->Tabs[4].ID, ctx->GetID("Trailing"));
        IM_CHECK_EQ(vars.TabBar->Tabs[3].ID, ctx->GetID("Tab 2"));

        // Resize down
        vars.WindowAutoResize = false;
        ImGuiWindow* window = ctx->GetWindowByRef("/Test Window");
        ctx->WindowResize("Test Window", ImVec2(window->Size.x * 0.3f, window->Size.y));
        for (int i = 0; i < 2; ++i)
        {
            vars.TabBarFlags = ImGuiTabBarFlags_Reorderable | (i == 0 ? ImGuiTabBarFlags_FittingPolicyResizeDown : ImGuiTabBarFlags_FittingPolicyScroll);
            ctx->Yield();
            IM_CHECK_GT(ctx->ItemInfo("Leading")->RectClipped.GetWidth(), 1.0f);
            IM_CHECK_EQ(ctx->ItemInfo("Tab 0")->RectClipped.GetWidth(), 0.0f);
            IM_CHECK_EQ(ctx->ItemInfo("Tab 1")->RectClipped.GetWidth(), 0.0f);
            IM_CHECK_EQ(ctx->ItemInfo("Tab 2")->RectClipped.GetWidth(), 0.0f);
            IM_CHECK_GT(ctx->ItemInfo("Trailing")->RectClipped.GetWidth(), 1.0f);
        }
    };

    // ## Test reordering tabs (and ImGuiTabItemFlags_NoReorder flag)
    t = IM_REGISTER_TEST(e, "widgets", "widgets_tabbar_reorder");
    struct TabBarReorderVars { ImGuiTabBarFlags Flags = ImGuiTabBarFlags_Reorderable; ImGuiTabBar* TabBar = NULL; };
    t->SetVarsDataType<TabBarReorderVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiContext& g = *ctx->UiContext;
        auto& vars = ctx->GetVars<TabBarReorderVars>();
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings);
        if (ImGui::BeginTabBar("TabBar", vars.Flags))
        {
            vars.TabBar = g.CurrentTabBar;
            if (ImGui::BeginTabItem("Tab 0", NULL, ImGuiTabItemFlags_None))      { ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Tab 1", NULL, ImGuiTabItemFlags_None))      { ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Tab 2", NULL, ImGuiTabItemFlags_NoReorder)) { ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Tab 3", NULL, ImGuiTabItemFlags_None))      { ImGui::EndTabItem(); }
            ImGui::EndTabBar();
        }
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        TabBarReorderVars& vars = ctx->GetVars<TabBarReorderVars>();

        // Reset reorderable flags to ensure tabs are in their submission order
        vars.Flags = ImGuiTabBarFlags_None;
        ctx->Yield();
        vars.Flags = ImGuiTabBarFlags_Reorderable;
        ctx->Yield();

        ctx->SetRef("Test Window/TabBar");

        ctx->ItemDragAndDrop("Tab 0", "Tab 1");
        IM_CHECK_EQ(vars.TabBar->Tabs[0].ID, ctx->GetID("Tab 1"));
        IM_CHECK_EQ(vars.TabBar->Tabs[1].ID, ctx->GetID("Tab 0"));

        ctx->ItemDragAndDrop("Tab 0", "Tab 1");
        IM_CHECK_EQ(vars.TabBar->Tabs[0].ID, ctx->GetID("Tab 0"));
        IM_CHECK_EQ(vars.TabBar->Tabs[1].ID, ctx->GetID("Tab 1"));

        ctx->ItemDragAndDrop("Tab 0", "Tab 2"); // Tab 2 has NoReorder flag
        ctx->ItemDragAndDrop("Tab 0", "Tab 3"); // Tab 2 has NoReorder flag
        ctx->ItemDragAndDrop("Tab 3", "Tab 2"); // Tab 2 has NoReorder flag
        IM_CHECK_EQ(vars.TabBar->Tabs[1].ID, ctx->GetID("Tab 0"));
        IM_CHECK_EQ(vars.TabBar->Tabs[2].ID, ctx->GetID("Tab 2"));
        IM_CHECK_EQ(vars.TabBar->Tabs[3].ID, ctx->GetID("Tab 3"));
    };

    // ## Test nested/recursing Tab Bars (Bug #2371)
    t = IM_REGISTER_TEST(e, "widgets", "widgets_tabbar_nested");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        if (ImGui::BeginTabBar("TabBar 0"))
        {
            if (ImGui::BeginTabItem("TabItem"))
            {
                // If we have many tab bars here, it will invalidate pointers from pooled tab bars
                for (int i = 0; i < 10; i++)
                    if (ImGui::BeginTabBar(Str30f("Inner TabBar %d", i).c_str()))
                    {
                        if (ImGui::BeginTabItem("Inner TabItem"))
                            ImGui::EndTabItem();
                        ImGui::EndTabBar();
                    }
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        ImGui::End();
    };

    // ## Test BeginTabBar() append
    struct TabBarMultipleSubmissionVars { bool AppendToTabBar = true; ImVec2 CursorAfterActiveTab; ImVec2 CursorAfterFirstBeginTabBar; ImVec2 CursorAfterFirstWidget; ImVec2 CursorAfterSecondBeginTabBar; ImVec2 CursorAfterSecondWidget; ImVec2 CursorAfterSecondEndTabBar; };
    t = IM_REGISTER_TEST(e, "widgets", "widgets_tabbar_append");
    t->SetVarsDataType<TabBarMultipleSubmissionVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiContext& g = *ctx->UiContext;
        auto& vars = ctx->GetVars<TabBarMultipleSubmissionVars>();

        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Checkbox("AppendToTabBar", &vars.AppendToTabBar);
        if (ImGui::BeginTabBar("TabBar"))
        {
            vars.CursorAfterFirstBeginTabBar = g.CurrentWindow->DC.CursorPos;
            if (ImGui::BeginTabItem("Tab 0"))
            {
                ImGui::Text("Tab 0");
                ImGui::EndTabItem();
                vars.CursorAfterActiveTab = g.CurrentWindow->DC.CursorPos;
            }
            if (ImGui::BeginTabItem("Tab 1"))
            {
                for (int i = 0; i < 3; i++)
                    ImGui::Text("Tab 1 Line %d", i);
                ImGui::EndTabItem();
                vars.CursorAfterActiveTab = g.CurrentWindow->DC.CursorPos;
            }
            if (ImGui::BeginTabItem("Tab 2"))
            {
                ImGui::EndTabItem();
                vars.CursorAfterActiveTab = g.CurrentWindow->DC.CursorPos;
            }
            ImGui::EndTabBar();
        }
        ImGui::Text("After first TabBar submission");
        vars.CursorAfterFirstWidget = g.CurrentWindow->DC.CursorPos;

        if (vars.AppendToTabBar && ImGui::BeginTabBar("TabBar"))
        {
            vars.CursorAfterSecondBeginTabBar = g.CurrentWindow->DC.CursorPos;
            if (ImGui::BeginTabItem("Tab A"))
            {
                ImGui::Text("I'm tab A");
                ImGui::EndTabItem();
                vars.CursorAfterActiveTab = g.CurrentWindow->DC.CursorPos;
            }
            ImGui::EndTabBar();
            vars.CursorAfterSecondEndTabBar = g.CurrentWindow->DC.CursorPos;
        }
        ImGui::Text("After second TabBar submission");
        vars.CursorAfterSecondWidget = g.CurrentWindow->DC.CursorPos;

        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiContext& g = *ctx->UiContext;
        auto& vars = ctx->GetVars<TabBarMultipleSubmissionVars>();

        ctx->SetRef("Test Window/TabBar");

        const float line_height = g.FontSize + g.Style.ItemSpacing.y;
        for (bool append_to_tab_bar : { false, true })
        {
            vars.AppendToTabBar = append_to_tab_bar;
            ctx->Yield();

            for (const char* tab_name : { "Tab 0", "Tab 1", "Tab A" })
            {
                if (!append_to_tab_bar && strcmp(tab_name, "Tab A") == 0)
                    continue;

                ctx->ItemClick(tab_name);
                ctx->Yield();

                float active_tab_height = line_height;
                if (strcmp(tab_name, "Tab 1") == 0)
                    active_tab_height *= 3;

                IM_CHECK(vars.CursorAfterActiveTab.y == vars.CursorAfterFirstBeginTabBar.y + active_tab_height);
                IM_CHECK(vars.CursorAfterFirstWidget.y == vars.CursorAfterActiveTab.y + line_height);
                if (append_to_tab_bar)
                {
                    IM_CHECK(vars.CursorAfterSecondBeginTabBar.y == vars.CursorAfterFirstBeginTabBar.y);
                    IM_CHECK(vars.CursorAfterSecondEndTabBar.y == vars.CursorAfterFirstWidget.y);
                }
                IM_CHECK(vars.CursorAfterSecondWidget.y == vars.CursorAfterFirstWidget.y + line_height);
            }
        }
    };


#ifdef IMGUI_HAS_DOCK
    // ## Test Dockspace within a TabItem
    t = IM_REGISTER_TEST(e, "widgets", "widgets_tabbar_dockspace");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        if (ImGui::BeginTabBar("TabBar"))
        {
            if (ImGui::BeginTabItem("TabItem"))
            {
                ImGui::DockSpace(ImGui::GetID("Hello"), ImVec2(0, 0));
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        ImGui::End();
    };
#endif

    // ## Test SetSelected on first frame of a TabItem
    t = IM_REGISTER_TEST(e, "widgets", "widgets_tabbar_tabitem_setselected");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        if (ImGui::BeginTabBar("tab_bar"))
        {
            if (ImGui::BeginTabItem("TabItem 0"))
            {
                ImGui::TextUnformatted("First tab content");
                ImGui::EndTabItem();
            }

            if (ctx->FrameCount >= 0)
            {
                bool tab_item_visible = ImGui::BeginTabItem("TabItem 1", NULL, ctx->FrameCount == 0 ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None);
                if (tab_item_visible)
                {
                    ImGui::TextUnformatted("Second tab content");
                    ImGui::EndTabItem();
                }
                if (ctx->FrameCount > 0)
                    IM_CHECK(tab_item_visible);
            }
            ImGui::EndTabBar();
        }
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx) { ctx->Yield(); };

    // ## Test SetNextItemWidth() with TabItem(). (#5262)
#if IMGUI_VERSION_NUM >= 18732
    t = IM_REGISTER_TEST(e, "widgets", "widgets_tabbar_tabitem_setwidth");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        auto& vars = ctx->GenericVars;
        const ImGuiTabItemFlags test_flags[] = { ImGuiTabItemFlags_None, ImGuiTabItemFlags_Trailing, ImGuiTabItemFlags_Leading, ImGuiTabItemFlags_None };
        vars.Count = IM_ARRAYSIZE(test_flags);
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        if (ImGui::BeginTabBar("TabBar"))
        {
            for (int i = 0; i < IM_ARRAYSIZE(test_flags); i++)
            {
                ImGui::SetNextItemWidth(30.0f + i * 10.0f);
                if (ImGui::BeginTabItem(Str16f("Tab %d", i).c_str(), NULL, test_flags[i]))
                {
                    ImGui::TextUnformatted(Str16f("Content %d", i).c_str());
                    ImGui::EndTabItem();
                }
            }
            ImGui::EndTabBar();
        }
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        auto& vars = ctx->GenericVars;
        ctx->SetRef("Test Window");
        for (int i = 0; i < vars.Count; i++)
        {
            ImGuiTestItemInfo* item_info = ctx->ItemInfo(Str30f("TabBar/Tab %d", i).c_str());
            IM_CHECK(item_info != NULL);
            IM_CHECK_EQ(item_info->RectFull.GetWidth(), 30.0f + i * 10.0f);
        }
    };
#endif

    // ## Tests: Coverage: TabBar: TabBarTabListPopupButton() and TabBarScrollingButtons()
    t = IM_REGISTER_TEST(e, "widgets", "widgets_tabbar_popup_scrolling_button");
    struct TabBarCoveragePopupScrolling { int TabCount = 9; int Selected = -1; };
    t->SetVarsDataType<TabBarCoveragePopupScrolling>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        auto& vars = ctx->GetVars<TabBarCoveragePopupScrolling>();
        ImGui::SetNextWindowSize(ImVec2(200, 100));
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        if (ImGui::BeginTabBar("TabBar", ImGuiTabBarFlags_TabListPopupButton | ImGuiTabBarFlags_FittingPolicyScroll))
        {
            for (int i = 0; i < vars.TabCount; i++)
                if (ImGui::BeginTabItem(Str16f{ "Tab %d", i }.c_str(), NULL)) { vars.Selected = i; ImGui::EndTabItem(); }
            ImGui::EndTabBar();
        }
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        auto& vars = ctx->GetVars<TabBarCoveragePopupScrolling>();
        ctx->SetRef("Test Window");
        ctx->ItemClick("TabBar/Tab 0"); // Ensure first tab is selected

        for (int i = 0; i < vars.TabCount; i++)
        {
            ctx->ItemClick("TabBar/##<");
            ctx->Yield();
            IM_CHECK_EQ(vars.Selected, i == 0 ? 0 : i - 1);

            ctx->ItemClick("TabBar/##v");
            ctx->ItemClick(Str64f("/##Combo_00/Tab %d", i).c_str());
            ctx->Yield();
            IM_CHECK_EQ(vars.Selected, i);

            ctx->ItemClick("TabBar/##>");
            ctx->Yield();
            IM_CHECK_EQ(vars.Selected, i == vars.TabCount - 1 ? vars.TabCount - 1 : i + 1);
        }

        // Click on all even tab
        for (int i = 0; i < vars.TabCount / 2; i++)
        {
            const int even_i = i * 2;
            ctx->ItemClick(Str64f("TabBar/Tab %d", even_i).c_str());
            IM_CHECK_EQ(vars.Selected, even_i);
        }

        // Click on all odd tab
        for (int i = 0; i < vars.TabCount / 2; i++)
        {
            const int odd_i = i * 2 + 1;
            ctx->ItemClick(Str64f("TabBar/Tab %d", odd_i).c_str());
            IM_CHECK_EQ(vars.Selected, odd_i);
        }
    };

    // ## Test various TreeNode flags
    t = IM_REGISTER_TEST(e, "widgets", "widgets_treenode_behaviors");
    struct TreeNodeTestVars { bool Reset = true, IsOpen = false, IsMultiSelect = false; int ToggleCount = 0; int DragSourceCount = 0;  ImGuiTreeNodeFlags Flags = 0; };
    t->SetVarsDataType<TreeNodeTestVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiCond_Always);
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);

        ImGui::ColorButton("Color", ImVec4(1.0f, 1.0f, 0.0f, 1.0f), ImGuiColorEditFlags_NoTooltip); // To test open-on-drag-hold

        TreeNodeTestVars& vars = ctx->GetVars<TreeNodeTestVars>();
        if (vars.Reset)
        {
            ImGui::GetStateStorage()->SetInt(ImGui::GetID("AAA"), 0);
            vars.ToggleCount = vars.DragSourceCount = 0;
        }
        vars.Reset = false;
        ImGui::Text("Flags: 0x%08X, MultiSelect: %d", vars.Flags, vars.IsMultiSelect);

#ifdef IMGUI_HAS_MULTI_SELECT
        if (vars.IsMultiSelect)
        {
            ImGui::BeginMultiSelect(ImGuiMultiSelectFlags_None, NULL, false); // Placeholder, won't interact properly
            ImGui::SetNextItemSelectionData(NULL);
        }
#endif

        vars.IsOpen = ImGui::TreeNodeEx("AAA", vars.Flags);
        if (ImGui::IsItemToggledOpen())
            vars.ToggleCount++;
        if (ImGui::BeginDragDropSource())
        {
            vars.DragSourceCount++;
            ImGui::SetDragDropPayload("_TREENODE", NULL, 0);
            ImGui::Text("Drag Source Tooltip");
            ImGui::EndDragDropSource();
        }
        if (vars.IsOpen)
        {
            ImGui::Text("Contents");
            ImGui::TreePop();
        }

#ifdef IMGUI_HAS_MULTI_SELECT
        if (vars.IsMultiSelect)
            ImGui::EndMultiSelect();
#endif

        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        TreeNodeTestVars& vars = ctx->GetVars<TreeNodeTestVars>();
        ctx->SetRef("Test Window");

#ifdef IMGUI_HAS_MULTI_SELECT
        int loop_count = 2;
#else
        int loop_count = 1;
#endif

        for (int loop_n = 0; loop_n < loop_count; loop_n++)
        {
            vars.IsMultiSelect = (loop_n == 1);

            {
                ctx->LogInfo("## ImGuiTreeNodeFlags_None, IsMultiSelect=%d", vars.IsMultiSelect);
                vars.Reset = true;
                vars.Flags = ImGuiTreeNodeFlags_None;
                ctx->Yield();
                IM_CHECK(vars.IsOpen == false && vars.ToggleCount == 0);

                // Click on arrow
                ctx->MouseMove("AAA", ImGuiTestOpFlags_MoveToEdgeL);
                ctx->MouseDown(0); // Toggle on Down when hovering Arrow
                IM_CHECK_EQ(vars.IsOpen, true);
                ctx->MouseUp(0);
                IM_CHECK_EQ(vars.IsOpen, true);
                ctx->MouseClick(0);
                IM_CHECK_EQ(vars.IsOpen, false);
                ctx->MouseDoubleClick(0);
                IM_CHECK_EQ(vars.IsOpen, false);
                IM_CHECK_EQ(vars.ToggleCount, 4);

                // Click on main section (_OpenOnArrow is implicit/automatic with MultiSelect, so Main Section won't react)
                if (!vars.IsMultiSelect)
                {
                    vars.ToggleCount = 0;
                    ctx->MouseMove("AAA");
                    ctx->MouseClick(0);
                    IM_CHECK_EQ_NO_RET(vars.IsOpen, true);
                    ctx->MouseClick(0);
                    IM_CHECK_EQ_NO_RET(vars.IsOpen, false);
                    ctx->MouseDoubleClick(0);
                    IM_CHECK_EQ_NO_RET(vars.IsOpen, false);
                    IM_CHECK_EQ_NO_RET(vars.ToggleCount, 4);
                }

                // Test TreeNode as drag source
                IM_CHECK_EQ(vars.DragSourceCount, 0);
                ctx->ItemDragWithDelta("AAA", ImVec2(50, 50));
                IM_CHECK_GT(vars.DragSourceCount, 0);
                IM_CHECK_EQ(vars.IsOpen, false);

                // Test TreeNode opening on drag-hold
                ctx->ItemDragOverAndHold("Color", "AAA");
                IM_CHECK_EQ(vars.IsOpen, true);
            }

            {
                ctx->LogInfo("## ImGuiTreeNodeFlags_OpenOnDoubleClick, IsMultiSelect=%d", vars.IsMultiSelect);
                vars.Reset = true;
                vars.Flags = ImGuiTreeNodeFlags_OpenOnDoubleClick;
                ctx->Yield();
                IM_CHECK(vars.IsOpen == false && vars.ToggleCount == 0);

                // Click on arrow (_OpenOnArrow is implicit/automatic with MultiSelect)
                if (!vars.IsMultiSelect)
                {
                    ctx->MouseMove("AAA", ImGuiTestOpFlags_MoveToEdgeL);
                    ctx->MouseDown(0);
                    IM_CHECK_EQ(vars.IsOpen, false);
                    ctx->MouseUp(0);
                    IM_CHECK_EQ(vars.IsOpen, false);
                    ctx->MouseClick(0);
                    IM_CHECK_EQ(vars.IsOpen, false);
                    IM_CHECK_EQ(vars.ToggleCount, 0);
                    ctx->MouseDoubleClick(0);
                    IM_CHECK_EQ(vars.IsOpen, true);
                    ctx->MouseDoubleClick(0);
                    IM_CHECK_EQ(vars.IsOpen, false);
                    IM_CHECK_EQ(vars.ToggleCount, 2);
                }

                // Double-click on main section
                vars.ToggleCount = 0;
                ctx->MouseMove("AAA");
                ctx->MouseClick(0);
                IM_CHECK_EQ(vars.IsOpen, false);
                ctx->MouseClick(0);
                IM_CHECK_EQ(vars.IsOpen, false);
                IM_CHECK_EQ(vars.ToggleCount, 0);
                ctx->MouseDoubleClick(0);
                IM_CHECK_EQ(vars.IsOpen, true);
                ctx->MouseDoubleClick(0);
                IM_CHECK_EQ(vars.IsOpen, false);
                IM_CHECK_EQ(vars.ToggleCount, 2);

                // Test TreeNode as drag source
                IM_CHECK_EQ(vars.DragSourceCount, 0);
                ctx->ItemDragWithDelta("AAA", ImVec2(50, 50));
                IM_CHECK_GT(vars.DragSourceCount, 0);
                IM_CHECK_EQ(vars.IsOpen, false);

                // Test TreeNode opening on drag-hold
                ctx->ItemDragOverAndHold("Color", "AAA");
                IM_CHECK_EQ(vars.IsOpen, true);
            }

            {
                ctx->LogInfo("## ImGuiTreeNodeFlags_OpenOnArrow, IsMultiSelect=%d", vars.IsMultiSelect);
                vars.Reset = true;
                vars.Flags = ImGuiTreeNodeFlags_OpenOnArrow;
                ctx->Yield();
                IM_CHECK(vars.IsOpen == false && vars.ToggleCount == 0);

                // Click on arrow
                ctx->MouseMove("AAA", ImGuiTestOpFlags_MoveToEdgeL);
                ctx->MouseDown(0);
                IM_CHECK_EQ(vars.IsOpen, true);
                ctx->MouseUp(0);
                IM_CHECK_EQ(vars.IsOpen, true);
                ctx->MouseClick(0);
                IM_CHECK_EQ(vars.IsOpen, false);
                IM_CHECK_EQ(vars.ToggleCount, 2);
                ctx->MouseDoubleClick(0);
                IM_CHECK_EQ(vars.IsOpen, false);
                IM_CHECK_EQ(vars.ToggleCount, 4);

                // Click on main section
                vars.ToggleCount = 0;
                ctx->MouseMove("AAA");
                ctx->MouseClick(0);
                IM_CHECK_EQ(vars.IsOpen, false);
                ctx->MouseClick(0);
                IM_CHECK_EQ(vars.IsOpen, false);
                IM_CHECK_EQ(vars.ToggleCount, 0);
                ctx->MouseDoubleClick(0);
                IM_CHECK_EQ(vars.IsOpen, false);
                IM_CHECK_EQ(vars.ToggleCount, 0);

                // Test TreeNode as drag source
                IM_CHECK_EQ(vars.DragSourceCount, 0);
                ctx->ItemDragWithDelta("AAA", ImVec2(50, 50));
                IM_CHECK_GT(vars.DragSourceCount, 0);
                IM_CHECK_EQ(vars.IsOpen, false);

                // Test TreeNode opening on drag-hold
                ctx->ItemDragOverAndHold("Color", "AAA");
                IM_CHECK_EQ(vars.IsOpen, true);
            }

            {
                ctx->LogInfo("## ImGuiTreeNodeFlags_OpenOnArrow|ImGuiTreeNodeFlags_OpenOnDoubleClick, IsMultiSelect=%d", vars.IsMultiSelect);
                vars.Reset = true;
                vars.Flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
                ctx->Yield();
                IM_CHECK(vars.IsOpen == false && vars.ToggleCount == 0);

                // Click on arrow
                ctx->MouseMove("AAA", ImGuiTestOpFlags_MoveToEdgeL);
                ctx->MouseDown(0);
                IM_CHECK_EQ(vars.IsOpen, true);
                ctx->MouseUp(0);
                ctx->MouseClick(0);
                IM_CHECK_EQ(vars.IsOpen, false);
                IM_CHECK_EQ(vars.ToggleCount, 2);
                ctx->MouseDoubleClick(0);
                IM_CHECK_EQ(vars.IsOpen, false);
                IM_CHECK_EQ(vars.ToggleCount, 4);

                // Click on main section
                vars.ToggleCount = 0;
                ctx->MouseMove("AAA");
                ctx->MouseClick(0);
                IM_CHECK_EQ(vars.IsOpen, false);
                ctx->MouseClick(0);
                IM_CHECK_EQ(vars.IsOpen, false);
                IM_CHECK_EQ(vars.ToggleCount, 0);
                ctx->MouseDoubleClick(0);
                IM_CHECK_EQ(vars.IsOpen, true);
                ctx->MouseDoubleClick(0);
                IM_CHECK_EQ(vars.IsOpen, false);
                IM_CHECK_EQ(vars.ToggleCount, 2);

                // Test TreeNode as drag source
                IM_CHECK_EQ(vars.DragSourceCount, 0);
                ctx->ItemDragWithDelta("AAA", ImVec2(50, 50));
                IM_CHECK_GT(vars.DragSourceCount, 0);
                IM_CHECK_EQ(vars.IsOpen, false);

                // Test TreeNode opening on drag-hold
                ctx->ItemDragOverAndHold("Color", "AAA");
                IM_CHECK_EQ(vars.IsOpen, true);
            }
        }
    };

    // ## Test ImGuiTreeNodeFlags_SpanAvailWidth and ImGuiTreeNodeFlags_SpanFullWidth flags
    t = IM_REGISTER_TEST(e, "widgets", "widgets_treenode_span_width");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiCond_Always);
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);

        ImGuiContext& g = *ImGui::GetCurrentContext();
        ImGuiWindow* window = ImGui::GetCurrentWindow();

        ImGui::SetNextItemOpen(true);
        if (ImGui::TreeNodeEx("Parent"))
        {
            // Interaction rect zoes not span entire width of work area.
            IM_CHECK(g.LastItemData.Rect.Max.x < window->WorkRect.Max.x);
            // But it starts at very beginning of WorkRect for first tree level.
            IM_CHECK(g.LastItemData.Rect.Min.x == window->WorkRect.Min.x);
            ImGui::SetNextItemOpen(true);
            if (ImGui::TreeNodeEx("Regular"))
            {
                // Interaction rect does not span entire width of work area.
                IM_CHECK(g.LastItemData.Rect.Max.x < window->WorkRect.Max.x);
                IM_CHECK(g.LastItemData.Rect.Min.x > window->WorkRect.Min.x);
                ImGui::TreePop();
            }
            ImGui::SetNextItemOpen(true);
            if (ImGui::TreeNodeEx("SpanAvailWidth", ImGuiTreeNodeFlags_SpanAvailWidth))
            {
                // Interaction rect matches visible frame rect
                IM_CHECK((g.LastItemData.StatusFlags & ImGuiItemStatusFlags_HasDisplayRect) != 0);
                IM_CHECK(g.LastItemData.DisplayRect.Min == g.LastItemData.Rect.Min);
                IM_CHECK(g.LastItemData.DisplayRect.Max == g.LastItemData.Rect.Max);
                // Interaction rect extends to the end of the available area.
                IM_CHECK(g.LastItemData.Rect.Max.x == window->WorkRect.Max.x);
                ImGui::TreePop();
            }
            ImGui::SetNextItemOpen(true);
            if (ImGui::TreeNodeEx("SpanFullWidth", ImGuiTreeNodeFlags_SpanFullWidth))
            {
                // Interaction rect matches visible frame rect
                IM_CHECK((g.LastItemData.StatusFlags & ImGuiItemStatusFlags_HasDisplayRect) != 0);
                IM_CHECK(g.LastItemData.DisplayRect.Min == g.LastItemData.Rect.Min);
                IM_CHECK(g.LastItemData.DisplayRect.Max == g.LastItemData.Rect.Max);
                // Interaction rect extends to the end of the available area.
                IM_CHECK(g.LastItemData.Rect.Max.x == window->WorkRect.Max.x);
                // ImGuiTreeNodeFlags_SpanFullWidth also extends interaction rect to the left.
                IM_CHECK(g.LastItemData.Rect.Min.x == window->WorkRect.Min.x);
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }

        ImGui::End();
    };

    // ## Test PlotLines() with a single value (#2387).
    t = IM_REGISTER_TEST(e, "widgets", "widgets_plot_lines_unexpected_input");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        float values[1] = {0.f};
        ImGui::PlotLines("PlotLines 1", NULL, 0);
        ImGui::PlotLines("PlotLines 2", values, 0);
        ImGui::PlotLines("PlotLines 3", values, 1);
        // FIXME-TESTS: If test did not crash - it passed. A better way to check this would be useful.
    };

    // ## Test ColorEdit hex input
    t = IM_REGISTER_TEST(e, "widgets", "widgets_coloredit_hexinput");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::ColorEdit4("ColorEdit1", &vars.Color1.x, ImGuiColorEditFlags_DisplayHex);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        vars.Color1 = ImVec4(1, 0, 0, 1);

        ctx->SetRef("Test Window");

        // Compare with epsilon.
#define IM_EQUAL_EPS(a, b) (ImAbs(a - b) < 0.0000001f)
        auto equal = [](const ImVec4& a, const ImVec4& b) { return IM_EQUAL_EPS(a.x, b.x) && IM_EQUAL_EPS(a.y, b.y) && IM_EQUAL_EPS(a.z, b.z) && IM_EQUAL_EPS(a.w, b.w); };
#undef IM_EQUAL_EPS

        // Test hex inputs.
        ctx->ItemClick("ColorEdit1/##ColorButton");
        ctx->SetRef("/$FOCUSED");
        ctx->ItemInputValue("##picker/##hex/##Text", "112233");
        IM_CHECK(equal(vars.Color1, ImVec4(ImColor(0x11, 0x22, 0x33, 0xFF))));
        ctx->ItemInputValue("##picker/##hex/##Text", "11223344");
        IM_CHECK(equal(vars.Color1, ImVec4(ImColor(0x11, 0x22, 0x33, 0x44))));
        ctx->ItemInputValue("##picker/##hex/##Text", "#112233");
        IM_CHECK(equal(vars.Color1, ImVec4(ImColor(0x11, 0x22, 0x33, 0xFF))));
        ctx->ItemInputValue("##picker/##hex/##Text", "#11223344");
        IM_CHECK(equal(vars.Color1, ImVec4(ImColor(0x11, 0x22, 0x33, 0x44))));
    };

    // ## Test preserving hue and saturation for certain colors having these components undefined.
    t = IM_REGISTER_TEST(e, "widgets", "widgets_coloredit_hue_sat");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        int& rgbi = vars.Int1;
        ImVec4& rgbf = vars.Color1;
        ImVec4 color = (vars.Step == 0) ? rgbf : ImGui::ColorConvertU32ToFloat4(rgbi);
        ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiCond_Appearing);
        if (ImGui::Begin("Window", NULL, ImGuiWindowFlags_NoSavedSettings))
        {
            bool use_int = (vars.Step == 1);
            if (ImGui::Checkbox("Use int", &use_int))
                vars.Step = use_int ? 1 : 0;
            if (ImGui::ColorEdit4("Color", &color.x, ImGuiColorEditFlags_PickerHueBar))
            {
                if (vars.Step == 0)
                    rgbf = color;
                else
                    rgbi = ImGui::ColorConvertFloat4ToU32(color);
            }
        }
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;

        struct Color
        {
            float R, G, B;
            float H, S, V;
        };

        auto read_color = [&]()
        {
            int& rgbi = vars.Int1;
            ImVec4& rgbf = vars.Color1;
            ImVec4 rgb = vars.Step == 0 ? rgbf : ImGui::ColorConvertU32ToFloat4(rgbi);
            Color color;
            color.R = rgb.x;
            color.G = rgb.y;
            color.B = rgb.z;
            ImGui::ColorConvertRGBtoHSV(color.R, color.G, color.B, color.H, color.S, color.V);
            return color;
        };

        ImGuiContext& g = *ctx->UiContext;
        ctx->ItemClick("Window/Color/##ColorButton");
        ctx->SetRef("/$FOCUSED");
        ImGuiWindow* popup = g.NavWindow;

        // Variant 0: use float RGB.
        // Variant 1: use int32 RGB.
        for (int variant = 0; variant < 2; variant++)
        {
            vars.Step = variant;
            ctx->LogDebug("Test variant: %d", variant);

            ctx->ItemClick("##picker/sv");                  // Set initial color
            ctx->ItemClick("##picker/hue");

            Color color_start = read_color();
            IM_CHECK_NE(color_start.H, 0.0f);
            IM_CHECK_NE(color_start.S, 0.0f);
            IM_CHECK_NE(color_start.V, 0.0f);

            // Set saturation to 0, hue must be preserved.
            ctx->ItemDragWithDelta("##picker/sv", ImVec2(-popup->Size.x * 0.5f, 0));

            Color color = read_color();
            IM_CHECK_EQ(color.S, 0.0f);
            IM_CHECK_EQ(color.H, 0.0f);                     // Hue undefined
            IM_CHECK_EQ(color_start.H, g.ColorEditLastHue); // Preserved hue matches original color

            // Test saturation preservation during mouse input.
            ctx->ItemClick("##picker/sv");
            ctx->ItemClick("##picker/hue");
            IM_CHECK_NE(read_color().S, 0.0f);
            color_start = read_color();
            ctx->MouseDown();

            // Move mouse across hue slider (extremes)
            ctx->MouseMoveToPos(g.IO.MousePos + ImVec2(0.0f, +popup->Size.y * 0.5f));  // Bottom
            ctx->MouseMoveToPos(g.IO.MousePos + ImVec2(0.0f, -popup->Size.y * 1.0f));  // Top
            ctx->MouseUp();
            IM_CHECK_EQ(read_color().S, color_start.S);
            IM_CHECK_EQ(g.ColorEditLastSat, color_start.H);

            ctx->ItemClick("##picker/sv");                  // Reset color
            ctx->ItemClick("##picker/hue");
            color_start = read_color();

            // Set value to 0, saturation must be preserved.
            ctx->ItemDragWithDelta("##picker/sv", ImVec2(0, +popup->Size.y * 0.5f));
            color = read_color();
            IM_CHECK_EQ(color.V, 0.0f);
            IM_CHECK_EQ(color.S, 0.0f);                     // Saturation undefined
            IM_CHECK_EQ(color_start.S, g.ColorEditLastSat); // Preserved saturation matches original color

            // Set color to pure white and verify it can reach (1.0f, 1.0f, 1.0f).
            ctx->ItemDragWithDelta("##picker/sv", ImVec2(-popup->Size.x * 0.5f, -popup->Size.y * 0.5f));
            color = read_color();
            IM_CHECK_EQ(color.R, 1.0f);
            IM_CHECK_EQ(color.G, 1.0f);
            IM_CHECK_EQ(color.B, 1.0f);

            // Move hue to extreme ends, see if it does not wrap around.
            ctx->ItemDragWithDelta("##picker/hue", ImVec2(0, +popup->Size.y * 0.5f));
            IM_CHECK_EQ(read_color().H, 0.0f);              // Hue undefined
            IM_CHECK_EQ(g.ColorEditLastHue, 1.0f);          // Preserved hue matches just set value
            ctx->ItemDragWithDelta("##picker/hue", ImVec2(0, -popup->Size.y * 0.5f));
            IM_CHECK_EQ(read_color().H, 0.0f);              // Hue undefined
            IM_CHECK_EQ(g.ColorEditLastHue, 0.0f);          // Preserved hue matches just set value

            // Test hue preservation during mouse input.
            ctx->ItemClick("##picker/hue");
            ctx->ItemClick("##picker/sv");
            IM_CHECK_NE(read_color().H, 0.0f);              // Hue defined
            color_start = read_color();
            ctx->MouseDown();

            // Move mouse across all edges (extremes)
            ctx->MouseMoveToPos(g.IO.MousePos + ImVec2(-popup->Size.x * 0.5f, -popup->Size.y * 0.5f));  // TL
            ctx->MouseMoveToPos(g.IO.MousePos + ImVec2(+popup->Size.x * 1.0f, 0.0f));  // TR
            ctx->MouseMoveToPos(g.IO.MousePos + ImVec2(0.0f, +popup->Size.y * 1.0f));  // BR
            ctx->MouseMoveToPos(g.IO.MousePos + ImVec2(-popup->Size.x * 1.0f, 0.0f));  // BL
            ctx->MouseMoveToPos(g.IO.MousePos + ImVec2(0.0f, -popup->Size.y * 1.0f));  // TL
            ctx->MouseUp();
            IM_CHECK_EQ(color_start.H, g.ColorEditLastHue); // Hue remains unchanged during all operations
        }
    };

    // ## Test ColorEdit basic Drag and Drop
    t = IM_REGISTER_TEST(e, "widgets", "widgets_drag_coloredit");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGui::SetNextWindowSize(ImVec2(300, 200));
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        ImGui::ColorEdit4("ColorEdit1", &vars.Color1.x, ImGuiColorEditFlags_None);
        ImGui::ColorEdit4("ColorEdit2", &vars.Color2.x, ImGuiColorEditFlags_None);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        vars.Color1 = ImVec4(1, 0, 0, 1);
        vars.Color2 = ImVec4(0, 1, 0, 1);

        ctx->SetRef("Test Window");

        IM_CHECK_NE(vars.Color1, vars.Color2);
        ctx->ItemDragAndDrop("ColorEdit1/##ColorButton", "ColorEdit2/##X"); // FIXME-TESTS: Inner items
        IM_CHECK_EQ(vars.Color1, vars.Color2);
    };

    // ## Test BeginDragDropSource() with NULL id.
    t = IM_REGISTER_TEST(e, "widgets", "widgets_drag_source_null_id");
    struct WidgetDragSourceNullIDData
    {
        ImGuiID SrcViewport = 0;
        ImGuiID DstViewport = 0;
        ImVec2 SrcPos;
        ImVec2 DstPos;
        bool Dropped = false;
    };
    t->SetVarsDataType<WidgetDragSourceNullIDData>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        WidgetDragSourceNullIDData& vars = ctx->GetVars<WidgetDragSourceNullIDData>();

        ImGui::Begin("Null ID Test", NULL, ImGuiWindowFlags_NoSavedSettings);
        ImGui::TextUnformatted("Null ID");

#ifdef IMGUI_HAS_DOCK
        vars.SrcViewport = ImGui::GetWindowViewport()->ID;
#endif
        vars.SrcPos = ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax()).GetCenter();

        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
        {
            int magic = 0xF00;
            ImGui::SetDragDropPayload("MAGIC", &magic, sizeof(int));
            ImGui::EndDragDropSource();
        }
        ImGui::TextUnformatted("Drop Here");
#ifdef IMGUI_HAS_DOCK
        vars.DstViewport = ImGui::GetWindowViewport()->ID;
#endif
        vars.DstPos = ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax()).GetCenter();

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MAGIC"))
            {
                vars.Dropped = true;
                IM_CHECK_EQ(payload->DataSize, (int)sizeof(int));
                IM_CHECK_EQ(*(int*)payload->Data, 0xF00);
            }
            ImGui::EndDragDropTarget();
        }
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        WidgetDragSourceNullIDData& vars = ctx->GetVars<WidgetDragSourceNullIDData>();

        // ImGui::TextUnformatted() does not have an ID therefore we can not use ctx->ItemDragAndDrop() as that refers
        // to items by their ID.
#ifdef IMGUI_HAS_DOCK
        ctx->MouseSetViewportID(vars.SrcViewport);
#endif
        ctx->MouseMoveToPos(vars.SrcPos);
        ctx->SleepStandard();
        ctx->MouseDown(0);

#ifdef IMGUI_HAS_DOCK
        ctx->MouseSetViewportID(vars.DstViewport);
#endif
        ctx->MouseMoveToPos(vars.DstPos);
        ctx->SleepStandard();
        ctx->MouseUp(0);

        IM_CHECK(vars.Dropped);
    };

    // ## Test preserving g.ActiveId during drag operation opening tree items.
    t = IM_REGISTER_TEST(e, "widgets", "widgets_drag_hold_to_open");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        if (ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings))
        {
            ImGui::Button("Drag");
            if (ImGui::BeginDragDropSource())
            {
                int magic = 0xF00;
                ImGui::SetDragDropPayload("MAGIC", &magic, sizeof(int));
                ImGui::EndDragDropSource();
            }
        }
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiContext& g = *ctx->UiContext;

        ImVec2 main_viewport_pos = ImGui::GetMainViewport()->Pos;
        ctx->WindowMove("Dear ImGui Demo", main_viewport_pos + ImVec2(16, 16));
        ctx->WindowResize("Dear ImGui Demo", ImVec2(400, 800));
        ctx->WindowMove("Test Window", main_viewport_pos = ImVec2(416, 16));

        ctx->SetRef("Dear ImGui Demo");
        ctx->ItemCloseAll("");
        ctx->WindowFocus(""); // So it's under TestWindow
        // FIXME-TESTS: This can still fail if "Test Window" covers "Dear ImGui Demo"

        ctx->SetRef("Test Window");
        ImGuiID active_id = ctx->GetID("Drag");
        ctx->MouseMove("Drag");
        ctx->SleepStandard();
        ctx->MouseDown();
        ctx->MouseLiftDragThreshold();
        IM_CHECK_EQ(g.ActiveId, active_id);

        ctx->SetRef("Dear ImGui Demo");
        ctx->MouseMove("Widgets", ImGuiTestOpFlags_NoFocusWindow);
        ctx->SleepNoSkip(1.0f, 1.0f / 60.0f);

        IM_CHECK(ctx->ItemInfo("Widgets") != NULL);
        IM_CHECK((ctx->ItemInfo("Widgets")->StatusFlags & ImGuiItemStatusFlags_Opened) != 0);
        IM_CHECK_EQ(g.ActiveId, active_id);
        ctx->MouseMove("Trees", ImGuiTestOpFlags_NoFocusWindow);
        ctx->SleepNoSkip(1.0f, 1.0f / 60.0f);
        IM_CHECK((ctx->ItemInfo("Trees")->StatusFlags & ImGuiItemStatusFlags_Opened) != 0);
        IM_CHECK_EQ(g.ActiveId, active_id);
        ctx->MouseUp(0);
    };

    // ## Test overlapping drag and drop targets. The drag and drop system always prioritize the smaller target.
    t = IM_REGISTER_TEST(e, "widgets", "widgets_drag_overlapping_targets");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);

        ImGui::Button("Drag");
        if (ImGui::BeginDragDropSource())
        {
            int value = 0xF00D;
            ImGui::SetDragDropPayload("_TEST_VALUE", &value, sizeof(int));
            ImGui::EndDragDropSource();
        }

        auto render_button = [](ImGuiTestContext* ctx, const char* name, const ImVec2& pos, const ImVec2& size)
        {
            ImGuiTestGenericVars& vars = ctx->GenericVars;
            ImGui::SetCursorScreenPos(pos);
            ImGui::Button(name, size);
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_TEST_VALUE"))
                {
                    IM_UNUSED(payload);
                    vars.Id = ImGui::GetItemID();
                }
                ImGui::EndDragDropTarget();
            }
        };

        // Render small button over big one
        ImVec2 pos = ImGui::GetCursorScreenPos();
        render_button(ctx, "Big1", pos, ImVec2(100, 100));
        render_button(ctx, "Small1", pos + ImVec2(25, 25), ImVec2(50, 50));

        // Render small button over small one
        render_button(ctx, "Small2", pos + ImVec2(0, 110) + ImVec2(25, 25), ImVec2(50, 50));
        render_button(ctx, "Big2", pos + ImVec2(0, 110), ImVec2(100, 100));

        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ctx->SetRef("Test Window");

        vars.Id = 0;
        ctx->ItemDragAndDrop("Drag", "Small1");
        IM_CHECK(vars.Id == ctx->GetID("Small1"));

        vars.Id = 0;
        ctx->ItemDragAndDrop("Drag", "Small2");
        IM_CHECK(vars.Id == ctx->GetID("Small2"));
    };

    // ## Test drag sources with _SourceNoPreviewTooltip flag not producing a tooltip.
    // ## Test drag target/accept with ImGuiDragDropFlags_AcceptNoPreviewTooltip
    t = IM_REGISTER_TEST(e, "widgets", "widgets_drag_no_preview_tooltip");
    struct DragNoPreviewTooltipVars { bool TooltipWasVisible = false; bool TooltipIsVisible = false; ImGuiDragDropFlags AcceptFlags = 0; };
    t->SetVarsDataType<DragNoPreviewTooltipVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        DragNoPreviewTooltipVars& vars = ctx->GetVars<DragNoPreviewTooltipVars>();

        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);

        auto create_drag_drop_source = [](ImGuiDragDropFlags flags)
        {
            if (ImGui::BeginDragDropSource(flags))
            {
                int value = 0xF00D;
                ImGui::SetDragDropPayload("_TEST_VALUE", &value, sizeof(int));
                ImGui::EndDragDropSource();
            }
        };

        ImGui::Button("Drag");
        create_drag_drop_source(ImGuiDragDropFlags_SourceNoPreviewTooltip);

        ImGui::Button("Drag Extern");
        if (ImGui::IsItemClicked())
            create_drag_drop_source(ImGuiDragDropFlags_SourceNoPreviewTooltip | ImGuiDragDropFlags_SourceExtern);

        ImGui::Button("Drag Accept");
        create_drag_drop_source(0);

        ImGui::Button("Drop");
        if (ImGui::BeginDragDropTarget())
        {
            ImGui::AcceptDragDropPayload("_TEST_VALUE", vars.AcceptFlags);
            ImGui::EndDragDropTarget();
        }

        ImGuiContext& g = *ctx->UiContext;
        ImGuiWindow* tooltip = ctx->GetWindowByRef(Str16f("##Tooltip_%02d", g.TooltipOverrideCount).c_str());
        vars.TooltipIsVisible = g.TooltipOverrideCount != 0 || (tooltip != NULL && (tooltip->Active || tooltip->WasActive));
        vars.TooltipWasVisible |= vars.TooltipIsVisible;

        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        DragNoPreviewTooltipVars& vars = ctx->GetVars<DragNoPreviewTooltipVars>();
        ctx->SetRef("Test Window");
        ctx->ItemDragAndDrop("Drag", "Drop");
        IM_CHECK(vars.TooltipWasVisible == false);
        vars.TooltipWasVisible = false;
        ctx->ItemDragAndDrop("Drag Extern", "Drop");
        IM_CHECK(vars.TooltipWasVisible == false);
        vars.TooltipWasVisible = false;

        vars.AcceptFlags = 0;
        ctx->ItemDragOverAndHold("Drag Accept", "Drop");
        //ctx->Yield();   // A visible tooltip window gets hidden with one frame delay. (due to how we test for Active || WasActive)
        IM_CHECK(vars.TooltipWasVisible == true);
        IM_CHECK(vars.TooltipIsVisible == true);
        ctx->MouseUp();
        vars.TooltipWasVisible = false;

        vars.AcceptFlags = ImGuiDragDropFlags_AcceptNoPreviewTooltip;
        ctx->ItemDragOverAndHold("Drag Accept", "Drop");
        ctx->Yield();   // A visible tooltip window gets hidden with one frame delay (due to how we test for Active || WasActive)
        IM_CHECK(vars.TooltipWasVisible == true);
        IM_CHECK(vars.TooltipIsVisible == false);
        ctx->MouseUp();
    };

    // ## Test drag & drop using three main mouse buttons.
    t = IM_REGISTER_TEST(e, "widgets", "widgets_drag_mouse_buttons");
    struct DragMouseButtonsVars { bool Pressed = false; bool Dropped = false; };
    t->SetVarsDataType<DragMouseButtonsVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiContext& g = *ImGui::GetCurrentContext();
        DragMouseButtonsVars& vars = ctx->GetVars<DragMouseButtonsVars>();
        ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Appearing);
        if (ImGui::Begin("Window", NULL, ImGuiWindowFlags_NoSavedSettings))
        {
            vars.Pressed |= ImGui::Button("Button");

            // This is a workaround for button widget not reacting to mouse clicks other than the left one.
            // See https://github.com/ocornut/imgui/issues/3885 for more details.
            ImGui::ButtonBehavior(g.LastItemData.Rect, g.LastItemData.ID, NULL, NULL, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonMiddle | ImGuiButtonFlags_MouseButtonRight);

            if (ImGui::BeginDragDropSource())
            {
                for (int button = 0; button < ImGuiMouseButton_COUNT; button++)
                    if (ImGui::IsMouseDown(button))
                        ImGui::Text("Dragged by button %d", button);
                ImGui::SetDragDropPayload("Button", "Works", 6);
                ImGui::EndDragDropSource();
            }

            ImGui::Button("Drop Here");
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Button"))
                    vars.Dropped = payload->Data != NULL && strcmp((const char*)payload->Data, "Works") == 0;
                ImGui::EndDragDropTarget();
            }
        }
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        DragMouseButtonsVars& vars = ctx->GetVars<DragMouseButtonsVars>();
        ctx->SetRef("Window");

        // Clicking still works.
        for (int button = 0; button < 3; button++)
        {
            vars.Pressed = false;
            ctx->ItemClick("Button", button);
            IM_CHECK(vars.Pressed);
        }

        // Drag & drop using all mouse buttons work.
        // FIXME: At this time only left, right and middle mouse buttons are supported for this usecase.
        for (ImGuiMouseButton button = 0; button < 3; button++)
        {
            vars.Dropped = false;
            ctx->ItemDragAndDrop("Button", "Drop Here", button);
            IM_CHECK(vars.Dropped);
        }
    };

    // ## Test using default context menu along with a combo (#4167)
#if IMGUI_VERSION_NUM >= 18211
    t = IM_REGISTER_TEST(e, "widgets", "widgets_combo_context_menu");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);

        if (ImGui::BeginCombo("Combo", "Preview"))
        {
            vars.Bool1 = true;
            ImGui::Selectable("Close");
            ImGui::EndCombo();
        }
        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::Button("Close"))
                ImGui::CloseCurrentPopup();
            vars.Bool2 = true;
            ImGui::EndPopup();
        }
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ctx->SetRef("Test Window");
        ctx->MouseMove("Combo");
        ctx->MouseClick(ImGuiMouseButton_Right);
        IM_CHECK(vars.Bool1 == false && vars.Bool2 == true);
        ctx->ItemClick("/$FOCUSED/Close");
        vars.Bool1 = vars.Bool2 = false;

        ctx->ItemClick("Combo");
        IM_CHECK(vars.Bool1 == true && vars.Bool2 == false);
        ctx->ItemClick("/$FOCUSED/Close");
        vars.Bool1 = vars.Bool2 = false;
    };
#endif

#if IMGUI_VERSION_NUM >= 18308
    // ## Test BeginComboPreview() and ImGuiComboFlags_CustomPreview
    t = IM_REGISTER_TEST(e, "widgets", "widgets_combo_custom_preview");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        const char* items[] = { "AAAA", "BBBB", "CCCC", "DDDD", "EEEE" };

        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGuiContext& g = *ImGui::GetCurrentContext();
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        ImGui::Text("Custom preview:");

        if (vars.Step == 2)
            ImGui::SetNextItemWidth(ImGui::GetFrameHeight() * 2.0f);

        const ImVec2 color_square_size = ImVec2(ImGui::GetFontSize(), ImGui::GetFontSize());
        bool open = ImGui::BeginCombo("custom", "", ImGuiComboFlags_CustomPreview);
        if (open)
        {
            for (int n = 0; n < 5; n++)
            {
                ImGui::PushID(n);
                ImGui::ColorButton("##color", ImVec4(1.0f, 0.5f, 0.5f, 1.0f), ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop, color_square_size);
                ImGui::SameLine();
                ImGui::Selectable(items[n]);
                ImGui::PopID();
            }
            ImGui::EndCombo();
        }
        ImRect combo_r = g.LastItemData.Rect;
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        int old_cmd_buffer_size = window->DrawList->CmdBuffer.Size;

        if (ImGui::BeginComboPreview())
        {
            ImGui::ColorButton("##color", ImVec4(1.0f, 0.5f, 0.5f, 1.0f), ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop, color_square_size);
            ImGui::TextUnformatted(items[0]);
            ImGui::EndComboPreview();
        }

        // Test sameline restore behavior
        ImGui::SameLine();
        IM_CHECK_EQ_NO_RET(combo_r.Min.y, window->DC.CursorPos.y);
        IM_CHECK_EQ(ImGui::GetStyle().FramePadding.y, window->DC.CurrLineTextBaseOffset);
        ImGui::Text("HELLO");

        // Test draw call merging behavior
        if (vars.Step == 1)
            IM_CHECK_EQ(old_cmd_buffer_size, window->DrawList->CmdBuffer.Size);
        if (vars.Step == 2)
            IM_CHECK_EQ(old_cmd_buffer_size + 2, window->DrawList->CmdBuffer.Size);

        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ctx->SetRef("Test Window");

        vars.Step = 0;
        ctx->WindowResize("", ImVec2(400, 400));
        vars.Step = 1;
        ctx->Yield(2);
        vars.Step = 2;
        ctx->Yield(2);
    };
#endif

#if IMGUI_VERSION_NUM >= 18408
    // ## Test null range in TextUnformatted() (#3615)
    t = IM_REGISTER_TEST(e, "widgets", "widgets_text_null");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        const char* str = "hello world";
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        {
            ImVec2 p0 = ImGui::GetCursorPos();
            ImGui::TextUnformatted(str, str + 5);
            ImGui::TextUnformatted(str + 1, str + 1);
            ImGui::TextUnformatted(NULL);
            ImVec2 p1 = ImGui::GetCursorPos();
            IM_CHECK_EQ(p0.y + ImGui::GetTextLineHeightWithSpacing() * 3, p1.y);
        }
        {
            ImVec2 p0 = ImGui::GetCursorScreenPos();
            ImGui::TextUnformatted("");
            ImGui::SameLine(0.0f, 0.0f);
            ImVec2 p1 = ImGui::GetCursorScreenPos();
            IM_CHECK_EQ(p0, p1);
        }
        ImGui::End();
    };
#endif

    // ## Test long text rendering in TextUnformatted()
    t = IM_REGISTER_TEST(e, "widgets", "widgets_text_long");
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ctx->SetRef("Dear ImGui Demo");
        ctx->MenuCheck("Examples/Long text display");
        ctx->SetRef("Example: Long text display");
        ctx->ItemClick("Add 1000 lines");
        ctx->SleepStandard();

        ImGuiWindow* log_panel = ctx->GetWindowByRef(ctx->GetChildWindowID("Example: Long text display", "Log"));
        IM_CHECK(log_panel != NULL);
        ImGui::SetScrollY(log_panel, log_panel->ScrollMax.y);
        ctx->SleepStandard();
        ctx->ItemClick("Clear");
        // FIXME-TESTS: A bit of extra testing that will be possible once tomato problem is solved.
        // ctx->ComboClick("Test type/Single call to TextUnformatted()");
        // ctx->ComboClick("Test type/Multiple calls to Text(), clipped");
        // ctx->ComboClick("Test type/Multiple calls to Text(), not clipped (slow)");
        ctx->WindowClose("");
    };

    // ## Flex code paths that try to avoid formatting when "%s" is used as format.
    t = IM_REGISTER_TEST(e, "widgets", "widgets_text_unformatted_shortcut");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        ImGui::Text("%s", "HELLO"); // FIXME: Maybe change this to add the shortcut?
        ImGui::TextDisabled("%s", "...");
        ImGui::TextColored(ImColor(IM_COL32_WHITE), "%s", "...");
        ImGui::TextWrapped("%s", "...");
        ImGuiContext& g = *ImGui::GetCurrentContext();
#if IMGUI_VERSION_NUM >= 18727
        IM_CHECK_STR_EQ(g.TempBuffer.Data, "HELLO");
#else
        IM_CHECK_STR_EQ(g.TempBuffer, "HELLO");
#endif
        ImGui::End();
    };

    // ## Test LabelText() variants layout (#4004)
    t = IM_REGISTER_TEST(e, "widgets", "widgets_label_text");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiContext& g = *ImGui::GetCurrentContext();

        ImGui::SetNextWindowSize(ImVec2(500, 500), ImGuiCond_Appearing);
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);

        ImGui::Separator();
        ImGui::LabelText("Single line label", "Single line text");
        IM_CHECK_EQ(g.LastItemData.Rect.GetHeight(), ImGui::GetFrameHeight());

        ImGui::Separator();
        ImGui::LabelText("Multi\n line\n label", "Single line text");
        IM_CHECK_EQ(g.LastItemData.Rect.GetHeight(), ImGui::GetTextLineHeight() * 3.0f + ImGui::GetStyle().FramePadding.y * 2.0f);

        ImGui::Separator();
        ImGui::LabelText("Single line label", "Multi\n line\n text");
        IM_CHECK_EQ(g.LastItemData.Rect.GetHeight(), ImGui::GetTextLineHeight() * 3.0f + ImGui::GetStyle().FramePadding.y * 2.0f);

        ImGui::End();
    };

    // ## Test menu appending.
    t = IM_REGISTER_TEST(e, "widgets", "widgets_menu_append");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGui::Begin("Append Menus", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_MenuBar);
        ImGui::BeginMenuBar();

        // Menu that we will append to.
        if (ImGui::BeginMenu("First Menu"))
        {
            ImGui::MenuItem("1 First");
            if (ImGui::BeginMenu("Second Menu"))
            {
                ImGui::MenuItem("2 First");
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }

        // Append to first menu.
        if (ImGui::BeginMenu("First Menu"))
        {
            if (ImGui::MenuItem("1 Second"))
                vars.Bool1 = true;
            if (ImGui::BeginMenu("Second Menu"))
            {
                ImGui::MenuItem("2 Second");
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ctx->SetRef("Append Menus");
        ctx->MenuClick("First Menu");
        ctx->MenuClick("First Menu/1 First");
        IM_CHECK_EQ(vars.Bool1, false);
        ctx->MenuClick("First Menu/1 Second");
        IM_CHECK_EQ(vars.Bool1, true);
        ctx->MenuClick("First Menu/Second Menu/2 First");
        ctx->MenuClick("First Menu/Second Menu/2 Second");
    };

    // ## Test text capture of separator in a menu.
    t = IM_REGISTER_TEST(e, "widgets", "widgets_menu_separator");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar);
        if (ImGui::BeginMenuBar())
        {
            ImGui::LogToBuffer();
            if (ImGui::BeginMenu("File"))
                ImGui::EndMenu();
            ImGui::Separator();
            IM_CHECK_EQ(ImGui::GetItemRectMax().y - ImGui::GetItemRectMin().y, ImGui::GetFrameHeight());
            if (ImGui::BeginMenu("Edit"))
                ImGui::EndMenu();
            ImGui::EndMenuBar();
            ImStrncpy(vars.Str1, ctx->UiContext->LogBuffer.c_str(), IM_ARRAYSIZE(vars.Str1));
            ImGui::LogFinish();
        }
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        IM_CHECK_STR_EQ(vars.Str1, "File | Edit");
    };

    // ## Test main menubar appending.
    t = IM_REGISTER_TEST(e, "widgets", "widgets_menu_mainmenubar_append");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        // Menu that we will append to.
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("First Menu"))
                ImGui::EndMenu();
            ImGui::EndMainMenuBar();
        }

        // Append to first menu.
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("Second Menu"))
            {
                if (ImGui::MenuItem("Second"))
                    vars.Bool1 = true;
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("Second Menu/Second");
        IM_CHECK_EQ(vars.Bool1, true);
    };

    // ## Test main menubar navigation
    t = IM_REGISTER_TEST(e, "widgets", "widgets_menu_mainmenubar_navigation");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("Menu 1"))
            {
                if (ImGui::BeginMenu("Sub Menu 1"))
                {
                    ImGui::MenuItem("Item 1");
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Menu 2"))
            {
                if (ImGui::BeginMenu("Sub Menu 2-1"))
                {
                    ImGui::MenuItem("Item 2");
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Sub Menu 2-2"))
                {
                    ImGui::MenuItem("Item 3");
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiContext& g = *ImGui::GetCurrentContext();

        ctx->ItemClick("##MainMenuBar##menubar/Menu 1");

        // Click doesn't affect g.NavId which is null at this point

        // Test basic key Navigation
        ctx->KeyPress(ImGuiKey_DownArrow);
        IM_CHECK_EQ(g.NavId, ctx->GetID("##Menu_00/Sub Menu 1"));
        ctx->KeyPress(ImGuiKey_RightArrow);
        IM_CHECK_EQ(g.NavId, ctx->GetID("##Menu_01/Item 1"));
        ctx->KeyPress(ImGuiKey_LeftArrow);
        IM_CHECK_EQ(g.NavId, ctx->GetID("##Menu_00/Sub Menu 1"));
        ctx->KeyPress(ImGuiKey_LeftArrow);
        IM_CHECK_EQ(g.NavId, ctx->GetID("##MainMenuBar##menubar/Menu 1"));

        ctx->MouseMove("##MainMenuBar##menubar/Menu 2"); // FIXME-TESTS: Workaround so TestEngine can find again Menu 1

        // Test forwarding of nav event to parent when there's no match in the current menu
        // Going from "Menu 1/Sub Menu 1/Item 1" to "Menu 2"
        ctx->ItemClick("##MainMenuBar##menubar/Menu 1");
        ctx->KeyPress(ImGuiKey_DownArrow);
        ctx->KeyPress(ImGuiKey_RightArrow);
        IM_CHECK_EQ(g.NavId, ctx->GetID("##Menu_01/Item 1"));
        ctx->KeyPress(ImGuiKey_RightArrow);
        IM_CHECK_EQ(g.NavId, ctx->GetID("##MainMenuBar##menubar/Menu 2"));

        ctx->KeyPress(ImGuiKey_DownArrow);
        IM_CHECK_EQ(g.NavId, ctx->GetID("##Menu_00/Sub Menu 2-1"));
        ctx->KeyPress(ImGuiKey_RightArrow);
        IM_CHECK_EQ(g.NavId, ctx->GetID("##Menu_01/Item 2"));

        ctx->KeyPress(ImGuiKey_LeftArrow);
        ctx->KeyPress(ImGuiKey_DownArrow);
        IM_CHECK_EQ(g.NavId, ctx->GetID("##Menu_00/Sub Menu 2-2"));
        ctx->KeyPress(ImGuiKey_RightArrow);
        IM_CHECK_EQ(g.NavId, ctx->GetID("##Menu_01/Item 3"));
    };

#ifdef IMGUI_HAS_MULTI_SELECT
    // ## Test MultiSelect API
    struct ExampleSelection
    {
        ImGuiStorage                        Storage;
        int                                 SelectionSize;  // Number of selected items (== number of 1 in the Storage, maintained by this class)
        int                                 RangeRef;       // Reference/pivot item (generally last clicked item)

        ExampleSelection()                  { RangeRef = 0; Clear(); }
        void Clear()                        { Storage.Clear(); SelectionSize = 0; }
        bool GetSelected(int n) const       { return Storage.GetInt((ImGuiID)n, 0) != 0; }
        void SetSelected(int n, bool v)     { int* p_int = Storage.GetIntRef((ImGuiID)n, 0); if (*p_int == (int)v) return; if (v) SelectionSize++; else SelectionSize--; *p_int = (bool)v; }
        int  GetSelectionSize() const       { return SelectionSize; }

        // When using SelectAll() / SetRange() we assume that our objects ID are indices.
        // In this demo we always store selection using indices and never in another manner (e.g. object ID or pointers).
        // If your selection system is storing selection using object ID and you want to support Shift+Click range-selection,
        // you will need a way to iterate from one object to another given the ID you use.
        // You are likely to need some kind of data structure to convert 'view index' <> 'object ID'.
        // FIXME-MULTISELECT: Would be worth providing a demo of doing this.
        // FIXME-MULTISELECT: SetRange() is currently very inefficient since it doesn't take advantage of the fact that ImGuiStorage stores sorted key.
        void SetRange(int n1, int n2, bool v)   { if (n2 < n1) { int tmp = n2; n2 = n1; n1 = tmp; } for (int n = n1; n <= n2; n++) SetSelected(n, v); }
        void SelectAll(int count)               { Storage.Data.resize(count); for (int idx = 0; idx < count; idx++) Storage.Data[idx] = ImGuiStorage::ImGuiStoragePair((ImGuiID)idx, 1); SelectionSize = count; } // This could be using SetRange(), but it this way is faster.

        // FIXME-MULTISELECT: This itself is a good condition we could improve either our API or our demos
        ImGuiMultiSelectData* BeginMultiSelect(ImGuiMultiSelectFlags flags, int items_count)
        {
            ImGuiMultiSelectData* data = ImGui::BeginMultiSelect(flags, (void*)(intptr_t)RangeRef, GetSelected(RangeRef));
            if (data->RequestClear)     { Clear(); }
            if (data->RequestSelectAll) { SelectAll(items_count); }
            return data;
        }
        void EndMultiSelect(int items_count)
        {
            ImGuiMultiSelectData* data = ImGui::EndMultiSelect();
            RangeRef = (int)(intptr_t)data->RangeSrc;
            if (data->RequestClear)     { Clear(); }
            if (data->RequestSelectAll) { SelectAll(items_count); }
            if (data->RequestSetRange)  { SetRange((int)(intptr_t)data->RangeSrc, (int)(intptr_t)data->RangeDst, data->RangeValue ? 1 : 0); }
        }
    };
    struct MultiSelectTestVars
    {
        ExampleSelection    Selection0;
        ExampleSelection    Selection1;
    };
    auto multiselect_guifunc = [](ImGuiTestContext* ctx)
    {
        MultiSelectTestVars& vars = ctx->GetVars<MultiSelectTestVars>();
        ExampleSelection& selection = vars.Selection0;

        const int ITEMS_COUNT = 100;
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("(Size = %3d items)", selection.SelectionSize);
        ImGui::Text("(RangeRef = %04d)", selection.RangeRef);
        ImGui::Separator();

        ImGuiMultiSelectData* multi_select_data = selection.BeginMultiSelect(ImGuiMultiSelectFlags_None, ITEMS_COUNT);

        if (ctx->Test->ArgVariant == 1)
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 0.0f));

        ImGuiListClipper clipper;
        clipper.Begin(ITEMS_COUNT);
        while (clipper.Step())
        {
            if (clipper.DisplayStart > selection.RangeRef)
                multi_select_data->RangeSrcPassedBy = true;
            for (int item_n = clipper.DisplayStart; item_n < clipper.DisplayEnd; item_n++)
            {
                Str64f label("Object %04d", item_n);
                bool item_is_selected = selection.GetSelected(item_n);

                ImGui::SetNextItemSelectionData((void*)(intptr_t)item_n);
                if (ctx->Test->ArgVariant == 0)
                {
                    ImGui::Selectable(label.c_str(), item_is_selected);
                    bool toggled = ImGui::IsItemToggledSelection();
                    //if (toggled)
                    //    ImGui::DebugLog("Item %d toggled selection %d->%d\n", item_n, item_is_selected, !item_is_selected);
                    if (toggled)
                        selection.SetSelected(item_n, !item_is_selected);
                }
                else if (ctx->Test->ArgVariant == 1)
                {
                    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth;
                    flags |= ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
                    if (item_is_selected)
                        flags |= ImGuiTreeNodeFlags_Selected;
                    if (ImGui::TreeNodeEx(label.c_str(), flags))
                        ImGui::TreePop();
                    //if (ImGui::IsItemToggledSelection())
                    //    ImGui::DebugLog("Item %d toggled selection %d->%d\n", item_n, item_is_selected, !item_is_selected);

                    if (ImGui::IsItemToggledSelection())
                        selection.SetSelected(item_n, !item_is_selected);
                }
            }
        }

        if (ctx->Test->ArgVariant == 1)
            ImGui::PopStyleVar();

        selection.EndMultiSelect(ITEMS_COUNT);

        ImGui::End();
    };
    auto multiselect_testfunc = [](ImGuiTestContext* ctx)
    {
        // We are using lots of MouseMove+MouseDown+MouseUp (instead of ItemClick) because we need to test precise MouseUp vs MouseDown reactions.
        ImGuiContext& g = *ctx->UiContext;
        MultiSelectTestVars& vars = ctx->GetVars<MultiSelectTestVars>();
        ExampleSelection& selection = vars.Selection0;

        ctx->SetRef("Test Window");

        selection.Clear();
        ctx->Yield();
        IM_CHECK_EQ(selection.SelectionSize, 0);

        // Single click
        ctx->ItemClick("Object 0000");
        IM_CHECK_EQ(selection.SelectionSize, 1);
        IM_CHECK_EQ(selection.GetSelected(0), true);

        // Verify that click on another item alter selection on MouseDown
        ctx->MouseMove("Object 0001");
        ctx->MouseDown(0);
        IM_CHECK_EQ(selection.SelectionSize, 1);
        IM_CHECK_EQ(selection.GetSelected(1), true);
        ctx->MouseUp(0);

        // CTRL-A
        ctx->KeyPress(ImGuiKey_A, ImGuiModFlags_Ctrl);
        IM_CHECK_EQ(selection.SelectionSize, 100);

        // Verify that click on selected item clear other items from selection on MouseUp
        ctx->MouseMove("Object 0001");
        ctx->MouseDown(0);
        IM_CHECK_EQ(selection.SelectionSize, 100);
        ctx->MouseUp(0);
        IM_CHECK_EQ(selection.SelectionSize, 1);
        IM_CHECK_EQ(selection.GetSelected(1), true);

        // Test SHIFT+Click
        ctx->ItemClick("Object 0001");
        ctx->KeyModDown(ImGuiModFlags_Shift);
        ctx->MouseMove("Object 0006");
        ctx->MouseDown(0);
        IM_CHECK_EQ(selection.SelectionSize, 6);
        ctx->MouseUp(0);
        ctx->KeyModUp(ImGuiModFlags_Shift);

        // Test that CTRL+A preserve RangeSrc (which was 0001)
        ctx->KeyPress(ImGuiKey_A, ImGuiModFlags_Ctrl);
        IM_CHECK_EQ(selection.SelectionSize, 100);
        ctx->KeyModDown(ImGuiModFlags_Shift);
        ctx->ItemClick("Object 0008");
        ctx->KeyModUp(ImGuiModFlags_Shift);
        IM_CHECK_EQ(selection.SelectionSize, 8);

        // Test reverse clipped SHIFT+Click
        // FIXME-TESTS: ItemInfo query could disable clipper?
        // FIXME-TESTS: We would need to disable clipper because it conveniently rely on cliprect which is affected by actual viewport, so ScrollToBottom() is not enough...
        //ctx->ScrollToBottom();
        ctx->ItemClick("Object 0030");
        ctx->ScrollToTop("");
        ctx->KeyModDown(ImGuiModFlags_Shift);
        ctx->ItemClick("Object 0002");
        ctx->KeyModUp(ImGuiModFlags_Shift);
        IM_CHECK_EQ(selection.SelectionSize, 29);

        // Test ESC to clear selection
        // FIXME-TESTS: Feature not implemented
#if IMGUI_BROKEN_TESTS
        ctx->KeyPress(ImGuiKey_Escape);
        ctx->Yield();
        IM_CHECK_EQ(selection.SelectionSize, 0);
#endif

        // Test CTRL+Click
#if IMGUI_VERSION_NUM >= 18730
        ctx->ItemClick("Object 0001");
        IM_CHECK_EQ(selection.SelectionSize, 1);
        ctx->KeyModDown(ImGuiModFlags_Ctrl);
        ctx->ItemClick("Object 0006");
        ctx->ItemClick("Object 0007");
        ctx->KeyModUp(ImGuiModFlags_Ctrl);
        IM_CHECK_EQ(selection.SelectionSize, 3);
        ctx->ItemClick("Object 0008");
        IM_CHECK_EQ(selection.SelectionSize, 1);
#endif

        // Test SHIFT+Arrow
        ctx->ItemClick("Object 0002");
        IM_CHECK_EQ(g.NavId, ctx->GetID("Object 0002"));
        IM_CHECK_EQ(selection.SelectionSize, 1);
        ctx->KeyPress(ImGuiKey_DownArrow, ImGuiModFlags_Shift);
        ctx->KeyPress(ImGuiKey_DownArrow, ImGuiModFlags_Shift);
        IM_CHECK_EQ(g.NavId, ctx->GetID("Object 0004"));
        IM_CHECK_EQ(selection.SelectionSize, 3);

        // Test CTRL+Arrow
        ctx->KeyPress(ImGuiKey_DownArrow, ImGuiModFlags_Ctrl);
        ctx->KeyPress(ImGuiKey_DownArrow, ImGuiModFlags_Ctrl);
        IM_CHECK_EQ(g.NavId, ctx->GetID("Object 0006"));
        IM_CHECK_EQ(selection.SelectionSize, 3);

        // Test SHIFT+Arrow after a gap
        ctx->KeyPress(ImGuiKey_DownArrow, ImGuiModFlags_Shift);
        IM_CHECK_EQ(g.NavId, ctx->GetID("Object 0007"));
        IM_CHECK_EQ(selection.SelectionSize, 6);

        // Test SHIFT+Arrow reducing selection
        ctx->KeyPress(ImGuiKey_UpArrow, ImGuiModFlags_Shift);
        IM_CHECK_EQ(g.NavId, ctx->GetID("Object 0006"));
        IM_CHECK_EQ(selection.SelectionSize, 5);

        // Test CTRL+Shift+Arrow moving or appending without reducing selection
        ctx->KeyPress(ImGuiKey_UpArrow, ImGuiModFlags_Ctrl | ImGuiModFlags_Shift, 4);
        IM_CHECK_EQ(g.NavId, ctx->GetID("Object 0002"));
        IM_CHECK_EQ(selection.SelectionSize, 5);

        // Test SHIFT+Arrow replacing selection
        ctx->KeyPress(ImGuiKey_UpArrow, ImGuiModFlags_Shift);
        IM_CHECK_EQ(g.NavId, ctx->GetID("Object 0001"));
        IM_CHECK_EQ(selection.SelectionSize, 2);

        // Test Arrow replacing selection
        ctx->KeyPress(ImGuiKey_DownArrow);
        IM_CHECK_EQ(g.NavId, ctx->GetID("Object 0002"));
        IM_CHECK_EQ(selection.SelectionSize, 1);
        IM_CHECK_EQ(selection.GetSelected(2), true);

        // Test Home/End
        ctx->KeyPress(ImGuiKey_Home);
        IM_CHECK_EQ(g.NavId, ctx->GetID("Object 0000"));
        IM_CHECK_EQ(selection.SelectionSize, 1);
        IM_CHECK_EQ(selection.GetSelected(0), true);
        ctx->KeyPress(ImGuiKey_End);
        IM_CHECK_EQ(g.NavId, ctx->GetID("Object 0099"));
        IM_CHECK_EQ(selection.SelectionSize, 1);
        IM_CHECK_EQ(selection.GetSelected(99), true); // Would break if clipped by viewport
        ctx->KeyPress(ImGuiKey_Home, ImGuiModFlags_Ctrl);
        IM_CHECK_EQ(g.NavId, ctx->GetID("Object 0000"));
        IM_CHECK_EQ(selection.SelectionSize, 1);
        IM_CHECK_EQ(selection.GetSelected(99), true);
        ctx->KeyPress(ImGuiKey_Home);
        IM_CHECK_EQ(selection.SelectionSize, 1);
        IM_CHECK_EQ(selection.GetSelected(99), true); // FIXME: A Home/End/PageUp/PageDown leading to same target doesn't trigger JustMovedTo, may be reasonable.
        ctx->KeyPress(ImGuiKey_Space);
        IM_CHECK_EQ(selection.SelectionSize, 1);
        IM_CHECK_EQ(selection.GetSelected(0), true);
        ctx->KeyPress(ImGuiKey_End, ImGuiModFlags_Shift);
        IM_CHECK_EQ(g.NavId, ctx->GetID("Object 0099"));
        IM_CHECK_EQ(selection.SelectionSize, 100);
    };

    t = IM_REGISTER_TEST(e, "widgets", "widgets_multiselect_1_selectables");
    t->SetVarsDataType<MultiSelectTestVars>();
    t->GuiFunc = multiselect_guifunc;
    t->TestFunc = multiselect_testfunc;
    t->ArgVariant = 0;

    t = IM_REGISTER_TEST(e, "widgets", "widgets_multiselect_2_treenode");
    t->SetVarsDataType<MultiSelectTestVars>();
    t->GuiFunc = multiselect_guifunc;
    t->TestFunc = multiselect_testfunc;
    t->ArgVariant = 1;

    t = IM_REGISTER_TEST(e, "widgets", "widgets_multiselect_3_multiple");
    t->SetVarsDataType<MultiSelectTestVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        MultiSelectTestVars& vars = ctx->GetVars<MultiSelectTestVars>();

        const int ITEMS_COUNT = 10;
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        for (int scope_n = 0; scope_n < 2; scope_n++)
        {
            ExampleSelection& selection = (scope_n == 1) ? vars.Selection1 : vars.Selection0;
            ImGui::Text("SCOPE %d\n", scope_n);
            ImGui::PushID(Str16f("Scope %d", scope_n).c_str());
            selection.BeginMultiSelect(ImGuiMultiSelectFlags_None, ITEMS_COUNT);
            for (int item_n = 0; item_n < ITEMS_COUNT; item_n++)
            {
                bool item_is_selected = selection.GetSelected(item_n);
                ImGui::SetNextItemSelectionData((void*)(intptr_t)item_n);
                ImGui::Selectable(Str16f("Object %04d", item_n).c_str(), item_is_selected);
                if (ImGui::IsItemToggledSelection())
                    selection.SetSelected(item_n, !item_is_selected);
            }
            selection.EndMultiSelect(ITEMS_COUNT);
            ImGui::PopID();
        }
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        MultiSelectTestVars& vars = ctx->GetVars<MultiSelectTestVars>();
        ExampleSelection& selection0 = vars.Selection0;
        ExampleSelection& selection1 = vars.Selection1;

        ctx->SetRef("Test Window");
        selection0.Clear();
        selection1.Clear();

        ctx->ItemClick("Scope 0/Object 0001");
        IM_CHECK_EQ(selection0.SelectionSize, 1);
        IM_CHECK_EQ(selection1.SelectionSize, 0);

        ctx->ItemClick("Scope 1/Object 0002");
        IM_CHECK_EQ(selection0.SelectionSize, 1);
        IM_CHECK_EQ(selection1.SelectionSize, 1);

        ctx->KeyPress(ImGuiKey_A, ImGuiModFlags_Ctrl);
        IM_CHECK_EQ(selection0.SelectionSize, 1);
        IM_CHECK_EQ(selection1.SelectionSize, 10);
    };
#endif

    // ## Test Selectable() with ImGuiSelectableFlags_SpanAllColumns inside Columns()
    t = IM_REGISTER_TEST(e, "widgets", "widgets_selectable_span_all_columns");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_Appearing);
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        ImGui::Columns(3);
        ImGui::Button("C1");
        ImGui::NextColumn();
        ImGui::Selectable("Selectable", &vars.Bool1, ImGuiSelectableFlags_SpanAllColumns);
        vars.Status.QuerySet();
        ImGui::NextColumn();
        ImGui::Button("C3");
        ImGui::Columns();
        ImGui::PopStyleVar();

        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ctx->SetRef("Test Window");
        ctx->MouseMove("C1", ImGuiTestOpFlags_NoCheckHoveredId); // Button itself won't be hovered, Selectable will!
        IM_CHECK(vars.Status.Hovered == 1);
        ctx->MouseMove("C3", ImGuiTestOpFlags_NoCheckHoveredId); // Button itself won't be hovered, Selectable will!
        IM_CHECK(vars.Status.Hovered == 1);
    };

    // ## Test ImGuiSelectableFlags_SpanAllColumns flag when used in a table.
    t = IM_REGISTER_TEST(e, "widgets", "widgets_selectable_span_all_table");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_Appearing);
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);

        const int column_count = 3;
        ImGui::BeginTable("table", column_count, ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Reorderable);
        for (int i = 0; i < column_count; i++)
            ImGui::TableSetupColumn(Str30f("%d", i + 1).c_str());

        ImGui::TableHeadersRow();
        ImGui::TableNextRow();

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        ImGui::TableSetColumnIndex(0);
        ImGui::Button("C1");
        ImGui::TableSetColumnIndex(1);
        ImGui::Selectable("Selectable", &vars.Bool1, ImGuiSelectableFlags_SpanAllColumns);
        vars.Status.QuerySet();
        ImGui::TableSetColumnIndex(2);
        ImGui::Button("C3");
        ImGui::PopStyleVar();

        ImGui::EndTable();

        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ctx->SetRef("Test Window");
        ImGuiTable* table = ImGui::TableFindByID(ctx->GetID("table"));

        for (int i = 0; i < 2; i++)
        {
            ctx->MouseMove("table/C1", ImGuiTestOpFlags_NoCheckHoveredId); // Button itself won't be hovered, Selectable will!
            IM_CHECK(vars.Status.Hovered == 1);
            ctx->MouseMove("table/C3", ImGuiTestOpFlags_NoCheckHoveredId); // Button itself won't be hovered, Selectable will!
            IM_CHECK(vars.Status.Hovered == 1);

            // Reorder columns and test again
            ctx->ItemDragAndDrop(TableGetHeaderID(table, "1"), TableGetHeaderID(table, "2"));
        }
    };

    // ## Test sliders with inverted ranges.
    t = IM_REGISTER_TEST(e, "widgets", "widgets_slider_ranges");
    struct SliderRangeVars { struct { ImGuiDataTypeStorage Cur, Min, Max; int ResultFlags; } Values[ImGuiDataType_COUNT]; SliderRangeVars() { memset(this, 0, sizeof(*this)); } };
    t->SetVarsDataType<SliderRangeVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        SliderRangeVars& vars = ctx->GetVars<SliderRangeVars>();
        const int OUT_OF_RANGE_FLAG = 1;
        const int MID_OF_RANGE_FLAG = 2;

        // Submit sliders for each data type, with and without inverted range
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        for (ImGuiDataType data_type = 0; data_type < ImGuiDataType_COUNT; data_type++)
        {
            const ImGuiDataTypeInfo* data_type_info = ImGui::DataTypeGetInfo(data_type);
            for (int invert_range = 0; invert_range < 2; invert_range++)
            {
                auto& v = vars.Values[data_type];
                GetSliderTestRanges(data_type, &v.Min, &v.Max);
                Str30f label("%s%c", data_type_info->Name, invert_range == 0 ? '+' : '-');
                if (invert_range)
                    ImGui::SliderScalar(label.c_str(), data_type, &v.Cur, &v.Max, &v.Min);
                else
                    ImGui::SliderScalar(label.c_str(), data_type, &v.Cur, &v.Min, &v.Max);

                v.ResultFlags = 0;
                if (ImGui::DataTypeCompare(data_type, &v.Cur, &v.Min) < 0 || ImGui::DataTypeCompare(data_type, &v.Cur, &v.Max) > 0)
                    v.ResultFlags |= OUT_OF_RANGE_FLAG;
                if (ImGui::DataTypeCompare(data_type, &v.Cur, &v.Min) > 0 && ImGui::DataTypeCompare(data_type, &v.Cur, &v.Max) < 0)
                    v.ResultFlags |= MID_OF_RANGE_FLAG;
                ImGui::SameLine();
                ImGui::Text("R:%d", v.ResultFlags); // Doesn't make sense to display if not reset
            }
        }
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        SliderRangeVars& vars = ctx->GetVars<SliderRangeVars>();
        const int OUT_OF_RANGE_FLAG = 1;
        const int MID_OF_RANGE_FLAG = 2;

        ctx->SetRef("Test Window");
        for (int data_type = 0; data_type < ImGuiDataType_COUNT; data_type++)
        {
            for (int invert_range = 0; invert_range < 2; invert_range++)
            {
                auto& v = vars.Values[data_type];
                ImGuiDataTypeStorage v_left = invert_range ? v.Max : v.Min;
                ImGuiDataTypeStorage v_right = invert_range ? v.Min : v.Max;

                const ImGuiDataTypeInfo* data_type_info = ImGui::DataTypeGetInfo(data_type);
                Str30f label("%s%c", data_type_info->Name, invert_range == 0 ? '+' : '-');

                for (int side = 0; side < 2; side++)
                {
                    // Click on center
                    ctx->MouseMove(label.c_str());
                    ctx->MouseDown();
                    IM_CHECK((v.ResultFlags & OUT_OF_RANGE_FLAG) == 0); // Widget value does not fall out of range
                    IM_CHECK((v.ResultFlags & MID_OF_RANGE_FLAG) != 0); // Widget value is between min and max

                    // Drag to left/right edge
                    ctx->MouseMove(label.c_str(), side ? ImGuiTestOpFlags_MoveToEdgeR : ImGuiTestOpFlags_MoveToEdgeL);
                    ctx->MouseUp();

                    // Print values and compare
                    char buf0[32], buf1[32], buf2[32];
                    ImGui::DataTypeFormatString(buf0, 32, data_type, &v.Cur, data_type_info->PrintFmt);
                    ImGui::DataTypeFormatString(buf1, 32, data_type, &v.Min, data_type_info->PrintFmt);
                    ImGui::DataTypeFormatString(buf2, 32, data_type, &v.Max, data_type_info->PrintFmt);
                    ctx->LogInfo("## DataType: %s, Inverted: %d, cur = %s, min = %s, max = %s", data_type_info->Name, invert_range, buf0, buf1, buf2);
                    IM_CHECK((v.ResultFlags & OUT_OF_RANGE_FLAG) == 0); // Widget value does not fall out of range
                    IM_CHECK((v.ResultFlags & MID_OF_RANGE_FLAG) == 0); // Widget value is not between min and max
                    if (side == 0)
                        IM_CHECK(ImGui::DataTypeCompare(data_type, &v.Cur, &v_left) == 0);
                    else
                        IM_CHECK(ImGui::DataTypeCompare(data_type, &v.Cur, &v_right) == 0);
                }
            }
        }

        // Test case where slider knob position calculation would snap to either end of the widget in some cases.
        // Breaks linking with GCC Release mode as those functions in imgui_widgets.cpp gets inlined.
#if 0
        const float s8_ratio_half = ImGui::ScaleRatioFromValueT<ImS32, ImS32, float>(ImGuiDataType_S8, 0, INT8_MIN, INT8_MAX, false, false, false);
        const float u8_ratio_half = ImGui::ScaleRatioFromValueT<ImU32, ImS32, float>(ImGuiDataType_U8, UINT8_MAX / 2, 0, UINT8_MAX, false, false, false);
        const float s8_ratio_half_inv = ImGui::ScaleRatioFromValueT<ImS32, ImS32, float>(ImGuiDataType_S8, 0, INT8_MAX, INT8_MIN, false, false, false);
        const float u8_ratio_half_inv = ImGui::ScaleRatioFromValueT<ImU32, ImS32, float>(ImGuiDataType_U8, UINT8_MAX / 2, UINT8_MAX, 0, false, false, false);
        IM_CHECK(ImAbs(s8_ratio_half - 0.5f) < 0.01f);
        IM_CHECK(ImAbs(u8_ratio_half - 0.5f) < 0.01f);
        IM_CHECK(ImAbs(s8_ratio_half_inv - 0.5f) < 0.01f);
        IM_CHECK(ImAbs(u8_ratio_half_inv - 0.5f) < 0.01f);
#endif
    };

    // ## Test logarithmic slider
    t = IM_REGISTER_TEST(e, "widgets", "widgets_slider_logarithmic");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::SetNextItemWidth(400);
        ImGui::SliderFloat("slider", &vars.Float1, -10.0f, 10.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
        ImGui::Text("%.4f", vars.Float1);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ctx->SetRef("Test Window");
        ctx->ItemClick("slider", 0);
        ctx->Yield();
        IM_CHECK_EQ(vars.Float1, 0.f);

        ctx->ItemClick("slider", 0, ImGuiTestOpFlags_MoveToEdgeR);
        ctx->Yield();
        IM_CHECK_EQ(vars.Float1, 10.f);

        ctx->ItemClick("slider", 0, ImGuiTestOpFlags_MoveToEdgeL);
        ctx->Yield();
        IM_CHECK_EQ(vars.Float1, -10.f);

        // Drag a bit
        ctx->ItemClick("slider", 0);
        float x_offset[] = {50.f,   100.f,  150.f,  190.f};
        float slider_v[] = {0.06f,  0.35f,  2.11f,  8.97f};
        for (float sign : {-1.f, 1.f})
            for (int i = 0; i < IM_ARRAYSIZE(x_offset); i++)
            {
                ctx->ItemDragWithDelta("slider", ImVec2(sign * x_offset[i], 0.f));
                IM_CHECK_GT(vars.Float1, sign * slider_v[i] - (slider_v[i] * 0.1f));  // FIXME-TESTS: Exact values actually depends on GrabSize.
                IM_CHECK_LT(vars.Float1, sign * slider_v[i] + (slider_v[i] * 0.1f));
            }
    };

    // ## Test slider navigation path using keyboard.
    t = IM_REGISTER_TEST(e, "widgets", "widgets_slider_nav");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::SliderFloat("Slider 1", &vars.Float1, -100.0f, 100.0f, "%.2f");
        ImGui::SliderInt("Slider 2", &vars.Int1, 0, 100);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiContext& g = *ImGui::GetCurrentContext();
        ImGuiTestGenericVars& vars = ctx->GenericVars;

        ctx->SetRef("Test Window");

        ctx->ItemNavActivate("Slider 1");
        IM_CHECK_EQ(vars.Float1, 0.f);
        IM_CHECK_EQ(vars.Int1, 0);
        IM_CHECK_EQ(g.NavId, ctx->GetID("Slider 1"));

        // FIXME-TESTS: the step/slow value are technically not exposed and bounds to change. Perhaps this test should only compare the respective magnitude of changes.
        const float slider_float_step = 2.0f;
        const float slider_float_step_slow = 0.2f;
        float slider_float_value = vars.Float1;
        ctx->KeyPress(ImGuiKey_LeftArrow);
        IM_CHECK_EQ(vars.Float1, slider_float_value - slider_float_step);
        ctx->KeyDown(ImGuiKey_None, ImGuiModFlags_Ctrl);
        ctx->KeyPress(ImGuiKey_RightArrow);
        ctx->KeyUp(ImGuiKey_None, ImGuiModFlags_Ctrl);
        IM_CHECK_EQ(vars.Float1, slider_float_value - slider_float_step + slider_float_step_slow);

        ctx->KeyPress(ImGuiKey_DownArrow);
        ctx->KeyPress(ImGuiKey_Space);
        IM_CHECK_EQ(g.NavId, ctx->GetID("Slider 2"));;

        const int slider_int_step = 1;
        const int slider_int_step_slow = 1;
        int slider_int_value = vars.Int1;
        ctx->KeyPress(ImGuiKey_RightArrow);
        IM_CHECK_EQ(vars.Int1, slider_float_value + slider_int_step);
        ctx->KeyDown(ImGuiKey_None, ImGuiModFlags_Ctrl);
        ctx->KeyPress(ImGuiKey_LeftArrow);
        ctx->KeyUp(ImGuiKey_None, ImGuiModFlags_Ctrl);
        IM_CHECK_EQ(vars.Int1, slider_int_value + slider_int_step - slider_int_step_slow);

        ctx->KeyPress(ImGuiKey_UpArrow);
        IM_CHECK_EQ(g.NavId, ctx->GetID("Slider 1"));
    };

    // ## Test whether numbers after format specifier do not influence widget value.
    t = IM_REGISTER_TEST(e, "widgets", "widgets_slider_format");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::SliderInt("Int", &vars.Int1, 10, 100, "22%d00");
        ImGui::SliderInt("Int2", &vars.Int2, 10, 100, "22%'d"); // May not display correct with all printf implementation, but our test stands
        ImGui::SliderFloat("Float", &vars.Float1, 10.0f, 100.0f, "22%.0f00");
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ctx->SetRef("Test Window");
        ctx->ItemClick("Int", 0, ImGuiTestOpFlags_MoveToEdgeR);
        IM_CHECK_EQ(vars.Int1, 100);
        ctx->ItemClick("Int2", 0, ImGuiTestOpFlags_MoveToEdgeR);
        IM_CHECK_EQ(vars.Int2, 100);
        ctx->ItemClick("Float", 0, ImGuiTestOpFlags_MoveToEdgeR);
        IM_CHECK_EQ(vars.Float1, 100.0f);
    };

    // ## Test tooltip positioning in various conditions.
    t = IM_REGISTER_TEST(e, "widgets", "widgets_popup_positioning");
    struct TooltipPosVars { ImVec2 Size = ImVec2(50, 50); };
    t->SetVarsDataType<TooltipPosVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        TooltipPosVars& vars = ctx->GetVars<TooltipPosVars>();

        auto popup_debug_controls = [&]()
        {
            if (ctx->IsGuiFuncOnly())
            {
                float step = ctx->UiContext->IO.DeltaTime * 500.0f;
                if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_UpArrow))) vars.Size.y -= step;
                if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_DownArrow))) vars.Size.y += step;
                if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_LeftArrow))) vars.Size.x -= step;
                if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_RightArrow))) vars.Size.x += step;
            }
        };

        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoNav);
        if (ctx->IsGuiFuncOnly())
            ImGui::DragFloat2("Tooltip Size", &vars.Size.x);
        ImGui::Button("Tooltip", ImVec2(100, 0));
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::InvisibleButton("Space", vars.Size);
            popup_debug_controls();
            ImGui::EndTooltip();
        }
        if (ImGui::Button("Popup", ImVec2(100, 0)))
            ImGui::OpenPopup("Popup");
        if (ImGui::BeginPopup("Popup"))
        {
            ImGui::InvisibleButton("Space", vars.Size);
            popup_debug_controls();
            ImGui::EndPopup();
        }
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiContext& g = *ctx->UiContext;
        TooltipPosVars& vars = ctx->GetVars<TooltipPosVars>();
        ImVec2 viewport_pos = ctx->GetMainMonitorWorkPos();
        ImVec2 viewport_size = ctx->GetMainMonitorWorkSize();

        struct TestCase
        {
            ImVec2 Pos;             // Window position
            ImVec2 Pivot;           // Window position pivot
            ImGuiDir DirSmall;      // Expected default tooltip location
            ImGuiDir DirBigH;       // Expected location when tooltip is as wide as viewport
            ImGuiDir DirBigV;       // Expected location when tooltip is as high as viewport
        };

        // Test tooltip positioning around viewport corners
        static TestCase test_cases[] =
        {
            // [0] Top-left corner
            {
                viewport_pos,
                ImVec2(0.0f, 0.0f),
                ImGuiDir_Right,
                ImGuiDir_Down,
                ImGuiDir_Right,
            },
            // [1] Top edge
            {
                viewport_pos + ImVec2(viewport_size.x * 0.5f, 0.0f),
                ImVec2(0.5f, 0.0f),
                ImGuiDir_Right,
                ImGuiDir_Down,
                ImGuiDir_Right,
            },
            // [2] Top-right corner
            {
                viewport_pos + ImVec2(viewport_size.x, 0.0f),
                ImVec2(1.0f, 0.0f),
                ImGuiDir_Down,
                ImGuiDir_Down,
                ImGuiDir_Left,
            },
            // [3] Right edge
            {
                viewport_pos + ImVec2(viewport_size.x, viewport_size.y * 0.5f),
                ImVec2(1.0f, 0.5f),
                ImGuiDir_Down,
                ImGuiDir_Down,
                ImGuiDir_Left,
            },
            // [4] Bottom-right corner
            {
                viewport_pos + viewport_size,
                ImVec2(1.0f, 1.0f),
                ImGuiDir_Up,
                ImGuiDir_Up,
                ImGuiDir_Left,
            },
            // [5] Bottom edge
            {
                viewport_pos + ImVec2(viewport_size.x * 0.5f, viewport_size.y),
                ImVec2(0.5f, 1.0f),
                ImGuiDir_Right,
                ImGuiDir_Up,
                ImGuiDir_Right,
            },
            // [6] Bottom-left corner
            {
                viewport_pos + ImVec2(0.0f, viewport_size.y),
                ImVec2(0.0f, 1.0f),
                ImGuiDir_Right,
                ImGuiDir_Up,
                ImGuiDir_Right,
            },
            // [7] Left edge
            {
                viewport_pos + ImVec2(0.0f, viewport_size.y * 0.5f),
                ImVec2(0.0f, 0.5f),
                ImGuiDir_Right,
                ImGuiDir_Down,
                ImGuiDir_Right,
            },
        };

        ctx->SetRef("Test Window");

        for (int variant = 0; variant < 2; variant++)
        {
            const char* button_name = variant ? "Popup" : "Tooltip";
            ctx->LogInfo("## Test variant: %s", button_name);
            ctx->ItemClick(button_name);        // Force tooltip creation so we can grab the pointer
            ImGuiWindow* tooltip = variant ? g.NavWindow : ctx->GetWindowByRef("##Tooltip_00");

            for (auto& test_case : test_cases)
            {
                ctx->LogInfo("## Test case %d", (int)(&test_case - test_cases));
                vars.Size = ImVec2(50, 50);
                ctx->WindowMove("", test_case.Pos, test_case.Pivot);
                ctx->ItemClick(button_name);

                // Check default tooltip location
                IM_CHECK_EQ(g.HoveredIdPreviousFrame, ctx->GetID(button_name));
                IM_CHECK_EQ(tooltip->AutoPosLastDirection, test_case.DirSmall);

                // Check tooltip location when it is real wide and verify that location does not change once it becomes too wide
                // First iteration: tooltip is just wide enough to fit within viewport
                // First iteration: tooltip is wider than viewport
                for (int j = 0; j < 2; j++)
                {
                    vars.Size = ImVec2((j * 0.25f * viewport_size.x) + (viewport_size.x - (g.Style.WindowPadding.x + g.Style.DisplaySafeAreaPadding.x) * 2), 50);
                    ctx->ItemClick(button_name);
                    IM_CHECK(tooltip->AutoPosLastDirection == test_case.DirBigH);
                }

                // Check tooltip location when it is real tall and verify that location does not change once it becomes too tall
                // First iteration: tooltip is just tall enough to fit within viewport
                // First iteration: tooltip is taller than viewport
                for (int j = 0; j < 2; j++)
                {
                    vars.Size = ImVec2(50, (j * 0.25f * viewport_size.x) + (viewport_size.y - (g.Style.WindowPadding.y + g.Style.DisplaySafeAreaPadding.y) * 2));
                    ctx->ItemClick(button_name);
                    IM_CHECK(tooltip->AutoPosLastDirection == test_case.DirBigV);
                }

                IM_CHECK_EQ(g.HoveredIdPreviousFrame, ctx->GetID(button_name));
            }
        }
    };

    // ## Test disabled items setting g.HoveredId and taking clicks.
#if (IMGUI_VERSION_NUM >= 18310)
    t = IM_REGISTER_TEST(e, "widgets", "widgets_disabled");
    struct WidgetsDisabledVars
    {
        bool WidgetsDisabled;
        bool Activated[4];
        bool Hovered[4];
        bool HoveredDisabled[4];
        void Reset() { memset(this, 0, sizeof(*this)); }
        WidgetsDisabledVars() { Reset(); }
    };
    t->SetVarsDataType<WidgetsDisabledVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        WidgetsDisabledVars& vars = ctx->GetVars<WidgetsDisabledVars>();
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_MenuBar);
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("Menu 1"))
            {
                if (ImGui::BeginMenu("Menu Enabled"))
                {
                    ImGui::MenuItem("Item");
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Menu Disabled", false))
                {
                    ImGui::MenuItem("Item");
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        ImGui::Selectable("Enabled A");
        int index = 0;

        if (vars.WidgetsDisabled)
            ImGui::BeginDisabled();

        vars.Activated[index] |= ImGui::Button("Button");
        vars.Hovered[index] |= ImGui::IsItemHovered();
        vars.HoveredDisabled[index] |= ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled);
        if (ctx->FrameCount == 0)
            IM_CHECK_EQ(ImGui::GetItemID(), ImGui::GetID("Button"));      // Make sure widget has an ID
        index++;

        float f = 0.0f;
        vars.Activated[index] |= ImGui::DragFloat("DragFloat", &f);
        vars.Hovered[index] |= ImGui::IsItemHovered();
        vars.HoveredDisabled[index] |= ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled);
        if (ctx->FrameCount == 0)
            IM_CHECK_EQ(ImGui::GetItemID(), ImGui::GetID("DragFloat"));     // Make sure widget has an ID
        index++;

        vars.Activated[index] |= ImGui::Selectable("Selectable", false, 0);
        vars.Hovered[index] |= ImGui::IsItemHovered();
        vars.HoveredDisabled[index] |= ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled);
        if (ctx->FrameCount == 0)
            IM_CHECK_EQ(ImGui::GetItemID(), ImGui::GetID("Selectable"));    // Make sure widget has an ID
        index++;

        if (vars.WidgetsDisabled)
            ImGui::EndDisabled();

        vars.Activated[index] |= ImGui::Selectable("SelectableFlag", false, vars.WidgetsDisabled ? ImGuiSelectableFlags_Disabled : 0);
        vars.Hovered[index] |= ImGui::IsItemHovered();
        vars.HoveredDisabled[index] |= ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled);
        if (ctx->FrameCount == 0)
            IM_CHECK_EQ(ImGui::GetItemID(), ImGui::GetID("SelectableFlag"));    // Make sure widget has an ID
        index++;

        ImGui::Selectable("Enabled B");

        // Simple nested check (#4655)
        ImGuiStyle& style = ImGui::GetStyle();
        const float alpha0 = style.Alpha;
        ImGui::BeginDisabled();
        IM_CHECK_EQ(alpha0 * style.DisabledAlpha, style.Alpha);
        ImGui::BeginDisabled();
        IM_CHECK_EQ(alpha0 * style.DisabledAlpha, style.Alpha);
        ImGui::EndDisabled();
        IM_CHECK_EQ(alpha0 * style.DisabledAlpha, style.Alpha);
        ImGui::EndDisabled();
        IM_CHECK_EQ(alpha0, style.Alpha);

        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiContext& g = *ctx->UiContext;
        WidgetsDisabledVars& vars = ctx->GetVars<WidgetsDisabledVars>();
        const char* disabled_items[] = { "Button", "DragFloat", "Selectable", "SelectableFlag" }; // Matches order of arrays in WidgetsDisabledVars.

        ImGuiWindow* window = ctx->GetWindowByRef("Test Window");
        ctx->SetRef("Test Window");
        vars.WidgetsDisabled = true;

        // Navigating over disabled menu.
        ctx->MenuAction(ImGuiTestAction_Hover, "Menu 1/Menu Enabled/Item");
        IM_CHECK(g.NavWindow != NULL);
        IM_CHECK_STR_EQ(g.NavWindow->Name, "##Menu_01");
        ctx->MenuAction(ImGuiTestAction_Hover, "Menu 1/Menu Disabled");
        IM_CHECK(g.NavWindow != NULL);
        IM_CHECK_STR_EQ(g.NavWindow->Name, "##Menu_00");

        // Navigating over a disabled item.
        ctx->ItemClick("Enabled A");
        IM_CHECK_EQ(g.NavId, ctx->GetID("Enabled A"));
        ctx->KeyPress(ImGuiKey_DownArrow);
        IM_CHECK_EQ(g.NavId, ctx->GetID("Enabled B"));              // Make sure we have skipped disabled items.
        ctx->KeyPress(ImGuiKey_Escape);

        // Clicking a disabled item.
        for (int i = 0; i < IM_ARRAYSIZE(disabled_items); i++)
        {
            ctx->MouseMove(disabled_items[i]);
            vars.Reset();
            vars.WidgetsDisabled = true;
            ctx->ItemClick(disabled_items[i]);
            IM_CHECK(vars.Activated[i] == false);                   // Was not clicked because button is disabled.
            IM_CHECK(vars.Hovered[i] == false);                     // Wont report as being hovered because button is disabled.
            IM_CHECK(vars.HoveredDisabled[i] == true);              // IsItemHovered with ImGuiHoveredFlags_AllowWhenDisabled.
            IM_CHECK(g.HoveredId == ctx->GetID(disabled_items[i])); // Will set HoveredId even when disabled.
        }

        // Dragging a disabled item.
        ImVec2 window_pos = window->Pos;
        for (int i = 0; i < IM_ARRAYSIZE(disabled_items); i++)
        {
            ctx->MouseMove(disabled_items[i]);
            vars.Reset();
            vars.WidgetsDisabled = true;
            ctx->ItemDragWithDelta(disabled_items[i], ImVec2(30, 0));
            IM_CHECK(window_pos == window->Pos);                    // Disabled items consume click events.
        }

        // Disable active ID when widget gets disabled while held.
        for (int i = 0; i < IM_ARRAYSIZE(disabled_items); i++)
        {
            vars.WidgetsDisabled = false;
            ctx->MouseMove(disabled_items[i]);
            ctx->MouseDown();
            IM_CHECK(g.ActiveId == ctx->GetID(disabled_items[i]));  // Enabled item is active.
            vars.WidgetsDisabled = true;
            ctx->Yield();
            IM_CHECK(g.ActiveId == 0);                              // Disabling item while it is active deactivates it.
            ctx->MouseUp();
        }
    };
#endif

    // ## Test BeginDisabled()/EndDisabled()
#if (IMGUI_VERSION_NUM >= 18405)
    t = IM_REGISTER_TEST(e, "widgets", "widgets_disabled_2");
    struct BeginDisabledVars
    {
        struct BeginDisabledItemInfo
        {
            const char* Name = NULL;
            ImGuiTestGenericItemStatus Status = {};
            ImGuiItemFlags FlagsBegin = 0;
            ImGuiItemFlags FlagsEnd = 0;
            float AlphaBegin = 0;
            float AlphaEnd = 0;

            BeginDisabledItemInfo(const char* name) { Name = name; }
        };
        BeginDisabledItemInfo ButtonInfo[6] = { "A", "B", "C", "D", "E", "F" };
    };
    t->SetVarsDataType<BeginDisabledVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiContext& g = *ctx->UiContext;
        BeginDisabledVars& vars = ctx->GetVars<BeginDisabledVars>();
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_MenuBar);

        int index = 0;
        auto begin_disabled = [&](bool disabled = true)
        {
            auto& button_info = vars.ButtonInfo[index];
            button_info.FlagsBegin = g.CurrentItemFlags;
            button_info.AlphaBegin = g.DisabledAlphaBackup;
            ImGui::BeginDisabled(disabled);
            bool flag_enabled = (index % 2) == 0;
            ImGui::PushItemFlag(ImGuiItemFlags_NoNav, flag_enabled);    // Add a random flag to try mix things up.
            index++;
        };
        auto end_disabled = [&]()
        {
            index--;
            auto& button_info = vars.ButtonInfo[index];
            ImGui::PopItemFlag();                                       // ImGuiItemFlags_NoNav
            ImGui::EndDisabled();
            button_info.FlagsEnd = g.CurrentItemFlags;
            button_info.AlphaEnd = g.DisabledAlphaBackup;
        };

        begin_disabled();
        vars.ButtonInfo[index].Status.QueryInc(ImGui::Button("A"));
        begin_disabled();
        vars.ButtonInfo[index].Status.QueryInc(ImGui::Button("B"));
        end_disabled();
        begin_disabled(false);
        vars.ButtonInfo[index].Status.QueryInc(ImGui::Button("C"));
        end_disabled();
        vars.ButtonInfo[index].Status.QueryInc(ImGui::Button("D"));
        end_disabled();

        begin_disabled();
        bool ret = ImGui::Button("E");
        end_disabled();
        vars.ButtonInfo[4].Status.QueryInc(ret);

        ImGui::BeginDisabled(false);
        vars.ButtonInfo[5].Status.QueryInc(ImGui::Button("F"));
        ImGui::EndDisabled();
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiContext& g = *ctx->UiContext;
        BeginDisabledVars& vars = ctx->GetVars<BeginDisabledVars>();
        ctx->SetRef("Test Window");
        for (int i = 0; i < 5; i++)
        {
            auto& button_info = vars.ButtonInfo[i];
            ctx->LogDebug("Button %s", button_info.Name);
            ctx->ItemClick(button_info.Name);
            IM_CHECK(button_info.Status.Ret == 0);                      // No clicks
            IM_CHECK(button_info.Status.Clicked == 0);
            IM_CHECK(g.HoveredId == ctx->GetID(button_info.Name));      // HoveredId is set
            IM_CHECK(button_info.FlagsBegin == button_info.FlagsEnd);   // Flags and Alpha match between Begin/End calls
            IM_CHECK(button_info.AlphaBegin == button_info.AlphaEnd);
        }
        ctx->ItemClick("E");
        IM_CHECK(vars.ButtonInfo[4].Status.Hovered == 0);               // Ensure we rely on last item storage, not current state
        ctx->ItemClick("F");
        IM_CHECK(vars.ButtonInfo[5].Status.Ret == 1);                   // BeginDisabled(false) does not prevent clicks
        IM_CHECK(vars.ButtonInfo[5].Status.Clicked == 1);
    };
#endif

    // ## Test SetItemUsingMouseWheel() preventing scrolling.
    t = IM_REGISTER_TEST(e, "widgets", "widgets_item_using_mouse_wheel");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGui::SetNextWindowSize(ImVec2(0, 100), ImGuiCond_Always);
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysVerticalScrollbar);
        ImGui::Button("You! Shall! Not! Scroll!");
        ImGui::SetItemUsingMouseWheel();
        for (int i = 0; i < 10; i++)
            ImGui::Text("Line %d", i + 1);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ctx->SetRef("Test Window");
        ctx->ScrollToTop("");
        ImGuiWindow* window = ctx->GetWindowByRef("");
        IM_CHECK(window->Scroll.y == 0.0f);
        ctx->MouseMove("You! Shall! Not! Scroll!");
        ctx->MouseWheelY(-10.0f);
        IM_CHECK(window->Scroll.y == 0.0f);
    };

    // ## Test Splitter().
    t = IM_REGISTER_TEST(e, "widgets", "widgets_splitter");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_Always);
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        ImVec2& child_size = vars.Size;
        ImGuiAxis axis = (ImGuiAxis)ctx->Test->ArgVariant;

        ImGui::Splitter("splitter", &child_size.x, &child_size.y, axis, +1);

        if (ImGui::BeginChild("Child 1", ImVec2(axis == ImGuiAxis_X ? child_size.x : 0.0f, axis == ImGuiAxis_Y ? child_size.x : 0.0f)))
            ImGui::TextUnformatted("Child 1");
        ImGui::EndChild();

        if (ImGui::BeginChild("Child 2", ImVec2(axis == ImGuiAxis_X ? child_size.y : 0.0f, axis == ImGuiAxis_Y ? child_size.y : 0.0f)))
            ImGui::TextUnformatted("Child 1");
        ImGui::EndChild();

        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGuiWindow* child1 = ctx->GetWindowByRef(ctx->GetChildWindowID("Test Window", "Child 1"));
        ImGuiWindow* child2 = ctx->GetWindowByRef(ctx->GetChildWindowID("Test Window", "Child 2"));
        ctx->SetRef("Test Window");
        ImVec2& child_size = vars.Size;
        for (int axis = 0; axis < 2; axis++)
        {
            ctx->LogDebug("Axis: ImGuiAxis_%s", axis ? "Y" : "X");
            ctx->Test->ArgVariant = axis;
            child_size = ImVec2(100, 100);
            for (int i = -1; i < 1; i++)
            {
                ctx->ItemDragWithDelta("splitter", ImVec2(50.0f * i, 50.0f * i));
                IM_CHECK_EQ(axis == ImGuiAxis_X ? child1->Size.x + child2->Size.x : child1->Size.y + child2->Size.y, child_size.x + child_size.y);
            }
        }
    };

#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    // ## Test legacy float format string patching in DragInt().
    t = IM_REGISTER_TEST(e, "widgets", "widgets_dragint_float_format");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        if (ctx->IsFirstGuiFrame())
            vars.Int1 = 123;
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::LogToBuffer();
        ImGui::DragInt("%f", &vars.Int1, 1, 0, 0, "%f");
        ImGui::DragInt("%.0f", &vars.Int1, 1, 0, 0, "%.0f");
        ImGui::DragInt("%.3f", &vars.Int1, 1, 0, 0, "%.3f");
        ImStrncpy(vars.Str1, ctx->UiContext->LogBuffer.c_str(), IM_ARRAYSIZE(vars.Str2));
        ImGui::LogFinish();
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        IM_CHECK_STR_EQ(vars.Str1, "{ 123 } %f" IM_NEWLINE "{ 123 } %.0f" IM_NEWLINE "{ 123 } %.3f");
    };
#endif
}
