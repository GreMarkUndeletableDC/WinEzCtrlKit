#pragma once
#include "UiElement.h"
#include "UiScroll.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
using namespace UiBasic::Declaration;

using IScrollController = UiBasic::IScrollControllerT<float>;

class CElement;

// 元素样式
enum
{
    DES_BLUR_BACK = (1u << 31),	// 模糊背景
    // 元素的内容受周边其他内容影响，若无效区域与元素相交，
    // 则必须更新整个元素，设置时DES_BLURBKG强制设置此样式
    DES_CONTENT_EXPAND = (1u << 30),
    // 元素的内容受周边其他内容影响，确定更新区域时DUI系统发送
    // EWM_QUERY_EXPAND_RECT以获取扩展矩形
    DES_CONTENT_EXPAND_RECT = (1u << 29),
    // DUI系统应当检查当前元素的祖元素，因为它们可能设置了混合器
    DES_PARENT_COMP = (1u << 28),
    // 不允许重绘，一般不使用此样式
    DES_NO_REDRAW = (1u << 27),
    // 对于手动混合元素，DUI不应自行分配后台缓存，
    // 而应调用CCompositor::CreateCacheBitmap
    DES_OWNER_COMP_CACHE = (1u << 26),
    // 指示当前手动混合元素不使用后台缓存，
    // 设置后DUI系统适时调用CCompositor::PreRender
    DES_COMP_NO_REDIRECTION = (1u << 25),
    // 【仅供内部使用】元素的后台缓存内容需要更新。
    // 设置该标志的意图是尽量减少手动混合元素的重渲染（特别是在播放动画时）。
    // 普通元素需要每次重渲染是因为其内容没有保存，但未设置DES_COMP_NO_REDIRECTION
    // 的手动混合标志都具有一副后台位图，内容为上一次渲染结果。
    // DUI系统直接在m_dwStyle字段上操作该位，该位永远不会传递到SetStyle
    DESP_COMP_CONTENT_INVALID = (1u << 24),
    // 指示基类事件处理函数应调用BeginPaint/EndPaint对
    DES_BASE_BEGIN_END_PAINT = (1u << 23),

    // 【仅供内部使用】
    DESP_EXPANDED = DES_CONTENT_EXPAND | DES_CONTENT_EXPAND_RECT,
};

// 元素产生的通知
enum : UINT
{
    ENC_DUI_DUMMY = ENC_SYSBEGIN,

    // Header
    ENC_HD_BEGINDRAG,   // 开始拖拽(EVT_ITEM*)
    ENC_HD_ENDDRAG,     // 结束拖拽(EVT_ITEM*)
    ENC_HD_WIDTHCHANGED,// 宽度改变(EVT_ITEM*)
    ENC_HD_ORDERCHANGED,// 顺序改变(EVT_ORDER*)
    ENC_HD_DELETEITEM,  // 删除项(EVT_ITEM*)

    // Edit
    ENC_ED_TXNOTIFY,    // 来自文本服务的通知(NMEDTXNOTIFY*)

    ENC_PRIVATE_BEGIN = 0x0400
};

enum class PresentMode : BYTE
{
    DCompositionSurface,
    DCompositionVisual,
    BitBltSwapChain,	// 支持透明混合，必须无WS_EX_NRB
    FlipSwapChain,		// 不支持透明混合
    WindowRenderTarget,	// 支持透明混合，必须无WS_EX_NRB
    UpdateLayeredWindow,
};

// 渲染事件代码
enum
{
    // 查询渲染目标。
    // 重画开始前产生此事件，应用程序可选重定向渲染的目标图面
    // Field:
    //   QueryTarget
    // Return:
    //   RER_NONE           执行默认操作
    //   RER_REDIRECTION    应用程序重定向渲染，DUI系统应使用pDstSurface和prcDirty
    RE_QUERY_TARGET,

    // 即将开始渲染
    // Field:
    //   PreRender
    // Return:
    //   RER_NONE           执行默认操作
    //   RER_NO_FILLBACK    执行默认操作，且不产生RE_FILLBACK事件
    //   RER_POST_RENDER    触发RE_POSTRENDER
    RE_PRERENDER,

    // 渲染完毕。
    // 如果应用程序为RE_PRERENDER返回了RER_POST_RENDER，则渲染完毕后产生该事件；
    // 此事件产生时关联图面仍然有效，且DC未调用EndDraw
    // Return:
    //   RER_NONE
    RE_POSTRENDER,

    // 提交显示。
    // 仅呈现模式为DCompositionVisual时产生，应用程序应提交DComposition设备
    // Return:
    //   RER_NONE
    RE_COMMIT,

    // 正在填充背景
    // Field:
    //   FillBack
    // Return:
    //   RER_NONE           执行默认操作
    //   RER_NO_FILLBACK    跳过背景填充
    RE_FILLBACK,
};

// 渲染事件返回值
enum : LRESULT
{
    RER_NONE = 0,
    RER_REDIRECTION = 1 << 0,
    RER_NO_FILLBACK = 1 << 1,
    RER_POST_RENDER = 1 << 2,
};

union RENDER_EVENT
{
    struct
    {
        // 如果应用程序需要，此字段设为渲染到的新图面
        // 应用程序必须将引用计数**+1**
        IDXGISurface1* pDstSurface;
        // 如果应用程序需要，此字段设置为绘图偏移。
        // 仅在pDstSurface不为0时有效
        POINT ptOffset;
        // In: 本次更新的脏矩形
        // Out: 如果应用程序需要，将新的脏矩形写入该结构，仅在pDstSurface不为0时有效
        RECT* prcDirty;
    } QueryTarget;// 均为物理坐标
    struct
    {
        // 本次更新的DXGI图面。在非DComposition模式下此字段为nullptr
        IDXGISurface1* pDstSurface;
        // 本次更新的DComposition图面重画偏移
        POINT ptOffset;
        // 本次更新的脏矩形
        const RECT* prcDirty;
    } PreRender;// 均为物理坐标
    struct
    {
        D2D1_RECT_F rc;
    } FillBack;
};

// 元素事件
enum
{
    ECKPRIV_EWM_PLACEHOLDER = EWM_SYSBEGIN,

    // 颜色主题改变 (BOOL 是否为深色, 0)
    EWM_COLORSCHEMECHANGED,
    // 查询扩展矩形 (_Inout_ D2D1_RECT_F*, 0)
    EWM_QUERY_EXPAND_RECT,
    // 创建缓存位图 (_Inout_ CREATE_CACHE_BITMAP_INFO*, 0)
    // 若成功则应返回TRUE
    EWM_CREATE_CACHE_BITMAP,

    EWM_USERBEGIN,
};

namespace Detail
{
    struct PAINT_EXTRA// WM_PAINT的不透明lParam
    {
        float ox;
        float oy;
        const D2D1_RECT_F* prcClipInClient;
    };
}

struct PAINTINFO
{
    D2D1_RECT_F rcClip;        // 剪裁矩形，相对客户区
    D2D1_RECT_F rcClipInEle;  // 剪裁矩形，相对元素
    float ox;
    float oy;
    BOOLEAN bClip;
};

constexpr inline auto DrawTextLayoutFlags =
D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT | D2D1_DRAW_TEXT_OPTIONS_NO_SNAP;

class CBitmap final
{
private:
    ComPtr<ID2D1Bitmap1> m_pBitmap{};
    D2D1_RECT_F m_rcSource{};
public:
    void SetSourceRect(const D2D1_RECT_F* prc) noexcept
    {
        if (prc)
        {
            m_rcSource = *prc;
#ifdef _DEBUG
            EckAssert(m_rcSource.left >= 0 && m_rcSource.top >= 0);
            if (m_pBitmap.Get())
            {
                const auto size = m_pBitmap->GetSize();
                EckAssert(m_rcSource.right <= size.width);
                EckAssert(m_rcSource.bottom <= size.height);
            }
#endif
        }
        else
            m_rcSource = { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX };
    }

    void Set(ID2D1Bitmap1* p, const D2D1_RECT_F* prc = nullptr) noexcept
    {
        m_pBitmap = p;
        SetSourceRect(prc);
    }
    EckInlineNdCe auto Get() const noexcept { return m_pBitmap.Get(); }

    void Clear() noexcept
    {
        m_pBitmap = nullptr;
        m_rcSource.left = FLT_MAX;
    }

    const D2D1_RECT_F* GetSourceRect() const noexcept
    {
        if (m_rcSource.left == FLT_MAX)
            return nullptr;
        else
            return &m_rcSource;
    }
    D2D1_RECT_F GetActualSourceRect() const noexcept
    {
        if (m_rcSource.left == FLT_MAX)
        {
            const auto size = m_pBitmap->GetSize();
            return { 0, 0, size.width, size.height };
        }
        else
            return m_rcSource;
    }
};

EckInlineNdCe Kw::Vec2& EagPoint(LPARAM lParam) noexcept { return *(Kw::Vec2*)lParam; }
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END