#include "FormDesignerWindow.h"
#include <imgui.h>

HWND FormDesignerWindow::g_lazarusWindow = nullptr;
PROCESS_INFORMATION FormDesignerWindow::g_lazarusProcess = {};
HWND FormDesignerWindow::g_mainApplicationWindow = nullptr;

// Track previous state
static RECT previousRect = { 0, 0, 0, 0 };
static WINDOWPLACEMENT mainAppPlacement = { sizeof(WINDOWPLACEMENT) };

void FormDesignerWindow::Render()
{
    ImGui::Begin("Lazarus Container");

    // Get current position and size of the ImGui window
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();

    if (g_lazarusWindow)
    {
        // Synchronize Lazarus with the main application's minimize/maximize/restore actions
        SynchronizeWindowState();

        // Calculate bounds for Lazarus
        RECT newRect = {
            static_cast<LONG>(windowPos.x),
            static_cast<LONG>(windowPos.y),
            static_cast<LONG>(windowPos.x + windowSize.x),
            static_cast<LONG>(windowPos.y + windowSize.y)
        };

        if (!EqualRect(&previousRect, &newRect))
        {
            previousRect = newRect;

            // Ensure Lazarus is embedded and always on top of the application
            HWND currentParent = GetParent(g_lazarusWindow);
            if (currentParent != g_mainApplicationWindow)
            {
                SetParent(g_lazarusWindow, g_mainApplicationWindow);
            }

            SetWindowPos(
                g_lazarusWindow,
                HWND_TOPMOST,                   // Ensure Lazarus stays above your application
                newRect.left,
                newRect.top,
                newRect.right - newRect.left,
                newRect.bottom - newRect.top,
                SWP_NOACTIVATE | SWP_SHOWWINDOW
            );

            // Remove the Lazarus title bar and borders
            LONG currentStyle = GetWindowLong(g_lazarusWindow, GWL_STYLE);
            currentStyle &= ~WS_EX_APPWINDOW;
            currentStyle |= WS_EX_TOOLWINDOW;
            if ((currentStyle & WS_CAPTION) || (currentStyle & WS_THICKFRAME))
            {
                SetWindowLong(g_lazarusWindow, GWL_STYLE, currentStyle & ~(WS_CAPTION | WS_THICKFRAME));
                SetWindowPos(g_lazarusWindow, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
            }
        }
    }
    else
    {
        // Button to launch Lazarus
        if (ImGui::Button("Launch Lazarus"))
        {
            LaunchLazarus();
        }
    }

    ImGui::End();
}

void FormDesignerWindow::LaunchLazarus()
{
    // Launch Lazarus IDE
    STARTUPINFO si = { sizeof(STARTUPINFO) };
    if (!CreateProcessA(lazarusPath.c_str(), NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &g_lazarusProcess))
    {
        std::cerr << "Failed to start Lazarus. Error: " << GetLastError() << std::endl;
        return;
    }

    // Wait for Lazarus to appear
    Sleep(2000);

    // Find the main Lazarus window
    g_lazarusWindow = nullptr;
    EnumWindows(EnumWindowsProc, 0);

    if (!g_lazarusWindow)
    {
        OutputDebugString("No Lazarus main window found!");
        return;
    }
}

// Static callback function to find Lazarus main window
BOOL CALLBACK FormDesignerWindow::EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    DWORD processId;
    GetWindowThreadProcessId(hwnd, &processId);

    if (processId == g_lazarusProcess.dwProcessId)
    {
        char windowTitle[256];
        GetWindowTextA(hwnd, windowTitle, sizeof(windowTitle));

        if (std::string(windowTitle).find("Lazarus") != std::string::npos)
        {
            g_lazarusWindow = hwnd;
            return FALSE; // Stop enumeration after finding the main window
        }
    }
    return TRUE;
}

FormDesignerWindow::~FormDesignerWindow()
{
    SendMessage(g_lazarusWindow, WM_CLOSE, 0, NULL);
}

void FormDesignerWindow::SynchronizeWindowState()
{
    // Get the current state of the main application window
    WINDOWPLACEMENT currentPlacement = { sizeof(WINDOWPLACEMENT) };
    GetWindowPlacement(g_mainApplicationWindow, &currentPlacement);

    // Propagate state to Lazarus if it has changed
    if (currentPlacement.showCmd != mainAppPlacement.showCmd)
    {
        mainAppPlacement = currentPlacement;

        switch (currentPlacement.showCmd)
        {
        case SW_SHOWMINIMIZED:
            ShowWindow(g_lazarusWindow, SW_SHOWMINIMIZED);
            break;
        case SW_SHOWMAXIMIZED:
            ShowWindow(g_lazarusWindow, SW_SHOWMAXIMIZED);
            break;
        case SW_SHOWNORMAL:
            ShowWindow(g_lazarusWindow, SW_SHOWNORMAL);
            break;
        }
    }
}
