#include "FBXModel.h"

using namespace std;

std::unordered_map<std::wstring, UINT> g_fbxModelIdMap;
std::vector<std::pair<Object*, FBXModel*>> g_renderFBXModels;
static UINT s_nextFBXModelId = 0;

FBXModel::FBXModel(const std::wstring& modelPath) : m_modelPath(modelPath)
{
	// Generate or get existing model ID
	if (g_fbxModelIdMap.find(modelPath) == g_fbxModelIdMap.end())
	{
		g_fbxModelIdMap[modelPath] = s_nextFBXModelId++;
	}
	m_modelId = g_fbxModelIdMap[modelPath];
}

FBXModel::~FBXModel()
{
	// Component destructor will call OnDetached
}

void FBXModel::OnAttached(Object* owner)
{
	Component::OnAttached(owner);

	if (!m_isLoaded)
	{
		LoadModel();
	}

	if (m_isLoaded)
	{
		g_renderFBXModels.emplace_back(owner, this);
	}
}

void FBXModel::OnDetached()
{
	auto it = remove_if(g_renderFBXModels.begin(), g_renderFBXModels.end(),
		[this](const pair<Object*, FBXModel*>& p) { return p.second == this; });
	g_renderFBXModels.erase(it, g_renderFBXModels.end());

	Component::OnDetached();
}

bool FBXModel::LoadModel()
{
	string pathStr(m_modelPath.begin(), m_modelPath.end());
	m_modelData = FBXFileParser::LoadFBXModel(pathStr);

	m_isLoaded = (m_modelData != nullptr);
	return m_isLoaded;
}