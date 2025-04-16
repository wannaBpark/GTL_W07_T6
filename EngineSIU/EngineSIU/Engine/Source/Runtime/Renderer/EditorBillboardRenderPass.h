#pragma once

#include "BillboardRenderPass.h"

class FEditorBillboardRenderPass : public FBillboardRenderPass
{
public:
    FEditorBillboardRenderPass();
    virtual ~FEditorBillboardRenderPass() = default;

    virtual void PrepareRender() override;
};
