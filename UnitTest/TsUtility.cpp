#include "pch.h"
#include "../eck/KwDefine.h"
#include "../eck/Utility.h"

using namespace eck;

TS_NS_BEGIN
// 取整工具：目标类型由显式模板参数指定，行为应与 floorf/ceilf/roundf 一致
TEST_CLASS(TsUtilityRounding)
{
public:
    TEST_METHOD(TsMatchesStd)
    {
        const float vals[]
        {
            -3.9f, -2.5f, -2.1f, -1.f, -0.5f, -0.1f,
            0.f, 0.1f, 0.5f, 1.f, 2.5f, 3.9f
        };
        for (const auto x : vals)
        {
            Assert::AreEqual((int)floorf(x), Floor<int>(x));
            Assert::AreEqual((int)ceilf(x), Ceil<int>(x));
            // roundf 同样是 0.5 远离零
            Assert::AreEqual((int)roundf(x), Round<int>(x));
        }
    }

    TEST_METHOD(TsFloorCeil)
    {
        Assert::AreEqual(2, Floor<int>(2.9f));
        Assert::AreEqual(-3, Floor<int>(-2.1f));
        Assert::AreEqual(0, Floor<int>(0.f));
        Assert::AreEqual(3, Ceil<int>(2.1f));
        Assert::AreEqual(-2, Ceil<int>(-2.9f));
        Assert::AreEqual(0, Ceil<int>(0.f));
        // 整数值不变
        Assert::AreEqual(5, Floor<int>(5.f));
        Assert::AreEqual(-5, Ceil<int>(-5.f));
    }

    TEST_METHOD(TsRoundHalfAwayFromZero)
    {
        Assert::AreEqual(3, Round<int>(2.5f));
        Assert::AreEqual(-3, Round<int>(-2.5f));
        Assert::AreEqual(1, Round<int>(0.5f));
        Assert::AreEqual(-1, Round<int>(-0.5f));
        Assert::AreEqual(2, Round<int>(2.4f));
        Assert::AreEqual(-2, Round<int>(-2.4f));
    }

    // 目标类型可以是无符号或更宽的整数类型
    TEST_METHOD(TsTargetType)
    {
        Assert::AreEqual(2u, Floor<UINT>(2.7f));
        Assert::AreEqual(3u, Ceil<UINT>(2.1f));
        Assert::AreEqual(-3L, Floor<LONG>(-2.7));
        Assert::AreEqual(-2L, Ceil<LONG>(-2.7));
        Assert::AreEqual(3LL, Round<long long>(2.5));
    }

    // *F 版本取整后仍返回浮点类型
    TEST_METHOD(TsFloatPreserving)
    {
        Assert::AreEqual(-3.f, FloorF<int>(-2.5f), 1e-6f);
        Assert::AreEqual(-2.f, CeilF<int>(-2.5f), 1e-6f);
        Assert::AreEqual(2.f, RoundF<int>(1.5f), 1e-6f);
    }
};

// CeilRect 依赖上面的取整工具（同时验证声明顺序可用）
TEST_CLASS(TsUtilityRect)
{
public:
    TEST_METHOD(TsCeilRect_Float)
    {
        Kw::Rect rc{ 0.2f, 1.7f, 5.1f, 9.9f };
        CeilRect(rc);
        Assert::AreEqual(0.f, rc.left, 1e-6f);
        Assert::AreEqual(1.f, rc.top, 1e-6f);
        Assert::AreEqual(6.f, rc.right, 1e-6f);
        Assert::AreEqual(10.f, rc.bottom, 1e-6f);
    }

    TEST_METHOD(TsCeilRect_ToRECT)
    {
        const Kw::Rect rc{ 0.2f, 1.7f, 5.1f, 9.9f };
        RECT rcOut{};
        CeilRect(rc, rcOut);
        Assert::AreEqual(0L, rcOut.left);
        Assert::AreEqual(1L, rcOut.top);
        Assert::AreEqual(6L, rcOut.right);
        Assert::AreEqual(10L, rcOut.bottom);
    }
};

// 数值小工具
TEST_CLASS(TsUtilityNumber)
{
public:
    TEST_METHOD(TsAbsolute)
    {
        Assert::AreEqual(3, Absolute(-3));
        Assert::AreEqual(0, Absolute(0));
        Assert::AreEqual(3.5f, Absolute(-3.5f), 1e-6f);
    }

    // 无符号类型不得回绕
    TEST_METHOD(TsDistanceUnsigned)
    {
        Assert::AreEqual(7u, Distance(3u, 10u));
        Assert::AreEqual(7u, Distance(10u, 3u));
        Assert::AreEqual(7, Distance(3, 10));
        Assert::AreEqual(7, Distance(10, 3));
        Assert::AreEqual(2.5f, Distance(1.f, 3.5f), 1e-6f);
    }

    // 结果是 |x| 并取 iSign 的符号；iSign 为 0 时原样返回
    TEST_METHOD(TsSetSign)
    {
        Assert::AreEqual(-5, SetSign(5, -1));
        Assert::AreEqual(5, SetSign(-5, 1));
        Assert::AreEqual(-5, SetSign(-5, -1));
        Assert::AreEqual(-3, SetSign(-3, 0));
        Assert::AreEqual(3, SetSign(3, 0));
    }

    TEST_METHOD(TsFloatEqual)
    {
        Assert::IsTrue(FloatEqual(2.f, 2.f));
        Assert::IsTrue(FloatEqual(0.f, 1e-10f));
        Assert::IsFalse(FloatEqual(1.f, 1.01f));
    }
};
TS_NS_END
