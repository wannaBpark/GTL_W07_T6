#include "PropertyEditor/ShowFlags.h"

ShowFlags& ShowFlags::GetInstance()
{
    static ShowFlags instance;
    return instance;
}

void ShowFlags::Draw(const std::shared_ptr<FEditorViewportClient>& ActiveViewport) const
{
    if (ImGui::Button("Show", ImVec2(60, 32)))
    {
        ImGui::OpenPopup("ShowFlags");
    }

    const char* items[] = { "AABB", "Primitives", "BillBoardText", "UUID", "Fog", "LightWireframe" };
    uint64 curFlag = ActiveViewport->GetShowFlag();

    if (ImGui::BeginPopup("ShowFlags"))
    {
        bool selected[IM_ARRAYSIZE(items)] = 
        {
            static_cast<bool>(curFlag & EEngineShowFlags::SF_AABB),
            static_cast<bool>(curFlag & EEngineShowFlags::SF_Primitives),
            static_cast<bool>(curFlag & EEngineShowFlags::SF_BillboardText),
            static_cast<bool>(curFlag & EEngineShowFlags::SF_UUIDText),
            static_cast<bool>(curFlag & EEngineShowFlags::SF_Fog),
            static_cast<bool>(curFlag & EEngineShowFlags::SF_LightWireframe)
        }; // 각 항목의 체크 상태 저장

        for (int i = 0; i < IM_ARRAYSIZE(items); i++)
        {
            ImGui::Checkbox(items[i], &selected[i]);
        }

        ActiveViewport->SetShowFlag(ConvertSelectionToFlags(selected));
        ImGui::EndPopup();
    }
}

uint64 ShowFlags::ConvertSelectionToFlags(const bool selected[]) const
{
    uint64 flags = EEngineShowFlags::None;

    if (selected[0])
        flags |= static_cast<uint64>(EEngineShowFlags::SF_AABB);
    if (selected[1])
        flags |= static_cast<uint64>(EEngineShowFlags::SF_Primitives);
    if (selected[2])
        flags |= static_cast<uint64>(EEngineShowFlags::SF_BillboardText);
    if (selected[3])
        flags |= static_cast<uint64>(EEngineShowFlags::SF_UUIDText);
    if (selected[4])
        flags |= static_cast<uint64>(EEngineShowFlags::SF_Fog);
    if (selected[5])
        flags |= static_cast<uint64>(EEngineShowFlags::SF_LightWireframe);
    return flags;
}

void ShowFlags::OnResize(HWND hWnd)
{
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    width = clientRect.right - clientRect.left;
    height = clientRect.bottom - clientRect.top;
}