#include "Light.h"

using namespace std;

vector<pair<Object*, LightData*>> g_lightDatas;

void Light::OnDetached()
{
	for (auto it = g_lightDatas.begin(); it != g_lightDatas.end();) // Can't use erase_if // dont know why
	{
		if (it->first == m_owner)
		{
			delete it->second;
			it = g_lightDatas.erase(it);
		}
		else ++it;
	}

	IComponent::OnDetached();
}