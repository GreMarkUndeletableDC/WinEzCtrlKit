#pragma once
#include "DuiFilter.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
class CFilterBlur : public CFilter
{
public:
    struct Extra : BaseExtra
    {
        float fDeviation;
    };
private:
    ComPtr<ID2D1Effect> m_pFxBlur{};
    ComPtr<ID2D1Effect> m_pFxCrop{};
    float m_fDeviation{ 15.f };
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
            CLSID_D2D1GaussianBlur, &m_pFxBlur);
        if (FAILED(hr))
            return hr;
        hr = m_pFxBlur->SetValue(
            D2D1_GAUSSIANBLUR_PROP_BORDER_MODE,
            D2D1_BORDER_MODE_HARD);
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
        _In_opt_ const BaseExtra* pExtra = nullptr) noexcept override
    {
        CheckExtraSize((const Extra*)pExtra);
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

        hr = m_pFxBlur->SetValue(
            D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION,
            pExtra ? ((const Extra*)pExtra)->fDeviation : m_fDeviation);
        if (FAILED(hr))
            return hr;
        m_pFxBlur->SetInputEffect(0, m_pFxCrop.Get());

        CopyEffect(
            m_pFxBlur.Get(),
            { rc.left, rc.top },
            D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
            pExtra);
        return S_OK;
    }

    EckInlineCe void SetDeviation(float f) noexcept { m_fDeviation = f; }
    EckInlineNdCe float GetDeviation() const noexcept { return m_fDeviation; }
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END