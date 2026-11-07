#pragma once
#include "ComPtr.h"

ECK_NAMESPACE_BEGIN
#if !ECK_OPT_NO_D2D
struct EZD2D_PARAM
{
    static constexpr D2D1_DEVICE_CONTEXT_OPTIONS DefaultDcOptions =
        D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS;
    static constexpr D2D1_ALPHA_MODE DefaultBitmapAlphaMode = D2D1_ALPHA_MODE_IGNORE;
    static constexpr D2D1_BITMAP_OPTIONS DefaultBitmapOptions =
        D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;

    HWND hWnd{};

    IDXGIFactory2* pDxgiFactory{};
    IDXGIDevice* pDxgiDevice{};
    ID2D1Device* pD2DDevice{};

    UINT cx{ 8 };
    UINT cy{ 8 };
    UINT cBuffer{ 1 };
    DXGI_SCALING uScaling{ DXGI_SCALING_NONE };
    DXGI_SWAP_EFFECT uSwapEffect{ DXGI_SWAP_EFFECT_DISCARD };
    DXGI_ALPHA_MODE uAlphaMode{ DXGI_ALPHA_MODE_IGNORE };
    UINT uFlags{};

    D2D1_DEVICE_CONTEXT_OPTIONS uDcOptions{ DefaultDcOptions };

    D2D1_ALPHA_MODE eBitmapAlphaMode{ DefaultBitmapAlphaMode };
    D2D1_BITMAP_OPTIONS uBitmapOptions{ DefaultBitmapOptions };

    float fDpi{ 96.f };

    static EZD2D_PARAM MakeBitBlit(
        HWND hWnd,
        int cx,
        int cy,
        float fDpi,
        IDXGIFactory2* pDxgiFactory = nullptr,
        IDXGIDevice* pDxgiDevice = nullptr,
        ID2D1Device* pD2DDevice = nullptr) noexcept
    {
        return
        {
            .hWnd = hWnd,
            .pDxgiFactory = pDxgiFactory ? pDxgiFactory : g_pDxgiFactory,
            .pDxgiDevice = pDxgiDevice ? pDxgiDevice : g_pDxgiDevice,
            .pD2DDevice = pD2DDevice ? pD2DDevice : g_pD2DDevice,
            .cx = (UINT)cx,
            .cy = (UINT)cy,
            .uScaling = DXGI_SCALING_STRETCH,
            .uSwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL,
            .fDpi = fDpi
        };
    }

    static EZD2D_PARAM MakeFlip(
        HWND hWnd,
        int cx,
        int cy,
        float fDpi,
        IDXGIFactory2* pDxgiFactory = nullptr,
        IDXGIDevice* pDxgiDevice = nullptr,
        ID2D1Device* pD2DDevice = nullptr) noexcept
    {
        return
        {
            .hWnd = hWnd,
            .pDxgiFactory = pDxgiFactory ? pDxgiFactory : g_pDxgiFactory,
            .pDxgiDevice = pDxgiDevice ? pDxgiDevice : g_pDxgiDevice,
            .pD2DDevice = pD2DDevice ? pD2DDevice : g_pD2DDevice,
            .cx = (UINT)cx,
            .cy = (UINT)cy,
            .cBuffer = 2,
            .uSwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL,
            .fDpi = fDpi
        };
    }
};

class CEasyD2D
{
private:
    ComPtr<ID2D1DeviceContext> m_pDC{};
    ComPtr<IDXGISwapChain1> m_pSwapChain{};
    ComPtr<ID2D1Bitmap1> m_pBitmap{};

    static HRESULT InternalCreateD2D(
        const EZD2D_PARAM& Param,
        IDXGISwapChain1* pSwapChain,
        ComPtr<ID2D1DeviceContext>& pDC,
        ComPtr<ID2D1Bitmap1>& pBitmap) noexcept
    {
        HRESULT hr;

        ComPtr<IDXGISurface1> pSurface;
        hr = pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pSurface));
        if (FAILED(hr))
            return hr;

        hr = Param.pD2DDevice->CreateDeviceContext(Param.uDcOptions, &pDC);
        if (FAILED(hr))
            return hr;

        const D2D1_BITMAP_PROPERTIES1 BitmapProp
        {
            { DXGI_FORMAT_B8G8R8A8_UNORM, Param.eBitmapAlphaMode },
            Param.fDpi,
            Param.fDpi,
            Param.uBitmapOptions
        };

        hr = pDC->CreateBitmapFromDxgiSurface(pSurface.Get(), &BitmapProp, &pBitmap);
        if (FAILED(hr))
            return hr;
        pDC->SetTarget(pBitmap.Get());
        return S_OK;
    }
public:
    HRESULT Create(const EZD2D_PARAM& Param) noexcept
    {
        HRESULT hr;
        Clear();

        const DXGI_SWAP_CHAIN_DESC1 SwapChainDesc
        {
            Param.cx,
            Param.cy,
            DXGI_FORMAT_B8G8R8A8_UNORM,
            false,
            { 1, 0 },
            DXGI_USAGE_RENDER_TARGET_OUTPUT,
            Param.cBuffer,
            Param.uScaling,
            Param.uSwapEffect,
            Param.uAlphaMode,
            Param.uFlags
        };

        ComPtr<IDXGISwapChain1> pSwapChain{};
        hr = Param.pDxgiFactory->CreateSwapChainForHwnd(
            Param.pDxgiDevice,
            Param.hWnd,
            &SwapChainDesc,
            nullptr,
            nullptr,
            &pSwapChain);
        if (FAILED(hr))
            return hr;

        ComPtr<ID2D1DeviceContext> pDC{};
        ComPtr<ID2D1Bitmap1> pBitmap{};
        hr = InternalCreateD2D(Param, pSwapChain.Get(), pDC, pBitmap);
        if (FAILED(hr))
            return hr;

        m_pDC = std::move(pDC);
        m_pSwapChain = std::move(pSwapChain);
        m_pBitmap = std::move(pBitmap);
        return S_OK;
    }

    HRESULT CreateComposition(const EZD2D_PARAM& Param) noexcept
    {
        HRESULT hr;
        Clear();

        const DXGI_SWAP_CHAIN_DESC1 SwapChainDesc
        {
            Param.cx,
            Param.cy,
            DXGI_FORMAT_B8G8R8A8_UNORM,
            false,
            { 1, 0 },
            DXGI_USAGE_RENDER_TARGET_OUTPUT,
            Param.cBuffer,
            Param.uScaling,
            Param.uSwapEffect,
            Param.uAlphaMode,
            Param.uFlags
        };

        ComPtr<IDXGISwapChain1> pSwapChain{};
        hr = Param.pDxgiFactory->CreateSwapChainForComposition(
            Param.pDxgiDevice,
            &SwapChainDesc,
            nullptr,
            &pSwapChain);
        if (FAILED(hr))
            return hr;

        ComPtr<ID2D1DeviceContext> pDC{};
        ComPtr<ID2D1Bitmap1> pBitmap{};
        hr = InternalCreateD2D(Param, pSwapChain.Get(), pDC, pBitmap);
        if (FAILED(hr))
            return hr;

        m_pDC = std::move(pDC);
        m_pSwapChain = std::move(pSwapChain);
        m_pBitmap = std::move(pBitmap);
        return S_OK;
    }

    HRESULT CreateDeviceContext(
        ID2D1Device* pD2DDevice = nullptr,
        D2D1_DEVICE_CONTEXT_OPTIONS uDcOptions = EZD2D_PARAM::DefaultDcOptions) noexcept
    {
        if (!pD2DDevice)
            pD2DDevice = g_pD2DDevice;
        return pD2DDevice->CreateDeviceContext(uDcOptions, m_pDC.AtClear());
    }

    HRESULT ReSizeSwapChain(
        UINT cBuffer,
        UINT cx, UINT cy,
        float fDpi,
        UINT uSwapChainFlags,
        D2D1_ALPHA_MODE eBitmapAlphaMode = EZD2D_PARAM::DefaultBitmapAlphaMode,
        D2D1_BITMAP_OPTIONS uBitmapOptions = EZD2D_PARAM::DefaultBitmapOptions) noexcept
    {
        EckAssert(m_pDC && m_pSwapChain && m_pBitmap);
        m_pDC->SetTarget(nullptr);
        m_pBitmap.Clear();

        HRESULT hr;
        hr = m_pSwapChain->ResizeBuffers(
            cBuffer,
            std::max(8u, cx),
            std::max(8u, cy),
            DXGI_FORMAT_UNKNOWN,
            uSwapChainFlags);
        if (FAILED(hr))
            return hr;

        ComPtr<IDXGISurface1> pSurface;
        hr = m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pSurface));
        if (FAILED(hr))
            return hr;

        D2D1_BITMAP_PROPERTIES1 BitmapProp
        {
            { DXGI_FORMAT_B8G8R8A8_UNORM, eBitmapAlphaMode },
            fDpi,
            fDpi,
            uBitmapOptions,
        };

        hr = m_pDC->CreateBitmapFromDxgiSurface(pSurface.Get(), &BitmapProp, &m_pBitmap);
        if (FAILED(hr))
            return hr;

        m_pDC->SetTarget(m_pBitmap.Get());
        return S_OK;
    }

    EckInlineNdCe auto& GetDC() const noexcept { return m_pDC; }
    EckInlineNdCe auto& GetSwapChain() const noexcept { return m_pSwapChain; }
    EckInlineNdCe auto& GetBitmap() const noexcept { return m_pBitmap; }
    EckInlineNdCe auto& GetDC() noexcept { return m_pDC; }
    EckInlineNdCe auto& GetSwapChain() noexcept { return m_pSwapChain; }
    EckInlineNdCe auto& GetBitmap() noexcept { return m_pBitmap; }

    void Clear() noexcept
    {
        m_pDC.Clear();
        m_pSwapChain.Clear();
        m_pBitmap.Clear();
    }
};
#endif// !ECK_OPT_NO_D2D
ECK_NAMESPACE_END