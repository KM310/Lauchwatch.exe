#include <windows.h>
#include "WebView2.h"

HWND g_hWnd = nullptr;
ICoreWebView2Controller* g_controller = nullptr;
ICoreWebView2* g_webview = nullptr;

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_SIZE:
        if (g_controller)
        {
            RECT r;
            GetClientRect(hWnd, &r);
            g_controller->put_Bounds(r);
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

void InitWebView2()
{
    CreateCoreWebView2EnvironmentWithOptions(
        nullptr, nullptr, nullptr,
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [](HRESULT hr, ICoreWebView2Environment* env) -> HRESULT
            {
                env->CreateCoreWebView2Controller(
                    g_hWnd,
                    Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                        [](HRESULT hr, ICoreWebView2Controller* controller) -> HRESULT
                        {
                            g_controller = controller;

                            controller->get_CoreWebView2(&g_webview);

                            RECT r;
                            GetClientRect(g_hWnd, &r);
                            controller->put_Bounds(r);

                            g_webview->Navigate(L"https://lauchwatch.base44.app/");

                            return S_OK;
                        }
                    ).Get()
                );
                return S_OK;
            }
        ).Get()
    );
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int)
{
    // Icon aus DLL laden
    HINSTANCE hIconDll = LoadLibraryA("icon.dll");
    HICON hIcon = nullptr;

    if (hIconDll)
        hIcon = (HICON)LoadImageA(hIconDll, "IDI_ICON1", IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);

    // Fensterklasse
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = L"LauchWatchWindow";
    wc.hIcon = hIcon;

    RegisterClass(&wc);

    g_hWnd = CreateWindow(
        wc.lpszClassName,
        L"LauchWatch",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        900, 600,
        nullptr, nullptr, hInst, nullptr
    );

    ShowWindow(g_hWnd, SW_SHOW);

    InitWebView2();

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

