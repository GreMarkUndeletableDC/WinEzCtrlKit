#pragma once
#include "DuiBase.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
class CFilter
{
public:
    struct BaseExtra
    {
        UINT cbExtra;
        const D2D1_LAYER_PARAMETERS1* pLayerParam;
        ID2D1Layer* pLayer;
    };
private:
    CDuiWindow* m_pWindow{};
protected:
    // 替换当前DC的指定范围内容为指定效果的输出
    void CopyEffect(
        _In_ ID2D1Effect* pEffect,
        D2D1_POINT_2F ptDrawing,
        D2D1_INTERPOLATION_MODE eInterpolation,
        _In_opt_ const BaseExtra* pExtra) const noexcept
    {
        const auto pDC = GetWindow()->RdGetDC();
        if (pExtra && pExtra->pLayerParam)
        {
            const auto eBlend = pDC->GetPrimitiveBlend();
            pDC->SetPrimitiveBlend(D2D1_PRIMITIVE_BLEND_COPY);
            pDC->PushLayer(pExtra->pLayerParam, pExtra->pLayer);
            pDC->DrawImage(pEffect, ptDrawing, eInterpolation);
            pDC->PopLayer();
            pDC->SetPrimitiveBlend(eBlend);
        }
        else
        {
            pDC->DrawImage(pEffect, ptDrawing, eInterpolation,
                D2D1_COMPOSITE_MODE_SOURCE_COPY);
        }
    }

    void ReserveWindowCacheBitmap(const D2D1_RECT_F& rc) const noexcept
    {
        GetWindow()->CcReserveBitmapLogical(
            rc.right - rc.left, rc.bottom - rc.top);
    }

    template<class TExtra>
        requires std::is_base_of_v<BaseExtra, TExtra>
    static void CheckExtraSize(const TExtra* pExtra) noexcept
    {
        if (pExtra)
        {
            if (pExtra->cbExtra != sizeof(TExtra))
                EckBugCheck(BccGeneric, nullptr, pExtra->cbExtra, sizeof(TExtra));
        }
    }
public:
    // 依附窗口，将清除所有设备相关资源
    virtual HRESULT Attach(_In_opt_ CDuiWindow* pWnd) noexcept
    {
        m_pWindow = pWnd;
        return S_OK;
    }

    // 应用滤镜到当前设备上下文内容的指定范围。
    // 调用方负责初始化缓存（如位图和效果），还负责刷新DC上任何挂起的操作
    // WARNING 使用CopyEffect且不使用图层的滤镜时，调用方负责设置合适的轴对齐剪辑
    virtual HRESULT FilterDC(
        const D2D1_RECT_F& rc,
        float ox, float oy,
        _In_opt_ const BaseExtra* pExtra = nullptr) noexcept
    {
        return E_NOTIMPL;
    }

    EckInlineNdCe CDuiWindow* GetWindow() const noexcept { return m_pWindow; }
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END