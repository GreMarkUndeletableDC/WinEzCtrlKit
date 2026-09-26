#include "CWindowTest.h"
#include "eck\AutoLink.h"

int APIENTRY wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ PWSTR pszCmdLine,
    _In_ int nCmdShow)
{
    const auto r = eck::Initialize(hInstance);
    EckAssert(r == eck::StartupStatus::Ok);

    CWindowTest w;
    w.Create(
        nullptr,
        WS_OVERLAPPEDWINDOW, 0,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
        nullptr, nullptr);
    w.Show(SW_SHOW);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0))
    {
        if (!eck::PreTranslateMessage(msg))
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
    return (int)msg.wParam;
}