#include "CWindowTest.h"

#ifdef _DEBUG
#define ECK_OPT_CRT_DLL 1
#endif
#include "eck\AutoLink.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR pszCmdLine, _In_ int nCmdShow)
{
    const auto r = eck::Initialize(hInstance);
    EckAssert(r == eck::InitStatus::Ok);

    CWindowTest w;
    w.Create(nullptr, WS_OVERLAPPEDWINDOW, 0, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, 0);
    w.Show(SW_SHOW);

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0))
    {
        if (!eck::PreTranslateMessage(msg))
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
    return (int)msg.wParam;
}