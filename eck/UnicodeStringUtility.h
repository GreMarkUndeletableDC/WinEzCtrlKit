#pragma once
#include "Utility.h"

ECK_NAMESPACE_BEGIN
namespace Detail
{
    // http://bjoern.hoehrmann.de/utf-8/decoder/dfa/
    /*
    Copyright (c) 2008-2009 Bjoern Hoehrmann <bjoern@hoehrmann.de>
    Permission is hereby granted, free of charge, to any person obtaining a copy of this
    software and associated documentation files (the "Software"), to deal in the Software
    without restriction, including without limitation the rights to use, copy, modify,
    merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
    permit persons to whom the Software is furnished to do so, subject to the following
    conditions:
    The above copyright notice and this permission notice shall be included in all copies
    or substantial portions of the Software.
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
    INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
    PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
    LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
    OR OTHER DEALINGS IN THE SOFTWARE.
    */
    constexpr inline BYTE Utf8Dfa[]
    {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 00..1f
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 20..3f
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 40..5f
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 60..7f
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, // 80..9f
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, // a0..bf
        8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, // c0..df
        0xa,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x4,0x3,0x3, // e0..ef
        0xb,0x6,0x6,0x6,0x5,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8, // f0..ff
        0x0,0x1,0x2,0x3,0x5,0x8,0x7,0x1,0x1,0x1,0x4,0x6,0x1,0x1,0x1,0x1, // s0..s0
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1, // s1..s2
        1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1, // s3..s4
        1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,3,1,3,1,1,1,1,1,1, // s5..s6
        1,3,1,1,1,1,1,3,1,3,1,1,1,1,1,1,1,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // s7..s8
    };
}

enum : BYTE
{
    UTF8DC_ACCEPT,
    UTF8DC_REJECT,
};

inline void Ucs8DecodeChar(
    _Inout_ BYTE& byState, _Inout_ UINT& uCodePoint, UINT byUtf8)
{
    const UINT Type = Detail::Utf8Dfa[byUtf8];
    uCodePoint = (byState != UTF8DC_ACCEPT) ?
        (byUtf8 & 0x3fu) | (uCodePoint << 6) :
        (0xff >> Type) & (byUtf8);
    byState = Detail::Utf8Dfa[256 + byState * 16 + Type];
}

/// <summary>
/// 计算UTF8序列的码点个数
/// </summary>
/// <param name="psz、cch">输入UTF8序列</param>
/// <param name="posInvalid">第一个无效字节的位置，如果整个序列有效则返回负值</param>
/// <returns>返回已扫描出的码点个数，函数在第一个无效字节位置终止扫描</returns>
EckNfInlineNdCe int Ucs8CountChar(
    _In_NLS_string_(cch) PCCH psz,
    int cch,
    _Out_ int& posInvalid) noexcept
{
    if (cch < 0)
        cch = (int)strlen(psz);
    UINT Dummy{};
    BYTE byState{};
    int c{};
    const auto pEnd = psz + cch;
    auto p = psz;
    for (; p < pEnd; ++p)
    {
        Ucs8DecodeChar(byState, Dummy, (BYTE)*p);
        if (byState == UTF8DC_ACCEPT)
            ++c;
        else if (byState == UTF8DC_REJECT)
        {
            posInvalid = int(p - psz);
            return c;
        }
    }
    posInvalid = (byState == UTF8DC_ACCEPT) ? -1 : int(p - psz);
    return c;
}

EckInlineNd bool Ucs8IsValid(_In_NLS_string_(cch) PCCH psz, int cch) noexcept
{
    int posInvalid;
    (void)Ucs8CountChar(psz, cch, posInvalid);
    return posInvalid < 0;
}

/// <summary>
/// 计算UTF16序列的码点个数
/// </summary>
/// <param name="psz、cch">输入UTF16序列</param>
/// <param name="posInvalid">第一个无效WCHAR的位置，如果整个序列有效则返回负值</param>
/// <returns>返回已扫描出的码点个数，函数在第一个无效WCHAR位置终止扫描</returns>
EckNfInlineNd int Ucs16CountChar(
    _In_NLS_string_(cch) PCWCH psz,
    int cch,
    _Out_ int& posInvalid) noexcept
{
    if (cch < 0)
        cch = (int)wcslen(psz);
    const auto pEnd = psz + cch;
    int c{};
    for (auto p = psz; p < pEnd; ++p, ++c)
    {
        if (IS_HIGH_SURROGATE(*p))
        {
            const auto pNext = p + 1;
            if (pNext >= pEnd || !IS_LOW_SURROGATE(*pNext))
            {
                posInvalid = int(p - psz);
                return c;
            }
            ++p;
        }
        else if (IS_LOW_SURROGATE(*p))
        {
            posInvalid = int(p - psz);
            return c;
        }
    }
    posInvalid = -1;
    return c;
}

EckInlineNd bool Ucs16IsValid(_In_NLS_string_(cch) PCWCH psz, int cch) noexcept
{
    int posInvalid;
    (void)Ucs16CountChar(psz, cch, posInvalid);
    return posInvalid < 0;
}

/// <summary>
/// UTF16LE到UTF32
/// </summary>
/// <param name="psz、cch">输入UTF16LE序列</param>
/// <param name="pOut">输出缓冲区，必须提供尺寸至少为cch的缓冲区，**不会**添加结尾NULL</param>
/// <param name="bLittleEndian">输出UTF32序列的端序</param>
/// <param name="posInvalid">第一个无效WCHAR的位置，如果整个序列有效则返回负值</param>
/// <returns>输出UTF32序列的长度，失败返回0
/// </returns>
inline int Ucs16ConvertToUcs32(
    _In_NLS_string_(cch) PCWCH psz,
    int cch,
    _Out_writes_(cch) UINT* pOut,
    bool bLittleEndian,
    _Out_ int& posInvalid) noexcept
{
    if (cch < 0)
        cch = (int)wcslen(psz);
    const auto pOutOrg = pOut;
    const auto pEnd = psz + cch;
    for (auto p = psz; p < pEnd; ++p)
    {
        UINT u;
        if (IS_HIGH_SURROGATE(*p))
        {
            const auto pNext = p + 1;
            if (pNext >= pEnd || !IS_LOW_SURROGATE(*pNext))
            {
                posInvalid = int(p - psz);
                return 0;
            }
            u = 0x10000 + (((*p - 0xD800) << 10) | (*pNext - 0xDC00));
            ++p;
        }
        else if (IS_LOW_SURROGATE(*p))
        {
            posInvalid = int(p - psz);
            return 0;
        }
        else
            u = *p;
        *pOut++ = (bLittleEndian ? u : ReverseInteger(u));
    }
    posInvalid = -1;
    return int(pOut - pOutOrg);
}

inline void Ucs16ReverseByteOrder(_Inout_updates_(cch) PWCH psz, int cch) noexcept
{
    const auto pEnd = psz + cch;
    for (; psz < pEnd; ++psz)
        *psz = _byteswap_ushort(*psz);
}
ECK_NAMESPACE_END