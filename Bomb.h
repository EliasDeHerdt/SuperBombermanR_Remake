#pragma once
#include "GameObject.h"

class Node;
class Bomberman;
class ModelComponent;
class BombermanScene;
class DiffuseMaterial_Shadow;

enum class BombType
{
	Normal,
	Penetrating,
	SizeBuffer
};

class Bomb : public GameObject
{
public:
	Bomb(BombermanScene* scene, UINT StartTexture);
	virtual ~Bomb() = default;

	Bomb(const Bomb& other) = delete;
	Bomb(Bomb&& other) noexcept = delete;
	Bomb& operator=(const Bomb& other) = delete;
	Bomb& operator=(Bomb&& other) noexcept = delete;

	void Initialize(const GameContext& gameContext) override;
	void Update(const GameContext& gameContext) override;
	
	bool IsActive() const;
	bool Spawn(Node* node, Bomberman* owner, UINT range, BombType bombType, UINT materialID);
	void Explode();
	void ResetBomb();
private:
	BombermanScene* m_pGameScene;
	ModelComponent* m_pModelComponent;

	Node* m_Node;
	Bomberman* m_Owner;
	
	float m_Timer;
	bool m_Active;
	UINT m_Range;
	UINT m_StartTexture;
	BombType m_BombType;
	DirectX::XMFLOAT2 m_Position;
	
	const float m_FuzeTime;
	const float m_PulseDecrease;
	const float m_Scale;

	static FMOD::Sound* m_pPlaceSound;
	static FMOD::Sound* m_pExplosionSound;
	static FMOD::Channel* m_pSoundChannel;

	void Pulse() const;
};

// Used the following source to make this:
// https://stackoverflow.com/questions/5093460/how-to-convert-an-enum-type-variable-to-a-string
inline const wchar_t* ToString(BombType v)
{
	switch (v)
	{
	case BombType::Normal:
		return L"Normal";
	case BombType::Penetrating:
		return L"Penetrating";
	default:
		return L"Normal";
	}
}