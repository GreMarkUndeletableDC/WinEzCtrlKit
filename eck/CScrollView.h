#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
template<class T>
    requires (std::is_signed_v<T> || std::is_floating_point_v<T>)
class CScrollViewT
{
public:
    constexpr static T InvalidValue = std::numeric_limits<T>::max();
protected:
    T m_Min{};
    T m_Max{};
    T m_Page{};
    T m_Pos{};

    T m_ViewSize{};
    T m_MinThumbSize{};

    T m_oxyThumbCursor{};
#ifdef _DEBUG
    bool m_bLBtnDown = false;
#endif // _DEBUG

    // 若数据超界返回TRUE
    constexpr bool RangeChanged() noexcept
    {
        if (!IsValid())
            SetPositionUncheck(GetMinimum());
        else if (GetPosition() < GetMinimum())
            SetPositionUncheck(GetMinimum());
        else if (GetPosition() > GetMaxWithPage())
            SetPositionUncheck(GetMaxWithPage());
        else
            return false;
        return true;
    }
public:
    EckInlineCe T GetMinimum() const noexcept { return m_Min; }
    EckInlineCe void SetMinimum(T iMin) noexcept
    {
        m_Min = iMin;
        RangeChanged();
    }

    EckInlineCe T GetMaximum() const noexcept { return m_Max; }
    EckInlineCe T GetMaxWithPage() const noexcept { return m_Max - m_Page + 1; }
    EckInlineCe void SetMaximum(T Max) noexcept
    {
        m_Max = Max;
        RangeChanged();
    }

    EckInlineCe T GetPage() const noexcept { return m_Page; }
    EckInlineCe void SetPage(T Page) noexcept
    {
        m_Page = Page;
        RangeChanged();
    }

    EckInlineCe T GetPosition() const noexcept { return m_Pos; }
    // 若数据超界返回TRUE
    EckInlineCe bool SetPosition(T Pos) noexcept
    {
        m_Pos = Pos;
        return RangeChanged();
    }
    EckInlineCe void SetPositionUncheck(T iPos) noexcept { m_Pos = iPos; }

    EckInlineCe void SetRange(T Min, T Max) noexcept
    {
        m_Min = Min;
        m_Max = Max;
        RangeChanged();
    }
    EckInlineCe T GetRangeDistance() const noexcept
    {
        return GetMaxWithPage() - GetMinimum();
    }

    EckInlineCe bool IsValid() const noexcept { return GetRangeDistance() > 0; }

    constexpr void OnMouseWheel(T Delta) noexcept
    {
        if (!IsValid())
            return;
        T iPos = GetPosition() + Delta;
        if (iPos < GetMinimum())
            iPos = GetMinimum();
        else if (iPos > GetMaxWithPage())
            iPos = GetMaxWithPage();

        SetPosition(iPos);
    }

    constexpr float GetPrecent() const noexcept
    {
        const T d = GetMaxWithPage() - GetMinimum();
        if (d <= 0)
            return 0.f;
        else
            return float(GetPosition() - GetMinimum()) / (float)d;
    }

    EckInlineCe void SetViewSize(T ViewSize) noexcept { m_ViewSize = ViewSize; }
    EckInlineCe T GetViewSize() const noexcept { return m_ViewSize; }

    EckInlineCe void SetMinimumThumbSize(T MinThumbSize) noexcept { m_MinThumbSize = MinThumbSize; }
    EckInlineCe T GetMinimumThumbSize() const noexcept { return m_MinThumbSize; }

    EckInlineCe T GetThumbSize() const noexcept
    {
        const T d = GetMaximum() - GetMinimum();
        if (d <= 0)
            return InvalidValue;
        const T i = GetPage() * GetViewSize() / d;
        return std::max(GetMinimumThumbSize(), i);
    }

    EckInlineCe T GetThumbPosition(T ThumbSize) const noexcept
    {
        const T d = GetRangeDistance();
        if (d <= 0)
            return InvalidValue;
        return (GetPosition() - GetMinimum()) * (GetViewSize() - ThumbSize) / d;
    }
    EckInlineCe T GetThumbPosition() const noexcept { return GetThumbPosition(GetThumbSize()); }

    EckInlineCe void OnLButtonDown(T xy) noexcept
    {
#ifdef _DEBUG
        EckAssert(!m_bLBtnDown);
        m_bLBtnDown = true;
#endif // _DEBUG
        if (!IsValid())
            return;
        EckAssert(xy >= GetThumbPosition() && xy <= GetThumbPosition() + GetThumbSize());
        m_oxyThumbCursor = xy - GetThumbPosition();
    }

    EckInlineCe bool OnMouseMove(T xy) noexcept
    {
        EckAssert(m_bLBtnDown);
        if (!IsValid())
            return false;
        const auto pos = GetMinimum() +
            (xy - m_oxyThumbCursor) *
            GetRangeDistance() /
            (GetViewSize() - GetThumbSize());
        if (!CanMove(pos - GetPosition()))
            return false;
        SetPosition(pos);
        return true;
    }

    EckInlineCe void OnLButtonUp() noexcept
    {
#ifdef _DEBUG
        EckAssert(m_bLBtnDown);
        m_bLBtnDown = false;
#endif // _DEBUG
    }

    constexpr void SetScrollInformation(const SCROLLINFO& si) noexcept
    {
        if (si.fMask & SIF_RANGE)
        {
            m_Min = (T)si.nMin;
            m_Max = (T)si.nMax;
        }
        if (si.fMask & SIF_PAGE)
            m_Page = (T)si.nPage;
        if (si.fMask & SIF_POS)
            m_Pos = (T)si.nPos;
        RangeChanged();
    }

    constexpr void GetScrollInformation(_Inout_ SCROLLINFO& si) const noexcept
    {
        if (si.fMask & SIF_RANGE)
        {
            si.nMin = (int)m_Min;
            si.nMax = (int)m_Max;
        }
        if (si.fMask & SIF_PAGE)
            si.nPage = (UINT)m_Page;
        if (si.fMask & SIF_POS)
            si.nPos = (int)m_Pos;
    }

    // 是否有必要显示滚动条
    EckInlineCe bool IsVisible() const noexcept
    {
        return IsValid() && GetViewSize() > GetMinimumThumbSize();
    }

    EckInlineNdCe bool IsMinimum() const noexcept { return GetPosition() <= GetMinimum(); }
    EckInlineNdCe bool IsMaximum() const noexcept { return GetPosition() >= GetMaxWithPage(); }
    EckInlineNdCe bool CanMove(T d) const noexcept
    {
        if (d == 0)
            return false;
        return d > 0 ? !IsMaximum() : !IsMinimum();
    }
};

using CScrollView = CScrollViewT<int>;
using CScrollViewF = CScrollViewT<float>;
ECK_NAMESPACE_END