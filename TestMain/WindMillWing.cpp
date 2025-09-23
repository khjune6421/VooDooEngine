#include "WindMillWing.h"

using namespace std;
using namespace DirectX;

static UINT s_windmillIndex = 0;

WindMillWing::WindMillWing() : Object()
{
	m_isActive = false;

	for (int i = 0; i < 3; ++i)
	{
		m_wing[i] = make_unique<Object>(Shapes::WindmillWing);
		AddChild(m_wing[i].get());

		m_wing[i]->SetPosition(XMVECTOR{ 0.0f, 0.0f, 0.5f * (i + 1), 1.0f});
		m_wing[i]->SetScale(XMVECTOR{ 2.0f / (i + 2), 2.0f / (i + 2), 2.0f / (i + 2), 0.0f });
	}
}

void WindMillWing::Update(float deltaTime)
{
	//for (auto& wing : m_wing) wing->Rotate(XMVECTOR{ 0.0f, 0.0f, XM_PI * deltaTime, 0.0f });
	if (m_wing[0]) m_wing[0]->Rotate(XMVECTOR{ 0.0f, 0.0f, XM_PI * deltaTime, 0.0f });
	if (m_wing[1]) m_wing[1]->Rotate(XMVECTOR{ 0.0f, 0.0f, -XM_PI * deltaTime, 0.0f });
	if (m_wing[2]) m_wing[2]->Rotate(XMVECTOR{ 0.0f, 0.0f, XM_PI * deltaTime, 0.0f });
}