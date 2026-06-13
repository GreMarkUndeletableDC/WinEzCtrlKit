#pragma once
#include "DuiBase.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
class CHeader : public CElement, public ITimeLine
{
public:
    enum : UINT
    {
        SsNormal,
        SsHot,
        SsPressed,
        SsDisabled,
        SsMax
    };

    struct HITTEST
    {
        Kw::Vec2 pt;
        BOOLEAN bHitDivider;
    };

    struct EVT_ITEM : ELENMHDR
    {
        int idx;
    };

    struct EVT_ORDER : ELENMHDR
    {
        int idxFrom;
        int idxTo;
    };

    const inline static UINT IdMeInsertMarkWidth = TmNextResourceId();

    constexpr static float DividerHitTestWidthHalf = 4.f;
private:
    struct ITEM
    {
        CStringW rsText{};
        ComPtr<IDWriteTextLayout> pLayout{};
        float x{};      // 当前可视位置
        float xStart{}; // 拖动重排动画起始位置
        float xTarget{};// 拖动重排动画目标位置
        float cx{};
        LPARAM lParam{};
    };

    std::vector<ITEM> m_vItem{};
    EasingCurve<Easing::FOutCubic> m_ec{};

    int m_idxHot{ -1 };
    int m_idxPressed{ -1 };
    int m_idxDrag{ -1 };
    int m_idxInsertMark{ -1 };// 拖动时被拖项的最终序号
    float m_xDragOffset{};
    USHORT m_msLastDuration{};

    BITBOOL m_bDraggable : 1{};
    BITBOOL m_bAllowReSize : 1{ TRUE };

    BITBOOL m_bHitDivider : 1{};
    BITBOOL m_bDragging : 1{};
    BITBOOL m_bDraggingDivider : 1{};
    BITBOOL m_bAnimating : 1{};
    BITBOOL m_bLBtnDown : 1{};

    SimpleStyle m_Style[SsMax]
    {
        // Normal
        { IdCrFore, IdCrBack,         IdTmInvalid },
        // Hot
        { IdCrFore, IdCrBackHot,      IdTmInvalid },
        // Pressed
        { IdCrFore, IdCrBackPressed,  IdTmInvalid },
        // Disabled
        { IdCrFore, IdCrBackDisabled, IdTmInvalid },
    };

    UINT TmSimpleStyleFromItemState(int idx) noexcept
    {
        if (GetStyle() & DES_DISABLE)
            return SsDisabled;
        if (idx == m_idxPressed)
            return SsPressed;
        if (idx == m_idxHot)
            return SsHot;
        return SsNormal;
    }

    void UpdateTextLayout(int idx) noexcept
    {
        const auto dOuter = GetTheme()->GetMetric(IdMePaddingOuter);
        auto& e = m_vItem[idx];
        g_pDwFactory->CreateTextLayout(
            e.rsText.Data(), e.rsText.Size(),
            GetTextFormat().Get(),
            e.cx - dOuter * 2.f,
            GetHeight() - dOuter * 2,
            e.pLayout.AtClear());
        if (e.pLayout)
        {
            e.pLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
            e.pLayout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
        }
    }
    void UpdateAllTextLayout() noexcept
    {
        EckCounter(GetItemCount(), i)
            UpdateTextLayout(i);
    }

    void PaintItem(int idx, ITEM& e, const D2D1_RECT_F& rcClip) noexcept
    {
        const float dOuter = GetTheme()->GetMetric(IdMePaddingOuter);
        const auto iSs = TmSimpleStyleFromItemState(idx);

        D2D1_RECT_F rcItem{ e.x, 0.f, e.x + e.cx, GetHeight() };
        ElementToClient(rcItem);
        if (!IsRectsIntersect(rcItem, rcClip))
            return;
        GetTheme()->Draw(this, &m_Style[iSs], IdPtNormal, rcItem, &rcClip);

        if (e.pLayout)
        {
            GetDC()->DrawTextLayout(
                { rcItem.left + dOuter, rcItem.top + dOuter },
                e.pLayout.Get(),
                GetWindow().CcSetBrushColor(
                    ArgbToD2DColorF(GetTheme()->GetStyleColor(&m_Style[iSs], SfFore))));
        }
    }

    // 为了方便起见，允许传入超出有效范围的索引，函数自动夹紧到[0, ItemCount)
    void ReCalculateItemPosition(int idxBegin = 0) noexcept
    {
        m_bAnimating = FALSE;
        if (idxBegin >= GetItemCount())
            return;
        if (idxBegin < 0)
            idxBegin = 0;
        float x;
        if (idxBegin)
            x = m_vItem[idxBegin - 1].x + m_vItem[idxBegin - 1].cx;
        else
            x = 0.f;
        for (int i = idxBegin; i < GetItemCount(); ++i)
        {
            auto& e = m_vItem[i];
            e.x = e.xStart = e.xTarget = x;
            x += e.cx;
        }
    }

    void DragReCalculateTargetPosition() noexcept
    {
        const auto cItem = GetItemCount();
        if (m_idxDrag < 0 || m_idxDrag >= cItem || cItem <= 0)
            return;

        const auto idxInsert = std::clamp(m_idxInsertMark, 0, cItem - 1);
        float x{};
        int iSrc{};
        EckCounter(cItem, i)
        {
            if (i == idxInsert)
                x += m_vItem[m_idxDrag].cx;
            if (iSrc == m_idxDrag)
                ++iSrc;
            if (iSrc >= cItem)
                break;
            auto& e = m_vItem[iSrc++];
            e.xTarget = x;
            x += e.cx;
        }
        m_vItem[m_idxDrag].xTarget = m_vItem[m_idxDrag].x;
    }

    float DragGetInsertMarkPosition() const noexcept
    {
        const auto cItem = GetItemCount();
        if (m_idxDrag < 0 || m_idxDrag >= cItem || cItem <= 0)
            return 0.f;

        const auto idxInsert = std::clamp(m_idxInsertMark, 0, cItem - 1);
        float x{};
        int iSrc{};
        for (int iOrder{}; iOrder < idxInsert; ++iOrder)
        {
            while (iSrc == m_idxDrag)
                ++iSrc;
            if (iSrc >= cItem)
                break;
            x += m_vItem[iSrc++].cx;
        }
        return x;
    }

    void DragMoveItem(int idxFrom, int idxTo) noexcept
    {
        const auto cItem = GetItemCount();
        if (idxFrom < 0 || idxFrom >= cItem || cItem <= 0)
            return;
        idxTo = std::clamp(idxTo, 0, cItem - 1);
        if (idxFrom == idxTo)
            return;

        auto e = std::move(m_vItem[idxFrom]);
        if (idxFrom < idxTo)
        {
            for (int i{ idxFrom }; i < idxTo; ++i)
                m_vItem[i] = std::move(m_vItem[i + 1]);
        }
        else
        {
            for (int i{ idxFrom }; i > idxTo; --i)
                m_vItem[i] = std::move(m_vItem[i - 1]);
        }
        m_vItem[idxTo] = std::move(e);
        ReCalculateItemPosition();
    }
    void DragEnd(BOOL bCommitOrder) noexcept
    {
        if (m_idxDrag < 0)
            return;

        const auto idxDragOld = m_idxDrag;
        const auto idxPressedOld = m_idxPressed;
        const auto bWasDragging = m_bDragging;
        const auto bWasDraggingDivider = m_bDraggingDivider;
        const auto bSendEndDrag = bWasDragging || bWasDraggingDivider;
        int idxTo = idxDragOld;
        if (bWasDragging && bCommitOrder && m_bDraggable && GetItemCount() > 0)
            idxTo = std::clamp(m_idxInsertMark, 0, GetItemCount() - 1);

        m_idxPressed = -1;
        m_idxDrag = -1;
        m_idxInsertMark = -1;
        m_idxHot = -1;
        m_bHitDivider = FALSE;
        m_bDragging = FALSE;
        m_bDraggingDivider = FALSE;
        m_bAnimating = FALSE;

        if (bWasDragging && bCommitOrder && idxTo != idxDragOld)
        {
            DragMoveItem(idxDragOld, idxTo);
            EvtOrderChanged(idxDragOld, idxTo);
        }
        else if (bWasDragging || bWasDraggingDivider)
        {
            ReCalculateItemPosition();
            Invalidate();
        }
        else if (idxPressedOld >= 0)
            InvalidateItem(idxPressedOld);

        if (bSendEndDrag)
            EvtEndDrag(idxDragOld);
    }

    int DragCalculateInsertIndex(float xMouse) const noexcept
    {
        const auto cItem = GetItemCount();
        if (cItem <= 1)
            return 0;

        int iBest{};
        int iSlot{};
        float dBest{ fabsf(xMouse) };
        float x{};

        for (int iSrc{}; iSrc < cItem; ++iSrc)
        {
            if (iSrc == m_idxDrag)
                continue;

            x += m_vItem[iSrc].cx;
            ++iSlot;
            const auto d = fabsf(xMouse - x);
            if (d < dBest)
            {
                dBest = d;
                iBest = iSlot;
            }
        }
        return iBest;
    }

    BOOL DragBeginAnimation() noexcept
    {
        BOOL bNeedAnimation{};
        EckCounter(GetItemCount(), i)
        {
            if (i == m_idxDrag)
                continue;
            auto& e = m_vItem[i];
            e.xStart = e.x;
            if (fabsf(e.xTarget - e.x) > 0.1f)
                bNeedAnimation = TRUE;
        }

        if (bNeedAnimation)
        {
            m_bAnimating = TRUE;
            m_ec.Start(0.f, 1.f);
            GetWindow().KctWake();
        }
        else
        {
            EckCounter(GetItemCount(), i)
            {
                if (i == m_idxDrag)
                    continue;
                m_vItem[i].x = m_vItem[i].xTarget;
            }
            m_bAnimating = FALSE;
        }
        return bNeedAnimation;
    }
public:
    static RcPtr<CThemeBase> TmMakeDefaultTheme(BOOL bDark) noexcept;
    static RcPtr<CThemeBase> TmDefaultTheme(BOOL bDark) noexcept
    {
        static auto p1{ TmMakeDefaultTheme(TRUE) };
        static auto p2{ TmMakeDefaultTheme(FALSE) };
        return bDark ? p1 : p2;
    }

    HRESULT EhUiaMakeInterface() noexcept override;

    LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept override
    {
        switch (uMsg)
        {
        case WM_PAINT:
        {
            PAINTINFO ps;
            BeginPaint(ps, wParam, lParam);
            for (int i{}; i < GetItemCount(); ++i)
            {
                if (m_bDragging && i == m_idxDrag)
                    continue;
                auto& e = m_vItem[i];
                if (e.x > ps.rcfClipInElem.right ||
                    e.x + e.cx < ps.rcfClipInElem.left)
                    continue;
                PaintItem(i, e, ps.rcfClipInElem);
            }

            if (m_bDragging && m_bDraggable && m_idxInsertMark >= 0)
            {
                const float xMark = DragGetInsertMarkPosition();
                D2D1_RECT_F rcMark{ xMark - 1.5f, 0.f, xMark + 1.5f, (float)GetHeight() };
                ElementToClient(rcMark);
                const auto pBrush = GetWindow().CcSetBrushColor(
                    ArgbToD2DColorF(GetTheme()->GetColor(IdCrAccent)));
                GetDC()->FillRectangle(rcMark, pBrush);
            }

            if (m_bDragging && m_idxDrag >= 0 && m_idxDrag < GetItemCount())
                PaintItem(m_idxDrag, m_vItem[m_idxDrag], ps.rcfClipInElem);

            DbgDrawFrame();
            EndPaint(ps);
        }
        return 0;

        case WM_SETCURSOR:
            if (m_bAllowReSize && m_bHitDivider)
            {
                SetCursor(LoadCursorW(nullptr, IDC_SIZEWE));
                return TRUE;
            }
            return FALSE;

        case WM_MOUSEMOVE:
        {
            HITTEST ht{ EagPoint(lParam) };
            auto idx = HitTest(ht);
            if (m_bDraggingDivider)
            {
                auto& e = m_vItem[m_idxDrag];
                const auto cxNew = ht.pt.x - e.x - m_xDragOffset;
                if (cxNew != e.cx)
                {
                    e.cx = cxNew;
                    UpdateTextLayout(m_idxDrag);
                    ReCalculateItemPosition(m_idxDrag + 1);

                    GetWindow().RdLockUpdate();
                    EvtWidthChanged(m_idxDrag);
                    Invalidate();
                    GetWindow().RdUnlockUpdate();
                }
            }
            else if (m_idxDrag >= 0 && m_bDraggable)
            {
                if (m_bDragging)
                {
                    const auto idxIns = DragCalculateInsertIndex(ht.pt.x);
                    if (idxIns != m_idxInsertMark)
                    {
                        m_idxInsertMark = idxIns;
                        DragReCalculateTargetPosition();
                        DragBeginAnimation();
                    }
                    auto& e = m_vItem[m_idxDrag];
                    e.xTarget = e.x = ht.pt.x - m_xDragOffset;
                    Invalidate();
                }
                else
                {
                    m_bDragging = TRUE;
                    m_xDragOffset = ht.pt.x - m_vItem[idx].x;
                    if (m_bDraggable)
                    {
                        m_idxInsertMark = idx;
                        DragReCalculateTargetPosition();
                    }
                    EvtBeginDrag(idx);
                }
            }
            else
            {
                if (m_bHitDivider != ht.bHitDivider)
                {
                    m_bHitDivider = ht.bHitDivider;
                    SetCursor(LoadCursorW(nullptr,
                        m_bHitDivider ? IDC_SIZEWE : IDC_ARROW));
                }
                if (m_idxHot != idx)
                {
                    std::swap(m_idxHot, idx);
                    GetWindow().RdLockUpdate();
                    if (m_idxHot >= 0)
                        InvalidateItem(m_idxHot);
                    if (idx >= 0)
                        InvalidateItem(idx);
                    GetWindow().RdUnlockUpdate();
                }
            }
        }
        return 0;
        case WM_MOUSELEAVE:
        {
            int idx{ -1 };
            std::swap(m_idxHot, idx);
            m_bHitDivider = FALSE;
            if (idx >= 0)
                InvalidateItem(idx);
        }
        return 0;
        case WM_LBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
        {
            HITTEST ht{ EagPoint(lParam) };
            const auto idx = HitTest(ht);
            if (idx < 0)
                return 0;

            SetCapture();
            if (!m_bAllowReSize)
                ht.bHitDivider = FALSE;
            m_idxDrag = idx;
            m_idxPressed = idx;
            m_bAnimating = FALSE;

            if (ht.bHitDivider && m_bAllowReSize)
            {
                m_bDraggingDivider = TRUE;
                m_xDragOffset = ht.pt.x - m_vItem[idx].x - m_vItem[idx].cx;
            }
            InvalidateItem(idx);
        }
        return 0;
        case WM_LBUTTONUP:
        {
            if (m_idxDrag >= 0)
            {
                DragEnd(TRUE);
                ReleaseCapture();
            }
        }
        return 0;
        case WM_CAPTURECHANGED:
            DragEnd(FALSE);
            break;

        case WM_SIZE:
            UpdateAllTextLayout();
            break;

        case WM_CREATE:
        {
            GetWindow().KctRegisterTimeLine(this);
            SetTheme(TmDefaultTheme(TmIsDarkMode()).Get());
            ReCalculateItemPosition();
        }
        break;
        case WM_DESTROY:
            GetWindow().KctUnregisterTimeLine(this);
            DeleteAllItems();// 应用程序可能需要清理自定义数据
            break;
        }
        return __super::OnEvent(uMsg, wParam, lParam);
    }

    void TlTick(int iMs) noexcept override
    {
        m_msLastDuration = (USHORT)iMs;
        const auto bRunning = m_ec.Tick((float)iMs, 200);

        if (!m_bDragging || m_idxDrag < 0 || m_idxDrag >= GetItemCount())
        {
            m_bAnimating = FALSE;
            return;
        }

        EckCounter(GetItemCount(), i)
        {
            if (i != m_idxDrag)
            {
                auto& e = m_vItem[i];
                e.x = e.xStart + (e.xTarget - e.xStart) * m_ec.K;
            }
        }
        if (!bRunning)
        {
            EckCounter(GetItemCount(), i)
            {
                if (i != m_idxDrag)
                    m_vItem[i].x = m_vItem[i].xTarget;
            }
            m_bAnimating = FALSE;
        }
        Invalidate(FALSE);
    }
    BOOL TlIsValid() noexcept override { return m_bAnimating; }
    int TlGetCurrentInterval() noexcept override { return (int)m_msLastDuration; }

    void EvtWidthChanged(int idx) noexcept
    {
        EVT_ITEM nm{ ENC_HD_WIDTHCHANGED };
        nm.idx = idx;
        SendNotify(&nm);
    }
    void EvtBeginDrag(int idx) noexcept
    {
        EVT_ITEM nm{ ENC_HD_BEGINDRAG };
        nm.idx = idx;
        SendNotify(&nm);
    }
    void EvtEndDrag(int idx) noexcept
    {
        EVT_ITEM nm{ ENC_HD_ENDDRAG };
        nm.idx = idx;
        SendNotify(&nm);
    }
    void EvtOrderChanged(int idxFrom, int idxTo) noexcept
    {
        EVT_ORDER nm{ ENC_HD_ORDERCHANGED };
        nm.idxFrom = idxFrom;
        nm.idxTo = idxTo;
        SendNotify(&nm);
    }
    void EvtDeleteItem(int idx) noexcept
    {
        EVT_ITEM nm{ ENC_HD_DELETEITEM };
        nm.idx = idx;
        SendNotify(&nm);
    }

    int InsertItem(
        int idx,
        std::wstring_view sv,
        float cx = 100.f,
        LPARAM lParam = 0) noexcept
    {
        if (idx < 0 || idx > GetItemCount())
            idx = GetItemCount();

        auto& e = *m_vItem.emplace(m_vItem.begin() + idx);
        e.rsText = sv;
        e.cx = cx;
        e.lParam = lParam;

        if (m_idxHot >= idx)
            ++m_idxHot;
        if (m_idxPressed >= idx)
            ++m_idxPressed;
        if (m_idxDrag >= idx)
            ++m_idxDrag;
        if (m_idxInsertMark >= idx)
            ++m_idxInsertMark;
        UpdateTextLayout(idx);
        ReCalculateItemPosition(idx - 1);
        return idx;
    }

    void DeleteItem(int idx) noexcept
    {
        if (m_idxHot == idx)
            m_idxHot = -1;
        else if (m_idxHot > idx)
            --m_idxHot;

        if (m_idxPressed == idx)
            m_idxPressed = -1;
        else if (m_idxPressed > idx)
            --m_idxPressed;

        if (m_idxInsertMark == idx)
            m_idxInsertMark = -1;
        else if (m_idxInsertMark > idx)
            --m_idxInsertMark;

        if (m_idxDrag == idx)
        {
            m_idxDrag = -1;
            m_idxInsertMark = -1;
            m_bDragging = FALSE;
            m_bDraggingDivider = FALSE;
            m_bAnimating = FALSE;
        }
        else if (m_idxDrag > idx)
            --m_idxDrag;

        EvtDeleteItem(idx);
        m_vItem.erase(m_vItem.begin() + idx);
        ReCalculateItemPosition(idx);
    }

    void DeleteAllItems() noexcept
    {
        EckCounter(GetItemCount(), i)
            EvtDeleteItem(i);
        m_vItem.clear();
        m_idxHot = -1;
        m_idxPressed = -1;
        m_idxDrag = -1;
        m_idxInsertMark = -1;
        m_bHitDivider = FALSE;
        m_bDragging = FALSE;
        m_bDraggingDivider = FALSE;
        m_bAnimating = FALSE;
    }

    void SetItemText(int idx, std::wstring_view sv) noexcept
    {
        m_vItem[idx].rsText = sv;
        UpdateTextLayout(idx);
    }
    EckInlineNdCe const CStringW& GetItemText(int idx) const noexcept { return m_vItem[idx].rsText; }

    void SetItemWidth(int idx, float cx) noexcept
    {
        m_vItem[idx].cx = cx;
        UpdateTextLayout(idx);
    }
    EckInlineNdCe float GetItemWidth(int idx) const noexcept { return m_vItem[idx].cx; }

    EckInlineCe void SetItemUserData(int idx, LPARAM lParam) noexcept { m_vItem[idx].lParam = lParam; }
    EckInlineNdCe LPARAM GetItemUserData(int idx) const noexcept { return m_vItem[idx].lParam; }

    EckInlineNdCe int GetItemCount() const noexcept { return (int)m_vItem.size(); }

    EckInlineCe void SetDraggable(BOOL b) noexcept { m_bDraggable = b; }
    EckInlineNdCe BOOL GetDraggable() const noexcept { return m_bDraggable; }

    EckInlineCe void SetAllowReSize(BOOL b) noexcept
    {
        m_bAllowReSize = b;
        if (!m_bAllowReSize)
            m_bHitDivider = FALSE;
    }
    EckInlineNdCe BOOL GetAllowReSize() const noexcept { return m_bAllowReSize; }

    void InvalidateCache(int idx = -1) noexcept
    {
        if (idx < 0)
            for (auto& e : m_vItem)
                e.pLayout.Clear();
        else
            m_vItem[idx].pLayout.Clear();
    }

    [[nodiscard]] int HitTest(HITTEST& ht) const noexcept
    {
        EckCounter(GetItemCount(), i)
        {
            const auto& e = m_vItem[i];
            if (ht.pt.x >= e.x &&
                ht.pt.x < e.x + e.cx + DividerHitTestWidthHalf)
            {
                if (m_bAllowReSize &&
                    ht.pt.x > e.x + e.cx - DividerHitTestWidthHalf)
                    ht.bHitDivider = TRUE;// 分隔条左界
                else
                    ht.bHitDivider = FALSE;
                return i;
            }
        }
        return -1;
    }

    void GetItemRect(int idx, Kw::Rect& rcItem) const noexcept
    {
        const auto& e = m_vItem[idx];
        rcItem.left = e.x;
        rcItem.right = e.x + e.cx;
        rcItem.top = 0;
        rcItem.bottom = GetHeight();
    }

    void InvalidateItem(int idx) noexcept
    {
        Kw::Rect rcItem;
        GetItemRect(idx, rcItem);
        Invalidate(rcItem);
    }

    float GetContentWidth() const noexcept
    {
        float cx{};
        for (const auto& e : m_vItem)
            cx += e.cx;
        return cx;
    }
};


class CTmHeader : public CThemeBase
{
public:
    TmResult Draw(
        CElement* pEle,
        const SimpleStyle* pStyle,
        UINT idPart,
        const D2D1_RECT_F& rc,
        _In_opt_ const D2D1_RECT_F* prcClip) noexcept override
    {
        if (idPart != IdPtNormal)
            return TmResult::NotSupport;
        const auto r = pEle->TmGenericDrawBackground(pStyle, rc);
        if (r != TmResult::Ok)
            return r;
        D2D1_RECT_F rcDivider{ rc };
        rcDivider.left = rcDivider.right - 1.f;
        const auto pBrush = pEle->GetWindow().CcSetBrushColor(
            ArgbToD2DColorF(pEle->GetTheme()->GetColor(IdCrBorder)));
        pEle->GetDC()->FillRectangle(rcDivider, pBrush);
        return TmResult::Ok;
    }
};
inline RcPtr<CThemeBase> CHeader::TmMakeDefaultTheme(BOOL bDark) noexcept
{
    return TmMakeTheme<CTmHeader>(bDark);
}


class CUiaHeader : public CUiaBase
{
    STDMETHODIMP GetPropertyValue(PROPERTYID idProp, VARIANT* pRetVal) override
    {
        if (idProp == UIA_ControlTypePropertyId)
        {
            pRetVal->vt = VT_I4;
            pRetVal->intVal = UIA_HeaderControlTypeId;
            return S_OK;
        }
        return CUiaBase::GetPropertyValue(idProp, pRetVal);
    }
};
inline HRESULT CHeader::EhUiaMakeInterface() noexcept
{
    const auto p = new CUiaHeader{};
    UiaSetInterface(p);
    p->Release();
    return S_OK;
}
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END