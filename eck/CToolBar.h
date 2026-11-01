#pragma once
#include "CWindow.h"

ECK_NAMESPACE_BEGIN
#define ECK_W_TLBE_STYLE(Name, Style)                      \
    ECKPROP(StyleGet##Name, StyleSet##Name) bool Name;     \
    bool StyleGet##Name() const noexcept                   \
    {                                                      \
        if constexpr (Style == 0)                          \
            return !GetTBExtendStyle();                    \
        else                                               \
            return IsBitSet(GetTBExtendStyle(), Style);    \
    }                                                      \
    void StyleSet##Name(bool b) const noexcept             \
    {                                                      \
        if (b)                                             \
            SetTBExtendStyle(GetTBExtendStyle() | Style);  \
        else                                               \
            SetTBExtendStyle(GetTBExtendStyle() & ~Style); \
    }

constexpr inline DWORD ToolBarPrettyStyle = TBSTYLE_LIST | TBSTYLE_TRANSPARENT |
CCS_NOPARENTALIGN | CCS_NORESIZE | CCS_NODIVIDER;

class CToolBar : public CWindow
{
public:
    ECK_RTTI(CToolBar, CWindow);
    ECK_W_ATTACHABLE(CToolBar);
    ECK_W_CREATE_CLASS(TOOLBARCLASSNAMEW);

    ECK_W_STYLE(AltDrag, TBSTYLE_ALTDRAG);
    ECK_W_STYLE(CustomErase, TBSTYLE_CUSTOMERASE);
    ECK_W_STYLE(Flat, TBSTYLE_FLAT);
    ECK_W_STYLE(List, TBSTYLE_LIST);
    ECK_W_STYLE(RegisterDrop, TBSTYLE_REGISTERDROP);
    ECK_W_STYLE(ToolTips, TBSTYLE_TOOLTIPS);
    ECK_W_STYLE(Transparent, TBSTYLE_TRANSPARENT);
    ECK_W_STYLE(Wrapable, TBSTYLE_WRAPABLE);
    ECK_W_TLBE_STYLE(DrawDropDownArrows, TBSTYLE_EX_DRAWDDARROWS);
    ECK_W_TLBE_STYLE(HideClippedButtons, TBSTYLE_EX_HIDECLIPPEDBUTTONS);
    ECK_W_TLBE_STYLE(DoubleBuffer, TBSTYLE_EX_DOUBLEBUFFER);
    ECK_W_TLBE_STYLE(MixedButtons, TBSTYLE_EX_MIXEDBUTTONS);

    EckInline int AddBitmap(int cBitmaps, HINSTANCE hInstance, UINT_PTR nID) const noexcept
    {
        TBADDBITMAP tbab{ hInstance,nID };
        return (int)SendMessageW(TB_ADDBITMAP, cBitmaps, (LPARAM)&tbab);
    }

    EckInline bool AddButtons(int cButtons, TBBUTTON* ptbb) const noexcept
    {
        return (int)SendMessageW(TB_ADDBUTTONSW, cButtons, (LPARAM)ptbb);
    }

    EckInline int AddString(HINSTANCE hInstance, PCWSTR pszIDOrText) const noexcept
    {
        return (int)SendMessageW(TB_ADDSTRINGW, (WPARAM)hInstance, (LPARAM)pszIDOrText);
    }

    EckInline void AutoSize() const noexcept
    {
        SendMessageW(TB_AUTOSIZE, 0, 0);
    }

    EckInline int GetButtonCount() const noexcept
    {
        return (int)SendMessageW(TB_BUTTONCOUNT, 0, 0);
    }

    EckInline void SetButtonStructSize(int cbStruct = sizeof(TBBUTTON)) const noexcept
    {
        SendMessageW(TB_BUTTONSTRUCTSIZE, cbStruct, 0);
    }

    EckInline bool ChangeBitmap(int idCommand, int idxImg) const noexcept
    {
        return (bool)SendMessageW(TB_CHANGEBITMAP, idCommand, idxImg);
    }

    EckInline bool CheckButton(int idCommand, bool bChecked) const noexcept
    {
        return (bool)SendMessageW(TB_CHECKBUTTON, idCommand, bChecked);
    }

    EckInline int CommandToIndex(int idCommand) const noexcept
    {
        return (int)SendMessageW(TB_COMMANDTOINDEX, idCommand, 0);
    }

    EckInline void Customize() const noexcept
    {
        SendMessageW(TB_CUSTOMIZE, 0, 0);
    }

    EckInline bool DeleteButton(int idxButton) const noexcept
    {
        return (bool)SendMessageW(TB_DELETEBUTTON, idxButton, 0);
    }

    EckInline bool EnableButton(int idCommand, bool bEnabled) const noexcept
    {
        return (bool)SendMessageW(TB_ENABLEBUTTON, idCommand, bEnabled);
    }

    EckInline bool GetAnchorHighlight() const noexcept
    {
        return (bool)SendMessageW(TB_GETANCHORHIGHLIGHT, 0, 0);
    }

    EckInline int GetBitmap(int idCommand) const noexcept
    {
        return (int)SendMessageW(TB_GETBITMAP, idCommand, 0);
    }

    EckInline UINT GetBitmapFlags() const noexcept
    {
        return (UINT)SendMessageW(TB_GETBITMAPFLAGS, 0, 0);
    }

    EckInline bool GetButton(int idxButton, TBBUTTON* ptbb) const noexcept
    {
        return (bool)SendMessageW(TB_GETBUTTON, idxButton, (LPARAM)ptbb);
    }

    EckInline int GetButtonInformation(int idCommand, TBBUTTONINFOW* ptbbi) const noexcept
    {
        return (int)SendMessageW(TB_GETBUTTONINFOW, idCommand, (LPARAM)ptbbi);
    }

    EckInline UINT GetButtonSize() const noexcept
    {
        return (UINT)SendMessageW(TB_GETBUTTONSIZE, 0, 0);
    }

    EckInline void GetButtonSize(int* pcx, int* pcy) const noexcept
    {
        const auto uRet = (UINT)SendMessageW(TB_GETBUTTONSIZE, 0, 0);
        if (pcx)
            *pcx = LOWORD(uRet);
        if (pcy)
            *pcy = HIWORD(uRet);
    }

    EckInline int GetButtonText(int idCommand, PWSTR pszBuf) const noexcept
    {
        return (int)SendMessageW(TB_GETBUTTONTEXTW, idCommand, (LPARAM)pszBuf);
    }

    EckInline bool GetColorScheme(COLORSCHEME* pcs) const noexcept
    {
        return (bool)SendMessageW(TB_GETCOLORSCHEME, 0, (LPARAM)pcs);
    }

    EckInline HIMAGELIST GetDisabledImageList() const noexcept
    {
        return (HIMAGELIST)SendMessageW(TB_GETDISABLEDIMAGELIST, 0, 0);
    }

    EckInline DWORD GetTBExtendStyle() const noexcept
    {
        return (DWORD)SendMessageW(TB_GETEXTENDEDSTYLE, 0, 0);
    }

    EckInline HIMAGELIST GetHotImageList() const noexcept
    {
        return (HIMAGELIST)SendMessageW(TB_GETHOTIMAGELIST, 0, 0);
    }

    EckInline int GetHotItem() const noexcept
    {
        return (int)SendMessageW(TB_GETHOTITEM, 0, 0);
    }

    EckInline bool GetIdealSize(bool bHeight, SIZE* psize) const noexcept
    {
        return (bool)SendMessageW(TB_GETIDEALSIZE, bHeight, (LPARAM)psize);
    }

    EckInline bool GetIdealSize(SIZE* psize) const noexcept
    {
        SIZE size;
        bool b = (bool)SendMessageW(TB_GETIDEALSIZE, true, (LPARAM)&size);
        psize->cy = size.cy;
        b = b && (bool)SendMessageW(TB_GETIDEALSIZE, false, (LPARAM)&size);
        psize->cx = size.cx;
        return b;
    }

    EckInline HIMAGELIST GetImageList() const noexcept
    {
        return (HIMAGELIST)SendMessageW(TB_GETIMAGELIST, 0, 0);
    }

    EckInline int GetImageListCount() const noexcept
    {
        return (int)SendMessageW(TB_GETIMAGELISTCOUNT, 0, 0);
    }

    EckInline void GetInsertMark(TBINSERTMARK* ptbim) const noexcept
    {
        SendMessageW(TB_GETINSERTMARK, 0, (LPARAM)ptbim);
    }

    EckInline COLORREF GetInsertMarkColor() const noexcept
    {
        return (COLORREF)SendMessageW(TB_GETINSERTMARKCOLOR, 0, 0);
    }

    EckInline void GetItemDropDownRect(int idxButton, RECT* prc) const noexcept
    {
        SendMessageW(TB_GETITEMDROPDOWNRECT, idxButton, (LPARAM)prc);
    }

    EckInline bool GetItemRect(int idxButton, RECT* prc) const noexcept
    {
        return (bool)SendMessageW(TB_GETITEMRECT, idxButton, (LPARAM)prc);
    }

    EckInline bool GetMaximumSize(SIZE* psize) const noexcept
    {
        return (bool)SendMessageW(TB_GETMAXSIZE, 0, (LPARAM)psize);
    }

    EckInline void GetMetrics(TBMETRICS* ptbm) const noexcept
    {
        SendMessageW(TB_GETMETRICS, 0, (LPARAM)ptbm);
    }

    EckInline HRESULT GetDropTargetObject(REFIID riid, IDropTarget** ppDropTarget) const noexcept
    {
        return (HRESULT)SendMessageW(TB_GETOBJECT, (WPARAM)&riid, (LPARAM)ppDropTarget);
    }

    EckInline UINT GetPadding() const noexcept
    {
        return (UINT)SendMessageW(TB_GETPADDING, 0, 0);
    }

    EckInline void GetPadding(int* pxPadding, int* pcPadding) const noexcept
    {
        const auto uRet = (UINT)SendMessageW(TB_GETPADDING, 0, 0);
        if (pxPadding)
            *pxPadding = LOWORD(uRet);
        if (pcPadding)
            *pcPadding = HIWORD(uRet);
    }

    EckInline HIMAGELIST GetPressedImageList() const noexcept
    {
        return (HIMAGELIST)SendMessageW(TB_GETPRESSEDIMAGELIST, 0, 0);
    }

    EckInline bool GetRect(int idCommand, RECT* prc) const noexcept
    {
        return (bool)SendMessageW(TB_GETRECT, idCommand, (LPARAM)prc);
    }

    EckInline int GetRows() const noexcept
    {
        return (int)SendMessageW(TB_GETROWS, 0, 0);
    }

    EckInline UINT GetState(int idxButton) const noexcept
    {
        return (UINT)SendMessageW(TB_GETSTATE, idxButton, 0);
    }

    EckInline int GetString(int idxString, int cbBuf, PWSTR pszBuf) const noexcept
    {
        return (int)SendMessageW(TB_GETSTRINGW, MAKEWPARAM(cbBuf, idxString), (LPARAM)pszBuf);
    }

    EckInline DWORD GetStyleTB() const noexcept
    {
        return (DWORD)SendMessageW(TB_GETSTYLE, 0, 0);
    }

    EckInline int GetTextRows() const noexcept
    {
        return (int)SendMessageW(TB_GETTEXTROWS, 0, 0);
    }

    EckInline HWND GetToolTips() const noexcept
    {
        return (HWND)SendMessageW(TB_GETTOOLTIPS, 0, 0);
    }

    EckInline bool HideButton(int idCommand, bool bHide) const noexcept
    {
        return (bool)SendMessageW(TB_HIDEBUTTON, idCommand, MAKELPARAM(bHide, 0));
    }

    EckInline int HitTest(POINT* ppt) const noexcept
    {
        return (int)SendMessageW(TB_HITTEST, 0, (LPARAM)ppt);
    }

    EckInline bool Indeterminate(int idCommand, bool bIndeterminate) const noexcept
    {
        return (bool)SendMessageW(TB_INDETERMINATE, idCommand, MAKELPARAM(bIndeterminate, 0));
    }

    EckInline bool InsertButton(int pos, TBBUTTON* ptbb) const noexcept
    {
        return (bool)SendMessageW(TB_INSERTBUTTONW, pos, (LPARAM)ptbb);
    }

    EckInline bool InsertMarkHitTest(POINT* ppt, TBINSERTMARK* ptbim) const noexcept
    {
        return (bool)SendMessageW(TB_INSERTMARKHITTEST, (WPARAM)ppt, (LPARAM)ptbim);
    }

    EckInline bool IsButtonChecked(int idCommand) const noexcept
    {
        return (bool)SendMessageW(TB_ISBUTTONCHECKED, idCommand, 0);
    }

    EckInline bool IsButtonEnabled(int idCommand) const noexcept
    {
        return (bool)SendMessageW(TB_ISBUTTONENABLED, idCommand, 0);
    }

    EckInline bool IsButtonHidden(int idCommand) const noexcept
    {
        return (bool)SendMessageW(TB_ISBUTTONHIDDEN, idCommand, 0);
    }

    EckInline bool IsButtonHighlighted(int idCommand) const noexcept
    {
        return (bool)SendMessageW(TB_ISBUTTONHIGHLIGHTED, idCommand, 0);
    }

    EckInline bool IsButtonIndeterminate(int idCommand) const noexcept
    {
        return (bool)SendMessageW(TB_ISBUTTONINDETERMINATE, idCommand, 0);
    }

    EckInline bool IsButtonPressed(int idCommand) const noexcept
    {
        return (bool)SendMessageW(TB_ISBUTTONPRESSED, idCommand, 0);
    }

    EckInline int LoadImages(UINT_PTR nID) const noexcept
    {
        return (int)SendMessageW(TB_LOADIMAGES, nID, (LPARAM)HINST_COMMCTRL);
    }

    EckInline bool MapAccelerator(WCHAR chAccel, UINT* pidCommand) const noexcept
    {
        return (bool)SendMessageW(TB_MAPACCELERATORW, chAccel, (LPARAM)pidCommand);
    }

    EckInline bool MarkButton(int idCommand, bool bHighlighted) const noexcept
    {
        return (bool)SendMessageW(TB_MARKBUTTON, idCommand, MAKELPARAM(bHighlighted, 0));
    }

    EckInline bool MoveButton(int idx, int idxDest) const noexcept
    {
        return (bool)SendMessageW(TB_MOVEBUTTON, idx, idxDest);
    }

    EckInline bool PressButton(int idCommand, bool bPressed) const noexcept
    {
        return (bool)SendMessageW(TB_PRESSBUTTON, idCommand, MAKELPARAM(bPressed, 0));
    }

    EckInline bool ReplaceBitmap(TBREPLACEBITMAP* ptbrb) const noexcept
    {
        return (bool)SendMessageW(TB_REPLACEBITMAP, 0, (LPARAM)ptbrb);
    }

    EckInline void SaveRestore(TBSAVEPARAMSW* ptbsp) const noexcept
    {
        SendMessageW(TB_SAVERESTOREW, 0, (LPARAM)ptbsp);
    }

    EckInline bool SetAnchorHighlighted(bool bHighlighted) const noexcept
    {
        return (bool)SendMessageW(TB_SETANCHORHIGHLIGHT, bHighlighted, 0);
    }

    EckInline bool SetBitmapSize(int cx, int cy) const noexcept
    {
        return (bool)SendMessageW(TB_SETBITMAPSIZE, 0, MAKELPARAM(cx, cy));
    }

    EckInline bool SetButtonInformation(int idCommand, TBBUTTONINFOW* ptbbi) const noexcept
    {
        return (bool)SendMessageW(TB_SETBUTTONINFOW, idCommand, (LPARAM)ptbbi);
    }

    EckInline bool SetButtonSize(int cx, int cy) const noexcept
    {
        return (bool)SendMessageW(TB_SETBUTTONSIZE, 0, MAKELPARAM(cx, cy));
    }

    EckInline bool SetButtonWidth(int cxMin, int cxMax) const noexcept
    {
        return (bool)SendMessageW(TB_SETBUTTONWIDTH, 0, MAKELPARAM(cxMin, cxMax));
    }

    EckInline bool SetCommandId(int idxButton, int idNew) const noexcept
    {
        return (bool)SendMessageW(TB_SETCMDID, idxButton, idNew);
    }

    EckInline void SetColorScheme(COLORSCHEME* pcs) const noexcept
    {
        SendMessageW(TB_SETCOLORSCHEME, 0, (LPARAM)pcs);
    }

    EckInline HIMAGELIST SetDisabledImageList(HIMAGELIST hImageList) const noexcept
    {
        return (HIMAGELIST)SendMessageW(TB_SETDISABLEDIMAGELIST, 0, (LPARAM)hImageList);
    }

    EckInline UINT SetDrawTextFlags(UINT uMask, UINT uDTFlags) const noexcept
    {
        return (UINT)SendMessageW(TB_SETDRAWTEXTFLAGS, uMask, uDTFlags);
    }

    EckInline DWORD SetTBExtendStyle(DWORD dwExStyle) const noexcept
    {
        return (DWORD)SendMessageW(TB_SETEXTENDEDSTYLE, 0, dwExStyle);
    }

    EckInline HIMAGELIST SetHotImageList(HIMAGELIST hImageList) const noexcept
    {
        return (HIMAGELIST)SendMessageW(TB_SETHOTIMAGELIST, 0, (LPARAM)hImageList);
    }

    EckInline int SetHotItem(int idxHot) const noexcept
    {
        return (int)SendMessageW(TB_SETHOTITEM, idxHot, 0);
    }

    EckInline int SetHotItem(int idxHot, UINT uFlags) const noexcept
    {
        return (int)SendMessageW(TB_SETHOTITEM2, idxHot, uFlags);
    }

    EckInline HIMAGELIST SetImageList(HIMAGELIST hImageList, int idxImageList = 0) const noexcept
    {
        return (HIMAGELIST)SendMessageW(TB_SETIMAGELIST, idxImageList, (LPARAM)hImageList);
    }

    EckInline bool SetIndent(int iIndent) const noexcept
    {
        return (bool)SendMessageW(TB_SETINDENT, iIndent, 0);
    }

    EckInline void SetInsertMark(TBINSERTMARK* ptbim) const noexcept
    {
        SendMessageW(TB_SETINSERTMARK, 0, (LPARAM)ptbim);
    }

    EckInline COLORREF SetInsertMarkColor(COLORREF cr) const noexcept
    {
        return (COLORREF)SendMessageW(TB_SETINSERTMARKCOLOR, 0, cr);
    }

    EckInline void SetListGap(int iGap) const noexcept
    {
        SendMessageW(TB_SETLISTGAP, iGap, 0);
    }

    EckInline bool SetMaximumRows(int cRows) const noexcept
    {
        return (bool)SendMessageW(TB_SETMAXTEXTROWS, cRows, 0);
    }

    EckInline void SetMetrics(TBMETRICS* ptbm) const noexcept
    {
        SendMessageW(TB_SETMETRICS, 0, (LPARAM)ptbm);
    }

    EckInline UINT SetPadding(int xPadding, int yPadding) const noexcept
    {
        return (UINT)SendMessageW(TB_SETPADDING, 0, MAKELPARAM(xPadding, yPadding));
    }

    EckInline HWND SetParent(HWND hWndNotify) const noexcept
    {
        return (HWND)SendMessageW(TB_SETPARENT, (WPARAM)hWndNotify, 0);
    }

    EckInline HIMAGELIST SetPressedImageList(HIMAGELIST hImageList, int idxImageList = 0) const noexcept
    {
        return (HIMAGELIST)SendMessageW(TB_SETPRESSEDIMAGELIST, idxImageList, (LPARAM)hImageList);
    }

    EckInline void SetRows(int cRows, bool bAllowMoreRows, RECT* prc) const noexcept
    {
        SendMessageW(TB_SETROWS, MAKEWPARAM(cRows, bAllowMoreRows), (LPARAM)prc);
    }

    EckInline UINT SetState(int idCommand, UINT uState) const noexcept
    {
        return (UINT)SendMessageW(TB_SETSTATE, idCommand, uState);
    }

    EckInline void SetStyleTB(DWORD dwStyle) const noexcept
    {
        SendMessageW(TB_SETSTYLE, 0, dwStyle);
    }

    EckInline void SetToolTips(HWND hToolTip) const noexcept
    {
        SendMessageW(TB_SETTOOLTIPS, (WPARAM)hToolTip, 0);
    }
};
ECK_NAMESPACE_END