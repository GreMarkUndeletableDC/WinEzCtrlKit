#pragma once
#include "RectTraits.h"

#define KW2D_NAMESPACE_BEGIN    namespace Kw {
#define KW2D_NAMESPACE_END      }

ECK_NAMESPACE_BEGIN
KW2D_NAMESPACE_BEGIN
#pragma region Vector
struct Vector2
{
    constexpr static size_t N = 2;

    float x, y;

    EckInlineNdCe float operator[](_In_range_(0, 1) size_t i) const noexcept
    {
        return (i == 0) ? x : y;
    }
    EckInlineNdCe float& operator[](_In_range_(0, 1) size_t i) noexcept
    {
        return (i == 0) ? x : y;
    }
};
struct Vector3
{
    constexpr static size_t N = 3;

    float x, y, z;

    EckInlineNdCe float operator[](_In_range_(0, 2) size_t i) const noexcept
    {
        return (i == 0) ? x : (i == 1) ? y : z;
    }
    EckInlineNdCe float& operator[](_In_range_(0, 2) size_t i) noexcept
    {
        return (i == 0) ? x : (i == 1) ? y : z;
    }
};
struct PolarPoint
{
    double Rho, Theta;
};

template<class T>
concept CcpVector = std::is_same_v<T, Vector2> || std::is_same_v<T, Vector3>;

#undef ECK_VEC_LOOP
#define ECK_VEC_LOOP() for (size_t i = 0; i < T::N; ++i)

template<CcpVector T>
EckInlineNdCe T operator+(const T& a, const T& b) noexcept
{
    T r;
    ECK_VEC_LOOP() r[i] = a[i] + b[i];
    return r;
}
template<CcpVector T>
EckInlineCe T& operator+=(T& a, const T& b) noexcept
{
    ECK_VEC_LOOP() a[i] += b[i];
    return a;
}

template<CcpVector T>
EckInlineNdCe T operator-(const T& a, const T& b) noexcept
{
    T r;
    ECK_VEC_LOOP() r[i] = a[i] - b[i];
    return r;
}
template<CcpVector T>
EckInlineCe T& operator-=(T& a, const T& b) noexcept
{
    ECK_VEC_LOOP() a[i] -= b[i];
    return a;
}

template<CcpVector T>
EckInlineNdCe T operator*(const T& v, float s) noexcept
{
    T r;
    ECK_VEC_LOOP() r[i] = v[i] * s;
    return r;
}
template<CcpVector T>
EckInlineCe T& operator*=(T& v, float s) noexcept
{
    ECK_VEC_LOOP() v[i] *= s;
    return v;
}

template<CcpVector T>
EckInlineNdCe T operator*(const T& v, const T& s) noexcept
{
    T r;
    ECK_VEC_LOOP() r[i] = v[i] * s[i];
    return r;
}
template<CcpVector T>
EckInlineCe T& operator*=(T& v, const T& s) noexcept
{
    ECK_VEC_LOOP() v[i] *= s[i];
    return v;
}

template<CcpVector T>
EckInlineNdCe T operator/(const T& v, float s) noexcept
{
    T r;
    ECK_VEC_LOOP() r[i] = v[i] / s;
    return r;
}
template<CcpVector T>
EckInlineCe T& operator/=(T& v, float s) noexcept
{
    ECK_VEC_LOOP() v[i] /= s;
    return v;
}

template<CcpVector T>
EckInlineNdCe T operator/(const T& v, const T& s) noexcept
{
    T r;
    ECK_VEC_LOOP() r[i] = v[i] / s[i];
    return r;
}
template<CcpVector T>
EckInlineCe T& operator/=(T& v, const T& s) noexcept
{
    ECK_VEC_LOOP() v[i] /= s[i];
    return v;
}

template<CcpVector T>
EckInlineNdCe T operator-(const T& v) noexcept
{
    T r;
    ECK_VEC_LOOP() r[i] = -v[i];
    return r;
}

template<CcpVector T>
EckInlineNdCe T operator*(float s, const T& v) noexcept { return v * s; }

template<CcpVector T>
EckInlineNdCe bool operator==(const T& a, const T& b) noexcept
{
    ECK_VEC_LOOP() if (a[i] != b[i]) return false;
    return true;
}

template<CcpVector T>
EckInlineNdCe float Dot(const T& a, const T& b) noexcept
{
    float r{};
    ECK_VEC_LOOP() r += a[i] * b[i];
    return r;
}

EckInlineNdCe float Cross(const Vector2& a, const Vector2& b) noexcept
{
    return a.x * b.y - a.y * b.x;
}
EckInlineNdCe Vector3 Cross(const Vector3& a, const Vector3& b) noexcept
{
    return
    {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

template<CcpVector T>
EckInlineNdCe float LengthSquared(const T& v) noexcept { return Dot(v, v); }

template<CcpVector T>
EckInlineNd float Length(const T& v) noexcept
{
    return (float)sqrt(LengthSquared(v));
}

#undef ECK_VEC_LOOP
#pragma endregion Vector

#pragma region Matrix
template<size_t M_, size_t N_>
struct Matrix
{
    constexpr static size_t M = M_;
    constexpr static size_t N = N_;

    float m[M][N];

    EckInlineNdCe float operator()(
        _In_range_(0, M - 1) size_t i,
        _In_range_(0, N - 1) size_t j) const noexcept
    {
        return m[i][j];
    }
    EckInlineNdCe float& operator()(
        _In_range_(0, M - 1) size_t i,
        _In_range_(0, N - 1) size_t j) noexcept
    {
        return m[i][j];
    }
};

using Matrix2x2 = Matrix<2, 2>;
using Matrix3x3 = Matrix<3, 3>;
using Matrix4x4 = Matrix<4, 4>;

#undef ECK_MAT_LOOP
#define ECK_MAT_LOOP()                  \
    for (size_t i = 0; i < M; ++i)      \
        for (size_t j = 0; j < N; ++j)

template<size_t M, size_t N>
EckNfInlineNdCe Matrix<M, N> operator+(const Matrix<M, N>& a, const Matrix<M, N>& b) noexcept
{
    Matrix<M, N> r;
    ECK_MAT_LOOP() r(i, j) = a(i, j) + b(i, j);
    return r;
}
template<size_t M, size_t N>
EckNfInlineCe Matrix<M, N>& operator+=(Matrix<M, N>& a, const Matrix<M, N>& b) noexcept
{
    ECK_MAT_LOOP() a(i, j) += b(i, j);
    return a;
}

template<size_t M, size_t N>
EckNfInlineNdCe Matrix<M, N> operator+(const Matrix<M, N>& a, float b) noexcept
{
    Matrix<M, N> r;
    ECK_MAT_LOOP() r(i, j) = a(i, j) + b;
    return r;
}
template<size_t M, size_t N>
EckNfInlineCe Matrix<M, N>& operator+=(Matrix<M, N>& a, float b) noexcept
{
    ECK_MAT_LOOP() a(i, j) += b;
    return a;
}

template<size_t M, size_t N>
EckNfInlineNdCe Matrix<M, N> operator-(const Matrix<M, N>& a, const Matrix<M, N>& b) noexcept
{
    Matrix<M, N> r;
    ECK_MAT_LOOP() r(i, j) = a(i, j) - b(i, j);
    return r;
}
template<size_t M, size_t N>
EckNfInlineNdCe Matrix<M, N> operator-(const Matrix<M, N>& a, float b) noexcept
{
    Matrix<M, N> r;
    ECK_MAT_LOOP() r(i, j) = a(i, j) - b;
    return r;
}

template<size_t M, size_t N>
EckNfInlineCe Matrix<M, N>& operator-=(Matrix<M, N>& a, const Matrix<M, N>& b) noexcept
{
    ECK_MAT_LOOP() a(i, j) -= b(i, j);
    return a;
}
template<size_t M, size_t N>
EckNfInlineCe Matrix<M, N>& operator-=(Matrix<M, N>& a, float b) noexcept
{
    ECK_MAT_LOOP() a(i, j) -= b;
    return a;
}

template<size_t M, size_t N, size_t P>
EckNfInlineNdCe Matrix<M, P> operator*(const Matrix<M, N>& a, const Matrix<N, P>& b) noexcept
{
    Matrix<M, P> r;
    for (size_t i = 0; i < M; ++i)
        for (size_t j = 0; j < P; ++j)
        {
            float s{};
            for (size_t k = 0; k < N; ++k)
                s += (a(i, k) * b(k, j));
            r(i, j) = s;
        }
    return r;
}
template<size_t N> requires (N == 2 || N == 3)
EckNfInlineNdCe auto operator*(const Vector2& a, const Matrix<2, N>& b) noexcept
{
    std::conditional_t<N == 2, Vector2, Vector3> r;
    for (size_t j = 0; j < N; ++j)
        r[j] = a.x * b(0, j) + a.y * b(1, j);
    return r;
}
template<size_t N> requires (N == 2 || N == 3)
EckNfInlineNdCe auto operator*(const Vector3& a, const Matrix<3, N>& b) noexcept
{
    std::conditional_t<N == 2, Vector2, Vector3> r;
    for (size_t j = 0; j < N; ++j)
        r[j] = a.x * b(0, j) + a.y * b(1, j) + a.z * b(2, j);
    return r;
}

template<size_t M> requires (M == 2 || M == 3)
EckNfInlineNdCe auto operator*(const Matrix<M, 2>& a, const Vector2& b) noexcept
{
    std::conditional_t<M == 2, Vector2, Vector3> r;
    for (size_t i = 0; i < M; ++i)
        r[i] = a(i, 0) * b.x + a(i, 1) * b.y;
    return r;
}
template<size_t M> requires (M == 2 || M == 3)
EckNfInlineNdCe auto operator*(const Matrix<M, 3>& a, const Vector3& b) noexcept
{
    std::conditional_t<M == 2, Vector2, Vector3> r;
    for (size_t i = 0; i < M; ++i)
        r[i] = a(i, 0) * b.x + a(i, 1) * b.y + a(i, 2) * b.z;
    return r;
}

template<size_t M, size_t N>
EckNfInlineNdCe Matrix<M, N> operator*(const Matrix<M, N>& a, float b) noexcept
{
    Matrix<M, N> r;
    ECK_MAT_LOOP() r(i, j) = a(i, j) * b;
    return r;
}
template<size_t M, size_t N>
EckNfInlineCe Matrix<M, N>& operator*=(Matrix<M, N>& a, float b) noexcept
{
    ECK_MAT_LOOP() a(i, j) *= b;
    return a;
}

template<size_t M, size_t N>
EckNfInlineNdCe Matrix<M, N> operator/(const Matrix<M, N>& a, const Matrix<M, N>& b) noexcept
{
    Matrix<M, N> r;
    ECK_MAT_LOOP() r(i, j) = a(i, j) / b(i, j);
    return r;
}
template<size_t M, size_t N>
EckNfInlineNdCe Matrix<M, N> operator/(const Matrix<M, N>& a, float b) noexcept
{
    Matrix<M, N> r;
    ECK_MAT_LOOP() r(i, j) = a(i, j) / b;
    return r;
}
template<size_t M, size_t N>
EckNfInlineCe Matrix<M, N>& operator/=(Matrix<M, N>& a, const Matrix<M, N>& b) noexcept
{
    ECK_MAT_LOOP() a(i, j) /= b(i, j);
    return a;
}
template<size_t M, size_t N>
EckNfInlineCe Matrix<M, N>& operator/=(Matrix<M, N>& a, float b) noexcept
{
    ECK_MAT_LOOP() a(i, j) /= b;
    return a;
}

template<size_t M, size_t N>
EckNfInlineNdCe Matrix<M, N> operator-(const Matrix<M, N>& a) noexcept
{
    Matrix<M, N> r;
    ECK_MAT_LOOP() r(i, j) = -a(i, j);
    return r;
}

template<size_t M, size_t N>
EckNfInlineNdCe Matrix<M, N> operator+(float a, const Matrix<M, N>& b) noexcept
{
    return b + a;
}
template<size_t M, size_t N>
EckNfInlineNdCe Matrix<M, N> operator-(float a, const Matrix<M, N>& b) noexcept
{
    Matrix<M, N> r;
    ECK_MAT_LOOP() r(i, j) = a - b(i, j);
    return r;
}
template<size_t M, size_t N>
EckNfInlineNdCe Matrix<M, N> operator*(float a, const Matrix<M, N>& b) noexcept
{
    return b * a;
}
template<size_t M, size_t N>
EckNfInlineNdCe Matrix<M, N> operator/(float a, const Matrix<M, N>& b) noexcept
{
    Matrix<M, N> r;
    ECK_MAT_LOOP() r(i, j) = a / b(i, j);
    return r;
}

template<size_t M, size_t N>
EckNfInlineNdCe bool operator==(const Matrix<M, N>& a, const Matrix<M, N>& b) noexcept
{
    ECK_MAT_LOOP() if (a(i, j) != b(i, j)) return false;
    return true;
}

template<size_t N>
EckNfInlineNdCe Matrix<N, N> IdentityMatrix() noexcept
{
    Matrix<N, N> r{};
    for (size_t i = 0; i < N; ++i)
        r(i, i) = 1.f;
    return r;
}

template<size_t M, size_t N>
EckNfInlineNdCe Matrix<N, M> Transpose(const Matrix<M, N>& a) noexcept
{
    Matrix<N, M> r;
    ECK_MAT_LOOP() r(j, i) = a(i, j);
    return r;
}

EckNfInlineNdCe float Determinant(
    const Matrix2x2& a,
    _Out_opt_ Matrix2x2* pAdjugate = nullptr) noexcept
{
    const auto
        a00 = a(0, 0), a01 = a(0, 1),
        a10 = a(1, 0), a11 = a(1, 1);
    if (pAdjugate)
    {
        auto& a = *pAdjugate;
        a(0, 0) = a11;
        a(0, 1) = -a01;
        a(1, 0) = -a10;
        a(1, 1) = a00;
    }
    return a00 * a11 - a01 * a10;
}

EckNfInlineNdCe float Determinant(
    const Matrix3x3& a,
    _Out_opt_ Matrix3x3* pAdjugate = nullptr) noexcept
{
    const auto a00 = a(0, 0), a01 = a(0, 1), a02 = a(0, 2);
    const auto a10 = a(1, 0), a11 = a(1, 1), a12 = a(1, 2);
    const auto a20 = a(2, 0), a21 = a(2, 1), a22 = a(2, 2);
    const auto b00 = a11 * a22 - a12 * a21;
    const auto b10 = a12 * a20 - a10 * a22;
    const auto b20 = a10 * a21 - a11 * a20;
    if (pAdjugate)
    {
        auto& a = *pAdjugate;
        a(0, 0) = b00;
        a(0, 1) = a02 * a21 - a01 * a22;
        a(0, 2) = a01 * a12 - a02 * a11;
        a(1, 0) = b10;
        a(1, 1) = a00 * a22 - a02 * a20;
        a(1, 2) = a02 * a10 - a00 * a12;
        a(2, 0) = b20;
        a(2, 1) = a01 * a20 - a00 * a21;
        a(2, 2) = a00 * a11 - a01 * a10;
    }
    return a00 * b00 + a01 * b10 + a02 * b20;
}

EckInlineNdCe Matrix2x2 Inverse(const Matrix2x2& a) noexcept
{
    Matrix2x2 Adjugate{};
    const auto Det = Determinant(a, &Adjugate);
    return Adjugate / Det;
}
EckInlineNdCe Matrix3x3 Inverse(const Matrix3x3& a) noexcept
{
    Matrix3x3 Adjugate{};
    const auto Det = Determinant(a, &Adjugate);
    return Adjugate / Det;
}
#undef ECK_MAT_LOOP
#pragma endregion Matrix

#pragma region Rect
struct Rect
{
    float left;
    float top;
    float right;
    float bottom;
};
struct RectU
{
    UINT left;
    UINT top;
    UINT right;
    UINT bottom;
};
struct RectI
{
    int left;
    int top;
    int right;
    int bottom;
};

EckInlineNdCe Rect MakeRect(const RECT& rc) noexcept
{
    return { (float)rc.left, (float)rc.top, (float)rc.right, (float)rc.bottom };
}

#ifdef _D2D1_H_
EckInlineNdCe const D2D1_RECT_F& MakeD2DRectF(const Kw::Rect& rc) noexcept { return *(D2D1_RECT_F*)&rc; }
EckInlineNdCe const Rect& MakeRect(const D2D1_RECT_F& rc) noexcept { return *(Rect*)&rc; }
EckInlineNdCe const D2D1_POINT_2F& MakeD2DPointF(const Kw::Vector2& pt) noexcept { return *(D2D1_POINT_2F*)&pt; }
#endif

#pragma endregion Rect

KW2D_NAMESPACE_END

ECK_DEF_RECT_TRAITS(Kw::Rect);
ECK_DEF_RECT_TRAITS(Kw::RectU);
ECK_DEF_RECT_TRAITS(Kw::RectI);

EckInlineNdCe bool PointInRect(CcpRect auto const& rc, Kw::Vector2 pt) noexcept
{
    return ((pt.x >= (float)rc.left) && (pt.x < (float)rc.right) &&
        (pt.y >= (float)rc.top) && (pt.y < (float)rc.bottom));
}

EckInlineNdCe RECT MakeRect(const Kw::Rect& rc) noexcept
{
    return { (int)rc.left, (int)rc.top, (int)rc.right, (int)rc.bottom };
}
ECK_NAMESPACE_END