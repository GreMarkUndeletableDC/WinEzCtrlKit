#pragma once
#include "CString.h"
#include "StringUtility.h"

ECK_NAMESPACE_BEGIN
using TChar = char;
class CStringReader
{
public:
    using TStringView = std::basic_string_view<TChar>;

    struct Xpt {};
    struct XptTcvResult : Xpt
    {
        TcvResult e;
        constexpr XptTcvResult(TcvResult e_) noexcept : e{ e_ } {}
    };
    struct XptRange : Xpt
    {
        const TChar* pBase{};
        size_t cchMax{};
        const TChar* pCurr{};
        constexpr XptRange(const TChar* pBase_, size_t cchMax_, const TChar* pCurr_) noexcept
            : pBase{ pBase_ }, cchMax{ cchMax_ }, pCurr{ pCurr_ }
        {
        }
    };
    struct XptIntOverflow : Xpt
    {
        size_t cch{};
        constexpr XptIntOverflow(size_t cch_) noexcept : cch{ cch_ } {}
    };
    struct XptBufferTooSmall : Xpt
    {
        size_t cch{};
        constexpr XptBufferTooSmall(size_t cch_) noexcept : cch{ cch_ } {}
    };
    struct XptNotNumber : Xpt {};
    struct XptReachEnd : Xpt {};
    struct XptEnumerationOutOfRange : Xpt
    {
        LONGLONG nValue{};
        LONGLONG nMin{};
        LONGLONG nMax{};
        constexpr XptEnumerationOutOfRange(
            LONGLONG nValue_,
            LONGLONG nMin_,
            LONGLONG nMax_) noexcept :
            nValue{ nValue_ }, nMin{ nMin_ }, nMax{ nMax_ }
        {
        }
    };
private:
    const TChar* m_pCurr{};
    const TChar* m_pBase{};
    size_t m_cchMax{};

    EckInlineNdCe const TChar* End() const noexcept { return m_pBase + m_cchMax; }

    static int CheckLengthInt(size_t cch)
    {
        if (cch > (size_t)std::numeric_limits<int>::max())
            throw XptIntOverflow{ cch };
        return (int)cch;
    }

    const TChar* FindBoundary(TChar chEnd, bool bAllowReachEnd) const
    {
        if (const auto p = TcsCharLength(m_pCurr, GetRemainingSize(), chEnd))
            return p;
        if (bAllowReachEnd)
            return End();
        throw XptReachEnd{};
    }
    const TChar* FindBoundary(TStringView svEnd, bool bAllowReachEnd) const
    {
        if (const auto p = TcsStringLength(
            m_pCurr, GetRemainingSize(), svEnd.data(), svEnd.size()))
            return p;
        if (bAllowReachEnd)
            return End();
        throw XptReachEnd{};
    }

    const TChar* RFindBoundary(TChar ch, bool bAllowReachEnd) const
    {
        // 只在已读内容中查找
        if (const auto p = TcsRCharLength(m_pBase, size_t(m_pCurr - m_pBase), ch))
            return p;
        if (bAllowReachEnd)
            return m_pBase;
        throw XptReachEnd{};
    }
    const TChar* RFindBoundary(TStringView svEnd, bool bAllowReachEnd) const
    {
        if (const auto p = TcsRStringLength(
            m_pBase, size_t(m_pCurr - m_pBase), svEnd.data(), svEnd.size()))
            return p;
        if (bAllowReachEnd)
            return m_pBase;
        throw XptReachEnd{};
    }

    const TChar* FindBoundaryI(TChar chEnd, bool bAllowReachEnd) const
    {
        if (const auto p = TcsCharLengthI(m_pCurr, GetRemainingSize(), chEnd))
            return p;
        if (bAllowReachEnd)
            return End();
        throw XptReachEnd{};
    }
    const TChar* FindBoundaryI(TStringView svEnd, bool bAllowReachEnd) const
    {
        if (const auto p = TcsStringLengthI(
            m_pCurr, GetRemainingSize(), svEnd.data(), svEnd.size()))
            return p;
        if (bAllowReachEnd)
            return End();
        throw XptReachEnd{};
    }

    const TChar* RFindBoundaryI(TChar ch, bool bAllowReachEnd) const
    {
        // 只在已读内容中查找
        if (const auto p = TcsRCharLengthI(m_pBase, size_t(m_pCurr - m_pBase), ch))
            return p;
        if (bAllowReachEnd)
            return m_pBase;
        throw XptReachEnd{};
    }
    const TChar* RFindBoundaryI(TStringView svEnd, bool bAllowReachEnd) const
    {
        if (const auto p = TcsRStringLengthI(
            m_pBase, size_t(m_pCurr - m_pBase), svEnd.data(), svEnd.size()))
            return p;
        if (bAllowReachEnd)
            return m_pBase;
        throw XptReachEnd{};
    }
public:
    CStringReader() noexcept = default;
    CStringReader(TStringView sv) noexcept :
        m_pCurr{ sv.data() }, m_pBase{ sv.data() }, m_cchMax{ sv.size() }
    {
    }

    template<std::integral T>
    CStringReader& ReadInt(_Out_ T& i, int iRadix = 10)
    {
        T t{};
        const auto r = TcvToInt(m_pCurr, GetRemainingSize(), t, iRadix);
        if (r.eResult != TcvResult::Ok)
            throw XptTcvResult{ r.eResult };
        if (r.pEnd == m_pCurr)// 没有消耗任何字符
            throw XptNotNumber{};
        i = t;
        m_pCurr = r.pEnd;
        return *this;
    }

    template<std::floating_point T>
    CStringReader& ReadFloat(
        _Out_ T& f,
        TcvFloatFormat eFmt = TcvFloatFormat::General,
        int iRadix = 10)
    {
        T t{};
        const auto r = TcvToFloat(m_pCurr, GetRemainingSize(), t, eFmt, iRadix);
        if (r.eResult != TcvResult::Ok)
            throw XptTcvResult{ r.eResult };
        f = t;
        m_pCurr = r.pEnd;
        return *this;
    }

    template<CcpEnumeration T>
    CStringReader& ReadEnumeration(_Out_ T& e, int iRadix = 10)
    {
        std::underlying_type_t<T> t{};
        ReadInt(t, iRadix);
        e = (T)t;
        return *this;
    }

    // 校验值落在开区间(eMin, eMax)内
    template<CcpEnumeration T>
    CStringReader& ReadEnumeration(_Out_ T& e, T eMin, T eMax, int iRadix = 10)
    {
        using TUnderlying = std::underlying_type_t<T>;
        TUnderlying t{};
        const auto r = TcvToInt(m_pCurr, GetRemainingSize(), t, iRadix);
        if (r.eResult != TcvResult::Ok)
            throw XptTcvResult{ r.eResult };
        if (r.pEnd == m_pCurr)
            throw XptNotNumber{};
        const auto nMin = (TUnderlying)eMin;
        const auto nMax = (TUnderlying)eMax;
        if (!(t > nMin && t < nMax))
            throw XptEnumerationOutOfRange{ (LONGLONG)t, (LONGLONG)nMin, (LONGLONG)nMax };
        e = (T)t;
        m_pCurr = r.pEnd;
        return *this;
    }

    CStringReader& Read(_Out_writes_all_(cch) TChar* pDst, size_t cch)
    {
        if (cch > GetRemainingSize())
            throw XptRange{ m_pBase, m_cchMax, m_pCurr + cch };
        memcpy(pDst, m_pCurr, cch * sizeof(TChar));
        m_pCurr += cch;
        return *this;
    }

    template<class U, class V>
    CStringReader& Read(CStringT<TChar, U, V>& x, size_t cch)
    {
        if (cch > GetRemainingSize())
            throw XptRange{ m_pBase, m_cchMax, m_pCurr + cch };
        const auto pDst = x.PushBack(CheckLengthInt(cch));
        memcpy(pDst, m_pCurr, cch * sizeof(TChar));
        m_pCurr += cch;
        return *this;
    }

    CStringReader& ReadUntil(
        TChar chEnd,
        _Out_writes_all_(cch) TChar* pDst,
        _Inout_ size_t& cch,
        bool bAllowReachEnd = false)
    {
        const auto p = FindBoundary(chEnd, bAllowReachEnd);
        const auto cchRead = size_t(p - m_pCurr);
        if (cchRead > cch)
            throw XptBufferTooSmall{ cchRead };
        memcpy(pDst, m_pCurr, cchRead * sizeof(TChar));
        m_pCurr = p;
        cch = cchRead;
        return *this;
    }

    CStringReader& ReadUntil(
        TStringView svEnd,
        _Out_writes_all_(cch) TChar* pDst,
        _Inout_ size_t& cch,
        bool bAllowReachEnd = false)
    {
        const auto p = FindBoundary(svEnd, bAllowReachEnd);
        const auto cchRead = size_t(p - m_pCurr);
        if (cchRead > cch)
            throw XptBufferTooSmall{ cchRead };
        memcpy(pDst, m_pCurr, cchRead * sizeof(TChar));
        m_pCurr = p;
        cch = cchRead;
        return *this;
    }

    template<class U, class V>
    CStringReader& ReadUntil(
        TChar chEnd,
        CStringT<TChar, U, V>& x,
        bool bAllowReachEnd = false)
    {
        const auto p = FindBoundary(chEnd, bAllowReachEnd);
        const auto cchRead = size_t(p - m_pCurr);
        const auto pDst = x.PushBack(CheckLengthInt(cchRead));
        memcpy(pDst, m_pCurr, cchRead * sizeof(TChar));
        m_pCurr = p;
        return *this;
    }

    template<class U, class V>
    CStringReader& ReadUntil(
        TStringView svEnd,
        CStringT<TChar, U, V>& x,
        bool bAllowReachEnd = false)
    {
        const auto p = FindBoundary(svEnd, bAllowReachEnd);
        const auto cchRead = size_t(p - m_pCurr);
        const auto pDst = x.PushBack(CheckLengthInt(cchRead));
        memcpy(pDst, m_pCurr, cchRead * sizeof(TChar));
        m_pCurr = p;
        return *this;
    }

    CStringReader& ReadUntil(
        TChar chEnd,
        Eck_Out_buffer_ TStringView& sv,
        bool bAllowReachEnd = false)
    {
        const auto p = FindBoundary(chEnd, bAllowReachEnd);
        sv = { m_pCurr, size_t(p - m_pCurr) };
        m_pCurr = p;
        return *this;
    }

    CStringReader& ReadUntil(
        TStringView svEnd,
        Eck_Out_buffer_ TStringView& sv,
        bool bAllowReachEnd = false)
    {
        const auto p = FindBoundary(svEnd, bAllowReachEnd);
        sv = { m_pCurr, size_t(p - m_pCurr) };
        m_pCurr = p;
        return *this;
    }

    CStringReader& StepTo(TChar ch, bool bAllowReachEnd = false)
    {
        m_pCurr = FindBoundary(ch, bAllowReachEnd);
        return *this;
    }
    CStringReader& StepTo(TStringView svEnd, bool bAllowReachEnd = false)
    {
        m_pCurr = FindBoundary(svEnd, bAllowReachEnd);
        return *this;
    }

    CStringReader& StepBackTo(TChar ch, bool bAllowReachEnd = false)
    {
        m_pCurr = RFindBoundary(ch, bAllowReachEnd);
        return *this;
    }

    CStringReader& StepBackTo(TStringView svEnd, bool bAllowReachEnd = false)
    {
        m_pCurr = RFindBoundary(svEnd, bAllowReachEnd);
        return *this;
    }

    // 以下为忽略大小写的版本
    CStringReader& StepToI(TChar ch, bool bAllowReachEnd = false)
    {
        m_pCurr = FindBoundaryI(ch, bAllowReachEnd);
        return *this;
    }
    CStringReader& StepToI(TStringView svEnd, bool bAllowReachEnd = false)
    {
        m_pCurr = FindBoundaryI(svEnd, bAllowReachEnd);
        return *this;
    }

    CStringReader& StepBackToI(TChar ch, bool bAllowReachEnd = false)
    {
        m_pCurr = RFindBoundaryI(ch, bAllowReachEnd);
        return *this;
    }
    CStringReader& StepBackToI(TStringView svEnd, bool bAllowReachEnd = false)
    {
        m_pCurr = RFindBoundaryI(svEnd, bAllowReachEnd);
        return *this;
    }

    // 获取剩余内容的视图
    EckInlineNdCe TStringView GetStringView() const noexcept
    {
        return { m_pCurr, GetRemainingSize() };
    }

    // 获取底层全部内容的视图
    EckInlineNdCe TStringView ToStringView() const noexcept { return { m_pBase, m_cchMax }; }

    // 是否已到达结尾（无剩余字符可读）
    EckInlineNdCe bool IsEnd() const noexcept { return m_pCurr == End(); }

    // 底层字符串是否为空（与当前读取位置无关）
    EckInlineNdCe bool IsEmpty() const noexcept { return m_cchMax == 0; }

    EckInlineCe size_t GetRemainingSize() const noexcept
    {
        return size_t(End() - m_pCurr);
    }
};
ECK_NAMESPACE_END