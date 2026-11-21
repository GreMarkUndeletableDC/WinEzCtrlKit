#include "pch.h"
#include "../eck/KwDefine.h"
#include <limits>

using namespace eck;

TS_NS_BEGIN
namespace
{
    template<Kw::CcpVector T>
    void AssertVectorEqual(const T& expected, const T& actual)
    {
        for (size_t i = 0; i < T::N; ++i)
            Assert::AreEqual(expected[i], actual[i], 1e-5f);
    }

    template<size_t M, size_t N>
    void AssertMatrixEqual(const Kw::Matrix<M, N>& expected,
        const Kw::Matrix<M, N>& actual)
    {
        for (size_t i = 0; i < M; ++i)
            for (size_t j = 0; j < N; ++j)
                Assert::AreEqual(expected(i, j), actual(i, j), 1e-5f);
    }

    template<Kw::CcpVector V, class M>
    void AssertRowVectorProduct(const V& a, const M& b, const V& expected)
    {
        // 不可调用的运算符也应使测试失败，而不是阻断整个测试项目的编译。
        if constexpr (requires { { a * b } -> std::same_as<V>; })
            AssertVectorEqual(expected, a * b);
        else
            Assert::Fail(L"Vector * Matrix must be callable without explicit template arguments.");
    }

    template<class M, class T>
    void AssertChainedAddAssign(M& a, const T& b, const T& c, const M& expected)
    {
        if constexpr (requires { (a += b) += c; })
        {
            (void)((a += b) += c);
            AssertMatrixEqual(expected, a);
        }
        else
            Assert::Fail(L"Chained += must be callable and must update the original matrix.");
    }

    template<Kw::CcpVector T>
    constexpr bool VectorArithmeticIsConstexpr()
    {
        T a{}, b{};
        for (size_t i = 0; i < T::N; ++i)
        {
            a[i] = 6.f;
            b[i] = 2.f;
        }
        const auto sum = a + b;
        const auto difference = a - b;
        const auto product = a * b;
        const auto quotient = a / b;
        const auto scaled = a * 2.f;
        const auto divided = a / 2.f;
        (a += b) -= b;
        (a *= b) /= b;
        (a *= 2.f) /= 2.f;
        return sum[T::N - 1] == 8.f && difference[T::N - 1] == 4.f &&
            product[T::N - 1] == 12.f && quotient[T::N - 1] == 3.f &&
            scaled[T::N - 1] == 12.f && divided[T::N - 1] == 3.f &&
            a[T::N - 1] == 6.f;
    }

    template<Kw::CcpVector T>
    void AssertVectorArithmeticIsConstexpr()
    {
        if constexpr (requires { typename std::bool_constant<VectorArithmeticIsConstexpr<T>()>; })
            Assert::IsTrue(VectorArithmeticIsConstexpr<T>());
        else
            Assert::Fail(L"Vector indexing and arithmetic must support constant evaluation.");
    }

    // 矩阵运算声明为 constexpr；使用非方阵验证常量求值及行列方向。
    constexpr Kw::Matrix<2, 3> ConstMatrix{ { { 1.f, 2.f, 3.f }, { 4.f, 5.f, 6.f } } };
    constexpr auto ConstTranspose = Kw::Transpose(ConstMatrix);
    static_assert(ConstTranspose.M == 3 && ConstTranspose.N == 2);
    static_assert(ConstTranspose(2, 1) == 6.f);
    constexpr auto ConstProduct = ConstMatrix * ConstTranspose;
    static_assert(ConstProduct(0, 0) == 14.f && ConstProduct(1, 1) == 77.f);
    static_assert(std::same_as<Kw::Matrix2x2, Kw::Matrix<2, 2>>);
    static_assert(std::same_as<Kw::Matrix3x3, Kw::Matrix<3, 3>>);
    static_assert(Kw::CcpVector<Kw::Vector2> && Kw::CcpVector<Kw::Vector3>);
    static_assert(!Kw::CcpVector<Kw::Matrix2x2>);
    static_assert(CcpRect<Kw::Rect> && CcpRect<Kw::RectU> && CcpRect<Kw::RectI>);
    static_assert(std::same_as<RectTraits<Kw::Rect>::T, float>);
    static_assert(std::same_as<RectTraits<Kw::RectU>::T, UINT>);
    static_assert(std::same_as<RectTraits<Kw::RectI>::T, int>);
}

TEST_CLASS(TsKwVector)
{
private:
    template<Kw::CcpVector T>
    static void TestArithmetic(const T& a, const T& b, const T& sum,
        const T& difference, const T& product, const T& quotient,
        const T& scaled, const T& divided)
    {
        const auto originalA = a, originalB = b;
        AssertVectorEqual(sum, a + b);
        AssertVectorEqual(difference, a - b);
        AssertVectorEqual(product, a * b);
        AssertVectorEqual(quotient, a / b);
        AssertVectorEqual(scaled, a * 2.f);
        AssertVectorEqual(divided, a / 2.f);
        AssertVectorEqual(originalA, a); // 非赋值运算不得修改输入。
        AssertVectorEqual(originalB, b);
    }

    template<Kw::CcpVector T>
    static void TestCompoundAssignment(T a, const T& b)
    {
        const auto original = a;
        Assert::IsTrue(&(a += b) == &a);
        AssertVectorEqual(original + b, a);
        Assert::IsTrue(&(a -= b) == &a);
        AssertVectorEqual(original, a);
        Assert::IsTrue(&(a *= b) == &a);
        AssertVectorEqual(original * b, a);
        Assert::IsTrue(&(a /= b) == &a);
        AssertVectorEqual(original, a);
        Assert::IsTrue(&(a *= 2.f) == &a);
        AssertVectorEqual(original * 2.f, a);
        Assert::IsTrue(&(a /= 2.f) == &a);
        AssertVectorEqual(original, a);
    }

public:
    TEST_METHOD(Vector2_IndexAccess)
    {
        Kw::Vector2 v{ 2.f, -3.f };
        const auto& cv = v;
        Assert::AreEqual(2.f, cv[0]);
        Assert::AreEqual(-3.f, cv[1]);
        v[0] = 5.f;
        v[1] = 7.f;
        Assert::AreEqual(5.f, v.x);
        Assert::AreEqual(7.f, v.y);
    }

    TEST_METHOD(Vector3_IndexAccess)
    {
        Kw::Vector3 v{ 2.f, -3.f, 4.f };
        const auto& cv = v;
        Assert::AreEqual(2.f, cv[0]);
        Assert::AreEqual(-3.f, cv[1]);
        Assert::AreEqual(4.f, cv[2]);
        v[0] = 5.f;
        v[1] = 7.f;
        v[2] = -9.f;
        Assert::AreEqual(5.f, v.x);
        Assert::AreEqual(7.f, v.y);
        Assert::AreEqual(-9.f, v.z);
    }

    TEST_METHOD(Vector2_Arithmetic)
    {
        TestArithmetic<Kw::Vector2>({ 6.f, -8.f }, { 2.f, -4.f },
            { 8.f, -12.f }, { 4.f, -4.f }, { 12.f, 32.f },
            { 3.f, 2.f }, { 12.f, -16.f }, { 3.f, -4.f });
    }

    TEST_METHOD(Vector3_Arithmetic)
    {
        TestArithmetic<Kw::Vector3>({ 6.f, -8.f, 0.f }, { 2.f, -4.f, 5.f },
            { 8.f, -12.f, 5.f }, { 4.f, -4.f, -5.f }, { 12.f, 32.f, 0.f },
            { 3.f, 2.f, 0.f }, { 12.f, -16.f, 0.f }, { 3.f, -4.f, 0.f });
    }

    TEST_METHOD(Vector2_ConstexprArithmetic)
    {
        AssertVectorArithmeticIsConstexpr<Kw::Vector2>();
    }

    TEST_METHOD(Vector3_ConstexprArithmetic)
    {
        AssertVectorArithmeticIsConstexpr<Kw::Vector3>();
    }

    TEST_METHOD(Vector2_CompoundAssignment)
    {
        TestCompoundAssignment<Kw::Vector2>({ 6.f, -8.f }, { 2.f, -4.f });
    }

    TEST_METHOD(Vector3_CompoundAssignment)
    {
        TestCompoundAssignment<Kw::Vector3>({ 6.f, -8.f, 10.f }, { 2.f, -4.f, 5.f });
    }

    TEST_METHOD(Vector_SelfAssignment)
    {
        Kw::Vector3 v{ 2.f, -3.f, 4.f };
        v += v;
        AssertVectorEqual(Kw::Vector3{ 4.f, -6.f, 8.f }, v);
        v *= v;
        AssertVectorEqual(Kw::Vector3{ 16.f, 36.f, 64.f }, v);
        v /= v;
        AssertVectorEqual(Kw::Vector3{ 1.f, 1.f, 1.f }, v);
        v -= v;
        AssertVectorEqual(Kw::Vector3{}, v);
    }

    TEST_METHOD(Vector2_RowMatrixProduct)
    {
        const Kw::Vector2 v{ 2.f, -3.f };
        const Kw::Matrix2x2 m{ { { 1.f, 4.f }, { 2.f, -5.f } } };
        // [2, -3] * [[1, 4], [2, -5]] = [-4, 23]，不是列向量乘法。
        AssertRowVectorProduct(v, m, Kw::Vector2{ -4.f, 23.f });
    }

    TEST_METHOD(Vector3_RowMatrixProduct)
    {
        const Kw::Vector3 v{ 2.f, -3.f, 1.f };
        const Kw::Matrix3x3 m{ { { 1.f, 4.f, 7.f }, { 2.f, -5.f, 8.f }, { 3.f, 6.f, -9.f } } };
        AssertRowVectorProduct(v, m, Kw::Vector3{ -1.f, 29.f, -19.f });
    }

    TEST_METHOD(Vector3_HomogeneousTranslation)
    {
        const Kw::Matrix3x3 m{ { { 1.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, { 10.f, -5.f, 1.f } } };
        AssertRowVectorProduct(Kw::Vector3{ 2.f, 3.f, 1.f }, m, Kw::Vector3{ 12.f, -2.f, 1.f });
        AssertRowVectorProduct(Kw::Vector3{ 2.f, 3.f, 0.f }, m, Kw::Vector3{ 2.f, 3.f, 0.f });
    }
};

TEST_CLASS(TsKwMatrix)
{
public:
    TEST_METHOD(Matrix_IndexAccessAndZeroInitialization)
    {
        Kw::Matrix<2, 3> m{};
        AssertMatrixEqual(Kw::Matrix<2, 3>{}, m);
        m(1, 2) = -7.f;
        const auto& cm = m;
        Assert::AreEqual(-7.f, cm(1, 2));
        Assert::AreEqual(-7.f, m.m[1][2]);
        Assert::AreEqual(0.f, cm(0, 2));
    }

    TEST_METHOD(Matrix_RectangularArithmetic)
    {
        const Kw::Matrix<2, 3> a{ { { 2.f, -4.f, 0.f }, { 6.f, 8.f, -10.f } } };
        const Kw::Matrix<2, 3> b{ { { 1.f, 2.f, 5.f }, { -3.f, 4.f, -2.f } } };
        const auto originalA = a, originalB = b;
        AssertMatrixEqual(Kw::Matrix<2, 3>{ { { 3.f, -2.f, 5.f }, { 3.f, 12.f, -12.f } } }, a + b);
        AssertMatrixEqual(Kw::Matrix<2, 3>{ { { 1.f, -6.f, -5.f }, { 9.f, 4.f, -8.f } } }, a - b);
        // 矩阵除法接口的约定是逐元素除法，不是乘逆矩阵。
        AssertMatrixEqual(Kw::Matrix<2, 3>{ { { 2.f, -2.f, 0.f }, { -2.f, 2.f, 5.f } } }, a / b);
        AssertMatrixEqual(Kw::Matrix<2, 3>{ { { 4.f, -2.f, 2.f }, { 8.f, 10.f, -8.f } } }, a + 2.f);
        AssertMatrixEqual(Kw::Matrix<2, 3>{ { { 0.f, -6.f, -2.f }, { 4.f, 6.f, -12.f } } }, a - 2.f);
        AssertMatrixEqual(Kw::Matrix<2, 3>{ { { -4.f, 8.f, 0.f }, { -12.f, -16.f, 20.f } } }, a * -2.f);
        AssertMatrixEqual(Kw::Matrix<2, 3>{ { { 1.f, -2.f, 0.f }, { 3.f, 4.f, -5.f } } }, a / 2.f);
        AssertMatrixEqual(originalA, a);
        AssertMatrixEqual(originalB, b);
    }

    TEST_METHOD(Matrix_AddAssignMutatesAllElements)
    {
        Kw::Matrix<2, 3> a{ { { 1.f, 2.f, 3.f }, { 4.f, 5.f, 6.f } } };
        const Kw::Matrix<2, 3> b{ { { 6.f, 5.f, 4.f }, { 3.f, 2.f, 1.f } } };
        (void)(a += b);
        AssertMatrixEqual(Kw::Matrix<2, 3>{ { { 7.f, 7.f, 7.f }, { 7.f, 7.f, 7.f } } }, a);
        (void)(a += a);
        AssertMatrixEqual(Kw::Matrix<2, 3>{ { { 14.f, 14.f, 14.f }, { 14.f, 14.f, 14.f } } }, a);
    }

    TEST_METHOD(Matrix_AddAssignReturnsLeftReference)
    {
        Kw::Matrix2x2 a{};
        const Kw::Matrix2x2 b{ { { 1.f, 2.f }, { 3.f, 4.f } } };
        Assert::IsTrue((std::same_as<decltype(a += b), Kw::Matrix2x2&>),
            L"Matrix += Matrix must return the left operand by reference.");
        auto&& result = (a += b);
        Assert::IsTrue(&result == &a);
        AssertChainedAddAssign(a, b, b,
            Kw::Matrix2x2{ { { 3.f, 6.f }, { 9.f, 12.f } } });
    }

    TEST_METHOD(Matrix_ScalarAddAssignMutatesAllElements)
    {
        Kw::Matrix<2, 3> a{ { { 1.f, -2.f, 3.f }, { -4.f, 5.f, -6.f } } };
        (void)(a += 2.f);
        AssertMatrixEqual(Kw::Matrix<2, 3>{ { { 3.f, 0.f, 5.f }, { -2.f, 7.f, -4.f } } }, a);
    }

    TEST_METHOD(Matrix_ScalarAddAssignReturnsLeftReference)
    {
        Kw::Matrix2x2 a{};
        Assert::IsTrue((std::same_as<decltype(a += 1.f), Kw::Matrix2x2&>),
            L"Matrix += scalar must return the left operand by reference.");
        auto&& result = (a += 1.f);
        Assert::IsTrue(&result == &a);
        AssertChainedAddAssign(a, 2.f, 3.f,
            Kw::Matrix2x2{ { { 6.f, 6.f }, { 6.f, 6.f } } });
    }

    TEST_METHOD(Matrix_RectangularProduct)
    {
        const Kw::Matrix<2, 3> a{ { { 1.f, 2.f, 3.f }, { 4.f, 5.f, 6.f } } };
        const Kw::Matrix<3, 4> b{ { { 7.f, 8.f, 9.f, 10.f }, { 11.f, 12.f, 13.f, 14.f }, { 15.f, 16.f, 17.f, 18.f } } };
        const auto product = a * b;
        static_assert(std::same_as<std::remove_cv_t<decltype(product)>, Kw::Matrix<2, 4>>);
        AssertMatrixEqual(Kw::Matrix<2, 4>{ { { 74.f, 80.f, 86.f, 92.f }, { 173.f, 188.f, 203.f, 218.f } } }, product);
    }

    TEST_METHOD(Matrix_ProductIsNotCommutative)
    {
        const Kw::Matrix2x2 a{ { { 1.f, 2.f }, { 3.f, 4.f } } };
        const Kw::Matrix2x2 b{ { { 0.f, 1.f }, { -1.f, 0.f } } };
        AssertMatrixEqual(Kw::Matrix2x2{ { { -2.f, 1.f }, { -4.f, 3.f } } }, a * b);
        AssertMatrixEqual(Kw::Matrix2x2{ { { 3.f, 4.f }, { -1.f, -2.f } } }, b * a);
    }

    TEST_METHOD(Matrix_IdentityAndZeroProduct)
    {
        const Kw::Matrix3x3 a{ { { 1.f, -2.f, 3.f }, { 4.f, 5.f, -6.f }, { 7.f, 8.f, 9.f } } };
        const Kw::Matrix3x3 identity{ { { 1.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, { 0.f, 0.f, 1.f } } };
        AssertMatrixEqual(a, a * identity);
        AssertMatrixEqual(a, identity * a);
        AssertMatrixEqual(Kw::Matrix3x3{}, a * Kw::Matrix3x3{});
        AssertMatrixEqual(Kw::Matrix3x3{}, Kw::Matrix3x3{} * a);
    }

    TEST_METHOD(Matrix_RowColumnAndOuterProduct)
    {
        const Kw::Matrix<1, 3> row{ { { 1.f, -2.f, 3.f } } };
        const Kw::Matrix<3, 1> column{ { { 4.f }, { 5.f }, { 6.f } } };
        AssertMatrixEqual(Kw::Matrix<1, 1>{ { { 12.f } } }, row * column);
        AssertMatrixEqual(Kw::Matrix3x3{ { { 4.f, -8.f, 12.f }, { 5.f, -10.f, 15.f }, { 6.f, -12.f, 18.f } } }, column * row);
    }

    TEST_METHOD(Matrix_RectangularTranspose)
    {
        const Kw::Matrix<2, 3> a{ { { 1.f, 2.f, 3.f }, { 4.f, 5.f, 6.f } } };
        const auto transposed = Kw::Transpose(a);
        AssertMatrixEqual(Kw::Matrix<3, 2>{ { { 1.f, 4.f }, { 2.f, 5.f }, { 3.f, 6.f } } }, transposed);
        AssertMatrixEqual(a, Kw::Transpose(transposed));
        AssertMatrixEqual(Kw::Matrix<1, 3>{ { { 1.f, 2.f, 3.f } } },
            Kw::Transpose(Kw::Matrix<3, 1>{ { { 1.f }, { 2.f }, { 3.f } } }));
    }

    TEST_METHOD(Matrix2x2_DeterminantAndAdjugate)
    {
        const Kw::Matrix2x2 a{ { { 2.f, 3.f }, { 5.f, 7.f } } };
        Kw::Matrix2x2 adjugate{};
        Assert::AreEqual(-1.f, Kw::Determinant(a));
        Assert::AreEqual(-1.f, Kw::Determinant(a, &adjugate));
        AssertMatrixEqual(Kw::Matrix2x2{ { { 7.f, -3.f }, { -5.f, 2.f } } }, adjugate);
        const Kw::Matrix2x2 expected{ { { -1.f, 0.f }, { 0.f, -1.f } } };
        AssertMatrixEqual(expected, a * adjugate);
        AssertMatrixEqual(expected, adjugate * a);
    }

    TEST_METHOD(Matrix3x3_DeterminantAndAdjugate)
    {
        const Kw::Matrix3x3 a{ { { 1.f, 2.f, 3.f }, { 0.f, 4.f, 5.f }, { 1.f, 0.f, 6.f } } };
        Kw::Matrix3x3 adjugate{};
        Assert::AreEqual(22.f, Kw::Determinant(a));
        Assert::AreEqual(22.f, Kw::Determinant(a, &adjugate));
        AssertMatrixEqual(Kw::Matrix3x3{ { { 24.f, -12.f, -2.f }, { 5.f, 3.f, -5.f }, { -4.f, 2.f, 4.f } } }, adjugate);
        const Kw::Matrix3x3 expected{ { { 22.f, 0.f, 0.f }, { 0.f, 22.f, 0.f }, { 0.f, 0.f, 22.f } } };
        AssertMatrixEqual(expected, a * adjugate);
        AssertMatrixEqual(expected, adjugate * a);
        Assert::AreEqual(22.f, Kw::Determinant(Kw::Transpose(a)));
    }

    TEST_METHOD(Matrix_DeterminantAllowsAliasedAdjugate)
    {
        Kw::Matrix2x2 a{ { { 2.f, 3.f }, { 5.f, 7.f } } };
        Assert::AreEqual(-1.f, Kw::Determinant(a, &a));
        AssertMatrixEqual(Kw::Matrix2x2{ { { 7.f, -3.f }, { -5.f, 2.f } } }, a);
        Kw::Matrix3x3 b{ { { 1.f, 2.f, 3.f }, { 0.f, 4.f, 5.f }, { 1.f, 0.f, 6.f } } };
        Assert::AreEqual(22.f, Kw::Determinant(b, &b));
        AssertMatrixEqual(Kw::Matrix3x3{ { { 24.f, -12.f, -2.f }, { 5.f, 3.f, -5.f }, { -4.f, 2.f, 4.f } } }, b);
    }

    TEST_METHOD(Matrix_SingularDeterminantAndAdjugate)
    {
        const Kw::Matrix2x2 a{ { { 1.f, 2.f }, { 2.f, 4.f } } };
        Kw::Matrix2x2 adjugate2{};
        Assert::AreEqual(0.f, Kw::Determinant(a, &adjugate2));
        AssertMatrixEqual(Kw::Matrix2x2{ { { 4.f, -2.f }, { -2.f, 1.f } } }, adjugate2);
        const Kw::Matrix3x3 b{ { { 1.f, 2.f, 3.f }, { 2.f, 4.f, 6.f }, { 0.f, 1.f, 1.f } } };
        Kw::Matrix3x3 adjugate3{};
        Assert::AreEqual(0.f, Kw::Determinant(b, &adjugate3));
        AssertMatrixEqual(Kw::Matrix3x3{ { { -2.f, 1.f, 0.f }, { -2.f, 1.f, 0.f }, { 2.f, -1.f, 0.f } } }, adjugate3);
        AssertMatrixEqual(Kw::Matrix3x3{}, b * adjugate3);
    }

    TEST_METHOD(Matrix_ZeroAndIdentityDeterminant)
    {
        const Kw::Matrix2x2 identity2{ { { 1.f, 0.f }, { 0.f, 1.f } } };
        const Kw::Matrix3x3 identity3{ { { 1.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, { 0.f, 0.f, 1.f } } };
        Kw::Matrix2x2 adjugate2{};
        Kw::Matrix3x3 adjugate3{};
        Assert::AreEqual(1.f, Kw::Determinant(identity2, &adjugate2));
        Assert::AreEqual(1.f, Kw::Determinant(identity3, &adjugate3));
        AssertMatrixEqual(identity2, adjugate2);
        AssertMatrixEqual(identity3, adjugate3);
        Assert::AreEqual(0.f, Kw::Determinant(Kw::Matrix2x2{}, &adjugate2));
        Assert::AreEqual(0.f, Kw::Determinant(Kw::Matrix3x3{}, &adjugate3));
        AssertMatrixEqual(Kw::Matrix2x2{}, adjugate2);
        AssertMatrixEqual(Kw::Matrix3x3{}, adjugate3);
    }

    TEST_METHOD(Matrix2x2_DeterminantFractionalValues)
    {
        const Kw::Matrix2x2 a{ { { 0.5f, 1.25f }, { -2.f, 0.75f } } };
        Kw::Matrix2x2 adjugate{};
        Assert::AreEqual(2.875f, Kw::Determinant(a, &adjugate));
        AssertMatrixEqual(Kw::Matrix2x2{ { { 0.75f, -1.25f }, { 2.f, 0.5f } } }, adjugate);
    }

    TEST_METHOD(Matrix3x3_DeterminantTriangularFractionalValues)
    {
        const Kw::Matrix3x3 a{ { { 0.5f, 1.25f, -2.f }, { 0.f, -0.75f, 1.5f }, { 0.f, 0.f, 4.f } } };
        Assert::AreEqual(-1.5f, Kw::Determinant(a));
    }

    TEST_METHOD(Matrix2x2_DeterminantRowSwapSign)
    {
        const Kw::Matrix2x2 a{ { { 0.f, 2.f }, { 3.f, 0.f } } };
        Assert::AreEqual(-6.f, Kw::Determinant(a));
        Assert::AreEqual(-6.f, Kw::Determinant(Kw::Transpose(a)));
    }

    TEST_METHOD(Matrix3x3_DeterminantRowSwapSign)
    {
        const Kw::Matrix3x3 a{ { { 0.f, 0.f, 2.f }, { 0.f, 3.f, 0.f }, { 4.f, 0.f, 0.f } } };
        Assert::AreEqual(-24.f, Kw::Determinant(a));
    }
};

TEST_CLASS(TsKwLinearAlgebra)
{
public:
    TEST_METHOD(Vector_UnarySignsScalarLeftAndEquality)
    {
        constexpr Kw::Vector2 a{ 2.f, -3.f };
        constexpr Kw::Vector3 b{ 2.f, -3.f, 4.f };
        static_assert(-a == Kw::Vector2{ -2.f, 3.f });
        static_assert(2.f * b == Kw::Vector3{ 4.f, -6.f, 8.f });
        static_assert(b != Kw::Vector3{ 2.f, -3.f, 5.f });
        AssertVectorEqual(Kw::Vector2{ 4.f, -6.f }, 2.f * a);
        AssertVectorEqual(Kw::Vector3{ -2.f, 3.f, -4.f }, -b);
    }

    TEST_METHOD(Vector_DotAndCross)
    {
        constexpr Kw::Vector2 x{ 1.f, 0.f }, y{ 0.f, 1.f };
        static_assert(Kw::Dot(x, y) == 0.f);
        static_assert(Kw::Cross(x, y) == 1.f && Kw::Cross(y, x) == -1.f);
        Assert::AreEqual(-5.f, Kw::Dot(Kw::Vector2{ 1.f, -2.f }, Kw::Vector2{ 3.f, 4.f }));
        Assert::AreEqual(-24.f, Kw::Dot(Kw::Vector3{ 1.f, 2.f, -3.f }, Kw::Vector3{ 4.f, -5.f, 6.f }));
        const Kw::Vector3 a{ 1.f, 2.f, 3.f }, b{ 4.f, 5.f, 6.f };
        const auto cross = Kw::Cross(a, b);
        AssertVectorEqual(Kw::Vector3{ -3.f, 6.f, -3.f }, cross);
        AssertVectorEqual(-cross, Kw::Cross(b, a));
        Assert::AreEqual(0.f, Kw::Dot(a, cross));
        Assert::AreEqual(0.f, Kw::Dot(b, cross));
        AssertVectorEqual(Kw::Vector3{}, Kw::Cross(a, a));
    }

    TEST_METHOD(Vector_LengthAndDistance)
    {
        const Kw::Vector2 a{ 3.f, 4.f };
        Assert::AreEqual(25.f, Kw::LengthSquared(a));
        Assert::AreEqual(5.f, Kw::Length(a), 1e-5f);
        Assert::AreEqual(169.f, Kw::LengthSquared(Kw::Vector3{ 3.f, 4.f, 12.f }));
        Assert::AreEqual(13.f, Kw::Length(Kw::Vector3{ 3.f, 4.f, 12.f }), 1e-5f);
    }

    TEST_METHOD(Matrix_UnarySignsScalarLeftAndEquality)
    {
        constexpr Kw::Matrix2x2 a{ { { 2.f, -4.f }, { 6.f, 8.f } } };
        static_assert(-(-a) == a);
        static_assert(a != Kw::Matrix2x2{});
        AssertMatrixEqual(Kw::Matrix2x2{ { { 4.f, -2.f }, { 8.f, 10.f } } }, 2.f + a);
        AssertMatrixEqual(Kw::Matrix2x2{ { { 0.f, 6.f }, { -4.f, -6.f } } }, 2.f - a);
        AssertMatrixEqual(Kw::Matrix2x2{ { { 4.f, -8.f }, { 12.f, 16.f } } }, 2.f * a);
        AssertMatrixEqual(Kw::Matrix2x2{ { { 12.f, -6.f }, { 4.f, 3.f } } }, 24.f / a);
    }

    TEST_METHOD(Matrix_CompoundAssignmentReturnsReference)
    {
        Kw::Matrix<2, 3> a{ { { 2.f, -4.f, 6.f }, { 8.f, -10.f, 12.f } } };
        const auto original = a;
        const Kw::Matrix<2, 3> b{ { { 1.f, 2.f, 3.f }, { 4.f, 5.f, 6.f } } };
        Assert::IsTrue(&(a -= b) == &a);
        AssertMatrixEqual(Kw::Matrix<2, 3>{ { { 1.f, -6.f, 3.f }, { 4.f, -15.f, 6.f } } }, a);
        (void)(a += b);
        Assert::IsTrue(&(a -= 2.f) == &a);
        AssertMatrixEqual(Kw::Matrix<2, 3>{ { { 0.f, -6.f, 4.f }, { 6.f, -12.f, 10.f } } }, a);
        (void)(a += 2.f);
        Assert::IsTrue(&(a *= 2.f) == &a);
        AssertMatrixEqual(Kw::Matrix<2, 3>{ { { 4.f, -8.f, 12.f }, { 16.f, -20.f, 24.f } } }, a);
        Assert::IsTrue(&(a /= 2.f) == &a);
        AssertMatrixEqual(original, a);
        Assert::IsTrue(&(a /= b) == &a);
        AssertMatrixEqual(Kw::Matrix<2, 3>{ { { 2.f, -2.f, 2.f }, { 2.f, -2.f, 2.f } } }, a);
        (void)(a /= a);
        AssertMatrixEqual(Kw::Matrix<2, 3>{ { { 1.f, 1.f, 1.f }, { 1.f, 1.f, 1.f } } }, a);
        (void)(a -= a);
        AssertMatrixEqual(Kw::Matrix<2, 3>{}, a);
    }

    TEST_METHOD(Matrix_ColumnVectorProduct)
    {
        const Kw::Matrix2x2 a{ { { 1.f, 4.f }, { 2.f, -5.f } } };
        AssertVectorEqual(Kw::Vector2{ -10.f, 19.f }, a * Kw::Vector2{ 2.f, -3.f });
        const Kw::Matrix3x3 b{ { { 1.f, 4.f, 7.f }, { 2.f, -5.f, 8.f }, { 3.f, 6.f, -9.f } } };
        AssertVectorEqual(Kw::Vector3{ -3.f, 27.f, -21.f }, b * Kw::Vector3{ 2.f, -3.f, 1.f });
    }

    TEST_METHOD(Matrix_RectangularVectorProducts)
    {
        constexpr Kw::Matrix<2, 3> a{ { { 1.f, 2.f, 3.f }, { 4.f, 5.f, 6.f } } };
        constexpr Kw::Matrix<3, 2> b{ { { 1.f, 4.f }, { 2.f, 5.f }, { 3.f, 6.f } } };
        static_assert(a * Kw::Vector3{ 1.f, 2.f, 3.f } == Kw::Vector2{ 14.f, 32.f });
        static_assert(Kw::Vector2{ 1.f, 2.f } * a == Kw::Vector3{ 9.f, 12.f, 15.f });
        AssertVectorEqual(Kw::Vector3{ 9.f, 12.f, 15.f }, b * Kw::Vector2{ 1.f, 2.f });
        AssertVectorEqual(Kw::Vector2{ 14.f, 32.f }, Kw::Vector3{ 1.f, 2.f, 3.f } * b);
    }
};

TEST_CLASS(TsKwRect)
{
public:
    TEST_METHOD(Rect_FromWin32Rect)
    {
        const RECT source{ -3, 5, 11, 17 };
        const auto rc = Kw::MakeRect(source);
        Assert::AreEqual(-3.f, rc.left);
        Assert::AreEqual(5.f, rc.top);
        Assert::AreEqual(11.f, rc.right);
        Assert::AreEqual(17.f, rc.bottom);
    }

    TEST_METHOD(Rect_ToWin32RectTruncatesTowardZero)
    {
        const auto rc = eck::MakeRect(Kw::Rect{ -3.75f, 5.75f, 11.5f, -17.5f });
        Assert::AreEqual(-3L, rc.left);
        Assert::AreEqual(5L, rc.top);
        Assert::AreEqual(11L, rc.right);
        Assert::AreEqual(-17L, rc.bottom);
    }

    TEST_METHOD(Rect_PointContainmentIsHalfOpen)
    {
        const Kw::Rect rc{ -2.f, 3.f, 8.f, 13.f };
        Assert::IsTrue(PointInRect(rc, Kw::Vector2{ -2.f, 3.f }));
        Assert::IsTrue(PointInRect(rc, Kw::Vector2{ 7.5f, 12.5f }));
        Assert::IsFalse(PointInRect(rc, Kw::Vector2{ 8.f, 5.f }));
        Assert::IsFalse(PointInRect(rc, Kw::Vector2{ 0.f, 13.f }));
        Assert::IsFalse(PointInRect(rc, Kw::Vector2{ -2.5f, 5.f }));
        Assert::IsFalse(PointInRect(rc, Kw::Vector2{ 0.f, 2.5f }));
        Assert::IsTrue(PointInRect(Kw::RectI{ -2, 3, 8, 13 }, Kw::Vector2{ 0.f, 5.f }));
        Assert::IsTrue(PointInRect(Kw::RectU{ 2, 3, 8, 13 }, Kw::Vector2{ 4.f, 5.f }));
        Assert::IsFalse(PointInRect(Kw::Rect{ 1.f, 1.f, 1.f, 5.f }, Kw::Vector2{ 1.f, 2.f }));
        Assert::IsFalse(PointInRect(Kw::Rect{ 8.f, 13.f, -2.f, 3.f }, Kw::Vector2{ 0.f, 5.f }));
    }

#ifdef _D2D1_H_
    TEST_METHOD(Rect_Direct2DConversions)
    {
        const Kw::Rect rc{ -1.5f, 2.5f, 3.5f, 4.5f };
        const auto& d2d = Kw::MakeD2DRectF(rc);
        Assert::AreEqual(rc.left, d2d.left);
        Assert::AreEqual(rc.top, d2d.top);
        Assert::AreEqual(rc.right, d2d.right);
        Assert::AreEqual(rc.bottom, d2d.bottom);
        const D2D1_RECT_F source{ -5.5f, 6.5f, 7.5f, 8.5f };
        const auto& converted = Kw::MakeRect(source);
        Assert::AreEqual(source.left, converted.left);
        Assert::AreEqual(source.top, converted.top);
        Assert::AreEqual(source.right, converted.right);
        Assert::AreEqual(source.bottom, converted.bottom);
        const Kw::Vector2 vector{ -9.f, 10.f };
        const auto& point = Kw::MakeD2DPointF(vector);
        Assert::AreEqual(-9.f, point.x);
        Assert::AreEqual(10.f, point.y);
    }
#endif
};
TS_NS_END
