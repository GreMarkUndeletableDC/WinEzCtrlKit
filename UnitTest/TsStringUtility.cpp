#include "pch.h"
#include "../eck/StringUtility.h"

#include <string>
#include <string_view>
#include <vector>

using namespace eck;
using namespace std::literals;

TS_NS_BEGIN
TEST_CLASS(TsStringUtility)
{
public:
    TEST_METHOD(TcsCharLength_Find)
    {
        constexpr char sz[]{ "hello" };
        Assert::IsTrue(TcsCharLength(sz, 5, 'h') == sz);
        Assert::IsTrue(TcsCharLength(sz, 5, 'l') == sz + 2);
        Assert::IsTrue(TcsCharLength(sz, 5, 'o') == sz + 4);
        Assert::IsTrue(TcsCharLength(sz, 5, 'z') == nullptr);
        Assert::IsTrue(TcsCharLength(sz, 0, 'h') == nullptr);

        constexpr wchar_t szW[]{ L"hello" };
        Assert::IsTrue(TcsCharLength(szW, 5, L'l') == szW + 2);
        Assert::IsTrue(TcsCharLength(szW, 5, L'z') == nullptr);
    }

    TEST_METHOD(TcsCharLengthI_Find)
    {
        constexpr char sz[]{ "Hello" };
        Assert::IsTrue(TcsCharLengthI(sz, 5, 'h') == sz);
        Assert::IsTrue(TcsCharLengthI(sz, 5, 'L') == sz + 2);
        Assert::IsTrue(TcsCharLengthI(sz, 5, 'Z') == nullptr);
        Assert::IsTrue(TcsCharLengthI(sz, 0, 'h') == nullptr);
    }

    TEST_METHOD(TcsRCharLength_Find)
    {
        constexpr char sz[]{ "hello" };
        Assert::IsTrue(TcsRCharLength(sz, 5, 'l') == sz + 3);
        Assert::IsTrue(TcsRCharLength(sz, 5, 'h') == sz);
        Assert::IsTrue(TcsRCharLength(sz, 5, 'z') == nullptr);
        // 长度为0时不得进行越界指针运算
        Assert::IsTrue(TcsRCharLength(sz, 0, 'h') == nullptr);

        constexpr wchar_t szW[]{ L"hello" };
        Assert::IsTrue(TcsRCharLength(szW, 5, L'l') == szW + 3);
    }

    TEST_METHOD(TcsRCharLengthI_Find)
    {
        constexpr char sz[]{ "hELLo" };
        Assert::IsTrue(TcsRCharLengthI(sz, 5, 'L') == sz + 3);
        Assert::IsTrue(TcsRCharLengthI(sz, 5, 'H') == sz);
        Assert::IsTrue(TcsRCharLengthI(sz, 5, 'z') == nullptr);
        Assert::IsTrue(TcsRCharLengthI(sz, 0, 'h') == nullptr);
    }

    TEST_METHOD(TcsStringLength_Find)
    {
        constexpr char sz[]{ "hello world" };
        Assert::IsTrue(TcsStringLength(sz, 11, "world", 5) == sz + 6);
        Assert::IsTrue(TcsStringLength(sz, 11, "hello", 5) == sz);
        Assert::IsTrue(TcsStringLength(sz, 11, "o w", 3) == sz + 4);
        Assert::IsTrue(TcsStringLength(sz, 11, "zzz", 3) == nullptr);

        constexpr char sz2[]{ "abcabc" };
        Assert::IsTrue(TcsStringLength(sz2, 6, "cab", 3) == sz2 + 2);
        Assert::IsTrue(TcsStringLength(sz2, 6, "abc", 3) == sz2);
    }

    // 首个字符部分匹配后，下一个候选位置不能被跳过
    TEST_METHOD(TcsStringLength_OverlappedPrefix)
    {
        constexpr char sz[]{ "aab" };
        Assert::IsTrue(TcsStringLength(sz, 3, "ab", 2) == sz + 1);

        constexpr char sz2[]{ "aaab" };
        Assert::IsTrue(TcsStringLength(sz2, 4, "aab", 3) == sz2 + 1);

        constexpr char sz3[]{ "xxxxy" };
        Assert::IsTrue(TcsStringLength(sz3, 5, "xy", 2) == sz3 + 3);

        constexpr char sz4[]{ "abab" };
        Assert::IsTrue(TcsStringLength(sz4, 4, "bab", 3) == sz4 + 1);
    }

    TEST_METHOD(TcsStringLength_EdgeCase)
    {
        constexpr char sz[]{ "abc" };
        // 空模式返回起始位置
        Assert::IsTrue(TcsStringLength(sz, 3, "", 0) == sz);
        // 模式长于文本
        Assert::IsTrue(TcsStringLength(sz, 3, "abcd", 4) == nullptr);
        // 精确匹配整个文本
        Assert::IsTrue(TcsStringLength(sz, 3, "abc", 3) == sz);
        // 长度为0
        Assert::IsTrue(TcsStringLength(sz, 0, "abc", 3) == nullptr);
        Assert::IsTrue(TcsStringLength(sz, 0, "", 0) == sz);
    }

    TEST_METHOD(TcsStringLengthI_Find)
    {
        constexpr char sz[]{ "HelloWorld" };
        Assert::IsTrue(TcsStringLengthI(sz, 10, "world", 5) == sz + 5);
        Assert::IsTrue(TcsStringLengthI(sz, 10, "LOW", 3) == sz + 3);
        Assert::IsTrue(TcsStringLengthI(sz, 10, "zzz", 3) == nullptr);
        Assert::IsTrue(TcsStringLengthI(sz, 10, "", 0) == sz);
        Assert::IsTrue(TcsStringLengthI(sz, 2, "low", 3) == nullptr);
    }

    TEST_METHOD(TcsRStringLength_Find)
    {
        constexpr char sz[]{ "abcabc" };
        Assert::IsTrue(TcsRStringLength(sz, 6, "abc", 3) == sz + 3);
        Assert::IsTrue(TcsRStringLength(sz, 6, "abc", 3, 2) == sz);
        Assert::IsTrue(TcsRStringLength(sz, 6, "cab", 3) == sz + 2);
        Assert::IsTrue(TcsRStringLength(sz, 6, "zzz", 3) == nullptr);
        Assert::IsTrue(TcsRStringLength(sz, 6, "abcd", 4) == nullptr);
        // 空模式返回末尾
        Assert::IsTrue(TcsRStringLength(sz, 6, "", 0) == sz + 6);
        Assert::IsTrue(TcsRStringLength(sz, 0, "a", 1) == nullptr);

        constexpr wchar_t szW[]{ L"abcabc" };
        Assert::IsTrue(TcsRStringLength(szW, 6, L"abc", 3) == szW + 3);
    }

    TEST_METHOD(TcsRStringLengthI_Find)
    {
        constexpr char sz[]{ "xABCx" };
        Assert::IsTrue(TcsRStringLengthI(sz, 5, "abc", 3) == sz + 1);
        Assert::IsTrue(TcsRStringLengthI(sz, 5, "ABC", 3, 2) == sz + 1);
        Assert::IsTrue(TcsRStringLengthI(sz, 5, "ABC", 3, 0) == nullptr);// 起始位置限制在0，形不成完整匹配
        Assert::IsTrue(TcsRStringLengthI(sz, 5, "zzz", 3) == nullptr);
        Assert::IsTrue(TcsRStringLengthI(sz, 0, "a", 1) == nullptr);
    }

    TEST_METHOD(TcsCharFirstOfAndNotOf_Length)
    {
        constexpr char sz[]{ "hello" };
        Assert::IsTrue(TcsCharFirstOf(sz, 5, "lo", 2) == sz + 2);
        Assert::IsTrue(TcsCharFirstOf(sz, 5, "xyz", 3) == nullptr);
        Assert::IsTrue(TcsCharFirstOf(sz, 0, "lo", 2) == nullptr);

        constexpr char sz2[]{ "  ab" };
        Assert::IsTrue(TcsCharFirstNotOf(sz2, 4, " ", 1) == sz2 + 2);
        Assert::IsTrue(TcsCharFirstNotOf(sz2, 2, " ", 1) == nullptr);
        Assert::IsTrue(TcsCharFirstNotOf(sz2, 0, " ", 1) == nullptr);
    }

    TEST_METHOD(TcsCharLastOfAndLastNotOf_Length)
    {
        constexpr char sz[]{ "a.b.c" };
        Assert::IsTrue(TcsCharLastOf(sz, 5, ".", 1) == sz + 3);
        Assert::IsTrue(TcsCharLastOf(sz, 5, ".", 1, 2) == sz + 1);
        Assert::IsTrue(TcsCharLastOf(sz, 5, "x", 1) == nullptr);
        // 长度为0时返回空指针，而不是断言或越界
        Assert::IsTrue(TcsCharLastOf(sz, 0, ".", 1) == nullptr);

        constexpr char sz2[]{ "ab  " };
        Assert::IsTrue(TcsCharLastNotOf(sz2, 4, " ", 1) == sz2 + 1);
        Assert::IsTrue(TcsCharLastNotOf(sz2, 4, " ", 1, 0) == sz2);
        Assert::IsTrue(TcsCharLastNotOf(sz2, 4, "ab ", 3) == nullptr);
        Assert::IsTrue(TcsCharLastNotOf(sz2, 0, " ", 1) == nullptr);
    }

    TEST_METHOD(FindWrappers)
    {
        Assert::AreEqual(6, FindString("hello world", "world"));
        Assert::AreEqual(StrNPos, FindString("abc", "z"));
        Assert::AreEqual(6, FindStringI("hello WORLD", "world"));
        Assert::AreEqual(StrNPos, FindStringI("hello", "z"));

        Assert::AreEqual(4, FindStringLength("hello world", 11, "o w", 3));
        Assert::AreEqual(StrNPos, FindStringLength("hello world", 11, "z", 1));
        Assert::AreEqual(2, FindStringLengthI("Hello", 5, "LL", 2));

        Assert::AreEqual(3, RFindString("abcabc", 6, "abc", 3));
        Assert::AreEqual(0, RFindString("abcabc", 6, "abc", 3, 2));
        Assert::AreEqual(StrNPos, RFindString("abcabc", 6, "z", 1));
        Assert::AreEqual(3, RFindStringI("aBcAbC", 6, "abc", 3));

        Assert::AreEqual(2, FindChar("hello", 'l'));
        Assert::AreEqual(StrNPos, FindChar("hello", 'z'));

        Assert::AreEqual(2, FindCharLength("hello", 5, 'l'));
        Assert::AreEqual(3, FindCharLength("hello", 5, 'l', 3));
        Assert::AreEqual(StrNPos, FindCharLength("hello", 5, 'z'));

        Assert::AreEqual(3, RFindCharLength("hello", 5, 'l'));
        Assert::AreEqual(2, RFindCharLength("hello", 5, 'l', 2));
        Assert::AreEqual(StrNPos, RFindCharLength("hello", 5, 'z'));
        Assert::AreEqual(StrNPos, RFindCharLength("hello", 0, 'l'));
    }

    TEST_METHOD(FindCharOfWrappers)
    {
        Assert::AreEqual(2, FindCharFirstOf("hello", 5, "lo", 2));
        Assert::AreEqual(3, FindCharFirstOf("hello", 5, "lo", 2, 3));
        Assert::AreEqual(StrNPos, FindCharFirstOf("hello", 5, "xyz", 3));
        Assert::AreEqual(StrNPos, FindCharFirstOf("hello", 0, "lo", 2));

        Assert::AreEqual(2, FindCharFirstNotOf("  ab", 4, " ", 1));
        Assert::AreEqual(StrNPos, FindCharFirstNotOf("    ", 4, " ", 1));

        Assert::AreEqual(3, FindCharLastOf("a.b.c", 5, ".", 1));
        Assert::AreEqual(1, FindCharLastOf("a.b.c", 5, ".", 1, 2));
        Assert::AreEqual(StrNPos, FindCharLastOf("abc", 3, ".", 1));
        Assert::AreEqual(StrNPos, FindCharLastOf("abc", 0, ".", 1));

        Assert::AreEqual(1, FindCharLastNotOf("ab  ", 4, " ", 1));
        Assert::AreEqual(StrNPos, FindCharLastNotOf("    ", 4, " ", 1));
    }

    TEST_METHOD(TrimString_Trim)
    {
        Assert::IsTrue(TrimString("  abc  ", -1) == "abc"sv);
        Assert::IsTrue(TrimString("  abc  ", 7) == "abc"sv);
        Assert::IsTrue(TrimString("abc", -1) == "abc"sv);
        Assert::IsTrue(TrimString("   ", -1).empty());
        Assert::IsTrue(TrimString("", 0).empty());
        Assert::IsTrue(TrimString(" \t\r\nabc\r\n ", -1) == "abc"sv);

        // 宽字符
        Assert::IsTrue(TrimString(L"  abc  ", -1) == L"abc"sv);
    }

    TEST_METHOD(TrimString_TrimLeftAndRight)
    {
        constexpr char sz[]{ "  abc  " };
        Assert::IsTrue(TrimStringLeft(sz, 7) == sz + 2);
        Assert::IsTrue(TrimStringLeft(sz, -1) == sz + 2);
        Assert::IsTrue(TrimStringLeft(sz, 0) == sz);

        Assert::IsTrue(TrimStringRight(sz, 7) == sz + 5);
        Assert::IsTrue(TrimStringRight(sz, -1) == sz + 5);

        constexpr char szSpace[]{ "   " };
        Assert::IsTrue(TrimStringRight(szSpace, 3) == szSpace);
    }

    TEST_METHOD(SplitString_Split)
    {
        std::vector<std::string> v;
        SplitString("a,b,c", 5, ",", 1, 0,
            [&](const char* p, int cch) { v.emplace_back(p, (size_t)cch); });
        Assert::AreEqual<size_t>(3, v.size());
        Assert::IsTrue(v[0] == "a" && v[1] == "b" && v[2] == "c");

        v.clear();
        SplitString("a,,c", 4, ",", 1, 0,
            [&](const char* p, int cch) { v.emplace_back(p, (size_t)cch); });
        Assert::AreEqual<size_t>(3, v.size());
        Assert::IsTrue(v[1].empty());

        // 限制子串数量
        v.clear();
        SplitString("a,b,c", 5, ",", 1, 2,
            [&](const char* p, int cch) { v.emplace_back(p, (size_t)cch); });
        Assert::AreEqual<size_t>(2, v.size());

        // 无分隔符
        v.clear();
        SplitString("abc", 3, ",", 1, 0,
            [&](const char* p, int cch) { v.emplace_back(p, (size_t)cch); });
        Assert::AreEqual<size_t>(1, v.size());
        Assert::IsTrue(v[0] == "abc");

        // 多字符分隔符
        v.clear();
        SplitString("a::b::c", 7, "::", 2, 0,
            [&](const char* p, int cch) { v.emplace_back(p, (size_t)cch); });
        Assert::AreEqual<size_t>(3, v.size());
        Assert::IsTrue(v[2] == "c");
    }

    TEST_METHOD(SplitString_MultipleChar)
    {
        std::vector<std::string> v;
        SplitStringMultipleChar("a,b;c", 5, ",;", 2, 0,
            [&](const char* p, int cch) { v.emplace_back(p, (size_t)cch); });
        Assert::AreEqual<size_t>(3, v.size());
        Assert::IsTrue(v[0] == "a" && v[1] == "b" && v[2] == "c");

        v.clear();
        SplitStringMultipleChar("abc", 3, ",;", 2, 0,
            [&](const char* p, int cch) { v.emplace_back(p, (size_t)cch); });
        Assert::AreEqual<size_t>(1, v.size());
        Assert::IsTrue(v[0] == "abc");
    }

    TEST_METHOD(SplitString_AndCut)
    {
        char sz[]{ "a,b" };
        std::vector<char*> v;
        SplitStringAndCut(sz, 3, ",", 1, v);
        Assert::AreEqual<size_t>(2, v.size());
        Assert::IsTrue(strcmp(v[0], "a") == 0);
        Assert::IsTrue(strcmp(v[1], "b") == 0);

        char sz2[]{ "a,b;c" };
        std::vector<char*> v2;
        SplitStringMultipleCharAndCut(sz2, 5, ",;", 2, v2);
        Assert::AreEqual<size_t>(3, v2.size());
        Assert::IsTrue(strcmp(v2[0], "a") == 0);
        Assert::IsTrue(strcmp(v2[1], "b") == 0);
        Assert::IsTrue(strcmp(v2[2], "c") == 0);
    }

    TEST_METHOD(CompareAndPrefixSuffix)
    {
        Assert::IsTrue(TcsEqualLength("abc", "abc", 3));
        Assert::IsFalse(TcsEqualLength("abc", "abd", 3));
        Assert::IsTrue(TcsEqualLengthI("AbC", "aBc", 3));
        Assert::IsTrue(TcsEqualLength2("abc", 3, "abc", 3));
        Assert::IsFalse(TcsEqualLength2("abc", 3, "ab", 2));
        Assert::IsTrue(TcsEqualLength2I("ABC", 3, "abc", 3));

        Assert::IsTrue(TcsIsStartWithLength2("hello", 5, "he", 2));
        Assert::IsFalse(TcsIsStartWithLength2("hello", 5, "hello!", 6));
        Assert::IsTrue(TcsIsStartWithLength2I("Hello", 5, "he", 2));

        Assert::IsTrue(TcsIsEndWithLength2("hello", 5, "lo", 2));
        Assert::IsFalse(TcsIsEndWithLength2("hello", 5, "hello!", 6));
        Assert::IsTrue(TcsIsEndWithLength2I("hellO", 5, "LO", 2));
    }
};
TS_NS_END
