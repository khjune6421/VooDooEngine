#include "AnimationObject.h"

using namespace std;

AnimationObject::AnimationObject(const vector<wstring>& shapeNames, const wstring& vertexShader, const wstring& pixelShader) : Object(shapeNames[0], vertexShader, pixelShader)
{
	for (const auto& shapeName : shapeNames)
	{
		if (shapeName == L"None") return;
#ifdef _DEBUG
		if (g_shapeIdMap.find(shapeName) == g_shapeIdMap.end()) MessageBoxW(nullptr, (L"Shape name not found: " + shapeName).c_str(), L"Error", MB_OK);
#endif
		m_shapeIds.emplace_back(g_shapeIdMap[shapeName]);
	}
}