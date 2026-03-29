#include <windows.h>
#include <shlobj.h> // Für AppData Pfad
#include <string>
#include "WebView2.h"

HWND g_hWnd = nullptr;
ICoreWebView2Controller* g_controller = nullptr;
ICoreWebView2* g_webview = nullptr;

// ---------------------------
// COM Helper
// ---------------------------
template<typename T>
class ComBase : public T
{
public:
    ULONG refCount = 1;

    ULONG STDMETHODCALLTYPE AddRef() override { return ++refCount; }
    ULONG STDMETHODCALLTYPE Release() override
    {
        ULONG r = --refCount;
        if (r == 0) delete this;
        return r;
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) override
    {
        if (riid == __uuidof(IUnknown) || riid == __uuidof(T))
        {
            *ppv = this;
            AddRef();
            return S_OK;
        }
        *ppv = nullptr;
        return E_NOINTERFACE;
    }
};

// ---------------------------
// Controller Callback
// ---------------------------
class ControllerHandler :
    public ComBase<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>
{
public:
    HRESULT STDMETHODCALLTYPE Invoke(HRESULT hr, ICoreWebView2Controller* controller) override
    {
        if (FAILED(hr)) return hr;

        g_controller = controller;
        g_controller->AddRef();

        g_controller->get_CoreWebView2(&g_webview);

        RECT r;
        GetClientRect(g_hWnd, &r);
        g_controller->put_Bounds(r);

        g_webview->Navigate(L"https://lauchwatch.base44.app/");

        return S_OK;
    }
};

// ---------------------------
// Environment Callback
// ---------------------------
class EnvHandler :
    public ComBase<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>
{
public:
    HRESULT STDMETHODCALLTYPE Invoke(HRESULT hr, ICoreWebView2Environment* env) override
    {
        if (FAILED(hr)) return hr;

        env->CreateCoreWebView2Controller(g_hWnd, new ControllerHandler());
        return S_OK;
    }
};

// ---------------------------
// Window Proc
// ---------------------------
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

// ---------------------------
// AppData Pfad holen
// ---------------------------
std::wstring GetAppDataPath()
{
    wchar_t path[MAX_PATH];
    SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path);

    std::wstring fullPath = path;
    fullPath += L"\\Lauchwatch";

    // Ordner erstellen falls nicht vorhanden
    CreateDirectoryW(fullPath.c_str(), NULL);

    return fullPath;
}

// ---------------------------
// WinMain
// ---------------------------
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

    // 🔥 WICHTIG: eigener Datenpfad
    std::wstring userDataPath = GetAppDataPath();

    CreateCoreWebView2EnvironmentWithOptions(
        nullptr,
        userDataPath.c_str(), // <-- FIX
        nullptr,
        new EnvHandler()
    );

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
