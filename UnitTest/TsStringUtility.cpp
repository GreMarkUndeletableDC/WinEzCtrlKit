#include "pch.h"
#include "../eck/StringUtility.h"

using namespace eck;

TS_NS_BEGIN

static void AssertView(std::wstring_view sv, std::wstring_view svExpected)
{
    Assert::AreEqual(svExpected.size(), sv.size());
    Assert::IsTrue(std::equal(sv.begin(), sv.end(), svExpected.begin(), svExpected.end()));
}

static void AssertView(std::string_view sv, std::string_view svExpected)
{
    Assert::AreEqual(svExpected.size(), sv.size());
    Assert::IsTrue(std::equal(sv.begin(), sv.end(), svExpected.begin(), svExpected.end()));
}

TEST_CLASS(TsStringUtility)
{
public:
    //============================================================
    // 常量
    //============================================================
    TEST_METHOD(TsSpaceChars)
    {
        Assert::AreEqual((size_t)4, ARRAYSIZE(SpaceCharsA) - 1);
        Assert::AreEqual((size_t)4, ARRAYSIZE(SpaceCharsW) - 1);
        Assert::IsTrue(wcscmp(SpaceCharsW, L" \t\r\n") == 0);
        // 窄集不含全角空格
        Assert::IsNull(wcschr(SpaceCharsW, L'\u3000'));
        // 全集包含全角空格、不换行空格等
        Assert::IsNotNull(wcschr(SpaceCharsFullW, L'\u3000'));
        Assert::IsNotNull(wcschr(SpaceCharsFullW, L'\u00A0'));
        Assert::IsNotNull(wcschr(SpaceCharsFullW, L'\uFEFF'));
        Assert::IsTrue(wcslen(SpaceCharsFullW) > wcslen(SpaceCharsW));
    }

    //============================================================
    // TchToUpper / TchToLower / TchIsAlpha / TchIsDigit
    //============================================================
    TEST_METHOD(TsTchToUpperLower)
    {
        Assert::IsTrue(TchToUpper('a') == 'A');
        Assert::IsTrue(TchToUpper('z') == 'Z');
        Assert::IsTrue(TchToUpper('A') == 'A');
        Assert::IsTrue(TchToUpper('0') == '0');
        Assert::IsTrue(TchToUpper('_') == '_');
        Assert::IsTrue(TchToLower('A') == 'a');
        Assert::IsTrue(TchToLower('Z') == 'z');
        Assert::IsTrue(TchToLower('a') == 'a');
        Assert::IsTrue(TchToLower('.') == '.');

        Assert::IsTrue(TchToUpper(L'a') == L'A');
        Assert::IsTrue(TchToUpper(L'z') == L'Z');
        Assert::IsTrue(TchToLower(L'A') == L'a');
        // 仅 ASCII：非 ASCII 字符保持不变（不依赖区域设置）
        Assert::IsTrue(TchToUpper(L'\u00E9') == L'\u00E9');   // é
        Assert::IsTrue(TchToLower(L'\u0130') == L'\u0130');   // İ
        Assert::IsTrue(TchToUpper(L'\u4E16') == L'\u4E16');   // 世
    }

    TEST_METHOD(TsTchIsAlphaDigit)
    {
        Assert::IsTrue(TchIsAlpha('A'));
        Assert::IsTrue(TchIsAlpha('z'));
        Assert::IsFalse(TchIsAlpha('0'));
        Assert::IsFalse(TchIsAlpha('_'));
        Assert::IsTrue(TchIsDigit('0'));
        Assert::IsTrue(TchIsDigit('9'));
        Assert::IsFalse(TchIsDigit('a'));
        Assert::IsTrue(TchIsAlphaDigit('9'));
        Assert::IsTrue(TchIsAlphaDigit('Q'));
        Assert::IsFalse(TchIsAlphaDigit('-'));

        Assert::IsTrue(TchIsAlpha(L'A'));
        Assert::IsTrue(TchIsAlpha(L'z'));
        Assert::IsTrue(TchIsDigit(L'7'));
        Assert::IsFalse(TchIsDigit(L'x'));
        Assert::IsTrue(TchIsAlphaDigit(L'5'));
        // ASCII 范围外不算字母/数字
        Assert::IsFalse(TchIsAlpha(L'\u00E9'));
        Assert::IsFalse(TchIsDigit(L'\uFF15'));
    }

    TEST_METHOD(TsTchEqual)
    {
        Assert::IsTrue(TchEqual('a', 'a'));
        Assert::IsFalse(TchEqual('a', 'b'));
        Assert::IsTrue(TchEqualI('a', 'A'));
        Assert::IsTrue(TchEqualI('Z', 'z'));
        Assert::IsFalse(TchEqualI('a', 'b'));
        Assert::IsTrue(TchEqualI(L'a', L'A'));
        Assert::IsFalse(TchEqualI(L'\u00E9', L'\u00C9'));
    }

    //============================================================
    // TcsLength / TcsLengthSafe
    //============================================================
    TEST_METHOD(TsTcsLength)
    {
        Assert::AreEqual((size_t)0, TcsLength(L""));
        Assert::AreEqual((size_t)5, TcsLength(L"abcde"));
        Assert::AreEqual((size_t)2, TcsLength(L"世界"));
        Assert::AreEqual((size_t)0, TcsLength(""));
        Assert::AreEqual((size_t)5, TcsLength("abcde"));
    }

    TEST_METHOD(TsTcsLengthSafe)
    {
        const WCHAR szText[]{ L"abcdef" };
        Assert::AreEqual((size_t)6, TcsLengthSafe(szText, 6));
        Assert::AreEqual((size_t)3, TcsLengthSafe(szText, 3));
        Assert::AreEqual((size_t)0, TcsLengthSafe(szText, 0));
        Assert::AreEqual((size_t)6, TcsLengthSafe(szText, 100));  // 遇 NUL 提前停止

        const CHAR szTextA[]{ "abc" };
        Assert::AreEqual((size_t)2, TcsLengthSafe(szTextA, 2));
        Assert::AreEqual((size_t)3, TcsLengthSafe(szTextA, 3));
    }

    //============================================================
    // 相等比较
    //============================================================
    TEST_METHOD(TsTcsEqual)
    {
        Assert::IsTrue(TcsEqual(L"abc", L"abc"));
        Assert::IsFalse(TcsEqual(L"abc", L"abd"));
        Assert::IsFalse(TcsEqual(L"abc", L"ab"));
        Assert::IsFalse(TcsEqual(L"ab", L"abc"));
        Assert::IsTrue(TcsEqual(L"", L""));
        Assert::IsTrue(TcsEqual("abc", "abc"));
        Assert::IsFalse(TcsEqual("abc", "ABC"));
    }

    TEST_METHOD(TsTcsEqualI)
    {
        Assert::IsTrue(TcsEqualI(L"abc", L"ABC"));
        Assert::IsTrue(TcsEqualI(L"AbC", L"aBc"));
        Assert::IsFalse(TcsEqualI(L"abc", L"abd"));
        Assert::IsFalse(TcsEqualI(L"abc", L"ab"));
        Assert::IsTrue(TcsEqualI(L"", L""));
        Assert::IsTrue(TcsEqualI("ABC", "abc"));
        // 仅 ASCII 折叠：非 ASCII 字符不会相等
        Assert::IsFalse(TcsEqualI(L"\u00E9", L"\u00C9"));
    }

    TEST_METHOD(TsTcsEqualMaxLength)
    {
        Assert::IsTrue(TcsEqualMaxLength(L"abcdef", L"abcxyz", 3));
        Assert::IsFalse(TcsEqualMaxLength(L"abcdef", L"abcxyz", 4));
        Assert::IsTrue(TcsEqualMaxLength(L"abc", L"abc", 3));
        Assert::IsTrue(TcsEqualMaxLength(L"abc", L"abd", 0));     // Max=0 视为相等
        Assert::IsTrue(TcsEqualMaxLength(L"ab", L"ab", 10));      // 遇 NUL 提前停止
        Assert::IsTrue(TcsEqualMaxLength("abcdef", "abcxyz", 3));
    }

    TEST_METHOD(TsTcsEqualMaxLengthI)
    {
        Assert::IsTrue(TcsEqualMaxLengthI(L"ABCDEF", L"abcxyz", 3));
        Assert::IsFalse(TcsEqualMaxLengthI(L"ABCDEF", L"abcxyz", 4));
        Assert::IsTrue(TcsEqualMaxLengthI(L"AbC", L"aBc", 3));
        Assert::IsTrue(TcsEqualMaxLengthI(L"abc", L"abd", 0));
        Assert::IsTrue(TcsEqualMaxLengthI(L"AB", L"ab", 10));
        Assert::IsTrue(TcsEqualMaxLengthI("ABCDEF", "abcxyz", 3));
    }

    TEST_METHOD(TsTcsEqualLength)
    {
        Assert::IsTrue(TcsEqualLength(L"abc", L"abc", 3));
        Assert::IsFalse(TcsEqualLength(L"abc", L"abd", 3));
        Assert::IsTrue(TcsEqualLength(L"abc", L"abd", 2));
        Assert::IsTrue(TcsEqualLength(L"abc", L"abd", 0));
        // 二进制比较：嵌入 NUL 之后的字符也参与比较
        const WCHAR sz1[]{ L'a', 0, L'b' };
        const WCHAR sz2[]{ L'a', 0, L'b' };
        const WCHAR sz3[]{ L'a', 0, L'c' };
        Assert::IsTrue(TcsEqualLength(sz1, sz2, 3));
        Assert::IsFalse(TcsEqualLength(sz1, sz3, 3));
        Assert::IsTrue(TcsEqualLength(sz1, sz3, 2));
    }

    TEST_METHOD(TsTcsEqualLengthI)
    {
        Assert::IsTrue(TcsEqualLengthI(L"ABC", L"abc", 3));
        Assert::IsFalse(TcsEqualLengthI(L"ABC", L"abd", 3));
        Assert::IsTrue(TcsEqualLengthI(L"ABC", L"abd", 2));
        Assert::IsTrue(TcsEqualLengthI(L"ABC", L"abd", 0));
        Assert::IsTrue(TcsEqualLengthI("ABC", "abc", 3));
    }

    TEST_METHOD(TsTcsEqualLength2)
    {
        Assert::IsTrue(TcsEqualLength2(L"abc", 3, L"abc", 3));
        Assert::IsFalse(TcsEqualLength2(L"abc", 3, L"abc", 2));   // 长度不同
        Assert::IsFalse(TcsEqualLength2(L"abc", 3, L"abd", 3));
        Assert::IsTrue(TcsEqualLength2(L"abc", 0, L"abd", 0));
    }

    TEST_METHOD(TsTcsEqualLength2I)
    {
        Assert::IsTrue(TcsEqualLength2I(L"ABC", 3, L"abc", 3));
        Assert::IsFalse(TcsEqualLength2I(L"ABC", 3, L"abc", 2));
        Assert::IsFalse(TcsEqualLength2I(L"ABC", 3, L"abd", 3));
        Assert::IsTrue(TcsEqualLength2I("ABC", 3, "abc", 3));
    }

    TEST_METHOD(TsTcsIsStartWithLength2)
    {
        Assert::IsTrue(TcsIsStartWithLength2(L"abcdef", 6, L"abc", 3));
        Assert::IsTrue(TcsIsStartWithLength2(L"abc", 3, L"abc", 3));
        Assert::IsFalse(TcsIsStartWithLength2(L"abcdef", 6, L"abd", 3));
        Assert::IsFalse(TcsIsStartWithLength2(L"ab", 2, L"abc", 3));  // Len1 < Len2
        Assert::IsTrue(TcsIsStartWithLength2(L"abc", 3, L"xyz", 0));
    }

    TEST_METHOD(TsTcsIsStartWithLength2I)
    {
        Assert::IsTrue(TcsIsStartWithLength2I(L"ABCDEF", 6, L"abc", 3));
        Assert::IsFalse(TcsIsStartWithLength2I(L"ABCDEF", 6, L"abd", 3));
        Assert::IsFalse(TcsIsStartWithLength2I(L"AB", 2, L"abc", 3));
        Assert::IsTrue(TcsIsStartWithLength2I("ABCDEF", 6, "abc", 3));
    }

    TEST_METHOD(TsTcsIsEndWithLength2)
    {
        Assert::IsTrue(TcsIsEndWithLength2(L"abcdef", 6, L"def", 3));
        Assert::IsTrue(TcsIsEndWithLength2(L"abc", 3, L"abc", 3));
        Assert::IsFalse(TcsIsEndWithLength2(L"abcdef", 6, L"deg", 3));
        Assert::IsFalse(TcsIsEndWithLength2(L"ab", 2, L"abc", 3));    // Len1 < Len2
        Assert::IsTrue(TcsIsEndWithLength2(L"abc", 3, L"xyz", 0));
    }

    TEST_METHOD(TsTcsIsEndWithLength2I)
    {
        Assert::IsTrue(TcsIsEndWithLength2I(L"ABCDEF", 6, L"def", 3));
        Assert::IsFalse(TcsIsEndWithLength2I(L"ABCDEF", 6, L"deg", 3));
        Assert::IsFalse(TcsIsEndWithLength2I(L"AB", 2, L"abc", 3));
        Assert::IsTrue(TcsIsEndWithLength2I("ABCDEF", 6, "def", 3));
    }

    //============================================================
    // 字符查找
    //============================================================
    TEST_METHOD(TsTcsChar)
    {
        const WCHAR szText[]{ L"abcabc" };
        Assert::IsTrue(TcsChar(szText, L'a') == szText + 0);
        Assert::IsTrue(TcsChar(szText, L'c') == szText + 2);
        Assert::IsNull(TcsChar(szText, L'z'));
        Assert::IsNull(TcsChar(L"", L'a'));

        const CHAR szTextA[]{ "abcabc" };
        Assert::IsTrue(TcsChar(szTextA, 'b') == szTextA + 1);
        Assert::IsNull(TcsChar(szTextA, 'z'));
    }

    TEST_METHOD(TsTcsCharLength)
    {
        const WCHAR szText[]{ L"abcabc" };
        Assert::IsTrue(TcsCharLength(szText, 6, L'a') == szText + 0);
        Assert::IsTrue(TcsCharLength(szText, 6, L'c') == szText + 2);
        Assert::IsTrue(TcsCharLength(szText + 3, 3, L'a') == szText + 3);
        Assert::IsNull(TcsCharLength(szText, 0, L'a'));
        Assert::IsNull(TcsCharLength(szText, 2, L'c'));    // 限制长度内不存在
        Assert::IsNull(TcsCharLength(szText, 6, L'z'));
    }

    TEST_METHOD(TsTcsCharLengthI)
    {
        const WCHAR szText[]{ L"aBcAbC" };
        Assert::IsTrue(TcsCharLengthI(szText, 6, L'b') == szText + 1);
        Assert::IsTrue(TcsCharLengthI(szText, 6, L'a') == szText + 0);
        Assert::IsTrue(TcsCharLengthI(szText + 2, 4, L'A') == szText + 3);
        Assert::IsNull(TcsCharLengthI(szText, 0, L'b'));
        Assert::IsNull(TcsCharLengthI(szText, 6, L'z'));
        Assert::IsTrue(TcsCharLengthI("aBc", 3, 'C') != nullptr);
    }

    TEST_METHOD(TsTcsRChar)
    {
        const WCHAR szText[]{ L"abcabc" };
        Assert::IsTrue(TcsRChar(szText, L'a') == szText + 3);
        Assert::IsTrue(TcsRChar(szText, L'c') == szText + 5);
        Assert::IsNull(TcsRChar(szText, L'z'));
        Assert::IsNull(TcsRChar(L"", L'a'));
    }

    TEST_METHOD(TsTcsRCharLength)
    {
        const WCHAR szText[]{ L"abcabc" };
        Assert::IsTrue(TcsRCharLength(szText, 6, L'a') == szText + 3);
        Assert::IsTrue(TcsRCharLength(szText, 3, L'a') == szText + 0);
        Assert::IsTrue(TcsRCharLength(szText, 6, L'c') == szText + 5);
        Assert::IsNull(TcsRCharLength(szText, 0, L'a'));   // 长度 0 不得越界
        Assert::IsNull(TcsRCharLength(szText, 6, L'z'));
    }

    TEST_METHOD(TsTcsRCharLengthI)
    {
        const WCHAR szText[]{ L"aBcAbC" };
        Assert::IsTrue(TcsRCharLengthI(szText, 6, L'a') == szText + 3);
        Assert::IsTrue(TcsRCharLengthI(szText, 3, L'A') == szText + 0);
        Assert::IsTrue(TcsRCharLengthI(szText, 6, L'c') == szText + 5);
        Assert::IsNull(TcsRCharLengthI(szText, 0, L'a'));
        Assert::IsNull(TcsRCharLengthI(szText, 6, L'z'));
        Assert::IsTrue(TcsRCharLengthI("aBcA", 4, 'a') != nullptr);
    }

    //============================================================
    // 子串查找
    //============================================================
    TEST_METHOD(TsTcsString)
    {
        const WCHAR szText[]{ L"xxabcxx" };
        Assert::IsTrue(TcsString(szText, L"abc") == szText + 2);
        Assert::IsTrue(TcsString(szText, L"xx") == szText + 0);
        Assert::IsTrue(TcsString(szText, L"") == szText + 0);
        Assert::IsNull(TcsString(szText, L"abd"));
        Assert::IsNull(TcsString(szText, L"abcy"));

        const CHAR szTextA[]{ "xxabcxx" };
        Assert::IsTrue(TcsString(szTextA, "abc") == szTextA + 2);
    }

    TEST_METHOD(TsTcsStringI)
    {
        const WCHAR szText[]{ L"xxAbCxx" };
        Assert::IsTrue(TcsStringI(szText, L"abc") == szText + 2);
        Assert::IsTrue(TcsStringI(szText, L"") == szText + 0);
        Assert::IsNull(TcsStringI(szText, L"abd"));
        // 部分匹配失败后需继续尝试
        const WCHAR szText2[]{ L"aab" };
        Assert::IsTrue(TcsStringI(szText2, L"AB") == szText2 + 1);
    }

    TEST_METHOD(TsTcsStringLength)
    {
        const WCHAR szText[]{ L"xxabcxx" };
        Assert::IsTrue(TcsStringLength(szText, 7, L"abc", 3) == szText + 2);
        Assert::IsTrue(TcsStringLength(szText, 2, L"abc", 3) == nullptr);   // Len < SubLen
        Assert::IsTrue(TcsStringLength(szText, 7, L"", 0) == szText + 0);   // 空子串
        Assert::IsTrue(TcsStringLength(szText, 7, L"xx", 2) == szText + 0);
        // 恰好位于尾部
        Assert::IsTrue(TcsStringLength(szText + 3, 4, L"cxx", 3) == szText + 4);
        Assert::IsTrue(TcsStringLength(szText, 7, L"cxy", 3) == nullptr);
        // 重叠候选
        const WCHAR szText2[]{ L"aaa" };
        Assert::IsTrue(TcsStringLength(szText2, 3, L"aa", 2) == szText2 + 0);
        // 部分匹配失败后继续尝试
        const WCHAR szText3[]{ L"aab" };
        Assert::IsTrue(TcsStringLength(szText3, 3, L"ab", 2) == szText3 + 1);
        const WCHAR szText4[]{ L"aaab" };
        Assert::IsTrue(TcsStringLength(szText4, 4, L"aab", 3) == szText4 + 1);
        // 长度限制内无法容纳完整子串
        Assert::IsTrue(TcsStringLength(szText3, 2, L"ab", 2) == nullptr);
        Assert::IsTrue(TcsStringLength(szText3, 1, L"ab", 2) == nullptr);
    }

    TEST_METHOD(TsTcsStringLengthI)
    {
        const WCHAR szText[]{ L"xxAbCxx" };
        Assert::IsTrue(TcsStringLengthI(szText, 7, L"abc", 3) == szText + 2);
        Assert::IsTrue(TcsStringLengthI(szText, 2, L"abc", 3) == nullptr);
        Assert::IsTrue(TcsStringLengthI(szText, 7, L"", 0) == szText + 0);
        Assert::IsTrue(TcsStringLengthI(szText, 7, L"ABD", 3) == nullptr);
        const WCHAR szText2[]{ L"AAB" };
        Assert::IsTrue(TcsStringLengthI(szText2, 3, L"ab", 2) == szText2 + 1);
        Assert::IsTrue(TcsStringLengthI("xxABCxx", 7, "abc", 3) != nullptr);
    }

    TEST_METHOD(TsTcsRStringLength)
    {
        const WCHAR szText[]{ L"abab" };
        Assert::IsTrue(TcsRStringLength(szText, 4, L"ab", 2) == szText + 2);  // 默认从末尾开始
        Assert::IsTrue(TcsRStringLength(szText, 4, L"ab", 2, 1) == szText + 0); // 位置限制
        Assert::IsTrue(TcsRStringLength(szText, 4, L"ab", 2, 2) == szText + 2);
        Assert::IsTrue(TcsRStringLength(szText, 4, L"", 0) == szText + 4);
        Assert::IsTrue(TcsRStringLength(szText, 4, L"", 0, 2) == szText + 2);
        Assert::IsTrue(TcsRStringLength(szText, 4, L"ba", 2) == szText + 1);
        Assert::IsTrue(TcsRStringLength(szText, 4, L"xyz", 3) == nullptr);
        Assert::IsTrue(TcsRStringLength(szText, 2, L"xyz", 3) == nullptr);   // Len < SubLen
    }

    TEST_METHOD(TsTcsRStringLengthI)
    {
        const WCHAR szText[]{ L"aBaB" };
        Assert::IsTrue(TcsRStringLengthI(szText, 4, L"ab", 2) == szText + 2);
        Assert::IsTrue(TcsRStringLengthI(szText, 4, L"ab", 2, 1) == szText + 0);
        Assert::IsTrue(TcsRStringLengthI(szText, 4, L"", 0) == szText + 4);
        Assert::IsTrue(TcsRStringLengthI(szText, 4, L"XYZ", 3) == nullptr);
        Assert::IsTrue(TcsRStringLengthI("aBaB", 4, "ab", 2) != nullptr);
    }

    //============================================================
    // 字符集合查找
    //============================================================
    TEST_METHOD(TsTcsCharFirstOf)
    {
        const WCHAR szText[]{ L"abc,def" };
        Assert::IsTrue(TcsCharFirstOf(szText, 7, L",", 1) == szText + 3);
        Assert::IsTrue(TcsCharFirstOf(szText, 7, L",;", 2) == szText + 3);
        Assert::IsTrue(TcsCharFirstOf(szText, 3, L",", 1) == nullptr);     // 有界
        Assert::IsTrue(TcsCharFirstOf(szText, 0, L",", 1) == nullptr);
        Assert::IsTrue(TcsCharFirstOf(szText, 7, L"", 0) == nullptr);
        Assert::IsTrue(TcsCharFirstOf(szText, 7, L"a", 1) == szText + 0);
        // 零结尾重载
        Assert::IsTrue(TcsCharFirstOf(szText, L",") == szText + 3);
        Assert::IsNull(TcsCharFirstOf(szText, L"z"));
    }

    TEST_METHOD(TsTcsCharFirstNotOf)
    {
        const WCHAR szText[]{ L"  abc" };
        Assert::IsTrue(TcsCharFirstNotOf(szText, 5, L" ", 1) == szText + 2);
        Assert::IsTrue(TcsCharFirstNotOf(szText, 5, L"", 0) == szText + 0);
        Assert::IsTrue(TcsCharFirstNotOf(szText, 2, L" ", 1) == nullptr);  // 全部为空格
        Assert::IsTrue(TcsCharFirstNotOf(szText, 0, L" ", 1) == nullptr);
        Assert::IsTrue(TcsCharFirstNotOf(szText, 5, L" abc", 4) == nullptr);
    }

    TEST_METHOD(TsTcsCharLastOf)
    {
        const WCHAR szText[]{ L"a,b,c" };
        Assert::IsTrue(TcsCharLastOf(szText, 5, L",", 1) == szText + 3);
        Assert::IsTrue(TcsCharLastOf(szText, 5, L",", 1, 2) == szText + 1);   // 位置限制
        Assert::IsTrue(TcsCharLastOf(szText, 5, L",", 1, 0) == nullptr);
        Assert::IsTrue(TcsCharLastOf(szText, 0, L",", 1) == nullptr);        // 长度 0
        Assert::IsTrue(TcsCharLastOf(szText, 5, L"z", 1) == nullptr);
        Assert::IsTrue(TcsCharLastOf(szText, 5, L"", 0) == nullptr);
    }

    TEST_METHOD(TsTcsCharLastNotOf)
    {
        const WCHAR szText[]{ L"abc  " };
        Assert::IsTrue(TcsCharLastNotOf(szText, 5, L" ", 1) == szText + 2);
        Assert::IsTrue(TcsCharLastNotOf(szText, 5, L" ", 1, 1) == szText + 1);
        Assert::IsTrue(TcsCharLastNotOf(szText, 5, L"", 0) == szText + 4);
        Assert::IsTrue(TcsCharLastNotOf(szText, 5, L" abc", 4) == nullptr);
        Assert::IsTrue(TcsCharLastNotOf(szText, 0, L" ", 1) == nullptr);
    }

    //============================================================
    // 复制 / 移动 / 填充
    //============================================================
    TEST_METHOD(TsTcsCopy)
    {
        // TcsCopy 内部使用 strcpy/wcscpy，测试项目启用了 SDL 检查，这里局部抑制
#pragma warning(push)
#pragma warning(disable : 4996)
        WCHAR szDst[8]{};
        Assert::IsTrue(TcsCopy(szDst, L"abc") == szDst);
        Assert::IsTrue(wcscmp(szDst, L"abc") == 0);

        CHAR szDstA[8]{};
        Assert::IsTrue(TcsCopy(szDstA, "xy") == szDstA);
        Assert::IsTrue(strcmp(szDstA, "xy") == 0);
#pragma warning(pop)
    }

    TEST_METHOD(TsTcsCopyLength)
    {
        WCHAR szDst[8]{ L'0', L'0', L'0', L'0', L'0', L'0', L'0', L'0' };
        Assert::IsTrue(TcsCopyLength(szDst, L"ab", 2) == szDst);
        Assert::IsTrue(szDst[0] == L'a' && szDst[1] == L'b');
        Assert::IsTrue(szDst[2] == L'0');   // 不写终止符

        Assert::IsTrue(TcsCopyLength(szDst, L"x", 0) == szDst);
        Assert::IsTrue(szDst[0] == L'a');   // 长度 0 不修改
    }

    TEST_METHOD(TsTcsCopyLengthEnd)
    {
        WCHAR szDst[8]{ L'0', L'0', L'0', L'0', L'0', L'0', L'0', L'0' };
        Assert::IsTrue(TcsCopyLengthEnd(szDst, L"abc", 3) == szDst);
        Assert::IsTrue(wcscmp(szDst, L"abc") == 0);   // 写入终止符

        CHAR szDstA[4]{};
        TcsCopyLengthEnd(szDstA, "xy", 2);
        Assert::IsTrue(strcmp(szDstA, "xy") == 0);
    }

    TEST_METHOD(TsTcsMoveLength)
    {
        WCHAR szBuf[]{ L"abcdef" };
        Assert::IsTrue(TcsMoveLength(szBuf + 1, szBuf, 3) == szBuf + 1);
        Assert::IsTrue(wcsncmp(szBuf, L"aabc", 4) == 0);

        WCHAR szBuf2[]{ L"abcdef" };
        TcsMoveLength(szBuf2, szBuf2 + 1, 3);   // 重叠且前移
        Assert::IsTrue(wcsncmp(szBuf2, L"bcd", 3) == 0);
    }

    TEST_METHOD(TsTcsMoveLengthEnd)
    {
        WCHAR szBuf[8]{ L"abcdef" };
        TcsMoveLengthEnd(szBuf + 1, szBuf, 3);
        Assert::IsTrue(wcscmp(szBuf, L"aabc") == 0);   // 写入终止符
        WCHAR szBuf2[16]{ L"abcdef" };
        TcsMoveLengthEnd(szBuf2 + 2, szBuf2, 6);
        Assert::IsTrue(wcscmp(szBuf2, L"ababcdef") == 0);
    }

    TEST_METHOD(TsTcsSet)
    {
        WCHAR szBuf[8]{ L"abcdefg" };
        Assert::IsTrue(TcsSet(szBuf, L'x', 3) == szBuf);
        Assert::IsTrue(wcsncmp(szBuf, L"xxx", 3) == 0);
        Assert::IsTrue(szBuf[3] == L'd');   // 长度外不修改

        Assert::IsTrue(TcsSet(szBuf, L'y', 0) == szBuf);
        Assert::IsTrue(szBuf[0] == L'x');

        CHAR szBufA[4]{ "abc" };
        TcsSet(szBufA, '\0', 3);
        Assert::IsTrue(szBufA[0] == 0 && szBufA[1] == 0 && szBufA[2] == 0);
    }

    //============================================================
    // 比较
    //============================================================
    TEST_METHOD(TsTcsCompare)
    {
        Assert::AreEqual(0, TcsCompare(L"abc", L"abc"));
        Assert::IsTrue(TcsCompare(L"abc", L"abd") < 0);
        Assert::IsTrue(TcsCompare(L"abd", L"abc") > 0);
        Assert::IsTrue(TcsCompare(L"ab", L"abc") < 0);
        Assert::IsTrue(TcsCompare(L"ABC", L"abc") < 0);   // 区分大小写
        Assert::AreEqual(0, TcsCompare("abc", "abc"));
    }

    TEST_METHOD(TsTcsCompareMaxLength)
    {
        Assert::AreEqual(0, TcsCompareMaxLength(L"abcxxx", L"abcyyy", 3));
        Assert::IsTrue(TcsCompareMaxLength(L"abcxxx", L"abdyyy", 4) < 0);
        Assert::AreEqual(0, TcsCompareMaxLength(L"abc", L"abd", 0));
        Assert::IsTrue(TcsCompareMaxLength(L"abd", L"abc", 3) > 0);
    }

    TEST_METHOD(TsTcsCompareLength)
    {
        Assert::AreEqual(0, TcsCompareLength(L"abc", L"abc", 3));
        Assert::IsTrue(TcsCompareLength(L"abc", L"abd", 3) < 0);
        Assert::IsTrue(TcsCompareLength(L"abd", L"abc", 3) > 0);
        Assert::AreEqual(0, TcsCompareLength(L"abc", L"xyz", 0));
        Assert::AreEqual(0, TcsCompareLength(L"abc", L"abd", 2));
    }

    TEST_METHOD(TsTcsCompareLength2)
    {
        Assert::AreEqual(0, TcsCompareLength2(L"abc", 3, L"abc", 3));
        Assert::IsTrue(TcsCompareLength2(L"abc", 3, L"abc", 4) < 0);
        Assert::IsTrue(TcsCompareLength2(L"abc", 4, L"abc", 3) > 0);
        Assert::IsTrue(TcsCompareLength2(L"abc", 3, L"abd", 3) < 0);
        Assert::AreEqual(0, TcsCompareLength2(L"abc", 0, L"xyz", 0));
    }

    TEST_METHOD(TsTcsCompareI)
    {
        Assert::AreEqual(0, TcsCompareI(L"ABC", L"abc"));
        Assert::IsTrue(TcsCompareI(L"abc", L"ABD") < 0);
        Assert::IsTrue(TcsCompareI(L"ABD", L"abc") > 0);
        Assert::IsTrue(TcsCompareI(L"ab", L"ABC") < 0);
        Assert::AreEqual(0, TcsCompareI("AbC", "aBc"));
        // 仅 ASCII 折叠
        Assert::IsTrue(TcsCompareI(L"\u00E9", L"\u00C9") != 0);
    }

    TEST_METHOD(TsTcsCompareMaxLengthI)
    {
        Assert::AreEqual(0, TcsCompareMaxLengthI(L"ABCxxx", L"abcyyy", 3));
        Assert::IsTrue(TcsCompareMaxLengthI(L"ABCxxx", L"abdyyy", 4) < 0);
        Assert::AreEqual(0, TcsCompareMaxLengthI(L"ABC", L"abd", 0));
        Assert::IsTrue(TcsCompareMaxLengthI(L"ABD", L"abc", 3) > 0);
        Assert::AreEqual(0, TcsCompareMaxLengthI("ABC", "abc", 3));
    }

    TEST_METHOD(TsTcsCompareLengthI)
    {
        Assert::AreEqual(0, TcsCompareLengthI(L"ABC", L"abc", 3));
        Assert::IsTrue(TcsCompareLengthI(L"ABC", L"abd", 3) < 0);
        Assert::IsTrue(TcsCompareLengthI(L"ABD", L"abc", 3) > 0);
        Assert::AreEqual(0, TcsCompareLengthI(L"ABC", L"xyz", 0));
        Assert::AreEqual(0, TcsCompareLengthI("ABC", "abc", 3));
    }

    TEST_METHOD(TsTcsCompareLength2I)
    {
        Assert::AreEqual(0, TcsCompareLength2I(L"ABC", 3, L"abc", 3));
        Assert::IsTrue(TcsCompareLength2I(L"ABC", 3, L"abc", 4) < 0);
        Assert::IsTrue(TcsCompareLength2I(L"ABC", 4, L"abc", 3) > 0);
        Assert::IsTrue(TcsCompareLength2I(L"ABC", 3, L"abd", 3) < 0);
        Assert::AreEqual(0, TcsCompareLength2I("ABC", 3, "abc", 3));
    }

    //============================================================
    // Find* 索引版
    //============================================================
    TEST_METHOD(TsFindString)
    {
        Assert::AreEqual(2, FindString(L"xxabcxx", L"abc"));
        Assert::AreEqual(0, FindString(L"xxabcxx", L"xx"));
        Assert::AreEqual(-1, FindString(L"xxabcxx", L"abd"));
        Assert::AreEqual(5, FindString(L"xxabcxx", L"xx", 2));
        Assert::AreEqual(-1, FindString(L"xxabcxx", L"xx", 6));
        Assert::AreEqual(2, FindString("xxabcxx", "abc"));
    }

    TEST_METHOD(TsFindStringI)
    {
        Assert::AreEqual(2, FindStringI(L"xxAbCxx", L"abc"));
        Assert::AreEqual(-1, FindStringI(L"xxAbCxx", L"abd"));
        Assert::AreEqual(5, FindStringI(L"xxAbCxx", L"XX", 2));
        Assert::AreEqual(2, FindStringI("xxAbCxx", "abc"));
    }

    TEST_METHOD(TsFindStringLength)
    {
        Assert::AreEqual(2, FindStringLength(L"xxabcxx", 7, L"abc", 3));
        Assert::AreEqual(-1, FindStringLength(L"xxabcxx", 4, L"abc", 3));   // 长度限制
        Assert::AreEqual(2, FindStringLength(L"xxabcxx", 7, L"abc", 3, 1)); // 起始位置
        Assert::AreEqual(-1, FindStringLength(L"xxabcxx", 7, L"abc", 3, 3));
        Assert::AreEqual(0, FindStringLength(L"xxabcxx", 7, L"", 0));      // 空子串返回起始位置
    }

    TEST_METHOD(TsFindStringLengthI)
    {
        Assert::AreEqual(2, FindStringLengthI(L"xxAbCxx", 7, L"abc", 3));
        Assert::AreEqual(-1, FindStringLengthI(L"xxAbCxx", 4, L"abc", 3));
        Assert::AreEqual(2, FindStringLengthI(L"xxAbCxx", 7, L"abc", 3, 1));
        Assert::AreEqual(2, FindStringLengthI("xxAbCxx", 7, "abc", 3));
    }

    TEST_METHOD(TsRFindString)
    {
        Assert::AreEqual(2, RFindString(L"abab", 4, L"ab", 2));
        Assert::AreEqual(0, RFindString(L"abab", 4, L"ab", 2, 1));
        Assert::AreEqual(-1, RFindString(L"abab", 4, L"xy", 2));
        Assert::AreEqual(1, RFindString(L"abab", 4, L"ba", 2));
    }

    TEST_METHOD(TsRFindStringI)
    {
        Assert::AreEqual(2, RFindStringI(L"aBaB", 4, L"ab", 2));
        Assert::AreEqual(0, RFindStringI(L"aBaB", 4, L"ab", 2, 1));
        Assert::AreEqual(-1, RFindStringI(L"aBaB", 4, L"xy", 2));
        Assert::AreEqual(2, RFindStringI("aBaB", 4, "AB", 2));
    }

    TEST_METHOD(TsFindChar)
    {
        Assert::AreEqual(1, FindChar(L"abc", L'b'));
        Assert::AreEqual(-1, FindChar(L"abc", L'z'));
        Assert::AreEqual(-1, FindChar(L"abc", L'a', 1));
        Assert::AreEqual(0, FindChar(L"abc", L'a'));
        Assert::AreEqual(1, FindChar("abc", 'b'));
    }

    TEST_METHOD(TsFindCharLength)
    {
        Assert::AreEqual(1, FindCharLength(L"abcabc", 6, L'b'));
        Assert::AreEqual(4, FindCharLength(L"abcabc", 6, L'b', 2));
        Assert::AreEqual(-1, FindCharLength(L"abcabc", 3, L'c', 3));
        Assert::AreEqual(-1, FindCharLength(L"abcabc", 6, L'z'));
        Assert::AreEqual(1, FindCharLength("abc", 3, 'b'));
    }

    TEST_METHOD(TsRFindCharLength)
    {
        Assert::AreEqual(4, RFindCharLength(L"abcabc", 6, L'b'));
        Assert::AreEqual(1, RFindCharLength(L"abcabc", 6, L'b', 2));
        Assert::AreEqual(-1, RFindCharLength(L"abcabc", 6, L'z'));
        Assert::AreEqual(1, RFindCharLength("abc", 3, 'b'));
    }

    TEST_METHOD(TsFindCharFirstOf)
    {
        const WCHAR szText[]{ L"abc,def" };
        Assert::AreEqual(3, FindCharFirstOf(szText, 7, EckArgString(L",")));
        Assert::AreEqual(3, FindCharFirstOf(szText, 7, EckArgString(L",;")));
        Assert::AreEqual(-1, FindCharFirstOf(szText, 3, EckArgString(L",")));   // 有界
        Assert::AreEqual(-1, FindCharFirstOf(szText, 7, EckArgString(L"z")));
        Assert::AreEqual(0, FindCharFirstOf(szText, 7, EckArgString(L"a")));
        Assert::AreEqual(3, FindCharFirstOf(szText, 7, EckArgString(L","), 2));
        Assert::AreEqual(3, FindCharFirstOf("abc,def", 7, EckArgString(",")));
    }

    TEST_METHOD(TsFindCharFirstNotOf)
    {
        const WCHAR szText[]{ L"   abc" };
        Assert::AreEqual(3, FindCharFirstNotOf(szText, 6, EckArgString(L" ")));
        Assert::AreEqual(3, FindCharFirstNotOf(szText, 6, EckArgString(L" "), 1));
        Assert::AreEqual(-1, FindCharFirstNotOf(szText, 3, EckArgString(L" ")));
        Assert::AreEqual(-1, FindCharFirstNotOf(szText, 6, EckArgString(L" abc")));
    }

    TEST_METHOD(TsFindCharLastOf)
    {
        const WCHAR szText[]{ L"a,b,c" };
        Assert::AreEqual(3, FindCharLastOf(szText, 5, EckArgString(L",")));
        Assert::AreEqual(3, FindCharLastOf(szText, 5, EckArgString(L","), 3));
        Assert::AreEqual(1, FindCharLastOf(szText, 5, EckArgString(L","), 2));
        Assert::AreEqual(-1, FindCharLastOf(szText, 5, EckArgString(L","), 0));
        Assert::AreEqual(-1, FindCharLastOf(szText, 5, EckArgString(L"z")));
        Assert::AreEqual(3, FindCharLastOf("a,b,c", 5, EckArgString(",")));
    }

    TEST_METHOD(TsFindCharLastNotOf)
    {
        const WCHAR szText[]{ L"abc  " };
        Assert::AreEqual(2, FindCharLastNotOf(szText, 5, EckArgString(L" ")));
        Assert::AreEqual(1, FindCharLastNotOf(szText, 5, EckArgString(L" "), 1));
        Assert::AreEqual(-1, FindCharLastNotOf(szText, 5, EckArgString(L" abc")));
        Assert::AreEqual(-1, FindCharLastNotOf(szText, 0, EckArgString(L" ")));
    }

    //============================================================
    // Trim
    //============================================================
    TEST_METHOD(TsTrimStringLeft)
    {
        const WCHAR szText[]{ L"  \tab  " };
        Assert::IsTrue(TrimStringLeft(szText, 7) == szText + 3);
        Assert::IsTrue(TrimStringLeft(szText, -1) == szText + 3);      // 自动长度
        Assert::IsTrue(TrimStringLeft(szText, 0) == szText + 0);
        const WCHAR szAll[]{ L"   " };
        Assert::IsTrue(TrimStringLeft(szAll, 3) == szAll + 3);        // 全部为空白
        const WCHAR szNone[]{ L"ab  " };
        Assert::IsTrue(TrimStringLeft(szNone, 4) == szNone + 0);      // 无前导空白
        // 全角空格不在窄空白集内
        const WCHAR szFull[]{ L"\u3000ab" };
        Assert::IsTrue(TrimStringLeft(szFull, 3) == szFull + 0);

        const CHAR szTextA[]{ "  ab" };
        Assert::IsTrue(TrimStringLeft(szTextA, 4) == szTextA + 2);
    }

    TEST_METHOD(TsTrimStringRight)
    {
        const WCHAR szText[]{ L"ab  \t" };
        Assert::IsTrue(TrimStringRight(szText, 5) == szText + 2);
        Assert::IsTrue(TrimStringRight(szText, -1) == szText + 2);
        Assert::IsTrue(TrimStringRight(szText, 0) == szText + 0);
        const WCHAR szAll[]{ L"   " };
        Assert::IsTrue(TrimStringRight(szAll, 3) == szAll + 0);       // 全部为空白
        const WCHAR szNone[]{ L"  ab" };
        Assert::IsTrue(TrimStringRight(szNone, 4) == szNone + 4);     // 无尾随空白
        const WCHAR szFull[]{ L"ab\u3000" };
        Assert::IsTrue(TrimStringRight(szFull, 3) == szFull + 3);

        const CHAR szTextA[]{ "ab  " };
        Assert::IsTrue(TrimStringRight(szTextA, 4) == szTextA + 2);
    }

    TEST_METHOD(TsTrimString)
    {
        const WCHAR szText[]{ L"  ab  " };
        AssertView(TrimString(szText, 6), L"ab");
        AssertView(TrimString(szText, -1), L"ab");          // 自动长度
        AssertView(TrimString(szText, 0), L"");
        AssertView(TrimString(L"abc", 3), L"abc");
        AssertView(TrimString(L"   ", 3), L"");             // 全空白
        AssertView(TrimString(L"", 0), L"");
        AssertView(TrimString(L"\r\n\tabc\r\n", -1), L"abc");
        // 仅去两端空白，内部空白保留
        AssertView(TrimString(L"  a  b  ", -1), L"a  b");
        // 全角空格不被裁掉（窄空白集）
        AssertView(TrimString(L"\u3000ab\u3000", 4), L"\u3000ab\u3000");

        AssertView(TrimString("  ab  ", -1), std::string_view{ "ab" });
    }

    //============================================================
    // Split
    //============================================================
    TEST_METHOD(TsSplitStringBasic)
    {
        WCHAR szText[]{ L"a,b,c" };
        std::vector<std::wstring> v{};
        SplitString(szText, -1, EckArgString(L","), 0,
            [&v](PWSTR pszStart, int cchSub) noexcept { v.emplace_back(pszStart, cchSub); });
        Assert::AreEqual((size_t)3, v.size());
        Assert::IsTrue(v[0] == L"a" && v[1] == L"b" && v[2] == L"c");
    }

    TEST_METHOD(TsSplitStringNoSeparator)
    {
        WCHAR szText[]{ L"abc" };
        std::vector<std::wstring> v{};
        SplitString(szText, -1, EckArgString(L","), 0,
            [&v](PWSTR pszStart, int cchSub) noexcept { v.emplace_back(pszStart, cchSub); });
        Assert::AreEqual((size_t)1, v.size());
        Assert::IsTrue(v[0] == L"abc");
    }

    TEST_METHOD(TsSplitStringEmptyParts)
    {
        WCHAR szText[]{ L",a,," };
        std::vector<std::wstring> v{};
        SplitString(szText, -1, EckArgString(L","), 0,
            [&v](PWSTR pszStart, int cchSub) noexcept { v.emplace_back(pszStart, cchSub); });
        Assert::AreEqual((size_t)4, v.size());
        Assert::IsTrue(v[0].empty() && v[1] == L"a" && v[2].empty() && v[3].empty());
    }

    TEST_METHOD(TsSplitStringMultiCharSeparator)
    {
        WCHAR szText[]{ L"a::b::c" };
        std::vector<std::wstring> v{};
        SplitString(szText, -1, EckArgString(L"::"), 0,     // cchDiv = 2
            [&v](PWSTR pszStart, int cchSub) noexcept { v.emplace_back(pszStart, cchSub); });
        Assert::AreEqual((size_t)3, v.size());
        Assert::IsTrue(v[0] == L"a" && v[1] == L"b" && v[2] == L"c");
    }

    TEST_METHOD(TsSplitStringSubTextExpected)
    {
        WCHAR szText[]{ L"a,b,c,d" };
        std::vector<std::wstring> v{};
        SplitString(szText, -1, EckArgString(L","), 2,
            [&v](PWSTR pszStart, int cchSub) noexcept { v.emplace_back(pszStart, cchSub); });
        Assert::AreEqual((size_t)2, v.size());
        Assert::IsTrue(v[0] == L"a" && v[1] == L"b");
    }

    TEST_METHOD(TsSplitStringExplicitLength)
    {
        WCHAR szText[]{ L"a,b,c" };
        std::vector<std::wstring> v{};
        SplitString(szText, 3, EckArgString(L","), 0,        // 仅处理 "a,b"
            [&v](PWSTR pszStart, int cchSub) noexcept { v.emplace_back(pszStart, cchSub); });
        Assert::AreEqual((size_t)2, v.size());
        Assert::IsTrue(v[0] == L"a" && v[1] == L"b");
    }

    TEST_METHOD(TsSplitStringCallbackStop)
    {
        WCHAR szText[]{ L"a,b,c" };
        int cParts{};
        SplitString(szText, -1, EckArgString(L","), 0,
            [&cParts](PWSTR, int) noexcept { return ++cParts < 2; });   // 返回 false 提前终止
        Assert::AreEqual(2, cParts);
    }

    TEST_METHOD(TsSplitStringChar)
    {
        CHAR szText[]{ "a,b,c" };
        std::vector<std::string> v{};
        SplitString(szText, -1, EckArgString(","), 0,
            [&v](PSTR pszStart, int cchSub) noexcept { v.emplace_back(pszStart, cchSub); });
        Assert::AreEqual((size_t)3, v.size());
        Assert::IsTrue(v[0] == "a" && v[1] == "b" && v[2] == "c");
    }

    TEST_METHOD(TsSplitStringMultipleCharBasic)
    {
        WCHAR szText[]{ L"a,b;c" };
        std::vector<std::wstring> v{};
        SplitStringMultipleChar(szText, -1, EckArgString(L",;"), 0,
            [&v](PWSTR pszStart, int cchSub) noexcept { v.emplace_back(pszStart, cchSub); });
        Assert::AreEqual((size_t)3, v.size());
        Assert::IsTrue(v[0] == L"a" && v[1] == L"b" && v[2] == L"c");
    }

    TEST_METHOD(TsSplitStringMultipleCharSubTextExpected)
    {
        WCHAR szText[]{ L"a,b;c;d" };
        std::vector<std::wstring> v{};
        SplitStringMultipleChar(szText, -1, EckArgString(L",;"), 2,
            [&v](PWSTR pszStart, int cchSub) noexcept { v.emplace_back(pszStart, cchSub); });
        Assert::AreEqual((size_t)2, v.size());
        Assert::IsTrue(v[0] == L"a" && v[1] == L"b");
    }

    TEST_METHOD(TsSplitStringMultipleCharNoSeparator)
    {
        WCHAR szText[]{ L"abc" };
        std::vector<std::wstring> v{};
        SplitStringMultipleChar(szText, -1, EckArgString(L",;"), 0,
            [&v](PWSTR pszStart, int cchSub) noexcept { v.emplace_back(pszStart, cchSub); });
        Assert::AreEqual((size_t)1, v.size());
        Assert::IsTrue(v[0] == L"abc");
    }

    TEST_METHOD(TsSplitStringAndCut)
    {
        WCHAR szText[]{ L"a,b,c" };
        std::vector<PWSTR> v{};
        SplitStringAndCut(szText, -1, EckArgString(L","), v);
        Assert::AreEqual((size_t)3, v.size());
        Assert::IsTrue(wcscmp(v[0], L"a") == 0);
        Assert::IsTrue(wcscmp(v[1], L"b") == 0);
        Assert::IsTrue(wcscmp(v[2], L"c") == 0);
    }

    TEST_METHOD(TsSplitStringAndCutEmptyParts)
    {
        WCHAR szText[]{ L"a,,c" };
        std::vector<PWSTR> v{};
        SplitStringAndCut(szText, -1, EckArgString(L","), v);
        Assert::AreEqual((size_t)3, v.size());
        Assert::IsTrue(wcscmp(v[0], L"a") == 0);
        Assert::IsTrue(wcscmp(v[1], L"") == 0);
        Assert::IsTrue(wcscmp(v[2], L"c") == 0);
    }

    TEST_METHOD(TsSplitStringMultipleCharAndCut)
    {
        WCHAR szText[]{ L"a,b;c" };
        std::vector<PWSTR> v{};
        SplitStringMultipleCharAndCut(szText, -1, EckArgString(L",;"), v);
        Assert::AreEqual((size_t)3, v.size());
        Assert::IsTrue(wcscmp(v[0], L"a") == 0);
        Assert::IsTrue(wcscmp(v[1], L"b") == 0);
        Assert::IsTrue(wcscmp(v[2], L"c") == 0);
    }

    TEST_METHOD(TsSplitStringAndCutSubTextExpected)
    {
        WCHAR szText[]{ L"a,b,c" };
        std::vector<PWSTR> v{};
        SplitStringAndCut(szText, -1, EckArgString(L","), v, 2);
        Assert::AreEqual((size_t)2, v.size());
        Assert::IsTrue(wcscmp(v[0], L"a") == 0);
        Assert::IsTrue(wcscmp(v[1], L"b") == 0);
    }
};
TS_NS_END
