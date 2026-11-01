#include "pch.h"
#include "../eck/CStringReader.h"

#include <string_view>

using namespace eck;
using namespace std::literals;

namespace
{
    enum class ETest : int { Min = 0, A = 1, B = 2, C = 3, Max = 4 };
    enum class ETestU : unsigned { Min = 0, One = 1, Two = 2, Max = 3 };

    std::string_view Sv(const CStringA& rs) noexcept
    {
        return { rs.Data(), (size_t)rs.Size() };
    }
}

TS_NS_BEGIN
TEST_CLASS(TsCStringReader)
{
public:
    TEST_METHOD(State_ConstructAndQuery)
    {
        CStringReader rDefault{};
        Assert::IsTrue(rDefault.IsEmpty());
        Assert::IsTrue(rDefault.IsEnd());
        Assert::AreEqual<size_t>(0, rDefault.GetRemainingSize());
        Assert::IsTrue(rDefault.ToStringView().empty());

        CStringReader rEmpty{ ""sv };
        Assert::IsTrue(rEmpty.IsEmpty());
        Assert::IsTrue(rEmpty.IsEnd());

        CStringReader r{ "abc"sv };
        Assert::IsFalse(r.IsEmpty());
        Assert::IsFalse(r.IsEnd());
        Assert::AreEqual<size_t>(3, r.GetRemainingSize());
        Assert::IsTrue(r.GetStringView() == "abc"sv);
        Assert::IsTrue(r.ToStringView() == "abc"sv);
    }

    TEST_METHOD(State_AfterRead)
    {
        CStringReader r{ "abc"sv };
        char ch{};
        r.Read(&ch, 1);
        Assert::AreEqual('a', ch);
        Assert::AreEqual<size_t>(2, r.GetRemainingSize());
        Assert::IsTrue(r.GetStringView() == "bc"sv);

        char szTwo[2]{};
        r.Read(szTwo, 2);
        Assert::IsTrue(szTwo[0] == 'b' && szTwo[1] == 'c');
        Assert::IsTrue(r.IsEnd());
        Assert::IsFalse(r.IsEmpty());
        Assert::IsTrue(r.GetStringView().empty());
        Assert::IsTrue(r.ToStringView() == "abc"sv);
    }

    TEST_METHOD(ReadInt_Basic)
    {
        CStringReader r{ "123 -45 +7 0x1F 0b101 777"sv };
        int i{};
        r.ReadInt(i);
        Assert::AreEqual(123, i);
        Assert::IsTrue(r.GetStringView() == " -45 +7 0x1F 0b101 777"sv);

        char ch{};
        r.Read(&ch, 1);
        r.ReadInt(i);
        Assert::AreEqual(-45, i);

        r.Read(&ch, 1);
        r.ReadInt(i);
        Assert::AreEqual(7, i);

        r.Read(&ch, 1);
        r.ReadInt(i, 0);// 自动识别0x/0b前缀
        Assert::AreEqual(31, i);

        r.Read(&ch, 1);
        r.ReadInt(i, 0);
        Assert::AreEqual(5, i);

        r.Read(&ch, 1);
        r.ReadInt(i, 8);
        Assert::AreEqual(511, i);
        Assert::IsTrue(r.IsEnd());

        CStringReader rU{ "4294967295"sv };
        unsigned u{};
        rU.ReadInt(u);
        Assert::AreEqual(4294967295u, u);
    }

    TEST_METHOD(ReadInt_ErrorsAndStrongGuarantee)
    {
        int i = 12345;
        CStringReader rNotNumber{ "abc"sv };
        Assert::ExpectException<CStringReader::XptNotNumber>([&] { rNotNumber.ReadInt(i); });
        Assert::AreEqual(12345, i);
        Assert::AreEqual<size_t>(3, rNotNumber.GetRemainingSize());

        CStringReader rNothing{ ""sv };
        Assert::ExpectException<CStringReader::XptTcvResult>([&] { rNothing.ReadInt(i); });
        Assert::AreEqual(12345, i);

        CStringReader rSign{ "-"sv };
        bool bCaught = false;
        try { rSign.ReadInt(i); }
        catch (const CStringReader::XptTcvResult& x)
        {
            bCaught = true;
            Assert::AreEqual((int)TcvResult::OnlySign, (int)x.e);
        }
        Assert::IsTrue(bCaught);
        Assert::AreEqual<size_t>(1, rSign.GetRemainingSize());

        CStringReader rOverflow{ "99999999999999999999"sv };
        bCaught = false;
        try { rOverflow.ReadInt(i); }
        catch (const CStringReader::XptTcvResult& x)
        {
            bCaught = true;
            Assert::AreEqual((int)TcvResult::Overflow, (int)x.e);
        }
        Assert::IsTrue(bCaught);
        Assert::AreEqual(12345, i);
        Assert::AreEqual<size_t>(20, rOverflow.GetRemainingSize());
    }

    TEST_METHOD(ReadFloat_Basic)
    {
        CStringReader r{ "-3.5e2 abc"sv };
        double d{};
        r.ReadFloat(d);
        Assert::IsTrue(d > -350.1 && d < -349.9);
        Assert::IsTrue(r.GetStringView() == " abc"sv);

        CStringReader rF{ "1.25 -2"sv };
        float f{};
        rF.ReadFloat(f);
        Assert::AreEqual(1.25f, f);

        char ch{};
        rF.Read(&ch, 1);
        rF.ReadFloat(f);
        Assert::AreEqual(-2.0f, f);
        Assert::IsTrue(rF.IsEnd());

        CStringReader rFixed{ "0.1"sv };
        rFixed.ReadFloat(d, TcvFloatFormat::Fixed);
        Assert::IsTrue(d > 0.09 && d < 0.11);
    }

    TEST_METHOD(ReadFloat_ErrorsAndStrongGuarantee)
    {
        double d = 6.5;
        CStringReader r{ "abc"sv };
        bool bCaught = false;
        try { r.ReadFloat(d); }
        catch (const CStringReader::XptTcvResult& x)
        {
            bCaught = true;
            Assert::AreEqual((int)TcvResult::ErrorFormat, (int)x.e);
        }
        Assert::IsTrue(bCaught);
        Assert::IsTrue(d == 6.5);
        Assert::AreEqual<size_t>(3, r.GetRemainingSize());

        CStringReader rEmpty{ ""sv };
        bCaught = false;
        try { rEmpty.ReadFloat(d); }
        catch (const CStringReader::XptTcvResult& x)
        {
            bCaught = true;
            Assert::AreEqual((int)TcvResult::Nothing, (int)x.e);
        }
        Assert::IsTrue(bCaught);
    }

    TEST_METHOD(ReadEnumeration_Basic)
    {
        CStringReader r{ "2"sv };
        ETest e{};
        r.ReadEnumeration(e);
        Assert::IsTrue(e == ETest::B);
        Assert::IsTrue(r.IsEnd());

        CStringReader rU{ "0x2"sv };
        ETestU eu{};
        rU.ReadEnumeration(eu, 0);
        Assert::IsTrue(eu == ETestU::Two);
    }

    TEST_METHOD(ReadEnumeration_OpenInterval)
    {
        // 开区间(Min, Max)，合法值为1..3
        for (int i = 1; i <= 3; ++i)
        {
            const char sz[]{ char('0' + i) };
            CStringReader r{ std::string_view{ sz, 1 } };
            ETest e{};
            r.ReadEnumeration(e, ETest::Min, ETest::Max);
            Assert::AreEqual(i, (int)e);
            Assert::IsTrue(r.IsEnd());
        }

        // 端点不在开区间内
        for (const int i : { 0, 4 })
        {
            const char sz[]{ char('0' + i) };
            CStringReader r{ std::string_view{ sz, 1 } };
            ETest e = ETest::C;
            bool bCaught = false;
            try { r.ReadEnumeration(e, ETest::Min, ETest::Max); }
            catch (const CStringReader::XptEnumerationOutOfRange& x)
            {
                bCaught = true;
                Assert::AreEqual((LONGLONG)i, x.nValue);
                Assert::AreEqual((LONGLONG)0, x.nMin);
                Assert::AreEqual((LONGLONG)4, x.nMax);
            }
            Assert::IsTrue(bCaught);
            Assert::IsTrue(e == ETest::C);// 强异常安全
            Assert::AreEqual<size_t>(1, r.GetRemainingSize());
        }

        CStringReader rBad{ "x"sv };
        ETest eBad{};
        Assert::ExpectException<CStringReader::XptNotNumber>(
            [&] { rBad.ReadEnumeration(eBad, ETest::Min, ETest::Max); });
        Assert::AreEqual<size_t>(1, rBad.GetRemainingSize());

        CStringReader rU{ "2"sv };
        ETestU eu{};
        rU.ReadEnumeration(eu, ETestU::Min, ETestU::Max);
        Assert::IsTrue(eu == ETestU::Two);
    }

    TEST_METHOD(Read_Buffer)
    {
        CStringReader r{ "abcdef"sv };
        char szBuf[8]{};
        r.Read(szBuf, 3);
        Assert::IsTrue(std::string_view{ szBuf, 3 } == "abc"sv);
        Assert::IsTrue(r.GetStringView() == "def"sv);

        r.Read(szBuf, 3);
        Assert::IsTrue(std::string_view{ szBuf, 3 } == "def"sv);
        Assert::IsTrue(r.IsEnd());

        CStringReader rZero{ "ab"sv };
        rZero.Read(szBuf, 0);
        Assert::AreEqual<size_t>(2, rZero.GetRemainingSize());

        CStringReader rOver{ "ab"sv };
        bool bCaught = false;
        try { rOver.Read(szBuf, 3); }
        catch (const CStringReader::XptRange& x)
        {
            bCaught = true;
            Assert::AreEqual<size_t>(2, x.cchMax);
            Assert::IsTrue(x.pCurr == x.pBase + 3);
        }
        Assert::IsTrue(bCaught);
        Assert::AreEqual<size_t>(2, rOver.GetRemainingSize());
    }

    TEST_METHOD(Read_String)
    {
        CStringReader r{ "abcdef"sv };
        CStringA rs{ "P" };
        r.Read(rs, 2);
        Assert::IsTrue(Sv(rs) == "Pab"sv);
        Assert::IsTrue(r.GetStringView() == "cdef"sv);

        bool bCaught = false;
        try { r.Read(rs, 5); }
        catch (const CStringReader::XptRange&) { bCaught = true; }
        Assert::IsTrue(bCaught);
        Assert::IsTrue(Sv(rs) == "Pab"sv);// 失败时不改变目标字符串
        Assert::AreEqual<size_t>(4, r.GetRemainingSize());

        r.Read(rs, 4);
        Assert::IsTrue(Sv(rs) == "Pabcdef"sv);
        Assert::IsTrue(r.IsEnd());
    }

    TEST_METHOD(ReadUntil_CharToBuffer)
    {
        CStringReader r{ "key=value"sv };
        char szBuf[16]{};
        size_t cch = ARRAYSIZE(szBuf);
        r.ReadUntil('=', szBuf, cch);
        Assert::AreEqual<size_t>(3, cch);
        Assert::IsTrue(std::string_view{ szBuf, cch } == "key"sv);
        Assert::IsTrue(r.GetStringView() == "=value"sv);// 终止字符未被消耗

        // 终止字符就在当前位置时读取长度为零
        r.ReadUntil('=', szBuf, cch);
        Assert::AreEqual<size_t>(0, cch);
        Assert::IsTrue(r.GetStringView() == "=value"sv);
    }

    TEST_METHOD(ReadUntil_StringToBuffer)
    {
        CStringReader r{ "a::b::c"sv };
        char szBuf[16]{};
        char szSkip[2]{};
        size_t cch = ARRAYSIZE(szBuf);
        r.ReadUntil("::"sv, szBuf, cch);
        Assert::AreEqual<size_t>(1, cch);
        Assert::IsTrue(szBuf[0] == 'a');
        Assert::IsTrue(r.GetStringView() == "::b::c"sv);

        r.Read(szSkip, 2);
        r.ReadUntil("::"sv, szBuf, cch);
        Assert::AreEqual<size_t>(1, cch);
        Assert::IsTrue(szBuf[0] == 'b');
        Assert::IsTrue(r.GetStringView() == "::c"sv);
    }

    TEST_METHOD(ReadUntil_ToString)
    {
        CStringReader r{ "abc,def"sv };
        CStringA rs{ "P" };
        r.ReadUntil(',', rs);
        Assert::IsTrue(Sv(rs) == "Pabc"sv);
        Assert::IsTrue(r.GetStringView() == ",def"sv);

        char ch{};
        r.Read(&ch, 1);
        r.ReadUntil(',', rs, true);
        Assert::IsTrue(Sv(rs) == "Pabcdef"sv);
        Assert::IsTrue(r.IsEnd());

        CStringReader r2{ "x::y"sv };
        CStringA rs2{};
        r2.ReadUntil("::"sv, rs2);
        Assert::IsTrue(Sv(rs2) == "x"sv);
        Assert::IsTrue(r2.GetStringView() == "::y"sv);
    }

    TEST_METHOD(ReadUntil_ToView)
    {
        CStringReader r{ "key=value"sv };
        CStringReader::TStringView sv;
        r.ReadUntil('=', sv);
        Assert::IsTrue(sv == "key"sv);
        Assert::IsTrue(r.GetStringView() == "=value"sv);

        char ch{};
        r.Read(&ch, 1);
        r.ReadUntil('=', sv, true);
        Assert::IsTrue(sv == "value"sv);
        Assert::IsTrue(r.IsEnd());

        CStringReader r2{ "abc"sv };
        r2.ReadUntil(""sv, sv);// 空模式不消耗任何字符
        Assert::IsTrue(sv.empty());
        Assert::AreEqual<size_t>(3, r2.GetRemainingSize());
    }

    TEST_METHOD(ReadUntil_ReachEndBehavior)
    {
        CStringReader r{ "abcdef"sv };
        char szBuf[16]{};
        size_t cch = ARRAYSIZE(szBuf);
        bool bCaught = false;
        try { r.ReadUntil('z', szBuf, cch); }
        catch (const CStringReader::XptReachEnd&) { bCaught = true; }
        Assert::IsTrue(bCaught);
        Assert::AreEqual<size_t>(6, r.GetRemainingSize());
        Assert::AreEqual<size_t>(ARRAYSIZE(szBuf), cch);

        r.ReadUntil('z', szBuf, cch, true);
        Assert::AreEqual<size_t>(6, cch);
        Assert::IsTrue(std::string_view{ szBuf, cch } == "abcdef"sv);
        Assert::IsTrue(r.IsEnd());

        CStringReader r2{ "abc"sv };
        cch = ARRAYSIZE(szBuf);
        Assert::ExpectException<CStringReader::XptReachEnd>(
            [&] { r2.ReadUntil("zz"sv, szBuf, cch); });
        Assert::AreEqual<size_t>(3, r2.GetRemainingSize());

        r2.ReadUntil("zz"sv, szBuf, cch, true);
        Assert::AreEqual<size_t>(3, cch);
        Assert::IsTrue(r2.IsEnd());
    }

    TEST_METHOD(ReadUntil_BufferTooSmall)
    {
        CStringReader r{ "abcdef"sv };
        char szBuf[2]{};
        size_t cch = ARRAYSIZE(szBuf);
        bool bCaught = false;
        try { r.ReadUntil('z', szBuf, cch, true); }
        catch (const CStringReader::XptBufferTooSmall& x)
        {
            bCaught = true;
            Assert::AreEqual<size_t>(6, x.cch);
        }
        Assert::IsTrue(bCaught);
        Assert::AreEqual<size_t>(2, cch);
        Assert::AreEqual<size_t>(6, r.GetRemainingSize());

        CStringA rs{ "P" };
        CStringReader r2{ "abcdef"sv };
        Assert::ExpectException<CStringReader::XptReachEnd>([&] { r2.ReadUntil('z', rs); });
        Assert::IsTrue(Sv(rs) == "P"sv);
        Assert::AreEqual<size_t>(6, r2.GetRemainingSize());
    }

    TEST_METHOD(StepTo_Char)
    {
        CStringReader r{ "aa,bb,cc"sv };
        r.StepTo(',');
        Assert::IsTrue(r.GetStringView() == ",bb,cc"sv);

        char ch{};
        r.Read(&ch, 1);
        Assert::AreEqual(',', ch);
        r.StepTo(',');
        Assert::IsTrue(r.GetStringView() == ",cc"sv);

        bool bCaught = false;
        try { r.StepTo(';'); }
        catch (const CStringReader::XptReachEnd&) { bCaught = true; }
        Assert::IsTrue(bCaught);
        Assert::IsTrue(r.GetStringView() == ",cc"sv);

        r.StepTo(';', true);
        Assert::IsTrue(r.IsEnd());

        r.StepTo(';', true);// 已在结尾
        Assert::IsTrue(r.IsEnd());

        CStringReader r2{ ",a"sv };
        r2.StepTo(',');
        Assert::IsTrue(r2.GetStringView() == ",a"sv);// 当前位置即匹配
    }

    TEST_METHOD(StepTo_String)
    {
        CStringReader r{ "hello world"sv };
        r.StepTo("world"sv);
        Assert::IsTrue(r.GetStringView() == "world"sv);

        CStringReader r2{ "abc"sv };
        Assert::ExpectException<CStringReader::XptReachEnd>([&] { r2.StepTo("cb"sv); });
        Assert::AreEqual<size_t>(3, r2.GetRemainingSize());

        r2.StepTo("bc"sv);
        Assert::IsTrue(r2.GetStringView() == "bc"sv);

        CStringReader r3{ "abc"sv };
        r3.StepTo(""sv);// 空模式不移动
        Assert::AreEqual<size_t>(3, r3.GetRemainingSize());
    }

    TEST_METHOD(StepBackTo_Char)
    {
        CStringReader r{ "aa,bb,cc"sv };
        r.StepTo(',');
        char ch{};
        r.Read(&ch, 1);
        r.StepTo(',');
        Assert::IsTrue(r.GetStringView() == ",cc"sv);

        // 只在已读内容中查找
        r.StepBackTo(',');
        Assert::IsTrue(r.GetStringView() == ",bb,cc"sv);

        bool bCaught = false;
        try { r.StepBackTo(';'); }
        catch (const CStringReader::XptReachEnd&) { bCaught = true; }
        Assert::IsTrue(bCaught);
        Assert::IsTrue(r.GetStringView() == ",bb,cc"sv);

        r.StepBackTo(';', true);// 允许回到起点
        Assert::AreEqual<size_t>(8, r.GetRemainingSize());
        Assert::IsTrue(r.GetStringView() == "aa,bb,cc"sv);

        r.StepBackTo(';', true);
        Assert::AreEqual<size_t>(8, r.GetRemainingSize());

        // 已读内容为空
        Assert::ExpectException<CStringReader::XptReachEnd>([&] { r.StepBackTo(','); });

        // 读到结尾后可回到最后一个分隔符
        CStringReader r2{ "a,b"sv };
        char szBuf[8]{};
        size_t cch = ARRAYSIZE(szBuf);
        r2.ReadUntil(';', szBuf, cch, true);
        Assert::IsTrue(r2.IsEnd());
        r2.StepBackTo(',');
        Assert::IsTrue(r2.GetStringView() == ",b"sv);
    }

    TEST_METHOD(StepBackTo_String)
    {
        CStringReader r{ "ab::cd::ef"sv };
        char szSkip[2]{};
        r.StepTo("::"sv);
        r.Read(szSkip, 2);
        r.StepTo("::"sv);
        Assert::IsTrue(r.GetStringView() == "::ef"sv);

        r.StepBackTo("::"sv);
        Assert::IsTrue(r.GetStringView() == "::cd::ef"sv);

        r.StepBackTo("ab"sv);
        Assert::IsTrue(r.GetStringView() == "ab::cd::ef"sv);

        // 已读内容为空
        Assert::ExpectException<CStringReader::XptReachEnd>([&] { r.StepBackTo("ab"sv); });

        // 跨越当前位置的匹配不算
        CStringReader r2{ "a::b"sv };
        r2.StepTo('b');
        bool bCaught = false;
        try { r2.StepBackTo(":b"sv); }
        catch (const CStringReader::XptReachEnd&) { bCaught = true; }
        Assert::IsTrue(bCaught);
        Assert::AreEqual<size_t>(1, r2.GetRemainingSize());

        r2.StepBackTo(":b"sv, true);
        Assert::AreEqual<size_t>(4, r2.GetRemainingSize());
        Assert::IsTrue(r2.GetStringView() == "a::b"sv);
    }

    TEST_METHOD(Step_CaseInsensitive)
    {
        CStringReader r{ "AbC,dEf"sv };
        r.StepToI('c');
        Assert::IsTrue(r.GetStringView() == "C,dEf"sv);

        r.StepToI("dEF"sv);
        Assert::IsTrue(r.GetStringView() == "dEf"sv);

        r.StepBackToI('c');
        Assert::IsTrue(r.GetStringView() == "C,dEf"sv);

        // 已读内容不足以容纳模式
        Assert::ExpectException<CStringReader::XptReachEnd>([&] { r.StepBackToI("aBc"sv); });
        r.StepBackToI("aBc"sv, true);
        Assert::AreEqual<size_t>(7, r.GetRemainingSize());
        Assert::IsTrue(r.GetStringView() == "AbC,dEf"sv);

        // 必须找到与允许到结尾
        Assert::ExpectException<CStringReader::XptReachEnd>([&] { r.StepToI('z'); });
        Assert::AreEqual<size_t>(7, r.GetRemainingSize());
        r.StepToI('z', true);
        Assert::IsTrue(r.IsEnd());
    }

    TEST_METHOD(Exceptions_DerivedFromBase)
    {
        CStringReader r{ "abc"sv };
        char szBuf[2]{};
        size_t cch = ARRAYSIZE(szBuf);
        bool bCaught = false;
        try { r.ReadUntil('z', szBuf, cch); }
        catch (const CStringReader::Xpt&) { bCaught = true; }
        Assert::IsTrue(bCaught);// XptReachEnd

        bCaught = false;
        try { r.ReadUntil('z', szBuf, cch, true); }
        catch (const CStringReader::Xpt&) { bCaught = true; }
        Assert::IsTrue(bCaught);// XptBufferTooSmall
    }
};
TS_NS_END
