#pragma once
#include "DuiBase.h"
#include "GraphicsHelper.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
class CLabel : public CElement
{
public:
    const static inline UINT IdMeFadeWidth = TmNextResourceId();

    constexpr static float DefaultFadeWidth = 40.f;
private:
    ComPtr<IDWriteTextLayout> m_pLayout{};
    ComPtr<ID2D1LinearGradientBrush> m_pBrushFade{};
    CBitmap m_BmpIcon{};
    CBitmap m_BmpBk{};

    BITBOOL m_bTransparent : 1{ TRUE };
    BITBOOL m_bOnlyBitmap : 1{};
    BITBOOL m_bFullElem : 1{};
    BITBOOL m_bUserColor : 1{};
    BITBOOL m_bFade : 1{};
    ImageMode m_eBkImgMode{ ImageMode::TopLeft };
    BYTE m_eInterMode{ D2D1_INTERPOLATION_MODE_LINEAR };

    void UpdateTextLayout() noexcept
    {
        float cx;
        if (m_BmpIcon.Get())
        {
            const auto rc = m_BmpIcon.GetActualSourceRect();
            cx = GetWidth() - (rc.right - rc.left) -
                GetTheme()->GetMetric(IdMePaddingInner);
        }
        else
            cx = GetWidth();
        cx -= (GetTheme()->GetMetric(IdMePaddingOuter) * 2.f);
        g_pDwFactory->CreateTextLayout(
            GetText().Data(), GetText().Size(),
            GetTextFormat().Get(), cx, GetHeight(), m_pLayout.AtClear());
    }

    void UpdateFadeBrush() noexcept
    {
        const auto cxFade = GetTheme()->GetMetric(IdMeFadeWidth, DefaultFadeWidth);
        const auto cx = GetWidth();
        const D2D1_GRADIENT_STOP Stop[]
        {
            { (cx - cxFade) / cx, GetTheme()->GetColor(IdCrFore) },
            { 1.f }
        };
        ComPtr<ID2D1GradientStopCollection> pStopCollection;
        GetDC()->CreateGradientStopCollection(Stop, 2, &pStopCollection);
        const D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES Prop{ {}, { cx, 0.f } };
        GetDC()->CreateLinearGradientBrush(Prop, pStopCollection.Get(), &m_pBrushFade);
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

            if (m_bOnlyBitmap)
            {
            }
            else
            {
                if (m_pLayout)
                {
                    const auto pBrush = (m_bFade ?
                        static_cast<ID2D1Brush*>(m_pBrushFade.Get()) :
                        GetWindow().CcSetBrushColor(ArgbToD2DColorF(
                            GetTheme()->GetColor(IdCrFore))));
                    GetDC()->DrawTextLayout(
                        Kw::MakeD2DPointF(GetOffsetInClient()),
                        m_pLayout.Get(),
                        pBrush,
                        DrawTextLayoutFlags);
                }
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
            if (m_bFade)
                UpdateFadeBrush();
            return 0;
        case WM_SETFONT:
            UpdateTextLayout();
            if (lParam)
                Invalidate();
            break;

        case WM_CREATE:
            UpdateTextLayout();
            break;
        }
        return __super::OnEvent(uMsg, wParam, lParam);
    }

    void SetIcon(const CBitmap& Bmp) noexcept
    {
        m_BmpIcon = Bmp;
        UpdateTextLayout();
    }
    EckInlineNdCe auto& GetIcon() const noexcept { return m_BmpIcon; }

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

    EckInlineCe void SetTransparent(BOOL b) noexcept { m_bTransparent = b; }
    EckInlineNdCe BOOL GetTransparent() const noexcept { return m_bTransparent; }

    EckInlineCe void SetOnlyBitmap(BOOL b) noexcept { m_bOnlyBitmap = b; }
    EckInlineNdCe BOOL GetOnlyBitmap() const noexcept { return m_bOnlyBitmap; }

    EckInlineCe void SetFullElement(BOOL b) noexcept { m_bFullElem = b; }
    EckInlineNdCe BOOL GetFullElement() const noexcept { return m_bFullElem; }

    EckInlineCe void SetBackgroundMode(ImageMode e) noexcept { m_eBkImgMode = e; }
    EckInlineNdCe ImageMode GetBackgroundMode() const noexcept { return m_eBkImgMode; }

    EckInlineCe void SetInterpolationMode(D2D1_INTERPOLATION_MODE e) noexcept { m_eInterMode = (BYTE)e; }
    EckInlineNdCe D2D1_INTERPOLATION_MODE GetInterpolationMode() const noexcept { return (D2D1_INTERPOLATION_MODE)m_eInterMode; }
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END