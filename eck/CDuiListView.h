#pragma once
#include "CDuiScrollBar.h"
//#include "CDuiHeader.h"
#include "CD2DImageList.h"
#include "RefPtr.h"
#include "UiListController.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
class CListView : public CElement, public UiBasic::Lc::IHostT<float>
{
public:
    using IAdapter = UiBasic::Lc::IAdapterT<float>;

    constexpr static float
        CyInsertMark = 3,
        CyDefHeader = 30,
        CyGroupLine = 1;

    const static inline UINT IdPtItem = TmNextResourceId();

    enum : UINT
    {
        SsItemNormal,
        SsItemHot,
        SsItemSelected,

        SsMax
    };
public:
    static RcPtr<CThemeBase> TmMakeDefaultTheme(BOOL bDark) noexcept;
    static RcPtr<CThemeBase> TmDefaultTheme(BOOL bDark) noexcept
    {
        static auto p1{ TmMakeDefaultTheme(TRUE) };
        static auto p2{ TmMakeDefaultTheme(FALSE) };
        return bDark ? p1 : p2;
    }
private:
    using TController = UiBasic::Lc::CControllerT;

    std::unique_ptr<CScrollBar> m_pSBHorz{};
    std::unique_ptr<CScrollBar> m_pSBVert{};
    IScrollController* m_pSccH{};
    IScrollController* m_pSccV{};

    RefPtr<CD2DImageList> m_pImgList{};
    RefPtr<CD2DImageList> m_pImgListGroup{};
    IDWriteTextFormat* m_pTfGroup{};

    TController m_Controller{};

    CStringW m_rsDispBuffer{};

    SimpleStyle m_Style[SsMax]
    {
        { IdTmInvalid, IdTmInvalid, IdTmInvalid },
        TmsSsMakeHot(),
        TmsSsMakePressed(),
    };

    BITBOOL m_bUseBuiltInScrollBar : 1{ TRUE };
protected:
    void PaintText(const D2D1_RECT_F& rcText, UiBasic::Lc::Index idx) noexcept
    {
        ComPtr<IDWriteTextLayout> pTl;
        std::any Data{};
        m_Controller.GetAdapter()->LcaGet(idx, 0, UiBasic::Lc::Property::UiTextLayout, Data);
        if (Data.type() == typeid(ComPtr<IDWriteTextLayout>))
            pTl = std::move(std::any_cast<ComPtr<IDWriteTextLayout>&>(Data));
        
        if (!pTl)
        {
            Data = &m_rsDispBuffer;
            m_Controller.GetAdapter()->LcaGet(idx, 0, UiBasic::Lc::Property::Text, Data);
            const auto svText = UiBasic::Lc::AptWrapAnyText(Data);
            if (!svText.empty())
            {
                g_pDwFactory->CreateTextLayout(
                    svText.data(), (UINT32)svText.size(),
                    GetTextFormat().Get(),
                    rcText.right - rcText.left,
                    rcText.bottom - rcText.top, &pTl);
            }
        }

        if (pTl)
            GetDC()->DrawTextLayout(
                { rcText.left, rcText.top },
                pTl.Get(),
                GetWindow().CcSetBrushColor(ArgbToD2DColorF(GetTheme()->GetColor(IdCrFore))),
                DrawTextLayoutFlags);
    }

    void ScbCreateElement(BOOL bVert, BOOL bHorz) noexcept
    {
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
                m_pSBVert->TmSetDarkMode(TmIsDarkMode());
                m_pSBVert->Create({}, DES_NO_FOCUSABLE | DES_VISIBLE | DES_NO_CLIP, 0,
                    0, 0, 0, 0, this);
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
                m_pSBHorz->TmSetDarkMode(TmIsDarkMode());
                m_pSBHorz->Create({}, DES_NO_FOCUSABLE | DES_VISIBLE | DES_NO_CLIP, 0,
                    0, 0, 0, 0, this);
            }
        }
        SccConnectEvent();
    }

    void ScbOnSize() noexcept
    {
        if ((!m_pSBHorz && !m_pSBVert) || !m_bUseBuiltInScrollBar)
            return;
        const auto cx = GetWidth();
        const auto cy = GetHeight();
        const auto cxySB = GetTheme()->GetMetric(IdMeScrollBar);
        if (m_pSBVert && m_pSBVert->IsValid())
            m_pSBVert->SetRect({
                GetWidth() - cxySB,
                m_Controller.MtGetTopExtra(),
                GetWidth(),
                GetHeight() - m_Controller.MtGetBottomExtra() });
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
                    p->Invalidate();
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

public:
    CListView() noexcept
    {
        m_Controller.SetHost(this);
    }

    LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept override
    {
        switch (uMsg)
        {
        case WM_PAINT:
        {
            PAINTINFO ps;
            BeginPaint(ps, wParam, lParam);

            if (m_Controller.GetAdapter())
                m_Controller.ForEachItem(
                    [&](const TController::FOR_ITEM& e)
                    {
                        Kw::Rect rcItem;
                        m_Controller.GetItemRect(e.idx, rcItem);
                        ElementToClient(rcItem);

                        GetTheme()->Draw(
                            this,
                            &m_Style[TmSimpleStyleFromItemState(m_Controller.ItmGetState(e.idx))],
                            IdPtItem,
                            Kw::MakeD2DRectF(rcItem),
                            &ps.rcfClip);

                        PaintText(Kw::MakeD2DRectF(rcItem), e.idx);
                    },
                    [&](const TController::FOR_GROUP& e)
                    {
                        Kw::Rect rcItem;
                        m_Controller.GetGroupRect(e.idxGroup,
                            TController::Part::GroupHeader, rcItem);
                        ElementToClient(rcItem);

                        GetDC()->FillRectangle(
                            Kw::MakeD2DRectF(rcItem),
                            GetWindow().CcSetBrushColor(D2D1::ColorF{ D2D1::ColorF::Red, 0.2f }));

                        //GetTheme()->Draw(
                        //    this,
                        //    &m_Style[TmSimpleStyleFromItemState(m_Controller.ItmGetState(e.idx))],
                        //    IdPtItem,
                        //    Kw::MakeD2DRectF(rcItem),
                        //    &ps.rcfClip);

                        PaintText(Kw::MakeD2DRectF(rcItem), { .Group = e.idxGroup });
                    }, Kw::MakeRect(ps.rcfClipInElem), TRUE);

            DbgDrawFrame();
            EndPaint(ps);
        }
        return 0;

        case WM_SIZE:
        {
            ScbOnSize();
            SccUpdatePage();
            if (m_Controller.GetAdapter())
                m_Controller.ReCalculateScrollV();
        }
        break;

        case WM_MOUSEMOVE:
        {
            const auto& pt = *(Kw::Vec2*)lParam;
            GetWindow().RdLockUpdate();
            m_Controller.OnMouseMove(pt.x, pt.y, wParam);
            GetWindow().RdUnlockUpdate();
        }
        return 0;
        case WM_LBUTTONDOWN:
        {
            const auto& pt = *(Kw::Vec2*)lParam;
            GetWindow().RdLockUpdate();
            m_Controller.OnLButtonDown(pt.x, pt.y, wParam);
            GetWindow().RdUnlockUpdate();
        }
        return 0;

        case WM_MOUSEWHEEL:
        {
            m_pSccV->SccMouseWheel(-(float)GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA);
            GetWindow().KctWake();
        }
        return 0;

        case WM_CREATE:
            SetTheme(TmDefaultTheme(TmIsDarkMode()).Get());
            ScbCreateElement(TRUE, FALSE);
            break;
        case WM_DESTROY:
            SccDisconnectEvent();
            break;
        }
        return __super::OnEvent(uMsg, wParam, lParam);
    }

    float LchSccGetPosition(BOOL bVert) const noexcept override
    {
        if (bVert)
            return m_pSccV ? m_pSccV->SccGetPosition() : 0;
        else
            return m_pSccH ? m_pSccH->SccGetPosition() : 0;
    }
    void LchSccSetRange(BOOL bVert, TCoord Min, TCoord Max) noexcept override
    {
        if (bVert)
        {
            if (m_pSccV)
                m_pSccV->SccSetRange(Min, Max);
        }
        else
        {
            if (m_pSccH)
                m_pSccH->SccSetRange(Min, Max);
        }
    }
    TCoord LchGetHeight() const noexcept override { return GetHeight(); }
    TCoord LchGetWidth() const noexcept override { return GetWidth(); }
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
};


class CTmListView : public CThemeBase
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
            idPart != CListView::IdPtItem)
            return TmResult::NotSupport;
        return pEle->TmGenericDrawBackground(pStyle, rc);
    }
};
inline RcPtr<CThemeBase> CListView::TmMakeDefaultTheme(BOOL bDark) noexcept
{
    return TmMakeTheme<CTmListView>(bDark);
}
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END