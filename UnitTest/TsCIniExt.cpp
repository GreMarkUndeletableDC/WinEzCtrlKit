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
        ExpectResult(IniResult::KvSepNotFound, r);
    }

    TEST_METHOD(MissingEqualsSign_Escape)
    {
        CIniExt ini;
        const auto r = ini.Load(L"[S]\nJustAKeyNoEquals", -1, INIE_IF_ESCAPE);
        ExpectResult(IniResult::KvSepNotFound, r);
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
};
TS_NS_END