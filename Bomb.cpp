#include "stdafx.h"
#include "Bomb.h"

#include "BombermanScene.h"
#include "ModelComponent.h"
#include "TransformComponent.h"
#include "Node.h"
#include "Bomberman.h"
#include "PostChromaticAberration.h"
#include "DataTypes.h"
#include "SoundManager.h"

FMOD::Sound* Bomb::m_pPlaceSound{ nullptr };
FMOD::Sound* Bomb::m_pExplosionSound{ nullptr };
FMOD::Channel* Bomb::m_pSoundChannel{ nullptr };

Bomb::Bomb(BombermanScene* scene, UINT StartTexture)
	: GameObject()
	, m_pGameScene(scene)
	, m_pModelComponent(nullptr)
	, m_Node(nullptr)
	, m_Owner(nullptr)
	, m_Timer(0.f)
	, m_Active(false)
	, m_Range(0)
	, m_StartTexture(StartTexture)
	, m_BombType(BombType::Normal)
	, m_Position(0.f, 0.f)
	, m_FuzeTime(3.f)
	, m_PulseDecrease(0.05f)
	, m_Scale(0.18f)
{
}

void Bomb::Initialize(const GameContext&)
{
	// Bomb Mesh
	//**********
	m_pModelComponent = new ModelComponent(L"./Resources/Meshes/Bomberman/Bomb.ovm");
	m_pModelComponent->SetMaterial(m_StartTexture);
	AddComponent(m_pModelComponent);
	
	GetTransform()->Scale(m_Scale, m_Scale, m_Scale);
	GetTransform()->Translate(m_Position.x, 0.f, m_Position.y);

	// Sound
	//******
	if (!m_pPlaceSound 
		|| !m_pExplosionSound 
		|| !m_pSoundChannel)
	{
		SoundManager::GetInstance()->GetSystem()->createSound("Resources/Audio/Bomberman/BombPlaced.mp3", FMOD_2D, 0, &m_pPlaceSound);
		SoundManager::GetInstance()->GetSystem()->createSound("Resources/Audio/Bomberman/BombExplosion.mp3", FMOD_2D, 0, &m_pExplosionSound);
		SoundManager::GetInstance()->GetSystem()->getChannel(1, &m_pSoundChannel);
	}
}

void Bomb::Update(const GameContext& gameContext)
{
	if (!m_Active
		|| m_pGameScene->IsGamePaused())
		return;

	float deltaTime = gameContext.pGameTime->GetElapsed();

	Pulse();
	
	m_Timer += deltaTime;
	if (m_Timer >= m_FuzeTime)
		Explode();
}

bool Bomb::IsActive() const
{
	return m_Active;
}

// Returns false if it was unable to spawn the Bomb.
bool Bomb::Spawn(Node* node, Bomberman* owner, UINT range, BombType bombType, UINT materialID)
{
	if (node == nullptr
		|| owner == nullptr)
		return false;
	
	m_Node = node;
	m_Position = m_Node->GetPosition();
	m_Owner = owner;
	m_Range = range;
	m_BombType = bombType;
	
	m_pGameScene->AddChild(this);
	m_pModelComponent->SetMaterial(materialID);
	GetTransform()->Scale(m_Scale, m_Scale, m_Scale);
	GetTransform()->Translate(m_Position.x, 0.f, m_Position.y);

	m_Active = true;
	m_Node->SetHasBomb(true);

	SoundManager::GetInstance()->GetSystem()->playSound(m_pPlaceSound, 0, false, &m_pSoundChannel);
	
	return true;
}

void Bomb::Explode() 
{
	m_Owner->AddBomb();
	
	float random = randF(1.f, 2.f);
	
	// 0 = false, 1 = true;
	if (rand() % 2)
		random *= -1;
	
	m_pGameScene->GetChromaticEffect()->SetScale(random);

	switch (m_BombType)
	{
	case BombType::Normal:
		m_Node->NotifyDamageTaken(Direction::All, m_Range);
		break;
	case BombType::Penetrating:
		m_Node->NotifyDamageTaken(Direction::All, m_Range, true);
		break;
	default:
		m_Node->NotifyDamageTaken(Direction::All, m_Range);
		break;
	}

	SoundManager::GetInstance()->GetSystem()->playSound(m_pExplosionSound, 0, false, &m_pSoundChannel);
	
	ResetBomb();
}

void Bomb::Pulse() const
{
	float check = m_Timer - (float)(int)m_Timer;
	float sin = 0.f;
	
	if (check <= 0.5f)
		sin = m_Scale - (m_PulseDecrease * check / 0.5f);
	else
		sin = m_Scale - (m_PulseDecrease - (m_PulseDecrease * (check - 0.5f) / 0.5f));
	
	GetTransform()->Scale(sin, sin, sin);
}

void Bomb::ResetBomb()
{
	m_Timer = 0;
	m_Range = 0;
	m_Active = false;

	if (m_Node != nullptr)
	{
		m_Node->SetHasBomb(false);
		m_Node = nullptr;
	}
	
	m_Owner = nullptr;
	m_BombType = BombType::Normal;
	GetTransform()->Translate(0.f, -100.f, 0.f);
	m_pGameScene->RemoveChild(this, false);
}