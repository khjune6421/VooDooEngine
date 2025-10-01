#include "Object.h"

using namespace std;
using namespace DirectX;

vector<Object*> g_objects;

static UINT s_idCounter = 0;

DirectX::XMVECTOR Object::QuaternionToEuler(const DirectX::XMVECTOR& quat) const // Do I know how this works? No. Does it work? As far as I know, yes but even if it doesn't how would I know?
{
	XMFLOAT4 fQuat = {};
	XMStoreFloat4(&fQuat, quat);

	// This is to prevent gimbal lock?
	const float test = fQuat.x * fQuat.y + fQuat.z * fQuat.w;
	if (test > 0.499f)
	{
		float yaw = 2.0f * atan2f(fQuat.x, fQuat.w);
		float pitch = XM_PIDIV2;
		float roll = 0.0f;

		return XMVectorSet(roll, pitch, yaw, 0.0f);
	}
	if (test < -0.499f)
	{
		float yaw = -2.0f * atan2f(fQuat.x, fQuat.w);
		float pitch = -XM_PIDIV2;
		float roll = 0.0f;

		return XMVectorSet(roll, pitch, yaw, 0.0f);
	}

	float roll = atan2f
	(
		2.0f * (fQuat.w * fQuat.x + fQuat.y * fQuat.z),
		1.0f - 2.0f * (fQuat.x * fQuat.x + fQuat.y * fQuat.y)
	);
	float pitch = asinf(2.0f * (fQuat.w * fQuat.y - fQuat.z * fQuat.x));
	float yaw = atan2f
	(
		2.0f * (fQuat.w * fQuat.z + fQuat.x * fQuat.y),
		1.0f - 2.0f * (fQuat.y * fQuat.y + fQuat.z * fQuat.z)
	);

	return XMVectorSet(roll, pitch, yaw, 0.0f);
}

void Object::SetDirty()
{
	if (m_isDirty) return;

	m_isDirty = true;
	for (auto& child : m_childrens) child->SetDirty();
}

void Object::AddChild(Object* child)
{
	if (!child) return;

	if (child->m_parent) child->m_parent->RemoveChild(child);

	child->m_parent = this;
	m_childrens.emplace_back(child);

	SetDirty();
}

void Object::AddChildViaWorldPosition(Object* child)
{
	if (!child) return;

	const XMMATRIX inverseParentWorld = XMMatrixInverse(nullptr, GetWorldMatrix());
	const XMMATRIX childWorld = child->GetWorldMatrix();
	const XMMATRIX newLocal = childWorld * inverseParentWorld;

	XMVECTOR scale, rotationQuat, translation;
	XMMatrixDecompose(&scale, &rotationQuat, &translation, newLocal);

	child->SetPosition(translation);
	child->Rotate(-m_rotation); // To negate parent's rotation
	child->Scale(XMFLOAT3(1.0f / m_scale.x, 1.0f / m_scale.y, 1.0f / m_scale.z)); // To negate parent's scale

	AddChild(child);
}

void Object::RemoveChild(Object* child)
{
	if (!child) return;

	auto it = find(m_childrens.begin(), m_childrens.end(), child);
	if (it != m_childrens.end())
	{
		m_childrens.erase(it);
		child->m_parent = nullptr;
	}
}

Object::Object(const vector<wstring>& shapeNames, VertexShaders vertexShader, PixelShaders pixelShader)
	:
	m_vertexShader(vertexShader),
	m_pixelShader(pixelShader),
	m_id(s_idCounter++)
{
	for (const auto& shapeName : shapeNames)
	{
		if (shapeName == L"None") return;
		else if (g_shapeIdMap.find(shapeName) == g_shapeIdMap.end()) MessageBoxW(nullptr, (L"Shape name not found: " + shapeName).c_str(), L"Error", MB_OK);
		else m_shapeIds.emplace_back(g_shapeIdMap[shapeName]);
	}
	g_objects.emplace_back(this);

#ifdef _DEBUG
	cout << "Object created. ID: " << m_id << endl;
#endif
}

Object::~Object()
{
	auto it = find(g_objects.begin(), g_objects.end(), this);
	if (it != g_objects.end()) g_objects.erase(it);

#ifdef _DEBUG
	cout << "Object destroyed. ID: " << m_id << endl;
#endif
}

void Object::SetPosition(const XMVECTOR& pos)
{
	m_position = pos;
	m_positionMatrix = XMMatrixTranslationFromVector(m_position);

	if (!m_isDirty) SetDirty();
}
void Object::MovePosition(const XMVECTOR& delta)
{
	m_position += delta;
	m_positionMatrix = XMMatrixTranslationFromVector(m_position);

	if (!m_isDirty) SetDirty();
}
void Object::MoveDirection(Directions dir, float distance)
{
	const XMVECTOR forward = XMVector3Normalize(XMVector3TransformNormal(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), m_rotationMatrix));
	const XMVECTOR right = XMVector3Normalize(XMVector3TransformNormal(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), m_rotationMatrix));
	const XMVECTOR up = XMVector3Normalize(XMVector3TransformNormal(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), m_rotationMatrix));

	XMVECTOR moveDelta = XMVectorZero();

	switch (dir)
	{
	case Directions::Forward: moveDelta = XMVectorScale(forward, distance); break;
	case Directions::Backward: moveDelta = XMVectorScale(forward, -distance); break;

	case Directions::Right: moveDelta = XMVectorScale(right, distance); break;
	case Directions::Left: moveDelta = XMVectorScale(right, -distance); break;

	case Directions::Up: moveDelta = XMVectorScale(up, distance); break;
	case Directions::Down: moveDelta = XMVectorScale(up, -distance); break;

	default: break;
	}

	MovePosition(moveDelta);

	if (!m_isDirty) SetDirty();
}
void Object::LerpPosition(const XMVECTOR& start, const XMVECTOR& end, float t)
{
	m_position = XMVectorLerp(start, end, t);
	m_positionMatrix = XMMatrixTranslationFromVector(m_position);

	if (!m_isDirty) SetDirty();
}
XMVECTOR Object::GetWorldPosition() const
{
	if (m_isDirty) GetWorldMatrix();

	return XMVectorSet(m_worldMatrix.r[3].m128_f32[0], m_worldMatrix.r[3].m128_f32[1], m_worldMatrix.r[3].m128_f32[2], 1.0f);
}

void Object::SetRotation(const XMVECTOR& rot)
{
	m_rotation = rot;
	m_rotationMatrix = XMMatrixRotationRollPitchYawFromVector(m_rotation);

	if (!m_isDirty) SetDirty();
}
void Object::Rotate(const XMVECTOR& delta)
{
	m_rotation += delta;
	m_rotationMatrix = XMMatrixRotationRollPitchYawFromVector(m_rotation);

	if (!m_isDirty) SetDirty();
}
void Object::LookAt(const XMVECTOR& target) // I have no idea how the hell this works
{
	XMVECTOR toTarget = XMVectorSubtract(target, GetWorldPosition());
	if (XMVector3LessOrEqual(XMVector3LengthSq(toTarget), XMVectorReplicate(1e-8f))) return; // To avoid NaN errors?

	XMVECTOR direction = XMVector3Normalize(toTarget);

	const float dirX = XMVectorGetX(direction);
	const float dirY = XMVectorGetY(direction);
	const float dirZ = XMVectorGetZ(direction);

	float pitch = asinf(clamp(-dirY, -1.0f, 1.0f));
	float yaw = atan2f(dirX, dirZ);
	float roll = 0.0f;

	m_rotation = XMVectorSet(pitch, yaw, roll, 0.0f);
	m_rotationMatrix = XMMatrixRotationRollPitchYawFromVector(m_rotation);

	if (!m_isDirty) SetDirty();
}
XMVECTOR Object::GetWorldRotation() const
{
	if (m_isDirty) GetWorldMatrix();

	XMVECTOR scale, rotationQuat, translation;
	XMMatrixDecompose(&scale, &rotationQuat, &translation, m_worldMatrix);

	return QuaternionToEuler(rotationQuat);
}
XMVECTOR Object::GetWorldDirection(Directions dir) const // Returns normalized vector
{
	if (m_isDirty) GetWorldMatrix();
	XMVECTOR direction = XMVectorZero();

	switch (dir)
	{
	case Directions::Forward: direction = XMVector3Normalize(m_worldMatrix.r[2]); break;
	case Directions::Backward: direction = XMVector3Normalize(-m_worldMatrix.r[2]); break;

	case Directions::Right: direction = XMVector3Normalize(m_worldMatrix.r[0]); break;
	case Directions::Left: direction = XMVector3Normalize(-m_worldMatrix.r[0]); break;

	case Directions::Up: direction = XMVector3Normalize(m_worldMatrix.r[1]); break;
	case Directions::Down: direction = XMVector3Normalize(-m_worldMatrix.r[1]); break;

	default: break;
	}

	return direction;
}

void Object::SetScale(const XMFLOAT3& scl)
{
	m_scale = scl;
	m_scaleMatrix = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);

	if (!m_isDirty) SetDirty();
}
void Object::Scale(const XMFLOAT3& factor)
{
	m_scale.x *= factor.x;
	m_scale.y *= factor.y;
	m_scale.z *= factor.z;
	m_scaleMatrix = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);

	if (!m_isDirty) SetDirty();
}

XMFLOAT3 Object::GetWorldScale() const
{
	if (m_isDirty) GetWorldMatrix();

	XMVECTOR scale;
	XMVECTOR rotationQuat;
	XMVECTOR translation;
	XMMatrixDecompose(&scale, &rotationQuat, &translation, m_worldMatrix);

	XMFLOAT3 scl = {};
	XMStoreFloat3(&scl, scale);

	return scl;
}

XMMATRIX Object::GetWorldMatrix() const
{
	if (m_isDirty)
	{
		m_worldMatrix = m_scaleMatrix * m_rotationMatrix * m_positionMatrix;

		if (m_parent)
		{
			if (!m_ignorePosition && !m_ignoreRotation && !m_ignoreScale)
			{
				if (m_parent->m_isDirty) m_worldMatrix *= m_parent->GetWorldMatrix();
				else m_worldMatrix *= m_parent->m_worldMatrix;
			}
			else
			{
				XMMATRIX parentMatrix = m_parent->GetWorldMatrix();

				if (m_ignorePosition)
				{
					if (!(m_ignorePosition % IgnoreParentAxis::X)) parentMatrix.r[3].m128_f32[0] = 0.0f;
					if (!(m_ignorePosition % IgnoreParentAxis::Y)) parentMatrix.r[3].m128_f32[1] = 0.0f;
					if (!(m_ignorePosition % IgnoreParentAxis::Z)) parentMatrix.r[3].m128_f32[2] = 0.0f;
				}
				if (m_ignoreRotation)
				{
					// not sure how I should implement this
				}
				if (m_ignoreScale)
				{
					if (!(m_ignoreScale % IgnoreParentAxis::X)) parentMatrix.r[0].m128_f32[0] = 1.0f;
					if (!(m_ignoreScale % IgnoreParentAxis::Y)) parentMatrix.r[1].m128_f32[1] = 1.0f;
					if (!(m_ignoreScale % IgnoreParentAxis::Z)) parentMatrix.r[2].m128_f32[2] = 1.0f;
				}

				m_worldMatrix *= parentMatrix;
			}
		}

		m_isDirty = false;
	}

	return m_worldMatrix;
}