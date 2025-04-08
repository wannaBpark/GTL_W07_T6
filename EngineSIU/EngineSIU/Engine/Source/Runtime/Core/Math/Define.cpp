#include "Define.h"


//렌더 후처리용 쿼드
FVertexTexture QuadVertices[4]
{
    {-1.f, 1.f, 0.f, 0.f, 0.f},
    {1.f, 1.f, 0.f, 1.f, 0.f },
    {-1.f, -1.f, 0.f, 0.f, 1.f},
    {1.f, -1.f, 0.f, 1.f, 1.f}
};

uint32 QuadIndides[6] = { 0, 1, 2, 2, 1, 3 };
