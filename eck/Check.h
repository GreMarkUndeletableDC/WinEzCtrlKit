#pragma once
#include "StringConvert.h"

ECK_NAMESPACE_BEGIN
enum : UINT
{
    BccGeneric,
    BccPointerNull,
    BccNtHandleInvalid,
    BccNtStatusFailed,
    BccWin32Error,
    BccHResultFailed,
    BccBoolFalse,
    BccNotImplemented,
    BccException,
};

[[noreturn]] inline void BugCheck(
    PCWSTR pszFile, int nLine, PCWSTR pszFunc,
    UINT uCode = BccGeneric,
    PCWSTR pszError = nullptr,
    UINT_PTR C1 = 0u, UINT_PTR C2 = 0u) noexcept
{
    constexpr WCHAR TipLine[]{ L"Line " };
    WCHAR szLine[TcvIntBufferSize<int>() + ARRAYSIZE(TipLine)];
    EckCopyConstStringW(szLine, TipLine);
    TcvFromInt(
        szLine + ARRAYSIZE(TipLine) - 1,
        ARRAYSIZE(szLine),
        nLine);

    constexpr WCHAR TipCode[]{ L"Code = " };
    WCHAR szCode[TcvIntBufferSize<UINT>() + ARRAYSIZE(TipCode)];
    EckCopyConstStringW(szCode, TipCode);
    TcvFromInt(
        szCode + ARRAYSIZE(TipCode) - 1,
        ARRAYSIZE(szCode),
        uCode);

    constexpr WCHAR TipC1[]{ L"C1 = " };
    constexpr WCHAR TipC2[]{ L", C2 = " };
    WCHAR szParam[TcvIntBufferSize<UINT_PTR>() * 2 + ARRAYSIZE(TipC1) + ARRAYSIZE(TipC2)];
    EckCopyConstStringW(szParam, TipC1);
    auto p = szParam + ARRAYSIZE(TipC1) - 1;
    p = TcvFromInt(p, ARRAYSIZE(szParam), C1).pEnd;
    TcvFromInt(p,ARRAYSIZE(szParam),C2);

    PCWSTR pszMsg[]
    {
        L"!!! Eck BugCheck !!!",
        pszFile,
        szLine,
        pszFunc,
        pszError ? pszError : L"(Empty error message)",
        szCode,
        szParam
    };
#ifdef _DEBUG
    for (const auto psz : pszMsg)
    {
        OutputDebugStringW(psz);
        OutputDebugStringW(L"\n");
    }
    EckDbgBreak();
#else
    const auto hEvent = RegisterEventSourceW(nullptr, ECK_APP_NAME);
    if (hEvent)
    {
        ReportEventW(
            hEvent,
            EVENTLOG_ERROR_TYPE, 0, 0,
            nullptr,
            ARRAYSIZE(pszMsg), 0,
            pszMsg, nullptr);
        DeregisterEventSource(hEvent);
    }
#endif // _DEBUG
    std::terminate();
}

namespace Detail
{
    EckInlineNdCe BOOL CheckNtHandle(HANDLE h) noexcept
    {
        return h != nullptr && h != INVALID_HANDLE_VALUE;
    }
}

#define ECK_PRIV_BUGCHECK_POS __FILEW__, __LINE__, __FUNCTIONW__

#define EckBugCheck(...)            \
    ::eck::BugCheck(ECK_PRIV_BUGCHECK_POS, ##__VA_ARGS__)

#define EckCheckPointer(x, ...)     \
    if (!(x))                       \
        ::eck::BugCheck(ECK_PRIV_BUGCHECK_POS, BccPointerNull, ##__VA_ARGS__)
#define EckCheckNtHandle(x, ...)    \
    if (!::eck::Detail::CheckNtHandle(x)) \
        ::eck::BugCheck(ECK_PRIV_BUGCHECK_POS, BccNtHandleInvalid, ##__VA_ARGS__)
#define EckCheckNtStatus(x, ...)    \
    if (!NT_SUCCESS(x))             \
        ::eck::BugCheck(ECK_PRIV_BUGCHECK_POS, BccNtStatusFailed, ##__VA_ARGS__)
#define EckCheckWin32Error(x, ...)  \
    if (!(x))                       \
        ::eck::BugCheck(ECK_PRIV_BUGCHECK_POS, BccWin32Error, ##__VA_ARGS__)
#define EckCheckHResult(x, ...)     \
    if (FAILED(x))                  \
        ::eck::BugCheck(ECK_PRIV_BUGCHECK_POS, BccHResultFailed, ##__VA_ARGS__)
#define EckCheckBool(x, ...)        \
    if (!(x))                       \
        ::eck::BugCheck(ECK_PRIV_BUGCHECK_POS, BccBoolFalse, ##__VA_ARGS__)
ECK_NAMESPACE_END