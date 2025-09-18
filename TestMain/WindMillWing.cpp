#include "WindMillWing.h"

using namespace std;
using namespace DirectX;

WindMillWing::WindMillWing(Shapes shape) : Object(shape)
{
	m_isActive = false;

	m_windmillWing = make_unique<Object>(Shapes::WindmillWing);
	m_windmillWing->SetPosition(XMFLOAT3(0.0f, 0.0f, -2.0f));
	AddChild(m_windmillWing.get());
}

void WindMillWing::Update(float deltaTime)
{
	static float ATime = 0.0f; // Accumulated time
	ATime += deltaTime;

	if (m_rotationAngle) SetRotation(XMFLOAT3(0.0f, 2.0f * ATime, 0.0f));
	else SetRotation(XMFLOAT3(0.0f, 0.0f, 2.0f * ATime));

	if (GetAsyncKeyState(VK_OEM_PERIOD) & 0x8000)
	{
		if (GetScale().x <= 3.0f)
		{
			Scale(XMFLOAT3(1.0f + deltaTime * 2.0f, 1.0f + deltaTime * 2.0f, 1.0f + deltaTime * 2.0f));
		}
	}
	if (GetAsyncKeyState(VK_OEM_COMMA) & 0x8000)
	{
		if (GetScale().x >= 0.5f)
		{
			Scale(XMFLOAT3(1.0f - deltaTime * 2.0f, 1.0f - deltaTime * 2.0f, 1.0f - deltaTime * 2.0f));
		}
	}
}