#pragma once
#include "NativeWrapper.h"

ECK_NAMESPACE_BEGIN
EckInlineNd int LcsCompareLength2(
    _In_NLS_string_(Len1) PCWCH Str1,
    int Len1,
    _In_NLS_string_(Len2) PCWCH Str2,
    int Len2,
    UINT uFlags,
    _In_opt_z_ PCWSTR pszLocaleName = LOCALE_NAME_USER_DEFAULT) noexcept
{
    const int r = CompareStringEx(pszLocaleName, uFlags,
        Str1, Len1, Str2, Len2, nullptr, nullptr, 0);
#ifdef _DEBUG
    if (!r)
    {
        EckDbgPrint(L"CompareStringEx failed.");
        EckDbgBreak();
    }
#endif
    return r - 2;
}
EckInlineNd int LcsCompareLength(
    _In_NLS_string_(Len) PCWCH Str1,
    _In_NLS_string_(Len) PCWCH Str2,
    int Len,
    UINT uFlags,
    _In_opt_z_ PCWSTR pszLocaleName = LOCALE_NAME_USER_DEFAULT) noexcept
{
    return LcsCompareLength2(Str1, Len, Str2, Len, uFlags, pszLocaleName);
}

EckInlineNd int LcsFind(
    _In_NLS_string_(Len) PCWCH Str,
    int Len,
    _In_NLS_string_(SubLen) PCWCH SubStr,
    int SubLen,
    UINT uFlags,
    _Out_opt_ int* pcchMatch = nullptr,
    _In_opt_z_ PCWSTR pszLocaleName = LOCALE_NAME_USER_DEFAULT) noexcept
{
    const auto r = FindNLSStringEx(pszLocaleName, uFlags,
        Str, Len, SubStr, SubLen, pcchMatch, nullptr, nullptr, 0);
#ifdef _DEBUG
    if (r == -1)
    {
        const auto dwErr = NaGetLastError();
        EckDbgPrintFormat(L"FindNLSStringEx failed. Error code: %d", dwErr);
        EckDbgPrintFormatMessage(dwErr);
        EckDbgBreak();
    }
#endif
    return r;
}

EckInlineNd int LcsStringLength(
    _In_NLS_string_(Len) PCWCH Str,
    int Len,
    _In_NLS_string_(SubLen) PCWCH SubStr,
    int SubLen,
    UINT uFlags = 0,
    _Out_opt_ int* pcchMatch = nullptr,
    _In_opt_z_ PCWSTR pszLocaleName = LOCALE_NAME_USER_DEFAULT) noexcept
{
    return LcsFind(Str, Len, SubStr, SubLen,
        uFlags | FIND_FROMSTART, pcchMatch, pszLocaleName);
}
EckInlineNd int LcsStringLengthI(
    _In_NLS_string_(Len) PCWCH Str,
    int Len,
    _In_NLS_string_(SubLen) PCWCH SubStr,
    int SubLen,
    UINT uFlags = 0,
    _Out_opt_ int* pcchMatch = nullptr,
    _In_opt_z_ PCWSTR pszLocaleName = LOCALE_NAME_USER_DEFAULT) noexcept
{
    return LcsFind(Str, Len, SubStr, SubLen,
        uFlags | FIND_FROMSTART | NORM_IGNORECASE, pcchMatch, pszLocaleName);
}

EckInlineNd int LcsRStringLength(
    _In_NLS_string_(Len) PCWCH Str,
    int Len,
    _In_NLS_string_(SubLen) PCWCH SubStr,
    int SubLen,
    UINT uFlags = 0,
    _Out_opt_ int* pcchMatch = nullptr,
    _In_opt_z_ PCWSTR pszLocaleName = LOCALE_NAME_USER_DEFAULT) noexcept
{
    return LcsFind(Str, Len, SubStr, SubLen,
        uFlags | FIND_FROMEND, pcchMatch, pszLocaleName);
}
EckInlineNd int LcsRStringLengthI(
    _In_NLS_string_(Len) PCWCH Str,
    int Len,
    _In_NLS_string_(SubLen) PCWCH SubStr,
    int SubLen,
    UINT uFlags = 0,
    _Out_opt_ int* pcchMatch = nullptr,
    _In_opt_z_ PCWSTR pszLocaleName = LOCALE_NAME_USER_DEFAULT) noexcept
{
    return LcsFind(Str, Len, SubStr, SubLen,
        uFlags | FIND_FROMEND | NORM_IGNORECASE, pcchMatch, pszLocaleName);
}

EckInlineNd int LcsIsStartWith(
    _In_NLS_string_(Len) PCWCH Str,
    int Len,
    _In_NLS_string_(SubLen) PCWCH SubStr,
    int SubLen,
    UINT uFlags = 0,
    _Out_opt_ int* pcchMatch = nullptr,
    _In_opt_z_ PCWSTR pszLocaleName = LOCALE_NAME_USER_DEFAULT) noexcept
{
    return LcsFind(Str, Len, SubStr, SubLen,
        uFlags | FIND_STARTSWITH, pcchMatch, pszLocaleName);
}
EckInlineNd int LcsIsStartWithI(
    _In_NLS_string_(Len) PCWCH Str,
    int Len,
    _In_NLS_string_(SubLen) PCWCH SubStr,
    int SubLen,
    UINT uFlags = 0,
    _Out_opt_ int* pcchMatch = nullptr,
    _In_opt_z_ PCWSTR pszLocaleName = LOCALE_NAME_USER_DEFAULT) noexcept
{
    return LcsFind(Str, Len, SubStr, SubLen,
        uFlags | FIND_STARTSWITH | NORM_IGNORECASE, pcchMatch, pszLocaleName);
}

EckInlineNd int LcsIsEndWith(
    _In_NLS_string_(Len) PCWCH Str,
    int Len,
    _In_NLS_string_(SubLen) PCWCH SubStr,
    int SubLen,
    UINT uFlags = 0,
    _Out_opt_ int* pcchMatch = nullptr,
    _In_opt_z_ PCWSTR pszLocaleName = LOCALE_NAME_USER_DEFAULT) noexcept
{
    return LcsFind(Str, Len, SubStr, SubLen,
        uFlags | FIND_ENDSWITH, pcchMatch, pszLocaleName);
}
EckInlineNd int LcsIsEndWithI(
    _In_NLS_string_(Len) PCWCH Str,
    int Len,
    _In_NLS_string_(SubLen) PCWCH SubStr,
    int SubLen,
    UINT uFlags = 0,
    _Out_opt_ int* pcchMatch = nullptr,
    _In_opt_z_ PCWSTR pszLocaleName = LOCALE_NAME_USER_DEFAULT) noexcept
{
    return LcsFind(Str, Len, SubStr, SubLen,
        uFlags | FIND_ENDSWITH | NORM_IGNORECASE, pcchMatch, pszLocaleName);
}

EckNfInlineNd int DbcsCountChar(_In_NLS_string_(cch) PCCH psz, int cch, UINT uCp = CP_ACP) noexcept
{
    if (cch < 0)
        cch = (int)strlen(psz);
    int c = 0;
    for (auto p = psz; p < psz + cch; ++c)
    {
        if (IsDBCSLeadByteEx(uCp, *p))
            p += 2;
        else
            p += 1;
    }
    return c;
}
ECK_NAMESPACE_END