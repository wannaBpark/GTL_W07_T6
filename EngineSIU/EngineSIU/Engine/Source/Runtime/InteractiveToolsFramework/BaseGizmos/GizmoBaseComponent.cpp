#include "GizmoBaseComponent.h"

#include "GameFramework/Actor.h"
#include "LevelEditor/SLevelEditor.h"
#include "UnrealEd/EditorViewportClient.h"


int UGizmoBaseComponent::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
    int nIntersections = 0;
    if (staticMesh == nullptr) return 0;
    OBJ::FStaticMeshRenderData* renderData = staticMesh->GetRenderData();
    FVertexSimple* vertices = renderData->Vertices.GetData();
    int vCount = renderData->Vertices.Num();
    UINT* indices = renderData->Indices.GetData();
    int iCount = renderData->Indices.Num();

    if (!vertices) return 0;
    BYTE* pbPositions = reinterpret_cast<BYTE*>(renderData->Vertices.GetData());

    int nPrimitives = (!indices) ? (vCount / 3) : (iCount / 3);
    float fNearHitDistance = FLT_MAX;
    for (int i = 0; i < nPrimitives; i++) {
        int idx0, idx1, idx2;
        if (!indices) {
            idx0 = i * 3;
            idx1 = i * 3 + 1;
            idx2 = i * 3 + 2;
        }
        else {
            idx0 = indices[i * 3];
            idx2 = indices[i * 3 + 1];
            idx1 = indices[i * 3 + 2];
        }

        // 각 삼각형의 버텍스 위치를 FVector로 불러옵니다.
        uint32 stride = sizeof(FVertexSimple);
        FVector v0 = *reinterpret_cast<FVector*>(pbPositions + idx0 * stride);
        FVector v1 = *reinterpret_cast<FVector*>(pbPositions + idx1 * stride);
        FVector v2 = *reinterpret_cast<FVector*>(pbPositions + idx2 * stride);

        float fHitDistance;
        if (IntersectRayTriangle(rayOrigin, rayDirection, v0, v1, v2, fHitDistance)) {
            if (fHitDistance < fNearHitDistance) {
                pfNearHitDistance = fNearHitDistance = fHitDistance;
            }
            nIntersections++;
        }

    }
    return nIntersections;
}

void UGizmoBaseComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);

    // TODO: Selec 된 Actor의 크기 기준이 아닌 현재 카메라와 Gizmo의 위치 차이로 Scale 조정하기.
    
    if (GetOwner())
    {
        std::shared_ptr<FEditorViewportClient> ActiveViewport = GEngineLoop.GetLevelEditor()->GetActiveViewportClient();
        if (ActiveViewport->IsPerspective())
        {
            float Scaler = (ActiveViewport->ViewTransformPerspective.GetLocation() - GetOwner()->GetActorLocation()).Length();
            
            Scaler *= 0.1f;
            RelativeScale3D = FVector(Scaler);
        }
        else
        {
            float Scaler = ActiveViewport->orthoSize * 0.1f;
            RelativeScale3D = FVector(Scaler);
        }
    }
}
