#pragma once
#include "CString.h"
#include "CByteBuffer.h"

ECK_NAMESPACE_BEGIN
struct CClipboardGuard { ~CClipboardGuard() { CloseClipboard(); } };

inline bool GetClipboardString(HWND hWnd, CStringW& rs) noexcept
{
    if (!OpenClipboard(hWnd))
    {
        EckDbgPrintFormat(L"OpenClipboard failed, Owner = %p", GetClipboardOwner());
        return false;
    }
    CClipboardGuard _{};

    const HGLOBAL hData = GetClipboardData(CF_UNICODETEXT);
    if (!hData)
        return false;
    const PCVOID pData = GlobalLock(hData);
    if (!pData)
        return false;
    auto cch = int(GlobalSize(hData) / sizeof(WCHAR));
    if (cch)
    {
        rs.PushBack((PCWCH)pData, cch);
        while (cch && !rs.IsEmpty() && rs.Back() == L'\0')
        {
            rs.PopBack();
            --cch;
        }
    }
    return true;
}
inline bool SetClipboardString(HWND hWnd,
    _In_reads_or_z_(cch) PCWSTR pszText, int cch = -1) noexcept
{
    if (!OpenClipboard(hWnd))
    {
        EckDbgPrintFormat(L"OpenClipboard failed, Owner = %p", GetClipboardOwner());
        return false;
    }
    CClipboardGuard _{};

    if (cch < 0)
        cch = (int)TcsLength(pszText);
    const auto hData = GlobalAlloc(GMEM_MOVEABLE, CchToCbW(cch));
    if (!hData)
        return false;
    void* const pData = GlobalLock(hData);
    if (!pData)
    {
        GlobalFree(hData);
        return false;
    }
    TcsCopyLengthEnd((PWCH)pData, pszText, cch);
    GlobalUnlock(hData);

    EmptyClipboard();
    if (SetClipboardData(CF_UNICODETEXT, hData))
        return true;
    else
    {
        GlobalFree(hData);
        return false;
    }
}



ECK_NAMESPACE_END