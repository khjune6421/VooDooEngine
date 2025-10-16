#include "Light.h"

using namespace std;
using namespace DirectX;

XMFLOAT4 AmbientLight::s_ambientColor = { 0.25f, 0.25f, 0.25f, 1.0f };
AmbientLight g_defaultAmbientLight(XMFLOAT4{ 0.25f, 0.25f, 0.25f, 1.0f });

vector<PointLight*> g_pointLights;