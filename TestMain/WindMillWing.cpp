#include "WindMillWing.h"

using namespace std;
using namespace DirectX;

WindMillWing::WindMillWing(Shapes shape) : Object(shape), m_windmillWing(Shapes::WindmillWing)
{
	m_isActive = false;

	m_windmillWing.SetPosition(XMVECTOR{ 0.0f, 0.0f, -2.0f, 1.0f });
	AddChild(&m_windmillWing);
}

void WindMillWing::Update(float deltaTime)
{
	static float ATime = 0.0f; // Accumulated time
	ATime += deltaTime;

	if (m_rotationAngle) SetRotation(XMVECTOR{ 0.0f, 2.0f * ATime, 0.0f, 0.0f });
	else SetRotation(XMVECTOR{ 0.0f, 0.0f, 2.0f * ATime, 0.0f });

	if (GetAsyncKeyState(VK_OEM_PERIOD) & 0x8000 && GetScale().m128_f32[0] <= 3.0f) Scale(XMVECTOR{ 1.0f + deltaTime * 2.0f, 1.0f + deltaTime * 2.0f, 1.0f + deltaTime * 2.0f, 0.0f });
	if (GetAsyncKeyState(VK_OEM_COMMA) & 0x8000 && GetScale().m128_f32[0] >= 0.3f) Scale(XMVECTOR{ 1.0f - deltaTime * 2.0f, 1.0f - deltaTime * 2.0f, 1.0f - deltaTime * 2.0f, 0.0f });
}