#include "CReferenceCounted.h"
#include "Color.h"

ECK_NAMESPACE_BEGIN
ECK_UIBASIC_NAMESPACE_BEGIN
namespace Declaration
{
    enum : UINT
    {
        IdTmInvalid,

        // Color

        IdCrFore,
        IdCrForeDisabled,

        IdCrBorder,
        IdCrBorderHot,
        IdCrBorderPressed,
        IdCrBorderDisabled,

        IdCrBack,
        IdCrBackHot,
        IdCrBackPressed,
        IdCrBackDisabled,

        IdCrAccent,
        IdCrAccentHot,
        IdCrAccentPressed,
        IdCrAccentDisabled,
        IdCrAccentFore,
        IdCrAccentForeDisabled,

        IdCrDanger,
        IdCrDangerHot,
        IdCrDangerPressed,

        // Metric

        IdMePaddingOuter,
        IdMePaddingInner,
        IdMeFocusPadding,
        IdMeScrollBar,
        IdMeScrollThumbPadding,
        IdMeMinimumScrollThumb,

        // Part

        IdPtNormal,

        IdTmSystemBegin,
    };

    struct ACCENT_COLOR
    {
        ARGB argbAccent;

        ARGB argbAccentHot;     // 输出
        ARGB argbAccentPressed; // 输出

        ARGB argbAccentDisabled;
        ARGB argbAccentFore;
        ARGB argbAccentForeDisabled;
    };

    struct CAC_PARAM : ACCENT_COLOR
    {
        UINT uFlags;// CACF_*
    };

    enum : UINT
    {
        CACF_DARK_MODE = 1u << 0,
        CACF_NO_HOT = 1u << 1,
        CACF_NO_PRESSED = 1u << 2,
    };

    // 计算强调颜色组
    // argbAccent用作输入
    // 忽略argbAccentDisabled、argbAccentFore、argbAccentForeDisabled
    inline constexpr void TmCalculateAccentColor(_Inout_ CAC_PARAM& ac) noexcept
    {
        const auto bDark = !!(ac.uFlags & CACF_DARK_MODE);

        ACCENT_COLOR_SET acs;
        CalculateAccentColorSet(ac.argbAccent, acs);
        if (bDark)
            ac.argbAccent = acs.argbLight2;

        if (!(ac.uFlags & CACF_NO_HOT))
            if (bDark)
                ac.argbAccentHot = LerpArgb(ac.argbAccent, 0xFF'000000, 0.1f);
            else
                ac.argbAccentHot = LerpArgb(ac.argbAccent, 0xFF'FFFFFF, 0.1f);
        if (!(ac.uFlags & CACF_NO_PRESSED))
            if (bDark)
                ac.argbAccentPressed = LerpArgb(ac.argbAccent, 0xFF'000000, 0.2f);
            else
                ac.argbAccentPressed = LerpArgb(ac.argbAccent, 0xFF'FFFFFF, 0.2f);
    }
}

class CColorCollection : public CReferenceCounted
{
protected:
    struct ITEM
    {
        UINT id;
        UINT argb;
    };
    std::vector<ITEM> m_vItem{};

    EckInlineNdCe auto LowerBound(UINT id) noexcept
    {
        return std::lower_bound(m_vItem.begin(), m_vItem.end(), id,
            [](const ITEM& e, UINT id) { return e.id < id; });
    }
    EckInlineNdCe auto LowerBound(UINT id) const noexcept
    {
        return std::lower_bound(m_vItem.begin(), m_vItem.end(), id,
            [](const ITEM& e, UINT id) { return e.id < id; });
    }
public:
    void Set(UINT id, UINT argb) noexcept
    {
        const auto it = LowerBound(id);
        if (it != m_vItem.end() && it->id == id)
            it->argb = argb;
        else
            m_vItem.emplace(it, id, argb);
    }
    std::optional<UINT> Get(UINT id) noexcept
    {
        const auto it = LowerBound(id);
        if (it != m_vItem.end() && it->id == id)
            return { it->argb };
        else
            return std::nullopt;
    }
};

template<class T>
class CMetricCollection : public CReferenceCounted
{
protected:
    struct ITEM
    {
        UINT id;
        T d;
    };
    std::vector<ITEM> m_vItem{};

    EckInlineNdCe auto LowerBound(UINT id) noexcept
    {
        return std::lower_bound(m_vItem.begin(), m_vItem.end(), id,
            [](const ITEM& e, UINT id) {return e.id < id; });
    }
    EckInlineNdCe auto LowerBound(UINT id) const noexcept
    {
        return std::lower_bound(m_vItem.begin(), m_vItem.end(), id,
            [](const ITEM& e, UINT id) {return e.id < id; });
    }
public:
    void Set(UINT id, T d) noexcept
    {
        const auto it = LowerBound(id);
        if (it != m_vItem.end() && it->id == id)
            it->d = d;
        else
            m_vItem.emplace(it, id, d);
    }
    std::optional<T> Get(UINT id) noexcept
    {
        const auto it = LowerBound(id);
        if (it != m_vItem.end() && it->id == id)
            return { it->d };
        else
            return std::nullopt;
    }
};
ECK_UIBASIC_NAMESPACE_END
ECK_NAMESPACE_END