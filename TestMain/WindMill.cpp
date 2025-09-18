#include "WindMill.h"

using namespace std;
using namespace DirectX;

WindMill::WindMill(Shapes shape) : Object(shape)
{
	m_windmillWing = make_unique<WindMillWing>(Shapes::Triangle);
	AddChild(m_windmillWing.get());
	m_windmillWing->SetPosition(XMFLOAT3(0.0f, 1.5f, 0.0f));
}

void WindMill::Update(float deltaTime)
{
	if (m_windmillWing) m_windmillWing->Update(deltaTime);

	XMFLOAT3 moveDelta = { 0.0f, 0.0f, 0.0f };

	if (GetAsyncKeyState('W') & 0x8000) moveDelta.z += m_moveSpeed * deltaTime;
	if (GetAsyncKeyState('S') & 0x8000) moveDelta.z -= m_moveSpeed * deltaTime;
	if (GetAsyncKeyState('A') & 0x8000) moveDelta.x -= m_moveSpeed * deltaTime;
	if (GetAsyncKeyState('D') & 0x8000) moveDelta.x += m_moveSpeed * deltaTime;

	if (GetAsyncKeyState('Q') & 0x8000) Rotate(XMFLOAT3(0.0f, -2.0f * deltaTime, 0.0f));
	if (GetAsyncKeyState('E') & 0x8000) Rotate(XMFLOAT3(0.0f, 2.0f * deltaTime, 0.0f));

	if (GetAsyncKeyState('R') & 0x8000)
	{
		if (GetScale().x <= 5.0f)
		{
			Scale(XMFLOAT3(1.0f + deltaTime * 2.0f, 1.0f + deltaTime * 2.0f, 1.0f + deltaTime * 2.0f));
		}
	}
	if (GetAsyncKeyState('F') & 0x8000)
	{
		if (GetScale().x >= 0.5f)
		{
			Scale(XMFLOAT3(1.0f - deltaTime * 2.0f, 1.0f - deltaTime * 2.0f, 1.0f - deltaTime * 2.0f));
		}
	}

	MovePosition(moveDelta);
}