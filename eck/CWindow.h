#pragma once
#include "WindowHelper.h"
#include "ILayout.h"
#include "CEventChain.h"
#include "CString.h"

ECK_NAMESPACE_BEGIN
enum class FrameType
{
    None,   // 无边框
    Sunken, // 凹入式
    Raised, // 凸出式
    Flat,   // 浅凹入式
    Box,    // 镜框式
    Single, // 单线边框式
};

enum class ScrollType
{
    None,   // 无
    Horizontal,   // 水平滚动条
    Vertical,   // 垂直滚动条
    Both,   // 水平和垂直滚动条
};

// Create =======================================================

// 生成以ID创建的方法
#define ECK_W_CREATE                                             \
    HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,  \
        int x, int y, int cx, int cy, HWND hParent,              \
        int nId, void* pParam = nullptr) noexcept                \
    {                                                            \
        return Create(pszText, dwStyle, dwExStyle, x, y, cx, cy, \
            hParent, ::eck::DwordToPointer<HMENU>(nId), pParam); \
    }

// 按类名和实例句柄生成创建方法
#define ECK_W_CREATE_CLASS_INST(ClassName, HInst)                \
    ECK_W_CREATE                                                 \
    HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,  \
        int x, int y, int cx, int cy, HWND hParent,              \
        HMENU hMenu, void* pParam = nullptr) noexcept override   \
    {                                                            \
        NativeCreate(dwExStyle, ClassName, pszText, dwStyle,     \
            x, y, cx, cy, hParent, hMenu, HInst, pParam);        \
        return m_hWnd;                                           \
    }

// 按类名生成创建方法
#define ECK_W_CREATE_CLASS(ClassName) ECK_W_CREATE_CLASS_INST(ClassName, nullptr)

// Attach / Detach ==============================================

#define ECK_W_DISABLE_ATTACH                                                    \
    void Attach(HWND hWnd) noexcept override                                    \
    {                                                                           \
        EckBugCheck(BccNotImplemented, L"CWindow::Attach is disabled.");        \
    }                                                                           \
    HWND Detach() noexcept override                                             \
    {                                                                           \
        EckBugCheck(BccNotImplemented, L"CWindow::Detach is disabled.");        \
        return nullptr;                                                         \
    }

#define ECK_W_DISABLE_ATTACHNEW                                                 \
    void AttachNew(HWND hWnd) noexcept override                                 \
    {                                                                           \
        EckBugCheck(BccNotImplemented, L"CWindow::AttachNew is disabled.");     \
    }                                                                           \
    void DetachNew() noexcept override                                          \
    {                                                                           \
        EckBugCheck(BccNotImplemented, L"CWindow::DetachNew is disabled.");     \
    }

#define ECK_W_NONATTACHABLE(Class)          \
    Class() = default;                      \
    ECK_W_DISABLE_ATTACH                    \
    ECK_W_DISABLE_ATTACHNEW

#define ECK_W_NONATTACHABLE_NO_CONS(Class)  \
    ECK_W_DISABLE_ATTACH                    \
    ECK_W_DISABLE_ATTACHNEW

#define ECK_W_ATTACHABLE(Class)             \
    Class() = default;                      \
    Class(HWND hWnd) { m_hWnd = hWnd; }

// Style Get/Set =================================================

#define ECK_W_STYLE_GETSET(Name, Style)                   \
    BOOL StyleGet##Name() const                           \
    {                                                     \
        if constexpr (Style == 0)                         \
            return !GetStyle();                           \
        else                                              \
            return IsBitSet(GetStyle(), Style);           \
    }                                                     \
    void StyleSet##Name(BOOL b) const                     \
    {                                                     \
        ModifyStyle((b ? Style : 0), Style, GWL_STYLE);   \
    }

#define ECK_W_STYLE_GETSET_MASK(Name, Style, Mask)        \
    BOOL StyleGet##Name() const                           \
    {                                                     \
        if constexpr (Style == 0)                         \
            return !(GetStyle() & Mask);                  \
        else                                              \
            return IsBitSet(GetStyle(), Style);           \
    }                                                     \
    void StyleSet##Name(BOOL b) const                     \
    {                                                     \
        SetStyle((GetStyle() & ~Mask) | (b ? Style : 0)); \
    }

#define ECK_W_STYLE(Name, Style)                          \
    ECKPROP(StyleGet##Name, StyleSet##Name) BOOL Name;    \
    ECK_W_STYLE_GETSET(Name, Style)

#define ECK_W_STYLE_MASK(Name, Style, Mask)               \
    ECKPROP(StyleGet##Name, StyleSet##Name) BOOL Name;    \
    ECK_W_STYLE_GETSET_MASK(Name, Style, Mask)

class CWindow;

struct BBMSG
{
    CWindow* pWnd;
    UINT uMsg;
    WPARAM wParam;
    LPARAM lParam;
    LRESULT lResult;
};

EckInline CWindow* CWindowFromHandle(HWND hWnd) noexcept { return PtcCurrent()->WmAt(hWnd); }

class CWindow : public ILayout
{
    friend HHOOK BeginCbtHook(CWindow*, FWindowCreating) noexcept;
public:
    ECK_RTTI(CWindow, ILayout);
protected:
    HWND m_hWnd{};
    WNDPROC m_pfnRealProc{ DefWindowProcW };
    CEventChain<InterceptDelete_T, LRESULT, CWindow*, UINT, WPARAM, LPARAM> m_ec{};
public:
    using HSlot = decltype(m_ec)::HSlot;
protected:
    template<class T>
    EckInline void NmFillHeader(T& nm, UINT uCode) const noexcept
    {
        static_assert(sizeof(T) >= sizeof(NMHDR));
        auto p = (NMHDR*)&nm;
        p->hwndFrom = GetHandle();
        p->code = uCode;
        p->idFrom = GetDlgCtrlID(GetHandle());
    }
    EckInline LRESULT NmSend(auto& nm, HWND hParent) const noexcept
    {
        return ::SendMessageW(hParent, WM_NOTIFY, ((NMHDR*)&nm)->idFrom, (LPARAM)&nm);
    }
    EckInline LRESULT NmSend(auto& nm) const noexcept
    {
        return NmSend(nm, GetParent(GetHandle()));
    }
    EckInline LRESULT NmFillHeaderAndSend(auto& nm, HWND hParent, UINT uCode) const noexcept
    {
        NmFillHeader(nm, uCode);
        return NmSend(nm, hParent);
    }
    EckInline LRESULT NmFillHeaderAndSend(auto& nm, UINT uCode) const noexcept
    {
        NmFillHeader(nm, uCode);
        return NmSend(nm);
    }

    LRESULT DefaultNotifyMessage(HWND hParent, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept
    {
        return CWindowFromHandle(hParent)->OnMessage(uMsg, wParam, lParam);
    }

    EckInline LRESULT CallProcedure(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept
    {
        Slot Ctx{};
        const auto r = m_ec.EmitWithContext(Ctx, this, uMsg, wParam, lParam);
        if (Ctx.IsProcessed())
            return r;
        return OnMessage(uMsg, wParam, lParam);
    }
public:
    ECKPROP_R(GetHandle) HWND Handle;

    static LRESULT CALLBACK EckWindowProcedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept
    {
        const auto pCtx = PtcCurrent();
        const auto e = pCtx->WmAtInternal(hWnd);
        EckAssert(e);

        CWindow* pChild{};
        BOOL bProcessed{};
        switch (uMsg)
        {
        case WM_NOTIFY:
            if (pChild = pCtx->WmAt(((NMHDR*)lParam)->hwndFrom))
            {
                const auto lResult = pChild->OnNotifyMessage(hWnd, uMsg, wParam, lParam, bProcessed);
                if (bProcessed)
                    return lResult;
            }
            break;
        case WM_HSCROLL:
        case WM_VSCROLL:
        case WM_COMMAND:
        case WM_CHARTOITEM:
        case WM_VKEYTOITEM:
        case WM_CTLCOLORMSGBOX:
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORLISTBOX:
        case WM_CTLCOLORBTN:
        case WM_CTLCOLORDLG:
        case WM_CTLCOLORSCROLLBAR:
        case WM_CTLCOLORSTATIC:
            if (pChild = pCtx->WmAt((HWND)lParam))
                goto CallOnNotify;
            break;
        case WM_DRAWITEM:
            if (pChild = pCtx->WmAt(((DRAWITEMSTRUCT*)lParam)->hwndItem))
                goto CallOnNotify;
            break;
        case WM_MEASUREITEM:
            if (pChild = pCtx->WmAt(GetDlgItem(hWnd, ((MEASUREITEMSTRUCT*)lParam)->CtlID)))
                goto CallOnNotify;
            break;
        case WM_DELETEITEM:
            if (pChild = pCtx->WmAt(((DELETEITEMSTRUCT*)lParam)->hwndItem))
                goto CallOnNotify;
            break;
        case WM_COMPAREITEM:
            if (pChild = pCtx->WmAt(((COMPAREITEMSTRUCT*)lParam)->hwndItem))
            {
            CallOnNotify:
                const auto lResult = pChild->OnNotifyMessage(
                    hWnd, uMsg, wParam, lParam, bProcessed);
                if (bProcessed)
                    return lResult;
            }
            break;
        case WM_STYLECHANGED:
        {
            if (wParam == GWL_STYLE)
            {
                const auto* const pss = (STYLESTRUCT*)lParam;
                if (!(pss->styleNew & WS_CHILD) &&
                    (pss->styleOld & WS_CHILD))
                {
                    EckAssert(!pCtx->TwmAt(hWnd).bTopLevel);
                    pCtx->TwmMarkTopLevel(hWnd, TRUE);
                }

                if ((pss->styleNew & WS_CHILD) &&
                    !(pss->styleOld & WS_CHILD))
                {
                    EckAssert(pCtx->TwmAt(hWnd).bTopLevel);
                    pCtx->TwmMarkTopLevel(hWnd, FALSE);
                }
            }
        }
        break;
        case WM_CREATE:
        {
            if (pCtx->bAutoNcDark && pCtx->TwmAt(hWnd).pWnd == e->pWnd)
                EnableWindowNcDarkMode(hWnd, ShouldAppsUseDarkMode());
        }
        break;
        case WM_NCDESTROY:// 窗口生命周期中的最后一个消息，在这里解绑HWND和CWindow，从窗口映射中清除无效内容
        {
            const auto lResult = e->pWnd->CallProcedure(uMsg, wParam, lParam);
            (void)e->pWnd->CWindow::Detach();// 控件类可能不允许拆离，必须使用基类拆离
            return lResult;
        }
        }
        const auto lResult = e->pWnd->CallProcedure(uMsg, wParam, lParam);
        if (e->uBubbleFlags && !(e->uBubbleFlags & BBWM_DEF_NO_BUBBLE))
        {
            if ((e->uBubbleFlags & BBWM_ALL) ||
                ((e->uBubbleFlags & BBWM_INPUT) && IsInputMessage(uMsg)) ||
                ((e->uBubbleFlags & BBWM_NOTIFY) && pChild))
            {
                const auto lNew = e->pWnd->BubbleMessage(uMsg, wParam, lParam, bProcessed);
                if (bProcessed)
                    return lNew;
            }
        }
        return lResult;
    }

    CWindow() = default;
    constexpr CWindow(HWND hWnd) noexcept : m_hWnd{ hWnd } {}
    ECK_DISABLE_COPY_MOVE(CWindow);

    virtual ~CWindow()
    {
#ifdef _DEBUG
        // 对于已添加进映射的窗口，CWindow的生命周期必须在窗口生命周期之内
        if (m_hWnd)
            EckAssert(((PtcCurrent()->WmAt(m_hWnd) == this) ? (!m_hWnd) : TRUE));
#endif // _DEBUG
    }

    // 调用函数前本类不能持有句柄，且新句柄必须未被其他CWindow类持有
    virtual void Attach(HWND hWnd) noexcept
    {
        EckAssert(!m_hWnd);// 当前类必须未持有句柄
        const auto ptc = PtcCurrent();
        EckAssert(!ptc->WmAt(hWnd));// 新句柄必须未被CWindow持有
        m_hWnd = hWnd;
        ptc->WmAdd(hWnd, this, !(GetStyle() & WS_CHILD));
    }
    [[nodiscard]] virtual HWND Detach() noexcept
    {
        HWND hWnd{};
        std::swap(hWnd, m_hWnd);
        const auto ptc = PtcCurrent();
        EckAssert(ptc->WmAt(hWnd) == this);// 检查匹配性
        ptc->WmRemove(hWnd);
        return hWnd;
    }

    EckInline virtual void AttachNew(HWND hWnd) noexcept
    {
        CWindow::Attach(hWnd);
        m_pfnRealProc = eck::SetWindowProcedure(hWnd, EckWindowProcedure);
    }
    EckInline virtual void DetachNew() noexcept
    {
        eck::SetWindowProcedure(Detach(), m_pfnRealProc);
    }

    EckInline HWND NativeCreate(DWORD dwExStyle, PCWSTR pszClass, PCWSTR pszText, DWORD dwStyle,
        int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, HINSTANCE hInst, void* pParam,
        FWindowCreating pfnCreatingProc = nullptr) noexcept
    {
        BeginCbtHook(this, pfnCreatingProc);
        CreateWindowExW(dwExStyle, pszClass, pszText, dwStyle,
            x, y, cx, cy, hParent, hMenu, hInst, pParam);
        EndCbtHook();
        return m_hWnd;
    }

    EckInline HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
        int x, int y, int cx, int cy, HWND hParent, int nId, void* pParam = nullptr) noexcept
    {
        return Create(pszText, dwStyle, dwExStyle, x, y, cx, cy,
            hParent, DwordToPointer<HMENU>(nId), pParam);
    }

    virtual HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
        int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, void* pParam = nullptr) noexcept
    {
        EckBugCheck(BccNotImplemented, L"CWindow::Create not implemented");
        return nullptr;
    }

    virtual LRESULT OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept
    {
        return CallWindowProcW(m_pfnRealProc, Handle, uMsg, wParam, lParam);
    }

    // 返回TRUE禁止派发该消息
    virtual BOOL PreTranslateMessage(const MSG& Msg) noexcept
    {
        return FALSE;
    }

    // 所有者项目系列（WM_XxxITEM）
    // 标准通知系列（WM_COMMAND、WM_NOTIFY）
    // 着色系列（WM_CTLCOLORXxx）
    // 滚动条系列（WM_VSCROLL、WM_HSCROLL）
    virtual LRESULT OnNotifyMessage(HWND hParent, UINT uMsg,
        WPARAM wParam, LPARAM lParam, BOOL& bProcessed) noexcept
    {
        return 0;
    }

    LRESULT BubbleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam,
        _Out_ BOOL& bProcessed) noexcept
    {
        bProcessed = FALSE;
        auto hParent = GetParent(Handle);
        if (!hParent)
            return 0;
        const auto pCtx = PtcCurrent();
        BBMSG Msg{ this, uMsg, wParam, lParam };
        while (hParent)
        {
            const auto pWnd = pCtx->WmAt(hParent);
            if (pWnd && pWnd->SendMessageW(MessageBubble, 0, (LPARAM)&Msg))
            {
                bProcessed = TRUE;
                return Msg.lResult;
            }
            hParent = GetParent(hParent);
        }
        return 0;
    }

    void LoSetPosition(LYTPOINT pt) noexcept override
    {
        SetWindowPos(m_hWnd, nullptr, (int)pt.x, (int)pt.y, 0, 0,
            SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
    }
    void LoSetSize(LYTSIZE size) noexcept override
    {
        SetWindowPos(m_hWnd, nullptr, 0, 0, (int)size.cx, (int)size.cy,
            SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
    }
    void LoSetRect(const LYTRECT& rc) noexcept override
    {
        SetWindowPos(m_hWnd, nullptr, (int)rc.x, (int)rc.y, (int)rc.cx, (int)rc.cy,
            SWP_NOZORDER | SWP_NOACTIVATE);
    }
    LYTPOINT LoGetPosition() noexcept override
    {
        const auto pt{ GetPosition() };
        return { TLytCoord(pt.x), TLytCoord(pt.y) };
    }
    LYTSIZE LoGetSize() noexcept override
    {
        const auto size{ GetSize() };
        return { TLytCoord(size.cx), TLytCoord(size.cy) };
    }
    void LoShow(BOOL bShow) noexcept override { Show(bShow ? SW_SHOW : SW_HIDE); }
    HWND LoGetWindowHandle() noexcept override { return GetHandle(); }

    EckInlineNdCe HWND GetHandle() const noexcept { return m_hWnd; }

    EckInline void FrameChanged() const noexcept
    {
        SetWindowPos(m_hWnd, nullptr, 0, 0, 0, 0,
            SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_FRAMECHANGED);
    }

    EckInline void SetRedraw(BOOL bRedraw) const noexcept
    {
        SendMessageW(WM_SETREDRAW, bRedraw, 0);
    }

    EckInline BOOL Redraw(BOOL bErase = FALSE) const noexcept
    {
        return InvalidateRect(m_hWnd, nullptr, bErase);
    }
    EckInline BOOL Redraw(const RECT& rc, BOOL bErase = FALSE) const noexcept
    {
        return InvalidateRect(m_hWnd, &rc, bErase);
    }

    void SetFrameType(FrameType eType) const noexcept
    {
        DWORD dwStyle = GetStyle() & ~WS_BORDER;
        DWORD dwExStyle = GetExStyle()
            & ~(WS_EX_WINDOWEDGE | WS_EX_STATICEDGE | WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE);

        switch (eType)
        {
        case FrameType::Sunken: dwExStyle |= WS_EX_CLIENTEDGE; break;
        case FrameType::Raised: dwExStyle |= (WS_EX_WINDOWEDGE | WS_EX_DLGMODALFRAME); break;
        case FrameType::Flat:   dwExStyle |= WS_EX_STATICEDGE; break;
        case FrameType::Box:    dwExStyle |= (WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE); break;
        case FrameType::Single: dwStyle |= WS_BORDER; break;
        }

        SetStyle(dwStyle);
        SetExStyle(dwExStyle);
    }
    [[nodiscard]] FrameType GetFrameType() const noexcept
    {
        const DWORD dwStyle = GetStyle();
        const DWORD dwExStyle = GetExStyle();
        if (IsBitSet(dwExStyle, WS_EX_DLGMODALFRAME))
        {
            if (IsBitSet(dwExStyle, WS_EX_CLIENTEDGE))
                return FrameType::Box;
            if (IsBitSet(dwExStyle, WS_EX_WINDOWEDGE))
                return FrameType::Raised;
        }

        if (IsBitSet(dwExStyle, WS_EX_CLIENTEDGE))
            return FrameType::Sunken;
        if (IsBitSet(dwExStyle, WS_EX_STATICEDGE))
            return FrameType::Flat;
        if (IsBitSet(dwStyle, WS_BORDER))
            return FrameType::Single;

        return FrameType::None;
    }

    void SetScrollBar(ScrollType eType) const noexcept
    {
        switch (eType)
        {
        case ScrollType::None:
            ShowScrollBar(m_hWnd, SB_VERT, FALSE);
            ShowScrollBar(m_hWnd, SB_HORZ, FALSE);
            break;
        case ScrollType::Horizontal:
            ShowScrollBar(m_hWnd, SB_VERT, FALSE);
            ShowScrollBar(m_hWnd, SB_HORZ, TRUE);
            break;
        case ScrollType::Vertical:
            ShowScrollBar(m_hWnd, SB_VERT, TRUE);
            ShowScrollBar(m_hWnd, SB_HORZ, FALSE);
            break;
        case ScrollType::Both:
            ShowScrollBar(m_hWnd, SB_VERT, TRUE);
            ShowScrollBar(m_hWnd, SB_HORZ, TRUE);
            break;
        }
    }
    [[nodiscard]] ScrollType GetScrollBar() const noexcept
    {
        const BOOL bVSB = IsBitSet(GetWindowLongPtrW(m_hWnd, GWL_STYLE), WS_VSCROLL);
        const BOOL bHSB = IsBitSet(GetWindowLongPtrW(m_hWnd, GWL_STYLE), WS_HSCROLL);
        if (bVSB)
            return bHSB ? ScrollType::Both : ScrollType::Vertical;
        if (bHSB)
            return ScrollType::Horizontal;
        return ScrollType::None;
    }

    EckInline LRESULT SendMessageW(UINT uMsg, WPARAM wParam, LPARAM lParam) const noexcept
    {
        return ::SendMessageW(m_hWnd, uMsg, wParam, lParam);
    }
    EckInline LRESULT SendMessageA(UINT uMsg, WPARAM wParam, LPARAM lParam) const noexcept
    {
        return ::SendMessageA(m_hWnd, uMsg, wParam, lParam);
    }
    EckInline LRESULT PostMessageW(UINT uMsg, WPARAM wParam, LPARAM lParam) const noexcept
    {
        return ::PostMessageW(m_hWnd, uMsg, wParam, lParam);
    }
    EckInline LRESULT PostMessageA(UINT uMsg, WPARAM wParam, LPARAM lParam) const noexcept
    {
        return ::PostMessageA(m_hWnd, uMsg, wParam, lParam);
    }

    EckInlineNd DWORD GetStyle() const noexcept
    {
        return (DWORD)GetWindowLongPtrW(m_hWnd, GWL_STYLE);
    }
    EckInlineNd DWORD GetExStyle() const noexcept
    {
        return (DWORD)GetWindowLongPtrW(m_hWnd, GWL_EXSTYLE);
    }
    // 返回旧样式
    EckInline DWORD ModifyStyle(DWORD dwNew, DWORD dwMask, int idx = GWL_STYLE) const noexcept
    {
        return ModifyWindowStyle(m_hWnd, dwNew, dwMask, idx);
    }
    EckInline DWORD SetStyle(DWORD dwStyle) const noexcept
    {
        return (DWORD)SetWindowLongPtrW(m_hWnd, GWL_STYLE, dwStyle);
    }
    EckInline DWORD SetExStyle(DWORD dwStyle) const noexcept
    {
        return (DWORD)SetWindowLongPtrW(m_hWnd, GWL_EXSTYLE, dwStyle);
    }

    int GetText(CStringW& rs) const noexcept
    {
        const int cch = GetWindowTextLengthW(m_hWnd);
        if (cch)
            GetWindowTextW(m_hWnd, rs.PushBack(cch), cch + 1);
        return cch;
    }
    // For compatibility.
    EckInlineNd CStringW GetText() const noexcept
    {
        CStringW rs{};
        GetText(rs);
        return rs;
    }
    // 返回复制的字符数
    EckInline int GetText(PWSTR pszBuf, int cchBuf) const noexcept
    {
        return GetWindowTextW(m_hWnd, pszBuf, cchBuf);
    }
    EckInline BOOL SetText(PCWSTR pszText) const noexcept
    {
        return SetWindowTextW(m_hWnd, pszText);
    }

    EckInline HRESULT SetExplorerTheme() const noexcept
    {
        return SetWindowTheme(m_hWnd, L"Explorer", nullptr);
    }
    EckInline HRESULT SetItemsViewTheme() const noexcept
    {
        return SetWindowTheme(m_hWnd, L"ItemsView", nullptr);
    }
    EckInline HRESULT SetTheme(PCWSTR pszAppName, PCWSTR pszSubList = nullptr) const noexcept
    {
        return SetWindowTheme(m_hWnd, pszAppName, pszSubList);
    }

    EckInline BOOL Move(int x, int y, int cx, int cy, BOOL bNoActive = TRUE) const noexcept
    {
        return SetWindowPos(m_hWnd, nullptr, x, y, cx, cy,
            SWP_NOZORDER | (bNoActive ? SWP_NOACTIVATE : 0));
    }

    EckInline BOOL Destroy() const noexcept
    {
        EckAssert(IsWindow(m_hWnd));
        return DestroyWindow(m_hWnd);
    }

    EckInline void SetFont(HFONT hFont, BOOL bRedraw = FALSE) const noexcept
    {
        SendMessageW(WM_SETFONT, (WPARAM)hFont, bRedraw);
    }
    EckInlineNd HFONT GetFont() const noexcept
    {
        return (HFONT)SendMessageW(WM_GETFONT, 0, 0);
    }

    EckInline BOOL Show(int nCmdShow) const noexcept
    {
        return ShowWindow(m_hWnd, nCmdShow);
    }
    EckInline void SetVisibility(BOOL bVisible) const noexcept
    {
        Show(bVisible ? SW_SHOW : SW_HIDE);
    }
    EckInlineNd BOOL IsVisible() const noexcept
    {
        return IsWindowVisible(m_hWnd);
    }

    EckInline BOOL Enable(BOOL bEnable) const noexcept
    {
        return EnableWindow(m_hWnd, bEnable);
    }
    EckInlineNd BOOL IsEnabled() const noexcept
    {
        return IsWindowEnabled(m_hWnd);
    }

    EckInlineNd LONG_PTR GetLong(int i) const noexcept
    {
        return GetWindowLongPtrW(m_hWnd, i);
    }

    EckInlineNd LONG_PTR SetLong(int i, LONG_PTR l) const noexcept
    {
        return SetWindowLongPtrW(m_hWnd, i, l);
    }

    EckInlineNd CStringW GetWindowClass() const noexcept
    {
        CStringW rs(256);
        rs.ReSize(GetClassNameW(GetHandle(), rs.Data(), 256 + 1));
        return rs;
    }

    void SetLeft(int i) const noexcept
    {
        RECT rc;
        GetWindowRect(m_hWnd, &rc);
        ScreenToClient(GetParent(m_hWnd), (POINT*)&rc);
        SetWindowPos(m_hWnd, nullptr, i, rc.top, 0, 0,
            SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
    }
    [[nodiscard]] int GetLeft() const noexcept
    {
        RECT rc;
        GetWindowRect(m_hWnd, &rc);
        ScreenToClient(GetParent(m_hWnd), (POINT*)&rc);
        return rc.left;
    }
    void SetTop(int i) const noexcept
    {
        RECT rc;
        GetWindowRect(m_hWnd, &rc);
        ScreenToClient(GetParent(m_hWnd), (POINT*)&rc);
        SetWindowPos(m_hWnd, nullptr, rc.left, i, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
    }
    [[nodiscard]] int GetTop() const noexcept
    {
        RECT rc;
        GetWindowRect(m_hWnd, &rc);
        ScreenToClient(GetParent(m_hWnd), (POINT*)&rc);
        return rc.top;
    }
    void SetWidth(int i) const noexcept
    {
        RECT rc;
        GetWindowRect(m_hWnd, &rc);
        SetWindowPos(m_hWnd, nullptr, 0, 0, i, rc.bottom - rc.top,
            SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
    }
    [[nodiscard]] int GetWidth() const noexcept
    {
        RECT rc;
        GetWindowRect(m_hWnd, &rc);
        return rc.right - rc.left;
    }
    void SetHeight(int i) const noexcept
    {
        RECT rc;
        GetWindowRect(m_hWnd, &rc);
        SetWindowPos(m_hWnd, nullptr, 0, 0, rc.right - rc.left, i,
            SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
    }
    [[nodiscard]] int GetHeight() const noexcept
    {
        RECT rc;
        GetWindowRect(m_hWnd, &rc);
        return rc.bottom - rc.top;
    }
    EckInline void SetPosition(POINT pt) const noexcept
    {
        SetWindowPos(m_hWnd, nullptr, pt.x, pt.y, 0, 0,
            SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
    }
    [[nodiscard]] POINT GetPosition() const noexcept
    {
        RECT rc;
        GetWindowRect(m_hWnd, &rc);
        ScreenToClient(GetParent(m_hWnd), (POINT*)&rc);
        return { rc.left, rc.top };
    }
    EckInline void SetSize(SIZE sz) const noexcept
    {
        SetWindowPos(m_hWnd, nullptr, 0, 0, sz.cx, sz.cy,
            SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
    }
    EckInlineNd SIZE GetSize() const noexcept
    {
        RECT rc;
        GetWindowRect(m_hWnd, &rc);
        return { rc.right - rc.left, rc.bottom - rc.top };
    }

    EckInline BOOL ScbEnableArrows(int iOp, int iBarType) const noexcept
    {
        return EnableScrollBar(m_hWnd, iBarType, iOp);
    }
    int ScbGetPosition(int iType) const noexcept
    {
        SCROLLINFO si;
        si.cbSize = sizeof(SCROLLINFO);
        si.fMask = SIF_POS;
        GetScrollInfo(m_hWnd, iType, &si);
        return si.nPos;
    }
    int ScbGetTrackPosition(int iType) const noexcept
    {
        SCROLLINFO si;
        si.cbSize = sizeof(SCROLLINFO);
        si.fMask = SIF_TRACKPOS;
        GetScrollInfo(m_hWnd, iType, &si);
        return si.nTrackPos;
    }
    BOOL ScbGetRange(int iType, int* piMin = nullptr, int* piMax = nullptr) const noexcept
    {
        SCROLLINFO si;
        si.cbSize = sizeof(SCROLLINFO);
        si.fMask = SIF_RANGE;
        BOOL b = GetScrollInfo(m_hWnd, iType, &si);
        if (piMin)
            *piMin = si.nMin;
        if (piMax)
            *piMax = si.nMax;
        return b;
    }
    int ScbGetPage(int iType) const noexcept
    {
        SCROLLINFO si;
        si.cbSize = sizeof(SCROLLINFO);
        si.fMask = SIF_PAGE;
        GetScrollInfo(m_hWnd, iType, &si);
        return si.nPage;
    }
    EckInline BOOL ScbGetInfomation(int iType, SCROLLINFO* psi) const noexcept
    {
        psi->cbSize = sizeof(SCROLLINFO);
        return GetScrollInfo(m_hWnd, iType, psi);
    }
    void ScbSetPosition(int iType, int iPos, BOOL bRedraw = TRUE) const noexcept
    {
        SCROLLINFO si;
        si.cbSize = sizeof(SCROLLINFO);
        si.fMask = SIF_POS;
        si.nPos = iPos;
        SetScrollInfo(m_hWnd, iType, &si, bRedraw);
    }
    void ScbSetRange(int iType, int iMin, int iMax, BOOL bRedraw = TRUE) const noexcept
    {
        SCROLLINFO si;
        si.cbSize = sizeof(SCROLLINFO);
        si.fMask = SIF_RANGE;
        SetScrollInfo(m_hWnd, iType, &si, bRedraw);
        si.nMin = iMin;
        si.nMax = iMax;
    }
    void ScbSetMinimum(int iType, int iMin, BOOL bRedraw = TRUE) const noexcept
    {
        SCROLLINFO si;
        si.cbSize = sizeof(SCROLLINFO);
        si.fMask = SIF_RANGE;
        GetScrollInfo(m_hWnd, iType, &si);
        si.nMin = iMin;
        SetScrollInfo(m_hWnd, iType, &si, bRedraw);
    }
    void ScbSetMaximum(int iType, int iMax, BOOL bRedraw = TRUE) const noexcept
    {
        SCROLLINFO si;
        si.cbSize = sizeof(SCROLLINFO);
        si.fMask = SIF_RANGE;
        GetScrollInfo(m_hWnd, iType, &si);
        si.nMax = iMax;
        SetScrollInfo(m_hWnd, iType, &si, bRedraw);
    }
    void ScbSetPage(int iType, int iPage, BOOL bRedraw = TRUE) const noexcept
    {
        SCROLLINFO si;
        si.cbSize = sizeof(SCROLLINFO);
        si.fMask = SIF_PAGE;
        si.nPage = iPage;
        SetScrollInfo(m_hWnd, iType, &si, bRedraw);
    }
    void ScbSetInfomation(int iType, const SCROLLINFO* psi, BOOL bRedraw = TRUE) const noexcept
    {
        SetScrollInfo(m_hWnd, iType, psi, bRedraw);
    }

    EckInlineNd SIZE GetClientSize() const noexcept
    {
        RECT rc;
        GetClientRect(m_hWnd, &rc);
        return { rc.right,rc.bottom };
    }
    EckInlineNd int GetClientWidth() const noexcept { return GetClientSize().cx; }
    EckInlineNd int GetClientHeight() const noexcept { return GetClientSize().cy; }

    EckInlineNd BOOL IsValid() const noexcept
    {
#ifdef _DEBUG
        if (!IsWindow(m_hWnd))
            EckAssert(!m_hWnd);
#endif // _DEBUG
        return !!GetHandle();
    }

    EckInline WNDPROC SetWindowProcedure(WNDPROC pfnWndProc) noexcept
    {
        std::swap(m_pfnRealProc, pfnWndProc);
        return pfnWndProc;
    }

    EckInline void SetTopMost(BOOL bTopMost) const noexcept
    {
        SetWindowPos(m_hWnd, bTopMost ? HWND_TOPMOST : HWND_NOTOPMOST,
            0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }

    EckInlineNdCe auto& GetEventChain() noexcept { return m_ec; }
};

EckInline void AttachDialogItems(
    HWND hDlg,
    size_t cItem,
    _In_reads_(cItem) CWindow* const* pWnd,
    _In_reads_(cItem) const int* iId) noexcept
{
    EckCounter(cItem, i)
        pWnd[i]->AttachNew(GetDlgItem(hDlg, iId[i]));
}
ECK_NAMESPACE_END