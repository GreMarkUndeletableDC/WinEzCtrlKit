#pragma once
#include "DuiBase.h"
#include "GraphicsHelper.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
class CLabel : public CElement
{
private:
    ComPtr<IDWriteTextLayout> m_pLayout{};
    ComPtr<ID2D1LinearGradientBrush> m_pBrushFade{};
    CBitmap m_BitmapIcon{};
    CBitmap m_BitmapBack{};

    float m_cxFade{ 40.f };

    BITBOOL m_bOnlyBitmap : 1{};
    BITBOOL m_bFade : 1{};
    BITBOOL m_bAutoScale : 1{ TRUE };

    BYTE m_eInterMode{ D2D1_INTERPOLATION_MODE_LINEAR };

    ImageMode m_eBkImgMode{ ImageMode::TopLeft };

    float m_kIconOpacity{ 1.f };
    float m_kBackOpacity{ 1.f };

    Kw::Vec2 CalculateIconSize(float cxText) const noexcept
    {
        EckAssert(m_BitmapIcon.Get());
        const auto rc = m_BitmapIcon.GetActualSourceRect();
        const auto cx = rc.right - rc.left;
        const auto cy = rc.bottom - rc.top;
        if (m_bAutoScale)
        {
            const auto dOuter = GetTheme()->GetMetric(IdMePaddingOuter);
            const auto cyNew = GetHeight() - dOuter * 2;
            const auto cxNew = GetWidth() - dOuter - cxText;
            if (cxNew > cyNew)
                return { cyNew * cx / cy, cyNew };
            else
                return { cxNew, cxNew * cy / cx };
        }
        return { cx, cy };
    }

    void UpdateTextLayout() noexcept
    {
        if (GetText().IsEmpty())
        {
            m_pLayout.Clear();
            return;
        }
        float cx;
        if (m_BitmapIcon.Get())
        {
            const auto rc = m_BitmapIcon.GetActualSourceRect();
            cx = GetWidth() - (rc.right - rc.left) -
                GetTheme()->GetMetric(IdMePaddingInner);
        }
        else
            cx = GetWidth();
        g_pDwFactory->CreateTextLayout(
            GetText().Data(), GetText().Size(),
            GetTextFormat().Get(), cx, GetHeight(), m_pLayout.AtClear());
    }

    void UpdateFadeBrush() noexcept
    {
        const D2D1_GRADIENT_STOP Stop[]
        {
            { 0.f, GetTheme()->GetColorD2D(IdCrFore) },
            { 1.f }
        };
        ComPtr<ID2D1GradientStopCollection> pStopCollection;
        GetDC()->CreateGradientStopCollection(Stop, 2, &pStopCollection);
        const D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES Prop{ {}, { m_cxFade, 0.f } };
        GetDC()->CreateLinearGradientBrush(Prop, pStopCollection.Get(), m_pBrushFade.AtClear());
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

            if (m_BitmapBack.Get())
            {
                DrawBackgroundImage(
                    GetDC(), m_BitmapBack.Get(), m_eBkImgMode,
                    GetRectInClientD2D(),
                    m_BitmapBack.GetSourceRect(),
                    m_kBackOpacity,
                    (D2D1_INTERPOLATION_MODE)m_eInterMode);
            }

            if (m_bOnlyBitmap || !m_pLayout)
            {
                if (m_BitmapIcon.Get())
                {
                    const auto Size = CalculateIconSize(0.f);
                    D2D1_RECT_F rc{ 0.f, 0.f, Size.x, Size.y };
                    CenterRect(rc, GetRectInClientD2D());
                    GetDC()->DrawBitmap(
                        m_BitmapIcon.Get(),
                        rc,
                        m_kIconOpacity,
                        (D2D1_INTERPOLATION_MODE)m_eInterMode,
                        m_BitmapIcon.GetSourceRect());
                }
            }
            else
            {
                auto pt = Kw::MakeD2DPointF(GetOffsetInClient());
                DWRITE_TEXT_METRICS tm;
                m_pLayout->GetMetrics(&tm);

                if (m_BitmapIcon.Get())
                {
                    const auto dInner = GetTheme()->GetMetric(IdMePaddingInner);

                    const auto Size = CalculateIconSize(tm.width);
                    D2D1_RECT_F rc;
                    rc.left = (GetWidth() - Size.x - dInner - tm.width) / 2.f;
                    rc.top = (GetHeight() - Size.y) / 2.f;
                    rc.right = rc.left + Size.x;
                    rc.bottom = rc.top + Size.y;
                    ElementToClient(rc);

                    GetDC()->DrawBitmap(
                        m_BitmapIcon.Get(),
                        rc,
                        m_kIconOpacity,
                        (D2D1_INTERPOLATION_MODE)m_eInterMode,
                        m_BitmapIcon.GetSourceRect());

                    pt.x += (Size.x + dInner);
                }

                ID2D1Brush* pBrush;
                if (m_bFade)
                {
                    m_pBrushFade->SetTransform(D2D1::Matrix3x2F::Translation(
                        pt.x + tm.width - m_cxFade, 0.f));
                    pBrush = m_pBrushFade.Get();
                }
                else
                    pBrush = GetWindow().CcSetBrushColor(
                        GetTheme()->GetColorD2D(IdCrFore));
                GetDC()->DrawTextLayout(
                    pt,
                    m_pLayout.Get(),
                    pBrush,
                    DrawTextLayoutFlags);
            }

            DbgDrawFrame();
            EndPaint(ps);
        }
        return 0;

        case WM_SETTEXT:
            UpdateTextLayout();
            return 0;
        case WM_SIZE:
            UpdateTextLayout();
            return 0;
        case WM_SETFONT:
            UpdateTextLayout();
            if (lParam)
                Invalidate();
            break;

        case WM_STYLECHANGED:
            TmAutoSwitchTheme(this, wParam);
            break;
        case WM_THEMECHANGED:
            if (m_bFade)
                UpdateFadeBrush();
            break;

        case WM_CREATE:
            SetTheme(TmDefaultTheme(TmIsDarkMode()).Get());
            UpdateTextLayout();
            break;
        case WM_DESTROY:
            m_pLayout.Clear();
            m_pBrushFade.Clear();
            break;
        }
        return __super::OnEvent(uMsg, wParam, lParam);
    }

    void SetBitmap(const CBitmap& Bmp) noexcept
    {
        m_BitmapIcon = Bmp;
        UpdateTextLayout();
    }
    EckInlineNdCe auto& GetBitmap() const noexcept { return m_BitmapIcon; }
    void SetBackgroundBitmap(const CBitmap& Bmp) noexcept { m_BitmapBack = Bmp; }
    EckInlineNdCe auto& GetBackgroundBitmap() const noexcept { return m_BitmapBack; }

    void SetFade(BOOL b) noexcept
    {
        if (!!m_bFade == !!b)
            return;
        m_bFade = b;
        if (m_bFade)
            UpdateFadeBrush();
        else
            m_pBrushFade.Clear();
    }
    EckInlineNdCe BOOL GetFade() const noexcept { return m_bFade; }

    EckInlineCe void SetOnlyBitmap(BOOL b) noexcept { m_bOnlyBitmap = b; }
    EckInlineNdCe BOOL GetOnlyBitmap() const noexcept { return m_bOnlyBitmap; }

    EckInlineCe void SetBackgroundMode(ImageMode e) noexcept { m_eBkImgMode = e; }
    EckInlineNdCe ImageMode GetBackgroundMode() const noexcept { return m_eBkImgMode; }

    EckInlineCe void SetInterpolationMode(D2D1_INTERPOLATION_MODE e) noexcept { m_eInterMode = (BYTE)e; }
    EckInlineNdCe D2D1_INTERPOLATION_MODE GetInterpolationMode() const noexcept { return (D2D1_INTERPOLATION_MODE)m_eInterMode; }

    EckInlineCe void SetIconOpacity(float f) noexcept { m_kIconOpacity = f; }
    EckInlineNdCe float GetIconOpacity() const noexcept { return m_kIconOpacity; }
    EckInlineCe void SetBackgroundOpacity(float f) noexcept { m_kBackOpacity = f; }
    EckInlineNdCe float GetBackgroundOpacity() const noexcept { return m_kBackOpacity; }

    EckInlineCe void SetFadeWidth(float f) noexcept
    {
        m_cxFade = f;
        if (m_bFade)
            UpdateFadeBrush();
    }
    EckInlineNdCe float GetFadeWidth() const noexcept { return m_cxFade; }

    EckInlineCe void SetAutoScale(BOOL b) noexcept { m_bAutoScale = b; }
    EckInlineNdCe BOOL GetAutoScale() const noexcept { return m_bAutoScale; }
};


class CTmLabel : public CTheme
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
inline RcPtr<CTheme> CLabel::TmMakeDefaultTheme(BOOL bDark) noexcept
{
    return TmMakeTheme<CTmLabel>(bDark);
}

class CUiaLabel : public CUiaBase
{
    STDMETHODIMP GetPropertyValue(PROPERTYID idProp, VARIANT* pRetVal) override
    {
        if (idProp == UIA_ControlTypePropertyId)
        {
            pRetVal->vt = VT_I4;
            pRetVal->intVal = UIA_TextControlTypeId;
            return S_OK;
        }
        return CUiaBase::GetPropertyValue(idProp, pRetVal);
    }
};
inline HRESULT CLabel::EhUiaMakeInterface() noexcept
{
    const auto p = new CUiaLabel{};
    UiaSetInterface(p);
    p->Release();
    return S_OK;
}
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END