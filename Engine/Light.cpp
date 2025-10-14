#include "Light.h"

using namespace std;

vector<pair<Object*, LightData>> g_lightDatas;

void Light::OnDetached()
{
	auto it = find_if(g_lightDatas.begin(), g_lightDatas.end(), [this](const pair<Object*, LightData>& p) { return p.first == m_owner; });
	if (it != g_lightDatas.end()) g_lightDatas.erase(it);

	IComponent::OnDetached();
}