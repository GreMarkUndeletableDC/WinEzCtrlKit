#include "pch.h"
#include "../eck/LocaleStringUtility.h"

#include <climits>
#include <initializer_list>

using namespace eck;

TS_NS_BEGIN

// 校验Ucs8CountChar：返回的码点数与首个非法字节位置
static void AssertUcs8(const char* psz, int cch, int cchExpected, int posInvalidExpected)
{
    int posInvalid = INT_MIN;
    const auto cchActual = Ucs8CountChar(psz, cch, posInvalid);
    Assert::AreEqual(cchExpected, cchActual);
    Assert::AreEqual(posInvalidExpected, posInvalid);
}

// 校验Ucs16CountChar：返回的码点数与首个非法WCHAR位置
static void AssertUcs16(const wchar_t* psz, int cch, int cchExpected, int posInvalidExpected)
{
    int posInvalid = INT_MIN;
    const auto cchActual = Ucs16CountChar(psz, cch, posInvalid);
    Assert::AreEqual(cchExpected, cchActual);
    Assert::AreEqual(posInvalidExpected, posInvalid);
}

// 逐字节驱动UTF8 DFA
static void DecodeUcs8(BYTE& byState, UINT& uCodePoint, std::initializer_list<BYTE> il)
{
    for (BYTE by : il)
        Ucs8DecodeChar(byState, uCodePoint, by);
}

TEST_CLASS(TsLocaleStringUtility)
{
public:
    //============================================================
    // Ucs8DecodeChar
    //============================================================
    TEST_METHOD(TsUcs8DecodeChar)
    {
        BYTE byState = UTF8DC_ACCEPT;
        UINT u = 0;

        // 1字节
        Ucs8DecodeChar(byState, u, 0x41);
        Assert::AreEqual(0x41u, u);
        Assert::AreEqual((UINT)UTF8DC_ACCEPT, (UINT)byState);

        // 2字节，U+00E9
        byState = UTF8DC_ACCEPT;
        u = 0;
        DecodeUcs8(byState, u, { 0xC3, 0xA9 });
        Assert::AreEqual(0xE9u, u);
        Assert::AreEqual((UINT)UTF8DC_ACCEPT, (UINT)byState);

        // 3字节，U+4E16 世
        byState = UTF8DC_ACCEPT;
        u = 0;
        DecodeUcs8(byState, u, { 0xE4, 0xB8, 0x96 });
        Assert::AreEqual(0x4E16u, u);
        Assert::AreEqual((UINT)UTF8DC_ACCEPT, (UINT)byState);

        // 4字节，U+1F600
        byState = UTF8DC_ACCEPT;
        u = 0;
        DecodeUcs8(byState, u, { 0xF0, 0x9F, 0x98, 0x80 });
        Assert::AreEqual(0x1F600u, u);
        Assert::AreEqual((UINT)UTF8DC_ACCEPT, (UINT)byState);

        // 孤立续字节直接进入拒绝状态
        byState = UTF8DC_ACCEPT;
        u = 0;
        DecodeUcs8(byState, u, { 0x80 });
        Assert::AreEqual((UINT)UTF8DC_REJECT, (UINT)byState);

        // 非法起始字节直接进入拒绝状态
        byState = UTF8DC_ACCEPT;
        u = 0;
        DecodeUcs8(byState, u, { 0xFF });
        Assert::AreEqual((UINT)UTF8DC_REJECT, (UINT)byState);
    }

    //============================================================
    // Ucs8CountChar
    //============================================================
    TEST_METHOD(TsUcs8CountCharEmpty)
    {
        AssertUcs8("", 0, 0, -1);
        AssertUcs8("", -1, 0, -1);
    }

    TEST_METHOD(TsUcs8CountCharAscii)
    {
        AssertUcs8("ABC", 3, 3, -1);
        AssertUcs8("ABC", -1, 3, -1);
        AssertUcs8("ABC", 2, 2, -1);    // 显式长度截断
        AssertUcs8("ABC", 0, 0, -1);

        // 显式长度内NUL按普通字符计数
        const char szNul[] = { 'A', '\0', 'B' };
        AssertUcs8(szNul, (int)ARRAYSIZE(szNul), 3, -1);
        // cch为负时按NUL结尾
        AssertUcs8(szNul, -1, 1, -1);
    }

    TEST_METHOD(TsUcs8CountCharLength)
    {
        AssertUcs8("\xC3\xA9", -1, 1, -1);                          // U+00E9，2字节
        AssertUcs8("\xE4\xB8\x96", -1, 1, -1);                      // U+4E16，3字节
        AssertUcs8("\xF0\x9F\x98\x80", -1, 1, -1);                  // U+1F600，4字节

        AssertUcs8("A\xC3\xA9", 3, 2, -1);
        AssertUcs8("A\xF0\x9F\x98\x80\x42", 6, 3, -1);
        AssertUcs8("A\xC3\xA9\xE4\xB8\x96\xF0\x9F\x98\x80", -1, 4, -1);
        AssertUcs8("\xF0\x9F\x98\x80\xF0\x9F\x98\x80", -1, 2, -1);
    }

    TEST_METHOD(TsUcs8CountCharBoundary)
    {
        // 各长度合法边界码点
        AssertUcs8("\x7F", -1, 1, -1);              // U+007F
        AssertUcs8("\xC2\x80", -1, 1, -1);          // U+0080
        AssertUcs8("\xDF\xBF", -1, 1, -1);          // U+07FF
        AssertUcs8("\xE0\xA0\x80", -1, 1, -1);      // U+0800
        AssertUcs8("\xED\x9F\xBF", -1, 1, -1);      // U+D7FF
        AssertUcs8("\xEE\x80\x80", -1, 1, -1);      // U+E000
        AssertUcs8("\xEF\xBF\xBF", -1, 1, -1);      // U+FFFF
        AssertUcs8("\xF0\x90\x80\x80", -1, 1, -1);  // U+10000
        AssertUcs8("\xF4\x8F\xBF\xBF", -1, 1, -1);  // U+10FFFF
    }

    TEST_METHOD(TsUcs8CountCharInvalidStart)
    {
        // 孤立续字节
        AssertUcs8("\x80", -1, 0, 0);
        AssertUcs8("\x8F", -1, 0, 0);
        AssertUcs8("\x90", -1, 0, 0);
        AssertUcs8("\xBF", -1, 0, 0);
        // 超长编码起始字节
        AssertUcs8("\xC0\xAF", -1, 0, 0);
        AssertUcs8("\xC1\xBF", -1, 0, 0);
        // 超出U+10FFFF的起始字节
        AssertUcs8("\xF5\x80\x80\x80", -1, 0, 0);
        AssertUcs8("\xF7\xBF\xBF\xBF", -1, 0, 0);
        AssertUcs8("\xFF", -1, 0, 0);
    }

    TEST_METHOD(TsUcs8CountCharOverlong)
    {
        // 3字节超长编码
        AssertUcs8("\xE0\x80\xAF", -1, 0, 1);
        AssertUcs8("\xE0\x9F\xBF", -1, 0, 1);
        // 4字节超长编码
        AssertUcs8("\xF0\x80\x80\xAF", -1, 0, 1);
        AssertUcs8("\xF0\x8F\xBF\xBF", -1, 0, 1);
    }

    TEST_METHOD(TsUcs8CountCharSurrogate)
    {
        // UTF8不允许编码代理码点
        AssertUcs8("\xED\xA0\x80", -1, 0, 1);   // U+D800
        AssertUcs8("\xED\xBF\xBF", -1, 0, 1);   // U+DFFF
    }

    TEST_METHOD(TsUcs8CountCharOutOfRange)
    {
        AssertUcs8("\xF4\x90\x80\x80", -1, 0, 1);   // U+110000
        AssertUcs8("\xF4\xBF\xBF\xBF", -1, 0, 1);
    }

    TEST_METHOD(TsUcs8CountCharTruncated)
    {
        // 尾部截断：不产生码点，posInvalid指向序列末尾
        AssertUcs8("\xC3", -1, 0, 1);
        AssertUcs8("\xE4\xB8", -1, 0, 2);
        AssertUcs8("\xF0\x9F\x98", -1, 0, 3);
        // 前缀合法，尾部截断
        AssertUcs8("A\xC3", 2, 1, 2);
        AssertUcs8("A\xE4\xB8", 3, 1, 3);
    }

    TEST_METHOD(TsUcs8CountCharBadContinuation)
    {
        // 续字节位置出现ASCII
        AssertUcs8("\xC3\x41", 2, 0, 1);
        AssertUcs8("\xE4\x41", 2, 0, 1);
        AssertUcs8("\xF0\x9F\x41", 3, 0, 2);
        // 合法前缀之后的错误仍按绝对位置报告
        AssertUcs8("A\xC3\x41", 3, 1, 2);
        AssertUcs8("A\xE4\xB8\x28", 4, 1, 3);
    }

    TEST_METHOD(TsUcs8IsValid)
    {
        Assert::IsTrue(Ucs8IsValid("", 0));
        Assert::IsTrue(Ucs8IsValid("ABC", -1));
        Assert::IsTrue(Ucs8IsValid("A\xC3\xA9\xE4\xB8\x96\xF0\x9F\x98\x80", -1));

        Assert::IsFalse(Ucs8IsValid("\x80", -1));
        Assert::IsFalse(Ucs8IsValid("\xC3", -1));
        Assert::IsFalse(Ucs8IsValid("\xED\xA0\x80", -1));
        Assert::IsFalse(Ucs8IsValid("\xF4\x90\x80\x80", -1));
        Assert::IsFalse(Ucs8IsValid("\xF0\x8F\xBF\xBF", -1));
        Assert::IsFalse(Ucs8IsValid("A\xC3\x41", 3));
        // 截断在合法字符中间
        Assert::IsFalse(Ucs8IsValid("A\xC3\xA9", 2));
    }

    TEST_METHOD(TsUcs8IsValidConsistency)
    {
        const char* const asz[] =
        {
            "",
            "A",
            "\xC3\xA9",
            "A\xC3\xA9\xE4\xB8\x96",
            "\x80",
            "\xC3",
            "\xE0\x80\xAF",
            "\xE0\x9F\xBF",
            "\xED\xA0\x80",
            "\xF4\x90\x80\x80",
            "A\xE4\xB8\x28",
        };
        for (auto psz : asz)
        {
            int posInvalid;
            (void)Ucs8CountChar(psz, -1, posInvalid);
            Assert::AreEqual(posInvalid < 0, Ucs8IsValid(psz, -1));
        }
    }

    //============================================================
    // Ucs16CountChar
    //============================================================
    TEST_METHOD(TsUcs16CountCharBasic)
    {
        AssertUcs16(L"", 0, 0, -1);
        AssertUcs16(L"", -1, 0, -1);

        AssertUcs16(L"ABC", -1, 3, -1);
        AssertUcs16(L"ABC", 2, 2, -1);  // 显式长度截断
        AssertUcs16(L"ABC", 0, 0, -1);

        // 显式长度内NUL按普通字符计数
        const wchar_t szNul[] = { L'A', 0, L'B' };
        AssertUcs16(szNul, (int)ARRAYSIZE(szNul), 3, -1);
        // cch为负时按NUL结尾
        AssertUcs16(szNul, -1, 1, -1);

        // BMP内的普通字符，含代理区边界外侧
        const wchar_t szBmp[] = { 0xD7FF, 0xE000, 0xFFFF };
        AssertUcs16(szBmp, (int)ARRAYSIZE(szBmp), 3, -1);
    }

    TEST_METHOD(TsUcs16CountCharSurrogatePair)
    {
        AssertUcs16(L"\xD83D\xDE00", 2, 1, -1);                          // U+1F600
        AssertUcs16(L"\xD800\xDC00", 2, 1, -1);                          // U+10000
        AssertUcs16(L"\xDBFF\xDFFF", 2, 1, -1);                          // U+10FFFF
        AssertUcs16(L"\xD83D\xDE00\xD83D\xDE00", 4, 2, -1);              // 连续两个代理对
        AssertUcs16(L"A\xD83D\xDE00\x4E16", 4, 3, -1);                   // 与BMP混排
        AssertUcs16(L"A\xD83D\xDE00\x4E16", -1, 3, -1);                  // cch为负
    }

    TEST_METHOD(TsUcs16CountCharLoneSurrogate)
    {
        const wchar_t szHighAtStart[] = { 0xD800, L'A' };
        AssertUcs16(szHighAtStart, (int)ARRAYSIZE(szHighAtStart), 0, 0);

        const wchar_t szHighAtEnd[] = { L'A', 0xD800 };
        AssertUcs16(szHighAtEnd, (int)ARRAYSIZE(szHighAtEnd), 1, 1);

        const wchar_t szLowAtStart[] = { 0xDC00, L'A' };
        AssertUcs16(szLowAtStart, (int)ARRAYSIZE(szLowAtStart), 0, 0);

        const wchar_t szLowMid[] = { L'A', 0xDC00 };
        AssertUcs16(szLowMid, (int)ARRAYSIZE(szLowMid), 1, 1);

        // 高代理后接高代理
        const wchar_t szHighHigh[] = { 0xD800, 0xD800 };
        AssertUcs16(szHighHigh, (int)ARRAYSIZE(szHighHigh), 0, 0);

        // 合法代理对之后出现孤立高代理
        const wchar_t szPairThenHigh[] = { 0xD800, 0xDC00, 0xD800 };
        AssertUcs16(szPairThenHigh, (int)ARRAYSIZE(szPairThenHigh), 1, 2);

        // 合法字符之后出现孤立低代理
        const wchar_t szLowAfterText[] = { L'A', L'B', 0xDFFF };
        AssertUcs16(szLowAfterText, (int)ARRAYSIZE(szLowAfterText), 2, 2);
    }

    TEST_METHOD(TsUcs16IsValid)
    {
        Assert::IsTrue(Ucs16IsValid(L"", 0));
        Assert::IsTrue(Ucs16IsValid(L"ABC", -1));
        Assert::IsTrue(Ucs16IsValid(L"A\xD83D\xDE00\x4E16", -1));
        Assert::IsTrue(Ucs16IsValid(L"\xD800\xDC00", 2));

        const wchar_t szLoneHigh[] = { L'A', 0xD800 };
        Assert::IsFalse(Ucs16IsValid(szLoneHigh, (int)ARRAYSIZE(szLoneHigh)));

        const wchar_t szLoneLow[] = { L'A', 0xDC00 };
        Assert::IsFalse(Ucs16IsValid(szLoneLow, (int)ARRAYSIZE(szLoneLow)));

        const wchar_t szHighHigh[] = { 0xD800, 0xD800 };
        Assert::IsFalse(Ucs16IsValid(szHighHigh, (int)ARRAYSIZE(szHighHigh)));
    }

    //============================================================
    // Ucs16ConvertToUcs32
    //============================================================
    TEST_METHOD(TsUcs16ConvertToUcs32Basic)
    {
        UINT out[8]{};
        int posInvalid;

        // 空序列：成功，输出长度为0
        Assert::AreEqual(0, Ucs16ConvertToUcs32(L"", 0, out, true, posInvalid));
        Assert::AreEqual(-1, posInvalid);
        Assert::AreEqual(0u, out[0]);   // 未写入

        Assert::AreEqual(2, Ucs16ConvertToUcs32(L"AB", 2, out, true, posInvalid));
        Assert::AreEqual(-1, posInvalid);
        Assert::AreEqual(0x41u, out[0]);
        Assert::AreEqual(0x42u, out[1]);

        Assert::AreEqual(1, Ucs16ConvertToUcs32(L"\x4E16", -1, out, true, posInvalid));
        Assert::AreEqual(-1, posInvalid);
        Assert::AreEqual(0x4E16u, out[0]);

        Assert::AreEqual(1, Ucs16ConvertToUcs32(L"\xFFFF", 1, out, true, posInvalid));
        Assert::AreEqual(-1, posInvalid);
        Assert::AreEqual(0xFFFFu, out[0]);
    }

    TEST_METHOD(TsUcs16ConvertToUcs32Surrogate)
    {
        const wchar_t sz[] = L"A\xD83D\xDE00\x4E16";
        UINT out[4]{};
        int posInvalid;
        const auto cch = Ucs16ConvertToUcs32(sz, -1, out, true, posInvalid);
        Assert::AreEqual(3, cch);
        Assert::AreEqual(-1, posInvalid);
        Assert::AreEqual(0x41u, out[0]);
        Assert::AreEqual(0x1F600u, out[1]);
        Assert::AreEqual(0x4E16u, out[2]);

        // 输出长度应与Ucs16CountChar的码点数一致
        int posInvalid16;
        Assert::AreEqual(Ucs16CountChar(sz, -1, posInvalid16), cch);
        Assert::AreEqual(-1, posInvalid16);
    }

    TEST_METHOD(TsUcs16ConvertToUcs32BigEndian)
    {
        UINT out[4]{};
        int posInvalid;
        const auto cch = Ucs16ConvertToUcs32(L"A\xD83D\xDE00", -1, out, false, posInvalid);
        Assert::AreEqual(2, cch);
        Assert::AreEqual(-1, posInvalid);
        Assert::AreEqual(0x41000000u, out[0]);  // 0x00000041 字节翻转
        Assert::AreEqual(0x00F60100u, out[1]);  // 0x0001F600 字节翻转
    }

    TEST_METHOD(TsUcs16ConvertToUcs32Invalid)
    {
        // 低代理出现在末尾：返回0，posInvalid指出出错位置，已输出的前缀保留
        {
            const wchar_t sz[] = { L'A', 0xDC00 };
            UINT out[4]{};
            int posInvalid;
            Assert::AreEqual(0, Ucs16ConvertToUcs32(sz, (int)ARRAYSIZE(sz), out, true, posInvalid));
            Assert::AreEqual(1, posInvalid);
            Assert::AreEqual(0x41u, out[0]);
        }

        // 高代理位于末尾
        {
            const wchar_t sz[] = { L'A', L'B', 0xD800 };
            UINT out[4]{};
            int posInvalid;
            Assert::AreEqual(0, Ucs16ConvertToUcs32(sz, (int)ARRAYSIZE(sz), out, true, posInvalid));
            Assert::AreEqual(2, posInvalid);
            Assert::AreEqual(0x41u, out[0]);
            Assert::AreEqual(0x42u, out[1]);
        }

        // 高代理后接非低代理
        {
            const wchar_t sz[] = { L'A', 0xD800, L'B' };
            UINT out[4]{};
            int posInvalid;
            Assert::AreEqual(0, Ucs16ConvertToUcs32(sz, (int)ARRAYSIZE(sz), out, true, posInvalid));
            Assert::AreEqual(1, posInvalid);
        }

        // 高代理位于最后一个单元
        {
            const wchar_t sz[] = { L'A', 0xD800 };
            UINT out[4]{};
            int posInvalid;
            Assert::AreEqual(0, Ucs16ConvertToUcs32(sz, (int)ARRAYSIZE(sz), out, true, posInvalid));
            Assert::AreEqual(1, posInvalid);
        }

        // 起始位置的错误：返回值与空序列成功同为0，由posInvalid区分
        {
            const wchar_t sz[] = { 0xDC00, L'A' };
            UINT out[4]{};
            int posInvalid;
            Assert::AreEqual(0, Ucs16ConvertToUcs32(sz, (int)ARRAYSIZE(sz), out, true, posInvalid));
            Assert::AreEqual(0, posInvalid);
            Assert::AreEqual(0u, out[0]);   // 未写入
        }
    }

    TEST_METHOD(TsUcs16ConvertToUcs32NoOverrun)
    {
        const wchar_t sz[] = L"A\xD83D\xDE00";
        UINT out[4];
        int posInvalid;
        out[2] = 0xDEADBEEFu;
        out[3] = 0xDEADBEEFu;

        const auto cch = Ucs16ConvertToUcs32(sz, -1, out, true, posInvalid);
        Assert::AreEqual(2, cch);
        Assert::AreEqual(-1, posInvalid);
        Assert::AreEqual(0x41u, out[0]);
        Assert::AreEqual(0x1F600u, out[1]);
        // cch之外的元素不得被写入
        Assert::AreEqual(0xDEADBEEFu, out[2]);
        Assert::AreEqual(0xDEADBEEFu, out[3]);
    }

    //============================================================
    // Ucs16ReverseByteOrder
    //============================================================
    TEST_METHOD(TsUcs16ReverseByteOrder)
    {
        wchar_t sz[] = L"AB";
        Ucs16ReverseByteOrder(sz, 2);
        Assert::AreEqual(0x4100, (int)sz[0]);
        Assert::AreEqual(0x4200, (int)sz[1]);

        // 对偶操作应还原原始内容
        Ucs16ReverseByteOrder(sz, 2);
        Assert::AreEqual((int)L'A', (int)sz[0]);
        Assert::AreEqual((int)L'B', (int)sz[1]);
    }

    TEST_METHOD(TsUcs16ReverseByteOrderZeroLength)
    {
        wchar_t sz[] = L"AB";
        Ucs16ReverseByteOrder(sz, 0);
        Assert::AreEqual((int)L'A', (int)sz[0]);
        Assert::AreEqual((int)L'B', (int)sz[1]);
    }

    TEST_METHOD(TsUcs16ReverseByteOrderNoOverrun)
    {
        wchar_t sz[] = { L'A', L'B', L'C', L'\0' };
        Ucs16ReverseByteOrder(sz, 3);
        Assert::AreEqual(0x4100, (int)sz[0]);
        Assert::AreEqual(0x4200, (int)sz[1]);
        Assert::AreEqual(0x4300, (int)sz[2]);
        Assert::AreEqual(0, (int)sz[3]);    // cch之外不得被写入
    }

    TEST_METHOD(TsUcs16ReverseByteOrderSurrogate)
    {
        wchar_t sz[] = { (wchar_t)0xD83D, (wchar_t)0xDE00 };
        Ucs16ReverseByteOrder(sz, (int)ARRAYSIZE(sz));
        Assert::AreEqual(0x3DD8, (int)sz[0]);
        Assert::AreEqual(0x00DE, (int)sz[1]);
    }

    //============================================================
    // 交叉一致性
    //============================================================
    TEST_METHOD(TsUcs8Ucs16Consistent)
    {
        // 同一文本的UTF8与UTF16码点数应一致
        int pos8, pos16;
        const auto c8 = Ucs8CountChar("A\xE4\xB8\x96\xF0\x9F\x98\x80", -1, pos8);
        const auto c16 = Ucs16CountChar(L"A\x4E16\xD83D\xDE00", -1, pos16);
        Assert::AreEqual(3, c8);
        Assert::AreEqual(3, c16);
        Assert::AreEqual(-1, pos8);
        Assert::AreEqual(-1, pos16);
    }
};
TS_NS_END
