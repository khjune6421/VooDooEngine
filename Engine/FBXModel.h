#pragma once

#include "Component.h"
#include "FBXFileParser.h"
#include "DirectXLib.h"

extern std::unordered_map<std::wstring, UINT> g_fbxModelIdMap;
extern std::vector<std::pair<Object*, class FBXModel*>> g_renderFBXModels;

class FBXModel : public Component
{
private:
	std::unique_ptr<FBXModelData> m_modelData;
	std::wstring m_modelPath;
	UINT m_modelId = 0;
	bool m_isLoaded = false;

public:
	FBXModel(const std::wstring& modelPath);
	virtual ~FBXModel();

	void OnAttached(class Object* owner) override;
	void OnDetached() override;

	bool LoadModel();
	bool IsLoaded() const { return m_isLoaded; }
	const FBXModelData* GetModelData() const { return m_modelData.get(); }
	UINT GetModelId() const { return m_modelId; }
	const std::wstring& GetModelPath() const { return m_modelPath; }
};