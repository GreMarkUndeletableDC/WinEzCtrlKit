#pragma once
#include "ComPtr.h"
#include "CScopeGuard.h"

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
            } Geometry;
        } Shape;
    };
private:
    constexpr static float DefaultScale = 0.1f;
    constexpr static float BlurExtentFactor = 3.f;
    constexpr static UINT TextureAlign = 64u;

    ComPtr<ID2D1DeviceContext> m_pDC{};
    ComPtr<ID2D1Effect> m_pFxBlur{};
    ComPtr<ID2D1Bitmap1> m_pBlurBitmap{};
    D2D1_SIZE_U m_SizeCachePixel{};
    D2D1_SIZE_F m_SizeCacheLogical{};
    D2D1_RECT_F m_rcCacheContent{};
    D2D1_POINT_2F m_CacheCornerRadius{};
    float m_fScale{ DefaultScale };
    float m_xDpi{ 96.f }, m_yDpi{ 96.f };
    float m_fMarginCache{};

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
        m_fScale = fScale;
        pDC->GetDpi(&m_xDpi, &m_yDpi);
        m_CacheCornerRadius = {};
        const auto fCacheSigma = fSigma * fScale;
        m_fMarginCache = fCacheSigma * BlurExtentFactor;

        hr = pDC->CreateEffect(CLSID_D2D1GaussianBlur, m_pFxBlur.AtClear());
        if (FAILED(hr))
            return hr;

        m_pFxBlur->SetValue(
            D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION,
            fCacheSigma);
        m_pFxBlur->SetValue(
            D2D1_GAUSSIANBLUR_PROP_OPTIMIZATION,
            D2D1_GAUSSIANBLUR_OPTIMIZATION_SPEED);
        m_pFxBlur->SetValue(
            D2D1_GAUSSIANBLUR_PROP_BORDER_MODE,
            D2D1_BORDER_MODE_HARD);

        const D2D1_SIZE_F CacheContentSize
        {
            ContentSize.width * fScale,
            ContentSize.height * fScale,
        };
        m_SizeCachePixel =
        {
            CalculateSize(CacheContentSize.width + m_fMarginCache * 2.f, m_xDpi),
            CalculateSize(CacheContentSize.height + m_fMarginCache * 2.f, m_yDpi),
        };

        const D2D1_BITMAP_PROPERTIES1 Prop
        {
            .pixelFormat = { DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED },
            .dpiX = m_xDpi,
            .dpiY = m_yDpi,
            .bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET,
        };
        hr = pDC->CreateBitmap(m_SizeCachePixel, nullptr, 0u, Prop, m_pBlurBitmap.AtClear());
        if (FAILED(hr))
            return hr;

        m_SizeCacheLogical =
        {
            m_SizeCachePixel.width * 96.f / m_xDpi,
            m_SizeCachePixel.height * 96.f / m_yDpi,
        };
        m_rcCacheContent =
        {
            (m_SizeCacheLogical.width - CacheContentSize.width) * 0.5f,
            (m_SizeCacheLogical.height - CacheContentSize.height) * 0.5f,
            (m_SizeCacheLogical.width + CacheContentSize.width) * 0.5f,
            (m_SizeCacheLogical.height + CacheContentSize.height) * 0.5f,
        };

        m_pDC = pDC;
        return S_OK;
    }

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
            Shape.Shape.Geometry.pGeometry = pGeometry;

        switch (eShape)
        {
        case Shape::Rect:
            Shape.Shape.rc = m_rcCacheContent;
            break;
        case Shape::RoundRect:
            if (yCornerRadius < 0.f)
                yCornerRadius = xCornerRadius;
            Shape.Shape.rrc =
            {
                m_rcCacheContent,
                xCornerRadius * m_fScale,
                yCornerRadius * m_fScale,
            };
            break;
        case Shape::Ellipse:
            Shape.Shape.ell =
            {
                {
                    (m_rcCacheContent.left + m_rcCacheContent.right) * 0.5f,
                    (m_rcCacheContent.top + m_rcCacheContent.bottom) * 0.5f,
                },
                (m_rcCacheContent.right - m_rcCacheContent.left) * 0.5f,
                (m_rcCacheContent.bottom - m_rcCacheContent.top) * 0.5f,
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
                    Shape.Shape.Geometry.Offset = { rcBounds.left, rcBounds.top };
                    Shape.Shape.Geometry.Scale =
                    {
                        (m_rcCacheContent.right - m_rcCacheContent.left) / cx,
                        (m_rcCacheContent.bottom - m_rcCacheContent.top) / cy,
                    };
                }
            }
            break;
        }
        return Shape;
    }

    HRESULT MakeBlurBitmap(
        _In_opt_ ID2D1Bitmap1* pTempBitmap,
        _In_opt_ const DRAW_SHAPE* pShape) noexcept
    {
        HRESULT hr;
        m_CacheCornerRadius = {};

        ComPtr<ID2D1Bitmap1> pInternalBitmap;
        if (pTempBitmap)
        {
            float xDpiBitmap, yDpiBitmap;
            pTempBitmap->GetDpi(&xDpiBitmap, &yDpiBitmap);
            if (xDpiBitmap != m_xDpi || yDpiBitmap != m_yDpi)
                return E_INVALIDARG;
            const auto Size = pTempBitmap->GetPixelSize();
            if (Size.width < m_SizeCachePixel.width ||
                Size.height < m_SizeCachePixel.height)
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
            hr = m_pDC->CreateBitmap(m_SizeCachePixel, nullptr, 0u, Prop, &pInternalBitmap);
            if (FAILED(hr))
                return hr;
            pTempBitmap = pInternalBitmap.Get();
        }

        ComPtr<ID2D1Image> pOldTarget;
        m_pDC->GetTarget(&pOldTarget);
        const CScopeGuard _{ [&]() noexcept { m_pDC->SetTarget(pOldTarget.Get()); } };

        if (pShape)
        {
            D2D1_MATRIX_3X2_F MatOld;
            m_pDC->GetTransform(&MatOld);

            m_pDC->SetTarget(pTempBitmap);
            m_pDC->BeginDraw();
            m_pDC->Clear({});
            m_pDC->SetTransform(D2D1::Matrix3x2F::Identity());
            switch (pShape->eShape)
            {
            case Shape::Rect:
                m_pDC->FillRectangle(pShape->Shape.rc, pShape->pBrush);
                break;
            case Shape::RoundRect:
                m_pDC->FillRoundedRectangle(pShape->Shape.rrc, pShape->pBrush);
                m_CacheCornerRadius = { pShape->Shape.rrc.radiusX, pShape->Shape.rrc.radiusY };
                break;
            case Shape::Ellipse:
                m_pDC->FillEllipse(pShape->Shape.ell, pShape->pBrush);
                break;
            case Shape::Geometry:
            {
                const auto& Offset = pShape->Shape.Geometry.Offset;
                const auto& Scale = pShape->Shape.Geometry.Scale;
                m_pDC->SetTransform(
                    D2D1::Matrix3x2F
                    {
                        pShape->Shape.Geometry.Scale.x, 0.f,
                        0.f, pShape->Shape.Geometry.Scale.y,
                        m_rcCacheContent.left - Offset.x * Scale.x,
                        m_rcCacheContent.top - Offset.y * Scale.y
                    });
                m_pDC->FillGeometry(pShape->Shape.Geometry.pGeometry, pShape->pBrush);
            }
            break;
            }
            hr = m_pDC->EndDraw();
            m_pDC->SetTransform(MatOld);
            if (FAILED(hr))
                return hr;
        }

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

    /// <summary>
    /// 画九宫阴影
    /// </summary>
    /// <param name="pDC">设备上下文</param>
    /// <param name="rcContent">目标内容矩形，阴影环绕此矩形向外扩展</param>
    /// <param name="fMargin">阴影扩展距离</param>
    /// <param name="eInterMode">插值模式</param>
    /// <param name="fAlpha">透明度</param>
    /// <param name="pMatrix">4x4变换</param>
    /// <param name="fMarginTex">缓存纹理中阴影的外侧扩展距离，这通常为GetTextureMargin()</param>
    /// <param name="fMarginTexInner">
    /// 缓存纹理中阴影的内侧扩展距离，默认为GetTextureMargin()的1/8，圆角阴影忽略此参数，内侧扩展由圆角半径决定
    /// </param>
    void DrawGrid(
        _In_ ID2D1DeviceContext* pDC,
        const D2D1_RECT_F& rcContent,
        float fMargin,
        D2D1_INTERPOLATION_MODE eInterMode = D2D1_INTERPOLATION_MODE_LINEAR,
        float fAlpha = 1.f,
        _In_opt_ const D2D1_MATRIX_4X4_F* pMatrix = nullptr,
        float fMarginTex = 0.f,
        float fMarginTexInner = 0.f) const noexcept
    {
        const auto& rcTex = m_rcCacheContent;
        const auto& rc = rcContent;
        const auto m1 = (fMarginTex > 0.f) ? fMarginTex : m_fMarginCache;
        const auto m2 = (fMarginTexInner > 0.f) ? fMarginTexInner : (m_fMarginCache / 8.f);
        const auto n = fMargin;
        const auto DrawPart = [=](const D2D1_RECT_F& rcSrc, const D2D1_RECT_F& rcDst) noexcept
            {
                pDC->DrawBitmap(m_pBlurBitmap.Get(), rcDst, fAlpha, eInterMode, rcSrc);
            };

        if (m_CacheCornerRadius.x > 0.f || m_CacheCornerRadius.y > 0.f)
        {
            const auto k = std::max(
                (rc.right - rc.left) / (rcTex.right - rcTex.left),
                (rc.bottom - rc.top) / (rcTex.bottom - rcTex.top));
            const auto xm = m_CacheCornerRadius.x;
            const auto ym = m_CacheCornerRadius.y;
            const auto xn = xm * k;
            const auto yn = ym * k;

            DrawPart(// 左上圆角
                { rcTex.left - m1,  rcTex.top - m1,    rcTex.left + xm,  rcTex.top + ym },
                { rc.left - n,      rc.top - n,        rc.left + xn,     rc.top + yn });
            DrawPart(// 上边
                { rcTex.left + xm,  rcTex.top - m1,    rcTex.right - xm, rcTex.top + ym },
                { rc.left + xn,     rc.top - n,        rc.right - xn,    rc.top + yn });
            DrawPart(// 右上圆角
                { rcTex.right - xm, rcTex.top - m1,    rcTex.right + m1, rcTex.top + ym },
                { rc.right - xn,    rc.top - n,        rc.right + n,     rc.top + yn });
            DrawPart(// 左边
                { rcTex.left - m1,  rcTex.top + ym,    rcTex.left + xm,  rcTex.bottom - ym },
                { rc.left - n,      rc.top + yn,       rc.left + xn,     rc.bottom - yn });
            DrawPart(// 右边
                { rcTex.right - xm, rcTex.top + ym,    rcTex.right + m1, rcTex.bottom - ym },
                { rc.right - xn,    rc.top + yn,       rc.right + n,     rc.bottom - yn });
            DrawPart(// 右下圆角
                { rcTex.right - xm, rcTex.bottom - ym, rcTex.right + m1, rcTex.bottom + m1 },
                { rc.right - xn,    rc.bottom - yn,    rc.right + n,     rc.bottom + n });
            DrawPart(// 下边
                { rcTex.left + xm,  rcTex.bottom - ym, rcTex.right - xm, rcTex.bottom + ym },
                { rc.left + xn,     rc.bottom - n,     rc.right - xn,    rc.bottom + n });
            DrawPart(// 左下圆角
                { rcTex.left - m1,  rcTex.bottom - ym, rcTex.left + xm,  rcTex.bottom + m1 },
                { rc.left - n,      rc.bottom - yn,    rc.left + xn,     rc.bottom + n });
            return;
        }
        DrawPart(// 左上
            { rcTex.left - m1,  rcTex.top - m1,    rcTex.left + m2,  rcTex.top + m2 },
            { rc.left - n,      rc.top - n,        rc.left,          rc.top });
        DrawPart(// 上
            { rcTex.left + m2,  rcTex.top - m1,    rcTex.right - m2, rcTex.top + m2 },
            { rc.left,          rc.top - n,        rc.right,         rc.top });
        DrawPart(// 右上
            { rcTex.right - m2, rcTex.top - m1,    rcTex.right + m1, rcTex.top + m2 },
            { rc.right,         rc.top - n,        rc.right + n,     rc.top });
        DrawPart(// 左
            { rcTex.left - m1,  rcTex.top + m2,    rcTex.left + m2,  rcTex.bottom - m2 },
            { rc.left - n,      rc.top,            rc.left,          rc.bottom });
        DrawPart(// 右
            { rcTex.right - m2, rcTex.top + m2,    rcTex.right + m1, rcTex.bottom - m2 },
            { rc.right,         rc.top,            rc.right + n,     rc.bottom });
        DrawPart(// 左下
            { rcTex.left - m1,  rcTex.bottom - m2, rcTex.left + m2,  rcTex.bottom + m1 },
            { rc.left - n,      rc.bottom,         rc.left,          rc.bottom + n });
        DrawPart(// 下
            { rcTex.left + m2,  rcTex.bottom - m2, rcTex.right - m2, rcTex.bottom + m1 },
            { rc.left,          rc.bottom,         rc.right,         rc.bottom + n });
        DrawPart(// 右下
            { rcTex.right - m2, rcTex.bottom - m2, rcTex.right + m1, rcTex.bottom + m1 },
            { rc.right,         rc.bottom,         rc.right + n,     rc.bottom + n });
    }

    HRESULT DrawFull(
        _In_ ID2D1DeviceContext* pDC,
        const D2D1_RECT_F& rcContent,
        D2D1_INTERPOLATION_MODE eInterMode = D2D1_INTERPOLATION_MODE_LINEAR,
        float fAlpha = 1.f,
        _In_opt_ const D2D1_MATRIX_4X4_F* pMatrix = nullptr) const noexcept
    {
        const auto& rcTex = m_rcCacheContent;
        const auto& rc = rcContent;
        const auto m = m_fMarginCache;
        const auto kx = (rc.right - rc.left) / (rcTex.right - rcTex.left);
        const auto ky = (rc.bottom - rc.top) / (rcTex.bottom - rcTex.top);

        const D2D1_RECT_F rcSrc
        {
            m_rcCacheContent.left - m,
            m_rcCacheContent.top - m,
            m_rcCacheContent.right + m,
            m_rcCacheContent.bottom + m,
        };
        const D2D1_RECT_F rcDraw
        {
            rcContent.left - m * kx,
            rcContent.top - m * ky,
            rcContent.right + m * kx,
            rcContent.bottom + m * ky,
        };
        pDC->DrawBitmap(m_pBlurBitmap.Get(), rcDraw, fAlpha, eInterMode, rcSrc, pMatrix);
        return S_OK;
    }

    EckInlineNdCe BOOL IsValid() const noexcept { return !!m_pDC; }

    EckInlineNdCe D2D1_SIZE_U GetCachePixelSize() const noexcept { return m_SizeCachePixel; }
    EckInlineNdCe D2D1_SIZE_F GetCacheLogicalSize() const noexcept { return m_SizeCacheLogical; }
    EckInlineNdCe D2D1_RECT_F GetCacheContentRect() const noexcept { return m_rcCacheContent; }
    EckInlineNdCe float GetCacheMargin() const noexcept { return m_fMarginCache; }
    EckInlineNdCe float GetScale() const noexcept { return m_fScale; }
};
ECK_NAMESPACE_END