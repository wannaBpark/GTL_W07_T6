#include "Material.h"
#include "UObject/Casts.h"


UObject* UMaterial::Duplicate()
{
    ThisClass* NewMaterial = Cast<ThisClass>(Super::Duplicate());

    NewMaterial->materialInfo = materialInfo;

    return NewMaterial;
}
