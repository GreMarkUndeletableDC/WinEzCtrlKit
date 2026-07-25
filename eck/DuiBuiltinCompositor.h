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
        const auto cx = GetElement()->GetWidth();
        const auto cy = GetElement()->GetHeight();
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
        GetElement()->ElementToClient(rc);
        if (!bInClientOrParent && GetElement()->EtParent())
            GetElement()->EtParent()->ClientToElement(rc);
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

struct CCompositor2DAffineTransform : public CCompositor
{
private:
    D2D1::Matrix3x2F m_Matrix{};
    D2D1::Matrix3x2F m_MatrixR{};
    float m_kOpacity{ 1.f };
    D2D1_INTERPOLATION_MODE m_eInterMode{ D2D1_INTERPOLATION_MODE_LINEAR };
public:
    void TransformPoint(_Inout_ Kw::Vec2& pt, BOOL bNormalToComposited) noexcept override
    {
        D2D1_POINT_2F pt0;
        if (bNormalToComposited)
            pt0 = m_MatrixR.TransformPoint({ pt.x, pt.y });
        else
            pt0 = m_Matrix.TransformPoint({ pt.x, pt.y });
        pt.x = pt0.x;
        pt.y = pt0.y;
    }

    void CalculateCompositedRect(_Out_ D2D1_RECT_F& rc, BOOL bInClientOrParent) noexcept override
    {
        const auto cx = GetElement()->GetWidth();
        const auto cy = GetElement()->GetHeight();
        const D2D1_POINT_2F pt[]{ { 0, 0 }, { cx, 0 }, { cx, cy }, { 0, cy } };
        rc = { FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX };
        for (const auto& e : pt)
        {
            const auto pt0 = m_Matrix.TransformPoint(e);
            if (pt0.x < rc.left) rc.left = pt0.x;
            if (pt0.x > rc.right) rc.right = pt0.x;
            if (pt0.y < rc.top) rc.top = pt0.y;
            if (pt0.y > rc.bottom) rc.bottom = pt0.y;
        }
        GetElement()->ElementToClient(rc);
        if (!bInClientOrParent && GetElement()->EtParent())
            GetElement()->EtParent()->ClientToElement(rc);
    }

    BOOL IsInPlace() const noexcept override { return FALSE; }

    void PostRender(COMP_RENDER_INFO& cri) noexcept override
    {
        D2D1::Matrix3x2F MatOld;
        cri.pDC->GetTransform(&MatOld);
        cri.pDC->SetTransform(m_Matrix * MatOld);
        cri.pDC->DrawBitmap(cri.pBitmap, cri.rcDst,
            m_kOpacity, m_eInterMode, cri.rcSrc);
        cri.pDC->SetTransform(MatOld);
    }

    EckInline void SetMatrix(const D2D1::Matrix3x2F& Mat) noexcept
    {
        m_Matrix = m_MatrixR = Mat;
        m_MatrixR.Invert();
    }
    EckInline void SetMatrix(
        const D2D1::Matrix3x2F& Mat,
        const D2D1::Matrix3x2F& MatR) noexcept
    {
        m_Matrix = Mat;
        m_MatrixR = MatR;
    }
    EckInlineNdCe auto& GetMatrix() const noexcept { return m_Matrix; }
    EckInlineNdCe auto& GetMatrixR() const noexcept { return m_MatrixR; }

    EckInlineCe void SetOpacity(float f) noexcept { m_kOpacity = f; }
    EckInlineNdCe float GetOpacity() const noexcept { return m_kOpacity; }

    EckInlineCe void SetInterpolationMode(D2D1_INTERPOLATION_MODE e) noexcept { m_eInterMode = e; }
    EckInlineNdCe auto GetInterpolationMode() const noexcept { return m_eInterMode; }
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END