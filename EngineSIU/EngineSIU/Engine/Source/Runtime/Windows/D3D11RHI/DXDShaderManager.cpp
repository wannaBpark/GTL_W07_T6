#include "DXDShaderManager.h"


FDXDShaderManager::FDXDShaderManager(ID3D11Device* Device)
    : DXDDevice(Device)
{
    VertexShaders.Empty();
    PixelShaders.Empty();
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
    if (*FoundTime == currentTime) // Map에 저장된 마지막 수정 타임이 현재와 똑같을 경우
        return;

    if (IsVertexShader)
    {
        // Vertex Shader Map에 존재한다면 해당 VS, Input Layout 제거
        if (VertexShaders.Contains(Key)) { VertexShaders[Key]->Release(); VertexShaders[Key] = nullptr; }
        if (InputLayouts.Contains(Key)) { InputLayouts[Key]->Release();    InputLayouts[Key] = nullptr; }

        (Defines)
            ? AddVertexShaderAndInputLayout(Key, FilePath, EntryPoint, Layout, LayoutSize, Defines)
            : AddVertexShaderAndInputLayout(Key, FilePath, EntryPoint, Layout, LayoutSize);
    }
    else
    {
        // Pixel Shader Map에 존재한다면 해당 PS 제거
        if (PixelShaders.Contains(Key)) { 
            PixelShaders[Key]->Release();
            PixelShaders[Key] = nullptr;
        }
        ShaderTimeStamps[Key] = currentTime;
        (Defines)
            ? AddPixelShader(Key, FilePath, EntryPoint, Defines)
            : AddPixelShader(Key, FilePath, EntryPoint);
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
}

// 모든 리로드 대상 Shader에 대해 업데이트 시도
void FDXDShaderManager::ReloadAllShaders()  
{  
   auto Copied = RegisteredShaders;
   for (const auto& Shader : Copied)
   {  
       const D3D_SHADER_MACRO* definesPtr = Shader.Defines.empty() ? nullptr : Shader.Defines.data();
       const D3D11_INPUT_ELEMENT_DESC* layoutPtr = Shader.Layout.empty() ? nullptr : Shader.Layout.data();
       UpdateShaderIfOutdated(
           Shader.Key,
           Shader.FilePath,
           Shader.EntryPoint,
           Shader.IsVertexShader,
           definesPtr,
           layoutPtr,
           static_cast<uint32>(Shader.Layout.size()));
   }
}

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
    hr = D3DCompileFromFile(FileName.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, EntryPoint.c_str(), "ps_5_0", shaderFlags, 0, &PsBlob, nullptr);
    if (FAILED(hr))
        return hr;

    ID3D11PixelShader* NewPixelShader;
    hr = DXDDevice->CreatePixelShader(PsBlob->GetBufferPointer(), PsBlob->GetBufferSize(), nullptr, &NewPixelShader);
    if (PsBlob)
    {
        PsBlob->Release();
    }
    if (FAILED(hr))
        return hr;

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

    return S_OK;
}

HRESULT FDXDShaderManager::AddInputLayout(const std::wstring& Key, const D3D11_INPUT_ELEMENT_DESC* Layout, uint32_t LayoutSize)
{
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

    VertexShaders[Key] = NewVertexShader;
    InputLayouts[Key] = NewInputLayout;

    VertexShaderCSO->Release();

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
