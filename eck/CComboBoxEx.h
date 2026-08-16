#pragma once
#include "CComboBox.h"
#include "CByteBufferStream.h"
#include "CStreamView.h"

ECK_NAMESPACE_BEGIN
#define ECK_W_CBE_STYLE(Name, Style)                    \
    ECKPROP(StyleGet##Name, StyleSet##Name) BOOL Name;  \
    BOOL StyleGet##Name() const                         \
    {                                                   \
        if constexpr (Style == 0)                       \
            return !GetExtendedStyle();                 \
        else                                            \
            return IsBitSet(GetExtendedStyle(), Style); \
    }                                                   \
    void StyleSet##Name(BOOL b) const                   \
    {                                                   \
        SetExtendedStyle(b ? Style : 0, Style);         \
    }

class CComboBoxEx : public CComboBox
{
public:
    ECK_RTTI(CComboBoxEx, CComboBox);
    ECK_W_ATTACHABLE(CComboBoxEx);
    ECK_W_CREATE_CLASS(WC_COMBOBOXEXW);

    ECK_W_CBE_STYLE(CaseSensitive, CBES_EX_CASESENSITIVE);
    ECK_W_CBE_STYLE(NoEditImage, CBES_EX_NOEDITIMAGE);
    ECK_W_CBE_STYLE(NoEditImageIndent, CBES_EX_NOEDITIMAGEINDENT);
    ECK_W_CBE_STYLE(NoSizeLimit, CBES_EX_NOSIZELIMIT);
    ECK_W_CBE_STYLE(PathWordBreakProc, CBES_EX_PATHWORDBREAKPROC);
    ECK_W_CBE_STYLE(TextEndEllipsis, CBES_EX_TEXTENDELLIPSIS);

    /// <summary>
    /// 删除项目
    /// </summary>
    /// <param name="idx">索引</param>
    /// <returns>成功返回剩余项数，失败返回CB_ERR</returns>
    EckInline int DeleteItem(int idx) const noexcept
    {
        return (int)SendMessageW(CBEM_DELETEITEM, idx, 0);
    }

    EckInline HWND GetComboBoxControl() const noexcept
    {
        return (HWND)SendMessageW(CBEM_GETCOMBOCONTROL, 0, 0);
    }

    EckInline HWND GetEditControl() const noexcept
    {
        return (HWND)SendMessageW(CBEM_GETEDITCONTROL, 0, 0);
    }

    EckInline DWORD GetExtendedStyle() const noexcept
    {
        return (DWORD)SendMessageW(CBEM_GETEXTENDEDSTYLE, 0, 0);
    }

    EckInline HIMAGELIST GetImageList() const noexcept
    {
        return (HIMAGELIST)SendMessageW(CBEM_GETIMAGELIST, 0, 0);
    }

    EckInline BOOL GetItem(_Inout_ COMBOBOXEXITEMW* pcbei) const noexcept
    {
        return (BOOL)SendMessageW(CBEM_GETITEMW, 0, (LPARAM)pcbei);
    }

    EckInline BOOL GetItemText(
        int idx,
        Eck_Append_buffer_ CStringW& rs,
        int cchBuf = MAX_PATH) const noexcept
    {
        const auto cchOld = rs.Size();
        COMBOBOXEXITEMW cbei;
        cbei.iItem = idx;
        cbei.mask = CBEIF_TEXT;
        cbei.pszText = rs.PushBack(cchBuf);
        cbei.cchTextMax = cchBuf;
        const auto bRet = GetItem(&cbei);
        if (bRet && cbei.pszText)
        {
            if (cbei.pszText == rs.Data())
                rs.ReCalculateLength(cchOld);
            else
            {
                const auto cchText = (int)TcsLength(cbei.pszText);
                rs.ReSize(cchOld + cchText);
                TcsCopyLength(rs.Data() + cchOld, cbei.pszText, cchText);
            }
        }
        else
            rs.ReSize(cchOld);
        return bRet;
    }

    EckInline CStringW GetItemText(int idx) const noexcept
    {
        CStringW rs;
        GetItemText(idx, rs);
        return rs;
    }

    EckInline BOOL HasEditChanged() const noexcept
    {
        return (BOOL)SendMessageW(CBEM_HASEDITCHANGED, 0, 0);
    }

    EckInline int InsertItem(_In_ COMBOBOXEXITEMW* pcbei) const noexcept
    {
        return (int)SendMessageW(CBEM_INSERTITEMW, 0, (LPARAM)pcbei);
    }

    int InsertItem(_In_z_ PCWSTR pszText, int idx = -1, int iImage = -1,
        int idxSelImage = -1, LPARAM lParam = 0) const noexcept
    {
        COMBOBOXEXITEMW cbei;
        cbei.mask = CBEIF_IMAGE | CBEIF_LPARAM | CBEIF_TEXT | CBEIF_SELECTEDIMAGE;
        cbei.iItem = idx;
        cbei.pszText = (PWSTR)pszText;
        cbei.iImage = iImage;
        cbei.iSelectedImage = idxSelImage < 0 ? iImage : idxSelImage;
        cbei.lParam = lParam;
        return InsertItem(&cbei);
    }

    /// <summary>
    /// 置扩展样式
    /// </summary>
    /// <param name="dwStyle">样式</param>
    /// <param name="dwMask">掩码，若为0则修改所有样式</param>
    /// <returns>返回先前样式</returns>
    EckInline DWORD SetExtendedStyle(DWORD dwStyle, DWORD dwMask = 0u) const noexcept
    {
        return (DWORD)SendMessageW(CBEM_SETEXTENDEDSTYLE, dwMask, dwStyle);
    }

    EckInline HIMAGELIST SetImageList(HIMAGELIST hImageList) const noexcept
    {
        return (HIMAGELIST)SendMessageW(CBEM_SETIMAGELIST, 0, (LPARAM)hImageList);
    }

    EckInline IImageList* SetImageList(IImageList* pImageList) const noexcept
    {
        return (IImageList*)SetImageList((HIMAGELIST)pImageList);
    }

    EckInline BOOL SetItem(_In_ COMBOBOXEXITEMW* pcbei) const noexcept
    {
        return (BOOL)SendMessageW(CBEM_SETITEMW, 0, (LPARAM)pcbei);
    }

    EckInline BOOL SetItemText(int idx, _In_z_ PCWSTR pszText) const noexcept
    {
        COMBOBOXEXITEMW cbei;
        cbei.iItem = idx;
        cbei.mask = CBEIF_TEXT;
        cbei.pszText = (PWSTR)pszText;
        return SetItem(&cbei);
    }
};
ECK_NAMESPACE_END