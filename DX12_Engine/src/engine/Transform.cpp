#include "Transform.h"

#include "engine/Utils.h"

Transform::Transform()
	:m_Position(XMLoadFloat3(&XMFLOAT3(0.f, 0.f, 0.f)))
	,m_Rotation(XMLoadFloat3(&XMFLOAT3(0.f, 0.f, 0.f)))
	,m_Scale(XMLoadFloat3(&XMFLOAT3(1.f, 1.f, 1.f)))
	,m_NeedToRecompute(true)
{
}

Transform::Transform(XMFLOAT3 i_Translation, XMFLOAT3 i_Rotation, XMFLOAT3 i_Scale)
	:m_Position(XMLoadFloat3(&i_Translation))
	,m_Rotation(XMLoadFloat3(&i_Rotation))
	,m_Scale(XMLoadFloat3(&i_Scale))
	,m_NeedToRecompute(true)
{
}

Transform::~Transform()
{
}

void Transform::SetPosition(const XMFLOAT3 & i_Position)
{
	m_Position = DirectX::XMLoadFloat3(&i_Position);
	m_NeedToRecompute = true;
}

void Transform::Translate(const XMFLOAT3 & i_Translation)
{
	m_NeedToRecompute = true;
	XMVECTOR translation = DirectX::XMLoadFloat3(&i_Translation);
	m_Position += translation;
}

XMFLOAT3 Transform::GetPosition() const
{
	XMFLOAT3 vRet;
	DirectX::XMStoreFloat3(&vRet, m_Position);
	return vRet;
}

void Transform::SetScale(const XMFLOAT3 & i_Scale)
{
	m_NeedToRecompute = true;
	m_Scale = DirectX::XMLoadFloat3(&i_Scale);
}

XMFLOAT3 Transform::GetScale() const
{
	XMFLOAT3 vRet;
	DirectX::XMStoreFloat3(&vRet, m_Scale);
	return vRet;
}

void Transform::SetRotation(const XMFLOAT3 & i_Rotation)
{
	m_NeedToRecompute = true;
	m_Rotation = DirectX::XMLoadFloat3(&i_Rotation);
}

XMFLOAT3 Transform::GetRotation() const
{
	XMFLOAT3 vRet;
	DirectX::XMStoreFloat3(&vRet, m_Rotation);
	return vRet;
}

XMFLOAT4X4 Transform::GetMatrix()
{
	if (m_NeedToRecompute)
	{
		// recompute matrix here
		RecomputeMatrix();
		m_NeedToRecompute = false;
	}

	return m_CacheMatrix;
}

XMFLOAT4X4 Transform::GetTransposed()
{
	if (m_NeedToRecompute)
	{
		// recompute matrix here
		RecomputeMatrix();
		m_NeedToRecompute = false;
	}

	return m_CacheTransposed;
}

XMFLOAT4 Transform::GetForward()
{
	if (!m_NeedToRecompute)
	{
		// retreive the matrix and the forward vector
		XMFLOAT4X4 mat = GetMatrix();
		return XMFLOAT4(mat._31, mat._32, mat._33, mat._34);
	}

	return XMFLOAT4(
		cos(m_Rotation.m128_f32[1])*cos(m_Rotation.m128_f32[0]),	// x = cos(yaw)*cos(pitch)
		sin(m_Rotation.m128_f32[1])*cos(m_Rotation.m128_f32[0]),	// y = sin(yaw)*cos(pitch)
		sin(m_Rotation.m128_f32[0]),								// z = sin(pitch)
		1.f);
}

Transform & Transform::operator=(const Transform i_Other)
{
	m_Position	= i_Other.m_Position;
	m_Rotation	= i_Other.m_Rotation;
	m_Scale		= i_Other.m_Scale;

	m_NeedToRecompute = true;

	// TODO: insert return statement here
	return *this;
}

// recompute matrix here
inline void Transform::RecomputeMatrix()
{
	// compute rotation matrix
	XMFLOAT3 tRot;
	DirectX::XMStoreFloat3(&tRot, m_Rotation);

	XMMATRIX rotXMat = XMMatrixRotationX(tRot.x * DegToRad);
	XMMATRIX rotYMat = XMMatrixRotationY(tRot.y * DegToRad);
	XMMATRIX rotZMat = XMMatrixRotationZ(tRot.z * DegToRad);

	XMMATRIX rotMat = rotXMat * rotYMat * rotZMat;

	// compute scale matrix
	XMFLOAT3 tScale;
	DirectX::XMStoreFloat3(&tScale, m_Scale);

	XMMATRIX scaleMat = XMMatrixScaling(tScale.x, tScale.y, tScale.z);

	// compute translation matrix
	XMFLOAT3 tPos;
	DirectX::XMStoreFloat3(&tPos, m_Position);

	XMMATRIX posMat = XMMatrixTranslation(tPos.x, tPos.y, tPos.z);

	XMMATRIX mat = scaleMat * rotMat * posMat;

	// compute matrix
	XMStoreFloat4x4(&m_CacheMatrix, mat);
	XMStoreFloat4x4(&m_CacheTransposed, XMMatrixTranspose(mat));
}
