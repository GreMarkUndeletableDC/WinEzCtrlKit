#pragma once
#include "CDialog.h"

ECK_NAMESPACE_BEGIN
class CColorDialog : public CDialog
{
public:
    ECK_RTTI(CColorDialog, CDialog);

    const inline static UINT MessageSetColor = RegisterWindowMessageW(SETRGBSTRINGW);
protected:
    COLORREF m_crCust[16]{};
public:
    // CHOOSECOLORW*
    INT_PTR CreateModalDialog(
        HWND hParent,
        _In_reads_bytes_(sizeof(CHOOSECOLORW)) void* pData = nullptr) noexcept override
    {
        auto pcc = (CHOOSECOLORW*)pData;
        if (!pcc->lpCustColors)
            pcc->lpCustColors = m_crCust;
        BeginCbtHook(this);
        return ChooseColorW((CHOOSECOLORW*)pData);
    }

    INT_PTR CreateModalDialog(
        HWND hParent,
        COLORREF crInit = 0,
        UINT uFlags = 0,
        _Inout_opt_count_(16) COLORREF* pcrCust = nullptr)
    {
        CHOOSECOLORW cc{ sizeof(cc) };
        cc.hwndOwner = hParent;
        cc.rgbResult = crInit;
        cc.Flags = uFlags;
        cc.lpCustColors = (pcrCust ? pcrCust : m_crCust);
        return CreateModalDialog(hParent, &cc);
    }

    void OnOk(HWND hCtrl) noexcept override {}
    void OnCancel(HWND hCtrl) noexcept override {}

    EckInline void SetColor(COLORREF cr) const { SendMessageW(MessageSetColor, 0, cr); }
};
ECK_NAMESPACE_END