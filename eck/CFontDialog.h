#pragma once
#include "CDialog.h"
#include "Utility2.h"

ECK_NAMESPACE_BEGIN
class CFontDialog : public CDialog
{
public:
    ECK_RTTI(CFontDialog, CDialog);
public:
    // CHOOSEFONTW*
    INT_PTR CreateModalDialog(HWND hParent,
        _In_reads_bytes_(sizeof(CHOOSEFONTW)) void* pData = nullptr) noexcept override
    {
        BeginCbtHook(this);
        return ChooseFontW((CHOOSEFONTW*)pData);
    }

#if !ECK_OPT_NO_DWRITE && !ECK_OPT_NO_DARKMODE
    LRESULT OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept override
    {
        switch (uMsg)
        {
        case WM_DRAWITEM:
        {
            const auto* const pdis = (DRAWITEMSTRUCT*)lParam;
            switch (pdis->CtlID)
            {
            case 0x470:// 字体
            case 0x471:// 字形
            case 0x472:// 大小
            case 0x473:// 颜色
            case 0x474:// 脚本
            {
                if (!ShouldAppsUseDarkMode())
                    break;
                if (pdis->itemState & ODS_FOCUS)
                    DrawFocusRect(pdis->hDC, &pdis->rcItem);

                const auto cch = (int)::SendMessageW(pdis->hwndItem,
                    CB_GETLBTEXTLEN, pdis->itemID, 0);
                if (cch == CB_ERR)
                    return TRUE;
                const auto pszBuf = (PWSTR)_malloca((cch + 1) * sizeof(WCHAR));
                ::SendMessageW(pdis->hwndItem, CB_GETLBTEXT,
                    pdis->itemID, (LPARAM)pszBuf);
                const auto* const ptc = PtcCurrent();

                BOOL bDelFont{};
                HFONT hFont{};
                if (pdis->CtlID == 0x470)
                {
                    LOGFONTW lf;
                    GetObjectW(GetFont(), sizeof(lf), &lf);
                    const auto nDefHeight = lf.lfHeight;
                    if (SUCCEEDED(MatchLogFontFromFamilyName(pszBuf, lf)) &&
                        lf.lfCharSet != SYMBOL_CHARSET)
                    {
                        lf.lfHeight = nDefHeight;
                        lf.lfWidth = 0;
                        hFont = CreateFontIndirectW(&lf);
                        if (hFont)
                            bDelFont = TRUE;
#ifdef _DEBUG
                        else
                            EckDbgPrintFormat(L"CFontDialog::OnMessage: %s not found", pszBuf);
#endif// _DEBUG
                    }
                }
                if (!hFont)
                    hFont = GetFont();
                const auto hOldFont = (HFONT)SelectObject(pdis->hDC, hFont);
                if (pdis->itemState & ODS_SELECTED)
                {
                    SetTextColor(pdis->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
                    SetDCBrushColor(pdis->hDC, GetSysColor(COLOR_HIGHLIGHT));
                }
                else
                {
                    SetTextColor(pdis->hDC, ptc->crDefText);
                    SetDCBrushColor(pdis->hDC, ptc->crDefBkg);
                }
                SetBkMode(pdis->hDC, TRANSPARENT);
                const auto iDpi = GetDeviceCaps(pdis->hDC, LOGPIXELSX);
                const auto cxBorder = DaGetSystemMetrics(SM_CXBORDER, iDpi);
                int x = pdis->rcItem.left + cxBorder;
                if (pdis->CtlID == 0x473)// 让出颜色块位置
                    x += ((pdis->rcItem.bottom - pdis->rcItem.top) * 2 + cxBorder);
                RECT rc{ pdis->rcItem };
                rc.left += x;
                FillRect(pdis->hDC, &pdis->rcItem, GetStockBrush(DC_BRUSH));
                DrawTextW(pdis->hDC, pszBuf, cch, &rc,
                    DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);
                if (pdis->CtlID == 0x473)// 画颜色块
                {
                    const auto cyBorder = DaGetSystemMetrics(SM_CYBORDER, iDpi);
                    SetDCPenColor(pdis->hDC, ptc->crDefText);
                    SetDCBrushColor(pdis->hDC, (COLORREF)
                        ::SendMessageW(pdis->hwndItem, CB_GETITEMDATA,
                            pdis->itemID, 0));
                    SelectObject(pdis->hDC, GetStockObject(DC_PEN));
                    SelectObject(pdis->hDC, GetStockObject(DC_BRUSH));
                    Rectangle(pdis->hDC,
                        pdis->rcItem.left + cxBorder,
                        pdis->rcItem.top + cyBorder,
                        pdis->rcItem.left + (pdis->rcItem.bottom - pdis->rcItem.top) * 2,
                        pdis->rcItem.bottom - cyBorder);
                }
                SelectObject(pdis->hDC, hOldFont);
                _freea(pszBuf);
                if (bDelFont)
                    DeleteObject(hFont);
            }
            return TRUE;
            }
        }
        break;

        case WM_PAINT:
        {
            if (!ShouldAppsUseDarkMode())
                break;
            PAINTSTRUCT ps;
            BeginPaint(Handle, &ps);
            const auto hBTSample = GetDlgItem(Handle, 0x431);
            RECT rc;
            GetWindowRect(hBTSample, &rc);
            ScreenToClient(Handle, &rc);
            if (IsRectsIntersect(ps.rcPaint, rc))
            {
                const auto* const ptc = PtcCurrent();
                const auto hCBBColor = GetDlgItem(Handle, 0x473);
                COLORREF cr{ CLR_INVALID };
                if (GetWindowLongPtrW(hCBBColor, GWL_STYLE) & WS_VISIBLE)
                {
                    const auto idx = (int)::SendMessageW(hCBBColor, CB_GETCURSEL, 0, 0);
                    if (idx != CB_ERR)
                        cr = (COLORREF)::SendMessageW(hCBBColor, CB_GETITEMDATA,
                            ::SendMessageW(hCBBColor, CB_GETCURSEL, 0, 0), 0);
                }
                if (cr == CLR_INVALID)
                    cr = ptc->crDefText;
                SetTextColor(ps.hdc, cr);
                SetDCBrushColor(ps.hdc, ptc->crDefBkg);
                FillRect(ps.hdc, &rc, GetStockBrush(DC_BRUSH));
                SetBkMode(ps.hdc, TRANSPARENT);

                LOGFONTW lf;
                GetLogFont(&lf);
                lf.lfEscapement = 0;
                const auto hFont = CreateFontIndirectW(&lf);
                SelectObject(ps.hdc, hFont);

                const auto svText = GetResourceStringForCurrentLocale(
                    0x700 + lf.lfCharSet, GetModuleHandleW(L"comdlg32.dll"));
                DrawTextW(ps.hdc, svText.data(), (int)svText.size(), &rc,
                    DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);
                DeleteObject(hFont);
            }
            EndPaint(Handle, &ps);
        }
        return 0;
        }
        return __super::OnMessage(uMsg, wParam, lParam);
    }
#endif // !ECK_OPT_NO_DWRITE && !ECK_OPT_NO_DARKMODE

    void OnOk(HWND hCtrl) noexcept override {}
    void OnCancel(HWND hCtrl) noexcept override {}

    EckInline void GetLogFont(_Out_ LOGFONTW* plf) const noexcept
    {
        SendMessageW(WM_CHOOSEFONT_GETLOGFONT, 0, (LPARAM)plf);
    }
    EckInline void SetLogFont(_In_ LOGFONTW* plf) const noexcept
    {
        SendMessageW(WM_CHOOSEFONT_SETLOGFONT, 0, (LPARAM)plf);
    }
    EckInline void SetFlags(_In_ CHOOSEFONTW* pcf) const noexcept
    {
        SendMessageW(WM_CHOOSEFONT_SETFLAGS, 0, (LPARAM)pcf);
    }
    EckInline void SetFlags(UINT uFlags) const noexcept
    {
        CHOOSEFONTW cf{ sizeof(cf) };
        cf.Flags = uFlags;
        SetFlags(&cf);
    }
};
ECK_NAMESPACE_END