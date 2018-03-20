#include "LightComponent.h"

#include "engine/Utils.h"

LightComponent::LightComponent(const LightDesc & i_Desc, Actor * i_Actor)
	:ActorComponent(i_Actor, "Light Component")
{
	// generate light
	m_Light = new Light;

	// create default structure
	m_Light->SetType(i_Desc.Type);
	m_Light->SetColor(i_Desc.Color);
	m_Light->SetRange(i_Desc.Range);
	m_Light->SetLinear(i_Desc.Linear);
	m_Light->SetConstant(i_Desc.Constant);
	m_Light->SetQuadratic(i_Desc.Quadratic);
	m_Light->SetInnerCutoff(cos(i_Desc.InnerCutoff * DegToRad));
	m_Light->SetOuterCutoff(cos(i_Desc.InnerCutoff * DegToRad));
	m_Light->SetTheta(cos(i_Desc.Theta * DegToRad));
}

LightComponent::LightComponent(Actor * i_Actor)
	:ActorComponent(i_Actor, "Light Component")
{
	// generate light
	m_Light = new Light;

	// create default structure
	m_Light->SetType(Light::ePointLight);
	m_Light->SetColor(XMFLOAT4(1.f, 1.f, 1.f, 1.f));
	m_Light->SetRange(10.f);
	m_Light->SetLinear(0.35f);
	m_Light->SetConstant(1.f);
	m_Light->SetQuadratic(0.44f);
	m_Light->SetInnerCutoff(cos(12.5f * DegToRad));
	m_Light->SetOuterCutoff(cos(12.5f * DegToRad));
	m_Light->SetTheta(cos(50.f * DegToRad));
}

LightComponent::~LightComponent()
{
}

Light * LightComponent::GetLight() const
{
	return m_Light;
}

Light::ELightType LightComponent::GetLightType() const
{
	return m_Light->GetType();
}

#ifdef WITH_EDITOR
#include "ui/UI.h"
#include "engine/Utils.h"

// helper
#define IMGUI_SLIDER_MODIFIER(label, getVal, setVal, min, max, prec, pow)	\
{																			\
	float range = getVal();													\
	ImGui::SliderFloat(label, &range, min, max, prec, pow);					\
	if (getVal() != range)	setVal(range);									\
}

void LightComponent::DrawUIComponentInternal()
{
	static int selectedType = (int)GetLightType();
	static const char * typesName[] = { "Point Light", "Spot Light", "Directionnal" };
	DirectX::XMFLOAT4 lightColor = m_Light->GetColor();
	float color[3] = { lightColor.x,lightColor.y, lightColor.z };

	// type of the color
	ImGui::Combo("Type", &selectedType, typesName, Light::ELightType::eLightTypeCount);
	
	// light global data
	// color management
	ImGui::ColorEdit3("Color", color);
	if (color[0] != lightColor.x || color[1] != lightColor.y || color[2] != lightColor.z)
	{
		lightColor.x = color[0];
		lightColor.y = color[1];
		lightColor.z = color[2];
		m_Light->SetColor(lightColor);
	}

	if (selectedType != m_Light->GetType())
	{
		switch (selectedType)
		{
		case Light::ELightType::ePointLight:	m_Light->SetType(Light::ePointLight);		break;
		case Light::ELightType::eSpotLight:		m_Light->SetType(Light::eSpotLight);		break;
		}
	}

	switch (selectedType)
	{
	case 0:		// point light
		IMGUI_SLIDER_MODIFIER("Range", m_Light->GetRange, m_Light->SetRange, 0.0f, 100.f, "%.2f", 1.f);
		IMGUI_SLIDER_MODIFIER("Constant", m_Light->GetConstant, m_Light->SetConstant, 0.0f, 2.f, "%.2f", 1.f);
		IMGUI_SLIDER_MODIFIER("Quadtratic", m_Light->GetQuadratic, m_Light->SetQuadratic, 0.000005f, 2.f, "%.4f", 2.f);
		IMGUI_SLIDER_MODIFIER("Linear", m_Light->GetLinear, m_Light->SetLinear, 0.0001f, 1.f, "%.4f", 2.f);
		break;
	case 1:		// spot light
		IMGUI_SLIDER_MODIFIER("Range", m_Light->GetRange, m_Light->SetRange, 0.0f, 100.f, "%.2f", 1.f);
		IMGUI_SLIDER_MODIFIER("Constant", m_Light->GetConstant, m_Light->SetConstant, 0.0f, 2.f, "%.2f", 1.f);
		IMGUI_SLIDER_MODIFIER("Quadtratic", m_Light->GetQuadratic, m_Light->SetQuadratic, 0.000005f, 2.f, "%.4f", 2.f);
		IMGUI_SLIDER_MODIFIER("Linear", m_Light->GetLinear, m_Light->SetLinear, 0.0001f, 1.f, "%.4f", 2.f);
		IMGUI_SLIDER_MODIFIER("Angle", m_Light->GetTheta, m_Light->SetTheta, cos(0.5f * DegToRad), cos(189.f * DegToRad), "%.1f", 1.f);
		IMGUI_SLIDER_MODIFIER("OuterCutoff", m_Light->GetOuterCutoff, m_Light->SetOuterCutoff, cos(0.5f * DegToRad), cos(189.f * DegToRad), "%.1f", 1.f);
		break;
	case 2:		// directionnal light
		break;
	}
	

}
#endif