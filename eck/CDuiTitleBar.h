#pragma once
#include "DuiBase.h"
#include "GraphicsHelper.h"
#include "CUxDwmWindowTheme.h"
#include "RefPtr.h"
#include "CMenu.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
class CTitleBar : public CElement
{
private:
    RefPtr<CUxDwmWindowTheme> m_pUdwTheme{};
    ComPtr<ID2D1Bitmap1> m_pAtlas{};

    float m_cxClose{};
    float m_cxMax{};
    float m_cxMin{};
    float m_cyBtn{};

    UdwPart m_eHotPart{ UdwPart::Invalid };
    UdwPart m_ePressedPart{ UdwPart::Invalid };

    BYTE m_eInterMode{ (BYTE)D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR };
    BYTE m_eInterModeIcon{ (BYTE)D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR };

    BITBOOL m_bCloseButton : 1{ TRUE };
    BITBOOL m_bMaxButton : 1{ TRUE };
    BITBOOL m_bMinButton : 1{ TRUE };
    BITBOOL m_bCloseButtonEnabled : 1{};
    BITBOOL m_bMaxButtonEnabled : 1{};
    BITBOOL m_bMinButtonEnabled : 1{};
    BITBOOL m_bSyncToStyle : 1{ TRUE };

    BITBOOL m_bMaximized : 1{};


    UdwState GetPartState(UdwPart ePart) const noexcept
    {
        switch (ePart)
        {
        case UdwPart::Close:
            if (!m_bCloseButtonEnabled)
                return UdwState::Disabled;
            break;
        case UdwPart::Max:
            if (!m_bMaxButtonEnabled)
                return UdwState::Disabled;
            break;
        case UdwPart::Min:
            if (!m_bMinButtonEnabled)
                return UdwState::Disabled;
            break;
        }

        if (m_ePressedPart == ePart)
            return UdwState::Pressed;
        else if (m_eHotPart == ePart)
            return UdwState::Hot;
        return UdwState::Normal;
    }

    LRESULT OnWindowMessage(CWindow* pWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, Slot&) noexcept
    {
        switch (uMsg)
        {
        case WM_SIZE:
        {
            const auto bMax = (wParam == SIZE_MAXIMIZED);
            if (m_bMaximized != bMax)
            {
                m_bMaximized = bMax;
                InvalidatePart(UdwPart::Max);
            }
        }
        break;
        case WM_STYLECHANGED:
        {
            const auto* const pss = (STYLESTRUCT*)lParam;
            BOOL bUpdate{};
            if ((pss->styleOld ^ pss->styleNew) == WS_MINIMIZEBOX)
            {
                m_bMinButton = !!(pss->styleNew & WS_MINIMIZEBOX);
                bUpdate = TRUE;
            }
            if ((pss->styleOld ^ pss->styleNew) == WS_MAXIMIZEBOX)
            {
                m_bMaxButton = !!(pss->styleNew & WS_MAXIMIZEBOX);
                bUpdate = TRUE;
            }

            if (bUpdate)
                InvalidateButton();
        }
        break;
        }
        return 0;
    }

    static EckInlineNdCe BOOL IsPartValid(UdwPart ePart) noexcept
    {
        return ePart != UdwPart::Invalid && ePart != UdwPart::Extra;
    }

    float SnapToPixel(float f, int iDpi) noexcept
    {
        return roundf(f);
        return DpiScale(roundf(DpiScale(f, 96, iDpi)), iDpi, 96);
    }
    template<CcpRect T>
    void SnapToPixel(T& rc, int iDpi) noexcept
    {
        if constexpr (RectTraits<T>::IsRcwh)
        {
            rc.x = SnapToPixel(rc.x, iDpi);
            rc.y = SnapToPixel(rc.y, iDpi);
        }
        else
        {
            const auto cx = rc.right - rc.left;
            const auto cy = rc.bottom - rc.top;
            rc.left = SnapToPixel(rc.left, iDpi);
            rc.right = SnapToPixel(rc.right, iDpi);
            rc.top = SnapToPixel(rc.top, iDpi);
            rc.bottom = SnapToPixel(rc.bottom, iDpi);
        }
    }

    BOOL PaintButton(UdwPart ePart, const D2D1_RECT_F& rcClip) noexcept
    {
        D2D1_RECT_F rcDst;
        GetPartRect(ePart, rcDst);
        ElementToClient(rcDst);
        if (!IsRectsIntersect(rcDst, rcClip))
            return FALSE;

        LogicalToPixel(rcDst);
        SnapToPixel(rcDst, GetWindow().GetUserDpi());

        if (ePart == UdwPart::Max || ePart == UdwPart::Restore)
            ePart = (m_bMaximized ? UdwPart::Restore : UdwPart::Max);

        RECT rc, rcBkg;
        UDW_EXTRA Extra;
        if (m_pUdwTheme->GetPartRect(
            rc, rcBkg,
            ePart,
            GetPartState(ePart),
            TmIsDarkMode(),
            TRUE,
            GetWindow().GetUserDpi(),
            &Extra))
        {
            DrawImageFromGrid(
                GetDC(), m_pAtlas.Get(),
                rcDst,
                MakeD2DRectF(rcBkg),
                MarginsToD2DRectF(Extra.pBkg->mgSizing),
                GetInterpolationMode());

            auto rcIcon = MakeD2DRectF(rc);
            CenterRect(rcIcon, rcDst);
            SnapToPixel(rcIcon, GetWindow().GetUserDpi());

            GetDC()->DrawBitmap(m_pAtlas.Get(), rcIcon, 1.f,
                GetIconInterpolationMode(), MakeD2DRectF(rc));
            return TRUE;
        }
        return FALSE;
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

            GetDC()->SetDpi(96, 96);
            GetDC()->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
            if (m_bCloseButton)
                PaintButton(UdwPart::Close, ps.rcfClip);
            if (m_bMaxButton)
                PaintButton(UdwPart::Max, ps.rcfClip);
            if (m_bMinButton)
                PaintButton(UdwPart::Min, ps.rcfClip);
            GetDC()->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
            GetDC()->SetDpi(144, 144);

            DbgDrawFrame();
            EndPaint(ps);
        }
        return 0;

        case WM_NCHITTEST:
        {
            auto pt{ EagPoint(lParam) };
            ClientToElement(pt);
            const UdwPart ePart = HitTest(pt);
            switch (ePart)
            {
            case UdwPart::Close:
                return HTCLOSE;
            case UdwPart::Max:
                return HTMAXBUTTON;
            case UdwPart::Min:
                return HTMINBUTTON;
            case UdwPart::Extra:
            {
                const auto y = DpiScale(
                    pt.y,
                    GetWindow().GetUserDpi(),
                    GetWindow().GetWindowDpi());
                if (!m_bMaximized &&
                    y < DaGetSystemMetrics(SM_CYFRAME, GetWindow().GetWindowDpi()))
                    return HTTOP;
                else
                    return HTCAPTION;
            }
            ECK_UNREACHABLE;
            }
        }
        return HTTRANSPARENT;

        case WM_NCLBUTTONDOWN:
        {
            POINT ptInScr ECK_GET_PT_LPARAM(lParam);
            ScreenToClient(GetWindow().Handle, &ptInScr);
            Kw::Vec2 pt{ (float)ptInScr.x, (float)ptInScr.y };
            PixelToLogical(pt);
            ClientToElement(pt);

            auto ePart = HitTest(pt);
            if (ePart != m_ePressedPart)
            {
                std::swap(ePart, m_ePressedPart);
                GetWindow().RdLockUpdate();
                InvalidatePart(ePart);
                InvalidatePart(m_ePressedPart);
                GetWindow().RdUnlockUpdate();
            }
        }
        return 0;

        case WM_NCLBUTTONUP:
        {
            if (IsPartValid(m_ePressedPart))
            {
                POINT ptInScr ECK_GET_PT_LPARAM(lParam);
                ScreenToClient(GetWindow().Handle, &ptInScr);
                Kw::Vec2 pt{ (float)ptInScr.x, (float)ptInScr.y };
                PixelToLogical(pt);
                ClientToElement(pt);

                const auto ePart = HitTest(pt);
                if (m_ePressedPart == ePart)
                    switch (ePart)
                    {
                    case UdwPart::Close:
                        GetWindow().PostMessageW(WM_SYSCOMMAND, SC_CLOSE, 0);
                        break;
                    case UdwPart::Max:
                        if (m_bMaximized)
                            GetWindow().PostMessageW(WM_SYSCOMMAND, SC_RESTORE, 0);
                        else
                            GetWindow().PostMessageW(WM_SYSCOMMAND, SC_MAXIMIZE, 0);
                        break;
                    case UdwPart::Min:
                        GetWindow().PostMessageW(WM_SYSCOMMAND, SC_MINIMIZE, 0);
                        break;
                    }
                const auto eOld = m_ePressedPart;
                m_ePressedPart = UdwPart::Invalid;
                if (IsPartValid(eOld))
                    InvalidatePart(eOld);
            }
            else
                m_ePressedPart = UdwPart::Invalid;
        }
        return 0;

        case WM_NCMOUSEMOVE:
        {
            POINT ptInScr ECK_GET_PT_LPARAM(lParam);
            ScreenToClient(GetWindow().Handle, &ptInScr);
            Kw::Vec2 pt{ (float)ptInScr.x, (float)ptInScr.y };
            PixelToLogical(pt);
            ClientToElement(pt);

            auto ePart = HitTest(pt);
            if (ePart != m_eHotPart)
            {
                std::swap(ePart, m_eHotPart);
                GetWindow().RdLockUpdate();
                InvalidatePart(ePart);
                InvalidatePart(m_eHotPart);
                GetWindow().RdUnlockUpdate();
            }
        }
        return 0;

        case WM_MOUSELEAVE:
        {
            if (m_eHotPart != UdwPart::Invalid)
            {
                const auto eOld = m_eHotPart;
                m_eHotPart = UdwPart::Invalid;
                InvalidatePart(eOld);
            }
        }
        return 0;

        case WM_CAPTURECHANGED:
        {
            if (m_ePressedPart != UdwPart::Invalid)
            {
                const auto eOld = m_ePressedPart;
                m_ePressedPart = UdwPart::Invalid;
                InvalidatePart(eOld);
            }
        }
        return 0;

        case WM_CREATE:
        {
            GetWindow().GetEventChain().Connect(this, &CTitleBar::OnWindowMessage, MHI_DUI_TITLEBAR);
            m_bMaximized = IsZoomed(GetWindow().Handle);
            UpdateMetrics();
        }
        break;

        case WM_DESTROY:
            GetWindow().GetEventChain().Disconnect(MHI_DUI_TITLEBAR);
            m_pAtlas.Clear();
            break;
        }
        return __super::OnEvent(uMsg, wParam, lParam);
    }

    void UpdateMetrics()
    {
        if (g_NtVersion.uBuild >= WINVER_11_21H2)
        {
            const auto iWndDpi = GetWindow().GetWindowDpi();
            // -- 计算高度
            m_cyBtn = (float)DaGetSystemMetrics(SM_CYSIZE, iWndDpi);
            m_cyBtn += float(
                DaGetSystemMetrics(SM_CXPADDEDBORDER, iWndDpi) +
                DaGetSystemMetrics(SM_CYFRAME, iWndDpi) +
                DaGetSystemMetrics(SM_CYBORDER, iWndDpi));
            m_cyBtn = m_cyBtn * 96.f / iWndDpi;
            // 高度为SM_CYSIZE + 通常模式下的客户区上边距

            // -- 计算宽度
            int nSys = DaGetSystemMetrics(SM_CYSIZE, iWndDpi);
            nSys = (int)floorf(nSys * 0.95454544f + 0.5f);
            // 对于标准的4个按钮（关闭、最大化、最小化、帮助）
            // 在两边的使用2.2272727，中间的使用2.1818182，若只有关闭按钮，使用1.6363636
            m_cxClose = m_cxMin = floorf(nSys * 2.2272727f + 0.5f)
                * 96.f / iWndDpi;
            m_cxMax = floorf(nSys * 2.1818182f + 0.5f)
                * 96.f / iWndDpi;
            return;
        }

        if (g_NtVersion.uMajor == 6 && (g_NtVersion.uMinor == 2 || g_NtVersion.uMinor == 3))
        {
            m_cxClose = 46;
            m_cxMax = m_cxMin = m_cxClose * 80 / 150;
            m_cyBtn = 21;
        }
        else
        {
            m_cxClose = 46;
            m_cxMax = m_cxMin = m_cxClose;
            m_cyBtn = 31;
        }
    }

    // 函数永不返回UdwPart::Restore，使用UdwPart::Max代替
    UdwPart HitTest(Kw::Vec2 ptInEle) const noexcept
    {
        if (ptInEle.x < 0 || ptInEle.x > GetWidth() ||
            ptInEle.y < 0 || ptInEle.y > std::min(GetHeight(), m_cyBtn))
            return UdwPart::Invalid;
        Kw::Rect rc;
        if (m_bCloseButton)
        {
            GetPartRect(UdwPart::Close, rc);
            if (PointInRect(rc, ptInEle))
                return UdwPart::Close;
        }
        if (m_bMaxButton)
        {
            GetPartRect(UdwPart::Max, rc);
            if (PointInRect(rc, ptInEle))
                return UdwPart::Max;
        }
        if (m_bMinButton)
        {
            GetPartRect(UdwPart::Min, rc);
            if (PointInRect(rc, ptInEle))
                return UdwPart::Min;
        }
        GetPartRect(UdwPart::Extra, rc);
        if (PointInRect(rc, ptInEle))
            return UdwPart::Extra;
        return UdwPart::Invalid;
    }

    float GetButtonGap() const noexcept
    {
        return DpiScale(1.f, 96, GetWindow().GetUserDpi());
    }

    void GetPartRect(UdwPart ePart, _Out_ Kw::Rect& rc) const noexcept
    {
        const auto dGap = GetButtonGap();
        rc.top = 0;
        rc.bottom = m_cyBtn;
        switch (ePart)
        {
        case UdwPart::Close:
            rc.left = GetWidth() - m_cxClose;
            rc.right = GetWidth();
            break;
        case UdwPart::Max:
        case UdwPart::Restore:
            rc.left = GetWidth() - m_cxClose - dGap - m_cxMax;
            rc.right = rc.left + m_cxMax;
            break;
        case UdwPart::Min:
            rc.left = GetWidth() - m_cxClose - dGap - m_cxMax - dGap - m_cxMin;
            rc.right = rc.left + m_cxMin;
            break;
        case UdwPart::Extra:
            rc.left = 0;
            rc.right = GetWidth() - m_cxClose - dGap - m_cxMax - dGap - m_cxMin;
            break;
        default:
            rc.left = rc.right = rc.bottom = 0.f;
            break;
        }
    }
    void GetPartRect(UdwPart ePart, _Out_ D2D1_RECT_F& rc) const noexcept
    {
        GetPartRect(ePart, *(Kw::Rect*)&rc);
    }

    void InvalidateButton(BOOL bUpdateNow = TRUE) noexcept
    {
        const auto cxBtn = m_cxClose + m_cxMax + m_cxMin;
        D2D1_RECT_F rc{ GetWidth() - cxBtn, 0, GetWidth(), m_cyBtn };
        Invalidate(rc, bUpdateNow);
    }

    void InvalidatePart(UdwPart ePart, BOOL bUpdateNow = TRUE) noexcept
    {
        if (!IsPartValid(ePart))
            return;
        Kw::Rect rc;
        GetPartRect(ePart, rc);
        Invalidate(rc, bUpdateNow);
    }

    void SynchronizeToSystemMenu() noexcept
    {
        CMenu Menu{ GetSystemMenu(GetWindow().Handle, FALSE) };
        constexpr UINT DisableFlags = MF_GRAYED | MF_DISABLED;
        m_bCloseButtonEnabled = !(Menu.GetItemState(SC_CLOSE, FALSE) & DisableFlags);
        m_bMaxButtonEnabled =
            !(Menu.GetItemState(SC_MAXIMIZE, FALSE) & DisableFlags) &&
            !(Menu.GetItemState(SC_RESTORE, FALSE) & DisableFlags);
        m_bMinButtonEnabled = !(Menu.GetItemState(SC_MINIMIZE, FALSE) & DisableFlags);
        (void)Menu.Detach();
    }

    EckInlineCe void SetInterpolationMode(D2D1_INTERPOLATION_MODE e) noexcept { m_eInterMode = (BYTE)e; }
    EckInlineNdCe D2D1_INTERPOLATION_MODE GetInterpolationMode() const noexcept { return (D2D1_INTERPOLATION_MODE)m_eInterMode; }
    EckInlineCe void SetIconInterpolationMode(D2D1_INTERPOLATION_MODE e) noexcept { m_eInterModeIcon = (BYTE)e; }
    EckInlineNdCe D2D1_INTERPOLATION_MODE GetIconInterpolationMode() const noexcept { return (D2D1_INTERPOLATION_MODE)m_eInterModeIcon; }

    EckInline void SetUxDwmWindowTheme(RefPtr<CUxDwmWindowTheme> p) noexcept { m_pUdwTheme = std::move(p); }
    EckInlineNdCe auto& GetUxDwmWindowTheme() const noexcept { return m_pUdwTheme; }
    EckInline void SetThemeAtlas(ComPtr<ID2D1Bitmap1> p) noexcept { m_pAtlas = std::move(p); }
    EckInlineNdCe auto& GetThemeAtlas() const noexcept { return m_pAtlas; }
};


class CTmTitleBar : public CThemeBase
{
public:
    TmResult Draw(
        CElement* pEle,
        const SimpleStyle* pStyle,
        UINT idPart,
        const D2D1_RECT_F& rc,
        _In_opt_ const D2D1_RECT_F* prcClip) noexcept override
    {
        return TmResult::NotSupport;
    }
};
inline RcPtr<CThemeBase> CTitleBar::TmMakeDefaultTheme(BOOL bDark) noexcept
{
    return TmMakeTheme<CTmTitleBar>(bDark);
}


class CUiaTitleBar : public CUiaBase
{
    STDMETHODIMP GetPropertyValue(PROPERTYID idProp, VARIANT* pRetVal) override
    {
        if (idProp == UIA_ControlTypePropertyId)
        {
            pRetVal->vt = VT_I4;
            pRetVal->intVal = UIA_TitleBarControlTypeId;
            return S_OK;
        }
        return CUiaBase::GetPropertyValue(idProp, pRetVal);
    }
};
inline HRESULT CTitleBar::EhUiaMakeInterface() noexcept
{
    const auto p = new CUiaTitleBar{};
    UiaSetInterface(p);
    p->Release();
    return S_OK;
}
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END