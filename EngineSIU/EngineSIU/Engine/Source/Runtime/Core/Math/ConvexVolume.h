#pragma once
#include "Define.h"
#include "Plane.h"
#include "Matrix.h"

class FConvexVolume
{
public:
// 프러스텀을 정의하는 6개의 평면 (Near, Far, Left, Right, Top, Bottom 순서 등)
    FPlane Planes[6];
    // 필요에 따라 추가 정보 (예: 프러스텀 꼭지점)
    

       // 여기에 FBoundingBox와의 교차 검사 함수를 추가합니다.
    /**
     * 이 볼록 볼륨이 주어진 축 정렬 경계 박스(AABB)와 교차하는지 검사합니다.
     *
     * @param Box 검사할 바운딩 박스.
     * @return 박스가 볼륨과 교차하면 true, 그렇지 않으면 false.
     */
    bool Intersects(const FBoundingBox& Box) const
    {
        // 1. 유효하지 않은 박스는 교차하지 않는 것으로 처리 (선택적)
        if (!Box.IsValid())
        {
            return false;
        }

        // 2. 박스의 중심(Center)과 범위(Extent) 계산
        FVector Center = Box.GetCenter();
        FVector Extent = Box.GetExtent();

        // 3. 볼륨을 정의하는 각 평면에 대해 검사
        for (int i = 0; i < 6; ++i)
        {
            const FPlane& P = Planes[i]; // 현재 검사할 평면

            // 4. 박스 중심에서 평면까지의 최대 반경(거리) 계산
            // 평면 법선(P.X, P.Y, P.Z) 방향으로 박스 중심에서 가장 멀리 떨어진 꼭짓점까지의 거리를 계산합니다.
            // Extent의 각 성분에 해당 평면 법선 성분의 절대값을 곱하여 합산합니다.
            float ProjectedRadius = Extent.X * FMath::Abs(P.X) +
                                    Extent.Y * FMath::Abs(P.Y) +
                                    Extent.Z * FMath::Abs(P.Z);

            // 5. 박스 중심점에서 평면까지의 부호 있는 거리 계산
            // PlaneDot은 정규화된 평면 법선을 가정합니다.
            // 결과가 양수이면 중심이 평면의 안쪽(법선 방향), 음수이면 바깥쪽에 있습니다.
            float CenterDistance = P.PlaneDot(Center);

            // 6. 박스가 평면의 완전히 '바깥쪽'에 있는지 확인
            // 중심까지의 거리(CenterDistance)가 음수이고, 그 절대값이 ProjectedRadius보다 크면
            // (즉, CenterDistance < -ProjectedRadius) 박스의 모든 점이 평면의 바깥쪽에 있다는 의미입니다.
            if (CenterDistance < -ProjectedRadius)
            {
                // 이 평면 하나에 대해 완전히 외부에 있으므로, 볼륨과 교차하지 않습니다. 즉시 false 반환.
                return false;
            }
        }

        // 7. 모든 평면에 대해 '완전히 바깥쪽' 조건이 만족되지 않았습니다.
        // 이는 박스가 볼륨 내부에 있거나, 최소한 하나의 평면과 교차(걸쳐 있음)한다는 의미입니다.
        return true;
    }

	/**
	 * 이 볼록 볼륨이 주어진 AABB와 교차하는지, 그리고 완전히 포함하는지 검사합니다. (언리얼 엔진 스타일)
	 *
	 * @param Origin AABB의 중심점.
	 * @param Extent AABB의 범위 (크기의 절반).
	 * @param bOutFullyContained [out] AABB가 볼륨에 완전히 포함되면 true가 설정됩니다.
	 * @return AABB가 볼륨과 교차하면 true, 그렇지 않으면 false.
	 */
	bool Intersects(const FVector& Origin, const FVector& Extent, bool& bOutFullyContained) const
	{
		bOutFullyContained = true; // 일단 완전히 포함된다고 가정

		for (int i = 0; i < 6; ++i)
		{
			const FPlane& P = Planes[i];
			float ProjectedRadius = Extent.X * FMath::Abs(P.X) +
									Extent.Y * FMath::Abs(P.Y) +
									Extent.Z * FMath::Abs(P.Z);
			float CenterDistance = P.PlaneDot(Origin);

			// 완전히 바깥쪽이면 교차하지 않음
			if (CenterDistance < -ProjectedRadius)
			{
				bOutFullyContained = false; // 바깥에 있으니 완전히 포함될 수 없음
				return false; // 교차하지 않음
			}
			// 평면과 걸쳐 있다면(straddling), 완전히 포함된 것은 아님
			// 중심 거리가 양수이더라도 ProjectedRadius보다 작으면 일부가 음수 쪽에 있을 수 있음
			else if (CenterDistance < ProjectedRadius)
			// 아니면 이렇게 검사해도 됨: else if (FMath::Abs(CenterDistance) <= ProjectedRadius)
			{
				bOutFullyContained = false; // 걸쳐 있으니 완전히 포함된 것은 아님
				// 하지만 교차는 하므로 루프는 계속 진행
			}
			// CenterDistance >= ProjectedRadius 인 경우는 박스가 평면의 완전히 안쪽에 있음
		}

		// 루프를 통과했다면 교차하는 것임
		return true;
	}
};


extern void GetViewFrustumBounds(FConvexVolume& OutResult, const FMatrix& ViewProjectionMatrix);
