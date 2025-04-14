#pragma once
#include "LightComponent.h"

class UAmbientLightComponent : public ULightComponentBase
{

    DECLARE_CLASS(UAmbientLightComponent, ULightComponentBase)
public:
    UAmbientLightComponent();
    virtual ~UAmbientLightComponent() override;
    FORCEINLINE const FAmbientLightInfo& GetAmbientLightInfo() const;
    FORCEINLINE void SetAmbientLightInfo(const FAmbientLightInfo& InAmbient);
private:
    FAmbientLightInfo AmbientLightInfo;
};
