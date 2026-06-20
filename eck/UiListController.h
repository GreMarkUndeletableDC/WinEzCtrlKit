#pragma once
#include "KwDef.h"
#include "CString.h"
#include "CCowByteBuffer.h"


#define ECK_LC_NAMESPACE_BEGIN namespace Lc {
#define ECK_LC_NAMESPACE_END   }

ECK_NAMESPACE_BEGIN
ECK_UIBASIC_NAMESPACE_BEGIN
ECK_LC_NAMESPACE_BEGIN
enum class Result
{
    Ok,
    NotImplemented,
};

enum class Property
{
    Invalid,

    // 若输入CStringW*、CStringW、std::span<WCHAR>，
    // 则适配器可以选择（并不强制）复制文本到上述类型
    // 否则，可返回上述类型或CCowByteBuffer
    Text,
    // 返回图像列表索引，int
    Image,

    SystemState,    // TState

    UiTextLayout,   // ComPtr<IDWriteTextLayout>
    UiPosition,     // TCoord
    UiHeight,       // TCoord

    // --

    UserBegin,
};

using TState = BYTE;

enum : TState
{
    LIF_SELECTED = 1u << 0,
    LIF_CHECKED = 1u << 1,
    LIF_HOT = 1u << 2,
};

struct Index
{
    int Item{ -1 };
    int Group{ -1 };

    EckInlineNdCe BOOL IsValid() const noexcept { return Item >= 0 || Group >= 0; }
};
struct IndexRange
{
    Index Begin{};
    Index End{};
};

constexpr inline Index InvalidIndex{ -1, -1 };

template<CcpNumber TCoord>
using TRect = std::conditional_t<std::floating_point<TCoord>, Kw::Rect, RECT>;


inline std::wstring_view AptWrapAnyText(const std::any& Data) noexcept
{
    if (Data.type() == typeid(std::wstring_view))
        return std::any_cast<std::wstring_view>(Data);
    if (Data.type() == typeid(CStringW))
        return std::any_cast<const CStringW&>(Data).ToStringView();
    if (Data.type() == typeid(CStringW*))
        return std::any_cast<CStringW*>(Data)->ToStringView();
    if (Data.type() == typeid(CCowByteBuffer))
    {
        const auto& e = std::any_cast<const CCowByteBuffer&>(Data);
        return { (PCWCH)e.Data(), e.Size() / sizeof(WCHAR) };
    }
    if (Data.type() == typeid(std::span<const WCHAR>))
    {
        const auto e = std::any_cast<std::span<const WCHAR>>(Data);
        return { (PCWCH)e.data(), e.size() / sizeof(WCHAR) };
    }
    if (Data.type() == typeid(std::span<WCHAR>))
    {
        const auto e = std::any_cast<std::span<WCHAR>>(Data);
        return { (PCWCH)e.data(), e.size() / sizeof(WCHAR) };
    }
    return {};
}

template<CcpNumber TCoord>
struct IHostT
{
    virtual TCoord LchSccGetPosition(BOOL bVert) const noexcept = 0;
    virtual void LchSccSetRange(BOOL bVert, TCoord Min, TCoord Max) noexcept = 0;
    virtual TCoord LchGetHeight() const noexcept = 0;
    virtual TCoord LchGetWidth() const noexcept = 0;
    virtual void LchInvalidateRect(const TRect<TCoord>* prc) noexcept = 0;
};

template<CcpNumber TCoord>
struct IAdapterT
{
    virtual Index LcaGetCount() const noexcept = 0;
    virtual int LcaGetCount(int idxGroup) const noexcept { return 0; }
    virtual int LcaGetColumnCount() const noexcept { return 0; }

    // -- 以下函数仅在分组模式下使用

    // 给定y坐标，返回所在组的索引
    // idx.Group输入起始组索引，若为-1则从头开始搜索，函数返回后设为命中结果
    // idx.Item保留供将来使用
    // 对于超界y坐标，必须夹紧到0或Max-1
    // 适配器可选实现此功能以减少计算过程中的虚函数调用
    virtual void LcaGroupFromY(
        _Inout_ Index& idx,
        TCoord yAbs,
        _Out_opt_ TCoord* pyAbs) noexcept
    {
        int l{}, r{ LcaGetCount().Group };
        if (!r)
        {
            idx.Group = -1;
            if (pyAbs)
                *pyAbs = 0;
            return;
        }

        if (idx.Group >= 0)
        {
            if (AptListGetGroupTop(idx.Group) <= yAbs)
                l = idx.Group;
            else
                r = idx.Group;
        }

        while (l < r)
        {
            const int m = l + (r - l) / 2;
            const auto yGroup = AptListGetGroupTop(m);
            if (yGroup <= yAbs)
                l = m + 1;
            else
                r = m;
        }

        idx.Group = (l > 0) ? (l - 1) : 0;
        if (pyAbs)
            *pyAbs = AptListGetGroupTop(idx.Group);
    }

    // -- 以下函数仅在项目高度可变时使用

    // 给定y坐标，返回项目索引，仅当项目高度可变时使用
    // idx.Group指定组索引
    // idx.Item输入起始项目索引，若为-1则从头开始搜索，函数返回后设为命中结果
    // 适配器可选实现此功能以减少计算过程中的虚函数调用
    virtual void LcaVariableHeightItemFromY(
        _Inout_ Index& idx,
        TCoord yAbs,
        _Out_opt_ TCoord* pyAbs) noexcept
    {
        idx = {};
        if (pyAbs)
            *pyAbs = 0;

    }

    // --

    virtual void LcaGet(const Index& idx, int idxCol, Property eProp, std::any& Data) const noexcept = 0;
    virtual void LcaSet(const Index& idx, int idxCol, Property eProp, std::any& Data, BOOL bMove = FALSE) noexcept = 0;

    EckInlineNd TCoord AptListGetGroupTop(int idxGroup) noexcept
    {
        std::any Data{};
        LcaGet({ .Group = idxGroup }, 0, Property::UiPosition, Data);
        return std::any_cast<TCoord>(Data);
    }
    EckInlineNd TCoord AptListGetGroupHeight(int idxGroup) noexcept
    {
        std::any Data{};
        LcaGet({ .Group = idxGroup }, 0, Property::UiHeight, Data);
        return std::any_cast<TCoord>(Data);
    }

    EckInlineNd std::pair<TCoord, TCoord> AptListGetGroupTopHeight(int idxGroup) noexcept
    {
        std::any Data{};
        LcaGet({ .Group = idxGroup }, 0, Property::UiPosition, Data);
        const auto y = std::any_cast<TCoord>(Data);
        LcaGet({ .Group = idxGroup }, 0, Property::UiHeight, Data);
        return { y, std::any_cast<TCoord>(Data) };
    }
    EckInlineNd void AptListSetGroupTopHeight(int idxGroup, TCoord y, TCoord cy) noexcept
    {
        std::any Data{ y };
        LcaSet({ .Group = idxGroup }, 0, Property::UiPosition, Data);
        Data = cy;
        LcaSet({ .Group = idxGroup }, 0, Property::UiHeight, Data);
    }


    EckInlineNd TCoord AptListGetItemTop(const Index& idx) noexcept
    {
        std::any Data{};
        LcaGet(idx, 0, Property::UiPosition, Data);
        return std::any_cast<TCoord>(Data);
    }
    EckInlineNd TCoord AptListGetItemHeight(const Index& idx) noexcept
    {
        std::any Data{};
        LcaGet(idx, 0, Property::UiHeight, Data);
        return std::any_cast<TCoord>(Data);
    }

    EckInlineNd std::wstring_view AptGetText(const Index& idx, int idxCol, std::any& Data) noexcept
    {
        LcaGet(idx, idxCol, Property::Text, Data);
        return AptWrapAnyText(Data);
    }
};


class CControllerT
{
public:
    using TCoord = float;
    using TRect = Lc::TRect<TCoord>;

    enum class View : BYTE
    {
        List,
        Icon,
    };

    enum class Selection : BYTE
    {
        Single,
        Multiple,
    };

    enum class Part : BYTE
    {
        Group,
        GroupHeader,
        GroupImage,
        GroupExpandButton,
        Item,
    };

    struct FOR_ITEM
    {
        Index idx;
    };
    struct FOR_GROUP
    {
        int idxGroup;
    };

    struct HT_INFO
    {
        TCoord x{};
        TCoord y{};
        Index idxAfterSpace{};
    };

    constexpr static TCoord DefaultItemHeight{ 40 };
    constexpr static TCoord DefaultPadding{ 4 };
    constexpr static TCoord DefaultHeaderHeight{ 46 };
private:
    IAdapterT<TCoord>* m_pAdapter{};
    IHostT<TCoord>* m_pHost{};

    int m_idxHot{ -1 };
    int m_idxSel{ -1 };       // 仅用于单选
    int m_idxInsertMark{ -1 };// 插入标记应当显示在哪一项之前
    int m_idxFocus{ -1 };
    int m_idxMark{ -1 };

    int m_idxTop{};
    TCoord m_oyTopItem{};     // 小于等于零的值，指示第一可见项的遮挡高度

    TCoord m_cyTopExtra{};
    TCoord m_cyBottomExtra{};

    TCoord m_cyItem{ DefaultItemHeight };
    TCoord m_cyPadding{ DefaultPadding };

    TCoord m_cyHeader{ DefaultHeaderHeight };

    // -- 分组

    int m_idxHotItemGroup{ -1 };    // 热点项所在的组
    int m_idxSelItemGroup{ -1 };    // 选中项所在的组
    int m_idxInsertMarkGroup{ -1 }; // 插入标记所在的组
    int m_idxFocusItemGroup{ -1 };  // 焦点项所在的组
    int m_idxMarkItemGroup{ -1 };   // 标记项所在的组
    int m_idxTopGroup{};            // 第一个可见组
    int m_idxHotGroup{ -1 };        // 热点组

    TCoord m_cyGroupHeader{ 40 };   // 组头高度
    TCoord m_cxGroupImage{};        // 组图片宽度
    TCoord m_cyGroupImage{};        // 组图片高度
    TCoord m_cyGroupItemTopPadding{ 4 };      // 组头与项目间的空白
    TCoord m_cyGroupItemBottomPadding{};   // 项目区底部空白

    // -- 图标

    TCoord m_cxItem{ 40 };      // 项目宽度
    TCoord m_cxPadding{ 3 };    // 水平项目间距
    int m_cItemPerRow{};        // 每行项目数

    // --

    TCoord m_xDragSelStart{}, m_yDragSelStart{};// 拖动选择起始点
    TRect m_rcDragSel{};		// 当前选择矩形
    TCoord m_dCursorToItemMax{};// 鼠标指针到项目的最大距离

    View m_eView{ View::List };
    Selection m_eSelection{ Selection::Single };

    BITBOOL m_bGroup : 1{1};
    BITBOOL m_bGroupImage : 1{};
    BITBOOL m_bEnableDragSel : 1{ TRUE };
    BITBOOL m_bVarItemHeight : 1{};
    BITBOOL m_bEnableHeader : 1{};

    BITBOOL m_bDraggingSel : 1{};
    BITBOOL m_bPendingDragSel : 1{};
public:
    CControllerT() = default;
    constexpr CControllerT(IHostT<TCoord>* pHost) noexcept : m_pHost(pHost) {}

    void SetAdapter(IAdapterT<TCoord>* pAdapter) noexcept
    {
        m_pAdapter = pAdapter;
        ReCalculateTopItem();
    }
    EckInlineNdCe auto GetAdapter() const noexcept { return m_pAdapter; }
    void SetHost(IHostT<TCoord>* pHost) noexcept
    {
        m_pHost = pHost;
    }

    void ReCalculateTopItem() noexcept
    {
        const auto posV = m_pHost->LchSccGetPosition(TRUE);
        switch (m_eView)
        {
        case View::List:
        {
            if (m_bGroup)
            {
                if (!m_pAdapter->LcaGetCount().Group)
                {
                    m_idxTopGroup = m_idxTop = 0;
                    return;
                }

                Index idx{ InvalidIndex };
                TCoord yGroup;
                GroupFromY(idx, posV, &yGroup);
                m_oyTopItem = yGroup - posV;
                m_idxTop = int((posV -
                    (yGroup + m_cyGroupHeader + m_cyGroupItemTopPadding)) / m_cyItem);
                if (m_idxTop < 0)
                    m_idxTop = 0;
            }
            else
            {
                m_idxTop = int(posV / (m_cyItem + m_cyPadding));
                m_oyTopItem = m_idxTop * (m_cyItem + m_cyPadding) - posV;
            }
        }
        break;
        case View::Icon:
        {
            const int cItemV = int(posV / (m_cyItem + m_cyPadding));
            m_idxTop = cItemV * m_cItemPerRow;
            m_oyTopItem = cItemV * (m_cyItem + m_cyPadding) - posV;
        }
        break;
        }
    }

    void ReCalculateScrollV() noexcept
    {
        if (!m_cyItem || !m_cxItem)
            return;
        const auto cx = m_pHost->LchGetWidth();

        TCoord cyContent;
        switch (m_eView)
        {
        case View::List:
        {
            if (m_bGroup)
            {
                const int idxLast = m_pAdapter->LcaGetCount().Group - 1;
                cyContent =
                    m_pAdapter->AptListGetGroupTop(idxLast) +
                    m_pAdapter->AptListGetGroupHeight(idxLast);
            }
            else
            {
                if (m_bVarItemHeight)
                {
                    const Index idxLast{ m_pAdapter->LcaGetCount().Item - 1 };
                    cyContent =
                        m_pAdapter->AptListGetItemTop(idxLast) +
                        m_pAdapter->AptListGetItemHeight(idxLast);
                }
                else
                {
                    const auto cItem = m_pAdapter->LcaGetCount().Item;
                    cyContent = cItem * (m_cyItem + m_cyPadding);
                }
            }
        }
        break;
        case View::Icon:
        {
            m_cItemPerRow = int((cx + m_cxPadding) / (m_cxItem + m_cxPadding));
            const auto cItem = m_pAdapter->LcaGetCount().Item;
            const int cItemV = (cItem - 1) / m_cItemPerRow + 1;
            cyContent = cItemV * (m_cyItem + m_cyPadding);
        }
        break;
        default:
            cyContent = 0;
            break;
        }
        m_pHost->LchSccSetRange(TRUE, -MtRealTopExtra(), cyContent + m_cyBottomExtra);
    }

    void GroupFromY(
        _Inout_ Index& idx,
        TCoord y,
        _Out_opt_ TCoord* py = nullptr) const noexcept
    {
        m_pAdapter->LcaGroupFromY(idx, y, py);
    }
    void LvVariableHeightItemFromY(
        _Inout_ Index& idx,
        TCoord y,
        _Out_opt_ TCoord* pyItem = nullptr) const noexcept
    {
        m_pAdapter->LcaVariableHeightItemFromY(idx, y, pyItem);
    }

    /// <summary>
    /// 视图位置到索引。
    /// WARNING 此函数忽略间距，将无效位置夹紧到[0, Max - 1]
    /// </summary>
    /// <param name="y">在视图中的y坐标</param>
    /// <param name="pyItem">返回项目在视图中的y坐标</param>
    /// <param name="pyGroup">返回组在视图中的y坐标</param>
    /// <returns>命中项目</returns>
    Index LvItemFromY(
        TCoord y,
        _Out_opt_ TCoord* pyItem = nullptr,
        _Out_opt_ TCoord* pyGroup = nullptr) const noexcept
    {
        const auto posV = m_pHost->LchSccGetPosition(TRUE);
        y += posV;
        Index idx{ InvalidIndex };
        if (m_bVarItemHeight)
        {
            if (m_bGroup)
            {
                GroupFromY(idx, y, pyGroup);
                if (pyGroup)
                    *pyGroup -= posV;
            }
            else
                if (pyGroup) *pyGroup = 0;
            LvVariableHeightItemFromY(idx, y, pyItem);
            if (pyItem)
                *pyItem -= posV;
            return idx;
        }
        else
        {
            if (m_bGroup)
            {
                TCoord yGroup;
                GroupFromY(idx, y, &yGroup);
                if (pyGroup)
                    *pyGroup = yGroup - posV;

                const auto yGroupBottom =
                    yGroup + m_cyGroupHeader + m_cyGroupItemTopPadding;
                const auto yInGroup = y - yGroupBottom;

                idx.Item = int(yInGroup / (m_cyItem + m_cyPadding));
                const auto cItem = m_pAdapter->LcaGetCount(idx.Group);
                if (idx.Item >= cItem)
                    idx.Item = cItem - 1;
                if (pyItem)
                    *pyItem = idx.Item * (m_cyItem + m_cyPadding) + yGroupBottom - posV;
            }
            else
            {
                idx.Item = int(y / (m_cyItem + m_cyPadding));
                const auto cItem = m_pAdapter->LcaGetCount().Item;
                if (idx.Item >= cItem)
                    idx.Item = cItem - 1;
                if (pyItem)
                    *pyItem = idx.Item * (m_cyItem + m_cyPadding) - posV;
                if (pyGroup) *pyGroup = 0;
            }
        }
        return idx;
    }

    IndexRange LvCalculateItemRange(TCoord y0, TCoord y1) noexcept
    {
        return { LvItemFromY(y0), LvItemFromY(y1) };
    }

    TCoord LvGetContentWidth() const noexcept
    {
        return m_pHost->LchGetWidth();
    }

    // NOTE 回调得到的项目或组的部件不一定都需要重画，
    // 调用方应获取部件矩形并与脏区域进行相交测试以排除不在脏区的部件
    void ForEachItem(
        std::invocable<const FOR_ITEM&> auto&& FnItem,
        std::invocable<const FOR_GROUP&> auto&& FnGroup,
        const TRect& rc, BOOL bPaint) noexcept
    {
        if (m_bGroup)
        {
            switch (m_eView)
            {
            case View::List:
            {
                auto idx0 = LvItemFromY(rc.top);
                const auto idx1 = LvItemFromY(rc.bottom);

                for (; idx0.Group <= idx1.Group; ++idx0.Group)
                {
                    FnGroup({ idx0.Group });
                    const auto cGroup = m_pAdapter->LcaGetCount(idx0.Group);
                    EckCounter(cGroup, i)
                        FnItem({ { i, idx0.Group } });
                }
            }
            break;
            }
        }
        else
        {
            switch (m_eView)
            {
            case View::List:
            {
                auto idx = LvItemFromY(rc.top);
                const auto idx1 = LvItemFromY(rc.bottom);
                for (; idx.Item <= idx1.Item; ++idx.Item)
                    FnItem({ idx });
            }
            break;
            }
        }
    }

    void GetItemRect(Index idx, _Out_ TRect& rc) const noexcept
    {
        const auto posV = m_pHost->LchSccGetPosition(TRUE);
        switch (m_eView)
        {
        case View::List:
            rc.left = 0;
            rc.right = LvGetContentWidth();
            if (m_bVarItemHeight)
            {
                rc.top = m_pAdapter->AptListGetItemTop(idx) - posV;
                rc.bottom = rc.top + m_pAdapter->AptListGetItemHeight(idx);
            }
            else
            {
                if (m_bGroup)
                {
                    const auto yGroup = m_pAdapter->AptListGetGroupTop(idx.Group);
                    rc.top = yGroup + m_cyGroupHeader + m_cyGroupItemTopPadding +
                        idx.Item * (m_cyItem + m_cyPadding) - posV;
                }
                else
                    rc.top = idx.Item * (m_cyItem + m_cyPadding) - posV;
                rc.bottom = rc.top + m_cyItem;
            }
            break;
        case View::Icon:
            rc = {};
            break;
        default:
            rc = {};
            break;
        }
    }

    void GetGroupRect(int idxGroup, Part ePart, _Out_ TRect& rc) const noexcept
    {
        switch (m_eView)
        {
        case View::List:
            rc.left = 0;
            rc.right = LvGetContentWidth();
            rc.top = m_pAdapter->AptListGetGroupTop(idxGroup) -
                m_pHost->LchSccGetPosition(TRUE);
            rc.bottom = rc.top + m_cyGroupHeader;
            break;
        case View::Icon:
            rc = {};
            break;
        default:
            rc = {};
            break;
        }
    }

    void GetIndexRangeRect(const IndexRange& idxRange, _Out_ TRect& rc) const noexcept
    {
        switch (m_eView)
        {
        case View::List:
            if (idxRange.Begin.IsValid())
                if (idxRange.End.IsValid())
                {
                    TRect rc1;
                    GetItemRect(idxRange.Begin, rc);
                    GetItemRect(idxRange.End, rc1);
                    UnionRect(rc, rc, rc1);
                }
                else
                    GetItemRect(idxRange.Begin, rc);
            else
                if (idxRange.End.IsValid())
                    GetItemRect(idxRange.End, rc);
                else
                    rc = {};
            break;
        case View::Icon:
            rc = {};
            break;
        default:
            rc = {};
            break;
        }
    }

    Index HitTest(HT_INFO& Info) const noexcept
    {
        if (Info.x < 0 || Info.x > m_pHost->LchGetWidth() ||
            Info.y < 0 || Info.y > m_pHost->LchGetHeight())
            return InvalidIndex;
        switch (m_eView)
        {
        case View::List:
        {
            TCoord yItem, cyItem, yGroup;
            const auto idx = LvItemFromY(Info.y, &yItem, &yGroup);
            if (m_bVarItemHeight)
            {
                std::any Data{};
                m_pAdapter->LcaGet(idx, 0, Property::UiHeight, Data);
                cyItem = (Data.type() == typeid(TCoord)) ?
                    std::any_cast<TCoord>(Data) : m_cyItem;
            }
            else
                cyItem = m_cyItem;
            if (m_bGroup)
            {
                if (Info.y <= yGroup + m_cyGroupHeader)
                    return { .Group = idx.Group };
            }
            if (Info.y <= yItem + cyItem)
                return idx;
        }
        break;
        case View::Icon:
            break;
        }
        return InvalidIndex;
    }

    void InvalidateItem(const Index& idx) noexcept
    {
        TRect rc;
        if (idx.Item < 0)
            GetGroupRect(idx.Group, Part::GroupHeader, rc);
        else
            GetItemRect(idx, rc);
        m_pHost->LchInvalidateRect(&rc);
    }

    void OnMouseMove(TCoord x, TCoord y, WPARAM uMk) noexcept
    {
        if (m_bDraggingSel)
        {

        }
        else
        {

            HT_INFO Info{ x, y };
            auto idx = HitTest(Info);
            if (idx.Item != m_idxHot || idx.Group != m_idxHotItemGroup)
            {
                std::swap(idx.Item, m_idxHot);
                std::swap(idx.Group, m_idxHotItemGroup);

                if (m_idxHot >= 0 || m_idxHotItemGroup >= 0)
                    InvalidateItem({ m_idxHot, m_idxHotItemGroup });
                if (idx.IsValid())
                    InvalidateItem(idx);
            }
        }
    }
    void OnLButtonDown(TCoord x, TCoord y, WPARAM uMk) noexcept
    {
        HT_INFO Info{ x, y };
        auto idx = HitTest(Info);

        if (m_eSelection != Selection::Single &&
            m_bEnableDragSel &&
            !idx.IsValid())
        {
            m_bPendingDragSel = TRUE;
            m_xDragSelStart = x;
            m_yDragSelStart = y;
        }

        switch (m_eSelection)
        {
        case Selection::Single:
            if (idx.Item != m_idxSel || idx.Group != m_idxSelItemGroup)
            {
                std::swap(idx.Item, m_idxSel);
                std::swap(idx.Group, m_idxSelItemGroup);
                if (m_idxSel >= 0 || m_idxSelItemGroup >= 0)
                    InvalidateItem({ m_idxSel, m_idxSelItemGroup });
                if (idx.IsValid())
                    InvalidateItem(idx);
            }
            break;
        case Selection::Multiple:
        {
            ItmDeselectAll();
            if (idx.IsValid())
            {
                std::any Data{};
                ItmpToggleSelect(idx, Data, TRUE);
            }
        }
        break;
        }
    }
    void OnLButtonUp(TCoord x, TCoord y, WPARAM uMk) noexcept
    {
        m_bDraggingSel = FALSE;
    }

    EckInlineNdCe int ItmGetHot() const noexcept { return m_idxHot; }
    EckInlineNdCe int ItmGetSelected() const noexcept { return m_idxSel; }

    [[nodiscard]] TState ItmGetState(Index idx) const noexcept
    {
        std::any Data{};
        m_pAdapter->LcaGet(idx, 0, Property::SystemState, Data);

        TState uState = (Data.type() == typeid(TState)) ? std::any_cast<TState>(Data) : 0;
        if (idx.Item == m_idxHot && idx.Group == m_idxHotItemGroup)
            uState |= LIF_HOT;
        if (m_eSelection == Selection::Single)
        {
            uState &= ~LIF_SELECTED;
            if (idx.Item == m_idxSel && idx.Group == m_idxSelItemGroup)
                uState |= LIF_SELECTED;
        }
        return uState;
    }

    void ItmListReCalculatePosition(Index idxBegin = { 0, 0 }) const noexcept
    {
        TCoord y;
        if (m_bGroup)
        {
            const auto cGroup = m_pAdapter->LcaGetCount().Group;
            if (!cGroup)
                return;
            if (m_bVarItemHeight)
            {

            }
            else
            {
                if (idxBegin.Group - 1 < 0)
                    y = 0;
                else
                {
                    const auto r = m_pAdapter->AptListGetGroupTopHeight(idxBegin.Group - 1);
                    y = r.first + r.second;
                }

                for (int i = idxBegin.Group; i < cGroup; ++i)
                {
                    const auto cItem = m_pAdapter->LcaGetCount(i);
                    const auto cy = std::max(m_cyGroupImage,
                        m_cyGroupHeader +
                        m_cyGroupItemTopPadding +
                        (m_cyItem + m_cyPadding) * cItem +
                        m_cyGroupItemBottomPadding);
                    m_pAdapter->AptListSetGroupTopHeight(i, y, cy);
                    y += cy;
                }
            }
        }
        else if (m_bVarItemHeight)
        {

        }
    }
private:
    void ItmpToggleSelect(Index idx, std::any& Data, BOOL bSelect) noexcept
    {
        m_pAdapter->LcaGet(idx, 0, Property::SystemState, Data);
        if (Data.type() == typeid(TState))
        {
            auto& u = std::any_cast<TState&>(Data);
            if (bSelect)
            {
                if (u & LIF_SELECTED)
                    return;
                u |= LIF_SELECTED;
            }
            else
            {
                if (!(u & LIF_SELECTED))
                    return;
                u &= ~LIF_SELECTED;
            }
            m_pAdapter->LcaSet(idx, 0, Property::SystemState, Data);
        }
    }
public:
    void ItmDeselectAll() noexcept
    {
        if (m_eSelection == Selection::Single)
        {
            m_idxSel = m_idxSelItemGroup = -1;
            return;
        }

        std::any Data{};
        if (m_bGroup)
        {
            Index idx{};
            const auto cGroup = m_pAdapter->LcaGetCount().Group;
            for (idx.Group = 0; idx.Group < cGroup; ++idx.Group)
            {
                idx.Item = -1;
                ItmpToggleSelect(idx, Data, FALSE);
                const auto cItem = m_pAdapter->LcaGetCount(idx.Group);
                for (idx.Item = 0; idx.Item < cItem; ++idx.Item)
                    ItmpToggleSelect(idx, Data, FALSE);
            }
        }
        else
        {
            const auto cItem = m_pAdapter->LcaGetCount().Item;
            Index idx{};
            for (idx.Item = 0; idx.Item < cItem; ++idx.Item)
                ItmpToggleSelect(idx, Data, FALSE);
        }
    }

    EckInlineNdCe TCoord MtRealTopExtra() const noexcept
    {
        if (m_eView == View::List && m_bEnableHeader)
            return m_cyTopExtra + m_cyHeader;
        return m_cyBottomExtra;
    }

    EckInlineCe void MtSetTopExtra(TCoord v) noexcept { m_cyTopExtra = v; }
    EckInlineNdCe TCoord MtGetTopExtra() const noexcept { return m_cyTopExtra; }
    EckInlineCe void MtSetBottomExtra(TCoord v) noexcept { m_cyBottomExtra = v; }
    EckInlineNdCe TCoord MtGetBottomExtra() const noexcept { return m_cyBottomExtra; }
    EckInlineCe void MtSetItemSpace(TCoord v) noexcept { m_cyPadding = v; }
    EckInlineNdCe TCoord MtGetItemSpace() const noexcept { return m_cyPadding; }
    EckInlineCe void MtSetItemHeight(TCoord v) noexcept { m_cyItem = v; }
    EckInlineNdCe TCoord MtGetItemHeight() const noexcept { return m_cyItem; }
    EckInlineCe void MtSetGroupHeaderHeight(TCoord v) noexcept { m_cyGroupHeader = v; }
    EckInlineNdCe TCoord MtGetGroupHeaderHeight() const noexcept { return m_cyGroupHeader; }
    EckInlineCe void MtSetGroupImageWidth(TCoord v) noexcept { m_cxGroupImage = v; }
    EckInlineNdCe TCoord MtGetGroupImageWidth() const noexcept { return m_cxGroupImage; }
    EckInlineCe void MtSetGroupImageHeight(TCoord v) noexcept { m_cyGroupImage = v; }
    EckInlineNdCe TCoord MtGetGroupImageHeight() const noexcept { return m_cyGroupImage; }
    EckInlineCe void MtSetGroupItemTopPadding(TCoord v) noexcept { m_cyGroupItemTopPadding = v; }
    EckInlineNdCe TCoord MtGetGroupItemTopPadding() const noexcept { return m_cyGroupItemTopPadding; }
    EckInlineCe void MtSetGroupItemBottomPadding(TCoord v) noexcept { m_cyGroupItemBottomPadding = v; }
    EckInlineNdCe TCoord MtGetGroupItemBottomPadding() const noexcept { return m_cyGroupItemBottomPadding; }
    EckInlineCe void MtSetHeaderHeight(TCoord v) noexcept { m_cyHeader = v; }
    EckInlineNdCe TCoord MtGetHeaderHeight() const noexcept { return m_cyHeader; }

    EckInlineCe void SetUseHeader(BOOL b) noexcept { m_bEnableHeader = b; }
    EckInlineNdCe BOOL GetUseHeader() const noexcept { return m_bEnableHeader; }

    EckInlineNdCe View GetView() const noexcept { return m_eView; }
};
ECK_LC_NAMESPACE_END
ECK_UIBASIC_NAMESPACE_END
ECK_NAMESPACE_END