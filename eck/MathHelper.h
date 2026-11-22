#pragma once
#include "KwDefine.h"
#include "Utility.h"

#include <DirectXMath.h>

ECK_NAMESPACE_BEGIN
// double计算后转为float：1e-6，只使用float：1e-4
using TAngle = double;

template<std::floating_point T>
EckInlineNdCe T DegreeToRadian(T fDeg) noexcept { return fDeg * (T)Pi / 180; }
template<std::floating_point T>
EckInlineNdCe T RadianToDegree(T fRad) noexcept { return fRad * 180 / (T)Pi; }

template<std::floating_point T>
EckInlineNd T DegreeIn0To360(T f) noexcept
{
    constexpr auto Full = T(360);
    f = (T)fmod(f, Full);
    return f < 0 ? f + Full : f;
}
template<std::floating_point T>
EckInlineNd T RadianIn0To2Pi(T f) noexcept
{
    constexpr auto TwoPi = T(Pi * 2.);
    f = (T)fmod(f, TwoPi);
    return f < 0 ? f + TwoPi : f;
}

EckInline Kw::Vector2 PointFromCircleAngle(float r, TAngle ag) noexcept
{
    return { float(r * cos(ag)), float(r * sin(ag)) };
}

EckInline float LineLength(
    const Kw::Vector2& pt1, const Kw::Vector2& pt2) noexcept
{
    return Kw::Length(pt1 - pt2);
}

EckInlineCe Kw::Vector2 PointFromLineScale(
    const Kw::Vector2& pt1, const Kw::Vector2& pt2, float k) noexcept
{
    return { pt1.x - (pt1.x - pt2.x) * k, pt1.y - (pt1.y - pt2.y) * k };
}

// 计算椭圆上一点
EckInline Kw::Vector2 PointFromEllipseAngle(float a, float b, TAngle ag) noexcept
{
    const auto agEccentric = atan2(a * sin(ag), b * cos(ag));
    return { float(a * cos(agEccentric)), float(b * sin(agEccentric)) };
}

EckInlineNd std::array<Kw::Vector2, 2> ArcFromEllipseAngle(
    const Kw::Vector2& ptCenter,
    float a, float b,
    TAngle agStart, TAngle agSweep) noexcept
{
    return
    {
        PointFromEllipseAngle(a, b, agStart) + ptCenter,
        PointFromEllipseAngle(a, b, agStart + agSweep) + ptCenter
    };
}
// 从外接矩形和角度计算一段弧的端点
EckInlineNd std::array<Kw::Vector2, 2> ArcFromEllipseAngle(
    const Kw::Rect& rc, TAngle agStart, TAngle agSweep) noexcept
{
    const auto a = (rc.right - rc.left) / 2.f;
    const auto b = (rc.bottom - rc.top) / 2.f;
    const Kw::Vector2 ptCenter{ (rc.left + rc.right) / 2.f, (rc.top + rc.bottom) / 2.f };
    return ArcFromEllipseAngle(ptCenter, a, b, agStart, agSweep);
}

// 从两点计算直线方程，返回系数
EckInlineCe Kw::Vector3 CalculateLineEquation(
    const Kw::Vector2& pt1, const Kw::Vector2& pt2) noexcept
{
    const auto m = pt1.y - pt2.y;
    const auto n = pt1.x - pt2.x;
    return { m, -n, -m * pt2.x + n * pt2.y };
}
// 从一点和斜率计算直线方程，返回系数
EckInlineCe Kw::Vector3 CalculateLineEquation(
    const Kw::Vector2& pt, float k) noexcept
{
    return { k, -1.f, pt.y - k * pt.x };
}

EckInline Kw::Vector2 PolarToRect(const Kw::PolarPoint& p) noexcept
{
    return { float(p.Rho * cos(p.Theta)), float(p.Rho * sin(p.Theta)) };
}
EckInline Kw::PolarPoint RectToPolar(const Kw::Vector2& p) noexcept
{
    return { Kw::Length(p), atan2(p.y, p.x) };
}

EckInlineNd float PointToLineDistance(
    const Kw::Vector2& pt, const Kw::Vector2& pt1, const Kw::Vector2& pt2) noexcept
{
    return Absolute(Kw::Cross(pt2 - pt1, pt - pt1)) / Kw::Length(pt2 - pt1);
}

namespace Detail
{
    // 计算一个单应变换，将单位正方形变换为四边形
    // 返回矩阵遵循点为行向量的约定
    inline Kw::Matrix3x3 HomographyUnitSquareToQuadrilateral(
        _In_reads_(4) const Kw::Vector2* pt) noexcept
    {
        const auto x0 = pt[0].x, y0 = pt[0].y;
        const auto x1 = pt[1].x, y1 = pt[1].y;
        const auto x2 = pt[2].x, y2 = pt[2].y;
        const auto x3 = pt[3].x, y3 = pt[3].y;

        const auto A = x1 - x2;
        const auto B = x3 - x2;
        const auto C = x0 - x1 - x3 + x2;
        const auto D = y1 - y2;
        const auto E = y3 - y2;
        const auto F = y0 - y1 - y3 + y2;
        const auto Delta = A * E - B * D;

        const auto g = (C * E - B * F) / Delta;
        const auto h = (A * F - C * D) / Delta;
        const auto a = x1 * (g + 1.f) - x0;
        const auto b = x3 * (h + 1.f) - x0;
        const auto d = y1 * (g + 1.f) - y0;
        const auto e = y3 * (h + 1.f) - y0;

        return
        {
            a,  d,  g,
            b,  e,  h,
            x0, y0, 1.f
        };
    }
}

// 输入输出顶点需对应，不要求顶点顺序
// WARNING 禁止传入退化四边形或凹四边形
EckInline void CalculateDistortMatrix(
    _In_reads_(4) const Kw::Vector2* ptSrc,
    _In_reads_(4) const Kw::Vector2* ptDst,
    _Out_ DirectX::XMFLOAT4X4& Matrix) noexcept
{
    const auto H1 = Detail::HomographyUnitSquareToQuadrilateral(ptSrc);
    const auto H2 = Detail::HomographyUnitSquareToQuadrilateral(ptDst);
    const auto H = Kw::Inverse(H1) * H2;
    Matrix =
    {
        H(0, 0), H(0, 1), 0.f, H(0, 2),
        H(1, 0), H(1, 1), 0.f, H(1, 2),
        0.f,     0.f,     1.f, 0.f,
        H(2, 0), H(2, 1), 0.f, H(2, 2)
    };
}

// WARNING
// 以下矩形参数重载要求顶点顺序TL,TR,BL,BR

EckInline void CalculateDistortMatrix(
    const Kw::Rect& rcSrc,
    _In_reads_(4) const Kw::Vector2* ptDst,
    _Out_ DirectX::XMFLOAT4X4& Matrix) noexcept
{
    const Kw::Vector2 ptSrc[4]
    {
        { rcSrc.left,  rcSrc.top },
        { rcSrc.right, rcSrc.top },
        { rcSrc.left,  rcSrc.bottom },
        { rcSrc.right, rcSrc.bottom }
    };
    CalculateDistortMatrix(ptSrc, ptDst, Matrix);
}
EckInline void CalculateDistortMatrix(
    _In_reads_(4) const Kw::Vector2* ptSrc,
    const Kw::Rect& rcDst,
    _Out_ DirectX::XMFLOAT4X4& Matrix) noexcept
{
    const Kw::Vector2 ptDst[4]
    {
        { rcDst.left,  rcDst.top },
        { rcDst.right, rcDst.top },
        { rcDst.left,  rcDst.bottom },
        { rcDst.right, rcDst.bottom }
    };
    CalculateDistortMatrix(ptSrc, ptDst, Matrix);
}

EckInline void CalculateDistortMatrix(
    const D2D1_RECT_F& rcSrc,
    _In_reads_(4) const D2D1_POINT_2F* ptDst,
    _Out_ D2D1_MATRIX_4X4_F& Matrix) noexcept
{
    CalculateDistortMatrix(Kw::MakeRect(rcSrc),
        (const Kw::Vector2*)ptDst, *(DirectX::XMFLOAT4X4*)&Matrix);
}
EckInline void CalculateDistortMatrix(
    _In_reads_(4) const D2D1_POINT_2F* ptSrc,
    const D2D1_RECT_F& rcDst,
    _Out_ D2D1_MATRIX_4X4_F& Matrix) noexcept
{
    CalculateDistortMatrix((const Kw::Vector2*)ptSrc,
        Kw::MakeRect(rcDst), *(DirectX::XMFLOAT4X4*)&Matrix);
}

EckInlineNd D2D1::Matrix3x2F D2DMatrixReflection(float A, float B, float C) noexcept
{
    const float t = A * A + B * B;
    const float u = -2.f * A * B / t;
    return
    {
        1.f - 2.f * A * A / t,
        u,
        u,
        1.f - 2.f * B * B / t,
        -2.f * A * C / t,
        -2.f * B * C / t
    };
}

EckInlineNdCe XFORM XFormTranslate(float dx, float dy) noexcept
{
    return
    {
        1.f, 0.f,
        0.f, 1.f,
        dx, dy
    };
}
EckInlineNd XFORM XFormRotate(TAngle fAngle) noexcept
{
    const auto c = cos(fAngle), s = sin(fAngle);
    return
    {
        float(c),  float(s),
        float(-s), float(c),
        0.f, 0.f
    };
}
EckInlineNd XFORM XFormRotate(TAngle fAngle, float x, float y) noexcept
{
    const auto c = cos(fAngle), s = sin(fAngle);
    return
    {
        float(c),  float(s),
        float(-s), float(c),
        float(x * (1. - c) + y * s),
        float(y * (1. - c) - x * s)
    };
}
EckInlineNdCe XFORM XFormScale(float xScale, float yScale) noexcept
{
    return
    {
        xScale, 0.f,
        0.f, yScale,
        0.f, 0.f
    };
}
EckInlineNdCe XFORM XFormScale(float xScale, float yScale, float x, float y) noexcept
{
    return
    {
        xScale, 0.f,
        0.f, yScale,
        -x * xScale + x, -y * yScale + y
    };
}
EckInlineNdCe XFORM XFormShear(float xFactor, float yFactor) noexcept
{
    return
    {
        1.f, xFactor,
        yFactor, 1.f,
        0.f, 0.f
    };
}
EckInlineNdCe XFORM XFormShear(float xFactor, float yFactor, float x, float y) noexcept
{
    return
    {
        1.f, xFactor,
        yFactor, 1.f,
        -x - y * yFactor + x, -x * xFactor - y + y
    };
}
EckInlineNdCe XFORM XFormReflection(float A, float B, float C) noexcept
{
    const float t = A * A + B * B;
    const float u = -2.f * A * B / t;
    return
    {
        1.f - 2.f * A * A / t,
        u,
        u,
        1.f - 2.f * B * B / t,
        -2.f * A * C / t,
        -2.f * B * C / t
    };
}

/// <summary>
/// 从离散点计算贝塞尔曲线
/// </summary>
/// <param name="vPt">结果点集合</param>
/// <param name="pPt">离散点</param>
/// <param name="cPt">离散点数量</param>
/// <param name="K">张力</param>
inline void CalculateBezierControlPoints(
    Eck_Append_buffer_ std::vector<Kw::Vector2>& vPt,
    _In_reads_(cPt) const Kw::Vector2* pPt,
    size_t cPt,
    float K = 1.f) noexcept
{
    if (cPt < 2)
        return;
    const auto cOld = vPt.size();
    vPt.resize(vPt.size() + (cPt - 1) * 3 + 1);
    auto p = vPt.data() + cOld;
    *p++ = *pPt;// 起点
    *p++ = *pPt;// 控点1

    Kw::Vector2 ptMidLast{ (pPt[0].x + pPt[1].x) / 2, (pPt[0].y + pPt[1].y) / 2 };
    float fLastLen = LineLength(pPt[0], pPt[1]);
    for (size_t i = 1; i < cPt - 1; ++i)
    {
        const auto& pt1 = pPt[i];
        const auto& pt2 = pPt[i + 1];
        const Kw::Vector2 ptMid{ (pt1.x + pt2.x) / 2, (pt1.y + pt2.y) / 2 };
        const auto fLen = Kw::Length(pt1 - pt2);
        const auto ptMid1 = PointFromLineScale(pt1, ptMidLast, K);
        const auto ptMid2 = PointFromLineScale(pt1, ptMid, K);
        // 相邻线段全为 0 长度时避免除零产生 NaN 控制点
        const auto fLenTotal = fLen + fLastLen;
        const auto fRatio = fLenTotal > 0.f ? fLastLen / fLenTotal : 0.5f;
        const auto ptLine = pt1 - PointFromLineScale(ptMidLast, ptMid, fRatio);
        *p++ = ptMid1 + ptLine;// 控点2
        *p++ = pt1;// 终点
        *p++ = ptMid2 + ptLine;// 下一段曲线的控点1

        ptMidLast = ptMid;
        fLastLen = fLen;
    }
    *p++ = pPt[cPt - 1];// 控点2
    *p++ = pPt[cPt - 1];// 终点
}

template<std::floating_point T>
inline bool RadianInArc(T ag, T agStart, T agSweep) noexcept
{
    constexpr auto TwoPi = T(Pi * 2.);
    if (agSweep >= TwoPi || agSweep <= -TwoPi)
        return true;
    if (agSweep < 0)
    {
        agStart += agSweep;
        agSweep = Absolute(agSweep);
    }

    auto d = (T)fmod(ag - agStart, TwoPi);
    if (d < 0)
        d += TwoPi;
    return d <= (agSweep + std::numeric_limits<T>::epsilon());
}
ECK_NAMESPACE_END