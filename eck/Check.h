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

    BccUserBegin = 0x10000,
};

[[noreturn]] inline void BugCheck(
    PCWSTR pszFile, int nLine, PCWSTR pszFunc,
    UINT uCode = BccGeneric,
    PCWSTR pszError = nullptr,
    UINT_PTR C1 = 0u, UINT_PTR C2 = 0u) noexcept
{
    PWCH p;

    // --

    constexpr WCHAR TipLine[]{ L"Line " };
    WCHAR szLine[TcvIntBufferSize<int>() + ARRAYSIZE(TipLine)];
    p = szLine;
    EckCopyConstStringW(p, TipLine);
    p += (ARRAYSIZE(TipLine) - 1);
    TcvFromInt(p, std::end(szLine) - p, nLine);

    // --
    
    constexpr WCHAR TipCode[]{ L"Code = " };
    WCHAR szCode[TcvIntBufferSize<UINT>() + ARRAYSIZE(TipCode)];
    p = szCode;
    EckCopyConstStringW(p, TipCode);
    p += (ARRAYSIZE(TipCode) - 1);
    TcvFromInt(p, std::end(szCode) - p, uCode);

    // --

    constexpr WCHAR TipC1[]{ L"C1 = 0x" };
    constexpr WCHAR TipC2[]{ L", C2 = 0x" };
    constexpr int FillTo = sizeof(UINT_PTR) * 2;
    WCHAR szParam[
        TcvIntBufferSize<UINT_PTR>(16, FillTo) * 2 +
            ARRAYSIZE(TipC1) + ARRAYSIZE(TipC2)
    ];
    p = szParam;
    EckCopyConstStringW(p, TipC1);
    p += (ARRAYSIZE(TipC1) - 1);
    p = TcvFromInt(p, std::end(szParam) - p, C1, 16, TRUE, FillTo).pEnd;

    EckCopyConstStringW(p, TipC2);
    p += (ARRAYSIZE(TipC2) - 1);
    TcvFromInt(p, std::end(szParam) - p, C2, 16, TRUE, FillTo);

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

#define EckBugCheck(...)                    \
    ::eck::BugCheck(ECK_PRIV_BUGCHECK_POS, ##__VA_ARGS__)

#define EckCheckPointer(x, ...) do {        \
    if (!(x))                               \
        ::eck::BugCheck(ECK_PRIV_BUGCHECK_POS, BccPointerNull, ##__VA_ARGS__); } while (0)
#define EckCheckNtHandle(x, ...) do {       \
    if (!::eck::Detail::CheckNtHandle(x))   \
        ::eck::BugCheck(ECK_PRIV_BUGCHECK_POS, BccNtHandleInvalid, ##__VA_ARGS__); } while (0)
#define EckCheckNtStatus(x, ...) do {       \
    if (!NT_SUCCESS(x))                     \
        ::eck::BugCheck(ECK_PRIV_BUGCHECK_POS, BccNtStatusFailed, ##__VA_ARGS__); } while (0)
#define EckCheckWin32Error(x, ...) do {     \
    if (x)                                  \
        ::eck::BugCheck(ECK_PRIV_BUGCHECK_POS, BccWin32Error, ##__VA_ARGS__); } while (0)
#define EckCheckHResult(x, ...) do {        \
    if (FAILED(x))                          \
        ::eck::BugCheck(ECK_PRIV_BUGCHECK_POS, BccHResultFailed, ##__VA_ARGS__); } while (0)
#define EckCheckBool(x, ...) do {           \
    if (!(x))                               \
        ::eck::BugCheck(ECK_PRIV_BUGCHECK_POS, BccBoolFalse, ##__VA_ARGS__); } while (0)
ECK_NAMESPACE_END