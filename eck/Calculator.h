#pragma once
#include "StringUtility.h"

#define ECK_CE_NAMESPACE_BEGIN      namespace Ce {
#define ECK_CE_NAMESPACE_END        }

ECK_NAMESPACE_BEGIN
ECK_CE_NAMESPACE_BEGIN
enum class Result
{
    Ok,
    InvalidChar,// 非法字符
    AdjacentOp,	// 相邻运算符
    UnmatchedParentheses,// 括号不匹配
    OpError,	// 运算符错误
};

namespace Detail
{
    enum class Function : char
    {
        None,
        Ln,
        Log,

        Sin,
        Cos,
        Tan,
        Cot,
        Sec,
        Csc,

        Asin,
        Acos,
        Atan,
        Acot,
        Asec,
        Acsc,

        Sqrt,
        Sinh,
        Cosh,

        Ceil,
        Floor,
        Round,
        Max,
    };

    struct FUNC_NAME
    {
        WCHAR szNameW[6];
        char szNameA[6];
        Function eOp;
        SCHAR cchName;
    };
#undef ECK_TEMP
#define ECK_TEMP(s) L## #s, #s, Function::s, CHAR(sizeof(#s) - 1)
    constexpr inline FUNC_NAME CalcExpFuncList[]
    {
        { ECK_TEMP(Ln) },
        { ECK_TEMP(Log) },
        { ECK_TEMP(Sin) },
        { ECK_TEMP(Cos) },
        { ECK_TEMP(Tan) },
        { ECK_TEMP(Cot) },
        { ECK_TEMP(Sec) },
        { ECK_TEMP(Csc) },
        { ECK_TEMP(Asin) },
        { ECK_TEMP(Acos) },
        { ECK_TEMP(Atan) },
        { ECK_TEMP(Acot) },
        { ECK_TEMP(Asec) },
        { ECK_TEMP(Acsc) },
        { ECK_TEMP(Sqrt) },
        { ECK_TEMP(Sinh) },
        { ECK_TEMP(Cosh) },
        { ECK_TEMP(Ceil) },
        { ECK_TEMP(Floor) },
        { ECK_TEMP(Round) },
    };
#undef ECK_TEMP

    struct CONST_NAME
    {
        WCHAR szNameW[3];
        char szNameA[3];
        CHAR cchName;
        double lfVal;
    };
#undef ECK_TEMP
#define ECK_TEMP(s) L## #s, #s, CHAR(sizeof(#s) - 1)
    constexpr inline CONST_NAME CalcExpConstList[]
    {
        { ECK_TEMP(Pi), 3.141592653589793 },
        { ECK_TEMP(E), 2.718281828459045 },
    };
#undef ECK_TEMP

    EckInlineNdCe int Priority(auto ch) noexcept
    {
        switch (ch)
        {
        case '+':
        case '-':
            return 1;
        case '*':
        case '/':
        case '%':
            return 2;
        case '^':
            return 3;
        case '(':
            return 5;
        default:
            return 4;
        }
    }

    template<class TChar>
    BOOL DoOperation(std::vector<double>& vNum, std::vector<TChar>& vOp) noexcept
    {
        if (vOp.empty() || vNum.empty())
            return FALSE;
        const auto bBinary = (vOp.back() >= (TChar)Function::Max);
        if (bBinary ? (vNum.size() < 2) : FALSE)
            return FALSE;
        const auto chOp = vOp.back();
        vOp.pop_back();
        const auto n2 = vNum.back();
        vNum.pop_back();
        double n1;
        if (bBinary)
        {
            n1 = vNum.back();
            vNum.pop_back();
        }
        else
            n1 = 0.;

        switch (chOp)
        {
        case '+': vNum.push_back(n1 + n2);      break;
        case '-': vNum.push_back(n1 - n2);      break;
        case '*': vNum.push_back(n1 * n2);      break;
        case '/': vNum.push_back(n1 / n2);      break;
        case '%': vNum.push_back(fmod(n1, n2)); break;
        case '^': vNum.push_back(pow(n1, n2));  break;
        case (TChar)Function::Ln:      vNum.push_back(log(n2));        break;
        case (TChar)Function::Log:     vNum.push_back(log10(n2));      break;
        case (TChar)Function::Sin:     vNum.push_back(sin(n2));        break;
        case (TChar)Function::Cos:     vNum.push_back(cos(n2));        break;
        case (TChar)Function::Tan:     vNum.push_back(tan(n2));        break;
        case (TChar)Function::Cot:     vNum.push_back(1. / tan(n2));   break;
        case (TChar)Function::Sec:     vNum.push_back(1. / cos(n2));   break;
        case (TChar)Function::Csc:     vNum.push_back(1. / sin(n2));   break;
        case (TChar)Function::Asin:    vNum.push_back(asin(n2));       break;
        case (TChar)Function::Acos:    vNum.push_back(acos(n2));       break;
        case (TChar)Function::Atan:    vNum.push_back(atan(n2));       break;
        case (TChar)Function::Acot:    vNum.push_back(atan(1. / n2));  break;
        case (TChar)Function::Asec:    vNum.push_back(acos(1. / n2));  break;
        case (TChar)Function::Acsc:    vNum.push_back(asin(1. / n2));  break;
        case (TChar)Function::Sqrt:    vNum.push_back(sqrt(n2));       break;
        case (TChar)Function::Sinh:    vNum.push_back(sinh(n2));       break;
        case (TChar)Function::Cosh:    vNum.push_back(cosh(n2));       break;
        case (TChar)Function::Ceil:    vNum.push_back(ceil(n2));       break;
        case (TChar)Function::Floor:   vNum.push_back(floor(n2));      break;
        case (TChar)Function::Round:   vNum.push_back(round(n2));      break;
        default:return FALSE;
        }
        return TRUE;
    }

    template<class TChar>
    struct CharTraits {};

    template<>
    struct CharTraits<CHAR>
    {
        static BOOL IsDigit(char ch) noexcept { return isdigit(ch); }
        static BOOL IsAlpha(char ch) noexcept { return isalpha(ch); }
        static double ToDouble(const char* p, const char** ppEnd) noexcept { return strtod(p, (char**)ppEnd); }
        static PCSTR GetFuntionName(const FUNC_NAME& e) noexcept { return e.szNameA; }
        static PCSTR GetConstName(const CONST_NAME& e) noexcept { return e.szNameA; }
    };

    template<>
    struct CharTraits<WCHAR>
    {
        static BOOL IsDigit(WCHAR ch) noexcept { return iswdigit(ch); }
        static BOOL IsAlpha(WCHAR ch) noexcept { return iswalpha(ch); }
        static double ToDouble(const WCHAR* p, const WCHAR** ppEnd) noexcept { return wcstod(p, (WCHAR**)ppEnd); }
        static PCWSTR GetFuntionName(const FUNC_NAME& e) noexcept { return e.szNameW; }
        static PCWSTR GetConstName(const CONST_NAME& e) noexcept { return e.szNameW; }
    };
}

template<CcpCharPointer TPtr>
inline Result CalculateExpression(
    _Out_ double& lfResult,
    _In_reads_or_z_(cchExp) TPtr pszExp,
    int cchExp = -1) noexcept
{
    using TChar = CharFromPointer_T<TPtr>;
    using TTraits = Detail::CharTraits<TChar>;

    lfResult = 0.;
    if (cchExp < 0)
        cchExp = (int)TcsLength(pszExp);
    std::vector<double> vNum{};
    std::vector<TChar> vOp{};
    BOOL bNumberBegin = TRUE;// TRUE = 期待数字
    for (auto p = pszExp; p < pszExp + cchExp; ++p)
    {
        const auto ch = *p;
        if (ch == ' ' || ch == '\t')
            continue;
        if (TTraits::IsDigit(ch))
        {
            bNumberBegin = FALSE;
            vNum.push_back(TTraits::ToDouble(p, &p));
            --p;
            continue;
        }
        else if (ch == '(')
        {
            bNumberBegin = TRUE;
            vOp.push_back(ch);
            continue;
        }
        else if (ch == ')')
        {
            bNumberBegin = FALSE;
            while (!vOp.empty() && vOp.back() != '(')
                if (!Detail::DoOperation(vNum, vOp))
                    return Result::OpError;
            if (vOp.empty())
                return Result::UnmatchedParentheses;
            vOp.pop_back();
        }
        else if (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '%' || ch == '^')
        {
            if (bNumberBegin)
                if (ch == '+')
                    continue;
                else if (ch == '-')
                {
                    vNum.push_back(0.);
                    bNumberBegin = FALSE;
                }
                else
                    return Result::AdjacentOp;
            if (vOp.empty() || Detail::Priority(ch) > Detail::Priority(vOp.back()))
            {
                vOp.push_back(ch);
                continue;
            }
            else
            {
                while (!vOp.empty() &&
                    vOp.back() != '(' &&
                    Detail::Priority(ch) <= Detail::Priority(vOp.back()))
                {
                    if (!Detail::DoOperation(vNum, vOp))
                        return Result::OpError;
                }
                vOp.push_back(ch);
            }
        }
        else if (TTraits::IsAlpha(ch))
        {
            for (const auto& e : Detail::CalcExpConstList)
                if (TcsCompareMaxLengthI(p, TTraits::GetConstName(e), e.cchName) == 0)
                {
                    vNum.push_back(e.lfVal);
                    p += (e.cchName - 1);
                    bNumberBegin = FALSE;
                    goto ExitSearchSym;
                }
            for (const auto& e : Detail::CalcExpFuncList)
                if (TcsCompareMaxLengthI(p, TTraits::GetFuntionName(e), e.cchName) == 0)
                {
                    vOp.push_back((TChar)e.eOp);
                    p += (e.cchName - 1);
                    bNumberBegin = TRUE;
                    goto ExitSearchSym;
                }
            return Result::InvalidChar;
        ExitSearchSym:;
        }
        else
            return Result::InvalidChar;
    }
    while (!vOp.empty())
        if (!Detail::DoOperation(vNum, vOp))
            return Result::OpError;
    if (vNum.size() != 1)
        return Result::OpError;
    lfResult = vNum.back();
    return Result::Ok;
}
ECK_CE_NAMESPACE_END
ECK_NAMESPACE_END