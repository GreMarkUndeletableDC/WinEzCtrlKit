#include "pch.h"
#include "../eck/MathHelper.h"

using namespace eck;

namespace
{
    void AssertPoint(const Kw::Vector2& pt, float x, float y, float tol = 1e-5f)
    {
        Assert::AreEqual(x, pt.x, tol);
        Assert::AreEqual(y, pt.y, tol);
    }

    // 直线方程 A x + B y + C = 0 的左端取值，点位于直线上时结果应为 0
    float EvalLineEquation(const Kw::Vector3& ln, float x, float y) noexcept
    {
        return ln.x * x + ln.y * y + ln.z;
    }

    // XFORM 采用行向量约定：X' = x*eM11 + y*eM21 + eDx，Y' = x*eM12 + y*eM22 + eDy
    void ApplyXForm(const XFORM& tf, float x, float y,
        _Out_ float& ox, _Out_ float& oy) noexcept
    {
        ox = x * tf.eM11 + y * tf.eM21 + tf.eDx;
        oy = x * tf.eM12 + y * tf.eM22 + tf.eDy;
    }
}

TS_NS_BEGIN
TEST_CLASS(TsAngles)
{
public:
    template<typename T>
    void TsDegRad()
    {
        Assert::AreEqual((T)Pi, DegreeToRadian<T>((T)180), (T)1e-6);
        Assert::AreEqual((T)180, RadianToDegree<T>((T)Pi), (T)1e-6);
        Assert::AreEqual((T)(Pi / 2), DegreeToRadian<T>((T)90), (T)1e-6);
        Assert::AreEqual((T)(-Pi / 2), DegreeToRadian<T>((T)-90), (T)1e-6);
    }

    TEST_METHOD(TsDegRad_float) { TsDegRad<float>(); }
    TEST_METHOD(TsDegRad_double) { TsDegRad<double>(); }

    TEST_METHOD(TsDegIn0To360)
    {
        Assert::AreEqual(350.0, DegreeIn0To360<double>(-10.0), 1e-12);
        Assert::AreEqual(10.0, DegreeIn0To360<double>(370.0), 1e-12);
        Assert::AreEqual(0.0, DegreeIn0To360<double>(0.0), 1e-12);
        Assert::AreEqual(45.0, DegreeIn0To360<double>(360.0 + 45.0), 1e-12);
        Assert::AreEqual(315.f, DegreeIn0To360<float>(-45.f), 1e-4f);
    }

    // 负角度应映射到 [0, 2π)，超出 2π 的角度按周期归一化
    TEST_METHOD(TsRadIn0To2Pi)
    {
        Assert::AreEqual(Pi * 1.5, RadianIn0To2Pi<double>(-Pi / 2), 1e-12);
        Assert::AreEqual(Pi, RadianIn0To2Pi<double>(3 * Pi), 1e-12);
        Assert::AreEqual(0.0, RadianIn0To2Pi<double>(0.0), 1e-12);
        Assert::AreEqual(PiF * 1.5f, RadianIn0To2Pi<float>(-PiF / 2.f), 1e-5f);
    }
};

TEST_CLASS(TsCircle)
{
public:
    // 四个象限方向上的点
    TEST_METHOD(TsPointFromCircleAngle_Cardinal)
    {
        AssertPoint(PointFromCircleAngle(1.f, 0.0), 1.f, 0.f);
        AssertPoint(PointFromCircleAngle(2.f, Pi / 2), 0.f, 2.f);
        AssertPoint(PointFromCircleAngle(1.f, Pi), -1.f, 0.f);
        AssertPoint(PointFromCircleAngle(3.f, -Pi / 2), 0.f, -3.f);
    }

    // 任意角度：点应始终落在半径圆上，且极角等于输入角度
    TEST_METHOD(TsPointFromCircleAngle_OnCircle)
    {
        const float r = 5.f, ag = PiF / 6.f;
        const auto pt = PointFromCircleAngle(r, ag);
        AssertPoint(pt, r * cosf(ag), r * sinf(ag));
        Assert::AreEqual(r, Kw::Length(pt), 1e-5f);
        Assert::AreEqual(ag, atan2f(pt.y, pt.x), 1e-5f);
    }

    TEST_METHOD(TsPointFromCircleAngle_ZeroRadius)
    {
        AssertPoint(PointFromCircleAngle(0.f, PiF / 3), 0.f, 0.f);
    }
};

TEST_CLASS(TsLine)
{
public:
    TEST_METHOD(TsLineLength)
    {
        Assert::AreEqual(5.f, LineLength({ 0.f, 0.f }, { 3.f, 4.f }), 1e-5f);
        Assert::AreEqual(0.f, LineLength({ 7.f, -2.f }, { 7.f, -2.f }), 1e-6f);
        // 与方向无关
        Assert::AreEqual(5.f, LineLength({ 3.f, 4.f }, { 0.f, 0.f }), 1e-5f);
    }

    // k=0 返回 pt1，k=1 返回 pt2，允许外插
    TEST_METHOD(TsPointFromLineScale)
    {
        const Kw::Vector2 pt1{ 0.f, 0.f }, pt2{ 10.f, 20.f };
        AssertPoint(PointFromLineScale(pt1, pt2, 0.f), 0.f, 0.f);
        AssertPoint(PointFromLineScale(pt1, pt2, 1.f), 10.f, 20.f);
        AssertPoint(PointFromLineScale(pt1, pt2, 0.5f), 5.f, 10.f);
        AssertPoint(PointFromLineScale(pt1, pt2, 2.f), 20.f, 40.f);
        AssertPoint(PointFromLineScale(pt2, pt1, 0.25f), 7.5f, 15.f);
    }

    TEST_METHOD(TsLineEquation_TwoPoints)
    {
        const auto ln = CalculateLineEquation(
            Kw::Vector2{ 0.f, 0.f }, Kw::Vector2{ 1.f, 1.f });
        // y = x 上的点都应满足方程
        Assert::AreEqual(0.f, EvalLineEquation(ln, 0.f, 0.f), 1e-6f);
        Assert::AreEqual(0.f, EvalLineEquation(ln, 2.f, 2.f), 1e-6f);
        Assert::AreEqual(0.f, EvalLineEquation(ln, -3.f, -3.f), 1e-6f);
        // 系数为 (y1-y2, x2-x1, ...) 的等价形式：A + B = 0
        Assert::AreEqual(-ln.x, ln.y, 1e-6f);
    }

    // 竖直线：B = 0
    TEST_METHOD(TsLineEquation_Vertical)
    {
        const auto ln = CalculateLineEquation(
            Kw::Vector2{ 2.f, 0.f }, Kw::Vector2{ 2.f, 5.f });
        Assert::AreEqual(0.f, ln.y, 1e-6f);
        Assert::AreEqual(0.f, EvalLineEquation(ln, 2.f, 3.f), 1e-6f);
        Assert::AreEqual(0.f, EvalLineEquation(ln, 2.f, -7.f), 1e-6f);
    }

    // 水平线：A = 0
    TEST_METHOD(TsLineEquation_Horizontal)
    {
        const auto ln = CalculateLineEquation(
            Kw::Vector2{ -1.f, 3.f }, Kw::Vector2{ 4.f, 3.f });
        Assert::AreEqual(0.f, ln.x, 1e-6f);
        Assert::AreEqual(0.f, EvalLineEquation(ln, 0.f, 3.f), 1e-6f);
        Assert::AreEqual(0.f, EvalLineEquation(ln, 100.f, 3.f), 1e-6f);
    }

    // 一点 + 斜率：y = 2x + 3 => 2x - y + 3 = 0
    TEST_METHOD(TsLineEquation_PointSlope)
    {
        const auto ln = CalculateLineEquation(Kw::Vector2{ 0.f, 3.f }, 2.f);
        Assert::AreEqual(2.f, ln.x, 1e-6f);
        Assert::AreEqual(-1.f, ln.y, 1e-6f);
        Assert::AreEqual(3.f, ln.z, 1e-6f);
        Assert::AreEqual(0.f, EvalLineEquation(ln, 1.f, 5.f), 1e-6f);
        Assert::AreEqual(0.f, EvalLineEquation(ln, -3.f, -3.f), 1e-6f);
    }

    // 距离取绝对值：点位于直线两侧时结果相同；按无限长直线计算
    TEST_METHOD(TsPointToLineDistance)
    {
        const Kw::Vector2 a{ 0.f, 0.f }, b{ 4.f, 0.f };
        Assert::AreEqual(3.f, PointToLineDistance({ 0.f, 3.f }, a, b), 1e-5f);
        Assert::AreEqual(3.f, PointToLineDistance({ 0.f, -3.f }, a, b), 1e-5f);
        Assert::AreEqual(0.f, PointToLineDistance({ 7.f, 0.f }, a, b), 1e-6f);
        // 竖直线 x = 2
        Assert::AreEqual(1.f, PointToLineDistance(
            { 1.f, 1.f }, { 2.f, 0.f }, { 2.f, 6.f }), 1e-5f);
    }
};

TEST_CLASS(TsPolarRect)
{
public:
    TEST_METHOD(TsPolarToRect)
    {
        AssertPoint(PolarToRect(Kw::Polar{ 2.0, 0.0 }), 2.f, 0.f);
        AssertPoint(PolarToRect(Kw::Polar{ 2.0, Pi / 2 }), 0.f, 2.f);
        AssertPoint(PolarToRect(Kw::Polar{ 0.0, 1.0 }), 0.f, 0.f);
    }

    TEST_METHOD(TsRectToPolar)
    {
        auto p = RectToPolar(Kw::Vector2{ 0.f, 1.f });
        Assert::AreEqual(1.0, p.Rho, 1e-6);
        Assert::AreEqual(Pi / 2, p.Theta, 1e-6);

        p = RectToPolar(Kw::Vector2{ 3.f, 4.f });
        Assert::AreEqual(5.0, p.Rho, 1e-5);
        Assert::AreEqual(atan2(4.0, 3.0), p.Theta, 1e-6);
    }

    TEST_METHOD(TsPolarRect_RoundTrip)
    {
        const double ags[]{ 0.0, 0.3, Pi / 4, 2.7, -1.2 };
        for (const auto ag : ags)
        {
            const auto p = RectToPolar(PolarToRect(Kw::Polar{ 3.5, ag }));
            Assert::AreEqual(3.5, p.Rho, 1e-5);
            Assert::AreEqual(ag, p.Theta, 1e-5);
        }
    }
};

TEST_CLASS(TsEllipse)
{
public:
    // 四个象限方向上的点：长轴/短轴端点
    TEST_METHOD(TsPointFromEllipseAngle_Cardinal)
    {
        AssertPoint(PointFromEllipseAngle(2.f, 1.f, 0.0), 2.f, 0.f);
        AssertPoint(PointFromEllipseAngle(2.f, 1.f, Pi / 2), 0.f, 1.f);
        AssertPoint(PointFromEllipseAngle(2.f, 1.f, Pi), -2.f, 0.f);
        AssertPoint(PointFromEllipseAngle(2.f, 1.f, Pi * 1.5), 0.f, -1.f);
    }

    // ag 为圆心角：点必须落在该方向的射线上，同时落在椭圆上
    TEST_METHOD(TsPointFromEllipseAngle_OnRayAndEllipse)
    {
        const float a = 2.f, b = 1.f;
        const double ags[]{ 0.3, Pi / 4, 1.2, 2.5 };
        for (const auto ag : ags)
        {
            const auto pt = PointFromEllipseAngle(a, b, ag);
            Assert::AreEqual(float(ag), atan2f(pt.y, pt.x), 1e-5f);
            const float f = pt.x * pt.x / (a * a) + pt.y * pt.y / (b * b);
            Assert::AreEqual(1.f, f, 1e-5f);
        }
    }

    TEST_METHOD(TsArcFromEllipseAngle_Center)
    {
        const Kw::Vector2 c{ 10.f, 20.f };
        const auto arc = ArcFromEllipseAngle(c, 2.f, 1.f, 0.0, Pi / 2);
        AssertPoint(arc[0], 12.f, 20.f);
        AssertPoint(arc[1], 10.f, 21.f);
    }

    // 矩形内切椭圆的中心为矩形中心，半轴为矩形半宽/半高
    TEST_METHOD(TsArcFromEllipseAngle_Rect)
    {
        const Kw::Rect rc{ 0.f, 0.f, 4.f, 2.f };
        const auto arc = ArcFromEllipseAngle(rc, 0.0, Pi / 2);
        AssertPoint(arc[0], 4.f, 1.f);
        AssertPoint(arc[1], 2.f, 2.f);
    }
};

TEST_CLASS(TsBezier)
{
public:
    // 少于两点不会产生任何输出（也不会越界写入）
    TEST_METHOD(TsTooFewPoints_NoChange)
    {
        std::vector<Kw::Vector2> v;
        CalculateBezierControlPoints(v, nullptr, 0);
        Assert::AreEqual(size_t(0), v.size());

        const Kw::Vector2 pt{ 1.f, 2.f };
        CalculateBezierControlPoints(v, &pt, 1);
        Assert::AreEqual(size_t(0), v.size());
    }

    // 两点时退化为直线：{p0, p0, p1, p1}
    TEST_METHOD(TsTwoPoints)
    {
        const Kw::Vector2 pt[]{ { 1.f, 2.f }, { 5.f, 6.f } };
        std::vector<Kw::Vector2> v;
        CalculateBezierControlPoints(v, pt, 2);
        Assert::AreEqual(size_t(4), v.size());
        AssertPoint(v[0], 1.f, 2.f);
        AssertPoint(v[1], 1.f, 2.f);
        AssertPoint(v[2], 5.f, 6.f);
        AssertPoint(v[3], 5.f, 6.f);
    }

    // 共线等距三点、K=1 时，控制点落在相邻线段的中点
    TEST_METHOD(TsCollinear_K1)
    {
        const Kw::Vector2 pt[]{ { 0.f, 0.f }, { 1.f, 0.f }, { 2.f, 0.f } };
        std::vector<Kw::Vector2> v;
        CalculateBezierControlPoints(v, pt, 3, 1.f);
        Assert::AreEqual(size_t(7), v.size());
        const float xExp[]{ 0.f, 0.f, 0.5f, 1.f, 1.5f, 2.f, 2.f };
        for (size_t i = 0; i < v.size(); ++i)
        {
            Assert::AreEqual(xExp[i], v[i].x, 1e-5f);
            Assert::AreEqual(0.f, v[i].y, 1e-5f);
        }
    }

    // 对称输入应产生对称的控制点
    TEST_METHOD(TsSymmetric)
    {
        const Kw::Vector2 pt[]{ { 0.f, 0.f }, { 1.f, 2.f }, { 2.f, 0.f } };
        std::vector<Kw::Vector2> v;
        CalculateBezierControlPoints(v, pt, 3, 1.f);
        Assert::AreEqual(size_t(7), v.size());
        const Kw::Vector2 exp[]
        {
            { 0.f, 0.f }, { 0.f, 0.f }, { 0.5f, 2.f }, { 1.f, 2.f },
            { 1.5f, 2.f }, { 2.f, 0.f }, { 2.f, 0.f }
        };
        for (size_t i = 0; i < v.size(); ++i)
            AssertPoint(v[i], exp[i].x, exp[i].y);
    }

    // 结果追加到既有内容之后
    TEST_METHOD(TsAppendToExisting)
    {
        const Kw::Vector2 pt[]{ { 1.f, 2.f }, { 5.f, 6.f } };
        std::vector<Kw::Vector2> v{ { 99.f, 99.f } };
        CalculateBezierControlPoints(v, pt, 2);
        Assert::AreEqual(size_t(5), v.size());
        AssertPoint(v[0], 99.f, 99.f);
        AssertPoint(v[1], 1.f, 2.f);
        AssertPoint(v[2], 1.f, 2.f);
        AssertPoint(v[3], 5.f, 6.f);
        AssertPoint(v[4], 5.f, 6.f);
    }

    // 曲线的锚点必须依次落在输入的离散点上
    TEST_METHOD(TsAnchorsMatchInput)
    {
        const Kw::Vector2 pt[]
        {
            { 0.f, 0.f }, { 10.f, 5.f }, { 20.f, -5.f }, { 30.f, 0.f }, { 40.f, 10.f }
        };
        const size_t cPt = std::size(pt);
        std::vector<Kw::Vector2> v;
        CalculateBezierControlPoints(v, pt, cPt);
        Assert::AreEqual((cPt - 1) * 3 + 1, v.size());
        for (size_t i = 0; i < cPt; ++i)
        {
            const size_t idx = (i == 0) ? 0 : (i == cPt - 1) ? v.size() - 2 : 3 * i;
            AssertPoint(v[idx], pt[i].x, pt[i].y);
        }
    }

    // 相邻点重合时不应产生 NaN
    TEST_METHOD(TsDuplicatePoints_FiniteResult)
    {
        const Kw::Vector2 pt[]{ { 0.f, 0.f }, { 0.f, 0.f }, { 1.f, 0.f } };
        std::vector<Kw::Vector2> v;
        CalculateBezierControlPoints(v, pt, 3);
        Assert::AreEqual(size_t(7), v.size());
        for (const auto& p : v)
            Assert::IsTrue(std::isfinite(p.x) && std::isfinite(p.y));
    }
};

TEST_CLASS(TsXForm)
{
public:
    TEST_METHOD(TsTranslate)
    {
        float x, y;
        ApplyXForm(XFormTranslate(10.f, -20.f), 1.f, 1.f, x, y);
        Assert::AreEqual(11.f, x, 1e-6f);
        Assert::AreEqual(-19.f, y, 1e-6f);
    }

    TEST_METHOD(TsRotate)
    {
        const auto tf = XFormRotate(Pi / 2);
        Assert::AreEqual(0.f, tf.eM11, 1e-6f);
        Assert::AreEqual(1.f, tf.eM12, 1e-6f);
        Assert::AreEqual(-1.f, tf.eM21, 1e-6f);
        Assert::AreEqual(0.f, tf.eM22, 1e-6f);
        Assert::AreEqual(0.f, tf.eDx, 1e-6f);
        Assert::AreEqual(0.f, tf.eDy, 1e-6f);

        float x, y;
        ApplyXForm(tf, 1.f, 0.f, x, y);   // +X 轴 -> +Y 轴
        Assert::AreEqual(0.f, x, 1e-6f);
        Assert::AreEqual(1.f, y, 1e-6f);
        ApplyXForm(tf, 0.f, 1.f, x, y);   // +Y 轴 -> -X 轴
        Assert::AreEqual(-1.f, x, 1e-6f);
        Assert::AreEqual(0.f, y, 1e-6f);
        ApplyXForm(tf, 0.f, 0.f, x, y);   // 原点不动
        Assert::AreEqual(0.f, x, 1e-6f);
        Assert::AreEqual(0.f, y, 1e-6f);
    }

    TEST_METHOD(TsRotateAboutPoint)
    {
        const auto tf = XFormRotate(Pi / 2, 5.f, 5.f);
        float x, y;
        ApplyXForm(tf, 5.f, 5.f, x, y);
        Assert::AreEqual(5.f, x, 1e-5f);
        Assert::AreEqual(5.f, y, 1e-5f);
        ApplyXForm(tf, 6.f, 5.f, x, y);   // 中心右侧1单位 -> 中心下方1单位
        Assert::AreEqual(5.f, x, 1e-5f);
        Assert::AreEqual(6.f, y, 1e-5f);
    }

    TEST_METHOD(TsScale)
    {
        float x, y;
        ApplyXForm(XFormScale(2.f, 3.f), 1.f, 1.f, x, y);
        Assert::AreEqual(2.f, x, 1e-6f);
        Assert::AreEqual(3.f, y, 1e-6f);

        const auto tf = XFormScale(2.f, 2.f, 5.f, 5.f);
        ApplyXForm(tf, 5.f, 5.f, x, y);   // 中心不动
        Assert::AreEqual(5.f, x, 1e-6f);
        Assert::AreEqual(5.f, y, 1e-6f);
        ApplyXForm(tf, 6.f, 7.f, x, y);
        Assert::AreEqual(7.f, x, 1e-6f);
        Assert::AreEqual(9.f, y, 1e-6f);
    }

    TEST_METHOD(TsShear)
    {
        float x, y;
        ApplyXForm(XFormShear(1.f, 0.f), 1.f, 2.f, x, y); // Y' = X + Y
        Assert::AreEqual(1.f, x, 1e-6f);
        Assert::AreEqual(3.f, y, 1e-6f);

        const auto tf = XFormShear(1.f, 0.f, 2.f, 3.f);
        ApplyXForm(tf, 2.f, 3.f, x, y);   // 中心不动
        Assert::AreEqual(2.f, x, 1e-6f);
        Assert::AreEqual(3.f, y, 1e-6f);
        ApplyXForm(tf, 2.f, 4.f, x, y);
        Assert::AreEqual(2.f, x, 1e-6f);
        Assert::AreEqual(4.f, y, 1e-6f);
    }

    // 关于坐标轴和直线 y = x 的反射
    TEST_METHOD(TsReflection)
    {
        float x, y;
        ApplyXForm(XFormReflection(0.f, 1.f, 0.f), 1.f, 2.f, x, y);
        Assert::AreEqual(1.f, x, 1e-6f);
        Assert::AreEqual(-2.f, y, 1e-6f);

        ApplyXForm(XFormReflection(1.f, 0.f, 0.f), 1.f, 2.f, x, y);
        Assert::AreEqual(-1.f, x, 1e-6f);
        Assert::AreEqual(2.f, y, 1e-6f);

        ApplyXForm(XFormReflection(1.f, -1.f, 0.f), 1.f, 2.f, x, y);
        Assert::AreEqual(2.f, x, 1e-6f);
        Assert::AreEqual(1.f, y, 1e-6f);
    }

    // D2D 版本与 XFORM 版本系数一致
    TEST_METHOD(TsD2DMatrixReflection_MatchesXForm)
    {
        const float A = 1.f, B = -1.f, C = 0.f;
        const auto tf = XFormReflection(A, B, C);
        const auto m = D2DMatrixReflection(A, B, C);
        Assert::AreEqual(tf.eM11, m._11, 1e-6f);
        Assert::AreEqual(tf.eM12, m._12, 1e-6f);
        Assert::AreEqual(tf.eM21, m._21, 1e-6f);
        Assert::AreEqual(tf.eM22, m._22, 1e-6f);
        Assert::AreEqual(tf.eDx, m._31, 1e-6f);
        Assert::AreEqual(tf.eDy, m._32, 1e-6f);
    }
};

TEST_CLASS(TsRadianInArc)
{
public:
    // 测试完整圆弧（扫描角度 >= 2π）
    TEST_METHOD(TsFullCirclePositive)
    {
        Assert::IsTrue(RadianInArc(0.0, 0.0, 2 * Pi));
        Assert::IsTrue(RadianInArc(Pi, 0.0, 2 * Pi));
        Assert::IsTrue(RadianInArc(-Pi, 0.0, 2 * Pi));
        Assert::IsTrue(RadianInArc(3.5, 1.0, 3 * Pi));
    }

    TEST_METHOD(TsFullCircleNegative)
    {
        Assert::IsTrue(RadianInArc(0.0, 0.0, -2 * Pi));
        Assert::IsTrue(RadianInArc(Pi, 0.0, -2 * Pi));
        Assert::IsTrue(RadianInArc(-Pi, 0.0, -2 * Pi));
    }

    // 测试正向扫描（逆时针）
    TEST_METHOD(TsPositiveSweepInRange)
    {
        // 从0开始，扫描π/2
        Assert::IsTrue(RadianInArc(0.0, 0.0, Pi / 2));
        Assert::IsTrue(RadianInArc(Pi / 4, 0.0, Pi / 2));
        Assert::IsTrue(RadianInArc(Pi / 2, 0.0, Pi / 2));
    }

    TEST_METHOD(TsPositiveSweepOutOfRange)
    {
        // 从0开始，扫描π/2
        Assert::IsFalse(RadianInArc(-0.1, 0.0, Pi / 2));
        Assert::IsFalse(RadianInArc(Pi / 2 + 0.1, 0.0, Pi / 2));
        Assert::IsFalse(RadianInArc(Pi, 0.0, Pi / 2));
    }

    // 测试负向扫描（顺时针）
    TEST_METHOD(TsNegativeSweepInRange)
    {
        // 从0开始，扫描-π/2
        Assert::IsTrue(RadianInArc(0.0, 0.0, -Pi / 2));
        Assert::IsTrue(RadianInArc(-Pi / 4, 0.0, -Pi / 2));
        Assert::IsTrue(RadianInArc(-Pi / 2, 0.0, -Pi / 2));
    }

    TEST_METHOD(TsNegativeSweepOutOfRange)
    {
        // 从0开始，扫描-π/2
        Assert::IsFalse(RadianInArc(0.1, 0.0, -Pi / 2));
        Assert::IsFalse(RadianInArc(-Pi / 2 - 0.1, 0.0, -Pi / 2));
        Assert::IsFalse(RadianInArc(Pi, 0.0, -Pi / 2));
    }

    // 测试跨越0度的情况
    TEST_METHOD(TsCrossingZero)
    {
        // 从-π/4开始，扫描π/2，应该包含0
        Assert::IsTrue(RadianInArc(0.0, -Pi / 4, Pi / 2));
        Assert::IsTrue(RadianInArc(-Pi / 4, -Pi / 4, Pi / 2));
        Assert::IsTrue(RadianInArc(Pi / 4, -Pi / 4, Pi / 2));
        Assert::IsFalse(RadianInArc(Pi / 2, -Pi / 4, Pi / 2));
    }

    // 测试跨越±π边界的情况
    TEST_METHOD(TsCrossingPiBoundary)
    {
        // 从π/2开始，扫描π，应该跨越π/-π边界
        Assert::IsTrue(RadianInArc(Pi / 2, Pi / 2, Pi));
        Assert::IsTrue(RadianInArc(Pi, Pi / 2, Pi));
        Assert::IsTrue(RadianInArc(-Pi + 0.5, Pi / 2, Pi));
        Assert::IsTrue(RadianInArc(-Pi / 2, Pi / 2, Pi));
    }

    // 测试不同起始角度
    TEST_METHOD(TsDifferentStartAngles)
    {
        // 从π/4开始，扫描π/2
        Assert::IsTrue(RadianInArc(Pi / 4, Pi / 4, Pi / 2));
        Assert::IsTrue(RadianInArc(Pi / 2, Pi / 4, Pi / 2));
        Assert::IsTrue(RadianInArc(3 * Pi / 4, Pi / 4, Pi / 2));
        Assert::IsFalse(RadianInArc(0.0, Pi / 4, Pi / 2));
        Assert::IsFalse(RadianInArc(Pi, Pi / 4, Pi / 2));
    }

    // 测试大于2π的角度输入（周期性）
    TEST_METHOD(TsAngleNormalization)
    {
        // ag = 2π + π/4 应该等价于 π/4
        Assert::IsTrue(RadianInArc(2 * Pi + Pi / 4, 0.0, Pi / 2));
        Assert::IsTrue(RadianInArc(-2 * Pi + Pi / 4, 0.0, Pi / 2));

        // 起始角度也可以大于2π
        Assert::IsTrue(RadianInArc(Pi / 4, 2 * Pi, Pi / 2));
    }

    // 边界条件测试
    TEST_METHOD(TsBoundaryConditions)
    {
        const double epsilon = 1e-10;

        // 恰好在边界上
        Assert::IsTrue(RadianInArc(0.0, 0.0, Pi));
        Assert::IsTrue(RadianInArc(Pi, 0.0, Pi));

        // 刚好超出边界
        Assert::IsFalse(RadianInArc(Pi + epsilon, 0.0, Pi));
        Assert::IsFalse(RadianInArc(-epsilon, 0.0, Pi));
    }

    // 零扫描角度
    TEST_METHOD(TsZeroSweep)
    {
        Assert::IsTrue(RadianInArc(0.0, 0.0, 0.0));
        Assert::IsFalse(RadianInArc(0.1, 0.0, 0.0));
        Assert::IsFalse(RadianInArc(-0.1, 0.0, 0.0));
    }

    // 测试float类型
    TEST_METHOD(TsFloatType)
    {
        Assert::IsTrue(RadianInArc(0.0f, 0.0f, static_cast<float>(Pi / 2)));
        Assert::IsTrue(RadianInArc(static_cast<float>(Pi / 4), 0.0f, static_cast<float>(Pi / 2)));
        Assert::IsFalse(RadianInArc(static_cast<float>(Pi), 0.0f, static_cast<float>(Pi / 2)));
    }

    // 逆时针完整测试
    TEST_METHOD(TsCounterClockwiseArc)
    {
        // 从90度开始，逆时针扫描180度
        double start = Pi / 2;
        double sweep = Pi;

        Assert::IsTrue(RadianInArc(Pi / 2, start, sweep));      // 起点
        Assert::IsTrue(RadianInArc(Pi, start, sweep));          // 中点
        Assert::IsTrue(RadianInArc(3 * Pi / 2, start, sweep));  // 终点
        Assert::IsFalse(RadianInArc(0.0, start, sweep));        // 在弧外
        Assert::IsFalse(RadianInArc(Pi / 4, start, sweep));     // 在弧外
    }

    // 顺时针完整测试
    TEST_METHOD(TsClockwiseArc)
    {
        // 从90度开始，顺时针扫描180度
        double start = Pi / 2;
        double sweep = -Pi;

        Assert::IsTrue(RadianInArc(Pi / 2, start, sweep));      // 起点
        Assert::IsTrue(RadianInArc(0.0, start, sweep));         // 中点
        Assert::IsTrue(RadianInArc(-Pi / 2, start, sweep));     // 终点
        Assert::IsFalse(RadianInArc(Pi, start, sweep));         // 在弧外
        Assert::IsFalse(RadianInArc(3 * Pi / 4, start, sweep)); // 在弧外
    }
};

namespace
{
    // 按 Direct2D 的行向量语义应用 4x4（XMFLOAT4X4 与 D2D1_MATRIX_4X4_F 成员同名，可直接复用）：
    //   X = x*_11 + y*_21 + _41，Y = x*_12 + y*_22 + _42，w = x*_14 + y*_24 + _44
    template<class M44>
    void ApplyRowVector44(const M44& m, float x, float y,
        _Out_ float& ox, _Out_ float& oy, _Out_ float& w) noexcept
    {
        const float X = x * m._11 + y * m._21 + m._41;
        const float Y = x * m._12 + y * m._22 + m._42;
        w = x * m._14 + y * m._24 + m._44;
        ox = X / w;
        oy = Y / w;
    }

    // 按 Kw 的行向量语义应用 3x3：p' = (x, y, 1) * H
    void ApplyH33(const Kw::Matrix3x3& h, float x, float y,
        _Out_ float& ox, _Out_ float& oy) noexcept
    {
        const float X = x * h(0, 0) + y * h(1, 0) + h(2, 0);
        const float Y = x * h(0, 1) + y * h(1, 1) + h(2, 1);
        const float W = x * h(0, 2) + y * h(1, 2) + h(2, 2);
        ox = X / W;
        oy = Y / W;
    }

    // 4x4 必须把 ptSrc 的四个点依次映射到 ptDst 的四个点
    template<class M44>
    void AssertMaps4(const M44& m,
        const Kw::Vector2* ptSrc, const Kw::Vector2* ptDst, float tol)
    {
        for (int i = 0; i < 4; ++i)
        {
            float x, y, w;
            ApplyRowVector44(m, ptSrc[i].x, ptSrc[i].y, x, y, w);
            Assert::IsTrue(fabsf(w) > 1e-6f, L"w 分量不得为 0");
            Assert::AreEqual(ptDst[i].x, x, tol);
            Assert::AreEqual(ptDst[i].y, y, tol);
        }
    }

    void AssertIsIdentity4x4(const DirectX::XMFLOAT4X4& m, float tol)
    {
        Assert::AreEqual(1.f, m._11, tol); Assert::AreEqual(0.f, m._12, tol);
        Assert::AreEqual(0.f, m._13, tol); Assert::AreEqual(0.f, m._14, tol);
        Assert::AreEqual(0.f, m._21, tol); Assert::AreEqual(1.f, m._22, tol);
        Assert::AreEqual(0.f, m._23, tol); Assert::AreEqual(0.f, m._24, tol);
        Assert::AreEqual(0.f, m._31, tol); Assert::AreEqual(0.f, m._32, tol);
        Assert::AreEqual(1.f, m._33, tol); Assert::AreEqual(0.f, m._34, tol);
        Assert::AreEqual(0.f, m._41, tol); Assert::AreEqual(0.f, m._42, tol);
        Assert::AreEqual(0.f, m._43, tol); Assert::AreEqual(1.f, m._44, tol);
    }

    // 用双线性插值取凸四边形内部点（角点顺序 TL,TR,BL,BR）
    Kw::Vector2 SampleQuad(const Kw::Vector2* q, float s, float t) noexcept
    {
        const float a = (1.f - s) * (1.f - t), b = s * (1.f - t),
            c = (1.f - s) * t, d = s * t;
        return
        {
            q[0].x * a + q[1].x * b + q[2].x * c + q[3].x * d,
            q[0].y * a + q[1].y * b + q[2].y * c + q[3].y * d
        };
    }
}

// CalculateDistortMatrix / HomographyUnitSquareToQuadrilateral 的行为契约：
//   1) 单位正方形的四个规范角点 (0,0)(1,0)(1,1)(0,1) 依次对应四边形四点
//   2) CalculateDistortMatrix 输出的 4x4 在 Direct2D 行向量语义下
//      把 ptSrc 的四个点依次映射到 ptDst 的四个点
//   3) 矩形重载的角点顺序为 左上、右上、左下、右下
TEST_CLASS(TsDistortMatrix)
{
public:
    // 规范单位正方形 -> 自身应为单位矩阵
    TEST_METHOD(TsUnitSquareToQuad_Identity)
    {
        const Kw::Vector2 pt[4]
        {
            { 0.f, 0.f }, { 1.f, 0.f }, { 1.f, 1.f }, { 0.f, 1.f }
        };
        const auto H = Detail::HomographyUnitSquareToQuadrilateral(pt);
        const auto I = Kw::IdentityMatrix<3>();
        for (size_t i = 0; i < 3; ++i)
            for (size_t j = 0; j < 3; ++j)
                Assert::AreEqual(I(i, j), H(i, j), 1e-6f);
    }

    // 单位正方形的四个规范角点应精确映射到给定四边形
    TEST_METHOD(TsUnitSquareToQuad_MapsCorners)
    {
        const Kw::Vector2 pt[4]
        {
            { 10.f, 20.f }, { 110.f, 30.f }, { 90.f, 220.f }, { 5.f, 180.f }
        };
        const Kw::Vector2 ptUnit[4]
        {
            { 0.f, 0.f }, { 1.f, 0.f }, { 1.f, 1.f }, { 0.f, 1.f }
        };

        const auto H = Detail::HomographyUnitSquareToQuadrilateral(pt);
        for (int i = 0; i < 4; ++i)
        {
            float x, y;
            ApplyH33(H, ptUnit[i].x, ptUnit[i].y, x, y);
            Assert::AreEqual(pt[i].x, x, 1e-3f);
            Assert::AreEqual(pt[i].y, y, 1e-3f);
        }
    }

    // 一般四边形 -> 一般四边形
    TEST_METHOD(TsQuadToQuad_MapsCorners)
    {
        const Kw::Vector2 ptSrc[4]
        {
            { 0.f, 0.f }, { 256.f, 0.f }, { 0.f, 256.f }, { 256.f, 256.f }
        };
        const Kw::Vector2 ptDst[4]
        {
            { 120.f, 60.f }, { 700.f, 90.f }, { 60.f, 540.f }, { 720.f, 500.f }
        };

        DirectX::XMFLOAT4X4 m{};
        CalculateDistortMatrix(ptSrc, ptDst, m);
        AssertMaps4(m, ptSrc, ptDst, 1e-2f);
    }

    // 不要求固定角点顺序：src/dst 用**同一个**任意置换重排后，映射结果不变
    // （四点对应关系没变，而四组点对唯一确定一个 homography）
    TEST_METHOD(TsOrderIsNotFixed_AnyPermutation)
    {
        const Kw::Vector2 src[4]
        {
            { 0.f, 0.f }, { 256.f, 0.f }, { 0.f, 256.f }, { 256.f, 256.f }
        };
        const Kw::Vector2 dst[4]
        {
            { 120.f, 60.f }, { 700.f, 90.f }, { 60.f, 540.f }, { 720.f, 500.f }
        };

        DirectX::XMFLOAT4X4 m0{};
        CalculateDistortMatrix(src, dst, m0);

        constexpr int perm[4]{ 2, 0, 3, 1 }; // 任意置换
        Kw::Vector2 srcP[4], dstP[4];
        for (int i = 0; i < 4; ++i)
        {
            srcP[i] = src[perm[i]];
            dstP[i] = dst[perm[i]];
        }

        DirectX::XMFLOAT4X4 m1{};
        CalculateDistortMatrix(srcP, dstP, m1);

        // 不仅四角，内部采样点也必须落在同一位置（矩阵可能整体差一个尺度因子）
        for (int j = 0; j <= 4; ++j)
            for (int i = 0; i <= 4; ++i)
            {
                const auto p = SampleQuad(src, i / 4.f, j / 4.f);
                float x0, y0, w0, x1, y1, w1;
                ApplyRowVector44(m0, p.x, p.y, x0, y0, w0);
                ApplyRowVector44(m1, p.x, p.y, x1, y1, w1);
                Assert::AreEqual(x0, x1, 0.05f);
                Assert::AreEqual(y0, y1, 0.05f);
            }
    }

    // “围一圈”的循环序（TL,TR,BR,BL）同样有效，不是必须用旧 API 的 TL,TR,BL,BR
    TEST_METHOD(TsCyclicOrder_AlsoValid)
    {
        const Kw::Vector2 src[4]
        {
            { 0.f, 0.f }, { 256.f, 0.f }, { 256.f, 256.f }, { 0.f, 256.f }
        };
        const Kw::Vector2 dst[4]
        {
            { 120.f, 60.f }, { 700.f, 90.f }, { 720.f, 500.f }, { 60.f, 540.f }
        };

        DirectX::XMFLOAT4X4 m{};
        CalculateDistortMatrix(src, dst, m);
        AssertMaps4(m, src, dst, 1e-2f);
    }

    // 交叉对应（两边角点的相对顺序不一致）：四点仍被精确映射，
    // 但图像在内部折叠 —— 实现不做顺序检查，这是调用方的责任
    TEST_METHOD(TsCrossedCorrespondence_StillMapsCorners)
    {
        const Kw::Vector2 src[4] // 围一圈 TL,TR,BR,BL
        {
            { 0.f, 0.f }, { 256.f, 0.f }, { 256.f, 256.f }, { 0.f, 256.f }
        };
        const Kw::Vector2 dst[4] // 相对 src 是交叉的（BR 与 BL 对调）
        {
            { 120.f, 60.f }, { 700.f, 90.f }, { 60.f, 540.f }, { 720.f, 500.f }
        };

        DirectX::XMFLOAT4X4 m{};
        CalculateDistortMatrix(src, dst, m);

        // 四角依然严格满足 i -> i，说明实现只按索引对应，不检查拓扑
        AssertMaps4(m, src, dst, 1e-2f);
    }

    // 矩形 -> 四边形（D2D 重载），矩形角点顺序 TL,TR,BL,BR，且矩形不在原点
    TEST_METHOD(TsRectToQuad_MapsCorners)
    {
        const D2D1_RECT_F rc{ 10.f, 20.f, 266.f, 276.f };
        const D2D1_POINT_2F ptDstD2D[4]
        {
            { 120.f, 60.f }, { 700.f, 90.f }, { 60.f, 540.f }, { 720.f, 500.f }
        };
        const Kw::Vector2 ptSrc[4]
        {
            { 10.f, 20.f }, { 266.f, 20.f }, { 10.f, 276.f }, { 266.f, 276.f }
        };
        const Kw::Vector2 ptDst[4]
        {
            { 120.f, 60.f }, { 700.f, 90.f }, { 60.f, 540.f }, { 720.f, 500.f }
        };

        D2D1_MATRIX_4X4_F m{};
        CalculateDistortMatrix(rc, ptDstD2D, m);
        AssertMaps4(m, ptSrc, ptDst, 1e-2f);
    }

    // 四边形 -> 矩形（D2D 重载）
    TEST_METHOD(TsQuadToRect_MapsCorners)
    {
        const D2D1_POINT_2F ptSrcD2D[4]
        {
            { 120.f, 60.f }, { 700.f, 90.f }, { 60.f, 540.f }, { 720.f, 500.f }
        };
        const D2D1_RECT_F rc{ 0.f, 0.f, 256.f, 256.f };
        const Kw::Vector2 ptSrc[4]
        {
            { 120.f, 60.f }, { 700.f, 90.f }, { 60.f, 540.f }, { 720.f, 500.f }
        };
        const Kw::Vector2 ptDst[4]
        {
            { 0.f, 0.f }, { 256.f, 0.f }, { 0.f, 256.f }, { 256.f, 256.f }
        };

        D2D1_MATRIX_4X4_F m{};
        CalculateDistortMatrix(ptSrcD2D, rc, m);
        AssertMaps4(m, ptSrc, ptDst, 1e-2f);
    }

    // 矩形 -> 矩形必须是纯仿射（w 行恒为 (0,0,1)）
    TEST_METHOD(TsRectToRect_IsAffine)
    {
        const Kw::Rect rcSrc{ 0.f, 0.f, 10.f, 20.f };
        const Kw::Vector2 ptDst[4]
        {
            { 100.f, 50.f }, { 130.f, 50.f }, { 100.f, 90.f }, { 130.f, 90.f }
        };

        DirectX::XMFLOAT4X4 m{};
        CalculateDistortMatrix(rcSrc, ptDst, m);

        // X' = 3x + 100，Y' = 2y + 50
        Assert::AreEqual(3.f, m._11, 1e-5f);
        Assert::AreEqual(0.f, m._12, 1e-5f);
        Assert::AreEqual(0.f, m._14, 1e-5f);
        Assert::AreEqual(0.f, m._21, 1e-5f);
        Assert::AreEqual(2.f, m._22, 1e-5f);
        Assert::AreEqual(0.f, m._24, 1e-5f);
        Assert::AreEqual(100.f, m._41, 1e-5f);
        Assert::AreEqual(50.f, m._42, 1e-5f);
        Assert::AreEqual(1.f, m._44, 1e-5f);

        // 内部点同样满足
        float x, y, w;
        ApplyRowVector44(m, 5.f, 10.f, x, y, w);
        Assert::AreEqual(115.f, x, 1e-5f);
        Assert::AreEqual(70.f, y, 1e-5f);
    }

    // 源与目标相同时应得到单位矩阵
    // 容差按绝对量级取 1e-4：中间量（adj/det）在 700 级坐标下约为 1e5~1e6，
    // float 尾数 24 位对应的绝对误差就在 1e-5 量级（实测 _41 ≈ -3e-5）
    TEST_METHOD(TsSameSrcDst_IsIdentity)
    {
        const Kw::Vector2 pt[4]
        {
            { 120.f, 60.f }, { 700.f, 90.f }, { 60.f, 540.f }, { 720.f, 500.f }
        };
        DirectX::XMFLOAT4X4 m{};
        CalculateDistortMatrix(pt, pt, m);
        AssertIsIdentity4x4(m, 1e-4f);
    }

    // Direct2D 按 w > 0 裁剪，源四边形内部 w 必须同号为正
    TEST_METHOD(TsWPositiveOverSource)
    {
        const Kw::Vector2 ptSrc[4]
        {
            { 0.f, 0.f }, { 1920.f, 0.f }, { 0.f, 1080.f }, { 1920.f, 1080.f }
        };
        const Kw::Vector2 ptDst[4]
        {
            { 12.f, 8.f }, { 1901.f, 47.f }, { -30.f, 1052.f }, { 1888.f, 1041.f }
        };

        DirectX::XMFLOAT4X4 m{};
        CalculateDistortMatrix(ptSrc, ptDst, m);

        for (int j = 0; j <= 4; ++j)
            for (int i = 0; i <= 4; ++i)
            {
                const auto p = SampleQuad(ptSrc, i / 4.f, j / 4.f);
                const float w = p.x * m._14 + p.y * m._24 + m._44;
                Assert::IsTrue(w > 0.f, L"源区域内 w 必须为正，否则 D2D 会整块裁剪");
            }
    }

    // 大坐标（1920x1080）下仍保持亚像素精度
    TEST_METHOD(TsLargeCoordinates_Precision)
    {
        const Kw::Vector2 ptSrc[4]
        {
            { 0.f, 0.f }, { 1920.f, 0.f }, { 0.f, 1080.f }, { 1920.f, 1080.f }
        };
        const Kw::Vector2 ptDst[4]
        {
            { 12.f, 8.f }, { 1901.f, 47.f }, { -30.f, 1052.f }, { 1888.f, 1041.f }
        };

        DirectX::XMFLOAT4X4 m{};
        CalculateDistortMatrix(ptSrc, ptDst, m);
        AssertMaps4(m, ptSrc, ptDst, 0.05f);
    }

    // 正变换 + 逆变换应回到原点
    TEST_METHOD(TsRoundTrip)
    {
        const Kw::Vector2 ptQuad[4]
        {
            { 120.f, 60.f }, { 700.f, 90.f }, { 60.f, 540.f }, { 720.f, 500.f }
        };
        const Kw::Rect rc{ 0.f, 0.f, 256.f, 256.f };
        const Kw::Vector2 ptRect[4]
        {
            { 0.f, 0.f }, { 256.f, 0.f }, { 0.f, 256.f }, { 256.f, 256.f }
        };

        DirectX::XMFLOAT4X4 fwd{}, inv{};
        CalculateDistortMatrix(rc, ptQuad, fwd);
        CalculateDistortMatrix(ptQuad, rc, inv);

        for (int j = 0; j <= 4; ++j)
            for (int i = 0; i <= 4; ++i)
            {
                const auto p = SampleQuad(ptRect, i / 4.f, j / 4.f);
                float x1, y1, w1, x2, y2, w2;
                ApplyRowVector44(fwd, p.x, p.y, x1, y1, w1);
                ApplyRowVector44(inv, x1, y1, x2, y2, w2);
                Assert::AreEqual(p.x, x2, 0.05f);
                Assert::AreEqual(p.y, y2, 0.05f);
            }
    }

    // 组合律：A -> B 再 B -> C 应等于 A -> C
    TEST_METHOD(TsCompose)
    {
        const Kw::Vector2 ptA[4]
        {
            { 0.f, 0.f }, { 800.f, 0.f }, { 0.f, 600.f }, { 800.f, 600.f }
        };
        const Kw::Vector2 ptB[4]
        {
            { 50.f, 30.f }, { 760.f, 90.f }, { 10.f, 570.f }, { 790.f, 540.f }
        };
        const Kw::Vector2 ptC[4]
        {
            { 900.f, 400.f }, { 1700.f, 300.f }, { 800.f, 1000.f }, { 1600.f, 1100.f }
        };

        DirectX::XMFLOAT4X4 mAB{}, mBC{}, mAC{};
        CalculateDistortMatrix(ptA, ptB, mAB);
        CalculateDistortMatrix(ptB, ptC, mBC);
        CalculateDistortMatrix(ptA, ptC, mAC);

        for (int j = 0; j <= 4; ++j)
            for (int i = 0; i <= 4; ++i)
            {
                const auto p = SampleQuad(ptA, i / 4.f, j / 4.f);
                float x1, y1, w1, x2, y2, w2, x3, y3, w3;
                ApplyRowVector44(mAB, p.x, p.y, x1, y1, w1);
                ApplyRowVector44(mBC, x1, y1, x2, y2, w2);
                ApplyRowVector44(mAC, p.x, p.y, x3, y3, w3);
                Assert::AreEqual(x3, x2, 0.2f);
                Assert::AreEqual(y3, y2, 0.2f);
            }
    }
};
TS_NS_END