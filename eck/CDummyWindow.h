#pragma once
#include "CWindow.h"

ECK_NAMESPACE_BEGIN
class CDummyWindow : public CWindow
{
public:
    ECK_RTTI(CDummyWindow, CWindow);
    ECK_W_NONATTACHABLE(CDummyWindow);
    ECK_W_CREATE_CLASS_INST(WCN_DUMMY, g_hInstance);
};
ECK_NAMESPACE_END