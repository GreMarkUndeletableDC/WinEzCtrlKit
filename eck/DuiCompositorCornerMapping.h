#pragma once
#include "DuiBase.h"
#include "MathHelper.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
// 必须使用CalculateDistortMatrix/CalculateInverseDistortMatrix计算矩阵
class CCompositorCornerMapping : public CCompositor
{
private:
    float m_kOpacity{ 1.f };
    D2D1_INTERPOLATION_MODE m_eInterMode{ D2D1_INTERPOLATION_MODE_LINEAR };
    DirectX::XMFLOAT4X4A m_Matrix{};
    DirectX::XMFLOAT4X4A m_MatrixR{};
public:
    void TransformPoint(_Inout_ Kw::Vec2& pt, BOOL bNormalToComposited) noexcept override
    {
        const DirectX::XMFLOAT4A v{ pt.x, pt.y, 0.f, 1.f };
        auto p = DirectX::XMVector4Transform(
            DirectX::XMLoadFloat4A(&v),
            DirectX::XMLoadFloat4x4A(bNormalToComposited ? &m_MatrixR : &m_Matrix));
        const auto w = DirectX::XMVectorGetW(p);
        p = DirectX::XMVectorDivide(p, DirectX::XMVectorSet(w, w, w, w));
        pt.x = DirectX::XMVectorGetX(p);
        pt.y = DirectX::XMVectorGetY(p);
    }

    void CalculateCompositedRect(_Out_ D2D1_RECT_F& rc, BOOL bInClientOrParent) noexcept override
    {
        const auto pEle = GetElement();
        const auto cx = pEle->GetWidth();
        const auto cy = pEle->GetHeight();
        const D2D1_POINT_2F pt[]{ { 0, 0 }, { cx, 0 }, { cx, cy }, { 0, cy } };

        rc = { FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX };
        const auto m = DirectX::XMLoadFloat4x4A(&m_Matrix);
        for (const auto e : pt)
        {
            const DirectX::XMFLOAT4A v{ e.x, e.y, 0.f, 1.f };
            auto p = DirectX::XMVector4Transform(DirectX::XMLoadFloat4A(&v), m);
            const auto w = DirectX::XMVectorGetW(p);
            p = DirectX::XMVectorDivide(p, DirectX::XMVectorSet(w, w, w, w));
            const auto x = DirectX::XMVectorGetX(p);
            const auto y = DirectX::XMVectorGetY(p);
            if (x < rc.left) rc.left = x;
            if (x > rc.right) rc.right = x;
            if (y < rc.top) rc.top = y;
            if (y > rc.bottom) rc.bottom = y;
        }
        pEle->ElementToClient(rc);
        if (!bInClientOrParent && pEle->EtParent())
            pEle->EtParent()->ClientToElement(rc);
    }

    BOOL IsInPlace() const noexcept override { return FALSE; }

    void PostRender(COMP_RENDER_INFO& cri) noexcept override
    {
        cri.pDC->DrawBitmap(cri.pBitmap, cri.rcDst, m_kOpacity,
            m_eInterMode, cri.rcSrc, (D2D1_MATRIX_4X4_F*)&m_Matrix);
    }

    EckInlineNdCe auto AtMatrix() noexcept { return &m_Matrix; }
    EckInlineNdCe auto AtMatrixR() noexcept { return &m_MatrixR; }
    EckInlineNdCe auto AtMatrixD2D() noexcept { return (D2D1_MATRIX_4X4_F*)&m_Matrix; }
    EckInlineNdCe auto AtMatrixD2DR() noexcept { return (D2D1_MATRIX_4X4_F*)&m_MatrixR; }

    EckInlineCe void SetOpacity(float f) noexcept { m_kOpacity = f; }
    EckInlineNdCe float GetOpacity() const noexcept { return m_kOpacity; }

    EckInlineCe void SetInterpolationMode(D2D1_INTERPOLATION_MODE e) noexcept { m_eInterMode = e; }
    EckInlineNdCe auto GetInterpolationMode() const noexcept { return m_eInterMode; }
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END