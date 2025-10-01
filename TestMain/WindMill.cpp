#include "WindMill.h"

using namespace std;
using namespace DirectX;

WindMill::WindMill(const std::wstring& shape, UINT wingAmount) : Object({ shape })
{
	m_windmillWing = make_unique<WindMillWing>();
	AddChild(m_windmillWing.get());
	m_windmillWing->MovePosition(XMVECTOR{ 0.0f, 0.0f, 0.0f, 1.0f });
}

void WindMill::Update(float deltaTime)
{
	if (m_windmillWing) m_windmillWing->Update(deltaTime);
}