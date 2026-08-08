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
        int io;
    };

    struct EVT_ITEM : ELENMHDR
    {
        int idx;
    };

    struct EVT_ORDER : EVT_ITEM
    {
        int ioFrom;
        int ioTo;
    };

    const static inline UINT IdMeInsertMarkWidth = TmNextResourceId();

    constexpr static float DividerHitTestWidthHalf = 4.f;
    constexpr static float DefaultInsertMarkWidth = 3.f;
private:
    struct ITEM
    {
        CStringW rsText{};
        ComPtr<IDWriteTextLayout> pLayout{};
        float x{};      // 当前可视位置
        float xStart{}; // 拖动重排动画起始位置
        float xTarget{};// 拖动重排动画目标位置
        float cx{};
        float fSavedXOrCx{};
    };

    std::vector<ITEM> m_vItem{};
    std::vector<int> m_vOrder{};// io -> idx
    EasingCurve<Easing::FOutCubic> m_ec{};

    int m_idxHot{ -1 };
    int m_idxPressed{ -1 };
    int m_idxDrag{ -1 };
    int m_ioDrag{ -1 };
    int m_ioInsertMark{ -1 };

    float m_cxContent{};

    float m_xDragOffset{};
    USHORT m_msLastDuration{};

    BITBOOL m_bDraggable : 1{};
    BITBOOL m_bAllowReSize : 1{ TRUE };

    BITBOOL m_bHitDivider : 1{};
    BITBOOL m_bDragging : 1{};
    BITBOOL m_bDraggingDivider : 1{};
    BITBOOL m_bAnimating : 1{};

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

    void PaintItem(int idx, const ITEM& e, const D2D1_RECT_F& rcClipInClient) noexcept
    {
        D2D1_RECT_F rcItem{ e.x, 0.f, e.x + e.cx, GetHeight() };
        ElementToClient(rcItem);
        if (!IsRectsIntersect(rcItem, rcClipInClient))
            return;

        const float dOuter = GetTheme()->GetMetric(IdMePaddingOuter);
        const auto iSs = TmSimpleStyleFromItemState(idx);

        GetTheme()->Draw(this, &m_Style[iSs], IdPtNormal, rcItem, &rcClipInClient);

        if (e.pLayout)
        {
            GetDC()->DrawTextLayout(
                { rcItem.left + dOuter, rcItem.top + dOuter },
                e.pLayout.Get(),
                GetWindow().CcSetBrushColor(
                    GetTheme()->GetStyleColorD2D(&m_Style[iSs], SfFore)),
                DrawTextLayoutFlags);
        }
    }

    EckInlineNdCe auto& AtOrder(int io) noexcept { return m_vItem[m_vOrder[io]]; }
    EckInlineNdCe auto& AtOrder(int io) const noexcept { return m_vItem[m_vOrder[io]]; }

    // 为了方便起见，允许传入超出有效范围的序号，函数自动夹紧到[0, ItemCount)
    void ReCalculateItemPosition(int ioBegin = 0) noexcept
    {
        m_bAnimating = FALSE;
        if (ioBegin >= GetItemCount())
            return;
        if (ioBegin < 0)
            ioBegin = 0;
        float x;
        if (ioBegin)
            x = AtOrder(ioBegin - 1).x + AtOrder(ioBegin - 1).cx;
        else
            x = 0.f;
        for (int io = ioBegin; io < GetItemCount(); ++io)
        {
            auto& e = AtOrder(io);
            e.x = e.xStart = e.xTarget = x;
            x += e.cx;
        }
    }

    void DragReCalculateTargetPosition() noexcept
    {
        float x{};
        EckCounter(GetItemCount(), io)
        {
            if (io == m_ioInsertMark)
                x += m_vItem[m_idxDrag].cx;
            if (OrderToIndex(io) == m_idxDrag)
                continue;
            auto& e = AtOrder(io);
            e.xTarget = x;
            x += e.cx;
        }
        m_vItem[m_idxDrag].xTarget = m_vItem[m_idxDrag].x;
    }

    float DragCalculateInsertMarkPosition() const noexcept
    {
        float x{};
        EckCounter(m_ioInsertMark, io)
            x += AtOrder(io).cx;
        return x;
    }
    void DragCalculateInsertMarkRect(_Out_ Kw::Rect& rc) const noexcept
    {
        const auto cxMark = GetTheme()->GetMetric(
            IdMeInsertMarkWidth, DefaultInsertMarkWidth);
        const float xMark = DragCalculateInsertMarkPosition();
        rc = { xMark - cxMark / 2.f, 0.f, xMark + cxMark / 2.f, (float)GetHeight() };
    }
    void DragCalculateInsertMarkRect(_Out_ D2D1_RECT_F& rc) const noexcept
    {
        DragCalculateInsertMarkRect(*(Kw::Rect*)&rc);
    }

    void DragMoveItem(int io, int ioNew) noexcept
    {
        const auto idx = m_vOrder[io];
        if (io < ioNew)
            for (int i = io; i < ioNew; ++i)
                m_vOrder[i] = m_vOrder[i + 1];
        else
            for (int i = io; i > ioNew; --i)
                m_vOrder[i] = m_vOrder[i - 1];
        m_vOrder[ioNew] = idx;
        ReCalculateItemPosition(std::min(io, ioNew));
    }

    void DragEnd(BOOL bCommitOrder) noexcept
    {
        if (m_idxDrag < 0)
            return;

        const auto idxDragOld = m_idxDrag;
        const auto ioDragOld = m_ioDrag;
        const auto idxPressedOld = m_idxPressed;
        const auto bWasDragging = m_bDragging;
        const auto bWasDraggingDivider = m_bDraggingDivider;
        const auto bSendEndDrag = bWasDragging || bWasDraggingDivider;

        int ioTo = ioDragOld;
        if (bWasDragging && bCommitOrder)
        {
            ioTo = m_ioInsertMark;
            if (ioTo > ioDragOld)
                --ioTo;
        }

        m_idxPressed = -1;
        m_idxDrag = -1;
        m_ioDrag = -1;
        m_ioInsertMark = -1;
        m_idxHot = -1;
        m_bHitDivider = FALSE;
        m_bDragging = FALSE;
        m_bDraggingDivider = FALSE;
        m_bAnimating = FALSE;

        GetWindow().RdLockUpdate();
        if (ioTo != ioDragOld)
        {
            Kw::Rect rc, rc1;
            InternalGetItemRect(idxDragOld, rc);

            DragMoveItem(ioDragOld, ioTo);
            EvtOrderChanged(idxDragOld, ioDragOld, ioTo);

            InternalGetItemRect(OrderToIndex(ioDragOld), rc1);
            UnionRect(rc, rc, rc1);
            InternalGetItemRect(OrderToIndex(ioTo), rc1);
            UnionRect(rc, rc, rc1);

            Invalidate(rc);
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
        GetWindow().RdUnlockUpdate();
    }

    int DragCalculateInsertOrder(float xRef) const noexcept
    {
        const auto cItem = GetItemCount();
        if (cItem <= 1)
            return 0;
        int ioBest{};
        float dBest{ FLT_MAX }, dLast{ FLT_MAX }, d;
        float x{};
        EckCounter(cItem, io)
        {
            d = fabsf(xRef - x);
            if (d < dBest)
            {
                dBest = d;
                ioBest = io;
            }
            x += AtOrder(io).cx;
            if (d > dLast)
                goto End;
            dLast = d;
        }
        d = fabsf(xRef - x);
        if (d < dBest)
            ioBest = cItem;
    End:
        return ioBest;
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

    void DragSavePositionOrWidth(BOOL bPosOrWidth) noexcept
    {
        for (auto& e : m_vItem)
            e.fSavedXOrCx = bPosOrWidth ? e.x : e.cx;
    }

    void InternalGetItemRect(int idx, _Out_ Kw::Rect& rcItem) const noexcept
    {
        const auto& e = m_vItem[idx];
        rcItem.left = e.x;
        rcItem.right = e.x + e.cx;
        rcItem.top = 0;
        rcItem.bottom = GetHeight();
    }
public:
    static RcPtr<CTheme> TmMakeDefaultTheme(BOOL bDark) noexcept;
    static RcPtr<CTheme> TmDefaultTheme(BOOL bDark) noexcept
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
            EckCounter(GetItemCount(), i)
            {
                if (m_bDragging && i == m_idxDrag)
                    continue;
                auto& e = m_vItem[i];
                if (e.x > ps.rcClipInEle.right ||
                    e.x + e.cx < ps.rcClipInEle.left)
                    continue;
                PaintItem(i, e, ps.rcClip);
            }

            if (m_bDragging && m_bDraggable && m_ioInsertMark >= 0)
            {
                D2D1_RECT_F rcMark;
                DragCalculateInsertMarkRect(rcMark);
                ElementToClient(rcMark);
                const auto pBrush = GetWindow().CcSetBrushColor(
                    GetTheme()->GetColorD2D(IdCrAccent));
                GetDC()->FillRectangle(rcMark, pBrush);
            }

            if (m_bDragging && m_idxDrag >= 0 && m_idxDrag < GetItemCount())
                PaintItem(m_idxDrag, m_vItem[m_idxDrag], ps.rcClip);

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
                    m_cxContent += (cxNew - e.cx);
                    e.cx = cxNew;
                    UpdateTextLayout(m_idxDrag);
                    ReCalculateItemPosition(m_ioDrag + 1);

                    GetWindow().RdLockUpdate();
                    EvtWidthChanged(m_idxDrag);
                    Invalidate(Kw::Rect{ e.x, 0, GetWidth(), GetHeight() });
                    GetWindow().RdUnlockUpdate();
                }
            }
            else if (m_idxDrag >= 0 && m_bDraggable)
            {
                if (m_bDragging)
                {
                    GetWindow().RdLockUpdate();
                    const auto ioIns = DragCalculateInsertOrder(ht.pt.x);
                    if (ioIns != m_ioInsertMark)
                    {
                        Kw::Rect rcMark;
                        DragCalculateInsertMarkRect(rcMark);
                        Invalidate(rcMark);
                        m_ioInsertMark = ioIns;
                        DragReCalculateTargetPosition();
                        DragBeginAnimation();
                        DragCalculateInsertMarkRect(rcMark);
                        Invalidate(rcMark);
                    }
                    auto& e = m_vItem[m_idxDrag];
                    const auto xOld = e.x;
                    e.xTarget = e.x = ht.pt.x - m_xDragOffset;
                    Invalidate(
                        Kw::Rect{
                            std::min(e.x, xOld),
                            0,
                            std::max(e.x, xOld) + e.cx,
                            GetHeight()
                        });
                    GetWindow().RdUnlockUpdate();
                }
                else
                {
                    if (m_bDraggable)
                    {
                        m_bDragging = TRUE;
                        m_xDragOffset = ht.pt.x - m_vItem[idx].x;
                        m_ioInsertMark = ht.io;
                        DragSavePositionOrWidth(TRUE);
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
            m_idxDrag = idx;
            m_ioDrag = ht.io;
            m_idxPressed = idx;
            m_bAnimating = FALSE;

            if (ht.bHitDivider && m_bAllowReSize &&
                uMsg == WM_LBUTTONDOWN)
            {
                m_bDraggingDivider = TRUE;
                m_xDragOffset = ht.pt.x - m_vItem[idx].x - m_vItem[idx].cx;
                DragSavePositionOrWidth(FALSE);
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

        case WM_SETFONT:
            InvalidateCache();
            break;

        case WM_SIZE:
            UpdateAllTextLayout();
            break;

        case WM_STYLECHANGED:
            TmAutoSwitchTheme(this, wParam);
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
            DeleteAllItems();
            break;
        }
        return __super::OnEvent(uMsg, wParam, lParam);
    }

    void TlTick(int iMs) noexcept override
    {
        m_msLastDuration = (USHORT)iMs;
        if (!m_bDragging || m_idxDrag < 0 || m_idxDrag >= GetItemCount())
        {
            m_bAnimating = FALSE;
            return;
        }

        m_bAnimating = m_ec.Tick((float)iMs, 200);
        EckCounter(GetItemCount(), i)
        {
            auto& e = m_vItem[i];
            if (FloatEqual(e.x, e.xTarget))
            {
                e.x = e.xTarget;
                continue;
            }
            if (i != m_idxDrag)
            {
                const auto xOld = e.x;
                if (m_bAnimating)
                    e.x = e.xStart + (e.xTarget - e.xStart) * m_ec.K;
                else
                    e.x = e.xTarget;
                Invalidate(
                    Kw::Rect{
                        std::min(e.x, xOld),
                        0,
                        std::max(e.x, xOld) + e.cx,
                        GetHeight()
                    }, FALSE);
            }
        }
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
    void EvtOrderChanged(int idx, int ioFrom, int ioTo) noexcept
    {
        EVT_ORDER nm{ ENC_HD_ORDERCHANGED };
        nm.idx = idx;
        nm.ioFrom = ioFrom;
        nm.ioTo = ioTo;
        SendNotify(&nm);
    }
    void EvtDeleteItem(int idx) noexcept
    {
        EVT_ITEM nm{ ENC_HD_DELETEITEM };
        nm.idx = idx;
        SendNotify(&nm);
    }

    int InsertItem(int idx, std::wstring_view sv, float cx = 100.f) noexcept
    {
        DragEnd(FALSE);
        m_bAnimating = FALSE;

        if (idx < 0 || idx > GetItemCount())
            idx = GetItemCount();

        for (auto& e : m_vOrder)
        {
            if (e >= idx)
                ++e;
        }
        m_vOrder.emplace(m_vOrder.begin() + idx, idx);

        auto& e = *m_vItem.emplace(m_vItem.begin() + idx);
        e.rsText = sv;
        e.cx = cx;

        m_cxContent += cx;

        if (m_idxHot >= idx)
            ++m_idxHot;
        if (m_idxPressed >= idx)
            ++m_idxPressed;
        UpdateTextLayout(idx);
        ReCalculateItemPosition(idx - 1);
        return idx;
    }

    void DeleteItem(int idx) noexcept
    {
        DragEnd(FALSE);
        m_bAnimating = FALSE;

        if (m_idxHot == idx)
            m_idxHot = -1;
        else if (m_idxHot > idx)
            --m_idxHot;

        if (m_idxPressed == idx)
            m_idxPressed = -1;
        else if (m_idxPressed > idx)
            --m_idxPressed;

        m_cxContent -= m_vItem[idx].cx;
        int ioChangedBegin{ -1 };
        for (auto it = m_vOrder.begin(); it != m_vOrder.end(); )
        {
            if ((*it) == idx)
            {
                if (ioChangedBegin < 0)
                    ioChangedBegin = int(it - m_vOrder.begin());
                it = m_vOrder.erase(it);
            }
            else
            {
                if ((*it) > idx)
                {
                    --(*it);
                    if (ioChangedBegin < 0)
                        ioChangedBegin = int(it - m_vOrder.begin());
                }
                ++it;
            }
        }

        EvtDeleteItem(idx);
        m_vItem.erase(m_vItem.begin() + idx);
        ReCalculateItemPosition(ioChangedBegin);
    }

    void DeleteAllItems() noexcept
    {
        EckCounter(GetItemCount(), i)
            EvtDeleteItem(i);
        m_vItem.clear();
        m_cxContent = 0.f;
        m_idxHot = -1;
        m_idxPressed = -1;
        m_idxDrag = -1;
        m_ioDrag = -1;
        m_ioInsertMark = -1;
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
        m_cxContent += (cx - m_vItem[idx].cx);
        m_vItem[idx].cx = cx;
        UpdateTextLayout(idx);
    }
    EckInlineNdCe float GetItemWidth(int idx) const noexcept { return m_vItem[idx].cx; }

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

    [[nodiscard]] int HitTest(_Inout_ HITTEST& ht) const noexcept
    {
        EckCounter(GetItemCount(), io)
        {
            const auto& e = AtOrder(io);
            if (ht.pt.x >= e.x &&
                ht.pt.x < e.x + e.cx + DividerHitTestWidthHalf)
            {
                if (m_bAllowReSize &&
                    ht.pt.x > e.x + e.cx - DividerHitTestWidthHalf)
                    ht.bHitDivider = TRUE;// 分隔条左界
                else
                    ht.bHitDivider = FALSE;
                ht.io = io;
                return OrderToIndex(io);
            }
        }
        ht.io = -1;
        ht.bHitDivider = FALSE;
        return -1;
    }

    void GetItemRect(int idx, _Out_ Kw::Rect& rcItem) const noexcept
    {
        if (m_bDraggingDivider && m_idxDrag == idx)
        {
            const auto& e = m_vItem[idx];
            rcItem.left = e.x;
            rcItem.top = 0;
            rcItem.right = rcItem.left + e.fSavedXOrCx;
            rcItem.bottom = GetHeight();
        }
        else if (m_bDragging)
        {
            const auto& e = m_vItem[idx];
            rcItem.left = e.fSavedXOrCx;
            rcItem.top = 0;
            rcItem.right = rcItem.left + e.cx;
            rcItem.bottom = GetHeight();
        }
        else
            InternalGetItemRect(idx, rcItem);
    }

    void InvalidateItem(int idx) noexcept
    {
        Kw::Rect rcItem;
        GetItemRect(idx, rcItem);
        Invalidate(rcItem);
    }

    float GetContentWidth() const noexcept
    {
        return m_cxContent;
    }

    EckInlineNdCe int OrderToIndex(int io) const noexcept { return m_vOrder[io]; }

    EckInlineNdCe int IndexToOrder(int idx) const noexcept
    {
        EckCounter(GetItemCount(), io)
        {
            if (m_vOrder[io] == idx)
                return io;
        }
        return -1;
    }
};


class CTmHeader : public CTheme
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
            pEle->GetTheme()->GetColorD2D(IdCrBorder));
        pEle->GetDC()->FillRectangle(rcDivider, pBrush);
        return TmResult::Ok;
    }
};
inline RcPtr<CTheme> CHeader::TmMakeDefaultTheme(BOOL bDark) noexcept
{
    const auto pTheme = TmMakeTheme<CTmHeader>(bDark);
    const auto pmc = TmsMakeMetricCollection();
    pmc->Set(IdMeInsertMarkWidth, DefaultInsertMarkWidth);
    pTheme->SetMetricCollection(pmc.Get());
    return pTheme;
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