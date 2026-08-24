#pragma once
#include "StringUtility.h"

ECK_NAMESPACE_BEGIN
inline constexpr std::wstring_view
IllegalPathCharW{ LR"(\/:*?"<>|)" },
IllegalPathCharWithDotW{ LR"(\/:*?"<>|.)" };
inline constexpr std::string_view
IllegalPathCharA{ R"(\/:*?"<>|)" },
IllegalPathCharWithDotA{ R"(\/:*?"<>|.)" };

template<CcpCharPointer TPointer>
void PazLegalize(_In_z_ TPointer pszPath,
    CharFromPointer_T<TPointer> chReplace = '_', BOOL bReplaceDot = FALSE) noexcept
{
    if constexpr (std::is_same_v<CharFromPointer_T<TPointer>, char>)
    {
        const auto IllegalChars{ bReplaceDot ? IllegalPathCharA : IllegalPathCharWithDotA };
        auto p{ pszPath };
        while (p = TcsCharFirstOf(p, IllegalChars.data()))
            *p++ = chReplace;
    }
    else
    {
        const auto IllegalChars{ bReplaceDot ? IllegalPathCharW : IllegalPathCharWithDotW };
        auto p{ pszPath };
        while (p = TcsCharFirstOf(p, IllegalChars.data()))
            *p++ = chReplace;
    }
}
template<CcpCharPointer TPointer>
void PazLegalizeLength(_In_reads_(cchPath) TPointer pszPath, int cchPath,
    CharFromPointer_T<TPointer> chReplace = '_', BOOL bReplaceDot = FALSE) noexcept
{
    if constexpr (std::is_same_v<CharFromPointer_T<TPointer>, char>)
    {
        const auto IllegalChars{ bReplaceDot ? IllegalPathCharA : IllegalPathCharWithDotA };
        const auto pEnd = pszPath + cchPath;
        auto p{ pszPath };
        while (p = TcsCharFirstOf(p, pEnd - p, IllegalChars.data(), IllegalChars.size()))
            *p++ = chReplace;
    }
    else
    {
        const auto IllegalChars{ bReplaceDot ? IllegalPathCharW : IllegalPathCharWithDotW };
        const auto pEnd = pszPath + cchPath;
        auto p{ pszPath };
        while (p = TcsCharFirstOf(p, pEnd - p, IllegalChars.data(), IllegalChars.size()))
            *p++ = chReplace;
    }
}

template<CcpCharPointer TPointer>
HRESULT PazParseCommandLine(
    _In_reads_(cchCmdLine) TPointer pszCmdLine,
    int cchCmdLine,
    _Out_ TPointer& pszFile,
    _Out_ int& cchFile,
    _Out_ TPointer& pszParam,
    _Out_ int& cchParam) noexcept
{
    if (!pszCmdLine || !cchCmdLine)
    {
        pszFile = pszParam = nullptr;
        cchFile = cchParam = 0;
        return S_FALSE;
    }
    pszFile = pszCmdLine;
    BOOL bQuote = (*pszCmdLine == '\"');
    if (bQuote)
        ++pszFile;
    const auto pEnd = pszCmdLine + cchCmdLine;
    if (bQuote)
    {
        for (auto p = pszFile; p != pEnd; ++p)
            if (*p == '\"')
            {
                cchFile = int(p - pszFile);
                goto FileNameOk;
            }
        cchFile = 0;// 引号不匹配
        return HRESULT_FROM_WIN32(ERROR_INVALID_COMMAND_LINE);
    }
    else
    {
        for (auto p = pszFile; p != pEnd; ++p)
            if (*p == ' ')
            {
                cchFile = int(p - pszFile);
                goto FileNameOk;
            }
        cchFile = cchCmdLine;
        pszParam = nullptr;
        cchParam = 0;
        return S_OK;
    }
FileNameOk:;// 至此文件名处理完毕
    // 步进到第一个非空格字符
    pszParam = pszFile + cchFile;
    if (*pszParam == '\"')
        ++pszParam;
    for (; pszParam != pEnd; ++pszParam)
        if (*pszParam != ' ')
            break;
    cchParam = int(pEnd - pszParam);
    return S_OK;
}
template<CcpChar TChar>
HRESULT PazParseCommandLine(
    std::basic_string_view<TChar> svCmdLine,
    Eck_Out_buffer_ std::basic_string_view<TChar>& svFile,
    Eck_Out_buffer_ std::basic_string_view<TChar>& svParam) noexcept
{
    const TChar* pszFile, *pszParam;
    int cchFile, cchParam;
    const auto hr = PazParseCommandLine(
        svCmdLine.data(), (int)svCmdLine.size(),
        pszFile, cchFile, pszParam, cchParam);
    if (FAILED(hr))
    {
        svFile = svParam = {};
        return hr;
    }

    if (!pszFile)
        svFile = {};
    else
        svFile = { pszFile, (size_t)cchFile };

    if (!pszParam)
        svParam = {};
    else
        svParam = { pszParam, (size_t)cchParam };
    return S_OK;
}

template<CcpNonConstCharPointer TPointer>
HRESULT PazParseCommandLineAndCut(
    _Inout_updates_(cchCmdLine + 1) TPointer pszCmdLine,
    int cchCmdLine,
    _Out_ TPointer& pszFile,
    _Out_ int& cchFile,
    _Out_ TPointer& pszParam,
    _Out_ int& cchParam) noexcept
{
    EckAssert(&pszFile != &pszParam && &cchFile != &cchParam);
    const auto hr = PazParseCommandLineAndCut(
        pszCmdLine, cchCmdLine,
        pszFile, cchFile, pszParam, cchParam);
    if (SUCCEEDED(hr))
    {
        if (pszFile)
            *(pszFile + cchFile) = '\0';
        if (pszParam)
            *(pszParam + cchParam) = '\0';
    }
    return hr;
}
template<CcpNonConstCharPointer TPointer, class TChar = CharFromPointer_T<TPointer>>
HRESULT PazParseCommandLineAndCut(
    _Inout_updates_(cchCmdLine + 1) TPointer pszCmdLine,
    int cchCmdLine,
    Eck_Out_buffer_ std::basic_string_view<TChar>& svFile,
    Eck_Out_buffer_ std::basic_string_view<TChar>& svParam) noexcept
{
    TChar* pszFile, *pszParam;
    int cchFile, cchParam;
    const auto hr = PazParseCommandLineAndCut(
        pszCmdLine, cchCmdLine,
        pszFile, cchFile, pszParam, cchParam);
    if (FAILED(hr))
    {
        svFile = svParam = {};
        return hr;
    }

    if (!pszFile)
        svFile = {};
    else
        svFile = { pszFile, (size_t)cchFile };

    if (!pszParam)
        svParam = {};
    else
        svParam = { pszParam, (size_t)cchParam };
    return S_OK;
}

EckNfInlineNd BOOL PazIsDotFileName(_In_reads_z_(3) CcpCharPointer auto pszFileName) noexcept
{
    return pszFileName[0] == '.' && (pszFileName[1] == '\0' ||
        (pszFileName[1] == '.' && pszFileName[2] == '\0'));
}

EckNfInlineNd BOOL PazIsDotFileName(
    _In_reads_bytes_(cbFileName) PCWCH pszFileName,
    ULONG cbFileName) noexcept
{
    return (cbFileName == sizeof(WCHAR) && pszFileName[0] == L'.') ||
        (cbFileName == sizeof(WCHAR) * 2 && pszFileName[0] == L'.' && pszFileName[1] == L'.');
}

// 返回文件名的位置，注意：若分隔符在开头则返回-1
EckNfInlineNd int PazFindFileName(
    _In_reads_or_z_(cchPath) CcpCharPointer auto pszPath,
    int cchPath) noexcept
{
    if (cchPath < 0)
        cchPath = (int)TcsLength(pszPath);
    if (!pszPath || !cchPath)
        return -1;
    auto pEnd = pszPath + cchPath - 1;
    if (*pEnd == '\\' || *pEnd == '/')
        --pEnd;// 如果以反斜杠结尾，则跳过
    for (auto p = pEnd; p != pszPath; --p)
    {
        const auto ch = *p;
        if (ch == '\\' || ch == '/')
        {
            if (p < pszPath + 2)// NT路径或UNC路径的起始
                return -1;
            return int(p + 1 - pszPath);
        }
    }
    return -1;
}

EckNfInlineNd int PazFindExtension(
    _In_reads_(cchPath) PCWSTR pszPath,
    int cchPath) noexcept
{
    if (cchPath < 0)
        cchPath = (int)TcsLength(pszPath);
    if (!pszPath || !cchPath)
        return -1;
    int pos{ -1 };
    for (auto p = pszPath + cchPath - 1; p != pszPath; --p)
    {
        const auto ch = *p;
        if (ch == '.')
            return int(p - pszPath);
        else if (ch == ' ' /*扩展名内不能有空格*/ ||
            ch == '\\' || ch == '/')
            return -1;
    }
    return -1;
}
ECK_NAMESPACE_END