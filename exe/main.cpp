#include <windows.h>
#include <wrl.h>
#include <wil/com.h>
#include "WebView2.h"

using namespace Microsoft::WRL;

HWND g_hWnd;
wil::com_ptr<ICoreWebView2Controller> controller;
wil::com_ptr<ICoreWebView2> webview;

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_SIZE:
        if (controller)
            controller->put_Bounds({ 0, 0, LOWORD(lParam), HIWORD(lParam) });
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
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
    wc.lpszClassName = L"LauchWatch";
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

    // WebView2 starten
    CreateCoreWebView2EnvironmentWithOptions(
        nullptr, nullptr, nullptr,
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [](HRESULT, ICoreWebView2Environment* env) -> HRESULT
            {
                env->CreateCoreWebView2Controller(
                    g_hWnd,
                    Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                        [](HRESULT, ICoreWebView2Controller* ctrl) -> HRESULT
                        {
                            controller = ctrl;
                            controller->get_CoreWebView2(&webview);

                            RECT r;
                            GetClientRect(g_hWnd, &r);
                            controller->put_Bounds(r);

                            webview->Navigate(L"https://lauchwatch.base44.app/");

                            return S_OK;
                        }
                    ).Get()
                );
                return S_OK;
            }
        ).Get()
    );

    // Message Loop
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
