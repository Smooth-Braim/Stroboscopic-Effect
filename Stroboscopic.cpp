#include <windows.h>
#include <commctrl.h>
#include <string>
#include <thread>
#include <atomic>
#include <random>

#pragma comment(lib, "comctl32.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// Global variables
HWND g_hOverlay = NULL;
HWND g_hControl = NULL;
std::atomic<bool> g_isRunning(false);
std::atomic<bool> g_shouldExit(false);

// Mode: 0 = Static, 1 = Random
std::atomic<int> g_mode(0);

// Transparency (0-255, where 255 is fully opaque)
std::atomic<int> g_transparency(242);  // Default 95%

// Static mode parameters
std::atomic<int> g_staticInterval(200);    // How often (ms)
std::atomic<int> g_staticDuration(100);    // How long (ms)

// Random mode parameters
std::atomic<int> g_randomIntervalMin(100);
std::atomic<int> g_randomIntervalMax(500);
std::atomic<int> g_randomDurationMin(50);
std::atomic<int> g_randomDurationMax(200);

// Random number generator
std::random_device rd;
std::mt19937 rng(rd());

// Control IDs
#define IDC_START_STOP 1001
#define IDC_MODE_STATIC 1002
#define IDC_MODE_RANDOM 1003
#define IDC_STATIC_INTERVAL 1004
#define IDC_STATIC_DURATION 1005
#define IDC_RANDOM_INT_MIN 1006
#define IDC_RANDOM_INT_MAX 1007
#define IDC_RANDOM_DUR_MIN 1008
#define IDC_RANDOM_DUR_MAX 1009
#define IDC_LABEL_STATIC_INT 1010
#define IDC_LABEL_STATIC_DUR 1011
#define IDC_LABEL_RAND_INT_MIN 1012
#define IDC_LABEL_RAND_INT_MAX 1013
#define IDC_LABEL_RAND_DUR_MIN 1014
#define IDC_LABEL_RAND_DUR_MAX 1015
#define IDC_EXIT 1016
#define IDC_TRANSPARENCY_SLIDER 1017
#define IDC_LABEL_TRANSPARENCY 1018
#define IDC_EDIT_TRANSPARENCY 1019
#define IDC_EDIT_STATIC_INT 1020
#define IDC_EDIT_STATIC_DUR 1021
#define IDC_EDIT_RAND_INT_MIN 1022
#define IDC_EDIT_RAND_INT_MAX 1023
#define IDC_EDIT_RAND_DUR_MIN 1024
#define IDC_EDIT_RAND_DUR_MAX 1025

// Function to get random value in range
int GetRandomValue(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(rng);
}

// Update label functions
void UpdateStaticIntervalLabel(HWND hDlg, int value) {
    wchar_t buffer[64];
    swprintf_s(buffer, L"Interval (how often):");
    SetDlgItemText(hDlg, IDC_LABEL_STATIC_INT, buffer);
    SetDlgItemInt(hDlg, IDC_EDIT_STATIC_INT, value, FALSE);
}

void UpdateStaticDurationLabel(HWND hDlg, int value) {
    wchar_t buffer[64];
    swprintf_s(buffer, L"Duration (how long):");
    SetDlgItemText(hDlg, IDC_LABEL_STATIC_DUR, buffer);
    SetDlgItemInt(hDlg, IDC_EDIT_STATIC_DUR, value, FALSE);
}

void UpdateRandomIntMinLabel(HWND hDlg, int value) {
    wchar_t buffer[64];
    swprintf_s(buffer, L"Interval Min:");
    SetDlgItemText(hDlg, IDC_LABEL_RAND_INT_MIN, buffer);
    SetDlgItemInt(hDlg, IDC_EDIT_RAND_INT_MIN, value, FALSE);
}

void UpdateRandomIntMaxLabel(HWND hDlg, int value) {
    wchar_t buffer[64];
    swprintf_s(buffer, L"Interval Max:");
    SetDlgItemText(hDlg, IDC_LABEL_RAND_INT_MAX, buffer);
    SetDlgItemInt(hDlg, IDC_EDIT_RAND_INT_MAX, value, FALSE);
}

void UpdateRandomDurMinLabel(HWND hDlg, int value) {
    wchar_t buffer[64];
    swprintf_s(buffer, L"Duration Min:");
    SetDlgItemText(hDlg, IDC_LABEL_RAND_DUR_MIN, buffer);
    SetDlgItemInt(hDlg, IDC_EDIT_RAND_DUR_MIN, value, FALSE);
}

void UpdateRandomDurMaxLabel(HWND hDlg, int value) {
    wchar_t buffer[64];
    swprintf_s(buffer, L"Duration Max:");
    SetDlgItemText(hDlg, IDC_LABEL_RAND_DUR_MAX, buffer);
    SetDlgItemInt(hDlg, IDC_EDIT_RAND_DUR_MAX, value, FALSE);
}

void UpdateTransparencyLabel(HWND hDlg, int value) {
    wchar_t buffer[64];
    int percentage = (value * 100) / 255;
    swprintf_s(buffer, L"Opacity:");
    SetDlgItemText(hDlg, IDC_LABEL_TRANSPARENCY, buffer);
    SetDlgItemInt(hDlg, IDC_EDIT_TRANSPARENCY, percentage, FALSE);
}

// Show/hide controls based on mode
void UpdateControlVisibility(HWND hDlg) {
    bool isStatic = (g_mode == 0);

    // Static mode controls
    ShowWindow(GetDlgItem(hDlg, IDC_LABEL_STATIC_INT), isStatic ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hDlg, IDC_STATIC_INTERVAL), isStatic ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hDlg, IDC_EDIT_STATIC_INT), isStatic ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hDlg, IDC_LABEL_STATIC_DUR), isStatic ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hDlg, IDC_STATIC_DURATION), isStatic ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hDlg, IDC_EDIT_STATIC_DUR), isStatic ? SW_SHOW : SW_HIDE);

    // Random mode controls
    ShowWindow(GetDlgItem(hDlg, IDC_LABEL_RAND_INT_MIN), !isStatic ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hDlg, IDC_RANDOM_INT_MIN), !isStatic ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hDlg, IDC_EDIT_RAND_INT_MIN), !isStatic ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hDlg, IDC_LABEL_RAND_INT_MAX), !isStatic ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hDlg, IDC_RANDOM_INT_MAX), !isStatic ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hDlg, IDC_EDIT_RAND_INT_MAX), !isStatic ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hDlg, IDC_LABEL_RAND_DUR_MIN), !isStatic ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hDlg, IDC_RANDOM_DUR_MIN), !isStatic ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hDlg, IDC_EDIT_RAND_DUR_MIN), !isStatic ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hDlg, IDC_LABEL_RAND_DUR_MAX), !isStatic ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hDlg, IDC_RANDOM_DUR_MAX), !isStatic ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hDlg, IDC_EDIT_RAND_DUR_MAX), !isStatic ? SW_SHOW : SW_HIDE);
}

// Strobe thread function
void StrobeThread() {
    while (!g_shouldExit) {
        if (g_isRunning && g_hOverlay) {
            int interval, duration;

            if (g_mode == 0) {
                // Static mode
                interval = g_staticInterval;
                duration = g_staticDuration;
            }
            else {
                // Random mode
                interval = GetRandomValue(g_randomIntervalMin, g_randomIntervalMax);
                duration = GetRandomValue(g_randomDurationMin, g_randomDurationMax);
            }

            // Wait before showing black frame
            int waitTime = interval - duration;
            if (waitTime > 0) {
                Sleep(waitTime);
            }

            if (!g_isRunning) continue;

            // Flash dark
            SetLayeredWindowAttributes(g_hOverlay, 0, (BYTE)g_transparency.load(), LWA_ALPHA);
            Sleep(duration);

            if (!g_isRunning) continue;

            // Flash transparent
            SetLayeredWindowAttributes(g_hOverlay, 0, 0, LWA_ALPHA);    // Fully transparent
        }
        else {
            Sleep(50);  // Idle wait
        }
    }
}

// Overlay window procedure
LRESULT CALLBACK OverlayProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_DESTROY:
        g_hOverlay = NULL;
        return 0;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));
        FillRect(hdc, &ps.rcPaint, hBrush);
        DeleteObject(hBrush);
        EndPaint(hwnd, &ps);
        return 0;
    }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// Create the overlay window
void CreateOverlayWindow(HINSTANCE hInstance) {
    if (g_hOverlay) return;

    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = OverlayProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"StrobeOverlay";
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    RegisterClassEx(&wc);

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    g_hOverlay = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_NOACTIVATE,
        L"StrobeOverlay",
        L"Strobe Overlay",
        WS_POPUP,
        0, 0, screenWidth, screenHeight,
        NULL, NULL, hInstance, NULL
    );

    SetLayeredWindowAttributes(g_hOverlay, 0, 0, LWA_ALPHA);
    ShowWindow(g_hOverlay, SW_SHOW);
    UpdateWindow(g_hOverlay);
}

// Control window procedure
LRESULT CALLBACK ControlProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HINSTANCE hInstance;

    switch (msg) {
    case WM_CREATE: {
        hInstance = ((LPCREATESTRUCT)lParam)->hInstance;

        int yPos = 10;

        // Title
        CreateWindow(L"STATIC", L"Stroboscopic Vision Trainer",
            WS_CHILD | WS_VISIBLE | SS_CENTER,
            10, yPos, 330, 25, hwnd, NULL, hInstance, NULL);
        yPos += 35;

        // Mode selection
        CreateWindow(L"STATIC", L"Mode:",
            WS_CHILD | WS_VISIBLE,
            20, yPos, 100, 20, hwnd, NULL, hInstance, NULL);
        yPos += 25;

        CreateWindow(L"BUTTON", L"Static Timing",
            WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP,
            20, yPos, 150, 20, hwnd, (HMENU)IDC_MODE_STATIC, hInstance, NULL);

        CreateWindow(L"BUTTON", L"Random Timing",
            WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
            180, yPos, 150, 20, hwnd, (HMENU)IDC_MODE_RANDOM, hInstance, NULL);

        CheckRadioButton(hwnd, IDC_MODE_STATIC, IDC_MODE_RANDOM, IDC_MODE_STATIC);
        yPos += 35;

        // === TRANSPARENCY CONTROL (Global) ===
        CreateWindow(L"STATIC", L"Opacity:",
            WS_CHILD | WS_VISIBLE,
            20, yPos, 200, 20, hwnd, (HMENU)IDC_LABEL_TRANSPARENCY, hInstance, NULL);

        CreateWindow(L"EDIT", L"95",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
            230, yPos, 100, 20, hwnd, (HMENU)IDC_EDIT_TRANSPARENCY, hInstance, NULL);
        yPos += 25;

        {
            HWND hSlider = CreateWindow(TRACKBAR_CLASS, L"",
                WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_AUTOTICKS,
                20, yPos, 310, 30, hwnd, (HMENU)IDC_TRANSPARENCY_SLIDER, hInstance, NULL);
            SendMessage(hSlider, TBM_SETRANGE, TRUE, MAKELONG(0, 255));
            SendMessage(hSlider, TBM_SETPOS, TRUE, 242);
            SendMessage(hSlider, TBM_SETTICFREQ, 25, 0);
        }
        yPos += 45;

        // === STATIC MODE CONTROLS ===
        CreateWindow(L"STATIC", L"Interval (how often):",
            WS_CHILD | WS_VISIBLE,
            20, yPos, 200, 20, hwnd, (HMENU)IDC_LABEL_STATIC_INT, hInstance, NULL);

        CreateWindow(L"EDIT", L"200",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
            230, yPos, 100, 20, hwnd, (HMENU)IDC_EDIT_STATIC_INT, hInstance, NULL);
        yPos += 30;

        {
            HWND hSlider = CreateWindow(TRACKBAR_CLASS, L"",
                WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_AUTOTICKS,
                20, yPos, 310, 30, hwnd, (HMENU)IDC_STATIC_INTERVAL, hInstance, NULL);
            SendMessage(hSlider, TBM_SETRANGE, TRUE, MAKELONG(50, 2000));
            SendMessage(hSlider, TBM_SETPOS, TRUE, 200);
            SendMessage(hSlider, TBM_SETTICFREQ, 100, 0);
        }
        yPos += 45;

        CreateWindow(L"STATIC", L"Duration (how long):",
            WS_CHILD | WS_VISIBLE,
            20, yPos, 200, 20, hwnd, (HMENU)IDC_LABEL_STATIC_DUR, hInstance, NULL);

        CreateWindow(L"EDIT", L"100",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
            230, yPos, 100, 20, hwnd, (HMENU)IDC_EDIT_STATIC_DUR, hInstance, NULL);
        yPos += 30;

        {
            HWND hSlider = CreateWindow(TRACKBAR_CLASS, L"",
                WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_AUTOTICKS,
                20, yPos, 310, 30, hwnd, (HMENU)IDC_STATIC_DURATION, hInstance, NULL);
            SendMessage(hSlider, TBM_SETRANGE, TRUE, MAKELONG(10, 1000));
            SendMessage(hSlider, TBM_SETPOS, TRUE, 100);
            SendMessage(hSlider, TBM_SETTICFREQ, 50, 0);
        }
        yPos += 45;

        // === RANDOM MODE CONTROLS (initially hidden) ===
        int randomYPos = 150; // Adjusted for transparency slider

        CreateWindow(L"STATIC", L"Interval Min:",
            WS_CHILD,
            20, randomYPos, 200, 20, hwnd, (HMENU)IDC_LABEL_RAND_INT_MIN, hInstance, NULL);

        CreateWindow(L"EDIT", L"100",
            WS_CHILD | WS_BORDER | ES_NUMBER,
            230, randomYPos, 100, 20, hwnd, (HMENU)IDC_EDIT_RAND_INT_MIN, hInstance, NULL);
        randomYPos += 30;

        {
            HWND hSlider = CreateWindow(TRACKBAR_CLASS, L"",
                WS_CHILD | TBS_HORZ | TBS_AUTOTICKS,
                20, randomYPos, 310, 30, hwnd, (HMENU)IDC_RANDOM_INT_MIN, hInstance, NULL);
            SendMessage(hSlider, TBM_SETRANGE, TRUE, MAKELONG(50, 2000));
            SendMessage(hSlider, TBM_SETPOS, TRUE, 100);
            SendMessage(hSlider, TBM_SETTICFREQ, 100, 0);
        }
        randomYPos += 45;

        CreateWindow(L"STATIC", L"Interval Max:",
            WS_CHILD,
            20, randomYPos, 200, 20, hwnd, (HMENU)IDC_LABEL_RAND_INT_MAX, hInstance, NULL);

        CreateWindow(L"EDIT", L"500",
            WS_CHILD | WS_BORDER | ES_NUMBER,
            230, randomYPos, 100, 20, hwnd, (HMENU)IDC_EDIT_RAND_INT_MAX, hInstance, NULL);
        randomYPos += 30;

        {
            HWND hSlider = CreateWindow(TRACKBAR_CLASS, L"",
                WS_CHILD | TBS_HORZ | TBS_AUTOTICKS,
                20, randomYPos, 310, 30, hwnd, (HMENU)IDC_RANDOM_INT_MAX, hInstance, NULL);
            SendMessage(hSlider, TBM_SETRANGE, TRUE, MAKELONG(50, 2000));
            SendMessage(hSlider, TBM_SETPOS, TRUE, 500);
            SendMessage(hSlider, TBM_SETTICFREQ, 100, 0);
        }
        randomYPos += 45;

        CreateWindow(L"STATIC", L"Duration Min:",
            WS_CHILD,
            20, randomYPos, 200, 20, hwnd, (HMENU)IDC_LABEL_RAND_DUR_MIN, hInstance, NULL);

        CreateWindow(L"EDIT", L"50",
            WS_CHILD | WS_BORDER | ES_NUMBER,
            230, randomYPos, 100, 20, hwnd, (HMENU)IDC_EDIT_RAND_DUR_MIN, hInstance, NULL);
        randomYPos += 30;

        {
            HWND hSlider = CreateWindow(TRACKBAR_CLASS, L"",
                WS_CHILD | TBS_HORZ | TBS_AUTOTICKS,
                20, randomYPos, 310, 30, hwnd, (HMENU)IDC_RANDOM_DUR_MIN, hInstance, NULL);
            SendMessage(hSlider, TBM_SETRANGE, TRUE, MAKELONG(10, 1000));
            SendMessage(hSlider, TBM_SETPOS, TRUE, 50);
            SendMessage(hSlider, TBM_SETTICFREQ, 50, 0);
        }
        randomYPos += 45;

        CreateWindow(L"STATIC", L"Duration Max:",
            WS_CHILD,
            20, randomYPos, 200, 20, hwnd, (HMENU)IDC_LABEL_RAND_DUR_MAX, hInstance, NULL);

        CreateWindow(L"EDIT", L"200",
            WS_CHILD | WS_BORDER | ES_NUMBER,
            230, randomYPos, 100, 20, hwnd, (HMENU)IDC_EDIT_RAND_DUR_MAX, hInstance, NULL);
        randomYPos += 30;

        {
            HWND hSlider = CreateWindow(TRACKBAR_CLASS, L"",
                WS_CHILD | TBS_HORZ | TBS_AUTOTICKS,
                20, randomYPos, 310, 30, hwnd, (HMENU)IDC_RANDOM_DUR_MAX, hInstance, NULL);
            SendMessage(hSlider, TBM_SETRANGE, TRUE, MAKELONG(10, 1000));
            SendMessage(hSlider, TBM_SETPOS, TRUE, 200);
            SendMessage(hSlider, TBM_SETTICFREQ, 50, 0);
        }

        // Start/Stop button (at bottom)
        CreateWindow(L"BUTTON", L"START STROBE",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            20, 445, 310, 50, hwnd, (HMENU)IDC_START_STOP, hInstance, NULL);

        // Exit button
        CreateWindow(L"BUTTON", L"Exit Program",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            20, 505, 310, 30, hwnd, (HMENU)IDC_EXIT, hInstance, NULL);

        // Warning
        CreateWindow(L"STATIC", L"⚠ Not suitable for people with epilepsy",
            WS_CHILD | WS_VISIBLE | SS_CENTER,
            10, 545, 330, 20, hwnd, NULL, hInstance, NULL);

        UpdateControlVisibility(hwnd);
        return 0;
    }

    case WM_HSCROLL: {
        HWND hSlider = (HWND)lParam;
        int pos = SendMessage(hSlider, TBM_GETPOS, 0, 0);
        int id = GetDlgCtrlID(hSlider);

        switch (id) {
        case IDC_TRANSPARENCY_SLIDER:
            g_transparency = pos;
            UpdateTransparencyLabel(hwnd, pos);
            break;
        case IDC_STATIC_INTERVAL:
            g_staticInterval = pos;
            UpdateStaticIntervalLabel(hwnd, pos);
            break;
        case IDC_STATIC_DURATION:
            g_staticDuration = pos;
            UpdateStaticDurationLabel(hwnd, pos);
            break;
        case IDC_RANDOM_INT_MIN:
            g_randomIntervalMin = pos;
            UpdateRandomIntMinLabel(hwnd, pos);
            break;
        case IDC_RANDOM_INT_MAX:
            g_randomIntervalMax = pos;
            UpdateRandomIntMaxLabel(hwnd, pos);
            break;
        case IDC_RANDOM_DUR_MIN:
            g_randomDurationMin = pos;
            UpdateRandomDurMinLabel(hwnd, pos);
            break;
        case IDC_RANDOM_DUR_MAX:
            g_randomDurationMax = pos;
            UpdateRandomDurMaxLabel(hwnd, pos);
            break;
        }
        return 0;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_MODE_STATIC:
            g_mode = 0;
            UpdateControlVisibility(hwnd);
            return 0;

        case IDC_MODE_RANDOM:
            g_mode = 1;
            UpdateControlVisibility(hwnd);
            return 0;

        case IDC_EDIT_TRANSPARENCY:
            if (HIWORD(wParam) == EN_CHANGE) {
                int percentage = GetDlgItemInt(hwnd, IDC_EDIT_TRANSPARENCY, NULL, FALSE);
                if (percentage >= 0 && percentage <= 100) {
                    int value = (percentage * 255) / 100;
                    g_transparency = value;
                    SendDlgItemMessage(hwnd, IDC_TRANSPARENCY_SLIDER, TBM_SETPOS, TRUE, value);
                }
            }
            return 0;

        case IDC_EDIT_STATIC_INT:
            if (HIWORD(wParam) == EN_CHANGE) {
                int value = GetDlgItemInt(hwnd, IDC_EDIT_STATIC_INT, NULL, FALSE);
                if (value >= 50 && value <= 2000) {
                    g_staticInterval = value;
                    SendDlgItemMessage(hwnd, IDC_STATIC_INTERVAL, TBM_SETPOS, TRUE, value);
                }
            }
            return 0;

        case IDC_EDIT_STATIC_DUR:
            if (HIWORD(wParam) == EN_CHANGE) {
                int value = GetDlgItemInt(hwnd, IDC_EDIT_STATIC_DUR, NULL, FALSE);
                if (value >= 10 && value <= 1000) {
                    g_staticDuration = value;
                    SendDlgItemMessage(hwnd, IDC_STATIC_DURATION, TBM_SETPOS, TRUE, value);
                }
            }
            return 0;

        case IDC_EDIT_RAND_INT_MIN:
            if (HIWORD(wParam) == EN_CHANGE) {
                int value = GetDlgItemInt(hwnd, IDC_EDIT_RAND_INT_MIN, NULL, FALSE);
                if (value >= 50 && value <= 2000) {
                    g_randomIntervalMin = value;
                    SendDlgItemMessage(hwnd, IDC_RANDOM_INT_MIN, TBM_SETPOS, TRUE, value);
                }
            }
            return 0;

        case IDC_EDIT_RAND_INT_MAX:
            if (HIWORD(wParam) == EN_CHANGE) {
                int value = GetDlgItemInt(hwnd, IDC_EDIT_RAND_INT_MAX, NULL, FALSE);
                if (value >= 50 && value <= 2000) {
                    g_randomIntervalMax = value;
                    SendDlgItemMessage(hwnd, IDC_RANDOM_INT_MAX, TBM_SETPOS, TRUE, value);
                }
            }
            return 0;

        case IDC_EDIT_RAND_DUR_MIN:
            if (HIWORD(wParam) == EN_CHANGE) {
                int value = GetDlgItemInt(hwnd, IDC_EDIT_RAND_DUR_MIN, NULL, FALSE);
                if (value >= 10 && value <= 1000) {
                    g_randomDurationMin = value;
                    SendDlgItemMessage(hwnd, IDC_RANDOM_DUR_MIN, TBM_SETPOS, TRUE, value);
                }
            }
            return 0;

        case IDC_EDIT_RAND_DUR_MAX:
            if (HIWORD(wParam) == EN_CHANGE) {
                int value = GetDlgItemInt(hwnd, IDC_EDIT_RAND_DUR_MAX, NULL, FALSE);
                if (value >= 10 && value <= 1000) {
                    g_randomDurationMax = value;
                    SendDlgItemMessage(hwnd, IDC_RANDOM_DUR_MAX, TBM_SETPOS, TRUE, value);
                }
            }
            return 0;

        case IDC_START_STOP:
            if (!g_isRunning) {
                if (!g_hOverlay) {
                    CreateOverlayWindow(hInstance);
                }
                g_isRunning = true;
                SetDlgItemText(hwnd, IDC_START_STOP, L"STOP STROBE");
            }
            else {
                g_isRunning = false;
                if (g_hOverlay) {
                    SetLayeredWindowAttributes(g_hOverlay, 0, 0, LWA_ALPHA);
                }
                SetDlgItemText(hwnd, IDC_START_STOP, L"START STROBE");
            }
            return 0;

        case IDC_EXIT:
            PostMessage(hwnd, WM_CLOSE, 0, 0);
            return 0;
        }
        break;

    case WM_CLOSE:
        g_shouldExit = true;
        g_isRunning = false;
        if (g_hOverlay) {
            DestroyWindow(g_hOverlay);
        }
        DestroyWindow(hwnd);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    std::thread strobeThread(StrobeThread);

    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = ControlProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"StrobeControl";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassEx(&wc);

    g_hControl = CreateWindowEx(
        0,
        L"StrobeControl",
        L"Strobe Vision Trainer - Control Panel",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 370, 630,
        NULL, NULL, hInstance, NULL
    );

    ShowWindow(g_hControl, nCmdShow);
    UpdateWindow(g_hControl);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    g_shouldExit = true;
    strobeThread.join();

    return (int)msg.wParam;
}