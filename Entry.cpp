#include "eck\ECK.h"
#include "eck\AutoLink.h"

int APIENTRY wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ PWSTR pszCmdLine,
    _In_ int nCmdShow)
{
    const auto r = eck::Initialize(hInstance);
    EckAssert(r == eck::StartupStatus::Ok);

    NtWaitForSingleObject(NtCurrentProcess(), FALSE, nullptr);
    return 0;
}