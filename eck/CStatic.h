#pragma once
#include "CWindow.h"

ECK_NAMESPACE_BEGIN
inline constexpr DWORD StaticTypeMask = (SS_LEFT | SS_CENTER | SS_RIGHT | SS_ICON |
    SS_BLACKRECT | SS_GRAYRECT | SS_WHITERECT | SS_BLACKFRAME | SS_GRAYFRAME |
    SS_WHITEFRAME | SS_USERITEM | SS_SIMPLE | SS_LEFTNOWORDWRAP | SS_OWNERDRAW |
    SS_BITMAP | SS_ENHMETAFILE | SS_ETCHEDHORZ | SS_ETCHEDVERT | SS_ETCHEDFRAME);

class CStatic : public CWindow
{
public:
    ECK_RTTI(CStatic, CWindow);
    ECK_W_ATTACHABLE(CStatic);
    ECK_W_CREATE_CLASS(WC_STATICW);

    ECK_W_STYLE(ShowBitmap, SS_BITMAP);
    ECK_W_STYLE_MASK(BlackFrame, SS_BLACKFRAME, StaticTypeMask);
    ECK_W_STYLE_MASK(BlackRect, SS_BLACKRECT, StaticTypeMask);
    ECK_W_STYLE_MASK(AlignmentCenter, SS_CENTER, StaticTypeMask);
    ECK_W_STYLE(CenterImage, SS_CENTERIMAGE);
    ECK_W_STYLE(EditControl, SS_EDITCONTROL);
    ECK_W_STYLE(EndEllipsis, SS_ENDELLIPSIS);
    ECK_W_STYLE_MASK(EnhMetaFile, SS_ENHMETAFILE, StaticTypeMask);
    ECK_W_STYLE_MASK(EtchedFrame, SS_ETCHEDHORZ, StaticTypeMask);
    ECK_W_STYLE_MASK(EtchedHorizontal, SS_ETCHEDHORZ, StaticTypeMask);
    ECK_W_STYLE_MASK(EtchedVertical, SS_ETCHEDVERT, StaticTypeMask);
    ECK_W_STYLE_MASK(GrayFrame, SS_GRAYFRAME, StaticTypeMask);
    ECK_W_STYLE_MASK(GrayRect, SS_GRAYRECT, StaticTypeMask);
    ECK_W_STYLE_MASK(Icon, SS_ICON, StaticTypeMask);
    ECK_W_STYLE_MASK(AlignmentLeft, SS_LEFT, StaticTypeMask);
    ECK_W_STYLE_MASK(LeftNoWordWrap, SS_LEFTNOWORDWRAP, StaticTypeMask);
    ECK_W_STYLE(NoPrefix, SS_NOPREFIX);
    ECK_W_STYLE(Notify, SS_NOTIFY);
    ECK_W_STYLE_MASK(OwnerDraw, SS_OWNERDRAW, StaticTypeMask);
    ECK_W_STYLE(PathEllipsis, SS_PATHELLIPSIS);
    ECK_W_STYLE(RealSizeImage, SS_REALSIZEIMAGE);
    ECK_W_STYLE(RealSizeControl, SS_REALSIZECONTROL);
    ECK_W_STYLE_MASK(AlignmentRight, SS_RIGHT, StaticTypeMask);
    ECK_W_STYLE(RightJust, SS_RIGHTJUST);
    ECK_W_STYLE_MASK(Simple, SS_SIMPLE, StaticTypeMask);
    ECK_W_STYLE(Sunken, SS_SUNKEN);
    ECK_W_STYLE_MASK(WhiteFrame, SS_WHITEFRAME, StaticTypeMask);
    ECK_W_STYLE_MASK(WhiteRect, SS_WHITERECT, StaticTypeMask);
    ECK_W_STYLE(WordEllipsis, SS_WORDELLIPSIS);

    EckInline HICON GetIcon() const noexcept
    {
        return (HICON)SendMessageW(STM_GETICON, 0, 0);
    }

    /// <summary>
    /// 取图像
    /// </summary>
    /// <param name="uType">图像类型，IMAGE_常量</param>
    /// <returns></returns>
    EckInline HANDLE GetImage(UINT uType = IMAGE_BITMAP) const noexcept
    {
        return (HANDLE)SendMessageW(STM_GETIMAGE, uType, 0);
    }

    EckInline HICON SetIcon(HICON hIcon) const noexcept
    {
        return (HICON)SendMessageW(STM_SETICON, (WPARAM)hIcon, 0);
    }

    /// <summary>
    /// 置图像
    /// </summary>
    /// <param name="h">图像句柄，含义由uType决定</param>
    /// <param name="uType">图像类型，IMAGE_常量</param>
    /// <returns>先前的图像句柄</returns>
    EckInline HANDLE SetImage(HANDLE h, UINT uType = IMAGE_BITMAP) const noexcept
    {
        return (HANDLE)SendMessageW(STM_SETIMAGE, uType, (LPARAM)h);
    }
};
ECK_NAMESPACE_END