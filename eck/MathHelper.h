#pragma once
#include "KwDefine.h"

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
    f = fmod(f, 360);
    return f < 0 ? f + 360 : f;
}
template<std::floating_point T>
EckInlineNd T RadianIn0To2Pi(T f) noexcept
{
    f = fmod(f, Pi * 2.f);
    return f < 0 ? (f + Pi * 2.f) : f;
}

template<class T>
EckInline void CalculatePointFromCircleAngle(auto r, TAngle ag,
    _Out_ T& xRet, _Out_ T& yRet) noexcept
{
    xRet = T(r * cos(ag));
    yRet = T(r * sin(ag));
}

template<class T = float>
EckInline T CalculateLineLength(auto x1, auto y1, auto x2, auto y2) noexcept
{
    return (T)sqrt(((double)x1 - x2) * ((double)x1 - x2) +
        ((double)y1 - y2) * ((double)y1 - y2));
}

template<class T>
EckInline void CalculatePointFromLineScale(auto x1, auto y1, auto x2, auto y2,
    float fScale, _Out_ T& x, _Out_ T& y) noexcept
{
    x = T(x1 - (x1 - x2) * fScale);
    y = T(y1 - (y1 - y2) * fScale);
}

// 从角度计算椭圆上一段弧的端点
template<class T>
void CalculateArcFromEllipseAngle(auto x, auto y, auto xr, auto yr,
    TAngle agStart, TAngle agSweep,
    _Out_ T& x1, _Out_ T& y1, _Out_ T& x2, _Out_ T& y2) noexcept
{
    T x10, y10, x20, y20;
    const auto a = T((xr - x) / 2), b = T((yr - y) / 2);
    CalculatePointFromEllipseAngle(a, b, agStart, x10, y10);
    CalculatePointFromEllipseAngle(a, b, agStart + agSweep, x20, y20);
    x1 = x10 + a + x;
    y1 = -y10 + b + y;
    x2 = x20 + a + x;
    y2 = -y20 + b + y;
}

// 从两点计算直线方程
template<std::floating_point T>
EckInlineCe void CalculateLineEquation(auto x1, auto y1, auto x2, auto y2,
    _Out_ T& A, _Out_ T& B, _Out_ T& C) noexcept
{
    const auto m = y1 - y2;
    const auto n = x1 - x2;
    A = T(m);
    B = T(-n);
    C = T(-m * x2 + n * y2);
}
// 从一点和斜率计算直线方程
template<std::floating_point T>
EckInlineCe void CalculateLineEquation(auto x, auto y, auto k,
    _Out_ T& A, _Out_ T& B, _Out_ T& C) noexcept
{
    A = T(k);
    B = T(-1);
    C = T(y - k * x);
}

template<class T>
EckInline void PolarToRect(auto fRho, auto fTheta,
    _Out_ T& x, _Out_ T& y) noexcept
{
    x = T(fRho * cos(fTheta));
    y = T(fRho * sin(fTheta));
}
template<class T>
EckInline void RectToPolar(auto x, auto y,
    _Out_ T& fRho, _Out_ T& fTheta) noexcept
{
    fRho = (T)sqrt(x * x + y * y);
    fTheta = (T)atan2(y, x);
}

/// <summary>
/// 计算椭圆上一点
/// </summary>
/// <param name="a">半长轴</param>
/// <param name="b">半短轴</param>
/// <param name="ag">角度，圆心角</param>
/// <param name="xRet">点X坐标</param>
/// <param name="yRet">点Y坐标</param>
template<class T>
EckInline void CalculatePointFromEllipseAngle(auto a, auto b,
    TAngle ag, _Out_ T& xRet, _Out_ T& yRet) noexcept
{
    const auto agEccentric = atan2(a * sin(ag), b * cos(ag));
    xRet = T(a * cos(agEccentric));
    yRet = T(b * sin(agEccentric));
}

template<std::floating_point T = float, CcpPointStruct TPt>
EckInlineNd T CalculatePointToLineDistance(TPt pt, TPt pt1, TPt pt2) noexcept
{
    return T(fabs((pt2.y - pt1.y) * pt.x - (pt2.x - pt1.x) * pt.y + pt2.x * pt1.y - pt2.y * pt1.x) /
        sqrt((pt2.x - pt1.x) * (pt2.x - pt1.x) + (pt2.y - pt1.y) * (pt2.y - pt1.y)));
}

namespace Detail
{
    // 计算一个单应变换，将单位正方形变换为四边形
    // 返回矩阵遵循点为行向量的约定
    inline Kw::Matrix3x3 HomographyUnitSquareToQuadrilaterals(
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
    const auto H1 = Detail::HomographyUnitSquareToQuadrilaterals(ptSrc);
    const auto H2 = Detail::HomographyUnitSquareToQuadrilaterals(ptDst);
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
    return D2D1::Matrix3x2F(
        1.f - 2.f * A * A / t,
        u,
        u,
        1.f - 2.f * B * B / t,
        -2.f * A * C / t,
        -2.f * B * C / t);
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
EckInlineNd XFORM XFormRotate(float fAngle) noexcept
{
    return
    {
        cosf(fAngle), sinf(fAngle),
        -sinf(fAngle), cosf(fAngle),
        0.f, 0.f
    };
}
EckInlineNd XFORM XFormRotate(float fAngle, float x, float y) noexcept
{
    /*
    |1   0   0|   |m11 m12 0|   |1   0   0|   |m11                m12                0|
    |0   1   0| * |m21 m22 0| * |0   1   0| = |m21                m22                0|
    |-dx -dy 1|   |0   0   1|   |dx  dy  1|   |-dx*m11-dy*m21+dx  -dx*m12-dy*m22+dy  1|
    应用到XFORM时m12与m21应互换位置
    */
    return
    {
        cosf(fAngle), sinf(fAngle),
        -sinf(fAngle), cosf(fAngle),
        -x * cosf(fAngle) - y * sinf(fAngle) + x,
        x * sinf(fAngle) - y * cosf(fAngle) + y
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
template<class TPt>
inline void CalculateBezierControlPoints(
    std::vector<TPt>& vPt,
    _In_reads_(cPt) const TPt* pPt,
    size_t cPt,
    float K = 1.f) noexcept
{
    vPt.resize((cPt - 1) * 3 + 1);
    auto itResult = vPt.begin();
    *itResult++ = *pPt;// 起点
    *itResult++ = *pPt;// 控点1

    const auto [x2, y2] = pPt[1];
    const auto [x1, y1] = pPt[0];
    using T = std::remove_cvref_t<decltype(x1)>;

    T xMid1, yMid1, xMid2, yMid2, dx, dy;
    TPt ptMidLast{ (x1 + x2) / 2,(y1 + y2) / 2 };
    float LineLenLast = CalculateLineLength(x1, y1, x2, y2);
    for (int i = 1; i < cPt - 1; ++i)
    {
        const auto [x2, y2] = pPt[i + 1];
        const auto [x1, y1] = pPt[i];
        const TPt ptMid{ (x1 + x2) / 2,(y1 + y2) / 2 };
        const float LineLen = CalculateLineLength(x1, y1, x2, y2);
        const auto [x00, y00] = ptMidLast;
        const auto [x01, y01] = ptMid;
        CalculatePointFromLineScale(x1, y1, x00, y00, K, xMid1, yMid1);
        CalculatePointFromLineScale(x1, y1, x01, y01, K, xMid2, yMid2);
        CalculatePointFromLineScale(x00, y00, x01, y01,
            LineLenLast / (LineLen + LineLenLast), dx, dy);
        dx = x1 - dx;
        dy = y1 - dy;
        *itResult++ = { xMid1 + dx, yMid1 + dy };// 控点2
        *itResult++ = { x1, y1 };// 终点
        *itResult++ = { xMid2 + dx, yMid2 + dy };// 下一段曲线的控点1

        ptMidLast = ptMid;
        LineLenLast = LineLen;
    }
    *itResult++ = pPt[cPt - 1];// 控点2
    *itResult++ = pPt[cPt - 1];// 终点
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
        agSweep = abs(agSweep);
    }

    auto d = (T)fmod(ag - agStart, TwoPi);
    if (d < 0)
        d += TwoPi;
    return d <= (agSweep + std::numeric_limits<T>::epsilon());
}
ECK_NAMESPACE_END