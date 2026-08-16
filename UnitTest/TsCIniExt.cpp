#include "pch.h"
#include "../eck/CIniExt.h"

using namespace eck;

static void ExpectResult(IniResult eExpected, IniResult eActual)
{
    Assert::AreEqual(static_cast<int>(eExpected), static_cast<int>(eActual));
}

TS_NS_BEGIN
TEST_CLASS(TsIniExt)
{
public:

    TEST_METHOD(EmptyInput)
    {
        CIniExt ini;
        const auto r = ini.Load(L"");
        ExpectResult(IniResult::Ok, r);
        Assert::IsTrue((bool)ini.IsEmpty());
    }

    TEST_METHOD(SingleSectionSingleKeyValue)
    {
        CIniExt ini;
        const auto r = ini.Load(L"[Section]\nKey=Value\n");
        ExpectResult(IniResult::Ok, r);
        Assert::IsFalse((bool)ini.IsEmpty());

        const auto sec = ini.GetSection(L"Section");
        Assert::IsTrue((bool)sec);
        Assert::IsTrue(sec->GetName().ToStringView() == L"Section"sv);

        const auto kv = ini.GetKeyValue(sec, L"Key");
        Assert::IsTrue((bool)kv);
        Assert::IsTrue(kv.GetString() == L"Value"sv);
    }

    TEST_METHOD(MultipleSectionsAreSiblingsAtRoot)
    {
        CIniExt ini;
        const auto r = ini.Load(L"[A]\nK=1\n[B]\nK=2\n[C]\nK=3\n");
        ExpectResult(IniResult::Ok, r);

        Assert::IsTrue((bool)ini.GetSection(L"A"));
        Assert::IsTrue((bool)ini.GetSection(L"B"));
        Assert::IsTrue((bool)ini.GetSection(L"C"));

        Assert::IsTrue(ini.GetKeyValue(L"A", L"K").GetString() == L"1"sv);
        Assert::IsTrue(ini.GetKeyValue(L"B", L"K").GetString() == L"2"sv);
        Assert::IsTrue(ini.GetKeyValue(L"C", L"K").GetString() == L"3"sv);
    }

    TEST_METHOD(MultipleKeyValuesInOneSection)
    {
        CIniExt ini;
        const auto r = ini.Load(L"[S]\nA=1\nB=2\nC=3\n");
        ExpectResult(IniResult::Ok, r);

        const auto sec = ini.GetSection(L"S");
        Assert::IsTrue((bool)sec);
        Assert::IsTrue(ini.GetKeyValue(sec, L"A").GetString() == L"1"sv);
        Assert::IsTrue(ini.GetKeyValue(sec, L"B").GetString() == L"2"sv);
        Assert::IsTrue(ini.GetKeyValue(sec, L"C").GetString() == L"3"sv);
    }

    TEST_METHOD(LeadingBomBeforeFirstSectionIsTolerated)
    {
        CIniExt ini;
        std::wstring text;
        text.push_back(static_cast<wchar_t>(0xFEFF));
        text += L"[S]\nK=V\n";
        const auto r = ini.Load(text.c_str());
        ExpectResult(IniResult::Ok, r);
        Assert::IsTrue(ini.GetKeyValue(L"S", L"K").GetString() == L"V"sv);
    }

    TEST_METHOD(SectionWithNoTrailingNewlineAtEndOfInput)
    {
        CIniExt ini;
        const auto r = ini.Load(L"[S]\nK=V");
        ExpectResult(IniResult::Ok, r);
        Assert::IsTrue(ini.GetKeyValue(L"S", L"K").GetString() == L"V"sv);
    }

    TEST_METHOD(MissingClosingBracket)
    {
        CIniExt ini;
        const auto r = ini.Load(L"[Section");
        ExpectResult(IniResult::SecRBracketNotFound, r);
    }

    TEST_METHOD(EmptySectionName)
    {
        CIniExt ini;
        const auto r = ini.Load(L"[]\n");
        ExpectResult(IniResult::SecEmptyName, r);
    }

    TEST_METHOD(TrailingCharsAfterBracket)
    {
        CIniExt ini;
        // 'x' immediately follows ']' -- neither newline nor ';' -- illegal.
        const auto r = ini.Load(L"[Section]x\n");
        ExpectResult(IniResult::SecIllegalChar, r);
    }

    TEST_METHOD(GarbageBeforeFirstSection)
    {
        CIniExt ini;
        const auto r = ini.Load(L"garbage[Section]\n");
        ExpectResult(IniResult::SecIllegalChar, r);
    }

    TEST_METHOD(EmptyKeyName)
    {
        CIniExt ini;
        const auto r = ini.Load(L"[S]\n=Value\n");
        ExpectResult(IniResult::KvEmptyKey, r);
    }

    TEST_METHOD(MissingEqualsSign_NonEscape)
    {
        CIniExt ini;
        const auto r = ini.Load(L"[S]\nJustAKeyNoEquals");
        ExpectResult(IniResult::KvEqualNotFound, r);
    }

    TEST_METHOD(MissingEqualsSign_Escape)
    {
        CIniExt ini;
        const auto r = ini.Load(L"[S]\nJustAKeyNoEquals", -1, INIE_IF_ESCAPE);
        ExpectResult(IniResult::KvEqualNotFound, r);
    }

    TEST_METHOD(TrailingBackslashInSectionName_Escape)
    {
        CIniExt ini;
        const auto r = ini.Load(L"[Sec\\", -1, INIE_IF_ESCAPE);
        ExpectResult(IniResult::EscapeAtEnd, r);
    }

    TEST_METHOD(TrailingBackslashInKey_Escape)
    {
        CIniExt ini;
        const auto r = ini.Load(L"[S]\nKey\\", -1, INIE_IF_ESCAPE);
        ExpectResult(IniResult::EscapeAtEnd, r);
    }

    TEST_METHOD(StandardEscapesInValue)
    {
        CIniExt ini;
        // \n \t \\ \; decoded; '=' need not be escaped inside a value.
        const auto r = ini.Load(L"[S]\nK=a\\nb\\tc\\\\d\\;e\n", -1, INIE_IF_ESCAPE);
        ExpectResult(IniResult::Ok, r);

        std::wstring expected;
        expected += L'a'; expected += L'\n'; expected += L'b'; expected += L'\t';
        expected += L'c'; expected += L'\\'; expected += L'd'; expected += L';'; expected += L'e';

        const auto val = ini.GetKeyValue(L"S", L"K").GetString();
        Assert::IsTrue(val == std::wstring_view(expected));
    }

    TEST_METHOD(EscapedSemicolon)
    {
        CIniExt ini;
        const auto r = ini.Load(L"[S]\nK=before\\;after\n", -1, INIE_IF_ESCAPE);
        ExpectResult(IniResult::Ok, r);
        Assert::IsTrue(ini.GetKeyValue(L"S", L"K").GetString() == L"before;after"sv);
    }

    TEST_METHOD(UnrecognizedEscape)
    {
        CIniExt ini;
        // '\x' is not a recognized escape -- both characters are kept as-is.
        const auto r = ini.Load(L"[S]\nK=a\\xb\n", -1, INIE_IF_ESCAPE);
        ExpectResult(IniResult::Ok, r);
        Assert::IsTrue(ini.GetKeyValue(L"S", L"K").GetString() == L"a\\xb"sv);
    }

    TEST_METHOD(EscapedBracketsInSectionName)
    {
        CIniExt ini;
        const auto r = ini.Load(L"[Sec\\[tion\\]]\nK=V\n", -1, INIE_IF_ESCAPE);
        ExpectResult(IniResult::Ok, r);
        Assert::IsTrue((bool)ini.GetSection(L"Sec[tion]"));
    }

    TEST_METHOD(EscapedEqualsInKey)
    {
        CIniExt ini;
        const auto r = ini.Load(L"[S]\nA\\=B=Value\n", -1, INIE_IF_ESCAPE);
        ExpectResult(IniResult::Ok, r);
        Assert::IsTrue(ini.GetKeyValue(L"S", L"A=B").GetString() == L"Value"sv);
    }

    TEST_METHOD(WithoutEscapeFlag)
    {
        CIniExt ini;
        const auto r = ini.Load(L"[S]\nK=a\\nb\n"); // no INIE_IF_ESCAPE
        ExpectResult(IniResult::Ok, r);
        // Without escaping, "\n" inside the value is two literal characters.
        Assert::IsTrue(ini.GetKeyValue(L"S", L"K").GetString() == L"a\\nb"sv);
    }

    TEST_METHOD(WithoutKeepSpace)
    {
        CIniExt ini;
        const auto r = ini.Load(L"[S]\nKey = Value\n"); // default flags
        ExpectResult(IniResult::Ok, r);

        const auto sec = ini.GetSection(L"S");
        Assert::IsTrue((bool)ini.GetKeyValue(sec, L"Key"));    // trimmed key matches
        Assert::IsFalse((bool)ini.GetKeyValue(sec, L"Key "));  // untrimmed key does not
        Assert::IsTrue(ini.GetKeyValue(sec, L"Key").GetString() == L"Value"sv);
    }

    TEST_METHOD(WithKeepSpace)
    {
        CIniExt ini;
        const auto r = ini.Load(L"[S]\nKey = Value\n", -1, INIE_IF_KEEP_SPACE);
        ExpectResult(IniResult::Ok, r);

        const auto sec = ini.GetSection(L"S");
        Assert::IsFalse((bool)ini.GetKeyValue(sec, L"Key"));   // trimmed key no longer matches
        Assert::IsTrue((bool)ini.GetKeyValue(sec, L"Key "));   // exact untrimmed key matches
        Assert::IsTrue(ini.GetKeyValue(sec, L"Key ").GetString() == L" Value"sv);
    }

    TEST_METHOD(InlineCommentAfterKeyValue)
    {
        CIniExt ini;
        const auto r = ini.Load(L"[S]\nKey=Value;a trailing comment\n");
        ExpectResult(IniResult::Ok, r);
        // The comment terminates the value at ';'.
        Assert::IsTrue(ini.GetKeyValue(L"S", L"Key").GetString() == L"Value"sv);
    }

    TEST_METHOD(StandaloneCommentBeforeSection)
    {
        CIniExt ini;
        const auto r = ini.Load(L"; a standalone comment\n[S]\nKey=Value\n");
        ExpectResult(IniResult::Ok, r);
        Assert::IsTrue(ini.GetKeyValue(L"S", L"Key").GetString() == L"Value"sv);
    }

    TEST_METHOD(CommentImmediatelyAfterSectionBracket)
    {
        CIniExt ini;
        // No space between ']' and ';' -- allowed by the "char right after
        // ']' must be EOL or ';'" rule in UnescapeSectionName/ScanSectionName.
        const auto r = ini.Load(L"[S];comment\nKey=Value\n");
        ExpectResult(IniResult::Ok, r);
        Assert::IsTrue(ini.GetKeyValue(L"S", L"Key").GetString() == L"Value"sv);
    }

    TEST_METHOD(MultiLineCommentsBetweenEntries)
    {
        CIniExt ini;
        const auto r = ini.Load(
            L"[S]\n"
            L"; line 1\n"
            L"; line 2\n"
            L"Key=Value\n");
        ExpectResult(IniResult::Ok, r);
        Assert::IsTrue(ini.GetKeyValue(L"S", L"Key").GetString() == L"Value"sv);
    }

    TEST_METHOD(SimpleContainerOpenAndClose)
    {
        CIniExt ini;
        auto r = ini.Load(LR"(
[>A]
  Key=Val
[<A]

[Test]
  Key=Val2
[Test2]
  Key=Val3

)");
        ExpectResult(IniResult::Ok, r);
        Assert::IsTrue((bool)ini.GetSection(L"A"));
        Assert::IsTrue(ini.GetKeyValue(L"A", L"Key").GetString() == L"Val"sv);
        Assert::IsTrue(ini.GetKeyValue(L"Test", L"Key").GetString() == L"Val2"sv);
        Assert::IsTrue(ini.GetKeyValue(L"Test2", L"Key").GetString() == L"Val3"sv);
    }

    TEST_METHOD(BareCloseDirective)
    {
        CIniExt ini;
        const auto r = ini.Load(L"[>A]\nKey=Val\n[<]\n");
        ExpectResult(IniResult::Ok, r);
        Assert::IsTrue((bool)ini.GetSection(L"A"));
    }

    TEST_METHOD(ChildSectionNestedInsideContainer)
    {
        CIniExt ini;
        const auto r = ini.Load(
            L"[>Parent]\n"
            L"[Child]\n"
            L"Key=Val\n"
            L"[<]\n"          // closes Child (still dangling on the stack)
            L"[<Parent]\n");  // now closes Parent by name
        ExpectResult(IniResult::Ok, r);

        const auto parent = ini.GetSection(L"Parent");
        Assert::IsTrue((bool)parent);
        const auto child = ini.GetSection(parent, L"Child");
        Assert::IsTrue((bool)child);
        Assert::IsTrue(ini.GetKeyValue(child, L"Key").GetString() == L"Val"sv);
    }

    TEST_METHOD(ClosingByNameWithMismatchedTop)
    {
        CIniExt ini;
        // "Child" is still the top of the section stack (never popped),
        // so closing "[Parent]" by name here does not match it.
        const auto r = ini.Load(
            L"[>Parent]\n"
            L"[Child]\n"
            L"Key=Val\n"
            L"[<Parent]\n");
        ExpectResult(IniResult::SecContainerNotMatch, r);
    }

    TEST_METHOD(DuplicateContainerNameAtSameLevel)
    {
        CIniExt ini;
        // Both "A" containers are children of the still-open "Root"
        // container, so they collide in the same child set.
        const auto r = ini.Load(
            L"[>Root]\n"
            L"[>A]\n"
            L"[<A]\n"
            L"[>A]\n");
        ExpectResult(IniResult::SecDuplicate, r);
    }

    TEST_METHOD(NonContainerSectionsWithSameName)
    {
        CIniExt ini;
        // Duplicate detection only fires for container ("[>Name]")
        // sections; repeating a plain section header re-uses the
        // existing section, so the key/values end up merged into a
        // single "S" section instead of producing an error.
        const auto r = ini.Load(L"[S]\nA=1\n[S]\nB=2\n");
        ExpectResult(IniResult::Ok, r);

        const auto sec = ini.GetSection(L"S");
        Assert::IsTrue((bool)sec);
        Assert::IsTrue(ini.GetKeyValue(sec, L"A").GetString() == L"1"sv);
        Assert::IsTrue(ini.GetKeyValue(sec, L"B").GetString() == L"2"sv);
    }

    TEST_METHOD(GetSection_NotFound)
    {
        CIniExt ini;
        ini.Load(L"[S]\nK=V\n");
        const auto sec = ini.GetSection(L"DoesNotExist");
        Assert::IsFalse((bool)sec);
    }

    TEST_METHOD(GetKeyValue_KeyNotFound)
    {
        CIniExt ini;
        ini.Load(L"[S]\nK=V\n");
        const auto sec = ini.GetSection(L"S");
        const auto kv = ini.GetKeyValue(sec, L"NoSuchKey");
        Assert::IsFalse((bool)kv);
        Assert::IsTrue((bool)kv.IsEmpty());
    }

    TEST_METHOD(GetKeyValue_SectionNotFound)
    {
        CIniExt ini;
        ini.Load(L"[S]\nK=V\n");
        const auto kv = ini.GetKeyValue(ini.GetSection(L"NoSuchSection"), L"K");
        Assert::IsFalse((bool)kv);
    }

    TEST_METHOD(GetSection_TwoArgumentOverload)
    {
        CIniExt ini;
        ini.Load(L"[>Parent]\n[Child]\nK=V\n[<Parent]\n");
        const auto parent = ini.GetSection(L"Parent");
        Assert::IsTrue((bool)parent);
        const auto child = ini.GetSection(parent, L"Child");
        Assert::IsTrue((bool)child);
    }

    TEST_METHOD(GetString)
    {
        CIniExt ini;
        ini.Load(L"[S]\nK=Hello\n");
        Assert::IsTrue(ini.GetKeyValue(L"S", L"K").GetString() == L"Hello"sv);
    }

    TEST_METHOD(GetString_MissingKey)
    {
        CIniExt ini;
        ini.Load(L"[S]\nK=Hello\n");
        const auto sv = ini.GetKeyValue(L"S", L"Missing").GetString(L"fallback");
        Assert::IsTrue(sv == L"fallback"sv);
    }

    TEST_METHOD(GetInt_ParsesDecimal)
    {
        CIniExt ini;
        ini.Load(L"[S]\nK=42\n");
        Assert::AreEqual(42, ini.GetKeyValue(L"S", L"K").GetInt<int>());
    }

    TEST_METHOD(GetInt_ParsesNegativeDecimal)
    {
        CIniExt ini;
        ini.Load(L"[S]\nK=-7\n");
        Assert::AreEqual(-7, ini.GetKeyValue(L"S", L"K").GetInt<int>());
    }

    TEST_METHOD(GetInt_MissingKey)
    {
        CIniExt ini;
        ini.Load(L"[S]\nK=1\n");
        Assert::AreEqual(99, ini.GetKeyValue(L"S", L"Missing").GetInt<int>(99));
    }

    TEST_METHOD(GetInt_HexFlag_ParsesHexDigits)
    {
        CIniExt ini;
        ini.Load(L"[S]\nK=2A\n");
        Assert::AreEqual(42, ini.GetKeyValue(L"S", L"K").GetInt<int>(0, TRUE));
    }

    TEST_METHOD(GetFloat_ParsesDecimal)
    {
        CIniExt ini;
        ini.Load(L"[S]\nK=3.5\n");
        Assert::AreEqual(3.5, ini.GetKeyValue(L"S", L"K").GetFloat<double>(), 0.0001);
    }

    TEST_METHOD(GetBool_RecognizesTrueVariants)
    {
        CIniExt ini;
        ini.Load(L"[S]\nA=true\nB=TRUE\nC=1\nD=真\n");
        const auto sec = ini.GetSection(L"S");
        Assert::IsTrue((bool)ini.GetKeyValue(sec, L"A").GetBool());
        Assert::IsTrue((bool)ini.GetKeyValue(sec, L"B").GetBool());
        Assert::IsTrue((bool)ini.GetKeyValue(sec, L"C").GetBool());
        Assert::IsTrue((bool)ini.GetKeyValue(sec, L"D").GetBool());
    }

    TEST_METHOD(GetBool_RecognizesFalseVariants)
    {
        CIniExt ini;
        ini.Load(L"[S]\nA=false\nB=0\nC=no\n");
        const auto sec = ini.GetSection(L"S");
        Assert::IsFalse((bool)ini.GetKeyValue(sec, L"A").GetBool());
        Assert::IsFalse((bool)ini.GetKeyValue(sec, L"B").GetBool());
        Assert::IsFalse((bool)ini.GetKeyValue(sec, L"C").GetBool());
    }

    TEST_METHOD(GetBool_MissingKey)
    {
        CIniExt ini;
        ini.Load(L"[S]\nA=true\n");
        Assert::IsTrue((bool)ini.GetKeyValue(L"S", L"Missing").GetBool(TRUE));
        Assert::IsFalse((bool)ini.GetKeyValue(L"S", L"Missing").GetBool(FALSE));
    }

    enum class ETestEnum : int { A = 0, B = 1, C = 2 };

    TEST_METHOD(GetEnumeration_ParsesUnderlyingInteger)
    {
        CIniExt ini;
        ini.Load(L"[S]\nK=1\n");
        Assert::IsTrue(ini.GetKeyValue(L"S", L"K").GetEnumeration<ETestEnum>() == ETestEnum::B);
    }

    TEST_METHOD(GetEnumeration_MissingKey)
    {
        CIniExt ini;
        ini.Load(L"[S]\nK=1\n");
        Assert::IsTrue(
            ini.GetKeyValue(L"S", L"Missing").GetEnumeration<ETestEnum>(ETestEnum::C)
            == ETestEnum::C);
    }

    TEST_METHOD(Clear)
    {
        CIniExt ini;
        ini.Load(L"[S]\nK=V\n");
        Assert::IsFalse((bool)ini.IsEmpty());
        ini.Clear();
        Assert::IsTrue((bool)ini.IsEmpty());
        Assert::IsFalse((bool)ini.GetSection(L"S"));
    }

    TEST_METHOD(SecondLoadCall)
    {
        CIniExt ini;
        ini.Load(L"[First]\nK=1\n");
        Assert::IsTrue((bool)ini.GetSection(L"First"));

        const auto r = ini.Load(L"[Second]\nK=2\n");
        ExpectResult(IniResult::Ok, r);
        Assert::IsFalse((bool)ini.GetSection(L"First"));
        Assert::IsTrue((bool)ini.GetSection(L"Second"));
    }

    TEST_METHOD(LoadThatEndsInError)
    {
        CIniExt ini;
        auto r = ini.Load(L"[Bad");
        ExpectResult(IniResult::SecRBracketNotFound, r);

        // The object should still be usable for a subsequent, valid load.
        r = ini.Load(L"[Good]\nK=V\n");
        ExpectResult(IniResult::Ok, r);
        Assert::IsTrue(ini.GetKeyValue(L"Good", L"K").GetString() == L"V"sv);
    }

    // ------------------------------------------------------------------
    // CIniExtT 模板实例化：IsOrderedMap / IsAllowMultiKeys / IsCaseSensitive
    // ------------------------------------------------------------------

    TEST_METHOD(OrderedMap_ForEachKeyValueInSortedOrder)
    {
        CIniExtT<true> ini;
        const auto r = ini.Load(L"[S]\nZ=1\nA=2\nM=3\n");
        ExpectResult(IniResult::Ok, r);

        const auto sec = ini.GetSection(L"S");
        Assert::IsTrue((bool)sec);

        std::vector<std::wstring> keys;
        sec->ForEachKeyValue([&](const auto& kv)
            {
                keys.emplace_back(kv.GetName().ToStringView());
            });
        // 有序容器按键名升序排列：A, M, Z
        Assert::AreEqual<size_t>(3, keys.size());
        Assert::IsTrue(keys[0] == L"A"sv);
        Assert::IsTrue(keys[1] == L"M"sv);
        Assert::IsTrue(keys[2] == L"Z"sv);
    }

    TEST_METHOD(AllowMultiKeys_Ordered_AllKeysStored)
    {
        CIniExtT<true, true> ini;
        const auto r = ini.Load(L"[S]\nK=1\nK=2\nK=3\n");
        ExpectResult(IniResult::Ok, r);

        const auto sec = ini.GetSection(L"S");
        Assert::IsTrue((bool)sec);

        // multiset::find 返回首个等价元素。
        Assert::IsTrue(ini.GetKeyValue(sec, L"K").GetString() == L"1"sv);

        int n = 0;
        std::multiset<std::wstring> seen;
        sec->ForEachKeyValue([&](const auto& kv)
            {
                ++n;
                seen.emplace(kv.rsValue.ToStringView());
            });
        Assert::AreEqual(3, n);
        Assert::AreEqual<size_t>(1, seen.count(L"1"));
        Assert::AreEqual<size_t>(1, seen.count(L"2"));
        Assert::AreEqual<size_t>(1, seen.count(L"3"));
    }

    TEST_METHOD(AllowMultiKeys_Unordered_AllKeysStored)
    {
        CIniExtT<false, true> ini;
        const auto r = ini.Load(L"[S]\nK=1\nK=2\nK=3\n");
        ExpectResult(IniResult::Ok, r);

        const auto sec = ini.GetSection(L"S");
        Assert::IsTrue((bool)sec);
        // 无序多重集至少能找到其中一个键。
        Assert::IsTrue((bool)ini.GetKeyValue(sec, L"K"));

        int n = 0;
        sec->ForEachKeyValue([&](const auto&) { ++n; });
        Assert::AreEqual(3, n);
    }

    TEST_METHOD(AllowMultiKeys_DuplicateContainerAllowed)
    {
        CIniExtT<false, true> ini;
        const auto r = ini.Load(L"[>A]\nK=1\n[<A]\n[>A]\nK=2\n[<A]\n");
        ExpectResult(IniResult::Ok, r);
        Assert::IsTrue((bool)ini.GetSection(L"A"));
    }

    TEST_METHOD(CaseInsensitive_GetSectionAndKeyValue)
    {
        CIniExtT<false, false, false> ini;
        const auto r = ini.Load(L"[Section]\nKey=Value\n");
        ExpectResult(IniResult::Ok, r);

        Assert::IsTrue((bool)ini.GetSection(L"section"));
        Assert::IsTrue((bool)ini.GetSection(L"SECTION"));
        Assert::IsTrue(ini.GetKeyValue(L"Section", L"key").GetString() == L"Value"sv);
        Assert::IsTrue(ini.GetKeyValue(L"section", L"KEY").GetString() == L"Value"sv);
    }

    TEST_METHOD(CaseSensitive_DefaultDistinguishesCase)
    {
        CIniExt ini;
        ini.Load(L"[Section]\nKey=Value\n");
        Assert::IsFalse((bool)ini.GetSection(L"section"));
        Assert::IsFalse((bool)ini.GetSection(L"SECTION"));
        Assert::IsFalse((bool)ini.GetKeyValue(L"Section", L"key"));
    }

    TEST_METHOD(CombinedTemplate_OrderedMultiCaseInsensitive)
    {
        CIniExtT<true, true, false> ini;
        const auto r = ini.Load(L"[S]\nB=1\nA=2\nA=3\n");
        ExpectResult(IniResult::Ok, r);

        // 不区分大小写查找节。
        const auto sec = ini.GetSection(L"s");
        Assert::IsTrue((bool)sec);

        std::vector<std::wstring> names;
        sec->ForEachKeyValue([&](const auto& kv)
            {
                names.emplace_back(kv.GetName().ToStringView());
            });
        // 有序且不区分大小写排序：A, A, B
        Assert::AreEqual<size_t>(3, names.size());
        Assert::IsTrue(names[0] == L"A"sv);
        Assert::IsTrue(names[1] == L"A"sv);
        Assert::IsTrue(names[2] == L"B"sv);
    }

    // ------------------------------------------------------------------
    // 遍历 API：ForEachSectionInOrder / ForEachValueInOrder /
    //           ForEachKeyValue / ForEachChild
    // ------------------------------------------------------------------

    TEST_METHOD(ForEachSectionInOrder_IsInsertionOrder)
    {
        CIniExtT<> ini;
        ini.Load(L"[>P]\n[>C]\n[<C]\n[<P]\n[Z]\n");

        std::vector<std::wstring> all;
        ini.ForEachSectionInOrder([&](const auto& sec)
            {
                all.emplace_back(sec.GetName().ToStringView());
            });
        // 深度优先 + 按 uId 排序 => 插入顺序：P, C, Z
        Assert::AreEqual<size_t>(3, all.size());
        Assert::IsTrue(all[0] == L"P"sv);
        Assert::IsTrue(all[1] == L"C"sv);
        Assert::IsTrue(all[2] == L"Z"sv);

        // 指定父节时只遍历其子树。
        std::vector<std::wstring> sub;
        ini.ForEachSectionInOrder([&](const auto& sec)
            {
                sub.emplace_back(sec.GetName().ToStringView());
            }, ini.GetSection(L"P"));
        Assert::AreEqual<size_t>(1, sub.size());
        Assert::IsTrue(sub[0] == L"C"sv);
    }

    TEST_METHOD(ForEachValueInOrder_IsInsertionOrder)
    {
        CIniExtT<> ini;
        ini.Load(L"[S]\nK3=3\nK1=1\nK2=2\n");
        const auto sec = ini.GetSection(L"S");

        std::vector<std::wstring> vals;
        ini.ForEachValueInOrder([&](const auto& kv)
            {
                vals.emplace_back(kv.rsValue.ToStringView());
            }, sec);
        // 按插入顺序：3, 1, 2
        Assert::AreEqual<size_t>(3, vals.size());
        Assert::IsTrue(vals[0] == L"3"sv);
        Assert::IsTrue(vals[1] == L"1"sv);
        Assert::IsTrue(vals[2] == L"2"sv);
    }

    TEST_METHOD(ForEachKeyValue_IteratesAll)
    {
        CIniExtT<> ini;
        ini.Load(L"[S]\nA=1\nB=2\nC=3\n");
        const auto sec = ini.GetSection(L"S");

        int n = 0;
        sec->ForEachKeyValue([&](const auto&) { ++n; });
        Assert::AreEqual(3, n);
    }

    TEST_METHOD(ForEachChild_IteratesChildSections)
    {
        CIniExtT<> ini;
        ini.Load(L"[>P]\n[Child1]\nK=V\n[>Child2]\n[<Child2]\n[<P]\n");
        const auto p = ini.GetSection(L"P");
        Assert::IsTrue((bool)p);

        std::vector<std::wstring> names;
        p->ForEachChild([&](const auto& sec)
            {
                names.emplace_back(sec.GetName().ToStringView());
            });
        Assert::AreEqual<size_t>(2, names.size());
    }

    // ------------------------------------------------------------------
    // 条目内省 API 与标志位
    // ------------------------------------------------------------------

    TEST_METHOD(GetIdAndGetName)
    {
        CIniExtT<> ini;
        ini.Load(L"[S]\nK=V\n");
        const auto sec = ini.GetSection(L"S");
        const auto kv = ini.GetKeyValue(sec, L"K");

        Assert::IsTrue(sec->GetName().ToStringView() == L"S"sv);
        Assert::AreEqual<UINT>(0, sec->GetId());
        Assert::IsTrue(kv->GetName().ToStringView() == L"K"sv);
        Assert::AreEqual<UINT>(1, kv->GetId());
    }

    TEST_METHOD(EntryOperatorWstringView)
    {
        CIniExtT<> ini;
        ini.Load(L"[S]\nK=V\n");
        const auto sec = ini.GetSection(L"S");
        const auto kv = ini.GetKeyValue(sec, L"K");
        Assert::IsTrue(static_cast<std::wstring_view>(*sec) == L"S"sv);
        Assert::IsTrue(static_cast<std::wstring_view>(*kv) == L"K"sv);
    }

    TEST_METHOD(CommentAttachedToKeyValue)
    {
        CIniExtT<> ini;
        ini.Load(L"[S]\nK=V;comment\n");
        const auto kv = ini.GetKeyValue(L"S", L"K");
        Assert::IsTrue((bool)kv);
        Assert::IsTrue((bool)(kv->uFlags & INIE_EF_HAS_COMMENTS));
        Assert::IsTrue(kv->rsComment.ToStringView() == L"comment"sv);
    }

    TEST_METHOD(CommentAttachedToSection)
    {
        CIniExtT<> ini;
        ini.Load(L"[S];section comment\nK=V\n");
        const auto sec = ini.GetSection(L"S");
        Assert::IsTrue((bool)(sec->uFlags & INIE_EF_HAS_COMMENTS));
        Assert::IsTrue(sec->rsComment.ToStringView() == L"section comment"sv);
    }

    TEST_METHOD(ContainerSectionHasFlag)
    {
        CIniExtT<> ini;
        ini.Load(L"[>A]\n[<A]\n[B]\n");
        const auto a = ini.GetSection(L"A");
        const auto b = ini.GetSection(L"B");
        Assert::IsTrue((bool)(a->uFlags & INIE_EF_IS_CONTAINER));
        Assert::IsFalse((bool)(b->uFlags & INIE_EF_IS_CONTAINER));
    }

    // ------------------------------------------------------------------
    // 补充边界场景
    // ------------------------------------------------------------------

    TEST_METHOD(EmptyValue)
    {
        CIniExtT<> ini;
        ini.Load(L"[S]\nK=\n");
        const auto kv = ini.GetKeyValue(L"S", L"K");
        Assert::IsTrue((bool)kv);
        Assert::IsTrue((bool)kv.IsEmpty());
        Assert::IsTrue(kv.GetString() == L""sv);
        Assert::AreEqual(42, kv.GetInt<int>(42));
        Assert::IsTrue((bool)kv.GetBool(TRUE));
        Assert::IsFalse((bool)kv.GetBool(FALSE));
    }

    TEST_METHOD(EscapeCarriageReturnAndNul)
    {
        CIniExtT<> ini;
        const auto r = ini.Load(L"[S]\nK=a\\rb\\0c\n", -1, INIE_IF_ESCAPE);
        ExpectResult(IniResult::Ok, r);

        std::wstring expected;
        expected += L'a';
        expected += L'\r';
        expected += L'b';
        expected += L'\0';
        expected += L'c';
        Assert::IsTrue(ini.GetKeyValue(L"S", L"K").GetString() == std::wstring_view(expected));
    }

    TEST_METHOD(GetInt_LongLong)
    {
        CIniExtT<> ini;
        ini.Load(L"[S]\nA=1234567890123\n");
        Assert::AreEqual<long long>(
            1234567890123LL, ini.GetKeyValue(L"S", L"A").GetInt<long long>());
    }

    TEST_METHOD(GetFloat_Float)
    {
        CIniExtT<> ini;
        ini.Load(L"[S]\nA=1.5\n");
        Assert::AreEqual(1.5f, ini.GetKeyValue(L"S", L"A").GetFloat<float>(), 0.001f);
    }

    TEST_METHOD(GetSectionOnInvalidSectionContext)
    {
        CIniExtT<> ini;
        ini.Load(L"[S]\nK=V\n");
        const auto bad = ini.GetSection(L"Missing");
        Assert::IsFalse((bool)bad);
        Assert::IsFalse((bool)ini.GetSection(bad, L"X"));
        Assert::IsFalse((bool)ini.GetKeyValue(bad, L"K"));
    }

    TEST_METHOD(ClearResetsIds)
    {
        CIniExtT<> ini;
        ini.Load(L"[A]\nK=1\n");
        Assert::AreEqual<UINT>(0, ini.GetSection(L"A")->GetId());
        ini.Clear();
        ini.Load(L"[B]\n");
        Assert::AreEqual<UINT>(0, ini.GetSection(L"B")->GetId());
    }

    // ------------------------------------------------------------------
    // 回归测试：静态走查修复的缺陷
    // ------------------------------------------------------------------

    TEST_METHOD(GetKey_ReturnsKeyName)
    {
        CIniExtT<> ini;
        ini.Load(L"[S]\nK=V\n");
        const auto kv = ini.GetKeyValue(L"S", L"K");
        Assert::IsTrue((bool)kv);
        Assert::IsTrue(kv.GetKey().ToStringView() == L"K"sv);
    }

    TEST_METHOD(TrailingBackslashInValue_Escape)
    {
        CIniExtT<> ini;
        const auto r = ini.Load(L"[S]\nK=a\\", -1, INIE_IF_ESCAPE);
        ExpectResult(IniResult::EscapeAtEnd, r);
    }

    TEST_METHOD(BareCloseDirectiveWithEmptyStack)
    {
        CIniExtT<> ini;
        const auto r = ini.Load(L"[<]\n");
        ExpectResult(IniResult::SecContainerNotMatch, r);
    }

    TEST_METHOD(ForEachValueInOrder_InvalidSection)
    {
        CIniExtT<> ini;
        ini.Load(L"[S]\nK=V\n");
        const auto bad = ini.GetSection(L"Missing");
        int n = 0;
        ini.ForEachValueInOrder([&](const auto&) { ++n; }, bad);
        Assert::AreEqual(0, n);
    }

    TEST_METHOD(EmptyValueAtEndOfInput)
    {
        CIniExtT<> ini;
        const auto r = ini.Load(L"[S]\nK=");
        ExpectResult(IniResult::Ok, r);
        const auto kv = ini.GetKeyValue(L"S", L"K");
        Assert::IsTrue((bool)kv);
        Assert::IsTrue((bool)kv.IsEmpty());
    }

    TEST_METHOD(GetEnumeration_OnConstContext)
    {
        CIniExtT<> ini;
        ini.Load(L"[S]\nK=1\n");
        const auto kv = ini.GetKeyValue(L"S", L"K");
        Assert::IsTrue(kv.GetEnumeration<ETestEnum>() == ETestEnum::B);
    }
};
TS_NS_END