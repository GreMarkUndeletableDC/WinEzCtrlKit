#pragma once
#include "CString.h"

ECK_NAMESPACE_BEGIN
class CMenu
{
private:
    HMENU m_hMenu{};

    EckInlineNdCe static UINT PositionFlag(bool bPos) noexcept
    {
        return bPos ? MF_BYPOSITION : MF_BYCOMMAND;
    }
public:
    struct INITITEM
    {
        PCWSTR pszText = nullptr;
        UINT uId = 0u;
        UINT uFlags = 0u;
    };

    CMenu() = default;
    CMenu(std::initializer_list<INITITEM> Items) noexcept
        : m_hMenu{ CreatePopupMenu() }
    {
        for (auto x : Items)
            AppendString(x.pszText, x.uId, x.uFlags);
    }

    CMenu(HMENU hMenu) noexcept : m_hMenu{ hMenu } {}

    CMenu(const CMenu&) = delete;
    CMenu& operator=(const CMenu&) = delete;

    CMenu(CMenu&& x) noexcept { std::swap(m_hMenu, x.m_hMenu); }
    CMenu& operator=(CMenu&& x) noexcept
    {
        std::swap(m_hMenu, x.m_hMenu);
        return *this;
    }

    ~CMenu() noexcept { Clear(); }

    EckInline HMENU Attach(HMENU hMenu) noexcept
    {
        std::swap(m_hMenu, hMenu);
        return hMenu;
    }
    EckInlineNd HMENU Detach() noexcept { return Attach(nullptr); }

    EckInlineNd HMENU GetHandle() const noexcept { return m_hMenu; }

    EckInline bool AppendString(PCWSTR pszText, UINT uId, UINT uFlags = 0u) const noexcept
    {
        return AppendMenuW(m_hMenu, uFlags | MF_STRING, uId, pszText);
    }
    EckInline bool AppendString(PCWSTR pszText, HMENU hSubMenu, UINT uFlags = 0u) const noexcept
    {
        return AppendMenuW(m_hMenu, uFlags | MF_STRING | MF_POPUP, (UINT_PTR)hSubMenu, pszText);
    }

    EckInline bool AppendBitmap(HBITMAP hBitmap, UINT uId, UINT uFlags = 0u) const noexcept
    {
        return AppendMenuW(m_hMenu, uFlags | MF_BITMAP, uId, (PCWSTR)hBitmap);
    }
    EckInline bool AppendBitmap(HBITMAP hBitmap, HMENU hSubMenu, UINT uFlags = 0u) const noexcept
    {
        return AppendMenuW(m_hMenu, uFlags | MF_BITMAP | MF_POPUP, (UINT_PTR)hSubMenu, (PCWSTR)hBitmap);
    }

    EckInline bool AppendOwnerDraw(LPARAM lParam, UINT uId, UINT uFlags = 0u) const noexcept
    {
        return AppendMenuW(m_hMenu, uFlags | MF_OWNERDRAW, uId, (PCWSTR)lParam);
    }
    EckInline bool AppendOwnerDraw(LPARAM lParam, HMENU hSubMenu, UINT uFlags = 0u) const noexcept
    {
        return AppendMenuW(m_hMenu, uFlags | MF_OWNERDRAW | MF_POPUP, (UINT_PTR)hSubMenu, (PCWSTR)lParam);
    }

    EckInline bool AppendItem(UINT uFlags, UINT_PTR uIDNewItem, PCWSTR pNewItem) const noexcept
    {
        return AppendMenuW(m_hMenu, uFlags, uIDNewItem, pNewItem);
    }
    EckInline bool AppendSeparator() const noexcept
    {
        return AppendMenuW(m_hMenu, MF_SEPARATOR, 0, nullptr);
    }

    EckInline bool CheckItem(bool bCheck, UINT uPos, bool bPosition = false) const noexcept
    {
        MENUITEMINFOW mii;
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_STATE;
        if (!GetItemInformation(&mii, uPos, bPosition))
            return false;
        mii.fState &= (MF_CHECKED | MF_UNCHECKED);
        mii.fState |= (bCheck ? MF_CHECKED : MF_UNCHECKED);
        return SetItemInformation(&mii, uPos, bPosition);
    }

    EckInline bool CheckRadioItem(UINT uBegin, UINT uEnd, UINT uCheck, bool bPosition = false) const noexcept
    {
        return CheckMenuRadioItem(m_hMenu, uBegin, uEnd, uCheck, PositionFlag(bPosition));
    }

    EckInline HMENU Create() noexcept
    {
        DestroyMenu(Attach(CreateMenu()));
        return m_hMenu;
    }
    EckInline HMENU CreatePopup() noexcept
    {
        DestroyMenu(Attach(CreatePopupMenu()));
        return m_hMenu;
    }

    EckInline bool DeleteItem(UINT uPos, bool bPosition = false) const noexcept
    {
        return DeleteMenu(m_hMenu, uPos, PositionFlag(bPosition));
    }

    EckInline bool Clear() noexcept
    {
        return DestroyMenu(Attach(nullptr));
    }

    EckInline int EnableItem(UINT uPos, UINT uFlags, bool bPosition = false) const noexcept
    {
        return EnableMenuItem(m_hMenu, uPos, PositionFlag(bPosition) | uFlags);
    }

    EckInline UINT GetDefaultItem(UINT uFlags, bool bPosition = false) const noexcept
    {
        return GetMenuDefaultItem(m_hMenu, PositionFlag(bPosition), uFlags);
    }

    EckInline bool GetInformation(MENUINFO* pmi) const noexcept
    {
        return GetMenuInfo(m_hMenu, pmi);
    }

    EckInlineNd int GetItemCount() const noexcept
    {
        return GetMenuItemCount(m_hMenu);
    }

    EckInlineNd UINT GetItemId(int idx) const noexcept
    {
        return GetMenuItemID(m_hMenu, idx);
    }

    EckInline bool GetItemInformation(
        _Inout_ MENUITEMINFOW* pmii,
        UINT uPos,
        bool bPosition = false) const noexcept
    {
        return GetMenuItemInfoW(m_hMenu, uPos, bPosition, pmii);
    }

    EckInline bool GetItemRect(HWND hWnd, int idx, _Out_ RECT* prc) const noexcept
    {
        return GetMenuItemRect(hWnd, m_hMenu, idx, prc);
    }

    EckInlineNd UINT GetItemState(UINT uPos, bool bPosition = false) const noexcept
    {
        return GetMenuState(m_hMenu, uPos, PositionFlag(bPosition));
    }

    EckInlineNd bool GetItemString(
        Eck_Append_buffer_ CStringW& rs,
        UINT uPos,
        bool bPosition = false) const noexcept
    {
        MENUITEMINFOW mii;
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE;
        mii.cch = 0;
        if (!GetItemInformation(&mii, uPos, bPosition))
            return false;
        if (mii.cch)
        {
            ++mii.cch;
            mii.dwTypeData = rs.PushBackNoExtra(mii.cch);
            mii.fMask = MIIM_STRING;
            return GetItemInformation(&mii, uPos, bPosition);
        }
        return true;
    }

    EckInline bool GetItemString(
        _Out_writes_(cchBuf) PWSTR pszBuf,
        int cchBuf,
        UINT uPos,
        bool bPosition = false) const noexcept
    {
        MENUITEMINFOW mii;
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_STRING;
        mii.cch = cchBuf;
        mii.dwTypeData = pszBuf;
        return GetItemInformation(&mii, uPos, bPosition);
    }

    EckInlineNd HMENU GetSubMenu(int idx) const noexcept
    {
        return ::GetSubMenu(m_hMenu, idx);
    }

    EckInline bool HighLightItem(
        HWND hWnd,
        bool bHiLite,
        UINT uPos,
        bool bPosition = false) const noexcept
    {
        return HiliteMenuItem(hWnd, m_hMenu, uPos,
            PositionFlag(bPosition) | (bHiLite ? MF_HILITE : MF_UNHILITE));
    }

    EckInline bool InsertString(PCWSTR pszText, UINT uId, UINT uPos, UINT uFlags = 0u) const noexcept
    {
        return InsertMenuW(m_hMenu, uPos, uFlags | MF_STRING, uId, pszText);
    }
    EckInline bool InsertString(PCWSTR pszText, HMENU hSubMenu, UINT uPos, UINT uFlags = 0u) const noexcept
    {
        return InsertMenuW(m_hMenu, uPos, uFlags | MF_STRING | MF_POPUP, (UINT_PTR)hSubMenu, pszText);
    }

    EckInline bool InsertBitmap(HBITMAP hBitmap, UINT uId, UINT uPos, UINT uFlags = 0u) const noexcept
    {
        return InsertMenuW(m_hMenu, uPos, uFlags | MF_BITMAP, uId, (PCWSTR)hBitmap);
    }
    EckInline bool InsertBitmap(HBITMAP hBitmap, HMENU hSubMenu, UINT uPos, UINT uFlags = 0u) const noexcept
    {
        return InsertMenuW(m_hMenu, uPos, uFlags | MF_BITMAP | MF_POPUP, (UINT_PTR)hSubMenu, (PCWSTR)hBitmap);
    }

    EckInline bool InsertOwnerDraw(LPARAM lParam, UINT uId, UINT uPos, UINT uFlags = 0u) const noexcept
    {
        return InsertMenuW(m_hMenu, uPos, uFlags | MF_OWNERDRAW, uId, (PCWSTR)lParam);
    }
    EckInline bool InsertOwnerDraw(LPARAM lParam, HMENU hSubMenu, UINT uPos, UINT uFlags = 0u) const noexcept
    {
        return InsertMenuW(m_hMenu, uPos, uFlags | MF_OWNERDRAW | MF_POPUP, (UINT_PTR)hSubMenu, (PCWSTR)lParam);
    }

    EckInline bool InsertItem(UINT uPos, UINT uFlags, UINT_PTR uIDNewItem, PCWSTR pNewItem) const noexcept
    {
        return InsertMenuW(m_hMenu, uPos, uFlags, uIDNewItem, pNewItem);
    }
    EckInline bool InsertItem(const MENUITEMINFOW* pmii, UINT uPos, bool bPosition = false) const noexcept
    {
        return InsertMenuItemW(m_hMenu, uPos, bPosition, pmii);
    }
    EckInline bool InsertSeparator(UINT uPos, UINT uFlags) const noexcept
    {
        return InsertMenuW(m_hMenu, uPos, MF_SEPARATOR | uFlags, 0, nullptr);
    }

    EckInlineNd bool IsValid() const noexcept
    {
        return m_hMenu && ::IsMenu(m_hMenu);
    }

    EckInlineNd int HitTest(HWND hWnd, POINT pt) const noexcept
    {
        return MenuItemFromPoint(hWnd, m_hMenu, pt);
    }

    EckInline bool ModifyString(PCWSTR pszText, UINT uId, UINT uPos, UINT uFlags = 0u) const noexcept
    {
        return ModifyMenuW(m_hMenu, uPos, uFlags | MF_STRING, uId, pszText);
    }
    EckInline bool ModifyString(PCWSTR pszText, HMENU hSubMenu, UINT uPos, UINT uFlags = 0u) const noexcept
    {
        return ModifyMenuW(m_hMenu, uPos, uFlags | MF_STRING | MF_POPUP, (UINT_PTR)hSubMenu, pszText);
    }

    EckInline bool ModifyBitmap(HBITMAP hBitmap, UINT uId, UINT uPos, UINT uFlags = 0u) const noexcept
    {
        return ModifyMenuW(m_hMenu, uPos, uFlags | MF_BITMAP, uId, (PCWSTR)hBitmap);
    }
    EckInline bool ModifyBitmap(HBITMAP hBitmap, HMENU hSubMenu, UINT uPos, UINT uFlags = 0u) const noexcept
    {
        return ModifyMenuW(m_hMenu, uPos, uFlags | MF_BITMAP | MF_POPUP, (UINT_PTR)hSubMenu, (PCWSTR)hBitmap);
    }

    EckInline bool ModifyOwnerDraw(LPARAM lParam, UINT uId, UINT uPos, UINT uFlags = 0u) const noexcept
    {
        return ModifyMenuW(m_hMenu, uPos, uFlags | MF_OWNERDRAW, uId, (PCWSTR)lParam);
    }
    EckInline bool ModifyOwnerDraw(LPARAM lParam, HMENU hSubMenu, UINT uPos, UINT uFlags = 0u) const noexcept
    {
        return ModifyMenuW(m_hMenu, uPos, uFlags | MF_OWNERDRAW | MF_POPUP, (UINT_PTR)hSubMenu, (PCWSTR)lParam);
    }

    EckInline bool ModifyItem(UINT uPos, UINT uFlags, UINT_PTR uIDNewItem, PCWSTR pNewItem) const noexcept
    {
        return ModifyMenuW(m_hMenu, uPos, uFlags, uIDNewItem, pNewItem);
    }

    EckInline bool RemoveItem(UINT uPos, bool bPosition = false) const noexcept
    {
        return RemoveMenu(m_hMenu, uPos, PositionFlag(bPosition));
    }

    EckInline bool SetMenu(HWND hWnd) const noexcept
    {
        return ::SetMenu(hWnd, m_hMenu);
    }

    EckInline bool SetDefaultItem(UINT uPos, bool bPosition = false) const noexcept
    {
        return SetMenuDefaultItem(m_hMenu, uPos, bPosition);
    }

    EckInline bool SetInformation(const MENUINFO* pmi) const noexcept
    {
        return SetMenuInfo(m_hMenu, pmi);
    }

    EckInline bool SetItemBitmaps(
        HBITMAP hbmUnchecked,
        HBITMAP hbmChecked,
        UINT uPos,
        bool bPosition = false) const noexcept
    {
        return SetMenuItemBitmaps(m_hMenu, uPos, PositionFlag(bPosition), hbmUnchecked, hbmChecked);
    }

    EckInline bool SetItemInformation(
        _In_ const MENUITEMINFOW* pmii,
        UINT uPos,
        bool bPosition = false) const noexcept
    {
        return SetMenuItemInfoW(m_hMenu, uPos, bPosition, pmii);
    }

    EckInline bool TrackPopupMenu(HWND hWnd, int x, int y, UINT uFlags = 0u) const noexcept
    {
        return ::TrackPopupMenu(m_hMenu, uFlags, x, y, 0, hWnd, nullptr);
    }
    EckInline bool TrackPopupMenuEx(
        HWND hWnd,
        int x, int y,
        UINT uFlags,
        _In_opt_ const TPMPARAMS* ptpmp) const noexcept
    {
        return ::TrackPopupMenuEx(m_hMenu, uFlags, x, y, hWnd, (TPMPARAMS*)ptpmp);
    }
};
ECK_NAMESPACE_END