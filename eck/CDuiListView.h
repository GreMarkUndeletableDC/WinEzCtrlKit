#pragma once
#include "CDuiScrollBar.h"
#include "CDuiHeader.h"
#include "CD2DImageList.h"
#include "RefPtr.h"
#include "UiListController.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
class CListView : public CElement, public UiBasic::Lc::IHostT<float>
{
public:
    using IAdapter = UiBasic::Lc::IAdapterT<float>;

    const static inline UINT IdMeInsertMarkWidth = TmNextResourceId();
    const static inline UINT IdMeGroupLineWidth = TmNextResourceId();

    const static inline UINT IdPtItem = TmNextResourceId();
    const static inline UINT IdPtGroup = TmNextResourceId();
    const static inline UINT IdPtDragSelectRect = TmNextResourceId();

    constexpr static float DefaultInsertMarkWidth = 4.f;
    constexpr static float DefaultGroupLineWidth = 1.f;

    constexpr static float DefaultIndicatorWidth = 4.f;
    constexpr static float DefaultIndicatorPaddingWidth = 4.f;
    constexpr static float DefaultIndicatorPaddingHeight = 10.f;

    enum : UINT
    {
        SsItemNormal,
        SsItemHot,
        SsItemSelected,
        SsGroupNormal,
        SsGroupHot,
        SsGroupSelected,

        SsIndicator,

        SsDragSelectRect,

        SsMax
    };

    using TController = UiBasic::Lc::CControllerT;
    using Selection = TController::Selection;
    using View = TController::View;
public:
    static RcPtr<CTheme> TmMakeDefaultTheme(BOOL bDark) noexcept;
    static RcPtr<CTheme> TmDefaultTheme(BOOL bDark) noexcept
    {
        static auto p1{ TmMakeDefaultTheme(TRUE) };
        static auto p2{ TmMakeDefaultTheme(FALSE) };
        return bDark ? p1 : p2;
    }
private:
    struct Indicator
    {
        float cxIndicator{ DefaultIndicatorWidth };
        float cxPadding{ DefaultIndicatorPaddingWidth };
        float cyPadding{ DefaultIndicatorPaddingHeight };
        EasingCurve<Easing::FOutExpo> ecIndicator1{};
        EasingCurve<Easing::FOutExpo> ecIndicator2{};
    };

    std::unique_ptr<CHeader> m_pHeader{};
    std::unique_ptr<CScrollBar> m_pSBHorz{};
    std::unique_ptr<CScrollBar> m_pSBVert{};
    IScrollController* m_pSccH{};
    IScrollController* m_pSccV{};

    RefPtr<CD2DImageList> m_pImgList{};
    RefPtr<CD2DImageList> m_pImgListGroup{};
    ComPtr<IDWriteTextFormat> m_pTfGroup{};

    TController m_Controller{ this };

    CStringW m_rsDispBuffer{};

    std::unique_ptr<Indicator> m_pIndicator{};

    SimpleStyle m_Style[SsMax]
    {
        { IdTmInvalid, IdTmInvalid, IdTmInvalid },
        TmsSsMakeHot(),
        TmsSsMakePressed(),
        { IdTmInvalid, IdTmInvalid, IdTmInvalid },
        TmsSsMakeHot(),
        TmsSsMakePressed(),

        { IdTmInvalid, IdCrAccent,  IdTmInvalid },
        { IdTmInvalid, IdTmInvalid, IdCrFore, 0.f, 1.f },
    };

    BITBOOL m_bUseBuiltInScrollBar : 1{ TRUE };
    BITBOOL m_bIndicator : 1{};
    BITBOOL m_bCapturedMouse : 1{};
protected:
    void PaintCell(
        const D2D1_RECT_F& rc,
        UiBasic::Lc::Index idx,
        int idxCol) noexcept
    {
        std::any Data{};
        float x{ rc.left };
        if (m_pImgList)
        {
            const auto Size = m_pImgList->GetTileSizeLogical();
            m_Controller.GetAdapter()->LcaGet(idx, idxCol, UiBasic::Lc::Property::Image, Data);
            if (Data.type() == typeid(int))
            {
                const auto idxImg = std::any_cast<int>(Data);
                if (idxImg >= 0)
                {
                    Kw::Rect rcImg;
                    rcImg.left = rc.left;
                    rcImg.top = rc.top + (rc.bottom - rc.top - Size.height) / 2;
                    rcImg.right = rcImg.left + Size.width;
                    rcImg.bottom = rcImg.top + Size.height;
                    m_pImgList->Draw(GetDC(), idxImg, Kw::MakeD2DRectF(rcImg));
                    x += (Size.width + GetTheme()->GetMetric(IdMePaddingInner));
                }
            }
        }

        ComPtr<IDWriteTextLayout> pTl;
        m_Controller.GetAdapter()->LcaGet(idx, idxCol, UiBasic::Lc::Property::UiTextLayout, Data);
        if (Data.type() == typeid(ComPtr<IDWriteTextLayout>))
            pTl = std::move(std::any_cast<ComPtr<IDWriteTextLayout>&>(Data));

        if (!pTl)
        {
            Data = &m_rsDispBuffer;
            m_Controller.GetAdapter()->LcaGet(idx, idxCol, UiBasic::Lc::Property::Text, Data);
            const auto svText = UiBasic::Lc::AptWrapAnyText(Data);
            if (!svText.empty())
            {
                g_pDwFactory->CreateTextLayout(
                    svText.data(), (UINT)svText.size(),
                    idx.Item < 0 ? GetGroupTextFormat().Get() : GetTextFormat().Get(),
                    rc.right - x,
                    rc.bottom - rc.top, &pTl);
                if (pTl)
                {
                    Data = pTl;
                    m_Controller.GetAdapter()->LcaSet(idx, idxCol,
                        UiBasic::Lc::Property::UiTextLayout, Data);
                }
            }
        }

        if (pTl)
            GetDC()->DrawTextLayout(
                { x, rc.top },
                pTl.Get(),
                GetWindow().CcSetBrushColor(GetTheme()->GetColorD2D(IdCrFore)),
                DrawTextLayoutFlags);
    }
    void PaintItem(UiBasic::Lc::Index idx, const D2D1_RECT_F& rcClip) noexcept
    {
        Kw::Rect rcItem;
        m_Controller.GetItemRect(idx, rcItem);
        ElementToClient(rcItem);
        if (!IsRectsIntersect(Kw::MakeD2DRectF(rcItem), rcClip))
            return;

        GetTheme()->Draw(
            this,
            &m_Style[TmSimpleStyleFromItemState(m_Controller.ItmGetState(idx))],
            IdPtItem,
            Kw::MakeD2DRectF(rcItem),
            &rcClip);

        const auto dOuter = GetTheme()->GetMetric(IdMePaddingOuter);
        InflateRect(rcItem, -dOuter, -dOuter);

        switch (m_Controller.GetView())
        {
        case TController::View::Icon:
            break;
        case TController::View::List:
        {
            if (m_bIndicator)
            {
                // TODO
            }

            if (m_pHeader && m_pHeader->IsValid())
            {
                EckCounter(m_pHeader->GetItemCount(), io)
                {
                    const auto idxCol = m_pHeader->OrderToIndex(io);
                    Kw::Rect rcText;
                    m_pHeader->GetItemRect(idxCol, rcText);
                    OffsetRect(rcText, m_pHeader->GetRect().left, 0.f);
                    ElementToClient(rcText);

                    if (rcText.right <= rcClip.left)
                        continue;
                    if (rcText.left >= rcClip.right)
                        break;
                    rcText.top = rcItem.top;
                    rcText.bottom = rcItem.bottom;
                    PaintCell(Kw::MakeD2DRectF(rcText), idx, idxCol);
                }
            }
            else
                PaintCell(Kw::MakeD2DRectF(rcItem), idx, 0);
        }
        break;
        }

    }
    void PaintGroup(UiBasic::Lc::Index idx, const D2D1_RECT_F& rcClip) noexcept
    {
        Kw::Rect rcItem;
        m_Controller.GetGroupRect(idx.Group,
            TController::Part::GroupHeader, rcItem);
        ElementToClient(rcItem);
        if (!IsRectsIntersect(Kw::MakeD2DRectF(rcItem), rcClip))
            return;

        GetTheme()->Draw(
            this,
            &m_Style[TmSimpleStyleFromGroupState(m_Controller.ItmGetState(idx))],
            IdPtGroup,
            Kw::MakeD2DRectF(rcItem),
            &rcClip);

        PaintCell(Kw::MakeD2DRectF(rcItem), idx, 0);
    }

    void ScbCreateElement(BOOL bVert, BOOL bHorz) noexcept
    {
        const DWORD Style = TmDarkStyle() |
            DES_NO_FOCUSABLE | DES_VISIBLE | DES_NO_CLIP | DES_NOTIFY_PARENT;
        if (!m_bUseBuiltInScrollBar)
            return;
        SccDisconnectEvent();
        if (bVert)
        {
            if (!m_pSBVert)
                m_pSBVert = std::make_unique<CScrollBar>();
            m_pSccV = m_pSBVert.get();
            if (!m_pSBVert->IsValid())
            {
                m_pSBVert->Create({}, Style, 0, 0, 0, 0, 0, this);
                m_pSBVert->SetVertical(TRUE);
            }
        }
        if (bHorz)
        {
            if (!m_pSBHorz)
                m_pSBHorz = std::make_unique<CScrollBar>();
            m_pSccH = m_pSBHorz.get();
            if (!m_pSBHorz->IsValid())
            {
                m_pSBHorz->Create({}, Style, 0, 0, 0, 0, 0, this);
                m_pSBHorz->SetVertical(FALSE);
            }
        }
        SccConnectEvent();
    }

    void ScbLayout() noexcept
    {
        if ((!m_pSBHorz && !m_pSBVert) || !m_bUseBuiltInScrollBar)
            return;
        const auto cx = GetWidth();
        const auto cy = GetHeight();
        const auto cxySB = GetTheme()->GetMetric(IdMeScrollBar);
        const auto bVert = m_pSBVert && m_pSBVert->IsValid() &&
            m_pSBVert->GetScrollView().IsValid();
        const auto bHorz = m_pSBHorz && m_pSBHorz->IsValid() &&
            m_pSBHorz->GetScrollView().IsValid();
        if (bVert)
            m_pSBVert->SetRect({
                GetWidth() - cxySB,
                m_Controller.MtRealTopExtra(),
                GetWidth(),
                GetHeight() - m_Controller.MtGetBottomExtra() - (bHorz ? cxySB : 0) });
        if (bHorz)
            m_pSBHorz->SetRect({
                0,
                GetHeight() - cxySB,
                GetWidth() - (bVert ? cxySB : 0),
                GetHeight() });
    }

    void SccUpdatePage() noexcept
    {
        if (m_pSccV)
            m_pSccV->SccSetPage(GetHeight());
        if (m_pSccH)
            m_pSccH->SccSetPage(GetWidth());
    }

    void SccConnectEvent() noexcept
    {
        if (m_pSccV)
            m_pSccV->SccSetCallback(
                [](const IScrollController::SCC_CALLBACK_DATA& Data)
                {
                    const auto p = (CListView*)Data.pUser;
                    p->m_Controller.ReCalculateTopItem();
                    p->Invalidate();
                }, this);
        if (m_pSccH)
            m_pSccH->SccSetCallback(
                [](const IScrollController::SCC_CALLBACK_DATA& Data)
                {
                    const auto p = (CListView*)Data.pUser;
                    p->GetWindow().RdLockUpdate();
                    p->HdrLayout();
                    p->Invalidate();
                    p->GetWindow().RdUnlockUpdate();
                }, this);
    }
    void SccDisconnectEvent() noexcept
    {
        if (m_pSccV)
            m_pSccV->SccSetCallback(nullptr, nullptr);
        if (m_pSccH)
            m_pSccH->SccSetCallback(nullptr, nullptr);
    }

    UINT TmSimpleStyleFromItemState(UiBasic::Lc::TState uState) const noexcept
    {
        if (uState & UiBasic::Lc::LIF_SELECTED)
            return SsItemSelected;
        if (uState & UiBasic::Lc::LIF_HOT)
            return SsItemHot;
        return SsItemNormal;
    }
    UINT TmSimpleStyleFromGroupState(UiBasic::Lc::TState uState) const noexcept
    {
        if (uState & UiBasic::Lc::LIF_SELECTED)
            return SsGroupSelected;
        if (uState & UiBasic::Lc::LIF_HOT)
            return SsGroupHot;
        return SsGroupNormal;
    }

    void HdrCreateElement() noexcept
    {
        if (!m_pHeader)
            m_pHeader = std::make_unique<CHeader>();
        if (!m_pHeader->IsValid())
        {
            m_pHeader->Create(
                {},
                DES_VISIBLE | DES_DBG_FRAME | DES_NOTIFY_PARENT | TmDarkStyle(),
                0,
                0, 0, 0, 0, this);
            HdrSetTextFormat();
        }
    }

    void HdrLayout() noexcept
    {
        if (HdrIsEnabled())
        {
            Kw::Rect rc;
            rc.left = m_pSccH ? -m_pSccH->SccGetPosition() : 0;
            rc.top = 0;
            rc.right = rc.left + std::max(m_pHeader->GetContentWidth(), GetWidth());
            rc.bottom = rc.top + m_Controller.MtGetHeaderHeight();
            m_pHeader->SetRect(rc);
        }
    }

    void HdrSetTextFormat() const noexcept
    {
        if (HdrIsEnabled())
            m_pHeader->SetTextFormat(GetTextFormat().Get());
    }
public:
    LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept override
    {
        switch (uMsg)
        {
        case WM_PAINT:
        {
            PAINTINFO ps;
            BeginPaint(ps, wParam, lParam);

            if (m_Controller.GetAdapter())
            {
                m_Controller.ForEachItem(
                    [&](const TController::FOR_ITEM& e)
                    {
                        PaintItem(e.idx, ps.rcClip);
                    },
                    [&](const TController::FOR_GROUP& e)
                    {
                        PaintGroup({ .Group = e.idxGroup }, ps.rcClip);
                    }, Kw::MakeRect(ps.rcClipInEle), TRUE);

                if (m_Controller.IsDraggingSelect())
                {
                    auto rc{ Kw::MakeD2DRectF(m_Controller.GetDragSelectRect()) };
                    ElementToClient(rc);
                    GetTheme()->Draw(
                        this,
                        &m_Style[SsDragSelectRect],
                        IdPtDragSelectRect,
                        rc,
                        &ps.rcClip);
                }
            }

            DbgDrawFrame();
            EndPaint(ps);
        }
        return 0;

        case WM_SIZE:
        {
            SccUpdatePage();
            if (m_Controller.GetAdapter())
            {
                m_Controller.ReCalculateScrollV();
                m_Controller.ReCalculateScrollH();
                m_Controller.GetAdapter()->LcaColumnWidthChanged(-1, GetWidth());
            }
            HdrLayout();
            ScbLayout();
        }
        break;

        case WM_MOUSEMOVE:
        {
            const auto& pt = EagPoint(lParam);
            GetWindow().RdLockUpdate();
            m_Controller.OnMouseMove(pt.x, pt.y, wParam);
            GetWindow().RdUnlockUpdate();
        }
        return 0;
        case WM_LBUTTONDOWN:
        {
            m_bCapturedMouse = TRUE;
            SetCapture();
            const auto& pt = EagPoint(lParam);
            GetWindow().RdLockUpdate();
            m_Controller.OnLButtonDown(pt.x, pt.y, wParam);
            GetWindow().RdUnlockUpdate();
        }
        return 0;
        case WM_LBUTTONUP:
        {
            if (m_bCapturedMouse)
            {
                m_bCapturedMouse = FALSE;
                ReleaseCapture();
                const auto& pt = EagPoint(lParam);
                GetWindow().RdLockUpdate();
                m_Controller.OnLButtonUp(pt.x, pt.y, wParam);
                GetWindow().RdUnlockUpdate();
            }
        }
        return 0;
        case WM_MOUSELEAVE:
        {
            GetWindow().RdLockUpdate();
            m_Controller.OnMouseLeave();
            GetWindow().RdUnlockUpdate();
        }
        return 0;
        case WM_CAPTURECHANGED:
        {
            if (m_bCapturedMouse)
            {
                m_bCapturedMouse = FALSE;
                GetWindow().RdLockUpdate();
                m_Controller.OnCaptureChanged();
                GetWindow().RdUnlockUpdate();
            }
        }
        break;

        case WM_MOUSEWHEEL:
        {
            m_pSccV->SccMouseWheel(-(float)GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA);
            GetWindow().KctWake();
        }
        return 0;

        case WM_NOTIFY:
        {
            if (m_pHeader && wParam == (WPARAM)m_pHeader.get())
                switch (((ELENMHDR*)lParam)->uNotify)
                {
                case ENC_HD_ORDERCHANGED:
                {
                    const auto* const p = (CHeader::EVT_ORDER*)lParam;
                    Kw::Rect rc, rc1;
                    m_pHeader->GetItemRect(m_pHeader->OrderToIndex(p->ioFrom), rc);
                    m_pHeader->ElementToClient(rc);
                    ClientToElement(rc);
                    m_pHeader->GetItemRect(m_pHeader->OrderToIndex(p->ioTo), rc1);
                    m_pHeader->ElementToClient(rc1);
                    ClientToElement(rc1);
                    rc.left = std::min(rc.left, rc1.left);
                    rc.right = std::max(rc.right, rc1.right);
                    rc.top = m_Controller.MtGetHeaderHeight();
                    rc.bottom = GetHeight();
                    Invalidate(rc);
                }
                return 0;
                case ENC_HD_WIDTHCHANGED:
                {
                    const auto* const p = (CHeader::EVT_ITEM*)lParam;
                    Kw::Rect rc;
                    m_pHeader->GetItemRect(p->idx, rc);
                    m_pHeader->ElementToClient(rc);
                    ClientToElement(rc);
                    rc.right = m_pHeader->GetContentWidth();
                    rc.top = m_Controller.MtGetHeaderHeight();
                    rc.bottom = GetHeight();
                    if (m_Controller.GetAdapter())
                    {
                        m_Controller.ReCalculateScrollH();
                        m_Controller.GetAdapter()->LcaColumnWidthChanged(
                            p->idx, rc.right - rc.left);
                        HdrLayout();
                        ScbLayout();
                        Invalidate(rc);
                    }
                }
                return 0;
                }
        }
        break;
        case WM_SETFONT:
            HdrSetTextFormat();
            break;

        case EWM_COLORSCHEMECHANGED:
            TmAutoSwitchTheme(this, wParam);
            break;

        case WM_CREATE:
            SetTheme(TmDefaultTheme(TmIsDarkMode()).Get());
            ScbCreateElement(TRUE, TRUE);
            break;
        case WM_DESTROY:
            SccDisconnectEvent();
            break;
        }
        return __super::OnEvent(uMsg, wParam, lParam);
    }

    float LchSccGetPosition(BOOL bVert) const noexcept override
    {
        const auto pScc = (bVert ? m_pSccV : m_pSccH);
        if (pScc)
            return pScc->SccGetPosition();
        return 0.f;
    }
    void LchSccSetRange(BOOL bVert, TCoord Min, TCoord Max) noexcept override
    {
        const auto pScc = (bVert ? m_pSccV : m_pSccH);
        if (pScc)
            pScc->SccSetRange(Min, Max);
    }
    void LchSccScrollDelta(BOOL bVert, TCoord d, BOOL bSmooth) noexcept override
    {
        const auto pScc = (bVert ? m_pSccV : m_pSccH);
        if (pScc)
        {
            pScc->SccScrollDelta(d, bSmooth);
            if (bSmooth)
                GetWindow().KctWake();
        }
    }

    TCoord LchGetHeight() const noexcept override { return GetHeight(); }
    TCoord LchGetWidth() const noexcept override
    {
        return GetWidth();
    }
    TCoord LchGetListContentWidth() const noexcept override
    {
        if (m_pHeader && m_Controller.GetView() == TController::View::List)
            return m_pHeader->GetContentWidth();
        else
            return GetWidth();
    }
    void LchInvalidateRect(const Kw::Rect* prc) noexcept override
    {
        Invalidate(Kw::MakeD2DRectF(*prc));
    }

    EckInline void SetAdapter(IAdapter* pAdapter) noexcept
    {
        m_Controller.SetAdapter(pAdapter);
        ReCalculateItem();
    }
    EckInline void ReCalculateItem(int idxItemBegin = 0, int idxGroupBegin = 0) noexcept
    {
        m_Controller.ItmListReCalculatePosition({ idxItemBegin, idxGroupBegin });
        m_Controller.ReCalculateTopItem();
        m_Controller.ReCalculateScrollV();
    }

    EckInline void SetGroupTextFormat(IDWriteTextFormat* pTf) noexcept { m_pTfGroup = pTf; }
    EckInlineNdCe const ComPtr<IDWriteTextFormat>& GetGroupTextFormat() const noexcept { return m_pTfGroup; }

    EckInlineNd auto& GetHeader() noexcept { return *m_pHeader; }
    EckInlineNdCe auto& GetController() const noexcept { return m_Controller; }
    EckInlineNdCe auto& GetController() noexcept { return m_Controller; }

    EckInlineNd BOOL HdrIsEnabled() const noexcept { return m_pHeader && m_pHeader->IsValid(); }
    void HdrEnable(BOOL b) noexcept
    {
        m_Controller.SetUseHeader(b);
        if (b)
        {
            if (!HdrIsEnabled())
            {
                HdrCreateElement();
                HdrLayout();
                ScbLayout();
                m_Controller.ReCalculateScrollV();
            }
        }
        else
        {
            if (m_pHeader && m_pHeader->IsValid())
            {
                m_pHeader->Destroy();
                ScbLayout();
            }
        }
    }

    EckInline void SetImageList(RefPtr<CD2DImageList> pil) noexcept { m_pImgList = std::move(pil); }
    EckInlineNdCe const RefPtr<CD2DImageList>& GetImageList() const noexcept { return m_pImgList; }
};


class CTmListView : public CTheme
{
public:
    TmResult Draw(
        CElement* pEle,
        const SimpleStyle* pStyle,
        UINT idPart,
        const D2D1_RECT_F& rc,
        _In_opt_ const D2D1_RECT_F* prcClip) noexcept override
    {
        if (idPart != IdPtNormal &&
            idPart != CListView::IdPtItem &&
            idPart != CListView::IdPtGroup &&
            idPart != CListView::IdPtDragSelectRect)
            return TmResult::NotSupport;
        return pEle->TmGenericDrawBackground(pStyle, rc);
    }
};
inline RcPtr<CTheme> CListView::TmMakeDefaultTheme(BOOL bDark) noexcept
{
    return TmMakeTheme<CTmListView>(bDark);
}
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END