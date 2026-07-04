#pragma once
#include "CDialog.h"

ECK_NAMESPACE_BEGIN
class CTaskDialog : public CDialog
{
public:
    ECK_RTTI(CTaskDialog, CDialog);
    ECK_CWND_SINGLEOWNER(CTaskDialog);
public:
    struct DLGCTX
    {
        TASKDIALOGCONFIG* ptdc;
        int iRadioButton;
        BOOL bChecked;
        HRESULT hr;
    };
protected:
    DLGCTX* m_pParam{};
    PFTASKDIALOGCALLBACK m_pfnRealCallback{};
    LONG_PTR m_lRealRefData{};
    CEventChain<Intercept_T, HRESULT, HWND, UINT, WPARAM, LPARAM > m_CallbackSig{};

    static LRESULT CALLBACK LinkParentSubclassProcedure(
        HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
        UINT_PTR uSubclassId, DWORD_PTR lRefData) noexcept
    {
        if (ShouldAppsUseDarkMode())
            switch (uMsg)
            {
            case WM_PAINT:
            {
                PAINTSTRUCT ps;
                BeginPaint(hWnd, &ps);
                FillRect(ps.hdc, &ps.rcPaint, GetStockBrush(BLACK_BRUSH));
                EndPaint(hWnd, &ps);
            }
            return 0;
            case WM_CTLCOLORSTATIC:
            {
                const auto* const ptc = PtcCurrent();
                SetTextColor((HDC)wParam, ptc->crDefText);
                SetBkColor((HDC)wParam, ptc->crDefBkg);
                SetDCBrushColor((HDC)wParam, ptc->crDefBkg);
            }
            return (LRESULT)GetStockBrush(DC_BRUSH);// 防止展开时出现闪烁的白色部分
            }
        return DefSubclassProc(hWnd, uMsg, wParam, lParam);
    }

    static HRESULT CALLBACK EckTaskDialogCallback(
        HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LONG_PTR lRefData) noexcept
    {
        auto p = (CTaskDialog*)lRefData;
        switch (uMsg)
        {
        case TDN_NAVIGATED:
        case TDN_DIALOG_CONSTRUCTED:
        {
            EnumChildWindows(hWnd, [](HWND hWnd, LPARAM lParam)->BOOL
                {
                    WCHAR szCls[ARRAYSIZE(WC_LINK) + 2];
                    GetClassNameW(hWnd, szCls, ARRAYSIZE(szCls));
                    if (_wcsicmp(szCls, WC_LINK) == 0)
                        SetWindowSubclass(GetParent(hWnd), LinkParentSubclassProcedure, 0, 0);
                    return TRUE;
                }, 0);
        }
        break;
        }

        return p->OnTaskDialogNotify(hWnd, uMsg, wParam, lParam);
    }
public:
    INT_PTR CreateModalDialog(HWND hParent,
        _In_reads_bytes_(sizeof(DLGCTX)) void* pData) noexcept override
    {
        auto pCtx = (DLGCTX*)pData;
        const auto ptdc = pCtx->ptdc;

        m_pfnRealCallback = ptdc->pfCallback;
        m_lRealRefData = ptdc->lpCallbackData;
        ptdc->pfCallback = EckTaskDialogCallback;
        ptdc->lpCallbackData = (LONG_PTR)this;

        int iButton{};
        BeginCbtHook(this);
        pCtx->hr = TaskDialogIndirect(ptdc, &iButton,
            &pCtx->iRadioButton, &pCtx->bChecked);
        return iButton;
    }

    LRESULT OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept override
    {
        switch (uMsg)
        {
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(Handle, &ps);
            SetDCBrushColor(ps.hdc, PtcCurrent()->crDefBkg);
            FillRect(ps.hdc, &ps.rcPaint, GetStockBrush(DC_BRUSH));
            EndPaint(Handle, &ps);
        }
        return 0;
        }
        return CDialog::OnMessage(uMsg, wParam, lParam);
    }

    EckInline BOOL EndDialog(INT_PTR nResult) noexcept override { return FALSE; }

    virtual HRESULT OnTaskDialogNotify(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept
    {
        if (m_pfnRealCallback)
            return m_pfnRealCallback(hWnd, uMsg, wParam, lParam, m_lRealRefData);
        else
            return m_CallbackSig.Emit(hWnd, uMsg, wParam, lParam);
    }

    EckInline void ClickButton(int iID) const noexcept
    {
        SendMessageW(TDM_CLICK_BUTTON, iID, 0);
    }

    EckInline void ClickRadioButton(int iID) const noexcept
    {
        SendMessageW(TDM_CLICK_RADIO_BUTTON, iID, 0);
    }

    EckInline void ClickCheckBox(BOOL bChecked, BOOL bSetFocus = FALSE) const noexcept
    {
        SendMessageW(TDM_CLICK_VERIFICATION, bChecked, bSetFocus);
    }

    EckInline void EnableButton(int iID, BOOL bEnable) const noexcept
    {
        SendMessageW(TDM_ENABLE_BUTTON, iID, bEnable);
    }

    EckInline void EnableRadioButton(int iID, BOOL bEnable) const noexcept
    {
        SendMessageW(TDM_ENABLE_RADIO_BUTTON, iID, bEnable);
    }

    EckInline void NavigatePage(_In_ TASKDIALOGCONFIG* pInfo) const noexcept
    {
        SendMessageW(TDM_NAVIGATE_PAGE, 0, (LPARAM)pInfo);
    }

    EckInline void NavigatePage() const noexcept
    {
        SendMessageW(TDM_NAVIGATE_PAGE, 0, (LPARAM)m_pParam->ptdc);
    }

    EckInline void SetShieldIcon(int iID, BOOL bShieldIcon) const noexcept
    {
        SendMessageW(TDM_SET_BUTTON_ELEVATION_REQUIRED_STATE, iID, bShieldIcon);
    }

    /// <summary>
    /// 置元素文本。
    /// 窗口布局可能会变化以适应新文本
    /// </summary>
    /// <param name="uType">元素类型，TDE_常量</param>
    /// <param name="pszText">文本</param>
    EckInline void SetElementText(UINT uType, _In_z_ PCWSTR pszText) const noexcept
    {
        SendMessageW(TDM_SET_ELEMENT_TEXT, uType, (LPARAM)pszText);
    }

    EckInline void PBSetMarqueeShowing(BOOL bShowing) const noexcept
    {
        SendMessageW(TDM_SET_MARQUEE_PROGRESS_BAR, bShowing, 0);
    }

    EckInline void PBSetMarquee(BOOL bMarquee, UINT uAnimationGap = 0u) const noexcept
    {
        SendMessageW(TDM_SET_PROGRESS_BAR_MARQUEE, bMarquee, uAnimationGap);
    }

    EckInline void PBSetPosition(int iPos) const noexcept
    {
        SendMessageW(TDM_SET_PROGRESS_BAR_POS, iPos, 0);
    }

    EckInline void PBSetRange(int iMin, int iMax) const noexcept
    {
        SendMessageW(TDM_SET_PROGRESS_BAR_POS, 0, MAKELPARAM(iMin, iMax));
    }

    /// <summary>
    /// 进度条_置状态
    /// </summary>
    /// <param name="uState">状态，PBST_常量</param>
    EckInline void PBSetState(UINT uState) const noexcept
    {
        SendMessageW(TDM_SET_PROGRESS_BAR_STATE, uState, 0);
    }

    /// <summary>
    /// 更新元素文本。
    /// 窗口布局不会变化，因此新文本必须短于旧文本
    /// </summary>
    /// <param name="uType">元素类型，TDE_常量</param>
    /// <param name="pszText">文本</param>
    EckInline void UpdateElementText(UINT uType, _In_z_ PCWSTR pszText) const noexcept
    {
        SendMessageW(TDM_UPDATE_ELEMENT_TEXT, uType, (LPARAM)pszText);
    }

    /// <summary>
    /// 更新图标
    /// </summary>
    /// <param name="uType">元素类型，TDIE_ICON_常量</param>
    /// <param name="Icon">图标，可为HICON或PCWSTR，取决于创建对话框时的设置</param>
    EckInline void UpdateIcon(UINT uType, HICON hIcon) const noexcept
    {
        SendMessageW(TDM_UPDATE_ICON, uType, (LPARAM)hIcon);
    }

    EckInline void UpdateIcon(UINT uType, PCWSTR pszIcon) const noexcept
    {
        SendMessageW(TDM_UPDATE_ICON, uType, (LPARAM)pszIcon);
    }

    EckInline constexpr auto& GetCallbackSignal() const noexcept { return m_CallbackSig; }
};
ECK_NAMESPACE_END