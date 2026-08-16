#pragma once
#include "CWindow.h"

ECK_NAMESPACE_BEGIN
class CComboBox : public CWindow
{
public:
    ECK_RTTI(CComboBox, CWindow);
    ECK_W_ATTACHABLE(CComboBox);
    ECK_W_CREATE_CLASS(WC_COMBOBOXW);

    constexpr static DWORD TypeMask = CBS_SIMPLE | CBS_DROPDOWN | CBS_DROPDOWNLIST;

    ECK_W_STYLE(AutoHScroll, CBS_AUTOHSCROLL);
    ECK_W_STYLE(DisableNoScroll, CBS_DISABLENOSCROLL);
    ECK_W_STYLE_MASK(DropDown, CBS_DROPDOWN, TypeMask);
    ECK_W_STYLE_MASK(DropDownList, CBS_DROPDOWNLIST, TypeMask);
    ECK_W_STYLE(HasString, CBS_HASSTRINGS);
    ECK_W_STYLE(LowerCase, CBS_LOWERCASE);
    ECK_W_STYLE(NoIntegralHeight, CBS_NOINTEGRALHEIGHT);
    ECK_W_STYLE(OemConvert, CBS_OEMCONVERT);
    ECK_W_STYLE(OwnerDrawFixed, CBS_OWNERDRAWFIXED);
    ECK_W_STYLE(OwnerDrawVariable, CBS_OWNERDRAWVARIABLE);
    ECK_W_STYLE_MASK(Simple, CBS_SIMPLE, TypeMask);
    ECK_W_STYLE(Sort, CBS_SORT);
    ECK_W_STYLE(UpperCase, CBS_UPPERCASE);

    EckInline int AddString(_In_z_ PCWSTR psz) const noexcept
    {
        return (int)SendMessageW(CB_ADDSTRING, 0, (LPARAM)psz);
    }

    EckInline int AddString(LPARAM lParam) const noexcept
    {
        return (int)SendMessageW(CB_ADDSTRING, 0, lParam);
    }

    /// <summary>
    /// 删除项目
    /// </summary>
    /// <param name="idx"></param>
    /// <returns>返回剩余项目数</returns>
    EckInline int DeleteString(int idx) const noexcept
    {
        return (int)SendMessageW(CB_DELETESTRING, idx, 0);
    }

    /// <summary>
    /// 加入路径
    /// </summary>
    /// <param name="pszPath">路径</param>
    /// <param name="uFlags">DDL_常量</param>
    /// <returns>索引</returns>
    EckInline int Directory(_In_z_ PCWSTR pszPath, UINT uFlags) const noexcept
    {
        return (int)SendMessageW(CB_DIR, uFlags, (LPARAM)pszPath);
    }

    /// <summary>
    /// 查找项目。
    /// 不区分大小写
    /// </summary>
    /// <param name="pszText">文本，将匹配以该文本开头的项目</param>
    /// <param name="idxStart">起始索引，-1 = 从头搜索整个列表</param>
    /// <returns>索引</returns>
    EckInline int FindString(_In_z_ PCWSTR pszText, int idxStart = -1) const noexcept
    {
        return (int)SendMessageW(CB_FINDSTRING, idxStart, (LPARAM)pszText);
    }

    /// <summary>
    /// 查找完全匹配项目。
    /// 不区分大小写
    /// </summary>
    /// <param name="pszText">文本，将匹配与该文本完全相同的项目</param>
    /// <param name="idxStart">起始索引，-1 = 从头搜索整个列表</param>
    /// <returns>索引</returns>
    EckInline int FindStringExact(_In_z_ PCWSTR pszText, int idxStart = -1) const noexcept
    {
        return (int)SendMessageW(CB_FINDSTRINGEXACT, idxStart, (LPARAM)pszText);
    }

    EckInline BOOL GetComboBoxInfomation(_Inout_ COMBOBOXINFO* pcbi) const noexcept
    {
        return (BOOL)SendMessageW(CB_GETCOMBOBOXINFO, 0, (LPARAM)pcbi);
    }

    EckInline int GetItemCount() const noexcept
    {
        return (int)SendMessageW(CB_GETCOUNT, 0, 0);
    }

    ECK_SUPPRESS_MISSING_ZERO_TERMINATION;
    /// <summary>
    /// 取提示横幅文本
    /// </summary>
    /// <param name="pszBuf">缓冲区</param>
    /// <param name="cchBuf">pszBuf指示的缓冲区大小，以WCHAR计，包含结尾NULL</param>
    /// <returns>成功返回1，失败返回错误代码</returns>
    EckInline int GetCueBanner(_Out_writes_(cchBuf) PWSTR pszBuf, int cchBuf) const noexcept
    {
        return (int)SendMessageW(CB_GETCUEBANNER, (WPARAM)pszBuf, cchBuf);
    }

    // 取提示横幅文本
    EckInline int GetCueBanner(Eck_Append_buffer_ CStringW& rs, int cchBuf = 64) const noexcept
    {
        const auto cchOld = rs.Size();
        const auto cch = GetCueBanner(rs.PushBackNoExtra(cchBuf), cchBuf);
        rs.ReSize(cchOld + cch);
        return cch;
    }

    /// <summary>
    /// 取现行选中项。
    /// 对单选列表框调用返回现行选中项，对多选列表框调用返回焦点项目
    /// </summary>
    /// <returns>索引</returns>
    EckInline int GetCurrentSelection() const noexcept
    {
        return (int)SendMessageW(CB_GETCURSEL, 0, 0);
    }

    /// <summary>
    /// 取下拉列表框矩形
    /// </summary>
    /// <param name="prc">接收矩形，相对屏幕</param>
    /// <returns>成功返回TRUE，失败返回FALSE</returns>
    EckInline BOOL GetDroppedControlRect(_Out_ RECT* prc) const noexcept
    {
        return (BOOL)SendMessageW(CB_GETDROPPEDCONTROLRECT, 0, (LPARAM)prc);
    }

    EckInline BOOL GetDroppedState() const noexcept
    {
        return (BOOL)SendMessageW(CB_GETDROPPEDSTATE, 0, 0);
    }

    /// <summary>
    /// 取下拉列表框最小宽度。
    /// 默认最小宽度为0，列表框宽度为max(最小宽度, 组合框主控件宽度)
    /// </summary>
    /// <returns>成功返回正的最小宽度，失败返回CB_ERR</returns>
    EckInline BOOL GetDroppedWidth() const noexcept
    {
        return (BOOL)SendMessageW(CB_GETDROPPEDWIDTH, 0, 0);
    }

    EckInline void GetEditSelection(
        _Out_opt_ UINT* puStart = nullptr,
        _Out_opt_ UINT* puEnd = nullptr) const noexcept
    {
        SendMessageW(CB_GETEDITSEL, (WPARAM)puStart, (LPARAM)puEnd);
    }

    EckInline BOOL GetExtendUi() const noexcept
    {
        return (BOOL)SendMessageW(CB_GETEXTENDEDUI, 0, 0);
    }

    EckInline int GetHorizontalExtent() const noexcept
    {
        return (int)SendMessageW(CB_GETHORIZONTALEXTENT, 0, 0);
    }

    EckInline LPARAM GetItemData(int idx) const noexcept
    {
        return SendMessageW(CB_GETITEMDATA, idx, 0);
    }

    EckInline int GetItemHeight(int idx) const noexcept
    {
        return (int)SendMessageW(CB_GETITEMHEIGHT, idx, 0);
    }

    /// <summary>
    /// 取项目文本
    /// </summary>
    /// <param name="idx">项目索引</param>
    /// <param name="pszBuf">缓冲区</param>
    /// <returns>返回字符数（不含结尾NULL），失败返回-1</returns>
    EckInline int GetItemText(int idx, PWSTR pszBuf) const noexcept
    {
        return (int)SendMessageW(CB_GETLBTEXT, idx, (LPARAM)pszBuf);
    }

    EckInline BOOL GetItemText(int idx, Eck_Append_buffer_ CStringW& rs) const noexcept
    {
        int cch = GetItemTextLength(idx);
        if (cch <= 0)
            return FALSE;
        return GetItemText(idx, rs.PushBackNoExtra(cch)) >= 0;
    }

    EckInline CStringW GetItemText(int idx) const noexcept
    {
        CStringW rs;
        GetItemText(idx, rs);
        return rs;
    }

    /// <summary>
    /// 取项目文本长度
    /// </summary>
    /// <param name="idx"></param>
    /// <returns>返回字符数（不含结尾NULL）</returns>
    EckInline int GetItemTextLength(int idx) const noexcept
    {
        return (int)SendMessageW(CB_GETLBTEXTLEN, idx, 0);
    }

    EckInline LCID GetLocale() const noexcept
    {
        return (LCID)SendMessageW(CB_GETLOCALE, 0, 0);
    }

    EckInline int GetMinimumVisible() const noexcept
    {
        return (int)SendMessageW(CB_GETMINVISIBLE, 0, 0);
    }

    EckInline int GetTopIndex() const noexcept
    {
        return (int)SendMessageW(CB_GETTOPINDEX, 0, 0);
    }

    /// <summary>
    /// 保留空间
    /// </summary>
    /// <param name="cItems">保留项目数</param>
    /// <param name="cbString">保留字符串长度</param>
    /// <returns>成功返回已预分配的项目总数，失败返回CB_ERRSPACE</returns>
    EckInline int InitialzeStorage(int cItems, UINT cbString) const noexcept
    {
        return (int)SendMessageW(CB_INITSTORAGE, cItems, cbString);
    }

    EckInline int InsertString(_In_z_ PCWSTR psz, int idxPos = -1) const noexcept
    {
        return (int)SendMessageW(CB_INSERTSTRING, idxPos, (LPARAM)psz);
    }

    EckInline int InsertString(LPARAM lParam, int idxPos = -1) const noexcept
    {
        return (int)SendMessageW(CB_INSERTSTRING, idxPos, lParam);
    }

    /// <summary>
    /// 置文本输入限制
    /// </summary>
    /// <param name="cch">字符串最长长度，若为0则限制为0x7FFFFFFE</param>
    EckInline void LimitText(int cch = 0) const noexcept
    {
        SendMessageW(CB_LIMITTEXT, cch, 0);
    }

    EckInline void ResetContent() const noexcept
    {
        SendMessageW(CB_RESETCONTENT, 0, 0);
    }

    /// <summary>
    /// 查找并选择项目。
    /// 不区分大小写
    /// </summary>
    /// <param name="pszText">文本，将匹配以该文本开头的项目</param>
    /// <param name="idxStart">起始索引，-1 = 从头搜索整个列表</param>
    /// <returns>索引，失败返回CB_ERR</returns>
    EckInline int SelectString(_In_z_ PCWSTR pszText, int idxStart = -1) const noexcept
    {
        return (int)SendMessageW(CB_SELECTSTRING, idxStart, (LPARAM)pszText);
    }

    /// <summary>
    /// 置提示横幅文本
    /// </summary>
    /// <param name="pszText">文本</param>
    /// <returns>成功返回1，失败返回错误码</returns>
    EckInline int SetCueBanner(_In_z_ PWSTR pszText) const noexcept
    {
        return (int)SendMessageW(CB_SETCUEBANNER, 0, (LPARAM)pszText);
    }

    EckInline BOOL SetCurrentSelection(int idxSel = -1) const noexcept
    {
        int iRet = (int)SendMessageW(CB_SETCURSEL, idxSel, 0);
        if (idxSel < 0)
            return TRUE;
        else
            return (iRet != CB_ERR);
    }

    EckInline BOOL SetDroppedWidth(int cx = 0) const noexcept
    {
        return (SendMessageW(CB_SETDROPPEDWIDTH, cx, 0) != CB_ERR);
    }

    EckInline void SetEditSelection(WORD wStart, WORD wEnd) const noexcept
    {
        SendMessageW(CB_SETEDITSEL, 0, MAKELPARAM(wStart, wEnd));
    }

    EckInline BOOL SetExtendUi(BOOL bExtUI) const noexcept
    {
        return (SendMessageW(CB_SETEXTENDEDUI, bExtUI, 0) != CB_ERR);
    }

    EckInline void SetHorizontalExtent(int iHorizontalExtent) const noexcept
    {
        SendMessageW(CB_SETHORIZONTALEXTENT, iHorizontalExtent, 0);
    }

    EckInline BOOL SetItemData(int idx, LPARAM lParam) const noexcept
    {
        return (SendMessageW(CB_SETITEMDATA, idx, lParam) != CB_ERR);
    }

    /// <summary>
    /// 置项目高度
    /// </summary>
    /// <param name="idx">为0时设置列表项目高度，为-1时设置主控件高度。
    /// 若组合框具有CBS_OWNERDRAWVARIABLE，则该参数指示项目索引</param>
    /// <param name="cy">高度</param>
    /// <returns>成功返回TRUE，失败返回FALSE</returns>
    EckInline BOOL SetItemHeight(int idx, int cy) const noexcept
    {
        return (SendMessageW(CB_SETITEMHEIGHT, idx, cy) != CB_ERR);
    }

    /// <summary>
    /// 置LCID
    /// </summary>
    /// <param name="lcid"></param>
    /// <returns>成功返回先前的LCID，失败返回CB_ERR</returns>
    EckInline LCID SetLocale(LCID lcid) const noexcept
    {
        return (LCID)SendMessageW(CB_SETLOCALE, lcid, 0);
    }

    EckInline BOOL SetMinimumVisible(int cItems) const noexcept
    {
        return (BOOL)SendMessageW(CB_SETMINVISIBLE, cItems, 0);
    }

    EckInline BOOL SetTopIndex(int idx) const noexcept
    {
        return (SendMessageW(CB_SETTOPINDEX, idx, 0) != CB_ERR);
    }

    EckInline void ShowDropDown(BOOL bShow) const noexcept
    {
        SendMessageW(CB_SHOWDROPDOWN, bShow, 0);
    }

    /// <summary>
    /// 置项目高度扩展。
    /// 修复设置高度时有偏差的问题，仅用于设置主控件高度
    /// </summary>
    /// <param name="cy">高度</param>
    /// <returns>成功返回TRUE，失败返回FALSE</returns>
    EckInline BOOL SetItemHeight(int cy) const noexcept
    {
        RECT rc;
        GetClientRect(m_hWnd, &rc);
        int iOffset = rc.bottom - (int)SendMessageW(CB_GETITEMHEIGHT, -1, 0);
        return (SendMessageW(CB_SETITEMHEIGHT, -1, cy - iOffset) != CB_ERR);
    }

    void SetItemString(int idx, _In_z_ PCWSTR pszText) const noexcept
    {
        LPARAM lParam = GetItemData(idx);
        int idxNew = InsertString(pszText, idx);
        SetItemData(idxNew, lParam);
        if (idxNew <= idx)
            DeleteString(idx + 1);
        else
            DeleteString(idx);
        SetCurrentSelection(idxNew);
    }
};
ECK_NAMESPACE_END