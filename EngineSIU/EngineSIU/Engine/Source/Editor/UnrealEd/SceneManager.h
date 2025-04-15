#pragma once

#include "HAL/PlatformType.h"
#include "JSON/json.hpp"
#include "World/World.h"


using json = nlohmann::json;

namespace SceneManagerData
{
    // 컴포넌트 하나의 저장 정보를 담는 구조체
    struct FComponentSaveData {
        FString ComponentID;   // 컴포넌트의 고유 ID (액터 내에서 유일해야 함, FName) 
        FString ComponentClass; // 컴포넌트 클래스 이름 (예: "UStaticMeshComponent", "UPointLightComponent")
    

        TMap<FString, FString> Properties; 
    };

    // 액터 하나의 저장 정보를 담는 구조체
    struct FActorSaveData {
        FString ActorID;       // 액터의 고유 ID FName
        FString ActorClass;    // 액터의 클래스 이름 (예: "AStaticMeshActor", "APointLight")
        FString ActorLabel;    // 에디터에서 보이는 이름 (선택적)
        // FTransform ActorTransform; // 액터 자체의 트랜스폼 (보통 루트 컴포넌트가 결정) - 필요 여부 검토

        FString RootComponentID; // 이 액터의 루트 컴포넌트 ID (아래 Components 리스트 내 ID 참조)
        TArray<FComponentSaveData> Components; // 이 액터가 소유한 컴포넌트 목록
    };
    
    struct FSceneData {
        int32 Version = 0;
        int32 NextUUID = 0;
        //TMap<int32, UObject*> Primitives;
    
        TArray<FActorSaveData> Actors; // 씬에 있는 모든 액터 정보
        //TMap<int32, UObject*> Cameras;
    };

    //TODO : 레벨 데이타 구현
}

class SceneManager
{
public:
    // TODO: AssetManager 만들면 Manager에서 필요한 값을 가져오는걸로 바꾸기

    /**
     * Json형식으로 저장된 World파일을 불러옵니다.
     * @param filename Json형식으로 World정보가 저장된 파일의 경로
     * @param world
     * @return 생성된 World
     */
    static void LoadSceneFromJsonFile(const FString& filename, UWorld& world);

    /**
     * World를 Json형식으로 저장합니다.
     * @param FilePath World를 저장할 파일 경로
     * @param World 저장할 World
     * @return 성공적으로 저장되었는지 여부
     */
    static bool SaveSceneToJsonFile(const FString& FilePath, const UWorld& World);

private:
    /**
     * JSON 문자열을 역직렬화하여 FSceneData를 생성합니다.
     *
     * @param JsonString 역직렬화할 JSON 문자열
     * @return 역직렬화된 FSceneData 객체
     */
    static bool JsonToSceneData(const json& j , SceneManagerData::FSceneData& OutSceneData);

    static bool SceneDataToJson(const SceneManagerData::FSceneData& sceneData, json& j);

    /**
     * World를 FSceneData로 직렬화합니다.
     *
     * @param InWorld 직렬화할 World 객체
     * @return World를 표현하는 FSceneData 객체
     */
    static SceneManagerData::FSceneData WorldToSceneData(const UWorld& InWorld);

    
    static bool LoadWorldFromData(const SceneManagerData::FSceneData& sceneData, UWorld* targetWorld);

private:
    // TODO: IFileManager::Get().CreateFileReader() & Writer() 만들면 여기에 추가
    // static void DeserializeFromBinary(const void* Data, int64 Size, SceneManagerData::FSceneData& OutSceneData);
    // or 중간 구조체 사용 안할경우
    // static UWorld* DeserializeFromBinary(const void* Data, int64 Size);

    // static TArray<uint8> SerializeToBinary(const SceneManagerData::FSceneData& SceneData);
    // or 중간 구조체 사용 안할경우
    // static TArray<uint8> SerializeToBinary(UWorld* World);
};
