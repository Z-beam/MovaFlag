#define OEMRESOURCE
#include <windows.h>
#include <strsafe.h>
#include "resource.h" // Include the resource header

HINSTANCE   g_instance;
HCURSOR     g_hc_ibeam;
UINT_PTR    g_timer = NULL;
DWORD       g_layout = 0;
NOTIFYICONDATA g_notifyIconData;
HMENU       g_hMenu;

void CALLBACK UpdateTimer(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    HKL layout = GetKeyboardLayout(GetWindowThreadProcessId(GetForegroundWindow(), NULL));
    int caps = GetKeyState(VK_CAPITAL) & 0xFFFF;

    if (caps) {
        HCURSOR hc_new = LoadCursor(g_instance, MAKEINTRESOURCE((reinterpret_cast<UINT_PTR>(layout) & 0xFFFF) * 10 + 2));

        if (hc_new)
        {
            SetSystemCursor(hc_new, OCR_IBEAM);
        }
        else
        {
            SetSystemCursor(CopyCursor(g_hc_ibeam), OCR_IBEAM);
        }
    }

    if (g_layout != reinterpret_cast<UINT_PTR>(layout) && !caps)
    {
        HCURSOR hc_new = LoadCursor(g_instance, MAKEINTRESOURCE(reinterpret_cast<UINT_PTR>(layout) & 0xFFFF));

        if (hc_new)
        {
            SetSystemCursor(hc_new, OCR_IBEAM);
        }
        else
        {
            SetSystemCursor(CopyCursor(g_hc_ibeam), OCR_IBEAM);
        }
    }
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_COMMAND:
        if (LOWORD(wParam) == ID_TRAY_EXIT)
        {
            PostQuitMessage(0);
        }
        break;
    case WM_DESTROY:
        Shell_NotifyIcon(NIM_DELETE, &g_notifyIconData);
        PostQuitMessage(0);
        break;
    case WM_USER + 1:
        if (lParam == WM_RBUTTONUP)
        {
            POINT pt;
            GetCursorPos(&pt);
            SetForegroundWindow(hWnd);
            TrackPopupMenu(g_hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
        }
        break;
    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}

int Main()
{
    HANDLE mutex = CreateMutex(NULL, FALSE, "Mova");
    if (GetLastError() == ERROR_ALREADY_EXISTS || GetLastError() == ERROR_ACCESS_DENIED) return 1;

    g_hc_ibeam = CopyCursor(LoadCursor(NULL, IDC_IBEAM));
    if (!g_hc_ibeam) return 1;

    g_instance = GetModuleHandle(NULL);
    g_timer = SetTimer(NULL, g_timer, 200, UpdateTimer);
    if (!g_timer) return 1;

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = g_instance;
    wc.hIcon = LoadIcon(g_instance, MAKEINTRESOURCE(IDI_APP_ICON)); // Set application icon
    wc.lpszClassName = "LangCursorClass";
    RegisterClass(&wc);

    HWND hWnd = CreateWindow("LangCursorClass", "Mova", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, g_instance, NULL);
    if (!hWnd) return 1;

    g_hMenu = CreatePopupMenu();
    AppendMenu(g_hMenu, MF_STRING, ID_TRAY_EXIT, "Вихід");

    g_notifyIconData.cbSize = sizeof(NOTIFYICONDATA);
    g_notifyIconData.hWnd = hWnd;
    g_notifyIconData.uID = 1;
    g_notifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_notifyIconData.uCallbackMessage = WM_USER + 1;
    g_notifyIconData.hIcon = LoadIcon(g_instance, MAKEINTRESOURCE(IDI_TRAY_ICON)); // Set tray icon
    StringCchCopy(g_notifyIconData.szTip, ARRAYSIZE(g_notifyIconData.szTip), "Mova");
    Shell_NotifyIcon(NIM_ADD, &g_notifyIconData);

    MSG msg;
    while (GetMessage(&msg, 0, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    DestroyCursor(g_hc_ibeam);
    return 0;
}

EXTERN_C void WINAPI WinMainCRTStartup()
{
    ExitProcess(Main());
}
