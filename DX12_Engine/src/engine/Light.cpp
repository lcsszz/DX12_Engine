#include "Light.h"

#include "engine/Debug.h"

Light::Light()
	:m_LightType(eLightTypeCount)	// uninitialized
{
}

Light::~Light()
{
}

const XMFLOAT4 & Light::GetColor() const
{
	return m_DiffuseColor;
}

Light::ELightType Light::GetType() const
{
	return m_LightType;
}

float Light::GetIntensity() const
{
	return m_Intensity;
}

float Light::GetRange() const
{
	ASSERT(m_LightType == ePointLight || m_LightType == eSpotLight);
	return m_Range;
}

float Light::GetTheta() const
{
	return m_Theta;
}

float Light::GetInnerCutoff() const
{
	return m_InnerCutoff;
}

float Light::GetOuterCutoff() const
{
	return m_OuterCutoff;
}

float Light::GetQuadratic() const
{
	return m_Quadratic;
}

float Light::GetLinear() const
{
	return m_Linear;
}

float Light::GetConstant() const
{
	return m_Constant;
}

void Light::SetType(const ELightType & i_Type)
{
	m_LightType = i_Type;
}

void Light::SetColor(const XMFLOAT4 & i_Color)
{
	m_DiffuseColor = i_Color;
}

void Light::SetIntensity(float i_Intensity)
{
	m_Intensity = i_Intensity;
}

void Light::SetTheta(float i_Theta)
{
	m_Theta = i_Theta;
}

void Light::SetInnerCutoff(float i_Cutoff)
{
}

void Light::SetOuterCutoff(float i_Cutoff)
{
}

void Light::SetConstant(float i_Constant)
{
	m_Constant = i_Constant;
}

void Light::SetLinear(float i_Linear)
{
	m_Linear = i_Linear;
}

void Light::SetQuadratic(float i_Quadratic)
{
	m_Quadratic = i_Quadratic;
}

void Light::SetRange(float i_Range)
{
	ASSERT(m_LightType == ePointLight || m_LightType == eSpotLight);
	m_Range = i_Range;

	// To do: compute other light params
	m_Constant = 1.f;
	m_Linear = 2.f / m_Range;
	m_Quadratic = 1.f / (m_Range * m_Range);
}

FORCEINLINE void Light::ComputeLightData()
{
	TO_DO;
}
