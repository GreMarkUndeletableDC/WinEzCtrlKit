#pragma once
#include "MediaTag.h"

ECK_NAMESPACE_BEGIN
ECK_MEDIATAG_NAMESPACE_BEGIN
class CWav final : public CTag
{
private:

public:
    CWav(CMediaFile& mf) noexcept : CTag{ mf } {}

    Result SimpleGet(SimpleData& mi, const SIMPLE_OPT& Opt) noexcept override
    {

    }

    Result SimpleSet(SimpleData& mi, const SIMPLE_OPT& Opt) noexcept override
    {

    }

    Result ReadTag(UINT uFlags = 0u) noexcept override
    {

    }

    Result WriteTag(UINT uFlags = 0u) noexcept override
    {

    }

    void Reset() noexcept override
    {

    }

    BOOL IsEmpty() noexcept override
    {

    }
};
ECK_MEDIATAG_NAMESPACE_END
ECK_NAMESPACE_END