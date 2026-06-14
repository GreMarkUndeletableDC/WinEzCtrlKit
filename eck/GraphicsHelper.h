#pragma once
#include "Utility.h"

ECK_NAMESPACE_BEGIN
enum class GradientMode :BYTE
{
    None,	// 无效
    T2B,	// 从上到下
    B2T,	// 从下到上
    L2R,	// 从左到右
    R2L,	// 从右到左
    TL2BR,	// 从左上到右下↘
    BR2TL,	// 从右下到左上↖
    BL2TR,	// 从左下到右上↗
    TR2BL	// 从右上到左下↙
};

inline BOOL FillGradientRect(
    HDC hDC,
    const RECT& rc,
    _In_reads_(3) const COLORREF* crGradient,
    GradientMode eMode) noexcept
{
    TRIVERTEX tv[4];
    COLORREF cr1, cr2, cr3;
    if (eMode >= GradientMode::T2B && eMode <= GradientMode::R2L)
    {
        ULONG uMode;
        switch (eMode)
        {
        case GradientMode::T2B:// 从上到下
        case GradientMode::B2T:// 从下到上
        {
            cr2 = crGradient[1];
            tv[0].x = rc.left;
            tv[0].y = rc.top;
            tv[1].x = rc.right;
            tv[1].y = (rc.top + rc.bottom) / 2;

            tv[2].x = rc.left;
            tv[2].y = (rc.top + rc.bottom) / 2;
            tv[3].x = rc.right;
            tv[3].y = rc.bottom;
            uMode = GRADIENT_FILL_RECT_V;
            if (eMode == GradientMode::T2B)
            {
                cr1 = crGradient[0];
                cr3 = crGradient[2];
            }
            else
            {
                cr1 = crGradient[2];
                cr3 = crGradient[0];
            }
        }
        break;
        case GradientMode::L2R:// 从左到右
        case GradientMode::R2L:// 从右到左
        {
            cr2 = crGradient[1];
            tv[0].x = rc.left;
            tv[0].y = rc.top;
            tv[1].x = (rc.left + rc.right) / 2;
            tv[1].y = rc.bottom;

            tv[2].x = (rc.left + rc.right) / 2;
            tv[2].y = rc.top;
            tv[3].x = rc.right;
            tv[3].y = rc.bottom;
            uMode = GRADIENT_FILL_RECT_H;
            if (eMode == GradientMode::L2R)
            {
                cr1 = crGradient[0];
                cr3 = crGradient[2];
            }
            else
            {
                cr1 = crGradient[2];
                cr3 = crGradient[0];
            }
        }
        break;
        default: return FALSE;
        }

        tv[0].Red = GetRValue(cr1) << 8;
        tv[0].Green = GetGValue(cr1) << 8;
        tv[0].Blue = GetBValue(cr1) << 8;
        tv[0].Alpha = 0xFF << 8;

        tv[1].Red = GetRValue(cr2) << 8;
        tv[1].Green = GetGValue(cr2) << 8;
        tv[1].Blue = GetBValue(cr2) << 8;
        tv[1].Alpha = 0xFF << 8;

        tv[2] = tv[1];

        tv[3].Red = GetRValue(cr3) << 8;
        tv[3].Green = GetGValue(cr3) << 8;
        tv[3].Blue = GetBValue(cr3) << 8;
        tv[3].Alpha = 0xFF << 8;

        GRADIENT_RECT gr[2];
        gr[0].UpperLeft = 0;
        gr[0].LowerRight = 1;
        gr[1].UpperLeft = 2;
        gr[1].LowerRight = 3;
        return GradientFill(hDC, tv, ARRAYSIZE(tv), gr, ARRAYSIZE(gr), uMode);
    }
    else if (eMode >= GradientMode::TL2BR && eMode <= GradientMode::TR2BL)
    {
        // 左上
        tv[0].x = rc.left;
        tv[0].y = rc.top;
        // 左下
        tv[1].x = rc.left;
        tv[1].y = rc.bottom;
        // 右上
        tv[2].x = rc.right;
        tv[2].y = rc.top;
        // 右下
        tv[3].x = rc.right;
        tv[3].y = rc.bottom;

        GRADIENT_TRIANGLE gt[2];
        switch (eMode)
        {
        case GradientMode::TL2BR:// 左上到右下↘
        case GradientMode::BR2TL:// 右下到左上↖
        {
            gt[0].Vertex1 = 0;
            gt[0].Vertex2 = 1;
            gt[0].Vertex3 = 2;
            gt[1].Vertex1 = 3;
            gt[1].Vertex2 = 1;
            gt[1].Vertex3 = 2;
            cr2 = crGradient[1];
            if (eMode == GradientMode::TL2BR)
            {
                cr1 = crGradient[0];
                cr3 = crGradient[2];
            }
            else
            {
                cr1 = crGradient[2];
                cr3 = crGradient[0];
            }

            tv[0].Red = GetRValue(cr1) << 8;
            tv[0].Green = GetGValue(cr1) << 8;
            tv[0].Blue = GetBValue(cr1) << 8;
            tv[0].Alpha = 0xFF << 8;

            tv[1].Red = GetRValue(cr2) << 8;
            tv[1].Green = GetGValue(cr2) << 8;
            tv[1].Blue = GetBValue(cr2) << 8;
            tv[1].Alpha = 0xFF << 8;

            tv[2] = tv[1];

            tv[3].Red = GetRValue(cr3) << 8;
            tv[3].Green = GetGValue(cr3) << 8;
            tv[3].Blue = GetBValue(cr3) << 8;
            tv[3].Alpha = 0xFF << 8;
        }
        break;
        case GradientMode::BL2TR:// 左下到右上↗
        case GradientMode::TR2BL:// 右上到左下↙
        {
            gt[0].Vertex1 = 1;
            gt[0].Vertex2 = 0;
            gt[0].Vertex3 = 3;
            gt[1].Vertex1 = 2;
            gt[1].Vertex2 = 0;
            gt[1].Vertex3 = 3;
            cr2 = crGradient[1];
            if (eMode == GradientMode::BL2TR)
            {
                cr1 = crGradient[0];
                cr3 = crGradient[2];
            }
            else
            {
                cr1 = crGradient[2];
                cr3 = crGradient[0];
            }

            tv[0].Red = GetRValue(cr2) << 8;
            tv[0].Green = GetGValue(cr2) << 8;
            tv[0].Blue = GetBValue(cr2) << 8;
            tv[0].Alpha = 0xFF << 8;

            tv[1].Red = GetRValue(cr1) << 8;
            tv[1].Green = GetGValue(cr1) << 8;
            tv[1].Blue = GetBValue(cr1) << 8;
            tv[1].Alpha = 0xFF << 8;

            tv[3] = tv[0];

            tv[2].Red = GetRValue(cr3) << 8;
            tv[2].Green = GetGValue(cr3) << 8;
            tv[2].Blue = GetBValue(cr3) << 8;
            tv[2].Alpha = 0xFF << 8;
        }
        break;
        default: return FALSE;
        }
        return GradientFill(hDC, tv, ARRAYSIZE(tv),
            gt, ARRAYSIZE(gt), GRADIENT_FILL_TRIANGLE);
    }
    return FALSE;
}

inline BOOL FillGradientRect(
    HDC hDC,
    const RECT& rc,
    COLORREF cr1,
    COLORREF cr2,
    BOOL bVertical) noexcept
{
    TRIVERTEX tv[2];
    tv[0].x = rc.left;
    tv[0].y = rc.top;
    tv[0].Red = GetRValue(cr1) << 8;
    tv[0].Green = GetGValue(cr1) << 8;
    tv[0].Blue = GetBValue(cr1) << 8;

    tv[1].x = rc.right;
    tv[1].y = rc.bottom;
    tv[1].Red = GetRValue(cr2) << 8;
    tv[1].Green = GetGValue(cr2) << 8;
    tv[1].Blue = GetBValue(cr2) << 8;

    GRADIENT_RECT gr;
    gr.UpperLeft = 0;   // 左上角坐标为第一个成员
    gr.LowerRight = 1;  // 右下角坐标为第二个成员

    return GradientFill(hDC, tv, 2, &gr, 1,
        bVertical ? GRADIENT_FILL_RECT_V : GRADIENT_FILL_RECT_H);
}

inline BOOL DrawBackgroundImage(
    _In_ HDC hDC,
    _In_ HDC hdcBitmap,
    ImageMode eMode,
    const RECT& rc,
    _In_opt_ const RECT* prcSrc = nullptr,
    BOOL bAlphaBlend = FALSE,
    BYTE byAlpha = 255) noexcept
{
    const BLENDFUNCTION bf{ AC_SRC_OVER, 0, byAlpha, AC_SRC_ALPHA };
    RCWH rcSrc;
    if (prcSrc)
    {
        rcSrc.x = prcSrc->left;
        rcSrc.y = prcSrc->top;
        rcSrc.cx = prcSrc->right - prcSrc->left;
        rcSrc.cy = prcSrc->bottom - prcSrc->top;
    }
    else
    {
        BITMAP bm;
        if (!GetObjectW(GetCurrentObject(hdcBitmap, OBJ_BITMAP), sizeof(bm), &bm))
            return FALSE;
        rcSrc = { 0, 0, bm.bmWidth, bm.bmHeight };
    }

#undef ECK_BLIT
#define ECK_BLIT(x_, y_, cx_, cy_) \
    (bAlphaBlend ?                 \
        AlphaBlend(hDC, x_, y_, cx_, cy_, hdcBitmap,    \
            rcSrc.x, rcSrc.y, rcSrc.cx, rcSrc.cy, bf) : \
        BitBlt(hDC, x_, y_, cx_, cy_, hdcBitmap, rcSrc.x, rcSrc.y, SRCCOPY))

    switch (eMode)
    {
    case ImageMode::TopLeft:
        return ECK_BLIT(rc.left, rc.top, rcSrc.cx, rcSrc.cy);
    case ImageMode::TopLeftUniform:
    case ImageMode::TopLeftUniformFill:
    {
        const auto rcRef{ MakeRcwh(rc) };
        auto rcNew{ rcSrc };
        if (eMode == ImageMode::TopLeftUniform)
            AdjustRectToFitAnother(rcNew, rcRef);
        else
            AdjustRectToFillAnother(rcNew, rcRef);
        return ECK_BLIT(rc.left, rc.top, rcNew.cx, rcNew.cy);
    }
    ECK_UNREACHABLE;
    case ImageMode::Center:
        return ECK_BLIT(
            rc.left + (rc.right - rc.left - rcSrc.cx) / 2,
            rc.top + (rc.bottom - rc.top - rcSrc.cy) / 2, rcSrc.cx, rcSrc.cy);
    case ImageMode::CenterUniform:
    case ImageMode::CenterUniformFill:
    {
        const auto rcRef{ MakeRcwh(rc) };
        auto rcNew{ rcSrc };
        if (eMode == ImageMode::CenterUniform)
            AdjustRectToFitAnother(rcNew, rcRef);
        else
            AdjustRectToFillAnother(rcNew, rcRef);
        return ECK_BLIT(rcNew.x, rcNew.y, rcNew.cx, rcNew.cy);
    }
    ECK_UNREACHABLE;
    case ImageMode::Tile:
    {
        const auto cH = CeilDivide<int>(rc.right - rc.left, rcSrc.cx);
        const auto cV = CeilDivide<int>(rc.bottom - rc.top, rcSrc.cy);
        EckCounter(cH, i)
        {
            EckCounter(cV, j)
                if (!ECK_BLIT(
                    rc.left + i * rcSrc.cx,
                    rc.top + j * rcSrc.cy, rcSrc.cx, rcSrc.cy))
                    return FALSE;
        }
        return TRUE;
    }
    ECK_UNREACHABLE;
    case ImageMode::Stretch:// 缩放
        return ECK_BLIT(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
    }
    return FALSE;
#undef ECK_BLIT
}

#if !ECK_OPT_NO_GDIPLUS
inline GpStatus DrawBackgroundImage(
    _In_ GpGraphics* pGraphics,
    _In_ GpImage* pImage,
    ImageMode eMode,
    const GpRectF& rc,
    _In_opt_ const GpRectF* prcSrc = nullptr) noexcept
{
    // 巧妙利用 MakeRcwh 推导浮点版本 RCWH 结构体类型（如 RCWHF）
    auto rcSrc{ MakeRcwhF(rc) };
    if (prcSrc)
    {
        rcSrc.x = prcSrc->X;
        rcSrc.y = prcSrc->Y;
        rcSrc.cx = prcSrc->Width;
        rcSrc.cy = prcSrc->Height;
    }
    else
    {
        UINT w, h;
        GdipGetImageWidth(pImage, &w);
        GdipGetImageHeight(pImage, &h);
        rcSrc = { 0.f, 0.f, (float)w, (float)h };
    }

#undef ECK_BLIT
#define ECK_BLIT(x_, y_, cx_, cy_) \
    GdipDrawImageRectRect(pGraphics, pImage, \
        float(x_), float(y_), float(cx_), float(cy_), \
        (float)rcSrc.x, (float)rcSrc.y, (float)rcSrc.cx, (float)rcSrc.cy, \
        Gdiplus::UnitPixel, nullptr, nullptr, nullptr)

    switch (eMode)
    {
    case ImageMode::TopLeft:
        return ECK_BLIT(rc.X, rc.Y, rcSrc.cx, rcSrc.cy);
    case ImageMode::TopLeftUniform:
    case ImageMode::TopLeftUniformFill:
    {
        const auto rcRef{ MakeRcwhF(rc) };
        auto rcNew{ rcSrc };
        if (eMode == ImageMode::TopLeftUniform)
            AdjustRectToFitAnother(rcNew, rcRef);
        else
            AdjustRectToFillAnother(rcNew, rcRef);
        return ECK_BLIT(rc.X, rc.Y, rcNew.cx, rcNew.cy);
    }
    ECK_UNREACHABLE;
    case ImageMode::Center:
        return ECK_BLIT(
            rc.X + (rc.Width - rcSrc.cx) / 2.f,
            rc.Y + (rc.Height - rcSrc.cy) / 2.f, rcSrc.cx, rcSrc.cy);
    case ImageMode::CenterUniform:
    case ImageMode::CenterUniformFill:
    {
        const auto rcRef{ MakeRcwhF(rc) };
        auto rcNew{ rcSrc };
        if (eMode == ImageMode::CenterUniform)
            AdjustRectToFitAnother(rcNew, rcRef);
        else
            AdjustRectToFillAnother(rcNew, rcRef);
        return ECK_BLIT(rcNew.x, rcNew.y, rcNew.cx, rcNew.cy);
    }
    ECK_UNREACHABLE;
    case ImageMode::Tile:
    {
        const auto cH = CeilDivide<int>((int)rc.Width, (int)rcSrc.cx);
        const auto cV = CeilDivide<int>((int)rc.Height, (int)rcSrc.cy);
        EckCounter(cH, i)
        {
            EckCounter(cV, j)
            {
                const auto status = ECK_BLIT(
                    rc.X + i * rcSrc.cx,
                    rc.Y + j * rcSrc.cy, rcSrc.cx, rcSrc.cy);
                if (status != Gdiplus::Ok)
                    return status;
            }
        }
        return Gdiplus::Ok;
    }
    ECK_UNREACHABLE;
    case ImageMode::Stretch: // 缩放
        return ECK_BLIT(rc.X, rc.Y, rc.Width, rc.Height);
    }
    return Gdiplus::GenericError;
#undef ECK_BLIT
}
#endif // !ECK_OPT_NO_GDIPLUS

#if !ECK_OPT_NO_D2D
inline void DrawBackgroundImage(
    _In_ ID2D1DeviceContext* pDC,
    _In_ ID2D1Bitmap* pBmp,
    ImageMode eMode,
    const D2D1_RECT_F& rc,
    _In_opt_ const D2D1_RECT_F* prcSrc = nullptr,
    float fAlpha = 1.f,
    D2D1_INTERPOLATION_MODE eInterMode = D2D1_INTERPOLATION_MODE_LINEAR) noexcept
{
    auto rcSrc{ MakeRcwhF(rc) };
    D2D1_RECT_F rcSrcD2D;
    if (prcSrc)
    {
        rcSrc.x = prcSrc->left;
        rcSrc.y = prcSrc->top;
        rcSrc.cx = prcSrc->right - prcSrc->left;
        rcSrc.cy = prcSrc->bottom - prcSrc->top;
        rcSrcD2D = *prcSrc;
    }
    else
    {
        const auto size = pBmp->GetSize();
        rcSrc = { 0.f, 0.f, size.width, size.height };
        rcSrcD2D = { 0.f, 0.f, size.width, size.height };
    }

#undef ECK_BLIT
#define ECK_BLIT(x_, y_, cx_, cy_) \
    pDC->DrawBitmap(pBmp, \
        { float(x_), float(y_), float((x_) + (cx_)), float((y_) + (cy_)) }, \
        fAlpha, eInterMode, &rcSrcD2D)

    switch (eMode)
    {
    case ImageMode::TopLeft:
        ECK_BLIT(rc.left, rc.top, rcSrc.cx, rcSrc.cy);
        return;
    case ImageMode::TopLeftUniform:
    case ImageMode::TopLeftUniformFill:
    {
        const auto rcRef{ MakeRcwhF(rc) };
        auto rcNew{ rcSrc };
        if (eMode == ImageMode::TopLeftUniform)
            AdjustRectToFitAnother(rcNew, rcRef);
        else
            AdjustRectToFillAnother(rcNew, rcRef);
        ECK_BLIT(rc.left, rc.top, rcNew.cx, rcNew.cy);
        return;
    }
    ECK_UNREACHABLE;
    case ImageMode::Center:
        ECK_BLIT(
            rc.left + (rc.right - rc.left - rcSrc.cx) / 2.f,
            rc.top + (rc.bottom - rc.top - rcSrc.cy) / 2.f, rcSrc.cx, rcSrc.cy);
        return;
    case ImageMode::CenterUniform:
    case ImageMode::CenterUniformFill:
    {
        const auto rcRef{ MakeRcwhF(rc) };
        auto rcNew{ rcSrc };
        if (eMode == ImageMode::CenterUniform)
            AdjustRectToFitAnother(rcNew, rcRef);
        else
            AdjustRectToFillAnother(rcNew, rcRef);
        ECK_BLIT(rcNew.x, rcNew.y, rcNew.cx, rcNew.cy);
        return;
    }
    ECK_UNREACHABLE;
    case ImageMode::Tile:
    {
        const auto cH = CeilDivide<int>((int)(rc.right - rc.left), (int)rcSrc.cx);
        const auto cV = CeilDivide<int>((int)(rc.bottom - rc.top), (int)rcSrc.cy);
        EckCounter(cH, i)
        {
            EckCounter(cV, j)
                ECK_BLIT(
                    rc.left + i * rcSrc.cx,
                    rc.top + j * rcSrc.cy, rcSrc.cx, rcSrc.cy);
        }
        return;
    }
    ECK_UNREACHABLE;
    case ImageMode::Stretch: // 缩放
        ECK_BLIT(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
        return;
    }
#undef ECK_BLIT
}
#endif // !ECK_OPT_NO_D2D

struct SAVE_DC_CLIP
{
    HRGN hRgn;
};

EckInline SAVE_DC_CLIP SaveDcClip(HDC hDC) noexcept
{
    SAVE_DC_CLIP sdc{ CreateRectRgn(0,0,1,1) };
    if (GetClipRgn(hDC, sdc.hRgn) == 1)
        return sdc;
    else
    {
        DeleteObject(sdc.hRgn);
        return {};
    }
}

EckInline BOOL RestoreDcClip(HDC hDC, SAVE_DC_CLIP sdc) noexcept
{
    const auto b = (SelectClipRgn(hDC, sdc.hRgn) == ERROR);
    if (sdc.hRgn)
        DeleteObject(sdc.hRgn);
    return b;
}

EckInline int IntersectClipRect(HDC hDC, const RECT& rc) noexcept
{
    return IntersectClipRect(hDC, rc.left, rc.top, rc.right, rc.bottom);
}

#if !ECK_OPT_NO_D2D
inline void DrawImageFromGrid(ID2D1DeviceContext* pDC, ID2D1Bitmap* pBmp,
    const D2D1_RECT_F& rcDst, const D2D1_RECT_F& rcSrc, const D2D1_RECT_F& rcMargins,
    D2D1_INTERPOLATION_MODE eInterpolationMode = D2D1_INTERPOLATION_MODE_LINEAR,
    float fAlpha = 1.f) noexcept
{
    D2D1_RECT_F rcDstTmp, rcSrcTmp;
    // 左上
    rcDstTmp = { rcDst.left, rcDst.top, rcDst.left + rcMargins.left, rcDst.top + rcMargins.top };
    rcSrcTmp = { rcSrc.left, rcSrc.top, rcSrc.left + rcMargins.left, rcSrc.top + rcMargins.top };
    pDC->DrawBitmap(pBmp, rcDstTmp, fAlpha, eInterpolationMode, rcSrcTmp);
    // 上
    rcDstTmp = { rcDst.left + rcMargins.left, rcDst.top, rcDst.right - rcMargins.right, rcDst.top + rcMargins.top };
    rcSrcTmp = { rcSrc.left + rcMargins.left, rcSrc.top, rcSrc.right - rcMargins.right, rcSrc.top + rcMargins.top };
    pDC->DrawBitmap(pBmp, rcDstTmp, fAlpha, eInterpolationMode, rcSrcTmp);
    // 右上
    rcDstTmp = { rcDst.right - rcMargins.right, rcDst.top, rcDst.right, rcDst.top + rcMargins.top };
    rcSrcTmp = { rcSrc.right - rcMargins.right, rcSrc.top, rcSrc.right, rcSrc.top + rcMargins.top };
    pDC->DrawBitmap(pBmp, rcDstTmp, fAlpha, eInterpolationMode, rcSrcTmp);
    // 左
    rcDstTmp = { rcDst.left, rcDst.top + rcMargins.top, rcDst.left + rcMargins.left, rcDst.bottom - rcMargins.bottom };
    rcSrcTmp = { rcSrc.left, rcSrc.top + rcMargins.top, rcSrc.left + rcMargins.left, rcSrc.bottom - rcMargins.bottom };
    pDC->DrawBitmap(pBmp, rcDstTmp, fAlpha, eInterpolationMode, rcSrcTmp);
    // 右
    rcDstTmp = { rcDst.right - rcMargins.right, rcDst.top + rcMargins.top, rcDst.right, rcDst.bottom - rcMargins.bottom };
    rcSrcTmp = { rcSrc.right - rcMargins.right, rcSrc.top + rcMargins.top, rcSrc.right, rcSrc.bottom - rcMargins.bottom };
    pDC->DrawBitmap(pBmp, rcDstTmp, fAlpha, eInterpolationMode, rcSrcTmp);
    // 左下
    rcDstTmp = { rcDst.left, rcDst.bottom - rcMargins.bottom, rcDst.left + rcMargins.left, rcDst.bottom };
    rcSrcTmp = { rcSrc.left, rcSrc.bottom - rcMargins.bottom, rcSrc.left + rcMargins.left, rcSrc.bottom };
    pDC->DrawBitmap(pBmp, rcDstTmp, fAlpha, eInterpolationMode, rcSrcTmp);
    // 下
    rcDstTmp = { rcDst.left + rcMargins.left, rcDst.bottom - rcMargins.bottom, rcDst.right - rcMargins.right, rcDst.bottom };
    rcSrcTmp = { rcSrc.left + rcMargins.left, rcSrc.bottom - rcMargins.bottom, rcSrc.right - rcMargins.right, rcSrc.bottom };
    pDC->DrawBitmap(pBmp, rcDstTmp, fAlpha, eInterpolationMode, rcSrcTmp);
    // 右下
    rcDstTmp = { rcDst.right - rcMargins.right, rcDst.bottom - rcMargins.bottom, rcDst.right, rcDst.bottom };
    rcSrcTmp = { rcSrc.right - rcMargins.right, rcSrc.bottom - rcMargins.bottom, rcSrc.right, rcSrc.bottom };
    pDC->DrawBitmap(pBmp, rcDstTmp, fAlpha, eInterpolationMode, rcSrcTmp);
    // 中
    rcDstTmp = { rcDst.left + rcMargins.left, rcDst.top + rcMargins.top, rcDst.right - rcMargins.right, rcDst.bottom - rcMargins.bottom };
    rcSrcTmp = { rcSrc.left + rcMargins.left, rcSrc.top + rcMargins.top, rcSrc.right - rcMargins.right, rcSrc.bottom - rcMargins.bottom };
    pDC->DrawBitmap(pBmp, rcDstTmp, fAlpha, eInterpolationMode, rcSrcTmp);
}
#endif // !ECK_OPT_NO_D2D

#if !ECK_OPT_NO_GDIPLUS
inline GpStatus DrawImageFromGrid(GpGraphics* pGraphics, GpImage* pImage,
    int xDst, int yDst, int cxDst, int cyDst,
    int xSrc, int ySrc, int cxSrc, int cySrc,
    const MARGINS& Margins, Gdiplus::GpImageAttributes* pIA,
    Gdiplus::Unit eUnit = Gdiplus::UnitPixel) noexcept
{
    // 左上
    GdipDrawImageRectRectI(pGraphics, pImage,
        xDst, yDst, Margins.cxLeftWidth, Margins.cyTopHeight,
        xSrc, ySrc, Margins.cxLeftWidth, Margins.cyTopHeight,
        eUnit, pIA, nullptr, nullptr);
    // 上
    GdipDrawImageRectRectI(pGraphics, pImage,
        xDst + Margins.cxLeftWidth, yDst, cxDst - Margins.cxRightWidth - Margins.cxLeftWidth, Margins.cyTopHeight,
        xSrc + Margins.cxLeftWidth, ySrc, cxSrc - Margins.cxRightWidth - Margins.cxLeftWidth, Margins.cyTopHeight,
        eUnit, pIA, nullptr, nullptr);
    // 右上
    GdipDrawImageRectRectI(pGraphics, pImage,
        xDst + cxDst - Margins.cxRightWidth, yDst, Margins.cxRightWidth, Margins.cyTopHeight,
        xSrc + cxSrc - Margins.cxRightWidth, ySrc, Margins.cxRightWidth, Margins.cyTopHeight,
        eUnit, pIA, nullptr, nullptr);
    // 左
    GdipDrawImageRectRectI(pGraphics, pImage,
        xDst, yDst + Margins.cyTopHeight, Margins.cxLeftWidth, cyDst - Margins.cyBottomHeight - Margins.cyTopHeight,
        xSrc, ySrc + Margins.cyTopHeight, Margins.cxLeftWidth, cySrc - Margins.cyBottomHeight - Margins.cyTopHeight,
        eUnit, pIA, nullptr, nullptr);
    // 右
    GdipDrawImageRectRectI(pGraphics, pImage,
        xDst + cxDst - Margins.cxRightWidth, yDst + Margins.cyTopHeight, Margins.cxRightWidth, cyDst - Margins.cyBottomHeight - Margins.cyTopHeight,
        xSrc + cxSrc - Margins.cxRightWidth, ySrc + Margins.cyTopHeight, Margins.cxRightWidth, cySrc - Margins.cyBottomHeight - Margins.cyTopHeight,
        eUnit, pIA, nullptr, nullptr);
    // 左下
    GdipDrawImageRectRectI(pGraphics, pImage,
        xDst, yDst + cyDst - Margins.cyBottomHeight, Margins.cxLeftWidth, Margins.cyBottomHeight,
        xSrc, ySrc + cySrc - Margins.cyBottomHeight, Margins.cxLeftWidth, Margins.cyBottomHeight,
        eUnit, pIA, nullptr, nullptr);
    // 下
    GdipDrawImageRectRectI(pGraphics, pImage,
        xDst + Margins.cxLeftWidth, yDst + cyDst - Margins.cyBottomHeight, cxDst - Margins.cxRightWidth - Margins.cxLeftWidth, Margins.cyBottomHeight,
        xSrc + Margins.cxLeftWidth, ySrc + cySrc - Margins.cyBottomHeight, cxSrc - Margins.cxRightWidth - Margins.cxLeftWidth, Margins.cyBottomHeight,
        eUnit, pIA, nullptr, nullptr);
    // 右下
    GdipDrawImageRectRectI(pGraphics, pImage,
        xDst + cxDst - Margins.cxRightWidth, yDst + cyDst - Margins.cyBottomHeight, Margins.cxRightWidth, Margins.cyBottomHeight,
        xSrc + cxSrc - Margins.cxRightWidth, ySrc + cySrc - Margins.cyBottomHeight, Margins.cxRightWidth, Margins.cyBottomHeight,
        eUnit, pIA, nullptr, nullptr);
    // 中
    return GdipDrawImageRectRectI(pGraphics, pImage,
        xDst + Margins.cxLeftWidth, yDst + Margins.cyTopHeight, cxDst - Margins.cxRightWidth - Margins.cxLeftWidth, cyDst - Margins.cyBottomHeight - Margins.cyTopHeight,
        xSrc + Margins.cxLeftWidth, ySrc + Margins.cyTopHeight, cxSrc - Margins.cxRightWidth - Margins.cxLeftWidth, cySrc - Margins.cyBottomHeight - Margins.cyTopHeight,
        eUnit, pIA, nullptr, nullptr);
}
#endif // !ECK_OPT_NO_GDIPLUS
ECK_NAMESPACE_END