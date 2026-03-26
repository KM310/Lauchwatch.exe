#include <windows.h>
#include "WebView2.h"

HWND g_hWnd = nullptr;
ICoreWebView2Controller* g_controller = nullptr;
ICoreWebView2* g_webview = nullptr;

// ---------------------------
// Callback-Klasse 1: Environment
// ---------------------------
class EnvCompletedHandler :
    public ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler
{
public:
    ULONG STDMETHODCALLTYPE AddRef() override { return 1; }
    ULONG STDMETHODCALLTYPE Release() override { return 1; }
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) override
    {
        if (riid == __uuidof(IUnknown) ||
            riid == __uuidof(ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler))
        {
            *ppv = this;
            return S_OK;
        }
        *ppv = nullptr;
        return E_NOINTERFACE;
    }

    HRESULT STDMETHODCALLTYPE Invoke(HRESULT hr, ICoreWebView2Environment* env) override
    {
        env->CreateCoreWebView2Controller(
            g_hWnd,
            new ControllerCompletedHandler()
        );
        return S_OK;
    }

    // ---------------------------
    // Callback-Klasse 2: Controller
    // ---------------------------
    class ControllerCompletedHandler :
        public ICoreWebView2CreateCoreWebView2ControllerCompletedHandler
    {
    public:
        ULONG STDMETHODCALLTYPE AddRef() override { return 1; }
        ULONG STDMETHODCALLTYPE Release() override { return 1; }
        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) override
        {
            if (riid == __uuidof(IUnknown) ||
                riid == __uuidof(ICoreWebView2CreateCoreWebView2ControllerCompletedHandler))
            {
                *ppv = this;
                return S_OK;
            }
            *ppv = nullptr;
            return E_NOINTERFACE;
        }

        HRESULT STDMETHODCALLTYPE Invoke(HRESULT hr, ICoreWebView2Controller* controller) override
        {
            g_controller = controller;
            controller->get_CoreWebView2(&g_webview);

            RECT r;
            GetClientRect(g_hWnd, &r);
            controller->put_Bounds(r);

            g_webview->Navigate(L"https://lauchwatch.base44.app/");
            return S_OK;
        }
    };
};

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_SIZE && g_controller)
    {
        RECT r;
        GetClientRect(hWnd, &r);
        g_controller->put_Bounds(r);
    }

    if (msg == WM_DESTROY)
        PostQuitMessage(0);

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

    // WebView2 starten
    CreateCoreWebView2EnvironmentWithOptions(
        nullptr, nullptr, nullptr,
        new EnvCompletedHandler()
    );

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
