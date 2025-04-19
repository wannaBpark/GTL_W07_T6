#include "ConvexVolume.h"

void GetViewFrustumBoundsInternal(FConvexVolume& OutResult, const FMatrix& ViewProjectionMatrix)
{
    
    FPlane Temp;

    // NOTE: Be careful changing anything here! Some callers make assumptions about the order of the planes returned.
    // See for instance BuildLightViewFrustumConvexHull in ShadowSetup.cpp

    // Near clipping plane.
    if (ViewProjectionMatrix.GetFrustumNearPlane(Temp))
    {
        OutResult.Planes[0] = Temp;
    }

    // Left clipping plane.
    if (ViewProjectionMatrix.GetFrustumLeftPlane(Temp))
    {
        OutResult.Planes[1] = Temp;
    }

    // Right clipping plane.
    if (ViewProjectionMatrix.GetFrustumRightPlane(Temp))
    {
        OutResult.Planes[2] = Temp;
    }

    // Top clipping plane.
    if (ViewProjectionMatrix.GetFrustumTopPlane(Temp))
    {
        OutResult.Planes[3] = Temp;
    }

    // Bottom clipping plane.
    if (ViewProjectionMatrix.GetFrustumBottomPlane(Temp))
    {
        OutResult.Planes[4] = Temp;
    }


   if (ViewProjectionMatrix.GetFrustumFarPlane(Temp))
   {
       OutResult.Planes[5] = Temp;
   }

}

void GetViewFrustumBounds(FConvexVolume& OutResult, const FMatrix& ViewProjectionMatrix)
{
    GetViewFrustumBoundsInternal(OutResult, ViewProjectionMatrix);
}
