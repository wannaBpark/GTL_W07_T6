#pragma once
#include "Define.h"
#include "HAL/PlatformType.h"


inline FVertexSimple quadVertices[] =
{
    {-1.0f,1.0f,0.0f,0.0f,1.0f,0.0f,1.0f},
    { 1.0f,1.0f,0.0f,0.0f,1.0f,1.0f,1.0f},
    {-1.0f,-1.0f,0.0f,1.0f,1.0f,0.0f,1.0f},
    { 1.0f,-1.0f,0.0f,1.0f,1.0f,1.0f,1.0f}
};

inline uint32 quadIndices[] =
{
    0,1,2,
    1,3,2
};
