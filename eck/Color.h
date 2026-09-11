#pragma once
#include "Utility.h"

ECK_NAMESPACE_BEGIN
namespace Colorref
{
    inline constexpr COLORREF
        Red = 0x0000FF,         // 红色
        Green = 0x00FF00,       // 绿色
        Blue = 0xFF0000,        // 蓝色
        Yellow = 0x00FFFF,      // 黄色
        Magenta = 0xFF00FF,     // 品红/洋红
        Cyan = 0xFFFF00,        // 艳青/青色
        Aqua = Cyan,

        Maroon = 0x000080,      // 红褐/暗红
        OfficeGreen = 0x008000, // 墨绿/暗绿
        Olive = 0x008080,       // 褐绿/暗黄
        NavyBlue = 0x800000,    // 藏青/暗蓝
        Patriarch = 0x800080,   // 紫红/暗洋红
        Teal = 0x808000,        // 深青/暗青

        Silver = 0xC0C0C0,      // 浅灰/亮灰
        MoneyGreen = 0xC0DCC0,  // 美元绿
        LightBlue = 0xF0CAA6,   // 浅蓝/天蓝

        Gray = 0x808080,        // 灰色/暗灰
        NeutralGray = 0xA4A0A0, // 中性灰
        MilkyWhite = 0xF0FBFF,  // 乳白

        Black = 0x000000,       // 黑色
        White = 0xFFFFFF,       // 白色

        BlueGray = 0xFF8080,    // 蓝灰
        PurplishBlue = 0xE03058,// 藏蓝
        TenderGreen = 0x00E080, // 嫩绿
        Turquoise = 0x80E000,   // 青绿
        YellowishBrown = 0x0060C0,// 黄褐
        Pink = 0xFFA8FF,        // 粉红
        BrightYellow = 0x00D8D8,// 嫩黄
        JadeWhite = 0xECECEC,   // 银白
        Purple = 0xFF0090,      // 紫色
        Azure = 0xFF8800,       // 天蓝
        Celadon = 0x80A080,     // 灰绿
        CyanBlue = 0xC06000,    // 青蓝
        Orange = 0x0080FF,      // 橙黄
        Peachblow = 0x8050FF,   // 桃红
        HibiscusRed = 0xC080FF, // 芙红
        DeepGray = 0x606060     // 深灰
        ;
}

EckInlineNdCe UINT ReverseColorref(COLORREF cr) noexcept
{
    return BytesToInteger<UINT>(
        GetIntegerByte<2>(cr),
        GetIntegerByte<1>(cr),
        GetIntegerByte<0>(cr),
        0);
}

EckInlineNdCe ARGB ColorrefToArgb(COLORREF cr, BYTE byAlpha = 0xFF) noexcept
{
    return ReverseColorref(cr) | (byAlpha << 24);
}
EckInlineNdCe COLORREF ArgbToColorref(ARGB argb, _Out_opt_ BYTE* pbyAlpha = nullptr) noexcept
{
    if (pbyAlpha)
        *pbyAlpha = GetIntegerByte<3>(argb);
    return ReverseColorref(argb);
}

#ifdef _D2D1_H_
EckInlineNdCe D2D1_COLOR_F ArgbToD2DColorF(ARGB argb) noexcept
{
    return D2D1_COLOR_F
    {
        GetIntegerByte<2>(argb) / 255.f,
        GetIntegerByte<1>(argb) / 255.f,
        GetIntegerByte<0>(argb) / 255.f,
        GetIntegerByte<3>(argb) / 255.f
    };
}
EckInlineNdCe ARGB D2DColorFToArgb(const D2D1_COLOR_F& cr) noexcept
{
    return BytesToInteger<ARGB>(
        BYTE(cr.r * 255.f),
        BYTE(cr.g * 255.f),
        BYTE(cr.b * 255.f),
        BYTE(cr.a * 255.f));
}
EckInlineNdCe D2D1_COLOR_F RgbToD2DColorF(UINT rgb, float fAlpha = 1.f) noexcept
{
    return D2D1_COLOR_F
    {
        GetIntegerByte<2>(rgb) / 255.f,
        GetIntegerByte<1>(rgb) / 255.f,
        GetIntegerByte<0>(rgb) / 255.f,
        fAlpha
    };
}

EckInlineNdCe COLORREF D2DColorFToColorref(const D2D1_COLOR_F& cr) noexcept
{
    return BytesToInteger<COLORREF>(
        BYTE(cr.r * 255.f),
        BYTE(cr.g * 255.f),
        BYTE(cr.b * 255.f),
        0);
}
EckInlineNdCe D2D1_COLOR_F ColorrefToD2DColorF(COLORREF cr, float fAlpha = 1.f) noexcept
{
    return D2D1_COLOR_F
    {
        GetRValue(cr) / 255.f,
        GetGValue(cr) / 255.f,
        GetBValue(cr) / 255.f,
        fAlpha
    };
}
#endif// _D2D1_H_

EckInlineNdCe COLORREF ColorrefAlphaBlend(COLORREF cr, COLORREF crBK, BYTE byAlpha) noexcept
{
    return BytesToInteger<COLORREF>(
        (GetIntegerByte<0>(cr) * byAlpha + GetIntegerByte<0>(crBK) * (0xFF - byAlpha)) / 0xFF,
        (GetIntegerByte<1>(cr) * byAlpha + GetIntegerByte<1>(crBK) * (0xFF - byAlpha)) / 0xFF,
        (GetIntegerByte<2>(cr) * byAlpha + GetIntegerByte<2>(crBK) * (0xFF - byAlpha)) / 0xFF,
        0);
}
EckInlineNdCe ARGB ArgbAlphaBlend(ARGB cr, ARGB crBK) noexcept
{
    const BYTE byAlpha = GetIntegerByte<3>(cr);
    return BytesToInteger<ARGB>(
        (GetIntegerByte<0>(cr) * byAlpha + GetIntegerByte<0>(crBK) * (0xFF - byAlpha)) / 0xFF,
        (GetIntegerByte<1>(cr) * byAlpha + GetIntegerByte<1>(crBK) * (0xFF - byAlpha)) / 0xFF,
        (GetIntegerByte<2>(cr) * byAlpha + GetIntegerByte<2>(crBK) * (0xFF - byAlpha)) / 0xFF,
        (GetIntegerByte<3>(cr) * byAlpha + GetIntegerByte<3>(crBK) * (0xFF - byAlpha)) / 0xFF);
}

EckInlineNdCe ARGB MakeArgb(BYTE a, BYTE r, BYTE g, BYTE b) noexcept
{
    return BytesToInteger<ARGB>(b, g, r, a);
}

template<CcpNumber TOut, CcpNumber TIn>
EckInlineNdCe TOut CalculateGray(TIn r, TIn g, TIn b) noexcept
{
    return TOut(
        0.21264934272065283 * r +
        0.7151691357059038 * g +
        0.07218152157344333 * b);
}

EckInlineNdCe BYTE GetArgbR(ARGB argb) noexcept { return GetIntegerByte<2>(argb); }
EckInlineNdCe BYTE GetArgbG(ARGB argb) noexcept { return GetIntegerByte<1>(argb); }
EckInlineNdCe BYTE GetArgbB(ARGB argb) noexcept { return GetIntegerByte<0>(argb); }
EckInlineNdCe BYTE GetArgbA(ARGB argb) noexcept { return GetIntegerByte<3>(argb); }

EckInlineNdCe BOOL IsColorLight(BYTE r, BYTE g, BYTE b) noexcept
{
    return CalculateGray<BYTE>(r, g, b) >= 128_by;
}
EckInlineNdCe BOOL IsColorLightArgb(ARGB argb) noexcept
{
    return IsColorLight(GetArgbR(argb), GetArgbG(argb), GetArgbB(argb));
}
EckInlineNdCe BOOL IsColorLightColorref(COLORREF cr) noexcept
{
    return IsColorLight(GetRValue(cr), GetGValue(cr), GetBValue(cr));
}

template<CcpNumber TOut, CcpNumber TIn>
inline constexpr void RgbToYuv(
    TIn r, TIn g, TIn b,
    _Out_ TOut& y, _Out_ TOut& u, _Out_ TOut& v) noexcept
{
    y = TOut(0.299f * r + 0.587f * g + 0.114f * b);
    u = TOut(-0.14713f * r - 0.28886f * g + 0.436f * b);
    v = TOut(0.615f * r - 0.51499f * g - 0.10001f * b);
}

template<CcpNumber TIn>
EckNfInlineNd float CalculateColorDifference(
    TIn r1, TIn g1, TIn b1,
    TIn r2, TIn g2, TIn b2) noexcept
{
    float y1, u1, v1, y2, u2, v2;
    RgbToYuv(r1, g1, b1, y1, u1, v1);
    RgbToYuv(r2, g2, b2, y2, u2, v2);
    return sqrtf((y1 - y2) * (y1 - y2) + (u1 - u2) * (u1 - u2) + (v1 - v2) * (v1 - v2));
}
EckInlineNd float CalculateColorrefDifference(COLORREF cr1, COLORREF cr2) noexcept
{
    return CalculateColorDifference<BYTE>(
        GetRValue(cr1), GetGValue(cr1), GetBValue(cr1),
        GetRValue(cr2), GetGValue(cr2), GetBValue(cr2));
}
EckInlineNd float CalculateArgbDifference(ARGB argb1, ARGB argb2) noexcept
{
    return CalculateColorDifference<BYTE>(
        GetArgbR(argb1), GetArgbG(argb1), GetArgbB(argb1),
        GetArgbR(argb2), GetArgbG(argb2), GetArgbB(argb2));
}

EckInlineNdCe COLORREF AdjustColorrefLuma(COLORREF cr, int iPrecent) noexcept
{
    return RGB(
        std::min(GetRValue(cr) * iPrecent / 100, 0xFF),
        std::min(GetGValue(cr) * iPrecent / 100, 0xFF),
        std::min(GetBValue(cr) * iPrecent / 100, 0xFF));
}
EckInlineNdCe COLORREF DeltaColorrefLuma(COLORREF cr, int d) noexcept
{
    return RGB(
        std::clamp(GetRValue(cr) + d, 0, 0xFF),
        std::clamp(GetGValue(cr) + d, 0, 0xFF),
        std::clamp(GetBValue(cr) + d, 0, 0xFF));
}
EckInlineNdCe COLORREF DeltaColorrefLuma(COLORREF cr, float d) noexcept
{
    return RGB(
        std::clamp(int(GetRValue(cr) + d * 255), 0, 0xFF),
        std::clamp(int(GetGValue(cr) + d * 255), 0, 0xFF),
        std::clamp(int(GetBValue(cr) + d * 255), 0, 0xFF));
}

#ifdef _D2D1_H_
EckInlineNdCe D2D1_COLOR_F LerpD2DColorF(
    const D2D1_COLOR_F& c1,
    const D2D1_COLOR_F& c2,
    float fLerp) noexcept
{
    return {
        c1.r + (c2.r - c1.r) * fLerp,
        c1.g + (c2.g - c1.g) * fLerp,
        c1.b + (c2.b - c1.b) * fLerp,
        c1.a + (c2.a - c1.a) * fLerp,
    };
}
#endif // _D2D1_H_
EckInlineNdCe ARGB LerpArgb(ARGB c1, ARGB c2, float fLerp) noexcept
{
    return BytesToInteger<ARGB>(
        BYTE(GetArgbB(c1) + (GetArgbB(c2) - GetArgbB(c1)) * fLerp),
        BYTE(GetArgbG(c1) + (GetArgbG(c2) - GetArgbG(c1)) * fLerp),
        BYTE(GetArgbR(c1) + (GetArgbR(c2) - GetArgbR(c1)) * fLerp),
        BYTE(GetArgbA(c1) + (GetArgbA(c2) - GetArgbA(c1)) * fLerp));
}

template<CcpNumber TOut, CcpNumber TIn>
inline constexpr void RgbToHsl(
    TIn r, TIn g, TIn b,
    _Out_ TOut& h, _Out_ TOut& s, _Out_ TOut& l) noexcept
{
    const auto fR = (float)r / 255.f;
    const auto fG = (float)g / 255.f;
    const auto fB = (float)b / 255.f;
    const auto fMax = std::max(fR, std::max(fG, fB));
    const auto fMin = std::min(fR, std::min(fG, fB));
    const auto fL = (fMax + fMin) / 2.f;
    l = (TOut)fL;
    if (fMax == fMin)
    {
        h = s = 0.f;
        return;
    }
    const auto d = fMax - fMin;
    s = TOut(fL > 0.5f ? d / (2.f - fMax - fMin) : d / (fMax + fMin));
    float fH;
    if (fMax == fR)
        fH = (fG - fB) / d + (fG < fB ? 6.f : 0.f);
    else if (fMax == fG)
        fH = (fB - fR) / d + 2.f;
    else
        fH = (fR - fG) / d + 4.f;
    h = TOut(fH / 6.f);
}

namespace Detail
{
    inline constexpr float HueToChannel(float p, float q, float t) noexcept
    {
        if (t < 0.f)
            t += 1.f;
        if (t > 1.f)
            t -= 1.f;
        if (t < 1.f / 6.f)
            return p + (q - p) * 6.f * t;
        if (t < 1.f / 2.f)
            return q;
        if (t < 2.f / 3.f)
            return p + (q - p) * (2.f / 3.f - t) * 6.f;
        return p;
    }
}

template<CcpNumber TOut, std::floating_point TIn>
inline constexpr void HslToRgb(
    TIn h, TIn s, TIn l,
    _Out_ TOut& r, _Out_ TOut& g, _Out_ TOut& b) noexcept
{
    if (s == 0.f)
    {
        r = g = b = TOut(l * 255.f);
        return;
    }
    const auto q = float(l < 0.5f ? l * (1.f + s) : l + s - l * s);
    const auto p = float(2.f * l - q);
    r = TOut(Detail::HueToChannel(p, q, h + 1.f / 3.f) * 255.f);
    g = TOut(Detail::HueToChannel(p, q, h) * 255.f);
    b = TOut(Detail::HueToChannel(p, q, h - 1.f / 3.f) * 255.f);
}

template<std::floating_point TOut, CcpNumber TIn>
inline constexpr void RgbToHsv(
    TIn r, TIn g, TIn b,
    _Out_ TOut& h, _Out_ TOut& s, _Out_ TOut& v) noexcept
{
    const auto fR = (float)r / 255.f;
    const auto fG = (float)g / 255.f;
    const auto fB = (float)b / 255.f;
    const auto fMax = std::max(fR, std::max(fG, fB));
    const auto fMin = std::min(fR, std::min(fG, fB));
    v = TOut(fMax);
    const auto d = fMax - fMin;
    s = TOut(fMax == 0.f ? 0.f : d / fMax);

    float fH;
    if (fMax == fMin)
        fH = 0.f;
    else if (fMax == fR)
        fH = (fG - fB) / d + (fG < fB ? 6.f : 0.f);
    else if (fMax == fG)
        fH = (fB - fR) / d + 2.f;
    else
        fH = (fR - fG) / d + 4.f;
    h = TOut(fH / 6.f);
}

template<CcpNumber TOut, std::floating_point TIn>
inline constexpr void HsvToRgb(
    TIn h, TIn s, TIn v,
    _Out_ TOut& r, _Out_ TOut& g, _Out_ TOut& b) noexcept
{
    if (s == 0.f)
    {
        r = g = b = TOut(v * 255.f);
        return;
    }
    const auto i = int(h * 6.f);
    const auto f = h * 6.f - i;
    const auto p = v * (1.f - s);
    const auto q = v * (1.f - f * s);
    const auto t = v * (1.f - (1.f - f) * s);
    switch (i % 6)
    {
    case 0: r = TOut(v * 255.f); g = TOut(t * 255.f); b = TOut(p * 255.f); break;
    case 1: r = TOut(q * 255.f); g = TOut(v * 255.f); b = TOut(p * 255.f); break;
    case 2: r = TOut(p * 255.f); g = TOut(v * 255.f); b = TOut(t * 255.f); break;
    case 3: r = TOut(p * 255.f); g = TOut(q * 255.f); b = TOut(v * 255.f); break;
    case 4: r = TOut(t * 255.f); g = TOut(p * 255.f); b = TOut(v * 255.f); break;
    case 5: r = TOut(v * 255.f); g = TOut(p * 255.f); b = TOut(q * 255.f); break;
    default: ECK_UNREACHABLE;
    }
}

namespace Detail
{
    /*
    Copyright (C) 2016-2018 Frank Richter

    This software is provided 'as-is', without any express or implied
    warranty.  In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.
    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.
    3. This notice may not be removed or altered from any source distribution.

    Original source:
    https://github.com/res2k/Windows10Colors
    */

    EckInlineNdCe void AccentColorLighter(
        float vBase,
        float sPrev, float vPrev,
        _Out_ float& s, _Out_ float& v) noexcept
    {
        v = std::min(vPrev + vBase / 4.f, 1.f);
        s = (v >= 0.7f) ? (sPrev * 0.75f) : sPrev;
    }
    EckInlineNdCe void AccentColorDarker(
        float vBase,
        float vPrev,
        _Out_ float& v) noexcept
    {
        v = std::max(vPrev - vBase / 4.f, 0.f);
    }
}

struct ACCENT_COLOR_SET
{
    ARGB argbLight1;
    ARGB argbLight2;
    ARGB argbLight3;
    ARGB argbDark1;
    ARGB argbDark2;
    ARGB argbDark3;
};

inline constexpr void CalculateAccentColorSet(ARGB argbBase, _Out_ ACCENT_COLOR_SET& acs) noexcept
{
    argbBase = LerpArgb(
        0xFF'FFFFFF,
        argbBase | 0xFF'000000,
        GetArgbA(argbBase) / 255.f);

    const auto
        r = GetArgbR(argbBase),
        g = GetArgbG(argbBase),
        b = GetArgbB(argbBase);

    float h, s, v;
    RgbToHsv(r, g, b, h, s, v);

    float s1, v1;
    int x, y, z;// 临时RGB

    Detail::AccentColorLighter(v, s, v, s1, v1);
    HsvToRgb(h, s1, v1, x, y, z);
    acs.argbLight1 = MakeArgb(0xFF, x, y, z);

    Detail::AccentColorLighter(v, s1, v1, s1, v1);
    HsvToRgb(h, s1, v1, x, y, z);
    acs.argbLight2 = MakeArgb(0xFF, x, y, z);

    Detail::AccentColorLighter(v, s1, v1, s1, v1);
    HsvToRgb(h, s1, v1, x, y, z);
    acs.argbLight3 = MakeArgb(0xFF, x, y, z);

    Detail::AccentColorDarker(v, v, v1);
    HsvToRgb(h, s, v1, x, y, z);
    acs.argbDark1 = MakeArgb(0xFF, x, y, z);

    Detail::AccentColorDarker(v, v1, v1);
    HsvToRgb(h, s, v1, x, y, z);
    acs.argbDark2 = MakeArgb(0xFF, x, y, z);

    Detail::AccentColorDarker(v, v1, v1);
    HsvToRgb(h, s, v1, x, y, z);
    acs.argbDark3 = MakeArgb(0xFF, x, y, z);
}
ECK_NAMESPACE_END