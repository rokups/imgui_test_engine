// dear imgui
// (test engine, exporters)

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif
#include "imgui_te_exporters.h"
#include "imgui_te_engine.h"
#include "imgui_te_internal.h"
#include "thirdparty/Str/Str.h"

//-------------------------------------------------------------------------
// [SECTION] CONSTANTS
//-------------------------------------------------------------------------

static const double SLOW_TEST_SECONDS = 5.0;    // FIXME: This could be a command line parameter.

//-------------------------------------------------------------------------
// [SECTION] FORWARD DECLARATIONS
//-------------------------------------------------------------------------

static void ImGuiTestEngine_ExportJUnitXml(ImGuiTestEngine* engine, const char* output_file, const char* description);
static void ImGuiTestEngine_ExportMarkdown(ImGuiTestEngine* engine, const char* output_file, const char* description, bool verbose);

//-------------------------------------------------------------------------
// [SECTION] TEST ENGINE EXPORTER FUNCTIONS
//-------------------------------------------------------------------------
// - ImGuiTestEngine_PrintResultSummary()
// - ImGuiTestEngine_Export()
// - ImGuiTestEngine_ExportEx()
// - ImGuiTestEngine_ExportJUnitXml()
// - ImGuiTestEngine_ExportMarkdown()
//-------------------------------------------------------------------------

void ImGuiTestEngine_PrintResultSummary(ImGuiTestEngine* engine)
{
    int count_tested = 0;
    int count_success = 0;
    ImGuiTestEngine_GetResult(engine, count_tested, count_success);

    if (count_success < count_tested)
    {
        printf("\nFailing tests:\n");
        for (ImGuiTest* test : engine->TestsAll)
            if (test->Status == ImGuiTestStatus_Error)
                printf("- %s\n", test->Name);
    }

    ImOsConsoleSetTextColor(ImOsConsoleStream_StandardOutput, (count_success == count_tested) ? ImOsConsoleTextColor_BrightGreen : ImOsConsoleTextColor_BrightRed);
    printf("\nTests Result: %s\n", (count_success == count_tested) ? "OK" : "Errors");
    printf("(%d/%d tests passed)\n", count_success, count_tested);
    ImOsConsoleSetTextColor(ImOsConsoleStream_StandardOutput, ImOsConsoleTextColor_White);
}

// This is mostly a copy of ImGuiTestEngine_PrintResultSummary with few additions.
static void ImGuiTestEngine_ExportResultSummary(ImGuiTestEngine* engine, FILE* fp, int indent_count, ImGuiTestGroup group)
{
    int count_tested = 0;
    int count_success = 0;

    for (ImGuiTest* test : engine->TestsAll)
    {
        if (test->Group != group)
            continue;
        if (test->Status != ImGuiTestStatus_Unknown)
            count_tested++;
        if (test->Status == ImGuiTestStatus_Success)
            count_success++;
    }

    Str64 indent_str;
    indent_str.reserve(indent_count + 1);
    memset(indent_str.c_str(), ' ', indent_count);
    indent_str[indent_count] = 0;
    const char* indent = indent_str.c_str();

    if (count_success < count_tested)
    {
        fprintf(fp, "\n%sFailing tests:\n", indent);
        for (ImGuiTest* test : engine->TestsAll)
        {
            if (test->Group != group)
                continue;
            if (test->Status == ImGuiTestStatus_Error)
                fprintf(fp, "%s- %s\n", indent, test->Name);
        }
        fprintf(fp, "\n");
    }

    fprintf(fp, "%sTests Result: %s\n", indent, (count_success == count_tested) ? "OK" : "Errors");
    fprintf(fp, "%s(%d/%d tests passed)\n", indent, count_success, count_tested);
}

static bool ImGuiTestEngine_HasAnyLogLines(ImGuiTestLog* test_log, ImGuiTestVerboseLevel level)
{
    for (auto& line_info : test_log->LineInfo)
        if (line_info.Level <= level)
            return true;
    return false;
}

static void ImGuiTestEngine_PrintLogLines(FILE* fp, ImGuiTestLog* test_log, int indent, ImGuiTestVerboseLevel level, const char* new_line = "\n")
{
    Str128 log_line;
    for (auto& line_info : test_log->LineInfo)
    {
        if (line_info.Level > level)
            continue;
        const char* line_start = test_log->Buffer.c_str() + line_info.LineOffset;
        const char* line_end = strstr(line_start, "\n"); // FIXME: Incorrect.
        log_line.set(line_start, line_end);
        ImStrXmlEscape(&log_line); // FIXME: Should not be here considering the function name.
        for (int i = 0; i < indent; i++)
            fprintf(fp, " ");
        fprintf(fp, "%s%s", log_line.c_str(), new_line);
    }
}

void ImGuiTestEngine_Export(ImGuiTestEngine* engine)
{
    ImGuiTestEngineIO& io = engine->IO;
    ImGuiTestEngine_ExportEx(engine, io.ExportResultsFormat, io.ExportResultsFilename, io.ExportResultsDescription);
}

void ImGuiTestEngine_ExportEx(ImGuiTestEngine* engine, ImGuiTestEngineExportFormat format, const char* filename, const char* description)
{
    if (format == ImGuiTestEngineExportFormat_None)
        return;
    IM_ASSERT(filename != NULL);

    if (format == ImGuiTestEngineExportFormat_JUnitXml)
        ImGuiTestEngine_ExportJUnitXml(engine, filename, description);
    else if (format == ImGuiTestEngineExportFormat_Markdown || format == ImGuiTestEngineExportFormat_MarkdownMinimal)
        ImGuiTestEngine_ExportMarkdown(engine, filename, description, format == ImGuiTestEngineExportFormat_Markdown);
    else
        IM_ASSERT(0);
}

// Per-testsuite test statistics.
struct ImGuiTestGroupStatistics
{
    const char* Name     = NULL;
    int         Tests    = 0;
    int         Passed   = 0;
    int         Failures = 0;
    int         Disabled = 0;
    int         LogOutput = 0;
    int         SlowTests = 0;
    ImU64       TimeElapsed = 0;    // Microseconds
};

static void ImGuiTestEngine_GetTestSuiteStatistics(ImGuiTestEngine* engine, ImGuiTestGroupStatistics* testsuites)
{
    testsuites[ImGuiTestGroup_Tests].Name = "tests";
    testsuites[ImGuiTestGroup_Perfs].Name = "perfs";
    for (int n = 0; n < engine->TestsAll.Size; n++)
    {
        ImGuiTest* test = engine->TestsAll[n];
        auto* stats = &testsuites[test->Group];
        stats->Tests += 1;
        stats->TimeElapsed += test->EndTime - test->StartTime;
        if (test->Status == ImGuiTestStatus_Error)
        {
            stats->Failures += 1;
            if (ImGuiTestEngine_HasAnyLogLines(&test->TestLog, engine->IO.ConfigVerboseLevelOnError))
                stats->LogOutput += 1;
        }
        else if (ImGuiTestEngine_HasAnyLogLines(&test->TestLog, engine->IO.ConfigVerboseLevel))
        {
            stats->LogOutput += 1;
        }

        double duration_seconds = (double)(test->EndTime - test->StartTime) / 1000000.0;
        if (duration_seconds > SLOW_TEST_SECONDS)
            stats->SlowTests++;

        if (test->Status == ImGuiTestStatus_Success)
            stats->Passed += 1;
        else if (test->Status == ImGuiTestStatus_Unknown)
            stats->Disabled += 1;
    }
}
static void FormatSecondsToTimespanString(double seconds, Str* out)
{
    double minutes = (double)(int)(seconds / 60.0);
    seconds -= minutes * 60.0;
    if (minutes >= 1.0)
        out->appendf("%dm", (int)minutes);
    out->appendf("%.3fs", (float)seconds);
}

void ImGuiTestEngine_ExportJUnitXml(ImGuiTestEngine* engine, const char* output_file, const char* description)
{
    IM_ASSERT(engine != NULL);
    IM_ASSERT(output_file != NULL);

    FILE* fp = fopen(output_file, "w+b");
    if (fp == NULL)
    {
        fprintf(stderr, "Writing '%s' failed.\n", output_file);
        return;
    }

    ImGuiTestGroupStatistics testsuites[ImGuiTestGroup_COUNT];
    ImGuiTestEngine_GetTestSuiteStatistics(engine, testsuites);

    // Attributes for <testsuites> tag.
    int testsuites_failures = 0;
    int testsuites_tests = 0;
    int testsuites_disabled = 0;
    float testsuites_time = (float)((double)(engine->EndTime - engine->StartTime) / 1000000.0);
    for (int testsuite_id = ImGuiTestGroup_Tests; testsuite_id < ImGuiTestGroup_COUNT; testsuite_id++)
    {
        testsuites_tests += testsuites[testsuite_id].Tests;
        testsuites_failures += testsuites[testsuite_id].Failures;
        testsuites_disabled += testsuites[testsuite_id].Disabled;
    }

    // FIXME: "errors" attribute and <error> tag in <testcase> may be supported if we have means to catch unexpected errors like assertions.
    fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<testsuites disabled=\"%d\" errors=\"0\" failures=\"%d\" name=\"%s\" tests=\"%d\" time=\"%.3f\">\n",
        testsuites_disabled, testsuites_failures, description ? description : "Dear ImGui", testsuites_tests, testsuites_time);

    const char* teststatus_names[] = { "skipped", "success", "queued", "running", "error", "suspended" };
    for (int testsuite_id = ImGuiTestGroup_Tests; testsuite_id < ImGuiTestGroup_COUNT; testsuite_id++)
    {
        // Attributes for <testsuite> tag.
        ImGuiTestGroupStatistics* testsuite = &testsuites[testsuite_id];
        float testsuite_time = testsuites_time;         // FIXME: We do not differentiate between tests and perfs, they are executed in one big batch.
        Str30 testsuite_timestamp = "";
        ImTimestampToISO8601(engine->StartTime, &testsuite_timestamp);
        fprintf(fp, "  <testsuite name=\"%s\" tests=\"%d\" disabled=\"%d\" errors=\"0\" failures=\"%d\" hostname=\"\" id=\"%d\" package=\"\" skipped=\"0\" time=\"%.3f\" timestamp=\"%s\">\n",
            testsuite->Name, testsuite->Tests, testsuite->Disabled, testsuite->Failures, testsuite_id, testsuite_time, testsuite_timestamp.c_str());

        for (int n = 0; n < engine->TestsAll.Size; n++)
        {
            ImGuiTest* test = engine->TestsAll[n];
            if (test->Group != testsuite_id)
                continue;

            // Attributes for <testcase> tag.
            const char* testcase_name = test->Name;
            const char* testcase_classname = test->Category;
            const char* testcase_status = teststatus_names[test->Status + 1];   // +1 because _Unknown status is -1.
            float testcase_time = (float)((double)(test->EndTime - test->StartTime) / 1000000.0);

            fprintf(fp, "    <testcase name=\"%s\" assertions=\"0\" classname=\"%s\" status=\"%s\" time=\"%.3f\">\n",
                testcase_name, testcase_classname, testcase_status, testcase_time);

            if (test->Status == ImGuiTestStatus_Error)
            {
                // Skip last error message because it is generic information that test failed.
                Str128 log_line;
                for (int i = test->TestLog.LineInfo.Size - 2; i >= 0; i--)
                {
                    ImGuiTestLogLineInfo* line_info = &test->TestLog.LineInfo[i];
                    if (line_info->Level > engine->IO.ConfigVerboseLevelOnError)
                        continue;
                    if (line_info->Level == ImGuiTestVerboseLevel_Error)
                    {
                        const char* line_start = test->TestLog.Buffer.c_str() + line_info->LineOffset;
                        const char* line_end = strstr(line_start, "\n");
                        log_line.set(line_start, line_end);
                        ImStrXmlEscape(&log_line);
                        break;
                    }
                }

                // Failing tests save their "on error" log output in text element of <failure> tag.
                fprintf(fp, "      <failure message=\"%s\" type=\"error\">\n", log_line.c_str());
                ImGuiTestEngine_PrintLogLines(fp, &test->TestLog, 8, engine->IO.ConfigVerboseLevelOnError);
                fprintf(fp, "      </failure>\n");
            }

            if (test->Status == ImGuiTestStatus_Unknown)
            {
                fprintf(fp, "      <skipped message=\"Skipped\" />\n");
            }
            else
            {
                // Succeeding tests save their defaiult log output output as "stdout".
                if (ImGuiTestEngine_HasAnyLogLines(&test->TestLog, engine->IO.ConfigVerboseLevel))
                {
                    fprintf(fp, "      <system-out>\n");
                    ImGuiTestEngine_PrintLogLines(fp, &test->TestLog, 8, engine->IO.ConfigVerboseLevel);
                    fprintf(fp, "      </system-out>\n");
                }

                // Save error messages as "stderr".
                if (ImGuiTestEngine_HasAnyLogLines(&test->TestLog, ImGuiTestVerboseLevel_Error))
                {
                    fprintf(fp, "      <system-err>\n");
                    ImGuiTestEngine_PrintLogLines(fp, &test->TestLog, 8, ImGuiTestVerboseLevel_Error);
                    fprintf(fp, "      </system-err>\n");
                }
            }
            fprintf(fp, "    </testcase>\n");
        }

        if (testsuites[testsuite_id].Disabled < testsuites[testsuite_id].Tests) // Any tests executed
        {
            // Log all log messages as "stdout".
            fprintf(fp, "    <system-out>\n");
            for (int n = 0; n < engine->TestsAll.Size; n++)
            {
                ImGuiTest* test = engine->TestsAll[n];
                if (test->Group != testsuite_id)
                    continue;
                if (test->Status == ImGuiTestStatus_Unknown)
                    continue;
                fprintf(fp, "      [0000] Test: '%s' '%s'..\n", test->Category, test->Name);
                ImGuiTestVerboseLevel level = test->Status == ImGuiTestStatus_Error ? engine->IO.ConfigVerboseLevelOnError : engine->IO.ConfigVerboseLevel;
                ImGuiTestEngine_PrintLogLines(fp, &test->TestLog, 6, level);
            }
            ImGuiTestEngine_ExportResultSummary(engine, fp, 6, (ImGuiTestGroup)testsuite_id);
            fprintf(fp, "    </system-out>\n");

            // Log all warning and error messages as "stderr".
            fprintf(fp, "    <system-err>\n");
            for (int n = 0; n < engine->TestsAll.Size; n++)
            {
                ImGuiTest* test = engine->TestsAll[n];
                if (test->Group != testsuite_id)
                    continue;
                if (test->Status == ImGuiTestStatus_Unknown)
                    continue;
                fprintf(fp, "      [0000] Test: '%s' '%s'..\n", test->Category, test->Name);
                ImGuiTestEngine_PrintLogLines(fp, &test->TestLog, 6, ImGuiTestVerboseLevel_Warning);
            }
            ImGuiTestEngine_ExportResultSummary(engine, fp, 6, (ImGuiTestGroup)testsuite_id);
            fprintf(fp, "    </system-err>\n");
        }
        fprintf(fp, "  </testsuite>\n");
    }
    fprintf(fp, "</testsuites>\n");
    fclose(fp);
    fprintf(stdout, "Saved test results to '%s' successfully.\n", output_file);
}

void ImGuiTestEngine_ExportMarkdown(ImGuiTestEngine* engine, const char* output_file, const char* description, bool verbose)
{
    IM_ASSERT(engine != NULL);
    IM_ASSERT(output_file != NULL);

    ImGuiTestGroupStatistics testsuites[ImGuiTestGroup_COUNT];
    ImGuiTestEngine_GetTestSuiteStatistics(engine, testsuites);
    bool has_useful_information = false;
    for (int testsuite_id = ImGuiTestGroup_Tests; testsuite_id < ImGuiTestGroup_COUNT; testsuite_id++)
    {
        ImGuiTestGroupStatistics* testsuite = &testsuites[testsuite_id];
        has_useful_information |= testsuite->LogOutput > 0 || testsuite->Failures > 0 || testsuite->SlowTests > 0;
    }

    if (!has_useful_information)
        return; // All tests succeeded and did not produce any log output (according to configured verbosity level).

    FILE* fp = fopen(output_file, "w+b");
    if (fp == NULL)
    {
        fprintf(stderr, "Writing '%s' failed.\n", output_file);
        return;
    }

    fprintf(fp, "# Test results for: %s\n\n", description);
    fprintf(fp, "| Status | Group | Test     | Duration | Message       |\n");
    fprintf(fp, "| ------ | ----- | -------- | -------- | ------------- |\n");

    for (int n = 0; n < engine->TestsAll.Size; n++)
    {
        ImGuiTest* test = engine->TestsAll[n];
        ImGuiTestVerboseLevel verbosity_level = engine->IO.ConfigVerboseLevel;
        const char* status = "\xe2\x80\x94";        // U+2014, Em Dash
        if (test->Status == ImGuiTestStatus_Error)
        {
            status = "\xf0\x9f\x94\xb4";    // U+1F534, Large Red Circle
            verbosity_level = engine->IO.ConfigVerboseLevelOnError;
        }
        else if (test->Status == ImGuiTestStatus_Success)
        {
            if (ImGuiTestEngine_HasAnyLogLines(&test->TestLog, ImGuiTestVerboseLevel_Warning))
                status = "\xe2\x9a\xa0";    // U+26A0, Warning Sign
            else
                status = "\xe2\x9c\x85";    // U+2705, White Heavy Check Mark
        }

        // In non-verbose mode skip passing/skipped tests with no log messages.
        double duration_seconds = (double)(test->EndTime - test->StartTime) / 1000000.0;
        if (!verbose && (test->Status == ImGuiTestStatus_Success || test->Status == ImGuiTestStatus_Unknown))
            if (duration_seconds < SLOW_TEST_SECONDS && !ImGuiTestEngine_HasAnyLogLines(&test->TestLog, verbosity_level))
                continue;

        Str16 duration_str;
        FormatSecondsToTimespanString(duration_seconds, &duration_str);

        fprintf(fp, "| %s | %s | %s | %s | ", status, testsuites[test->Group].Name, test->Name, duration_str.c_str());
        ImGuiTestEngine_PrintLogLines(fp, &test->TestLog, 0, verbosity_level, "<br>");
        fprintf(fp, " |\n");
    }
    fprintf(fp, "\n");

    fclose(fp);
    fprintf(stdout, "Saved test results to '%s' successfully.\n", output_file);
}
