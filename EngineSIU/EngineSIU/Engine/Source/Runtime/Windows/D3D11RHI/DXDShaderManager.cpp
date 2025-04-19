#include "DXDShaderManager.h"
#include "Define.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <unordered_set>
#include <functional>

FDXDShaderManager::FDXDShaderManager(ID3D11Device* Device)
    : DXDDevice(Device)
{
    VertexShaders.Empty();
    PixelShaders.Empty();
}

FDXDShaderManager::~FDXDShaderManager()
{
    ReleaseAllShader();
}

void FDXDShaderManager::ReleaseAllShader()
{
    for (auto& [Key, Shader] : VertexShaders)
    {
        if (Shader)
        {
            Shader->Release();
            Shader = nullptr;
        }
    }
    VertexShaders.Empty();

    for (auto& [Key, Shader] : PixelShaders)
    {
        if (Shader)
        {
            Shader->Release();
            Shader = nullptr;
        }
    }
    PixelShaders.Empty();

}

// VS & IL / PS Outdated 여부 확인 후 업데이트 
void FDXDShaderManager::UpdateShaderIfOutdated(const std::wstring Key, const std::wstring FilePath, const std::string EntryPoint, bool IsVertexShader, const D3D_SHADER_MACRO* Defines, const D3D11_INPUT_ELEMENT_DESC* Layout, uint32 LayoutSize)
{
    if (!std::filesystem::exists(FilePath)) { return; }

    auto currentTime = std::filesystem::last_write_time(FilePath);
    auto* FoundTime = ShaderTimeStamps.Find(Key);
    if (!FoundTime) { // Key 값에 대한 value 없을 시 새로 만들어 줌
        ShaderTimeStamps.Add(Key, currentTime);
        return;
    }
    //if (*FoundTime == currentTime) // Map에 저장된 마지막 수정 타임이 현재와 똑같을 경우
    //    return;

    if (IsVertexShader)
    {
        ShaderTimeStamps[Key] = currentTime;
        (Defines)
            ? AddVertexShaderAndInputLayout(Key, FilePath, EntryPoint, Layout, LayoutSize, Defines)
            : AddVertexShaderAndInputLayout(Key, FilePath, EntryPoint, Layout, LayoutSize);
    }
    else
    {
        // Pixel Shader Map에 존재한다면 해당 PS 제거
        if (PixelShaders.Contains(Key)) {
            (Defines)
                ? AddPixelShader(Key, FilePath, EntryPoint, Defines)
                : AddPixelShader(Key, FilePath, EntryPoint);
        }
        // Pixel Shader가 아니라면 Compute Shader로 간주
        else
        {
            AddComputeShader(Key, FilePath, EntryPoint);
        }
        ShaderTimeStamps[Key] = currentTime;
    }
    
}

// 리로드 대상이 될 모든 Shader 등록 함수 
void FDXDShaderManager::RegisterShaderForReload(std::wstring Key, std::wstring FilePath, std::string EntryPoint, bool IsVertexShader, D3D_SHADER_MACRO* Defines, D3D11_INPUT_ELEMENT_DESC* Layout, uint32 LayoutSize)
{
    FShaderReloadInfo Info{ Key, FilePath, EntryPoint, IsVertexShader };

    if (Defines)
    {
        while (Defines->Name)
        {
            D3D_SHADER_MACRO CopyMacro;
            CopyMacro.Name = _strdup(Defines->Name); // LPCSTR = char* 버전
            CopyMacro.Definition = _strdup(Defines->Definition);
            Info.Defines.emplace_back(CopyMacro);
            ++Defines;
        }
        Info.Defines.emplace_back(D3D_SHADER_MACRO{ nullptr, nullptr });
    }

    if (Layout && LayoutSize > 0)
    {
        Info.Layout.assign(Layout, Layout + LayoutSize);
    }

    RegisteredShaders.push_back(Info);

    // 메인 셰이더 파일의 타임스탬프를
    if (std::filesystem::exists(FilePath))
    {
        ShaderTimeStamps.Add(Key, std::filesystem::last_write_time(FilePath));
    }
    
    BuildDependency(Info); // 해당 셰이더 파일이 포함하는 셰이더(헤더) 파일을 모두 찾아 graph, date 기록
}

// 모든 리로드 대상 Shader에 대해 업데이트 시도
void FDXDShaderManager::ReloadAllShaders()  
{  
   auto Copied = RegisteredShaders;
   bool bAnyUpdated = false;
   for (const auto& Shader : Copied)
   {  
       if (!IsOutdatedWithDependency(Shader)) { continue; } // 갱신 필요없으면 skip

       const D3D_SHADER_MACRO* definesPtr = Shader.Defines.empty() ? nullptr : Shader.Defines.data();
       const D3D11_INPUT_ELEMENT_DESC* layoutPtr = Shader.Layout.empty() ? nullptr : Shader.Layout.data();
       UpdateShaderIfOutdated(
           Shader.Key,
           Shader.FilePath,
           Shader.EntryPoint,
           Shader.IsVertexShader,
           definesPtr,
           layoutPtr,
           static_cast<uint32>(Shader.Layout.size())
       );
       UE_LOG(LogLevel::Display, TEXT("%ls Updated"), Shader.Key.c_str());

       // 업데이트 후, 해당 셰이더 자신과 인클루드하는 모든 파일 타임스탬프 갱신
       if (std::filesystem::exists(Shader.FilePath))
       {
           ShaderTimeStamps[Shader.Key] = std::filesystem::last_write_time(Shader.FilePath);
       }
       bAnyUpdated = true;
   }

   // 하나라도 업데이트 된 경우, 의존성 그래프에 있는 모든 include 파일들의 타임스탬프를 갱신
   if (bAnyUpdated) { UpdateDependencyTimestamps(); }
}



#ifdef Multi_Shader_Include
void FDXDShaderManager::BuildDependency(const FShaderReloadInfo& Info)
{
    std::wifstream wfile(Info.FilePath);
    if (!wfile.is_open()) return;

    std::wstring line;
    const std::wstring includeToken = L"#include";
    std::unordered_set<std::wstring> uniqueIncludes;

    while (std::getline(wfile, line))
    {
        size_t pos = line.find(includeToken);
        if (pos != std::wstring::npos)
        {
            size_t start = line.find(L"\"", pos);
            size_t end = line.find(L"\"", start + 1);
            if (start != std::wstring::npos && end != std::wstring::npos)
            {
                std::wstring includeFile = line.substr(start + 1, end - start - 1);

                // 중복 방지
                if (!uniqueIncludes.insert(includeFile).second) continue;

                ShaderDependencyGraph[includeFile].Add(Info.Key);

                std::wstring fullPath = L"Shaders/" + includeFile;
                if (std::filesystem::exists(fullPath))
                {
                    auto currentTime = std::filesystem::last_write_time(fullPath);
                    ShaderTimeStamps.Add(includeFile, currentTime);

                    // 중첩 include 탐색 재귀 호출
                    FShaderReloadInfo DummyInfo;
                    DummyInfo.FilePath = fullPath;
                    DummyInfo.Key = Info.Key;
                    BuildDependency(DummyInfo);
                }
            }
        }
    }
}

bool FDXDShaderManager::IsOutdatedWithDependency(const FShaderReloadInfo& Info)
{
    std::unordered_set<std::wstring> Visited;
    std::function<bool(const std::wstring&)> CheckDependency = [&](const std::wstring& includeFile) -> bool
        {
            if (Visited.contains(includeFile)) return false;
            Visited.insert(includeFile);

            std::wstring fullPath = L"Shaders/" + includeFile;
            if (std::filesystem::exists(fullPath))
            {
                auto depTime = std::filesystem::last_write_time(fullPath);
                auto* storedDepTime = ShaderTimeStamps.Find(includeFile);
                if (!storedDepTime || *storedDepTime != depTime)
                {
                    return true;
                }
            }

            // 중첩 의존성 확인
            if (ShaderDependencyGraph.Contains(includeFile))
            {
                for (const auto& key : ShaderDependencyGraph[includeFile])
                {
                    if (key == Info.Key && CheckDependency(includeFile))
                    {
                        return true;
                    }
                }
            }

            return false;
        };

    for (const auto& [includeFile, shaderKeys] : ShaderDependencyGraph)
    {
        if (shaderKeys.Contains(Info.Key))
        {
            if (CheckDependency(includeFile))
            {
                return true;
            }
        }
    }

    // 메인 셰이더 파일 검사
    if (!std::filesystem::exists(Info.FilePath)) { return false; }
    auto currentTime = std::filesystem::last_write_time(Info.FilePath);
    auto* FoundTime = ShaderTimeStamps.Find(Info.Key);
    if (!FoundTime || (*FoundTime != currentTime)) { return true; }

    return false;
}

void FDXDShaderManager::UpdateDependencyTimestamps()
{
    std::unordered_set<std::wstring> Visited;

    std::function<void(const std::wstring&)> UpdateRecursive = [&](const std::wstring& includeFile)
        {
            if (Visited.contains(includeFile)) return;
            Visited.insert(includeFile);

            std::wstring fullPath = L"Shaders/" + includeFile;
            if (std::filesystem::exists(fullPath))
            {
                ShaderTimeStamps[includeFile] = std::filesystem::last_write_time(fullPath);
            }

            // 중첩 include도 재귀적으로 갱신
            if (ShaderDependencyGraph.Contains(includeFile))
            {
                for (const auto& nestedKey : ShaderDependencyGraph[includeFile])
                {
                    UpdateRecursive(includeFile);
                }
            }
        };

    for (const auto& [includeFile, shaderKeys] : ShaderDependencyGraph)
    {
        UpdateRecursive(includeFile);
    }
}
#else
void FDXDShaderManager::BuildDependency(const FShaderReloadInfo& Info)
{
    std::wifstream wfile(Info.FilePath);
    if (!wfile.is_open())
        return;

    std::wstring line;
    const std::wstring includeToken = L"#include";
    while (std::getline(wfile, line))
    {
        size_t pos = line.find(includeToken);
        if (pos != std::wstring::npos)
        {
            // 예를 들어, #include "Common.hlsl" 형태를 찾음
            size_t start = line.find(L"\"", pos);
            size_t end = line.find(L"\"", start + 1);
            if (start != std::wstring::npos && end != std::wstring::npos)
            {
                std::wstring includeFile = line.substr(start + 1, end - start - 1);
                // include 파일 경로가 상대경로라면 처리하는 코드 추가 가능
                // 1. 의존성 그래프에 추가
                // 2. Shader 타임 스탬프에 수정시각 추가
                ShaderDependencyGraph[includeFile].Add(Info.Key);
                if (std::filesystem::exists(L"Shaders/"+includeFile))
                {
                    auto currentTime = std::filesystem::last_write_time(L"Shaders/"+includeFile);
                    ShaderTimeStamps.Add(includeFile, currentTime);
                }
            }
        }
    }
}
bool FDXDShaderManager::IsOutdatedWithDependency(const FShaderReloadInfo& Info)
{
    // 의존성 include 파일들의 수정 시각 검사
    for (const auto& [includeFile, shaderKeys] : ShaderDependencyGraph)
    {
        // 이 셰이더가 include하는 파일이라면 검사
        if (shaderKeys.Contains(Info.Key))
        {
            if (std::filesystem::exists(L"Shaders/"+includeFile))
            {
                auto depTime = std::filesystem::last_write_time(L"Shaders/" + includeFile);
                auto* storedDepTime = ShaderTimeStamps.Find(includeFile);
                // 저장된 타임스탬프가 없거나, 현재 수정 시각과 다르면 => 변경됨
                if (!storedDepTime || *storedDepTime != depTime)
                {
                    return true;
                }
            }
        }
    }

    // 2. 그 후, 메인 셰이더 파일의 수정 시각 검사
    if (!std::filesystem::exists(Info.FilePath)) { return false; }
    auto currentTime = std::filesystem::last_write_time(Info.FilePath);
    auto* FoundTime = ShaderTimeStamps.Find(Info.Key);
    // 만약 메인 파일의 저장된 타임스탬프가 없거나 현재와 다르면 변경된 것으로 처리
    if (!FoundTime || (*FoundTime != currentTime)) { return true; }

    return false;
}

void FDXDShaderManager::UpdateDependencyTimestamps()
{
    // 모든 의존(Include) 파일의 타임스탬프를 갱신
    for (const auto& [includeFile, shaderKeys] : ShaderDependencyGraph)
    {
        std::wstring fullPath = L"Shaders/" + includeFile;
        if (std::filesystem::exists(fullPath))
        {
            ShaderTimeStamps[includeFile] = std::filesystem::last_write_time(fullPath);
        }
    }
}
#endif

HRESULT FDXDShaderManager::AddPixelShader(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint)
{
    UINT shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    shaderFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
    HRESULT hr = S_OK;

    if (DXDDevice == nullptr)
        return S_FALSE;

    ID3DBlob* PsBlob = nullptr;
    ID3DBlob* ErrorBlob = nullptr;
    hr = D3DCompileFromFile(FileName.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, EntryPoint.c_str(), "ps_5_0", shaderFlags, 0, &PsBlob, &ErrorBlob);
    if (FAILED(hr))
    {
        std::string error = (char*)ErrorBlob->GetBufferPointer();
        return hr;
    }

    ID3D11PixelShader* NewPixelShader;
    hr = DXDDevice->CreatePixelShader(PsBlob->GetBufferPointer(), PsBlob->GetBufferSize(), nullptr, &NewPixelShader);
    if (PsBlob)
    {
        PsBlob->Release();
    }
    if (FAILED(hr))
    {
        return hr;
    }

    if (SUCCEEDED(hr) && !PixelShaders.Contains(Key))
    {
        RegisterShaderForReload(Key, FileName, EntryPoint, false, nullptr, nullptr, 0);
    }

    // Pixel Shader Map에 존재한다면 해당 PS 제거
    if (PixelShaders.Contains(Key)) { PixelShaders[Key]->Release();PixelShaders[Key] = nullptr; }

    PixelShaders[Key] = NewPixelShader;
    
    return S_OK;
}

HRESULT FDXDShaderManager::AddPixelShader(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint, const D3D_SHADER_MACRO* defines)
{
	UINT shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	shaderFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	HRESULT hr = S_OK;

	if (DXDDevice == nullptr)
		return S_FALSE;

	ID3DBlob* PsBlob = nullptr;

    // Begin Test
    ID3DBlob* errorBlob = nullptr;
	//hr = D3DCompileFromFile(FileName.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, EntryPoint.c_str(), "ps_5_0", shaderFlags, 0, &PsBlob, nullptr);
	//if (FAILED(hr))
	//	return hr;
    hr = D3DCompileFromFile(FileName.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, EntryPoint.c_str(), "ps_5_0", shaderFlags, 0, &PsBlob, &errorBlob);

    if (FAILED(hr)) {
        if (errorBlob) {
            // 에러 메시지를 문자열로 변환하여 출력
            const char* errorMsg = static_cast<const char*>(errorBlob->GetBufferPointer());
            OutputDebugStringA(errorMsg); // 디버그 창에 출력
            MessageBoxA(NULL, errorMsg, "Shader Compilation Error", MB_OK); // 팝업 창으로 표시
            errorBlob->Release();
        }
        return hr;
    }
    // End Test

	ID3D11PixelShader* NewPixelShader;
	hr = DXDDevice->CreatePixelShader(PsBlob->GetBufferPointer(), PsBlob->GetBufferSize(), nullptr, &NewPixelShader);
	if (PsBlob)
	{
		PsBlob->Release();
	}
	if (FAILED(hr))
		return hr;

    if (SUCCEEDED(hr) && !PixelShaders.Contains(Key))
    {
        RegisterShaderForReload(Key, FileName, EntryPoint, false, const_cast<D3D_SHADER_MACRO*>(defines), nullptr, 0);
    }

    // Pixel Shader Map에 존재한다면 해당 PS 제거
    if (PixelShaders.Contains(Key)) { PixelShaders[Key]->Release();PixelShaders[Key] = nullptr; }

	PixelShaders[Key] = NewPixelShader;
	return S_OK;
}

HRESULT FDXDShaderManager::AddVertexShader(const std::wstring& Key, const std::wstring& FileName)
{
    return E_NOTIMPL;
}

HRESULT FDXDShaderManager::AddVertexShader(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint)
{
    if (DXDDevice == nullptr)
        return S_FALSE;

    HRESULT hr = S_OK;

    ID3DBlob* VertexShaderCSO = nullptr;
    ID3DBlob* ErrorBlob = nullptr;

    hr = D3DCompileFromFile(FileName.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, EntryPoint.c_str(), "vs_5_0", 0, 0, &VertexShaderCSO, &ErrorBlob);
    if (FAILED(hr))
    {
        if (ErrorBlob) {
            OutputDebugStringA((char*)ErrorBlob->GetBufferPointer());
            ErrorBlob->Release();
        }
        return hr;
    }

    ID3D11VertexShader* NewVertexShader;
    hr = DXDDevice->CreateVertexShader(VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), nullptr, &NewVertexShader);
    if (FAILED(hr))
    {
        VertexShaderCSO->Release();
        return hr;
    }

    VertexShaders[Key] = NewVertexShader;

    VertexShaderCSO->Release();
    if (SUCCEEDED(hr) && !VertexShaders.Contains(Key))
    {
        RegisterShaderForReload(Key, FileName, EntryPoint, true, nullptr);
    }

    return S_OK;
}

HRESULT FDXDShaderManager::AddVertexShader(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint, const D3D_SHADER_MACRO* defines)
{
    if (DXDDevice == nullptr)
        return S_FALSE;

    HRESULT hr = S_OK;

    ID3DBlob* VertexShaderCSO = nullptr;
    ID3DBlob* ErrorBlob = nullptr;

    hr = D3DCompileFromFile(FileName.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, EntryPoint.c_str(), "vs_5_0", 0, 0, &VertexShaderCSO, &ErrorBlob);
    if (FAILED(hr))
    {
        if (ErrorBlob) {
            OutputDebugStringA((char*)ErrorBlob->GetBufferPointer());
            ErrorBlob->Release();
        }
        return hr;
    }

    ID3D11VertexShader* NewVertexShader;
    hr = DXDDevice->CreateVertexShader(VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), nullptr, &NewVertexShader);
    if (FAILED(hr))
    {
        VertexShaderCSO->Release();
        return hr;
    }

    VertexShaders[Key] = NewVertexShader;

    VertexShaderCSO->Release();
    if (SUCCEEDED(hr) && !VertexShaders.Contains(Key))
    {
        RegisterShaderForReload(Key, FileName, EntryPoint, true, const_cast<D3D_SHADER_MACRO*>(defines));
    }

    return S_OK;
}

HRESULT FDXDShaderManager::AddInputLayout(const std::wstring& Key, const D3D11_INPUT_ELEMENT_DESC* Layout, uint32_t LayoutSize)
{
    return S_OK;
}

HRESULT FDXDShaderManager::AddComputeShader(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint)
{
    if (DXDDevice == nullptr)
        return S_FALSE;

    HRESULT hr = S_OK;
    ID3DBlob* csBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;

    // --- 컴파일 플래그 설정 ---
    DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    shaderFlags |= D3DCOMPILE_DEBUG;
    shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    hr = D3DCompileFromFile( 
        FileName.c_str(),
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        EntryPoint.c_str(),
        "cs_5_0",
        shaderFlags,
        0,
        &csBlob,
        &errorBlob
    );

    if (FAILED(hr))
    {
        if (errorBlob)
        {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
            UE_LOG(LogLevel::Error, "%s", (char*)errorBlob->GetBufferPointer());
            errorBlob->Release();
        }
        return hr;
    }

    ID3D11ComputeShader* NewComputeShader = nullptr;
    hr = DXDDevice->CreateComputeShader(csBlob->GetBufferPointer(), csBlob->GetBufferSize(), nullptr, &NewComputeShader);
    if (FAILED(hr))
    {
        csBlob->Release();
        return hr;
    }

    if (SUCCEEDED(hr) && !ComputeShaders.Contains(Key))
    {
        RegisterShaderForReload(Key, FileName, EntryPoint, false, nullptr, nullptr, 0);
    }
    ComputeShaders[Key] = NewComputeShader;
    
    csBlob->Release();

    return S_OK;
}

HRESULT FDXDShaderManager::AddVertexShaderAndInputLayout(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint, const D3D11_INPUT_ELEMENT_DESC* Layout, uint32_t LayoutSize)
{
    UINT shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    shaderFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
    if (DXDDevice == nullptr)
        return S_FALSE;

    HRESULT hr = S_OK;

    ID3DBlob* VertexShaderCSO = nullptr;
    ID3DBlob* ErrorBlob = nullptr;

    hr = D3DCompileFromFile(FileName.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, EntryPoint.c_str(), "vs_5_0", shaderFlags, 0, &VertexShaderCSO, &ErrorBlob);
    if (FAILED(hr))
    {
        if (ErrorBlob) {
            OutputDebugStringA((char*)ErrorBlob->GetBufferPointer());
            ErrorBlob->Release();
        }
        return hr;
    }

    ID3D11VertexShader* NewVertexShader;
    hr = DXDDevice->CreateVertexShader(VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), nullptr, &NewVertexShader);
    if (FAILED(hr))
    {
        return hr;
    }

    ID3D11InputLayout* NewInputLayout;
    hr = DXDDevice->CreateInputLayout(Layout, LayoutSize, VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), &NewInputLayout);
    if (FAILED(hr))
    {
        VertexShaderCSO->Release();
        return hr;
    }

    VertexShaderCSO->Release();
    if (SUCCEEDED(hr) && !VertexShaders.Contains(Key))
    {
        RegisterShaderForReload(Key, FileName, EntryPoint, true, nullptr, const_cast<D3D11_INPUT_ELEMENT_DESC*>(Layout), LayoutSize);
    }

    // Vertex Shader Map에 존재한다면 해당 VS, Input Layout 제거
    if (VertexShaders.Contains(Key)) { VertexShaders[Key]->Release(); VertexShaders[Key] = nullptr; }
    if (InputLayouts.Contains(Key)) { InputLayouts[Key]->Release();    InputLayouts[Key] = nullptr; }

    VertexShaders[Key] = NewVertexShader;
    InputLayouts[Key] = NewInputLayout;


    return S_OK;
}

HRESULT FDXDShaderManager::AddVertexShaderAndInputLayout(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint, const D3D11_INPUT_ELEMENT_DESC* Layout, uint32_t LayoutSize, const D3D_SHADER_MACRO* defines)
{
    UINT shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    shaderFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
    if (DXDDevice == nullptr)
        return S_FALSE;

    HRESULT hr = S_OK;

    ID3DBlob* VertexShaderCSO = nullptr;
    ID3DBlob* ErrorBlob = nullptr;

    hr = D3DCompileFromFile(FileName.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, EntryPoint.c_str(), "vs_5_0", shaderFlags, 0, &VertexShaderCSO, &ErrorBlob);
    if (FAILED(hr))
    {
        if (ErrorBlob) {
            OutputDebugStringA((char*)ErrorBlob->GetBufferPointer());
            ErrorBlob->Release();
        }
        return hr;
    }

    ID3D11VertexShader* NewVertexShader;
    hr = DXDDevice->CreateVertexShader(VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), nullptr, &NewVertexShader);
    if (FAILED(hr))
    {
        return hr;
    }

    ID3D11InputLayout* NewInputLayout;
    hr = DXDDevice->CreateInputLayout(Layout, LayoutSize, VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), &NewInputLayout);
    if (FAILED(hr))
    {
        VertexShaderCSO->Release();
        return hr;
    }

    VertexShaderCSO->Release();
    if (SUCCEEDED(hr) && !VertexShaders.Contains(Key))
    {
        RegisterShaderForReload(Key, FileName, EntryPoint, true, const_cast<D3D_SHADER_MACRO*>(defines), const_cast<D3D11_INPUT_ELEMENT_DESC*>(Layout), LayoutSize);
    }

    // Vertex Shader Map에 존재한다면 해당 VS, Input Layout 제거
    if (VertexShaders.Contains(Key)) { VertexShaders[Key]->Release(); VertexShaders[Key] = nullptr; }
    if (InputLayouts.Contains(Key)) { InputLayouts[Key]->Release();    InputLayouts[Key] = nullptr; }

    VertexShaders[Key] = NewVertexShader;
    InputLayouts[Key] = NewInputLayout;
    return S_OK;
}

ID3D11InputLayout* FDXDShaderManager::GetInputLayoutByKey(const std::wstring& Key) const
{
    if (InputLayouts.Contains(Key))
    {
        return *InputLayouts.Find(Key);
    }
    return nullptr;
}

ID3D11VertexShader* FDXDShaderManager::GetVertexShaderByKey(const std::wstring& Key) const
{
    if (VertexShaders.Contains(Key))
    {
        return *VertexShaders.Find(Key);
    }
    return nullptr;
}

ID3D11PixelShader* FDXDShaderManager::GetPixelShaderByKey(const std::wstring& Key) const
{
    if (PixelShaders.Contains(Key))
    {
        return *PixelShaders.Find(Key);
    }
    return nullptr;
}

ID3D11ComputeShader* FDXDShaderManager::GetComputeShaderByKey(const std::wstring& Key)
{
    if (ComputeShaders.Contains(Key))
    {
        return *ComputeShaders.Find(Key);
    }
    return nullptr;
}
