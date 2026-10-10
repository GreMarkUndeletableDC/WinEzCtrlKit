#pragma once
#include "CUnknown.h"
#include <d2d1effectauthor.h>

ECK_NAMESPACE_BEGIN
namespace UnknownTraits
{
    ECK_DEF_COM_INHERIT(ID2D1DrawTransform, ID2D1Transform);
    ECK_DEF_COM_INHERIT(ID2D1ComputeTransform, ID2D1Transform);
    ECK_DEF_COM_INHERIT(ID2D1AnalysisTransform, ID2D1Transform);
    ECK_DEF_COM_INHERIT(ID2D1SourceTransform, ID2D1Transform);
    ECK_DEF_COM_INHERIT(ID2D1ConcreteTransform, ID2D1Transform);
    ECK_DEF_COM_INHERIT(ID2D1BlendTransform, ID2D1ConcreteTransform);
    ECK_DEF_COM_INHERIT(ID2D1BorderTransform, ID2D1ConcreteTransform);
    ECK_DEF_COM_INHERIT(ID2D1OffsetTransform, ID2D1ConcreteTransform);
    ECK_DEF_COM_INHERIT(ID2D1BoundsAdjustmentTransform, ID2D1ConcreteTransform);
    ECK_DEF_COM_INHERIT(ID2D1Transform, ID2D1TransformNode);
}
ECK_NAMESPACE_END