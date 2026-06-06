#include "pch.h"
#include "../eck/CalcExp.h"

using namespace eck::Ce;

namespace Microsoft::VisualStudio::CppUnitTestFramework
{
    template<>
    static std::wstring ToString<eck::Ce::Result>(const eck::Ce::Result& v)
    {
        switch (v)
        {
        case eck::Ce::Result::Ok:
            return L"Result::Ok";
        case eck::Ce::Result::InvalidChar:
            return L"Result::InvalidChar";
        case eck::Ce::Result::AdjacentOp:
            return L"Result::AdjacentOp";
        case eck::Ce::Result::UnmatchedParentheses:
            return L"Result::UnmatchedParentheses";
        case eck::Ce::Result::OpError:
            return L"Result::OpError";
        default:
            return L"(Unknown Result)";
        }
    }
}

TS_NS_BEGIN
TEST_CLASS(TsCalcExp)
{
    static void AssertDoubleEq(double a, double b, double eps = 1e-9)
    {
        Assert::IsTrue(fabs(a - b) < eps);
    }
public:
    TEST_METHOD(Calc_Simple_Add)
    {
        double r{};
        auto ret = CalculateExpression(r, "1+2");
        Assert::AreEqual(Result::Ok, ret);
        AssertDoubleEq(3, r);
    }

    TEST_METHOD(Calc_Priority)
    {
        double r{};
        auto ret = CalculateExpression(r, "1+2*3");
        Assert::AreEqual(Result::Ok, ret);
        AssertDoubleEq(7, r);
    }

    TEST_METHOD(Calc_Parentheses)
    {
        double r{};
        auto ret = CalculateExpression(r, "(1+2)*3");
        Assert::AreEqual(Result::Ok, ret);
        AssertDoubleEq(9, r);
    }

    TEST_METHOD(Calc_UnaryMinus)
    {
        double r{};
        auto ret = CalculateExpression(r, "-1+2");
        Assert::AreEqual(Result::Ok, ret);
        AssertDoubleEq(1, r);
    }

    TEST_METHOD(Calc_Exponent)
    {
        double r{};
        auto ret = CalculateExpression(r, "2^4");
        Assert::AreEqual(Result::Ok, ret);
        AssertDoubleEq(16, r);
    }

    TEST_METHOD(Calc_Const_Pi)
    {
        double r{};
        auto ret = CalculateExpression(r, "Pi");
        Assert::AreEqual(Result::Ok, ret);
        AssertDoubleEq(3.141592653589793, r);
    }

    TEST_METHOD(Calc_Func_Sin)
    {
        double r{};
        auto ret = CalculateExpression(r, "Sin(Pi/2)");
        Assert::AreEqual(Result::Ok, ret);
        AssertDoubleEq(1, r);
    }

    TEST_METHOD(Calc_Func_Log)
    {
        double r{};
        auto ret = CalculateExpression(r, "Log(100)");
        Assert::AreEqual(Result::Ok, ret);
        AssertDoubleEq(2, r);
    }

    TEST_METHOD(Calc_Func_Sqrt)
    {
        double r{};
        auto ret = CalculateExpression(r, "Sqrt(9)");
        Assert::AreEqual(Result::Ok, ret);
        AssertDoubleEq(3, r);
    }

    TEST_METHOD(Calc_InvalidChar)
    {
        double r{};
        auto ret = CalculateExpression(r, "1+2@3");
        Assert::AreEqual(Result::InvalidChar, ret);
    }

    // 以下3个测试将返回OpError，
    // 严格检测较复杂，暂不支持

    //TEST_METHOD(Calc_AdjacentOp)
    //{
    //    double r{};
    //    auto ret = CalculateExpression(r, "1*/2");
    //    Assert::AreEqual(Result::AdjacentOp, ret);
    //}

    //TEST_METHOD(Calc_AdjacentOp_DoublePlus)
    //{
    //    double r{};
    //    auto ret = CalculateExpression(r, "1++2");
    //    // "++" 中第二个 + 会触发 AdjacentOp
    //    Assert::AreEqual(Result::AdjacentOp, ret);
    //}

    //TEST_METHOD(Calc_UnmatchedParentheses_Left)
    //{
    //    double r{};
    //    auto ret = CalculateExpression(r, "(1+2");
    //    Assert::AreEqual(Result::UnmatchedParentheses, ret);
    //}

    TEST_METHOD(Calc_UnmatchedParentheses_Right)
    {
        double r{};
        auto ret = CalculateExpression(r, "1+2)");
        Assert::AreEqual(Result::UnmatchedParentheses, ret);
    }

    TEST_METHOD(Calc_OpError_MissingOperand)
    {
        double r{};
        auto ret = CalculateExpression(r, "1+");
        Assert::AreEqual(Result::OpError, ret);
    }

    TEST_METHOD(Calc_OpError_Empty)
    {
        double r{};
        auto ret = CalculateExpression(r, "");
        Assert::AreEqual(Result::OpError, ret);
    }

    TEST_METHOD(Calc_OpError_OnlyOperator)
    {
        double r{};
        auto ret = CalculateExpression(r, "*");
        Assert::AreEqual(Result::AdjacentOp, ret);
    }

    TEST_METHOD(Calc_Whitespace)
    {
        double r{};
        auto ret = CalculateExpression(r, " 1 + 2 * ( 3 + 1 ) ");
        Assert::AreEqual(Result::Ok, ret);
        AssertDoubleEq(1 + 2 * 4, r);
    }

    TEST_METHOD(Calc_ComplexExpression)
    {
        double r{};
        auto ret = CalculateExpression(
            r, "Sin(Pi/4)^2 + Cos(Pi/4)^2"
        );
        Assert::AreEqual(Result::Ok, ret);
        AssertDoubleEq(1.0, r);
    }
};
TS_NS_END