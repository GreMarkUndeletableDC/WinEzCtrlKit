#pragma once
#include "ComPtr.h"
#include "CScopeGuard.h"
#include "Utility.h"
#include "GraphicsHelper.h"

ECK_NAMESPACE_BEGIN
class CD2DShadow
{
public:
    enum class Shape : BYTE
    {
        Rect,
        RoundRect,
        Ellipse,
        Geometry,
    };

    struct DRAW_SHAPE
    {
        Shape eShape;
        ID2D1Brush* pBrush;
        union
        {
            D2D1_RECT_F rc;
            D2D1_ROUNDED_RECT rrc;
            D2D1_ELLIPSE ell;
            struct
            {
                ID2D1Geometry* pGeometry;
                D2D1_POINT_2F Offset;
                D2D1_POINT_2F Scale;
            };
        };
    };
private:
    constexpr static float DefaultScale = 0.1f;
    constexpr static float BlurExtentFactor = 3.f;
    constexpr static UINT TextureAlign = 64u;

    ComPtr<ID2D1DeviceContext> m_pDC{};
    ComPtr<ID2D1Effect> m_pFxBlur{};
    ComPtr<ID2D1Bitmap1> m_pBlurBitmap{};
    D2D1_SIZE_U m_TexPixelSize{};
    D2D1_SIZE_F m_TexLogicalSize{};
    D2D1_RECT_F m_rcContent{};
    D2D1_SIZE_F m_TexCornerRadius{};
    D2D1_SIZE_F m_ContentSize{};
    float m_fSigma{};
    float m_fScale{ DefaultScale };
    float m_xDpi{ 96.f }, m_yDpi{ 96.f };
    float m_fMargin{};
    float m_fTexMargin{};

    EckInlineNd static UINT CalculateSize(float fSize, float fDpi) noexcept
    {
        return (UINT(ceilf(fSize * fDpi / 96.f)) + (TextureAlign - 1u)) & ~(TextureAlign - 1u);
    }
public:
    /// <summary>
    /// 初始化阴影
    /// </summary>
    /// <param name="pDC">设备上下文，函数保存DPI，返回后调用方调用其他方法后必须保证DPI相同</param>
    /// <param name="ContentSize">阴影需要覆盖的最大内容尺寸，不含阴影边距</param>
    /// <param name="fSigma">高斯模糊标准差</param>
    /// <param name="fScale">预渲染位图相对最终绘制尺寸的缩放</param>
    /// <returns>HRESULT</returns>
    HRESULT Initialize(
        _In_ ID2D1DeviceContext* pDC,
        D2D1_SIZE_F ContentSize,
        float fSigma,
        float fScale = DefaultScale) noexcept
    {
        HRESULT hr;
        m_pDC.Clear();
        m_fSigma = fSigma;
        m_fScale = fScale;
        pDC->GetDpi(&m_xDpi, &m_yDpi);
        m_ContentSize = ContentSize;
        m_TexCornerRadius = {};
        m_fMargin = fSigma * BlurExtentFactor;
        const auto fTexSigma = fSigma * fScale;
        m_fTexMargin = fTexSigma * BlurExtentFactor;

        hr = pDC->CreateEffect(CLSID_D2D1GaussianBlur, m_pFxBlur.AtClear());
        if (FAILED(hr))
            return hr;

        m_pFxBlur->SetValue(
            D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION,
            fTexSigma);
        m_pFxBlur->SetValue(
            D2D1_GAUSSIANBLUR_PROP_OPTIMIZATION,
            D2D1_GAUSSIANBLUR_OPTIMIZATION_SPEED);
        m_pFxBlur->SetValue(
            D2D1_GAUSSIANBLUR_PROP_BORDER_MODE,
            D2D1_BORDER_MODE_HARD);

        const D2D1_SIZE_F TexContentSize
        {
            ContentSize.width * fScale,
            ContentSize.height * fScale,
        };
        m_TexPixelSize =
        {
            CalculateSize(TexContentSize.width + m_fTexMargin * 2.f, m_xDpi),
            CalculateSize(TexContentSize.height + m_fTexMargin * 2.f, m_yDpi),
        };

        const D2D1_BITMAP_PROPERTIES1 Prop
        {
            .pixelFormat = { DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED },
            .dpiX = m_xDpi,
            .dpiY = m_yDpi,
            .bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET,
        };
        hr = pDC->CreateBitmap(m_TexPixelSize, nullptr, 0u, Prop, m_pBlurBitmap.AtClear());
        if (FAILED(hr))
            return hr;

        m_TexLogicalSize =
        {
            m_TexPixelSize.width * 96.f / m_xDpi,
            m_TexPixelSize.height * 96.f / m_yDpi,
        };
        m_rcContent =
        {
            (m_TexLogicalSize.width - TexContentSize.width) * 0.5f,
            (m_TexLogicalSize.height - TexContentSize.height) * 0.5f,
            (m_TexLogicalSize.width + TexContentSize.width) * 0.5f,
            (m_TexLogicalSize.height + TexContentSize.height) * 0.5f,
        };

        m_pDC = pDC;
        return S_OK;
    }

    EckInlineNdCe BOOL IsValid() const noexcept { return !!m_pDC; }

    EckInlineNdCe D2D1_SIZE_U GetTexturePixelSize() const noexcept { return m_TexPixelSize; }
    EckInlineNdCe D2D1_SIZE_F GetTextureLogicalSize() const noexcept { return m_TexLogicalSize; }
    EckInlineNdCe D2D1_RECT_F GetContentRect() const noexcept { return m_rcContent; }
    EckInlineNdCe float GetMargin() const noexcept { return m_fMargin; }
    EckInlineNdCe float GetTextureMargin() const noexcept { return m_fTexMargin; }
    EckInlineNdCe float GetSigma() const noexcept { return m_fSigma; }
    EckInlineNdCe float GetScale() const noexcept { return m_fScale; }

    /// <summary>
    /// 生成推荐形状参数
    /// </summary>
    /// <param name="eShape">类型</param>
    /// <param name="pBrush">画刷，**不添加引用**</param>
    /// <param name="pGeometry">若类型为Geometry，则指定几何形，**不添加引用**</param>
    /// <param name="xCornerRadius、yCornerRadius">
    /// 若类型为RoundRect，则指定圆角半径，y半径为负时表示与x相同
    /// </param>
    /// <returns>形状参数</returns>
    EckInlineNd DRAW_SHAPE MakeShape(
        Shape eShape,
        _In_ ID2D1Brush* pBrush,
        _In_opt_ ID2D1Geometry* pGeometry = nullptr,
        float xCornerRadius = 0.f,
        float yCornerRadius = -1.f) const noexcept
    {
        DRAW_SHAPE Shape{};
        Shape.eShape = eShape;
        Shape.pBrush = pBrush;
        if (eShape == Shape::Geometry)
            Shape.pGeometry = pGeometry;

        switch (eShape)
        {
        case Shape::Rect:
            Shape.rc = m_rcContent;
            break;
        case Shape::RoundRect:
            if (yCornerRadius < 0.f)
                yCornerRadius = xCornerRadius;
            Shape.rrc =
            {
                m_rcContent,
                xCornerRadius * m_fScale,
                yCornerRadius * m_fScale,
            };
            break;
        case Shape::Ellipse:
            Shape.ell =
            {
                {
                    (m_rcContent.left + m_rcContent.right) * 0.5f,
                    (m_rcContent.top + m_rcContent.bottom) * 0.5f,
                },
                (m_rcContent.right - m_rcContent.left) * 0.5f,
                (m_rcContent.bottom - m_rcContent.top) * 0.5f,
            };
            break;
        case Shape::Geometry:
            if (pGeometry)
            {
                D2D1_RECT_F rcBounds{};
                if (SUCCEEDED(pGeometry->GetBounds(nullptr, &rcBounds)))
                {
                    const auto cx = rcBounds.right - rcBounds.left;
                    const auto cy = rcBounds.bottom - rcBounds.top;
                    Shape.Offset = { rcBounds.left, rcBounds.top };
                    Shape.Scale =
                    {
                        (m_rcContent.right - m_rcContent.left) / cx,
                        (m_rcContent.bottom - m_rcContent.top) / cy,
                    };
                }
            }
            break;
        }
        return Shape;
    }

    HRESULT MakeBlurBitmap(
        _In_opt_ ID2D1Bitmap1* pTempBitmap,
        const DRAW_SHAPE& Shape) noexcept
    {
        HRESULT hr;
        if (Shape.eShape == Shape::RoundRect)
        {
            m_TexCornerRadius = D2D1_SIZE_F{ Shape.rrc.radiusX, Shape.rrc.radiusY };
            m_TexCornerRadius.width = std::clamp(
                m_TexCornerRadius.width, 0.f,
                (m_rcContent.right - m_rcContent.left) * 0.5f);
            m_TexCornerRadius.height = std::clamp(
                m_TexCornerRadius.height, 0.f,
                (m_rcContent.bottom - m_rcContent.top) * 0.5f);
        }
        else
            m_TexCornerRadius = {};

        ComPtr<ID2D1Bitmap1> pInternalBitmap;
        if (pTempBitmap)
        {
            float xDpiBitmap, yDpiBitmap;
            pTempBitmap->GetDpi(&xDpiBitmap, &yDpiBitmap);
            if (xDpiBitmap != m_xDpi || yDpiBitmap != m_yDpi)
                return E_INVALIDARG;
            const auto Size = pTempBitmap->GetPixelSize();
            if (Size.width < m_TexPixelSize.width ||
                Size.height < m_TexPixelSize.height)
                return E_INVALIDARG;
        }
        else
        {
            D2D1_BITMAP_PROPERTIES1 Prop
            {
                .pixelFormat = { DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED },
                .dpiX = m_xDpi,
                .dpiY = m_yDpi,
                .bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET,
            };
            hr = m_pDC->CreateBitmap(m_TexPixelSize, nullptr, 0u, Prop, &pInternalBitmap);
            if (FAILED(hr))
                return hr;
            pTempBitmap = pInternalBitmap.Get();
        }

        ComPtr<ID2D1Image> pOldTarget;
        m_pDC->GetTarget(&pOldTarget);
        const CScopeGuard _{ [&] { m_pDC->SetTarget(pOldTarget.Get()); } };

        D2D1_MATRIX_3X2_F MatOld;
        m_pDC->GetTransform(&MatOld);

        m_pDC->SetTarget(pTempBitmap);
        m_pDC->BeginDraw();
        m_pDC->Clear({});
        m_pDC->SetTransform(D2D1::Matrix3x2F::Identity());
        switch (Shape.eShape)
        {
        case Shape::Rect:
            m_pDC->FillRectangle(Shape.rc, Shape.pBrush);
            break;
        case Shape::RoundRect:
            m_pDC->FillRoundedRectangle(Shape.rrc, Shape.pBrush);
            break;
        case Shape::Ellipse:
            m_pDC->FillEllipse(Shape.ell, Shape.pBrush);
            break;
        case Shape::Geometry:
            m_pDC->SetTransform(
                D2D1::Matrix3x2F
                {
                    Shape.Scale.x, 0.f,
                    0.f, Shape.Scale.y,
                    m_rcContent.left - Shape.Offset.x * Shape.Scale.x,
                    m_rcContent.top - Shape.Offset.y * Shape.Scale.y
                });
            m_pDC->FillGeometry(Shape.pGeometry, Shape.pBrush);
            break;
        }
        hr = m_pDC->EndDraw();
        m_pDC->SetTransform(MatOld);
        if (FAILED(hr))
            return hr;

        m_pFxBlur->SetInput(0, pTempBitmap);

        m_pDC->SetTarget(m_pBlurBitmap.Get());
        m_pDC->BeginDraw();
        m_pDC->DrawImage(
            m_pFxBlur.Get(),
            D2D1_INTERPOLATION_MODE_LINEAR,
            D2D1_COMPOSITE_MODE_SOURCE_COPY);
        hr = m_pDC->EndDraw();
        if (FAILED(hr))
            return hr;
        return S_OK;
    }

    HRESULT DrawGrid(
        _In_ ID2D1DeviceContext* pDC,
        const D2D1_RECT_F& rcContent,
        const D2D1_RECT_F& Margins,
        D2D1_INTERPOLATION_MODE eInterMode = D2D1_INTERPOLATION_MODE_LINEAR,
        float fAlpha = 1.f) const noexcept
    {
        const auto& rc = m_rcContent;
        const auto m = m_fTexMargin;

        // 纹理空间圆角半径映射到目标空间，并限制在内容尺寸的一半以内
        const auto fRx = std::min(
            m_TexCornerRadius.width / m_fScale,
            (rcContent.right - rcContent.left) * 0.5f);
        const auto fRy = std::min(
            m_TexCornerRadius.height / m_fScale,
            (rcContent.bottom - rcContent.top) * 0.5f);
        const auto tRx = m_TexCornerRadius.width;
        const auto tRy = m_TexCornerRadius.height;

        const auto fL = rcContent.left - Margins.left;
        const auto fT = rcContent.top - Margins.top;
        const auto fR = rcContent.right + Margins.right;
        const auto fB = rcContent.bottom + Margins.bottom;

        // 纹理圆角切点
        const auto tLi = rc.left + tRx;
        const auto tRi = rc.right - tRx;
        const auto tTi = rc.top + tRy;
        const auto tBi = rc.bottom - tRy;
        // 目标圆角切点
        const auto dLi = rcContent.left + fRx;
        const auto dRi = rcContent.right - fRx;
        const auto dTi = rcContent.top + fRy;
        const auto dBi = rcContent.bottom - fRy;

        const auto DrawPart = [&](const D2D1_RECT_F& rcSrc, const D2D1_RECT_F& rcDst) noexcept
            {
                if (rcSrc.right <= rcSrc.left || rcSrc.bottom <= rcSrc.top ||
                    rcDst.right <= rcDst.left || rcDst.bottom <= rcDst.top)
                    return;
                pDC->DrawBitmap(m_pBlurBitmap.Get(), rcDst, fAlpha, eInterMode, rcSrc);
            };

        const auto DrawCorner = [&](
            float xSrc, float ySrc, float xDst, float yDst,
            float sx, float sy, float mx, float my) noexcept
            {
                DrawPart(
                    MakeRect<D2D1_RECT_F>(xSrc, ySrc, xSrc + sx * tRx, ySrc + sy * tRy),
                    MakeRect<D2D1_RECT_F>(xDst, yDst, xDst + sx * fRx, yDst + sy * fRy));
                DrawPart(
                    MakeRect<D2D1_RECT_F>(xSrc, ySrc, xSrc - sx * m, ySrc + sy * tRy),
                    MakeRect<D2D1_RECT_F>(xDst, yDst, xDst - sx * mx, yDst + sy * fRy));
                DrawPart(
                    MakeRect<D2D1_RECT_F>(xSrc, ySrc, xSrc + sx * tRx, ySrc - sy * m),
                    MakeRect<D2D1_RECT_F>(xDst, yDst, xDst + sx * fRx, yDst - sy * my));
                DrawPart(
                    MakeRect<D2D1_RECT_F>(xSrc, ySrc, xSrc - sx * m, ySrc - sy * m),
                    MakeRect<D2D1_RECT_F>(xDst, yDst, xDst - sx * mx, yDst - sy * my));
            };

        DrawCorner(rc.left, rc.top, rcContent.left, rcContent.top,
            1.f, 1.f, Margins.left, Margins.top);
        DrawCorner(rc.right, rc.top, rcContent.right, rcContent.top,
            -1.f, 1.f, Margins.right, Margins.top);
        DrawCorner(rc.left, rc.bottom, rcContent.left, rcContent.bottom,
            1.f, -1.f, Margins.left, Margins.bottom);
        DrawCorner(rc.right, rc.bottom, rcContent.right, rcContent.bottom,
            -1.f, -1.f, Margins.right, Margins.bottom);

        DrawPart(
            { tLi, rc.top - m, tRi, rc.top },
            { dLi, fT, dRi, rcContent.top });
        DrawPart(
            { rc.left - m, tTi, rc.left, tBi },
            { fL, dTi, rcContent.left, dBi });
        DrawPart(
            { rc.right, tTi, rc.right + m, tBi },
            { rcContent.right, dTi, fR, dBi });
        DrawPart(
            { tLi, rc.bottom, tRi, rc.bottom + m },
            { dLi, rcContent.bottom, dRi, fB });
        return S_OK;
    }

    HRESULT DrawFull(
        _In_ ID2D1DeviceContext* pDC,
        const D2D1_RECT_F& rcContent,
        const D2D1_RECT_F& Margins,
        D2D1_INTERPOLATION_MODE eInterMode = D2D1_INTERPOLATION_MODE_LINEAR,
        float fAlpha = 1.f) const noexcept
    {
        const auto m = m_fTexMargin;
        const auto sx =
            (rcContent.right - rcContent.left) /
            (m_rcContent.right - m_rcContent.left);
        const auto sy =
            (rcContent.bottom - rcContent.top) /
            (m_rcContent.bottom - m_rcContent.top);
        const D2D1_RECT_F rcSrc
        {
            m_rcContent.left - m,
            m_rcContent.top - m,
            m_rcContent.right + m,
            m_rcContent.bottom + m,
        };
        const auto d = Margins.left;
        const D2D1_RECT_F rcDraw
        {
            rcContent.left - d,
            rcContent.top - d,
            rcContent.right + d,
            rcContent.bottom + d,
        };
        const D2D1_RECT_F rcClip
        {
            rcContent.left - Margins.left,
            rcContent.top - Margins.top,
            rcContent.right + Margins.right,
            rcContent.bottom + Margins.bottom,
        };
        if (rcClip.right <= rcClip.left || rcClip.bottom <= rcClip.top)
            return S_OK;

        pDC->PushAxisAlignedClip(rcClip, D2D1_ANTIALIAS_MODE_ALIASED);
        pDC->DrawBitmap(m_pBlurBitmap.Get(), rcDraw, fAlpha, eInterMode, rcSrc);
        pDC->PopAxisAlignedClip();
        return S_OK;
    }
};
ECK_NAMESPACE_END