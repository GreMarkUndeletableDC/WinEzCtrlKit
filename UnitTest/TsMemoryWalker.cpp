#include "pch.h"
#include "../eck/MemoryWalker.h"

using namespace eck;

void AssertBytesEqual(const void* pExpected, const void* pActual, size_t cb)
{
    Assert::IsTrue(std::memcmp(pExpected, pActual, cb) == 0);
}

template<class TException, class F>
static TException ExpectException(F&& fn)
{
    try
    {
        fn();
    }
    catch (const TException& e)
    {
        return e;
    }
    catch (...)
    {
        Assert::Fail(L"Unexpected exception type.");
    }

    Assert::Fail(L"Expected exception was not thrown.");
    return {};
}

template<class T>
static void AssertBytesEqual(
    const T* pExpected,
    const T* pActual,
    size_t count)
{
    Assert::IsTrue(
        std::memcmp(pExpected, pActual, count * sizeof(T)) == 0);
}

template<class T>
static void AssertBytesEqual(
    const std::vector<T>& expected,
    const void* actual,
    size_t count)
{
    Assert::AreEqual(expected.size(), count);
    AssertBytesEqual(expected.data(),
        static_cast<const T*>(actual),
        count);
}

TS_NS_BEGIN
TEST_CLASS(CMemoryWalkerTest)
{
public:

    // ========================================================================
    // Construction / SetData
    // ========================================================================

    TEST_METHOD(DefaultConstruct_CursorIsNullAndEmpty)
    {
        CMemoryReader reader;

        Assert::IsNull(reader.Data());
        Assert::AreEqual(size_t{ 0 }, reader.GetPosition());
        Assert::AreEqual(size_t{ 0 }, reader.GetRemainingSize());
        Assert::IsTrue(reader.IsEnd());

        CMemoryWalker walker;

        Assert::IsNull(walker.Data());
        Assert::AreEqual(size_t{ 0 }, walker.GetPosition());
        Assert::AreEqual(size_t{ 0 }, walker.GetRemainingSize());
        Assert::IsTrue(walker.IsEnd());
    }

    TEST_METHOD(Constructor_InitializesRange)
    {
        BYTE buffer[16]{};

        CMemoryReader reader{ buffer, sizeof(buffer) };

        Assert::AreEqual(
            reinterpret_cast<const BYTE*>(buffer),
            reader.Data());

        Assert::AreEqual(size_t{ 0 }, reader.GetPosition());
        Assert::AreEqual(sizeof(buffer), reader.GetRemainingSize());
        Assert::IsFalse(reader.IsEnd());
    }

    TEST_METHOD(SetData_ResetsCursor)
    {
        BYTE buffer1[8]{};
        BYTE buffer2[16]{};

        CMemoryWalker walker{ buffer1, sizeof(buffer1) };

        walker += 5;
        Assert::AreEqual(size_t{ 5 }, walker.GetPosition());

        walker.SetData(buffer2, sizeof(buffer2));

        Assert::AreEqual(size_t{ 0 }, walker.GetPosition());
        Assert::AreEqual(sizeof(buffer2), walker.GetRemainingSize());
        Assert::AreEqual(
            reinterpret_cast<BYTE*>(buffer2),
            walker.Data());
    }

    // ========================================================================
    // Basic Read / Write
    // ========================================================================

    TEST_METHOD(Write_CopiesBytesAndAdvances)
    {
        BYTE buffer[8]{};
        const BYTE source[] = { 1, 2, 3, 4 };

        CMemoryWalker walker{ buffer, sizeof(buffer) };

        auto& result = walker.Write(source, sizeof(source));

        Assert::AreEqual(
            reinterpret_cast<void*>(&walker),
            reinterpret_cast<void*>(&result));

        Assert::AreEqual(size_t{ 4 }, walker.GetPosition());
        Assert::AreEqual(BYTE{ 1 }, buffer[0]);
        Assert::AreEqual(BYTE{ 2 }, buffer[1]);
        Assert::AreEqual(BYTE{ 3 }, buffer[2]);
        Assert::AreEqual(BYTE{ 4 }, buffer[3]);
    }

    TEST_METHOD(Read_CopiesBytesAndAdvances)
    {
        const BYTE buffer[] = { 10, 20, 30, 40, 50 };

        BYTE output[3]{};

        CMemoryReader reader{ buffer, sizeof(buffer) };

        auto& result = reader.Read(output, sizeof(output));

        Assert::AreEqual(
            reinterpret_cast<void*>(&reader),
            reinterpret_cast<void*>(&result));

        Assert::AreEqual(size_t{ 3 }, reader.GetPosition());
        Assert::AreEqual(BYTE{ 10 }, output[0]);
        Assert::AreEqual(BYTE{ 20 }, output[1]);
        Assert::AreEqual(BYTE{ 30 }, output[2]);
    }

    TEST_METHOD(WriteZeroBytes_DoesNotAdvance)
    {
        BYTE buffer[8]{};
        BYTE source[3]{ 1, 2, 3 };

        CMemoryWalker walker{ buffer, sizeof(buffer) };

        walker.Write(source, 0);

        Assert::AreEqual(size_t{ 0 }, walker.GetPosition());
        Assert::AreEqual(sizeof(buffer), walker.GetRemainingSize());
    }

    TEST_METHOD(ReadZeroBytes_DoesNotAdvance)
    {
        BYTE buffer[8]{};
        BYTE output[3]{};

        CMemoryReader reader{ buffer, sizeof(buffer) };

        reader.Read(output, 0);

        Assert::AreEqual(size_t{ 0 }, reader.GetPosition());
        Assert::AreEqual(sizeof(buffer), reader.GetRemainingSize());
    }

    // ========================================================================
    // Reversed read/write
    // ========================================================================

    TEST_METHOD(WriteReversed_ReversesSource)
    {
        BYTE buffer[4]{};
        const BYTE source[] = { 1, 2, 3, 4 };

        CMemoryWalker walker{ buffer, sizeof(buffer) };

        walker.WriteReversed(source, sizeof(source));

        const BYTE expected[] = { 4, 3, 2, 1 };

        AssertBytesEqual(expected, buffer, 4);
        Assert::AreEqual(size_t{ 4 }, walker.GetPosition());
    }

    TEST_METHOD(ReadReversed_ReversesDestination)
    {
        const BYTE buffer[] = { 1, 2, 3, 4 };

        BYTE output[4]{};

        CMemoryReader reader{ buffer, sizeof(buffer) };

        reader.ReadReversed(output, sizeof(output));

        const BYTE expected[] = { 4, 3, 2, 1 };

        AssertBytesEqual(expected, output, 4);
        Assert::AreEqual(size_t{ 4 }, reader.GetPosition());
    }

    // ========================================================================
    // Trivial type operators
    // ========================================================================

    TEST_METHOD(OperatorWrite_Read_TrivialType)
    {
        BYTE buffer[32]{};

        const uint32_t a = 0x12345678u;
        const int16_t b = -1234;
        const float c = 123.5f;

        CMemoryWalker writer{ buffer, sizeof(buffer) };

        writer << a << b << c;

        Assert::AreEqual(
            sizeof(a) + sizeof(b) + sizeof(c),
            writer.GetPosition());

        CMemoryReader reader{ buffer, sizeof(buffer) };

        uint32_t a2{};
        int16_t b2{};
        float c2{};

        reader >> a2 >> b2 >> c2;

        Assert::AreEqual(a, a2);
        Assert::AreEqual(b, b2);
        Assert::AreEqual(c, c2);
        Assert::AreEqual(writer.GetPosition(), reader.GetPosition());
    }

    TEST_METHOD(WriteReversed_TrivialType)
    {
        BYTE buffer[sizeof(uint32_t)]{};

        constexpr uint32_t value = 0x12345678u;

        CMemoryWalker walker{ buffer, sizeof(buffer) };
        walker.WriteReversed(value);

        const BYTE expected[] = {
            0x12, 0x34, 0x56, 0x78
        };

        AssertBytesEqual(expected, buffer, sizeof(expected));
    }

    TEST_METHOD(ReadReversed_TrivialType)
    {
        const UINT buffer = 0x12345678u;

        CMemoryReader reader{ &buffer, sizeof(buffer) };

        uint32_t value{};
        reader.ReadReversed(value);

        Assert::AreEqual(uint32_t{ 0x78563412u }, value);
        Assert::AreEqual(sizeof(value), reader.GetPosition());
    }

    // ========================================================================
    // std::basic_string
    // ========================================================================

    TEST_METHOD(OperatorWrite_String_AppendsNullTerminator)
    {
        BYTE buffer[32]{};
        const std::string value = "Hello";

        CMemoryWalker walker{ buffer, sizeof(buffer) };
        walker << value;

        const size_t expectedSize =
            (value.size() + 1) * sizeof(char);

        Assert::AreEqual(expectedSize, walker.GetPosition());

        Assert::AreEqual(
            std::string("Hello"),
            std::string(reinterpret_cast<char*>(buffer)));

        Assert::AreEqual(char{}, reinterpret_cast<char*>(buffer)[5]);
    }

    TEST_METHOD(OperatorWrite_WString_AppendsNullTerminator)
    {
        wchar_t buffer[32]{};
        const std::wstring value = L"Hello";

        CMemoryWalker walker{
            reinterpret_cast<BYTE*>(buffer),
            sizeof(buffer)
        };

        walker << value;

        Assert::AreEqual(
            (value.size() + 1) * sizeof(wchar_t),
            walker.GetPosition());

        Assert::AreEqual(
            std::wstring(L"Hello"),
            std::wstring(buffer));
    }

    // ========================================================================
    // std::basic_string_view
    // ========================================================================

    TEST_METHOD(OperatorWrite_StringView_AppendsNull)
    {
        BYTE buffer[32]{};
        constexpr std::string_view value = "Hello";

        CMemoryWalker walker{ buffer, sizeof(buffer) };

        walker << value;

        Assert::AreEqual(
            (value.size() + 1) * sizeof(char),
            walker.GetPosition());

        const auto actual =
            reinterpret_cast<const char*>(buffer);

        Assert::AreEqual(
            std::string("Hello"),
            std::string(actual));
    }

    TEST_METHOD(OperatorWrite_WStringView_AppendsNull)
    {
        wchar_t buffer[32]{};
        constexpr std::wstring_view value = L"Hello";

        CMemoryWalker walker{
            reinterpret_cast<BYTE*>(buffer),
            sizeof(buffer)
        };

        walker << value;

        Assert::AreEqual(
            (value.size() + 1) * sizeof(wchar_t),
            walker.GetPosition());

        Assert::AreEqual(
            std::wstring(L"Hello"),
            std::wstring(buffer));
    }

    // ========================================================================
    // vector
    // ========================================================================

    TEST_METHOD(OperatorWrite_Vector)
    {
        BYTE buffer[32]{};

        const std::vector<uint32_t> value{
            10, 20, 30, 40
        };

        CMemoryWalker walker{ buffer, sizeof(buffer) };

        walker << value;

        Assert::AreEqual(
            value.size() * sizeof(uint32_t),
            walker.GetPosition());

        Assert::IsTrue(
            std::memcmp(
                buffer,
                value.data(),
                value.size() * sizeof(uint32_t)) == 0);
    }

    TEST_METHOD(OperatorWrite_EmptyVector)
    {
        BYTE buffer[8]{};

        const std::vector<uint32_t> value;

        CMemoryWalker walker{ buffer, sizeof(buffer) };

        walker << value;

        Assert::AreEqual(size_t{ 0 }, walker.GetPosition());
    }

    // ========================================================================
    // span
    // ========================================================================

    TEST_METHOD(OperatorWrite_Span)
    {
        BYTE buffer[16]{};

        const std::array<uint16_t, 4> value{
            100, 200, 300, 400
        };

        std::span<const uint16_t> view{ value };

        CMemoryWalker walker{ buffer, sizeof(buffer) };

        walker << view;

        Assert::AreEqual(
            value.size() * sizeof(uint16_t),
            walker.GetPosition());

        Assert::IsTrue(
            std::memcmp(
                buffer,
                value.data(),
                value.size() * sizeof(uint16_t)) == 0);
    }

    // ========================================================================
    // CByteBuffer
    // ========================================================================

    TEST_METHOD(OperatorWrite_CByteBuffer)
    {
        const BYTE source[] = {
            11, 22, 33, 44, 55
        };

        BYTE buffer[16]{};

        CByteBuffer byteBuffer{ source, sizeof(source) };

        CMemoryWalker walker{ buffer, sizeof(buffer) };

        walker << byteBuffer;

        Assert::AreEqual(
            sizeof(source),
            walker.GetPosition());

        Assert::IsTrue(
            std::memcmp(
                buffer,
                byteBuffer.Data(),
                byteBuffer.Size()) == 0);
    }

    TEST_METHOD(OperatorWrite_EmptyCByteBuffer)
    {
        BYTE buffer[16]{};

        CByteBuffer byteBuffer{};

        CMemoryWalker walker{ buffer, sizeof(buffer) };

        walker << byteBuffer;

        Assert::AreEqual(size_t{ 0 }, walker.GetPosition());
    }

    // ========================================================================
    // CString
    // ========================================================================

    TEST_METHOD(OperatorWrite_CString)
    {
        BYTE buffer[64]{};

        CStringW value{ L"Hello" };

        CMemoryWalker walker{ buffer, sizeof(buffer) };

        walker << value;

        Assert::AreEqual(
            value.ByteSize(),
            walker.GetPosition());

        Assert::IsTrue(
            std::memcmp(
                buffer,
                value.Data(),
                value.ByteSize()) == 0);
    }

    // ========================================================================
    // CountStringLength
    // ========================================================================

    TEST_METHOD(CountStringLength_FindsTerminator)
    {
        const char buffer[] = "Hello";

        CMemoryReader reader{
            buffer,
            sizeof(buffer)
        };

        Assert::AreEqual(
            5,
            reader.CountStringLength<char>());

        Assert::AreEqual(size_t{ 0 }, reader.GetPosition());
    }

    TEST_METHOD(CountStringLength_WideString)
    {
        const wchar_t buffer[] = L"Hello";

        CMemoryReader reader{
            buffer,
            sizeof(buffer)
        };

        Assert::AreEqual(
            5,
            reader.CountStringLength<wchar_t>());
    }

    TEST_METHOD(CountStringLength_OnlyScansRemainingData)
    {
        const char buffer[] = {
            'X', 'X', 'A', 'B', '\0', 'Z'
        };

        CMemoryReader reader{
            buffer,
            sizeof(buffer)
        };

        reader += 2;

        Assert::AreEqual(
            2,
            reader.CountStringLength<char>());
    }

    TEST_METHOD(CountStringLength_ThrowsWhenNoNullTerminator)
    {
        const char buffer[] = {
            'A', 'B', 'C', 'D'
        };

        CMemoryReader reader{
            buffer,
            sizeof(buffer)
        };

        const auto ex =
            ExpectException<CMemoryReader::XptRange>(
                [&]()
                {
                    (void)reader.CountStringLength<char>();
                });

        Assert::AreEqual(
            reinterpret_cast<const BYTE*>(buffer),
            ex.pBase);

        Assert::AreEqual(
            sizeof(buffer),
            ex.cbMax);

        Assert::AreEqual(
            reinterpret_cast<const BYTE*>(buffer + sizeof(buffer)),
            ex.pCurr);
    }

    TEST_METHOD(CountStringLengthOrEnd_ReturnsLengthWhenFound)
    {
        const char buffer[] = "Hello";

        CMemoryReader reader{
            buffer,
            sizeof(buffer)
        };

        Assert::AreEqual(
            5,
            reader.CountStringLengthOrEnd<char>());
    }

    TEST_METHOD(CountStringLengthOrEnd_ReturnsRemainingLengthWhenNotFound)
    {
        const char buffer[] = {
            'A', 'B', 'C', 'D'
        };

        CMemoryReader reader{
            buffer,
            sizeof(buffer)
        };

        Assert::AreEqual(
            4,
            reader.CountStringLengthOrEnd<char>());
    }

    TEST_METHOD(CountStringLengthOrEnd_DoesNotThrowWithoutTerminator)
    {
        const char buffer[] = {
            'A', 'B', 'C', 'D'
        };

        CMemoryReader reader{
            buffer,
            sizeof(buffer)
        };

        const int length =
            reader.CountStringLengthOrEnd<char>();

        Assert::AreEqual(4, length);
        Assert::AreEqual(size_t{ 0 }, reader.GetPosition());
    }

    // ========================================================================
    // operator >> CString
    // ========================================================================

    TEST_METHOD(OperatorRead_CString_ReadsNullTerminatedString)
    {
        const wchar_t source[] = L"Hello";

        CMemoryReader reader{
            source,
            sizeof(source)
        };

        CStringW result;

        reader >> result;

        Assert::AreEqual(
            sizeof(source),
            reader.GetPosition());

        Assert::AreEqual(
            5,
            result.Size());

        Assert::AreEqual(
            std::wstring(L"Hello"),
            std::wstring(result.Data()));
    }

    TEST_METHOD(OperatorRead_CString_ReadsEmptyString)
    {
        const wchar_t source[] = L"";

        CMemoryReader reader{
            source,
            sizeof(source)
        };

        CStringW result;

        reader >> result;

        Assert::AreEqual(
            sizeof(wchar_t),
            reader.GetPosition());

        Assert::AreEqual(
            0,
            result.Size());
    }

    TEST_METHOD(OperatorRead_CString_ThrowsWithoutTerminator)
    {
        const wchar_t source[] = {
            L'A', L'B', L'C'
        };

        CMemoryReader reader{
            source,
            sizeof(source)
        };

        CStringW result;

        const auto ex =
            ExpectException<CMemoryReader::XptRange>(
                [&]()
                {
                    reader >> result;
                });

        Assert::AreEqual(
            reinterpret_cast<const BYTE*>(source),
            ex.pBase);

        Assert::AreEqual(
            sizeof(source),
            ex.cbMax);

        Assert::AreEqual(
            reinterpret_cast<const BYTE*>(source + 3),
            ex.pCurr);
    }

    // ========================================================================
    // SkipPointer
    // ========================================================================

    TEST_METHOD(SkipPointer_ReturnsPointerToCurrentPosition)
    {
        BYTE buffer[16]{};

        const uint32_t value = 0x12345678u;
        std::memcpy(buffer, &value, sizeof(value));

        CMemoryReader reader{
            buffer,
            sizeof(buffer)
        };

        uint32_t* p = nullptr;

        reader.SkipPointer(p);

        Assert::IsNotNull(p);
        Assert::AreEqual(
            reinterpret_cast<BYTE*>(buffer),
            reinterpret_cast<BYTE*>(p));

        Assert::AreEqual(
            value,
            *p);

        Assert::AreEqual(
            sizeof(uint32_t),
            reader.GetPosition());
    }

    TEST_METHOD(SkipPointer_ThrowsAtInsufficientData)
    {
        BYTE buffer[2]{};

        CMemoryReader reader{
            buffer,
            sizeof(buffer)
        };

        uint32_t* p = nullptr;

        const auto ex =
            ExpectException<CMemoryReader::XptRange>(
                [&]()
                {
                    reader.SkipPointer(p);
                });

        Assert::AreEqual(
            reinterpret_cast<const BYTE*>(buffer),
            ex.pBase);

        Assert::AreEqual(
            size_t{ 2 },
            ex.cbMax);

        Assert::AreEqual(
            reinterpret_cast<const BYTE*>(buffer + sizeof(uint32_t)),
            ex.pCurr);

        Assert::IsNull(p);
        Assert::AreEqual(size_t{ 0 }, reader.GetPosition());
    }

    // ========================================================================
    // Seek / position
    // ========================================================================

    TEST_METHOD(SeekToBegin_ResetsPosition)
    {
        BYTE buffer[32]{};

        CMemoryReader reader{ buffer, sizeof(buffer) };

        reader += 20;

        Assert::AreEqual(size_t{ 20 }, reader.GetPosition());

        reader.SeekToBegin();

        Assert::AreEqual(size_t{ 0 }, reader.GetPosition());
        Assert::AreEqual(sizeof(buffer), reader.GetRemainingSize());
        Assert::AreEqual(
            reinterpret_cast<const BYTE*>(buffer),
            reader.Data());
    }

    TEST_METHOD(SeekToEnd_MovesToEnd)
    {
        BYTE buffer[32]{};

        CMemoryReader reader{ buffer, sizeof(buffer) };

        reader.SeekToEnd();

        Assert::AreEqual(sizeof(buffer), reader.GetPosition());
        Assert::AreEqual(size_t{ 0 }, reader.GetRemainingSize());
        Assert::IsTrue(reader.IsEnd());

        Assert::AreEqual(
            reinterpret_cast<const BYTE*>(buffer + sizeof(buffer)),
            reader.Data());
    }

    TEST_METHOD(Seek_ToValidPosition)
    {
        BYTE buffer[32]{};

        CMemoryReader reader{ buffer, sizeof(buffer) };

        reader.Seek(17);

        Assert::AreEqual(size_t{ 17 }, reader.GetPosition());
        Assert::AreEqual(size_t{ 15 }, reader.GetRemainingSize());

        Assert::AreEqual(
            reinterpret_cast<const BYTE*>(buffer + 17),
            reader.Data());
    }

    TEST_METHOD(Seek_ToEndIsValid)
    {
        BYTE buffer[32]{};

        CMemoryReader reader{ buffer, sizeof(buffer) };

        reader.Seek(sizeof(buffer));

        Assert::AreEqual(
            sizeof(buffer),
            reader.GetPosition());

        Assert::IsTrue(reader.IsEnd());
    }

    TEST_METHOD(Seek_BeyondEndThrows)
    {
        BYTE buffer[32]{};

        CMemoryReader reader{ buffer, sizeof(buffer) };

        reader += 2;

        const auto ex =
            ExpectException<CMemoryReader::XptRange>(
                [&]()
                {
                    reader.Seek(40);
                });

        Assert::AreEqual(
            reinterpret_cast<const BYTE*>(buffer),
            ex.pBase);

        Assert::AreEqual(
            sizeof(buffer),
            ex.cbMax);

        Assert::AreEqual(
            reinterpret_cast<const BYTE*>(buffer + 40),
            ex.pCurr);

        Assert::AreEqual(size_t{ 2 }, reader.GetPosition());
    }

    // ========================================================================
    // operator += / -=
    // ========================================================================

    TEST_METHOD(OperatorPlusEquals_MovesForward)
    {
        BYTE buffer[16]{};

        CMemoryReader reader{ buffer, sizeof(buffer) };

        reader += 5;

        Assert::AreEqual(size_t{ 5 }, reader.GetPosition());
        Assert::AreEqual(size_t{ 11 }, reader.GetRemainingSize());
    }

    TEST_METHOD(OperatorMinusEquals_MovesBackward)
    {
        BYTE buffer[16]{};

        CMemoryReader reader{ buffer, sizeof(buffer) };

        reader.Seek(10);
        reader -= 4;

        Assert::AreEqual(size_t{ 6 }, reader.GetPosition());
        Assert::AreEqual(size_t{ 10 }, reader.GetRemainingSize());
    }

    TEST_METHOD(OperatorPlusEquals_ExactEndIsValid)
    {
        BYTE buffer[16]{};

        CMemoryReader reader{ buffer, sizeof(buffer) };

        reader += sizeof(buffer);

        Assert::AreEqual(
            sizeof(buffer),
            reader.GetPosition());

        Assert::IsTrue(reader.IsEnd());
    }

    TEST_METHOD(OperatorPlusEquals_BeyondEndThrows)
    {
        BYTE buffer[16]{};

        CMemoryReader reader{ buffer, sizeof(buffer) };

        const auto ex =
            ExpectException<CMemoryReader::XptRange>(
                [&]()
                {
                    reader += sizeof(buffer) + 1;
                });

        Assert::AreEqual(
            reinterpret_cast<const BYTE*>(buffer),
            ex.pBase);

        Assert::AreEqual(
            sizeof(buffer),
            ex.cbMax);

        Assert::AreEqual(
            reinterpret_cast<const BYTE*>(buffer + sizeof(buffer) + 1),
            ex.pCurr);

        Assert::AreEqual(size_t{ 0 }, reader.GetPosition());
    }

    TEST_METHOD(OperatorMinusEquals_BeyondBeginThrows)
    {
        BYTE buffer[16]{};

        CMemoryReader reader{ buffer, sizeof(buffer) };

        reader.Seek(5);

        const auto ex =
            ExpectException<CMemoryReader::XptRange>(
                [&]()
                {
                    reader -= 6;
                });

        Assert::AreEqual(
            reinterpret_cast<const BYTE*>(buffer),
            ex.pBase);

        Assert::AreEqual(
            sizeof(buffer),
            ex.cbMax);

        Assert::AreEqual(
            reinterpret_cast<const BYTE*>(buffer - 1),
            ex.pCurr);

        Assert::AreEqual(size_t{ 5 }, reader.GetPosition());
    }

    // ========================================================================
    // End / remaining / position
    // ========================================================================

    TEST_METHOD(PositionAndRemaining_StayConsistent)
    {
        BYTE buffer[100]{};

        CMemoryReader reader{ buffer, sizeof(buffer) };

        const size_t positions[] = {
            0, 1, 25, 50, 99, 100
        };

        for (const auto pos : positions)
        {
            reader.Seek(pos);

            Assert::AreEqual(
                pos,
                reader.GetPosition());

            Assert::AreEqual(
                sizeof(buffer) - pos,
                reader.GetRemainingSize());

            Assert::AreEqual(
                pos == sizeof(buffer),
                reader.IsEnd() != FALSE);
        }
    }

    // ========================================================================
    // Upper-bound exceptions
    // ========================================================================

    TEST_METHOD(Read_BeyondEndThrowsRange)
    {
        BYTE buffer[8]{};
        BYTE output[16]{};

        CMemoryReader reader{ buffer, sizeof(buffer) };

        reader.Seek(4);

        const auto ex =
            ExpectException<CMemoryReader::XptRange>(
                [&]()
                {
                    reader.Read(output, 5);
                });

        Assert::AreEqual(
            reinterpret_cast<const BYTE*>(buffer),
            ex.pBase);

        Assert::AreEqual(
            sizeof(buffer),
            ex.cbMax);

        Assert::AreEqual(
            reinterpret_cast<const BYTE*>(buffer + 9),
            ex.pCurr);

        // 失败操作不能移动 cursor。
        Assert::AreEqual(
            size_t{ 4 },
            reader.GetPosition());
    }

    TEST_METHOD(Write_BeyondEndThrowsRange)
    {
        BYTE buffer[8]{};
        BYTE source[16]{};

        CMemoryWalker walker{ buffer, sizeof(buffer) };

        walker.Seek(4);

        const auto ex =
            ExpectException<CMemoryWalker::XptRange>(
                [&]()
                {
                    walker.Write(source, 5);
                });

        Assert::AreEqual(
            reinterpret_cast<const BYTE*>(buffer),
            ex.pBase);

        Assert::AreEqual(
            sizeof(buffer),
            ex.cbMax);

        Assert::AreEqual(
            reinterpret_cast<const BYTE*>(buffer + 9),
            ex.pCurr);

        Assert::AreEqual(
            size_t{ 4 },
            walker.GetPosition());
    }

    TEST_METHOD(ReadReversed_BeyondEndThrowsRange)
    {
        BYTE buffer[8]{};
        BYTE output[16]{};

        CMemoryReader reader{ buffer, sizeof(buffer) };

        reader.Seek(6);

        const auto ex =
            ExpectException<CMemoryReader::XptRange>(
                [&]()
                {
                    reader.ReadReversed(output, 3);
                });

        Assert::AreEqual(
            reinterpret_cast<const BYTE*>(buffer + 9),
            ex.pCurr);

        Assert::AreEqual(size_t{ 6 }, reader.GetPosition());
    }

    TEST_METHOD(WriteReversed_BeyondEndThrowsRange)
    {
        BYTE buffer[8]{};
        BYTE source[16]{};

        CMemoryWalker walker{ buffer, sizeof(buffer) };

        walker.Seek(6);

        const auto ex =
            ExpectException<CMemoryWalker::XptRange>(
                [&]()
                {
                    walker.WriteReversed(source, 3);
                });

        Assert::AreEqual(
            reinterpret_cast<const BYTE*>(buffer + 9),
            ex.pCurr);

        Assert::AreEqual(size_t{ 6 }, walker.GetPosition());
    }

    // ========================================================================
    // CheckDataLengthSafe / CheckDataLength
    // ========================================================================

    TEST_METHOD(CheckDataLengthSafe_PositionAndLength_Valid)
    {
        BYTE buffer[100]{};

        CMemoryReader reader{ buffer, sizeof(buffer) };

        Assert::IsTrue(
            reader.CheckDataLengthSafe(10, 90));

        Assert::IsTrue(
            reader.CheckDataLengthSafe(100, 0));

        Assert::IsTrue(
            reader.CheckDataLengthSafe(0, 100));
    }

    TEST_METHOD(CheckDataLengthSafe_PositionAndLength_Invalid)
    {
        BYTE buffer[100]{};

        CMemoryReader reader{ buffer, sizeof(buffer) };

        Assert::IsFalse(
            reader.CheckDataLengthSafe(101, 0));

        Assert::IsFalse(
            reader.CheckDataLengthSafe(100, 1));

        Assert::IsFalse(
            reader.CheckDataLengthSafe(50, 51));
    }

    TEST_METHOD(CheckDataLengthSafe_CurrentPosition_Valid)
    {
        BYTE buffer[100]{};

        CMemoryReader reader{ buffer, sizeof(buffer) };

        reader.Seek(40);

        Assert::IsTrue(
            reader.CheckDataLengthSafe(60));

        Assert::IsTrue(
            reader.CheckDataLengthSafe(0));
    }

    TEST_METHOD(CheckDataLengthSafe_CurrentPosition_Invalid)
    {
        BYTE buffer[100]{};

        CMemoryReader reader{ buffer, sizeof(buffer) };

        reader.Seek(40);

        Assert::IsFalse(
            reader.CheckDataLengthSafe(61));

        Assert::IsFalse(
            reader.CheckDataLengthSafe(1000));
    }

    TEST_METHOD(CheckDataLength_ValidDoesNotThrow)
    {
        BYTE buffer[100]{};

        CMemoryReader reader{ buffer, sizeof(buffer) };

        reader.Seek(20);

        reader.CheckDataLength(80);
        reader.CheckDataLength(20, 80);
        reader.CheckDataLength(0);
    }

    TEST_METHOD(CheckDataLength_InvalidThrows)
    {
        BYTE buffer[100]{};

        CMemoryReader reader{ buffer, sizeof(buffer) };

        const auto ex =
            ExpectException<CMemoryReader::XptDataLength>(
                [&]()
                {
                    reader.CheckDataLength(20, 81);
                });

        Assert::AreEqual(size_t{ 20 }, ex.pos);
        Assert::AreEqual(size_t{ 81 }, ex.cb);
    }

    TEST_METHOD(CheckDataLength_InvalidCurrentPositionThrows)
    {
        BYTE buffer[100]{};

        CMemoryReader reader{ buffer, sizeof(buffer) };

        reader.Seek(70);

        const auto ex =
            ExpectException<CMemoryReader::XptDataLength>(
                [&]()
                {
                    reader.CheckDataLength(31);
                });

        Assert::AreEqual(size_t{ 70 }, ex.pos);
        Assert::AreEqual(size_t{ 31 }, ex.cb);
    }

    // ========================================================================
    // Boundary conditions
    // ========================================================================

    TEST_METHOD(EmptyBuffer_AllowsZeroLengthOperations)
    {
        BYTE dummy = 0;

        CMemoryReader reader{ &dummy, 0 };
        CMemoryWalker writer{ &dummy, 0 };

        BYTE temp{};

        reader.Read(&temp, 0);
        writer.Write(&temp, 0);

        Assert::AreEqual(size_t{ 0 }, reader.GetPosition());
        Assert::AreEqual(size_t{ 0 }, writer.GetPosition());

        Assert::AreEqual(size_t{ 0 }, reader.GetRemainingSize());
        Assert::AreEqual(size_t{ 0 }, writer.GetRemainingSize());

        Assert::IsTrue(reader.IsEnd());
        Assert::IsTrue(writer.IsEnd());
    }

    TEST_METHOD(OneByteBuffer_ReadAndWrite)
    {
        BYTE buffer[1]{};
        const BYTE value = 0xAB;

        CMemoryWalker writer{ buffer, 1 };
        writer << value;

        Assert::AreEqual(BYTE{ 0xAB }, buffer[0]);
        Assert::AreEqual(size_t{ 1 }, writer.GetPosition());
        Assert::IsTrue(writer.IsEnd());

        BYTE output{};

        CMemoryReader reader{ buffer, 1 };
        reader >> output;

        Assert::AreEqual(value, output);
        Assert::AreEqual(size_t{ 1 }, reader.GetPosition());
        Assert::IsTrue(reader.IsEnd());
    }

    TEST_METHOD(ReadWriteWholeBuffer_ExactBoundary)
    {
        std::array<BYTE, 256> source{};
        std::array<BYTE, 256> destination{};

        for (size_t i = 0; i < source.size(); ++i)
            source[i] = static_cast<BYTE>(i);

        CMemoryWalker writer{
            destination.data(),
            destination.size()
        };

        writer.Write(
            source.data(),
            source.size());

        Assert::AreEqual(
            source.size(),
            writer.GetPosition());

        Assert::IsTrue(writer.IsEnd());

        CMemoryReader reader{
            destination.data(),
            destination.size()
        };

        std::array<BYTE, 256> result{};

        reader.Read(
            result.data(),
            result.size());

        Assert::IsTrue(
            source == result);

        Assert::IsTrue(reader.IsEnd());
    }

    // ========================================================================
    // Cursor stability after failed operations
    // ========================================================================

    TEST_METHOD(FailedRead_DoesNotAdvanceCursor)
    {
        BYTE buffer[8]{};
        BYTE output[16]{};

        CMemoryReader reader{ buffer, sizeof(buffer) };

        reader.Seek(5);

        try
        {
            reader.Read(output, 4);
            Assert::Fail(L"Expected exception.");
        }
        catch (const CMemoryReader::XptRange&)
        {
        }

        Assert::AreEqual(
            size_t{ 5 },
            reader.GetPosition());
    }

    TEST_METHOD(FailedWrite_DoesNotAdvanceCursor)
    {
        BYTE buffer[8]{};
        BYTE source[16]{};

        CMemoryWalker writer{ buffer, sizeof(buffer) };

        writer.Seek(5);

        try
        {
            writer.Write(source, 4);
            Assert::Fail(L"Expected exception.");
        }
        catch (const CMemoryWalker::XptRange&)
        {
        }

        Assert::AreEqual(
            size_t{ 5 },
            writer.GetPosition());
    }

    // ========================================================================
    // Chained operations
    // ========================================================================

    TEST_METHOD(ChainedWriteOperations)
    {
        BYTE buffer[32]{};

        uint16_t a = 0x1234;
        uint32_t b = 0x56789ABC;
        uint8_t c = 0xEF;

        CMemoryWalker writer{ buffer, sizeof(buffer) };

        writer
            << a
            << b
            << c;

        Assert::AreEqual(
            sizeof(a) + sizeof(b) + sizeof(c),
            writer.GetPosition());
    }

    TEST_METHOD(ChainedReadOperations)
    {
        BYTE buffer[32]{};

        uint16_t a = 0x1234;
        uint32_t b = 0x56789ABC;
        uint8_t c = 0xEF;

        std::memcpy(buffer + 0, &a, sizeof(a));
        std::memcpy(buffer + sizeof(a), &b, sizeof(b));
        std::memcpy(
            buffer + sizeof(a) + sizeof(b),
            &c,
            sizeof(c));

        CMemoryReader reader{ buffer, sizeof(buffer) };

        uint16_t a2{};
        uint32_t b2{};
        uint8_t c2{};

        reader
            >> a2
            >> b2
            >> c2;

        Assert::AreEqual(a, a2);
        Assert::AreEqual(b, b2);
        Assert::AreEqual(c, c2);

        Assert::AreEqual(
            sizeof(a) + sizeof(b) + sizeof(c),
            reader.GetPosition());
    }

    TEST_METHOD(SeekAndSeekToBeginAreEquivalent)
    {
        BYTE buffer[64]{};

        CMemoryReader reader{ buffer, sizeof(buffer) };

        reader.Seek(37);
        reader.SeekToBegin();

        Assert::AreEqual(size_t{ 0 }, reader.GetPosition());
        Assert::AreEqual(
            reinterpret_cast<const BYTE*>(buffer),
            reader.Data());
    }

    TEST_METHOD(SeekToEndAndEndPositionAreEquivalent)
    {
        BYTE buffer[64]{};

        CMemoryReader reader{ buffer, sizeof(buffer) };

        reader.SeekToEnd();

        Assert::AreEqual(
            sizeof(buffer),
            reader.GetPosition());

        Assert::AreEqual(
            size_t{ 0 },
            reader.GetRemainingSize());

        Assert::IsTrue(reader.IsEnd());
    }

    TEST_METHOD(AdvanceThenReverseReturnsToOriginalPosition)
    {
        BYTE buffer[64]{};

        CMemoryReader reader{ buffer, sizeof(buffer) };

        constexpr size_t pos = 23;
        constexpr size_t delta = 11;

        reader.Seek(pos);
        reader += delta;
        reader -= delta;

        Assert::AreEqual(
            pos,
            reader.GetPosition());
    }

    TEST_METHOD(ReadAndWriteRoundTripIsBitExact)
    {
        std::array<BYTE, 512> source{};
        std::array<BYTE, 512> encoded{};
        std::array<BYTE, 512> decoded{};

        for (size_t i = 0; i < source.size(); ++i)
            source[i] =
            static_cast<BYTE>((i * 37u + 13u) & 0xFFu);

        {
            CMemoryWalker writer{
                encoded.data(),
                encoded.size()
            };

            writer.Write(
                source.data(),
                source.size());

            Assert::AreEqual(
                source.size(),
                writer.GetPosition());
        }

        {
            CMemoryReader reader{
                encoded.data(),
                encoded.size()
            };

            reader.Read(
                decoded.data(),
                decoded.size());

            Assert::AreEqual(
                source.size(),
                reader.GetPosition());
        }

        Assert::IsTrue(source == decoded);
    }

    TEST_METHOD(CheckDataLengthSafe_MatchesActualRangeRule)
    {
        BYTE buffer[128]{};

        CMemoryReader reader{
            buffer,
            sizeof(buffer)
        };

        for (size_t pos = 0; pos <= sizeof(buffer); ++pos)
        {
            reader.Seek(pos);

            for (size_t cb = 0; cb <= sizeof(buffer) + 1; ++cb)
            {
                const bool expected =
                    cb <= sizeof(buffer) - pos;

                const bool actual =
                    reader.CheckDataLengthSafe(cb) != FALSE;

                Assert::AreEqual(
                    expected,
                    actual);
            }
        }
    }

    TEST_METHOD(CheckDataLengthSafe_AbsolutePositionRule)
    {
        BYTE buffer[128]{};

        CMemoryReader reader{
            buffer,
            sizeof(buffer)
        };

        for (size_t pos = 0; pos <= sizeof(buffer); ++pos)
        {
            for (size_t cb = 0; cb <= sizeof(buffer) + 1; ++cb)
            {
                const bool expected =
                    (pos <= sizeof(buffer)) &&
                    (cb <= sizeof(buffer) - pos);

                const bool actual =
                    reader.CheckDataLengthSafe(pos, cb) != FALSE;

                Assert::AreEqual(
                    expected,
                    actual);
            }
        }
    }
};
TS_NS_END