#pragma once
#include "DuiFilter.h"
#include "D2DEffectGradientBlur.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
class CFilterGradientBlur : public CFilter
{
public:
    struct Extra : BaseExtra
    {
        float fRadiusMin;
        float fRadiusMax;
        CGradientBlurEffect::Direction eDirection;
        D2D1_VECTOR_2F Range;
    };
private:
    ComPtr<ID2D1Effect> m_pFxBlur{};
    ComPtr<ID2D1Effect> m_pFxCrop{};
    Extra m_Param{ sizeof(Extra) };
public:
    HRESULT Attach(_In_opt_ CDuiWindow* pWnd) noexcept override
    {
        HRESULT hr;

        m_pFxBlur.Clear();
        m_pFxCrop.Clear();

        hr = CFilter::Attach(pWnd);
        if (FAILED(hr))
            return hr;
        if (!pWnd)
            return S_OK;
        const auto pDC = pWnd->RdGetDC();

        hr = pDC->CreateEffect(
            CLSID_GradientBlurEffect, &m_pFxBlur);
        if (FAILED(hr))
            return hr;

        hr = pDC->CreateEffect(CLSID_D2D1Crop, &m_pFxCrop);
        if (FAILED(hr))
            return hr;
        return S_OK;
    }

    HRESULT FilterDC(
        const D2D1_RECT_F& rc,
        float ox, float oy,
        _In_opt_ const BaseExtra* pExtra_ = nullptr) noexcept override
    {
        const Extra* pExtra = (const Extra*)pExtra_;
        CheckExtraSize(pExtra);
        if (!pExtra)
            pExtra = &m_Param;

        HRESULT hr;

        ComPtr<ID2D1Bitmap1> pBitmap;
        ComPtr<ID2D1Image> pTarget;
        GetWindow()->RdGetDC()->GetTarget(&pTarget);
        hr = pTarget->QueryInterface(&pBitmap);
        if (FAILED(hr))
            return hr;

        ReserveWindowCacheBitmap(rc);
        const auto pCacheBitmap = GetWindow()->CcGetBitmap();
        float xDpi, yDpi;
        pBitmap->GetDpi(&xDpi, &yDpi);
        const D2D1_RECT_U rcU
        {
            UINT32((rc.left + ox) * xDpi / 96.f),
            UINT32((rc.top + oy) * yDpi / 96.f),
            UINT32((rc.right + ox) * xDpi / 96.f),
            UINT32((rc.bottom + oy) * yDpi / 96.f)
        };
        hr = pCacheBitmap->CopyFromBitmap(nullptr, pBitmap.Get(), &rcU);
        if (FAILED(hr))
            return hr;

        hr = m_pFxCrop->SetValue(
            D2D1_CROP_PROP_RECT,
            D2D1::RectF(0.f, 0.f, rc.right - rc.left, rc.bottom - rc.top));
        if (FAILED(hr))
            return hr;
        m_pFxCrop->SetInput(0, pCacheBitmap);

        m_pFxBlur->SetValue(CGradientBlurEffect::PROP_RADIUS_MIN, pExtra->fRadiusMin);
        m_pFxBlur->SetValue(CGradientBlurEffect::PROP_RADIUS_MAX, pExtra->fRadiusMax);
        m_pFxBlur->SetValue(CGradientBlurEffect::PROP_GRADIENT_DIRECTION, pExtra->eDirection);
        m_pFxBlur->SetValue(CGradientBlurEffect::PROP_GRADIENT_RANGE, pExtra->Range);

        m_pFxBlur->SetInputEffect(0, m_pFxCrop.Get());

        CopyEffect(
            m_pFxBlur.Get(),
            { rc.left, rc.top },
            D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
            pExtra);
        return S_OK;
    }

    EckInlineCe void SetRadiusMinimum(float f) noexcept { m_Param.fRadiusMin = f; }
    EckInlineNdCe float GetRadiusMinimum() const noexcept { return m_Param.fRadiusMin; }

    EckInlineCe void SetRadiusMaximum(float f) noexcept { m_Param.fRadiusMax = f; }
    EckInlineNdCe float GetRadiusMaximum() const noexcept { return m_Param.fRadiusMax; }

    EckInlineCe void SetGradientDirection(CGradientBlurEffect::Direction e) noexcept
    {
        m_Param.eDirection = e;
    }
    EckInlineNdCe CGradientBlurEffect::Direction GetGradientDirection() const noexcept
    {
        return m_Param.eDirection;
    }

    EckInlineCe void SetGradientRange(const D2D1_VECTOR_2F& e) noexcept
    {
        m_Param.Range = e;
    }
    EckInlineNdCe const D2D1_VECTOR_2F& GetGradientRange() const noexcept
    {
        return m_Param.Range;
    }
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END