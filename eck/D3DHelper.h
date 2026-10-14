#pragma once
#include "ComPtr.h"

#include <d3dcompiler.h>

#define ECK_DX3D_NAMESPACE_BEGIN    namespace Dx3D {
#define ECK_DX3D_NAMESPACE_END      }

ECK_NAMESPACE_BEGIN
ECK_DX3D_NAMESPACE_BEGIN
inline HRESULT CompileShader(
    _Out_ ID3DBlob*& pShaderBlob,
    _In_reads_(cchShader) PCCH pszShader,
    int cchShader,
    PCSTR pszEntryPoint,
    PCSTR pszTarget,
    UINT uFlags1 = 0u,
    UINT uFlags2 = 0u) noexcept
{
    ComPtr<ID3DBlob> pErrorBlob;
    const auto hr = D3DCompile(pszShader, cchShader, nullptr, nullptr, nullptr,
        pszEntryPoint, pszTarget, uFlags1, uFlags2, &pShaderBlob, &pErrorBlob);
    if (FAILED(hr))
    {
#ifdef _DEBUG
        EckDbgPrint(L"D3D11: Compile shader failed!");
        if (pErrorBlob)
            OutputDebugStringA((PCSTR)pErrorBlob->GetBufferPointer());
        EckDbgBreak();
#endif
    }
    return hr;
}


struct CBuffer
{
    ComPtr<ID3D11Buffer> p;

    HRESULT Create(
        _In_opt_ ID3D11Device* pDevice,
        size_t cb,
        UINT eBind,
        D3D11_USAGE eUsage,
        UINT uCpuAccessFlags = 0u,
        UINT uMiscFlags = 0u,
        PCVOID pData = nullptr,
        size_t cbStruct = 0u) noexcept
    {
        D3D11_BUFFER_DESC Desc;
        Desc.ByteWidth = (UINT)cb;
        Desc.Usage = eUsage;
        Desc.BindFlags = eBind;
        Desc.CPUAccessFlags = uCpuAccessFlags;
        Desc.MiscFlags = uMiscFlags;
        Desc.StructureByteStride = (UINT)cbStruct;
        D3D11_SUBRESOURCE_DATA InitData;
        if (pData)
        {
            InitData.pSysMem = pData;
            InitData.SysMemPitch = 0u;
            InitData.SysMemSlicePitch = 0u;
        }
        if (!pDevice)
            pDevice = g_pD3D11Device;
        return pDevice->CreateBuffer(
            &Desc,
            pData ? &InitData : nullptr,
            p.AtClear());
    }
    HRESULT CreateConstantBuffer(
        _In_opt_ ID3D11Device* pDevice,
        size_t cb,
        _In_reads_bytes_opt_(cb) PCVOID pData = nullptr) noexcept
    {
        return Create(
            pDevice,
            cb,
            D3D11_BIND_CONSTANT_BUFFER,
            D3D11_USAGE_DYNAMIC,
            D3D11_CPU_ACCESS_WRITE,
            0u,
            pData);
    }
    HRESULT CreateVertexBuffer(
        _In_opt_ ID3D11Device* pDevice,
        BOOL bImmutable,
        size_t cb,
        _In_reads_bytes_opt_(cb) PCVOID pData = nullptr) noexcept
    {
        return Create(
            pDevice,
            cb,
            D3D11_BIND_VERTEX_BUFFER,
            bImmutable ? D3D11_USAGE_IMMUTABLE : D3D11_USAGE_DYNAMIC,
            bImmutable ? 0 : D3D11_CPU_ACCESS_WRITE,
            0u,
            pData);
    }
    HRESULT CreateIndexBuffer(
        _In_opt_ ID3D11Device* pDevice,
        BOOL bImmutable,
        size_t cb,
        _In_reads_bytes_opt_(cb) PCVOID pData = nullptr) noexcept
    {
        return Create(
            pDevice,
            cb,
            D3D11_BIND_INDEX_BUFFER,
            bImmutable ? D3D11_USAGE_IMMUTABLE : D3D11_USAGE_DYNAMIC,
            bImmutable ? 0 : D3D11_CPU_ACCESS_WRITE,
            0u,
            pData);
    }

    HRESULT UpdateDynamicBuffer(
        _In_ ID3D11DeviceContext* pContext,
        _In_reads_bytes_(cbData) PCVOID pData,
        size_t cbData) noexcept
    {
        D3D11_MAPPED_SUBRESOURCE Mapped;
        const auto hr = pContext->Map(
            p.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &Mapped);
        if (FAILED(hr))
            return hr;
        memcpy(Mapped.pData, pData, cbData);
        pContext->Unmap(p.Get(), 0);
        return S_OK;
    }

    EckInlineNdCe auto operator->() const noexcept { return p.Get(); }
    EckInlineNdCe auto operator&() noexcept { return &p; }
    EckInlineNdCe auto operator&() const noexcept { return &p; }
    EckInlineNdCe auto Get() const noexcept { return p.Get(); }
    EckInlineNdCe auto At() const noexcept { return p.At(); }
    EckInline void Clear() noexcept { p.Clear(); }
};

namespace Detail
{
    template<class TClass, class TInterface>
    struct CShader
    {
        ComPtr<TInterface> p;

        EckInlineNdCe auto operator->() const noexcept { return p.Get(); }
        EckInlineNdCe auto operator&() noexcept { return &p; }
        EckInlineNdCe auto operator&() const noexcept { return &p; }
        EckInlineNdCe auto Get() const noexcept { return p.Get(); }
        EckInlineNdCe auto At() const noexcept { return p.At(); }
        EckInline void Clear() noexcept { p.Clear(); }

        EckInline HRESULT Create(
            _In_opt_ ID3D11Device* pDevice,
            ID3DBlob* pBlob) noexcept
        {
            return static_cast<TClass*>(this)->Create(
                pDevice,
                pBlob->GetBufferPointer(),
                pBlob->GetBufferSize());
        }
    };
}
struct CPixelShader : Detail::CShader<CPixelShader, ID3D11PixelShader>
{
    EckInline HRESULT Create(
        _In_opt_ ID3D11Device* pDevice,
        _In_reads_bytes_(cbBytecode) PCVOID pBytecode,
        size_t cbBytecode) noexcept
    {
        if (!pDevice)
            pDevice = g_pD3D11Device;
        return pDevice->CreatePixelShader(
            pBytecode, cbBytecode, nullptr, p.AtClear());
    }
};
struct CVertexShader : Detail::CShader<CVertexShader, ID3D11VertexShader>
{
    EckInline HRESULT Create(
        _In_opt_ ID3D11Device* pDevice,
        _In_reads_bytes_(cbBytecode) PCVOID pBytecode,
        size_t cbBytecode) noexcept
    {
        if (!pDevice)
            pDevice = g_pD3D11Device;
        return pDevice->CreateVertexShader(
            pBytecode, cbBytecode, nullptr, p.AtClear());
    }
};
struct CVertexShaderAndInputLayout
{
    CVertexShader Shader;
    ComPtr<ID3D11InputLayout> InputLayout;

    HRESULT Create(
        _In_opt_ ID3D11Device* pDevice,
        _In_reads_bytes_(cbBytecode) PCVOID pBytecode,
        size_t cbBytecode,
        _In_reads_(cInput) const D3D11_INPUT_ELEMENT_DESC* pInput,
        size_t cInput) noexcept
    {
        HRESULT hr;
        if (!pDevice)
            pDevice = g_pD3D11Device;
        hr = Shader.Create(pDevice, pBytecode, cbBytecode);
        if (FAILED(hr))
            return hr;
        hr = pDevice->CreateInputLayout(
            pInput, (UINT)cInput,
            pBytecode, cbBytecode,
            InputLayout.AtClear());
        return hr;
    }

    EckInlineNdCe auto GetInputLayout() const noexcept { return InputLayout.Get(); }
    EckInlineNdCe auto GetShader() const noexcept { return Shader.Get(); }
    EckInline void Clear() noexcept
    {
        Shader.Clear();
        InputLayout.Clear();
    }
};

struct CTexture
{
    ComPtr<ID3D11Texture2D> p;

    HRESULT Create(
        _In_opt_ ID3D11Device* pDevice,
        UINT cx, UINT cy,
        DXGI_FORMAT eFormat = DXGI_FORMAT_B8G8R8A8_UNORM,
        UINT eBind = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET,
        D3D11_USAGE eUsage = D3D11_USAGE_DEFAULT,
        const D3D11_SUBRESOURCE_DATA* pInitData = nullptr,
        UINT uCPUAccessFlags = 0u,
        UINT uMiscFlags = 0u,
        UINT uMipLevels = 1u) noexcept
    {
        D3D11_TEXTURE2D_DESC Desc;
        Desc.Width = cx;
        Desc.Height = cy;
        Desc.MipLevels = uMipLevels;
        Desc.ArraySize = 1u;
        Desc.Format = eFormat;
        Desc.SampleDesc.Count = 1u;
        Desc.SampleDesc.Quality = 0u;
        Desc.Usage = eUsage;
        Desc.BindFlags = eBind;
        Desc.CPUAccessFlags = uCPUAccessFlags;
        Desc.MiscFlags = uMiscFlags;
        if (!pDevice)
            pDevice = g_pD3D11Device;
        return pDevice->CreateTexture2D(&Desc, pInitData, p.AtClear());
    }

    EckInlineNdCe auto operator->() const noexcept { return p.Get(); }
    EckInlineNdCe auto operator&() noexcept { return &p; }
    EckInlineNdCe auto operator&() const noexcept { return &p; }
    EckInlineNdCe auto Get() const noexcept { return p.Get(); }
    EckInlineNdCe auto At() const noexcept { return p.At(); }
    EckInline void Clear() noexcept { p.Clear(); }
};

struct CSampler
{
    ComPtr<ID3D11SamplerState> p;

    HRESULT Create(
        _In_opt_ ID3D11Device* pDevice,
        D3D11_FILTER eFilter = D3D11_FILTER_MIN_MAG_MIP_LINEAR,
        D3D11_TEXTURE_ADDRESS_MODE eAddressU = D3D11_TEXTURE_ADDRESS_CLAMP,
        D3D11_TEXTURE_ADDRESS_MODE eAddressV = D3D11_TEXTURE_ADDRESS_CLAMP,
        D3D11_TEXTURE_ADDRESS_MODE eAddressW = D3D11_TEXTURE_ADDRESS_CLAMP,
        D3D11_COMPARISON_FUNC eComparisonFunc = D3D11_COMPARISON_NEVER,
        _In_reads_opt_(4) const float* pBorderColor = nullptr) noexcept
    {
        D3D11_SAMPLER_DESC Desc{};
        Desc.Filter = eFilter;
        Desc.AddressU = eAddressU;
        Desc.AddressV = eAddressV;
        Desc.AddressW = eAddressW;
        Desc.ComparisonFunc = eComparisonFunc;
        if (pBorderColor)
            memcpy(Desc.BorderColor, pBorderColor, 4 * sizeof(float));
        if (!pDevice)
            pDevice = g_pD3D11Device;
        return pDevice->CreateSamplerState(&Desc, p.AtClear());
    }
    HRESULT CreateBorder(
        _In_opt_ ID3D11Device* pDevice,
        _In_reads_opt_(4) const float* pBorderColor,
        D3D11_FILTER eFilter = D3D11_FILTER_MIN_MAG_MIP_LINEAR) noexcept
    {
        return Create(
            pDevice,
            eFilter,
            D3D11_TEXTURE_ADDRESS_BORDER,
            D3D11_TEXTURE_ADDRESS_BORDER,
            D3D11_TEXTURE_ADDRESS_BORDER,
            D3D11_COMPARISON_NEVER,
            pBorderColor);
    }

    EckInlineNdCe auto operator->() const noexcept { return p.Get(); }
    EckInlineNdCe auto operator&() noexcept { return &p; }
    EckInlineNdCe auto operator&() const noexcept { return &p; }
    EckInlineNdCe auto Get() const noexcept { return p.Get(); }
    EckInlineNdCe auto At() const noexcept { return p.At(); }
    EckInline void Clear() noexcept { p.Clear(); }
};

struct CShaderResourceView
{
    ComPtr<ID3D11ShaderResourceView> p;

    HRESULT Create(
        _In_opt_ ID3D11Device* pDevice,
        _In_ ID3D11Resource* pResource,
        const D3D11_SHADER_RESOURCE_VIEW_DESC& Desc) noexcept
    {
        if (!pDevice)
            pDevice = g_pD3D11Device;
        return pDevice->CreateShaderResourceView(
            pResource, &Desc, p.AtClear());
    }
    HRESULT Create(
        _In_opt_ ID3D11Device* pDevice,
        _In_ ID3D11Resource* pResource,
        DXGI_FORMAT eFormat = DXGI_FORMAT_UNKNOWN,
        D3D11_SRV_DIMENSION eDimension = D3D11_SRV_DIMENSION_TEXTURE2D,
        UINT uMipLevels = 1u) noexcept
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC Desc{};
        Desc.Format = eFormat;
        Desc.ViewDimension = eDimension;
        Desc.Texture2D.MipLevels = uMipLevels;
        return Create(pDevice, pResource, Desc);
    }

    EckInlineNdCe auto operator->() const noexcept { return p.Get(); }
    EckInlineNdCe auto operator&() noexcept { return &p; }
    EckInlineNdCe auto operator&() const noexcept { return &p; }
    EckInlineNdCe auto Get() const noexcept { return p.Get(); }
    EckInlineNdCe auto At() const noexcept { return p.At(); }
    EckInline void Clear() noexcept { p.Clear(); }
};

struct CRenderTargetView
{
    ComPtr<ID3D11RenderTargetView> p;

    HRESULT Create(
        _In_opt_ ID3D11Device* pDevice,
        _In_ ID3D11Resource* pResource) noexcept
    {
        if (!pDevice)
            pDevice = g_pD3D11Device;
        return pDevice->CreateRenderTargetView(pResource, nullptr, p.AtClear());
    }

    EckInlineNdCe auto operator->() const noexcept { return p.Get(); }
    EckInlineNdCe auto operator&() noexcept { return &p; }
    EckInlineNdCe auto operator&() const noexcept { return &p; }
    EckInlineNdCe auto Get() const noexcept { return p.Get(); }
    EckInlineNdCe auto At() const noexcept { return p.At(); }
    EckInline void Clear() noexcept { p.Clear(); }
};

struct CDepthStencilView
{
    ComPtr<ID3D11DepthStencilView> p;

    HRESULT Create(
        _In_opt_ ID3D11Device* pDevice,
        _In_ ID3D11Resource* pResource,
        const D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc = nullptr) noexcept
    {
        if (!pDevice)
            pDevice = g_pD3D11Device;
        return pDevice->CreateDepthStencilView(pResource, pDesc, p.AtClear());
    }

    EckInlineNdCe auto operator->() const noexcept { return p.Get(); }
    EckInlineNdCe auto operator&() noexcept { return &p; }
    EckInlineNdCe auto operator&() const noexcept { return &p; }
    EckInlineNdCe auto Get() const noexcept { return p.Get(); }
    EckInlineNdCe auto At() const noexcept { return p.At(); }
    EckInline void Clear() noexcept { p.Clear(); }
};

struct CRwTexture
{
    CTexture Texture{};
    CRenderTargetView Rtv{};
    CShaderResourceView Srv{};

    HRESULT Create(
        _In_opt_ ID3D11Device* pDevice,
        UINT cx, UINT cy,
        DXGI_FORMAT eFormat = DXGI_FORMAT_B8G8R8A8_UNORM,
        PCVOID pData = nullptr,
        UINT cbStride = 0u) noexcept
    {
        HRESULT hr;
        constexpr static UINT Bind =
            D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        if (pData)
        {
            const D3D11_SUBRESOURCE_DATA Data{ pData,cbStride };
            hr = Texture.Create(
                pDevice,
                cx, cy,
                eFormat,
                Bind,
                D3D11_USAGE_DEFAULT,
                &Data);
        }
        else
            hr = Texture.Create(pDevice, cx, cy, eFormat, Bind);
        if (FAILED(hr))
            return hr;
        hr = Srv.Create(pDevice, Texture.Get());
        if (FAILED(hr))
            return hr;
        return Rtv.Create(pDevice, Texture.Get());
    }
};
ECK_DX3D_NAMESPACE_END
ECK_NAMESPACE_END