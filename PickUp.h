#pragma once
#include "GameObject.h"

class ModelComponent;
class Bomberman;
class Node;
class BombermanScene;

enum class PickUpType
{
	Bombs,
	Range,
	Penetrating,
	//Safety,
	Speed,
	//Kicker,
	//PassThrough,
	SizeBuffer
};

class PickUp : public GameObject
{
public:
	PickUp(BombermanScene* scene, UINT StartTexture);
	virtual ~PickUp() = default;

	PickUp(const PickUp& other) = delete;
	PickUp(PickUp&& other) noexcept = delete;
	PickUp& operator=(const PickUp& other) = delete;
	PickUp& operator=(PickUp&& other) noexcept = delete;

	void Initialize(const GameContext & gameContext) override;
	void Update(const GameContext & gameContext) override;

	bool IsActive() const;
	bool Spawn(Node* node, PickUpType pickUpType, UINT materialID);
	void PickedUp(Bomberman* player);
	void ResetPickUp();
private:
	BombermanScene* m_pGameScene;
	ModelComponent* m_pModelComponent;

	Node* m_Node;
	
	float m_Timer;
	bool m_Active;
	UINT m_StartTexture;
	PickUpType m_PickUpType;
	DirectX::XMFLOAT2 m_Position;
	
	const float m_YOffset;
	const float m_HoverRange;

	static FMOD::Sound* m_pPickUpSound;
	static FMOD::Channel* m_pSoundChannel;
	
	void Hover() const;
};

// Used the following source to make this:
// https://stackoverflow.com/questions/5093460/how-to-convert-an-enum-type-variable-to-a-string
inline const wchar_t* ToString(PickUpType v)
{
	switch (v)
	{
	case PickUpType::Bombs:
		return L"Bombs";
	case PickUpType::Range:
		return L"Range";
	case PickUpType::Penetrating:
		return L"Penetrating";
	//case PickUpType::Safety:
	//	return L"Safety";
	case PickUpType::Speed:
		return L"Speed";
	//case PickUpType::Kicker:
	//	return L"Kicker";
	//case PickUpType::PassThrough:
	//	return L"PassThrough";
	default:
		return L"Bombs";
	}
}