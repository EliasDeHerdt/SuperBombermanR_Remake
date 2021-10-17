#include "stdafx.h"
#include "PostChromaticAberration.h"
#include "RenderTarget.h"

PostChromaticAberration::PostChromaticAberration(bool LerpScale)
	: PostProcessingMaterial(L"./Resources/Effects/Post/ChromaticAberration.fx", 1)
	, m_pTextureMapVariabele(nullptr)
	, m_pRVariable(nullptr)
	, m_pGVariable(nullptr)
	, m_pBVariable(nullptr)
	, m_ROffset(.003f, .005f)
	, m_GOffset(.003f, -.006f)
	, m_BOffset(.007f, -.004f)
	, m_Scale(1.f)
	, m_SetScale(m_Scale)
	, m_FadeTime(1.f)
	, m_SafetyTimer(0.f)
	, m_LerpScale(LerpScale)
{
}

void PostChromaticAberration::LoadEffectVariables()
{
	m_pTextureMapVariabele = GetEffect()->GetVariableByName("gTexture")->AsShaderResource();
	if (!m_pTextureMapVariabele->IsValid())
		Logger::LogWarning(L"PostChromaticAberration: Failed to link m_pTextureMapVariabele!");

	m_pRVariable = GetEffect()->GetVariableByName("gROffset")->AsVector();
	if (!m_pRVariable->IsValid())
		Logger::LogWarning(L"PostChromaticAberration: Failed to link m_pRVariable!");

	m_pGVariable = GetEffect()->GetVariableByName("gGOffset")->AsVector();
	if (!m_pGVariable->IsValid())
		Logger::LogWarning(L"PostChromaticAberration: Failed to link m_pGVariable!");

	m_pBVariable = GetEffect()->GetVariableByName("gBOffset")->AsVector();
	if (!m_pBVariable->IsValid())
		Logger::LogWarning(L"PostChromaticAberration: Failed to link m_pBVariable!");

	m_pScaleVariable = GetEffect()->GetVariableByName("gScale")->AsScalar();
	if (!m_pScaleVariable->IsValid())
		Logger::LogWarning(L"PostChromaticAberration: Failed to link m_pScaleVariable!");
}

void PostChromaticAberration::UpdateEffectVariables(RenderTarget* pRendertarget)
{
	if (m_pTextureMapVariabele->IsValid())
		m_pTextureMapVariabele->SetResource(pRendertarget->GetShaderResourceView());
	if (m_pRVariable->IsValid())
		m_pRVariable->SetFloatVector(&m_ROffset.x);
	if (m_pGVariable->IsValid())
		m_pGVariable->SetFloatVector(&m_GOffset.x);
	if (m_pBVariable->IsValid())
		m_pBVariable->SetFloatVector(&m_BOffset.x);
	if (m_pScaleVariable->IsValid())
		m_pScaleVariable->SetFloat(m_Scale);

	if (!m_LerpScale)
		return;

	m_SafetyTimer += m_DeltaTime;
	if (m_SafetyTimer < m_FadeTime)
	{
		if (m_Scale != 0.f)
			m_Scale -= (m_SetScale / m_FadeTime) * m_DeltaTime;

		if (m_Scale < .1f
			&& m_Scale > -.1f)
			m_Scale = 0;
	}
}
