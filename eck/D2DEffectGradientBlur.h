#pragma once
#include "D2DEffectDefine.h"
#include "ComPtr.h"
#include "CByteBuffer.h"

#include <d2d1effecthelpers.h>
#include <d3dcompiler.h>

ECK_NAMESPACE_BEGIN
constexpr inline std::string_view FxPsGradientBlur = R"(
Texture2D<float4> gTexture0 : register(t0);
SamplerState gSampler0 : register(s0);

cbuffer BlurParams : register(b0)
{
    float2 blurDirection;
    float radiusMin;      // in pixel
    float radiusMax;      // in pixel
    float2 gradientRange; // in pixel
    uint gradientAxis;    // 0 = horizontal, 1 = vertical
    uint dummy;
};

float4 PSMain(
    float4 posClip : SV_Position,
    float4 posScene : SCENE_POSITION,
    float4 uv : TEXCOORD0) : SV_Target
{
    const float t = saturate(
        (posScene[gradientAxis] - gradientRange.x) /
        (gradientRange.y - gradientRange.x));
    const float radius = lerp(radiusMin, radiusMax, t);

    const float a = 4.5f / (radius * radius);
    const float e = exp(-a);
    const float q2 = e * e;
    const float q4 = q2 * q2;
    const float q8 = q4 * q4;

    float w0 = e;
    float ratio = e * q2;
    float oddStep = q8;
    float d0 = 1.0f;

    float4 sum = gTexture0.SampleLevel(gSampler0, uv.xy, 0.0f);
    float wsum = 1.0f;

    const uint samples = (uint)ceil(radius * 0.5f);

    for (uint i = 0; i < samples; ++i)
    {
        const float w1 = w0 * ratio;
        const float w = w0 + w1;

        const float sampleDistance = d0 + w1 * rcp(w);
        const float2 offset = uv.zw * (blurDirection * sampleDistance);

        const float4 c0 = gTexture0.SampleLevel(gSampler0, uv.xy + offset, 0.0f);
        const float4 c1 = gTexture0.SampleLevel(gSampler0, uv.xy - offset, 0.0f);

        sum += (c0 + c1) * w;
        wsum += 2.0f * w;

        w0 *= oddStep;
        ratio *= q4;
        oddStep *= q8;

        d0 += 2.0f;
    }
    return sum * rcp(wsum);
}
)";

// {811032D0-41D8-43CF-A2F3-8F3084A38694}
constexpr inline GUID CLSID_GradientBlurEffect =
{ 0x811032d0, 0x41d8, 0x43cf, { 0xa2, 0xf3, 0x8f, 0x30, 0x84, 0xa3, 0x86, 0x94 } };

// {E099307C-BA71-4211-96C7-4909C5E67613}
constexpr inline GUID GUID_GradientBlurPixelShader =
{ 0xe099307c, 0xba71, 0x4211, { 0x96, 0xc7, 0x49, 0x09, 0xc5, 0xe6, 0x76, 0x13 } };

class CGradientBlurTransform final :
    public CUnknown<CGradientBlurTransform, ID2D1DrawTransform>
{
public:
    struct alignas(16) Constant
    {
        D2D1_VECTOR_2F BlurDirection;
        float fRadiusMin;
        float fRadiusMax;
        D2D1_VECTOR_2F GradientRange;
        UINT uGradientAxis;
        UINT uPadding;
    };
private:
    ComPtr<ID2D1DrawInfo> m_pDrawInfo{};
    BOOL m_bHorizontal{ TRUE };
    Constant m_Constants{};

    void UpdateBlurDirection() noexcept
    {
        if (m_bHorizontal)
            m_Constants.BlurDirection = { 1.0f, 0.0f };
        else
            m_Constants.BlurDirection = { 0.0f, 1.0f };
    }
public:
    explicit CGradientBlurTransform(BOOL bHorizontal) noexcept
        : m_bHorizontal{ bHorizontal }
    {
        UpdateBlurDirection();
    }

    // -- ID2D1DrawTransform --

    IFACEMETHODIMP SetDrawInfo(_In_ ID2D1DrawInfo* pDrawInfo) override
    {
        HRESULT hr;
        m_pDrawInfo = pDrawInfo;

        constexpr D2D1_INPUT_DESCRIPTION Desc
        {
            D2D1_FILTER_MIN_MAG_MIP_LINEAR,
            0
        };
        hr = m_pDrawInfo->SetInputDescription(0, Desc);
        if (FAILED(hr))
            return hr;

        hr = m_pDrawInfo->SetPixelShader(GUID_GradientBlurPixelShader);
        if (FAILED(hr))
            return hr;

        return UploadConstantBuffer();
    }

    // -- ID2D1Transform --

    IFACEMETHODIMP_(UINT32) GetInputCount() const override { return 1u; }

    IFACEMETHODIMP MapInputRectsToOutputRect(
        _In_reads_(cInput) const D2D1_RECT_L* prcInput,
        _In_reads_(cInput) const D2D1_RECT_L* prcInputOpaque,
        UINT32 cInput,
        _Out_ D2D1_RECT_L* prcOutput,
        _Out_ D2D1_RECT_L* prcOutputOpaque) override
    {
        *prcOutput = *prcInput;
        *prcOutputOpaque = {};
        return S_OK;
    }

    IFACEMETHODIMP MapOutputRectToInputRects(
        _In_ const D2D1_RECT_L* prcOutput,
        _Out_writes_(cInput) D2D1_RECT_L* prcInput,
        UINT32 cInput) const override
    {
        const auto r = (LONG)ceil(m_Constants.fRadiusMax);
        prcInput[0].left = prcOutput->left - r;
        prcInput[0].top = prcOutput->top - r;
        prcInput[0].right = prcOutput->right + r;
        prcInput[0].bottom = prcOutput->bottom + r;
        return S_OK;
    }

    IFACEMETHODIMP MapInvalidRect(
        UINT32 idxInput,
        D2D1_RECT_L rcInvalidInput,
        _Out_ D2D1_RECT_L* prcInvalidOutput) const override
    {
        *prcInvalidOutput = rcInvalidInput;
        return S_OK;
    }

    void SetConstant(const Constant& c) noexcept
    {
        m_Constants = c;
        UpdateBlurDirection();
    }

    HRESULT UploadConstantBuffer() noexcept
    {
        return m_pDrawInfo->SetPixelShaderConstantBuffer(
            (PCBYTE)&m_Constants, sizeof(m_Constants));
    }
};

class CGradientBlurEffect final :
    public CUnknown<CGradientBlurEffect, ID2D1EffectImpl>
{
public:
    enum class Direction : UINT
    {
        Horizontal,
        Vertical,
    };

    enum
    {
        PROP_RADIUS_MIN,
        PROP_RADIUS_MAX,
        PROP_GRADIENT_DIRECTION,
        PROP_GRADIENT_RANGE,
    };
private:
    float m_fRadiusMin{ 0.0f };
    float m_fRadiusMax{ 40.0f };
    Direction m_eDirection{ Direction::Vertical };
    D2D1_VECTOR_2F m_GradientRange{ 0.0f, 500.0f };

    ComPtr<ID2D1EffectContext> m_pEffectContext{};

    ComPtr<CGradientBlurTransform> m_pTransformH{};
    ComPtr<CGradientBlurTransform> m_pTransformV{};

    static inline CByteBuffer s_PsByteCode{};

    static HRESULT __stdcall CreateEffect(_COM_Outptr_ IUnknown** ppEffectImpl) noexcept
    {
        *ppEffectImpl = new CGradientBlurEffect{};
        return S_OK;
    }

    static HRESULT CompileShader() noexcept
    {
        constexpr UINT uFlags =
#ifdef _DEBUG
            D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION
#else
            D3DCOMPILE_OPTIMIZATION_LEVEL3
#endif
            ;

        ComPtr<ID3DBlob> pByteCode, pError;
        const auto hr = D3DCompile(
            FxPsGradientBlur.data(),
            FxPsGradientBlur.size(),
            nullptr,
            nullptr,
            D3D_COMPILE_STANDARD_FILE_INCLUDE,
            "PSMain",
            "ps_5_0",
            uFlags,
            0u,
            &pByteCode,
            &pError);

        if (FAILED(hr))
        {
#ifdef _DEBUG
            EckDbgPrintFormat("D3DCompile failed (0x%08X)", hr);
            if (pError)
                EckDbgPrint((PCSTR)pError->GetBufferPointer());
            EckDbgBreak();
#endif
            return hr;
        }
        s_PsByteCode.ReSize(pByteCode->GetBufferSize());
        memcpy(s_PsByteCode.Data(), pByteCode->GetBufferPointer(), s_PsByteCode.Size());
        return S_OK;
    }

    void UpdateConstant() noexcept
    {
        float xDpi, yDpi;
        m_pEffectContext->GetDpi(&xDpi, &yDpi);

        CGradientBlurTransform::Constant c{};
        c.GradientRange =
        {
            m_GradientRange.x * xDpi / 96.f,
            m_GradientRange.y * yDpi / 96.f
        };
        c.uGradientAxis = (UINT)m_eDirection;

        const auto fDpi = m_eDirection == Direction::Horizontal ? xDpi : yDpi;
        c.fRadiusMin = m_fRadiusMin * fDpi / 96.f;
        c.fRadiusMax = m_fRadiusMax * fDpi / 96.f;

        m_pTransformH->SetConstant(c);
        m_pTransformV->SetConstant(c);
    }
public:
    IFACEMETHODIMP Initialize(
        _In_ ID2D1EffectContext* pEffectContext,
        _In_ ID2D1TransformGraph* pTransformGraph) override
    {
        HRESULT hr;

        m_pEffectContext = pEffectContext;

        hr = pEffectContext->LoadPixelShader(
            GUID_GradientBlurPixelShader,
            s_PsByteCode.Data(),
            (UINT)s_PsByteCode.Size());
        if (FAILED(hr))
            return hr;

        m_pTransformH.Attach(new CGradientBlurTransform{ TRUE });
        m_pTransformV.Attach(new CGradientBlurTransform{ FALSE });
        UpdateConstant();

        ComPtr<ID2D1BorderTransform> pBorderTransform;
        hr = pEffectContext->CreateBorderTransform(
            D2D1_EXTEND_MODE_CLAMP,
            D2D1_EXTEND_MODE_CLAMP,
            &pBorderTransform);

        if (SUCCEEDED(hr))
            hr = pTransformGraph->AddNode(pBorderTransform.Get());
        if (SUCCEEDED(hr))
            hr = pTransformGraph->AddNode(m_pTransformV.Get());
        if (SUCCEEDED(hr))
            hr = pTransformGraph->AddNode(m_pTransformH.Get());

        if (SUCCEEDED(hr))
            hr = pTransformGraph->ConnectToEffectInput(0, pBorderTransform.Get(), 0);
        if (SUCCEEDED(hr))
            hr = pTransformGraph->ConnectNode(pBorderTransform.Get(), m_pTransformV.Get(), 0);
        if (SUCCEEDED(hr))
            hr = pTransformGraph->ConnectNode(m_pTransformV.Get(), m_pTransformH.Get(), 0);
        if (SUCCEEDED(hr))
            hr = pTransformGraph->SetOutputNode(m_pTransformH.Get());
        return hr;
    }

    IFACEMETHODIMP PrepareForRender(D2D1_CHANGE_TYPE eChangeType) override
    {
        HRESULT hr;
        UpdateConstant();
        hr = m_pTransformH->UploadConstantBuffer();
        if (SUCCEEDED(hr))
            hr = m_pTransformV->UploadConstantBuffer();
        return hr;
    }

    IFACEMETHODIMP SetGraph(_In_ ID2D1TransformGraph* pGraph) override { return E_NOTIMPL; }

    HRESULT SetRadiusMinimum(float f) noexcept
    {
        m_fRadiusMin = f;
        return S_OK;
    }
    float GetRadiusMinimum() const noexcept { return m_fRadiusMin; }

    HRESULT SetRadiusMaximum(float f) noexcept
    {
        m_fRadiusMax = f;
        return S_OK;
    }
    float GetRadiusMaximum() const noexcept { return m_fRadiusMax; }

    HRESULT SetGradientDirection(UINT e) noexcept
    {
        m_eDirection = (Direction)e;
        return S_OK;
    }
    UINT GetGradientDirection() const noexcept { return (UINT)m_eDirection; }

    HRESULT SetGradientRange(D2D1_VECTOR_2F v) noexcept
    {
        m_GradientRange = v;
        return S_OK;
    }
    D2D1_VECTOR_2F GetGradientRange() const noexcept { return m_GradientRange; }

    static HRESULT Register() noexcept
    {
        const auto hr = CompileShader();
        if (FAILED(hr))
            return hr;

        constexpr PCWSTR Xml = LR"(<?xml version='1.0'?>
<Effect>
    <Property name='DisplayName' type='string' value='GradientBlur'/>
    <Property name='Author' type='string' value='Eck'/>
    <Property name='Category' type='string' value='None'/>
    <Property name='Description' type='string' value=''/>
    <Inputs>
        <Input name='Source'/>
    </Inputs>
    <Property name='RadiusMinimum' type='float'>
        <Property name='DisplayName' type='string' value='RadiusMinimum'/>
    </Property>
    <Property name='RadiusMaximum' type='float'>
        <Property name='DisplayName' type='string' value='RadiusMaximum'/>
    </Property>
    <Property name='GradientDirection' type='uint32'>
        <Property name='DisplayName' type='string' value='GradientDirection'/>
    </Property>
    <Property name='GradientRange' type='vector2'>
        <Property name='DisplayName' type='string' value='GradientRange'/>
    </Property>
</Effect>
)";

#undef ECK_TEMP
#define ECK_TEMP(Name, Setter, Getter) \
    D2D1_VALUE_TYPE_BINDING(L#Name, &CGradientBlurEffect::Set##Name, &CGradientBlurEffect::Get##Name)
        const D2D1_PROPERTY_BINDING Binding[]
        {
            ECK_TEMP(RadiusMinimum),
            ECK_TEMP(RadiusMaximum),
            ECK_TEMP(GradientDirection),
            ECK_TEMP(GradientRange),
        };
#undef ECK_TEMP

        return g_pD2DFactory->RegisterEffectFromString(
            CLSID_GradientBlurEffect,
            Xml, EckArgArray(Binding), CreateEffect);
    }
};
ECK_NAMESPACE_END