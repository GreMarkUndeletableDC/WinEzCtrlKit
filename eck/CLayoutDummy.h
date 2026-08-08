#pragma once
#include "CLayoutBase.h"

ECK_NAMESPACE_BEGIN
class CLayoutDummy : public CLayoutBase
{
public:
    ECK_RTTI(CLayoutDummy, CLayoutBase);

    void LoShow(BOOL) noexcept override {}
    size_t LobGetObjectCount() const noexcept override { return 0; }
    void LobUpdateIdealSize() noexcept override {}
    void LobAddObject(const LOB_PARAM& Param) noexcept override {}
};
ECK_NAMESPACE_END