#include "WindMillWing.h"

using namespace std;
using namespace DirectX;

static UINT s_windmillIndex = 0;

WindMillWing::WindMillWing(UINT wingAmount)
{
	for (UINT i = 0; i < wingAmount; ++i)
	{
		unique_ptr<Object> windmill = make_unique<Object>(L"WindmillWing");
		AddChild(windmill.get());
		windmill->SetPosition(XMVECTOR{ 0.0f, 0.0f, 0.5f * (i + 1), 1.0f });
		windmill->SetScale(XMFLOAT3{ 2.0f / (i + 2), 2.0f / (i + 2), 2.0f / (i + 2) });
		m_wing.emplace_back(move(windmill));
	}
}

void WindMillWing::Update(float deltaTime)
{
	for (UINT i = 0; i < m_wing.size(); ++i)
	{
		if (m_wing[i])
		{
			float direction = (i % 2 == 0) ? 1.0f : -1.0f;
			m_wing[i]->Rotate(XMVECTOR{ 0.0f, 0.0f, direction * XM_PI * deltaTime, 0.0f });
		}
	}
}