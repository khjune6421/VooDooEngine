#include "Light.h"

using namespace std;

vector<pair<Object*, LightData*>> g_lightDatas;

void Light::OnDetached()
{
	erase_if(g_lightDatas, [this](const pair<Object*, LightData*>& p) { return p.first == m_owner; });

	IComponent::OnDetached();
}