#pragma once
#include "CWindow.h"
#include "DDX.h"

ECK_NAMESPACE_BEGIN
class CButton : public CWindow
{
public:
    ECK_RTTI(CButton, CWindow);
    ECK_W_ATTACHABLE(CButton);
    ECK_W_CREATE_CLASS(WC_BUTTONW);

    constexpr static DWORD TypeMask = (BS_PUSHBUTTON | BS_DEFPUSHBUTTON |
        BS_SPLITBUTTON | BS_DEFSPLITBUTTON | BS_COMMANDLINK | BS_DEFCOMMANDLINK |
        BS_RADIOBUTTON | BS_AUTORADIOBUTTON | BS_CHECKBOX | BS_AUTOCHECKBOX |
        BS_3STATE | BS_AUTO3STATE | BS_GROUPBOX);

    enum class Type
    {
        PushButton,
        DefaultPushButton,
        CheckBox,
        AutoCheckButton,
        RadioButton,
        TripleState,
        AutoTripleState,
        GroupBox,
        UserButton,// 已弃用
        AutoRadioButton,
        PushBox,// 与PushButton类似，但仅显示文本
        OwnerDraw,
        SplitButton,
        DefaultSplitButton,
        CommandLink,
        DefaultCommandLink,

        Unknown = -1
    };
public:
    ECK_W_STYLE_MASK(TripleState, BS_3STATE, TypeMask);
    ECK_W_STYLE_MASK(AutoTripleState, BS_AUTO3STATE, TypeMask);
    ECK_W_STYLE_MASK(AutoCheckButton, BS_AUTOCHECKBOX, TypeMask);
    ECK_W_STYLE_MASK(AutoRadioButton, BS_AUTORADIOBUTTON, TypeMask);
    ECK_W_STYLE(ShowBitmap, BS_BITMAP);
    ECK_W_STYLE(AlignmentBottom, BS_BOTTOM);
    ECK_W_STYLE(AlignmentCenter, BS_CENTER);
    ECK_W_STYLE_MASK(CheckBox, BS_CHECKBOX, TypeMask);
    ECK_W_STYLE(CommandLink, BS_COMMANDLINK);
    ECK_W_STYLE(DefaultCommandLink, BS_DEFCOMMANDLINK);
    ECK_W_STYLE_MASK(DefaultPushButton, BS_DEFPUSHBUTTON, TypeMask);
    ECK_W_STYLE(DefaultSplitButton, BS_DEFSPLITBUTTON);
    ECK_W_STYLE_MASK(GroupBox, BS_GROUPBOX, TypeMask);
    ECK_W_STYLE(ShowIcon, BS_ICON);
    ECK_W_STYLE(Flat, BS_FLAT);
    ECK_W_STYLE(AlignmentLeft, BS_LEFT);
    ECK_W_STYLE(MultiLine, BS_MULTILINE);
    ECK_W_STYLE(Notify, BS_NOTIFY);
    ECK_W_STYLE_MASK(OwnerDraw, BS_OWNERDRAW, TypeMask);
    ECK_W_STYLE_MASK(PushBox, BS_PUSHBOX, TypeMask);
    ECK_W_STYLE_MASK(PushButton, BS_PUSHBUTTON, TypeMask);
    ECK_W_STYLE(PushLike, BS_PUSHLIKE);
    ECK_W_STYLE_MASK(RadioButton, BS_RADIOBUTTON, TypeMask);
    ECK_W_STYLE(AlignmentRight, BS_RIGHT);
    ECK_W_STYLE(RightButton, BS_RIGHTBUTTON);
    ECK_W_STYLE(SplitButton, BS_SPLITBUTTON);
    ECK_W_STYLE(ShowText, BS_TEXT);
    ECK_W_STYLE(AlignmentTop, BS_TOP);
    ECK_W_STYLE(AlignmentVCenter, BS_VCENTER);

    BOOL LoGetIdealSize(LYTSIZE& size) noexcept override
    {
        SIZE sizeInt{ (int)size.cx };
        const auto b = GetIdealSize(&sizeInt);
        size = { (TLytCoord)sizeInt.cx, (TLytCoord)sizeInt.cy };
        return b;
    }

    EckInline BOOL GetIdealSize(_Inout_ SIZE* psize) const noexcept
    {
        return (int)SendMessageW(BCM_GETIDEALSIZE, 0, (LPARAM)psize);
    }

    EckInline BOOL GetImageList(_Out_ BUTTON_IMAGELIST* pbil) const noexcept
    {
        return (BOOL)SendMessageW(BCM_GETIMAGELIST, 0, (LPARAM)pbil);
    }

    ECK_SUPPRESS_MISSING_ZERO_TERMINATION;
    EckInline BOOL GetNote(_Out_writes_(cchBuf) PWSTR pszBuf, _Inout_ UINT& cchBuf) const noexcept
    {
        return (BOOL)SendMessageW(BCM_GETNOTE, (WPARAM)&cchBuf, (LPARAM)pszBuf);
    }

    EckInlineNd UINT GetNoteLength() const noexcept
    {
        return (UINT)SendMessageW(BCM_GETNOTELENGTH, 0, 0);
    }

    BOOL GetNote(Eck_Append_buffer_ CStringW& rs) const noexcept
    {
        auto cch = GetNoteLength();
        if (!cch)
            return FALSE;
        rs.PushBack(cch);
        ++cch;
        return GetNote(rs.Data(), cch);
    }
    // For compatibility
    EckInlineNd CStringW GetNote() const noexcept
    {
        CStringW rs{};
        GetNote(rs);
        return rs;
    }

    EckInline BOOL GetSplitInfomation(_Inout_ BUTTON_SPLITINFO* pbsi) const noexcept
    {
        return (BOOL)SendMessageW(BCM_GETSPLITINFO, 0, (LPARAM)pbsi);
    }

    EckInline BOOL GetTextMargin(_Out_ RECT* prc) const noexcept
    {
        return (BOOL)SendMessageW(BCM_GETTEXTMARGIN, 0, (LPARAM)prc);
    }

    EckInline BOOL SetDropDownState(BOOL bDropDown) const noexcept
    {
        return (BOOL)SendMessageW(BCM_SETDROPDOWNSTATE, bDropDown, 0);
    }

    EckInline BOOL SetImageList(_In_ BUTTON_IMAGELIST* pbil) const noexcept
    {
        return (BOOL)SendMessageW(BCM_SETIMAGELIST, 0, (LPARAM)pbil);
    }

    EckInline BOOL SetNote(_In_z_ PCWSTR pszText) const noexcept
    {
        return (BOOL)SendMessageW(BCM_SETNOTE, 0, (LPARAM)pszText);
    }

    EckInline void SetShieldIcon(BOOL bShieldIcon) const noexcept
    {
        SendMessageW(BCM_SETSHIELD, 0, bShieldIcon);
    }

    EckInline BOOL SetSplitInfomation(_In_ BUTTON_SPLITINFO* pbsi) const noexcept
    {
        return (BOOL)SendMessageW(BCM_SETSPLITINFO, 0, (LPARAM)pbsi);
    }

    EckInline BOOL SetTextMargin(_In_ RECT* prc) const noexcept
    {
        return (BOOL)SendMessageW(BCM_SETTEXTMARGIN, 0, (LPARAM)prc);
    }

    EckInline void Click() const noexcept { SendMessageW(BM_CLICK, 0, 0); }

    EckInline void SetCheckState(int iState) const noexcept { SendMessageW(BM_SETCHECK, iState, 0); }

    EckInline int GetCheckState() const noexcept { return (int)SendMessageW(BM_GETCHECK, 0, 0); }

    EckInline HANDLE GetImage(UINT uType) const noexcept
    {
        return (HANDLE)SendMessageW(BM_GETIMAGE, uType, 0);
    }

    EckInline UINT GetState() const noexcept { return (UINT)SendMessageW(BM_GETSTATE, 0, 0); }

    EckInline void SetDontClick(BOOL bDontClick) const noexcept { SendMessageW(BM_SETDONTCLICK, bDontClick, 0); }

    EckInline HANDLE SetImage(HANDLE hImage, UINT uType) const noexcept
    {
        return (HANDLE)SendMessageW(BM_SETIMAGE, uType, (LPARAM)hImage);
    }

    // 该消息仅能设置按下状态
    EckInline void SetState(BOOL bPressed) const noexcept { SendMessageW(BM_SETSTATE, bPressed, 0); }

    EckInline void SetButtonStyle(DWORD dwStyle, BOOL bRedraw = TRUE) const noexcept
    {
        SendMessageW(BM_SETSTYLE, dwStyle, bRedraw);
    }

    EckInline Type GetButtonType() const noexcept { return Type(GetStyle() & TypeMask); }

    void SetButtonType(Type eType) const noexcept
    {
        SetStyle((GetStyle() & ~TypeMask) | (DWORD)eType);
    }

    BOOL GetButtonDefault() const noexcept
    {
        return !!(GetStyle() &
            (BS_DEFPUSHBUTTON | BS_DEFSPLITBUTTON | BS_DEFCOMMANDLINK));
    }

    void SetButtonDefault(BOOL bDef) const noexcept
    {
        const auto dwStyle = GetStyle();
        auto iType = dwStyle & TypeMask;
        if (iType == BS_PUSHBUTTON || iType == BS_DEFPUSHBUTTON)
            iType = bDef ? BS_DEFPUSHBUTTON : BS_PUSHBUTTON;
        else if (iType == BS_SPLITBUTTON || iType == BS_DEFSPLITBUTTON)
            iType = bDef ? BS_DEFSPLITBUTTON : BS_SPLITBUTTON;
        else if (iType == BS_COMMANDLINK || iType == BS_DEFCOMMANDLINK)
            iType = bDef ? BS_DEFCOMMANDLINK : BS_COMMANDLINK;
        else
            return;
        SetStyle((dwStyle & ~TypeMask) | iType);
    }

    /// 指定图片和文本是否同时显示
    void SetTextImageShowing(BOOL b) const noexcept
    {
        auto dwStyle = GetStyle();
        if (b)
            dwStyle &= ~(BS_BITMAP);
        else if (SendMessageW(BM_GETIMAGE, IMAGE_BITMAP, 0) || SendMessageW(BM_GETIMAGE, IMAGE_ICON, 0))
            dwStyle |= BS_BITMAP;
        SetStyle(dwStyle);
    }

    void SetAlignment(BOOL bHAlign, Alignment eAlign) const noexcept
    {
        auto dwStyle = GetStyle();
        if (bHAlign)
        {
            dwStyle &= (~(BS_LEFT | BS_CENTER | BS_RIGHT));
            switch (eAlign)
            {
            case Alignment::Near:   dwStyle |= BS_LEFT;   break;
            case Alignment::Center: dwStyle |= BS_CENTER; break;
            case Alignment::Far:    dwStyle |= BS_RIGHT;  break;
            }
        }
        else
        {
            dwStyle &= (~(BS_TOP | BS_VCENTER | BS_BOTTOM));
            switch (eAlign)
            {
            case Alignment::Near:   dwStyle |= BS_TOP;     break;
            case Alignment::Center: dwStyle |= BS_VCENTER; break;
            case Alignment::Far:    dwStyle |= BS_BOTTOM;  break;
            }
        }
        SetStyle(dwStyle);
    }

    Alignment GetAlignment(BOOL bHAlign) const noexcept
    {
        auto dwStyle = GetStyle();
        if (bHAlign)
        {
            if (IsBitSet(dwStyle, BS_CENTER))
                return Alignment::Center;
            else if (IsBitSet(dwStyle, BS_RIGHT))
                return Alignment::Far;
            else
                return Alignment::Near;
        }
        else
        {
            if (IsBitSet(dwStyle, BS_VCENTER))
                return Alignment::Center;
            else if (IsBitSet(dwStyle, BS_BOTTOM))
                return Alignment::Far;
            else
                return Alignment::Near;
        }
    }
};

namespace Detail
{
    struct DDXE_CHECKBOX
    {
        using FSetInt = void(*)(void*, int);

        void* pObservable;
        FSetInt pfnSetInt;
    };

    struct DdxFnCheckBox : public CDdxControlCollection<DDXE_CHECKBOX>
    {
        LRESULT operator()(CWindow* pWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, Slot& Ctx)
        {
            if (uMsg == WM_COMMAND && HIWORD(wParam) == BN_CLICKED)
            {
                const auto pExtra = At((HWND)lParam);
                if (pExtra)
                    pExtra->pfnSetInt(
                        pExtra->pObservable,
                        (int)SendMessageW((HWND)lParam, BM_GETCHECK, 0, 0));
            }
            return 0;
        }
    };
}

template<class T>
inline CWindow::HSlot DdxBindCheckBox(CButton& Ctrl, CWindow& Parent, Observable<T>& o) noexcept
{
    o.SetCallback([](const T& v, void* p)
        {
            ((CButton*)p)->SetCheckState((int)v);
        }, &Ctrl);
    return DdxpConnect<Detail::DdxFnCheckBox, MHI_DDX_CHECKBOX>(Ctrl, Parent,
        Detail::DDXE_CHECKBOX{
            &o, [](void* p, int v)
            {
                ((Observable<T>*)p)->Get() = (T)v;
            }
        });
}
template<class T>
inline BOOL DdxUnbindCheckBox(CButton& Ctrl, CWindow& Parent, Observable<T>& o) noexcept
{
    o.ClearCallback();
    return DdxpDisconnect<Detail::DdxFnCheckBox, MHI_DDX_CHECKBOX>(Ctrl, Parent);
}
ECK_NAMESPACE_END