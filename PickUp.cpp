#include "stdafx.h"
#include "PickUp.h"

#include "Bomb.h"
#include "Node.h"
#include "Bomberman.h"
#include "BombermanScene.h"
#include "ModelComponent.h"
#include "SoundManager.h"
#include "TransformComponent.h"

FMOD::Sound* PickUp::m_pPickUpSound{ nullptr };
FMOD::Channel* PickUp::m_pSoundChannel{ nullptr };

PickUp::PickUp(BombermanScene* scene, UINT StartTexture)
	: GameObject()
	, m_pGameScene(scene)
	, m_Node(nullptr)
	, m_Timer(0.f)
	, m_Active(false)
	, m_StartTexture(StartTexture)
	, m_PickUpType(PickUpType::Bombs)
	, m_Position(0, 0)
	, m_YOffset(12.f)
	, m_HoverRange(2.f)
{
}

void PickUp::Initialize(const GameContext& )
{
	// PickUp Mesh
	//**********
	m_pModelComponent = new ModelComponent(L"./Resources/Meshes/Bomberman/Plane.ovm");
	m_pModelComponent->SetMaterial(m_StartTexture);
	AddComponent(m_pModelComponent);

	GetTransform()->Translate(m_Position.x, m_YOffset, m_Position.y);
	GetTransform()->Rotate(55, 0, 0);
	GetTransform()->Scale(12.f / 100, 12.f / 100, 12.f / 100);

	// Sound
	//******
	if (!m_pPickUpSound
		|| !m_pSoundChannel)
	{
		SoundManager::GetInstance()->GetSystem()->createStream("Resources/Audio/Bomberman/PickUpActive.mp3", FMOD_2D, 0, &m_pPickUpSound);
		SoundManager::GetInstance()->GetSystem()->getChannel(1, &m_pSoundChannel);
	}
}

void PickUp::Update(const GameContext& gameContext)
{
	if (!m_Active
		|| m_pGameScene->IsGamePaused())
		return;

	float deltaTime = gameContext.pGameTime->GetElapsed();

	Hover();

	m_Timer += deltaTime;
}

bool PickUp::Spawn(Node* node, PickUpType pickUpType, UINT materialID)
{
	if (node == nullptr
		&& m_Node->GetPickUp() == nullptr)
		return false;

	m_Node = node;
	m_Position = m_Node->GetPosition();
	m_PickUpType = pickUpType;

	m_pGameScene->AddChild(this);
	m_pModelComponent->SetMaterial(materialID);

	m_Active = true;
	m_Node->SetPickUp(this);

	return true;
}

bool PickUp::IsActive() const
{
	return m_Active;
}

//implement this
void PickUp::PickedUp(Bomberman* player)
{
	switch (m_PickUpType)
	{
	case PickUpType::Bombs:
		player->AddBomb();
		break;
	case PickUpType::Range:
		player->AddRange();
		break;
	case PickUpType::Penetrating:
		player->SetBombType(BombType::Penetrating);
		break;
	//case PickUpType::Safety:
	//	break;
	case PickUpType::Speed:
		player->AddSpeed();
		break;
	//case PickUpType::Kicker:
	//	break;
	//case PickUpType::PassThrough:
	//	break;
	default:
		break;
	}

	SoundManager::GetInstance()->GetSystem()->playSound(m_pPickUpSound, 0, false, &m_pSoundChannel);

	ResetPickUp();
}

void PickUp::ResetPickUp()
{
	m_Timer = 0;
	m_Active = false;

	m_Node->SetPickUp(nullptr);
	m_Node = nullptr;

	m_PickUpType = PickUpType::Bombs;
	m_pGameScene->RemoveChild(this, false);
}

void PickUp::Hover() const
{
	const float modulo{ 2.f };
	float check = fmod(m_Timer, modulo);
	float sin = 0.f;

	if (check <= (modulo / 2.f))
		sin = m_YOffset + (m_HoverRange * check);
	else
		sin = m_YOffset + (m_HoverRange - (m_HoverRange * check - modulo));
	
	GetTransform()->Translate(m_Position.x, sin, m_Position.y);
}
