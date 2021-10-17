#pragma once
#include "PostProcessingMaterial.h"

class ID3D11EffectShaderResourceVariable;
class PostChromaticAberration : public PostProcessingMaterial
{
public:
	PostChromaticAberration(bool LerpScale = false);
	virtual ~PostChromaticAberration() = default;

	PostChromaticAberration(const PostChromaticAberration& other) = delete;
	PostChromaticAberration(PostChromaticAberration&& other) noexcept = delete;
	PostChromaticAberration& operator=(const PostChromaticAberration& other) = delete;
	PostChromaticAberration& operator=(PostChromaticAberration&& other) noexcept = delete;

	void SetScale(float scale) { m_Scale = m_SetScale = scale; m_SafetyTimer = 0.f; }
	void SetFadeTime(float descreaseSpeed) { m_FadeTime = descreaseSpeed; }
	void SetElapsedTime(float deltaTime) { m_DeltaTime = deltaTime; }

protected:
	void LoadEffectVariables() override;
	void UpdateEffectVariables(RenderTarget* pRendertarget) override;

private:
	ID3DX11EffectShaderResourceVariable* m_pTextureMapVariabele;
	ID3DX11EffectVectorVariable* m_pRVariable;
	ID3DX11EffectVectorVariable* m_pGVariable;
	ID3DX11EffectVectorVariable* m_pBVariable;
	ID3DX11EffectScalarVariable* m_pScaleVariable;
	
	DirectX::XMFLOAT2 m_ROffset;
	DirectX::XMFLOAT2 m_GOffset;
	DirectX::XMFLOAT2 m_BOffset;
	float m_Scale;
	float m_SetScale;
	float m_DeltaTime;
	float m_FadeTime;
	float m_SafetyTimer;

	bool m_LerpScale;
};

