#pragma once
#include "CNtObject.h"

ECK_NAMESPACE_BEGIN
class CWaitableObject : public CNtObject {};

EckInline NTSTATUS WaitObjectMs(
    const CWaitableObject& Object,
    LONGLONG ms) noexcept
{
    ms *= -10000ll;
    return NtWaitForSingleObject(Object.Get(), FALSE, (LARGE_INTEGER*)&ms);
}

EckInline NTSTATUS WaitObject(
    const CWaitableObject& Object,
    const LARGE_INTEGER* pli = nullptr) noexcept
{
    return NtWaitForSingleObject(Object.Get(), FALSE, (LARGE_INTEGER*)pli);
}
ECK_NAMESPACE_END