#include "WindMill.h"

using namespace std;
using namespace DirectX;

WindMill::WindMill(Shapes shape) : Object(shape)
{
	m_windmillWing = make_unique<WindMillWing>(Shapes::Triangle);
	AddChild(m_windmillWing.get());
	m_windmillWing->SetPosition(XMVECTOR{ 0.0f, 1.5f, 0.0f, 1.0f });
}

void WindMill::Update(float deltaTime)
{
	if (m_windmillWing) m_windmillWing->Update(deltaTime);

	XMVECTOR moveDelta = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

	if (GetAsyncKeyState('W') & 0x8000) moveDelta = XMVectorAdd(moveDelta, XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f));
	if (GetAsyncKeyState('S') & 0x8000) moveDelta = XMVectorAdd(moveDelta, XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f));
	if (GetAsyncKeyState('A') & 0x8000) moveDelta = XMVectorAdd(moveDelta, XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f));
	if (GetAsyncKeyState('D') & 0x8000) moveDelta = XMVectorAdd(moveDelta, XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f));

	XMVECTOR normMove = XMVector3Normalize(moveDelta);
	normMove *= m_moveSpeed * deltaTime;
	MovePosition(normMove);

	if (GetAsyncKeyState('Q') & 0x8000) Rotate(XMVECTOR{ 0.0f, -2.0f * deltaTime, 0.0f, 0.0f });
	if (GetAsyncKeyState('E') & 0x8000) Rotate(XMVECTOR{ 0.0f, 2.0f * deltaTime, 0.0f, 0.0f });

	if (GetAsyncKeyState('R') & 0x8000 && GetScale().m128_f32[0] <= 3.0f) Scale(XMVECTOR{ 1.0f + deltaTime * 2.0f, 1.0f + deltaTime * 2.0f, 1.0f + deltaTime * 2.0f, 0.0f });
	if (GetAsyncKeyState('F') & 0x8000 && GetScale().m128_f32[0] >= 0.3f) Scale(XMVECTOR{ 1.0f - deltaTime * 2.0f, 1.0f - deltaTime * 2.0f, 1.0f - deltaTime * 2.0f, 0.0f });
}