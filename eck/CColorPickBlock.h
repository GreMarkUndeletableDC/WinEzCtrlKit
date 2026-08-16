#pragma once
#include "CStatic.h"

ECK_NAMESPACE_BEGIN
class CColorPickBlock : public CStatic
{
public:
    ECK_RTTI(CColorPickBlock, CStatic);
private:
    COLORREF m_crCust[16]{};
    COLORREF m_cr{ CLR_INVALID };
    UINT m_uCcFlags{ CC_FULLOPEN };

    void InternalDetachNew() noexcept
    {
        ZeroMemory(m_crCust, sizeof(m_crCust));
        m_cr = CLR_INVALID;
        m_uCcFlags = CC_FULLOPEN;
    }
public:
    void AttachNew(HWND hWnd) noexcept override
    {
        __super::AttachNew(hWnd);
        SetText(nullptr);
        Redraw();
    }

    void DetachNew() noexcept override
    {
        __super::DetachNew();
        InternalDetachNew();
    }

    LRESULT OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept override
    {
        switch (uMsg)
        {
        case WM_LBUTTONDBLCLK:
        {
            CHOOSECOLORW cc{ sizeof(CHOOSECOLORW) };
            cc.hwndOwner = Handle;
            cc.Flags = m_uCcFlags;
            cc.lpCustColors = m_crCust;
            if (ChooseColorW(&cc))
            {
                m_cr = cc.rgbResult;
                Redraw();
            }
        }
        return 0;

        case WM_SETTEXT:
            return TRUE;

        case WM_CREATE:
            ((CREATESTRUCTW*)lParam)->lpszName = nullptr;
            break;
        case WM_DESTROY:
            InternalDetachNew();
            break;
        }
        return CStatic::OnMessage(uMsg, wParam, lParam);
    }

    LRESULT OnNotifyMessage(HWND hParent, UINT uMsg,
        WPARAM wParam, LPARAM lParam, BOOL& bProcessed) noexcept override
    {
        switch (uMsg)
        {
        case WM_CTLCOLORSTATIC:
        {
            bProcessed = TRUE;
            SetDCBrushColor((HDC)wParam, m_cr);
            return (LRESULT)GetStockBrush(DC_BRUSH);
        }
        break;
        }
        return __super::OnNotifyMessage(hParent, uMsg, wParam, lParam, bProcessed);
    }

    EckInlineCe void SetColor(COLORREF cr) noexcept { m_cr = cr; }
    EckInlineNdCe COLORREF GetColor() const noexcept { return m_cr; }

    EckInlineNdCe auto GetCustomColor() const noexcept { return m_crCust; }
    EckInlineNdCe auto GetCustomColor() noexcept { return m_crCust; }

    EckInlineNdCe UINT GetChooseColorFlags() const noexcept { return m_uCcFlags; }
    EckInlineCe void SetChooseColorFlags(UINT u) noexcept { m_uCcFlags = u; }
};
ECK_NAMESPACE_END