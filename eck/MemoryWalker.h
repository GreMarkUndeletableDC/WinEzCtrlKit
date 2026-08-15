#pragma once
#include "CByteBuffer.h"
#include "CString.h"

ECK_NAMESPACE_BEGIN
namespace Detail
{
    struct XptMemoryWalker {};
    struct XptMemoryWalkerRange : XptMemoryWalker
    {
        PCBYTE pBase{};
        size_t cbMax{};
        PCBYTE pCurr{};
        constexpr XptMemoryWalkerRange(PCBYTE pBase_, size_t cbMax_, PCBYTE pCurr_) noexcept
            : pBase{ pBase_ }, cbMax{ cbMax_ }, pCurr{ pCurr_ }
        {
        }
    };
    struct XptMemoryWalkerStringTooLong : XptMemoryWalker
    {
        size_t cb{};
        constexpr XptMemoryWalkerStringTooLong(size_t cb_) noexcept : cb{ cb_ } {}
    };
    struct XptMemoryWalkerDataLength : XptMemoryWalker
    {
        size_t pos{};
        size_t cb{};
        constexpr XptMemoryWalkerDataLength(size_t pos_, size_t cb_) noexcept
            : pos{ pos_ }, cb{ cb_ } {
        }
    };

    struct CMemoryReaderBase
    {
    protected:
        PCBYTE m_pMem{};
        PCBYTE m_pBase{};
        size_t m_cbMax{};

        EckInline void CheckUpperBound(size_t cb) const
        {
            if (cb > m_cbMax - (m_pMem - m_pBase))
                throw XptMemoryWalkerRange{ m_pBase, m_cbMax, m_pMem + cb };
        }
    public:
        CMemoryReaderBase() = default;
        constexpr CMemoryReaderBase(_In_reads_bytes_(cbMax) PCVOID p, size_t cbMax) noexcept
            : m_pMem{ (PCBYTE)p }, m_pBase{ (PCBYTE)p }, m_cbMax{ cbMax }
        {
        }

        constexpr void SetData(PCVOID p, size_t cbMax) noexcept
        {
            m_pMem = (BYTE*)p;
            m_pBase = m_pMem;
            m_cbMax = cbMax;
        }
    };

    class CMemoryWalkerBase
    {
    protected:
        BYTE* m_pMem{};
        BYTE* m_pBase{};
        size_t m_cbMax{};

        EckInline void CheckUpperBound(size_t cb) const
        {
            if (cb > m_cbMax - (m_pMem - m_pBase))
                throw XptMemoryWalkerRange{ m_pBase, m_cbMax, m_pMem + cb };
        }
    public:
        CMemoryWalkerBase() = default;
        constexpr CMemoryWalkerBase(_Inout_updates_bytes_(cbMax) void* p, size_t cbMax) noexcept
            : m_pMem{ (BYTE*)p }, m_pBase{ (BYTE*)p }, m_cbMax{ cbMax }
        {
        }

        EckInline auto& Write(_In_reads_bytes_(cb) PCVOID pSrc, size_t cb)
        {
            CheckUpperBound(cb);
            memcpy(m_pMem, pSrc, cb);
            m_pMem += cb;
            return *this;
        }
        EckInline auto& WriteReversed(_In_reads_bytes_(cb) PCVOID pSrc, size_t cb)
        {
            CheckUpperBound(cb);
            const auto p = (PCBYTE)pSrc;
            for (size_t i = 0; i < cb; ++i)
                *m_pMem++ = p[cb - 1 - i];
            return *this;
        }

        template<CcpTrivial T>
        EckInline auto& operator<<(const T& Data) { return Write(&Data, sizeof(T)); }

        template<CcpTrivial T>
        EckInline auto& WriteReversed(const T& Data) { return WriteReversed(&Data, sizeof(T)); }

        template<class T, class U>
        EckInline auto& operator<<(const std::basic_string<T, U>& Data)
        {
            return Write(Data.data(), (Data.size() + 1) * sizeof(T));
        }
        template<class T, class U>
        EckInline auto& operator<<(std::basic_string_view<T, U> Data)
        {
            return Write(Data.data(), Data.size() * sizeof(T)) << T{};
        }
        template<class T, class U>
        EckInline auto& operator<<(const std::vector<T, U>& Data)
        {
            return Write(Data.data(), Data.size() * sizeof(T));
        }
        template<class T, size_t N>
        EckInline auto& operator<<(std::span<T, N> Data)
        {
            return Write(Data.data(), Data.size() * sizeof(T));
        }
        template<class T>
        EckInline auto& operator<<(const CByteBufferT<T>& Data)
        {
            return Write(Data.Data(), Data.Size());
        }
        template<class T, class U, class V>
        EckInline auto& operator<<(const CStringT<T, U, V>& Data)
        {
            return Write(Data.Data(), Data.ByteSize());
        }

        constexpr void SetData(void* p, size_t cbMax) noexcept
        {
            m_pMem = (BYTE*)p;
            m_pBase = m_pMem;
            m_cbMax = cbMax;
        }
    };

    template<class T>
    class CMemoryWalkerWrapper : public T
    {
    private:
        EckInline void CheckLowerBound(size_t cb) const
        {
            if (cb > this->m_pMem - this->m_pBase)
                throw XptMemoryWalkerRange{ this->m_pBase, this->m_cbMax, this->m_pMem - cb };
        }
        EckInline int CheckLengthInt(size_t cch) const
        {
            if (cch > (size_t)std::numeric_limits<int>::max())
                throw XptStringTooLong{ cch };
            return (int)cch;
        }
    public:
        using T::T;

        using Xpt = XptMemoryWalker;
        using XptRange = XptMemoryWalkerRange;
        using XptStringTooLong = XptMemoryWalkerStringTooLong;
        using XptDataLength = XptMemoryWalkerDataLength;

        EckInline auto& Read(_Out_writes_bytes_all_(cb) void* pDst, size_t cb)
        {
            this->CheckUpperBound(cb);
            memcpy(pDst, this->m_pMem, cb);
            this->m_pMem += cb;
            return *this;
        }

        EckInline auto& ReadReversed(_Out_writes_bytes_all_(cb) void* pDst, size_t cb)
        {
            this->CheckUpperBound(cb);
            const auto p = (BYTE*)pDst;
            for (size_t i = 0; i < cb; ++i)
                p[cb - 1 - i] = *this->m_pMem++;
            return *this;
        }

        template<CcpTrivial T>
        EckInline auto& operator>>(_Out_ T& Data) { return Read(&Data, sizeof(Data)); }

        template<CcpTrivial T>
        EckInline auto& ReadReversed(_Out_ T& Data) { return ReadReversed(&Data, sizeof(T)); }

        template<class T, class U, class V>
        EckInline auto& operator>>(CStringT<T, U, V>& x)
        {
            const int cch = CountStringLength<T>();
            x.ReSize(cch);
            return Read(x.Data(), (cch + 1) * sizeof(T));
        }

        template<class T>
        int CountStringLength() const
        {
            const T UNALIGNED* p = (const T*)Data();
            const T UNALIGNED* const pEnd = p + GetRemainingSize() / sizeof(T);
            BOOL bFoundNull{};
            for (; p < pEnd; ++p)
                if (*p == T{})
                {
                    bFoundNull = TRUE;
                    break;
                }
            if (!bFoundNull)
                throw XptMemoryWalkerRange{ this->m_pBase, this->m_cbMax, (PCBYTE)p };
            return CheckLengthInt(p - (const T*)Data());
        }

        template<class T>
        int CountStringLengthSafe() const
        {
            const T UNALIGNED* = (const T*)Data();
            const T UNALIGNED* pEnd = p + GetRemainingSize() / sizeof(T);
            for (; p < pEnd; ++p)
                if (*p == T{})
                    break;
            return CheckLengthInt(p - (const T*)Data());
        }

        template<class T>
        auto& SkipPointer(_Out_ T*& p)
        {
            this->CheckUpperBound(sizeof(T));
            p = (T*)this->m_pMem;
            this->m_pMem += sizeof(T);
            return *this;
        }

        EckInlineNdCe auto Data() const noexcept { return this->m_pMem; }

        EckInline auto& operator+=(size_t cb)
        {
            this->CheckUpperBound(cb);
            this->m_pMem += cb;
            return *this;
        }
        EckInline auto& operator-=(size_t cb)
        {
            this->CheckLowerBound(cb);
            this->m_pMem -= cb;
            return *this;
        }

        EckInlineCe auto& SeekToBegin() noexcept
        {
            this->m_pMem = this->m_pBase;
            return *this;
        }
        EckInlineCe auto& SeekToEnd() noexcept
        {
            this->m_pMem = this->m_pBase + this->m_cbMax;
            return *this;
        }
        EckInline auto& Seek(size_t pos)
        {
            this->m_pMem = this->m_pBase;
            this->CheckUpperBound(pos);
            this->m_pMem = this->m_pBase + pos;
            return *this;
        }
        EckInlineCe size_t GetRemainingSize() const noexcept { return this->m_pBase + this->m_cbMax - this->m_pMem; }
        EckInlineCe BOOL IsEnd() const noexcept { return this->m_pMem >= this->m_pBase + this->m_cbMax; }
        EckInlineCe size_t GetPosition() const noexcept { return this->m_pMem - this->m_pBase; }

        EckInlineNdCe BOOL CheckDataLengthSafe(size_t pos, size_t cb) const noexcept
        {
            return (pos < this->m_cbMax) && (cb <= this->m_cbMax - pos);
        }
        EckInline void CheckDataLength(size_t pos, size_t cb) const
        {
            if (!CheckDataLengthSafe(pos, cb))
                throw XptDataLength{ pos, cb };
        }

        EckInlineNdCe BOOL CheckDataLengthSafe(size_t cb) const noexcept
        {
            return cb <= this->m_cbMax - GetPosition();
        }
        EckInline void CheckDataLength(size_t cb) const
        {
            if (!CheckDataLengthSafe(cb))
                throw XptDataLength{ GetPosition(), cb };
        }
    };
}

using CMemoryReader = Detail::CMemoryWalkerWrapper<Detail::CMemoryReaderBase>;
using CMemoryWalker = Detail::CMemoryWalkerWrapper<Detail::CMemoryWalkerBase>;
ECK_NAMESPACE_END