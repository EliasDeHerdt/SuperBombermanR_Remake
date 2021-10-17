#include "stdafx.h"
#include "Bomberman.h"

#include "Node.h"
#include "Bomb.h"
#include "BombManager.h"
#include "PhysxManager.h"
#include "PlayerManager.h"
#include "ModelAnimator.h"
#include "ModelComponent.h"
#include "BombermanScene.h"
#include "TransformComponent.h"
#include "ControllerComponent.h"
#include "ParticleEmitterComponent.h"
#include "DataTypes.h"
#include "HUD.h"
#include "SoundManager.h"

FMOD::Sound* Bomberman::m_pDeathSound{ nullptr };
FMOD::Sound* Bomberman::m_pCroudDeathSound{ nullptr };
FMOD::Channel* Bomberman::m_pSoundChannel{ nullptr };

Bomberman::Bomberman(BombermanScene* scene, BombManager& bombManager, UINT playerID, PlayerColor color, UINT materialID, PlayerInput inputLayout)
	: GameObject()
	, m_pGameScene(scene)
	, m_pModel(nullptr)
	, m_pModelComponent(nullptr)
	, m_pController(nullptr)
	, m_pCurrentNode(nullptr)
	, m_pBombManager(bombManager)
	, m_Score(0)
	, m_PlayerID(playerID)
	, m_MaterialID(materialID)
	, m_InputLayout(inputLayout)
	, m_PlayerColor(color)
	, m_BombRange(2)
	, m_BaseBombRange(m_BombRange)
	, m_BombCounter(1)
	, m_BaseBombCounter(m_BombCounter)
	, m_BombType(BombType::Normal)
	, m_IsAlive(true)
	, m_AllowPlayerControl(false)
	, m_SpeedBoost(0.f)
	, m_DeathJumpSpeed(200.0f)
	, m_MaxRunVelocity(50.0f)
	, m_RunAccelerationTime(0.3f)
	, m_RunAcceleration(m_MaxRunVelocity / m_RunAccelerationTime)
	, m_RunVelocity(0)
	, m_CapsuleHeight(12.f)
	, m_Velocity(0, 0, 0)
{
}

void Bomberman::Initialize(const GameContext& gameContext)
{
	auto physx = PhysxManager::GetInstance()->GetPhysics();

	// Controller
	//***********
	m_pController = new ControllerComponent(physx->createMaterial(0.f, 0.f, 1.f), 5.f, m_CapsuleHeight);
	AddComponent(m_pController);

	// Character Mesh
	//***************
	m_pModelComponent = new ModelComponent(L"./Resources/Meshes/Bomberman/BomberMan.ovm");
	m_pModelComponent->SetMaterial(m_MaterialID);

	m_pModel = new GameObject();
	m_pModel->AddComponent(m_pModelComponent);
	m_pModel->GetTransform()->Translate(0.f, -m_CapsuleHeight * 8, 0.f);
	AddChild(m_pModel);
	
	GetTransform()->Scale(0.1f, 0.1f, 0.1f);

	// Particle Emitter
	//*****************
	m_pParticleEmitter = new ParticleEmitterComponent(L"./Resources/Textures/Bomberman/Particle/Particle_Smoke.png", 50, false);
	m_pParticleEmitter->SetVelocity(DirectX::XMFLOAT3(0, 0.0f, 0));
	m_pParticleEmitter->SetMinSize(8.0f);
	m_pParticleEmitter->SetMaxSize(12.0f);
	m_pParticleEmitter->SetMinEnergy(1.0f);
	m_pParticleEmitter->SetMaxEnergy(2.0f);
	m_pParticleEmitter->SetMinSizeGrow(6.5f);
	m_pParticleEmitter->SetMaxSizeGrow(10.f);
	m_pParticleEmitter->SetMinEmitterRange(0.2f);
	m_pParticleEmitter->SetMaxEmitterRange(0.5f);
	m_pParticleEmitter->SetColor(DirectX::XMFLOAT4(1.f, 1.f, 1.f, 0.6f));
	AddComponent(m_pParticleEmitter);

	// Input
	//******
	UINT offset{ (UINT)CharacterInputs::SizeBuffer };
	gameContext.pInput->AddInputAction(InputAction((UINT)CharacterInputs::Up + (offset * m_PlayerID), InputTriggerState::Down, m_InputLayout.Up));
	gameContext.pInput->AddInputAction(InputAction((UINT)CharacterInputs::Left + (offset * m_PlayerID), InputTriggerState::Down, m_InputLayout.Left));
	gameContext.pInput->AddInputAction(InputAction((UINT)CharacterInputs::Down + (offset * m_PlayerID), InputTriggerState::Down, m_InputLayout.Down));
	gameContext.pInput->AddInputAction(InputAction((UINT)CharacterInputs::Right + (offset * m_PlayerID), InputTriggerState::Down, m_InputLayout.Right));
	gameContext.pInput->AddInputAction(InputAction((UINT)CharacterInputs::Bomb + (offset * m_PlayerID), InputTriggerState::Pressed, m_InputLayout.Bomb));

	// Sound
	//******
	if (!m_pDeathSound
		|| !m_pSoundChannel)
	{
		SoundManager::GetInstance()->GetSystem()->createStream("Resources/Audio/Bomberman/PlayerDeath.mp3", FMOD_2D, 0, &m_pDeathSound);
		SoundManager::GetInstance()->GetSystem()->createStream("Resources/Audio/Bomberman/CroudPlayerDies.mp3", FMOD_2D, 0, &m_pCroudDeathSound);
		SoundManager::GetInstance()->GetSystem()->getChannel(1, &m_pSoundChannel);
	}
}

void Bomberman::Update(const GameContext& gameContext)
{
	using namespace DirectX;
	
	if (!m_pController
		|| !m_AllowPlayerControl)
		return;

	if (m_pGameScene->IsGamePaused())
	{
		m_pParticleEmitter->SetPauseFrames(1);
		m_pModelComponent->GetAnimator()->Pause();
		return;
	}

	// Tell the animator to play again if it was paused
	m_pModelComponent->GetAnimator()->Play();
	
	if (!m_IsAlive)
	{
		float deltaTime = gameContext.pGameTime->GetElapsed();
		
		XMFLOAT3 playerPos = GetTransform()->GetPosition();
		XMFLOAT3 camPos = gameContext.pCamera->GetTransform()->GetPosition();

		// Y = height, X & Z is our grid!
		// If the player died, raise him above the camera
		if (GetTransform()->GetPosition().y < camPos.y + 100.f)
			GetTransform()->Translate(playerPos.x, playerPos.y + m_DeathJumpSpeed * deltaTime, playerPos.z);
		else
			m_pParticleEmitter->SetActive(false);
		return;
	}
	
	// Movement Input
	//***************
	UINT offset{ (UINT)CharacterInputs::SizeBuffer };
	
	float diagonal{ 0 };
	auto move = XMFLOAT2(0, 0);
	if (gameContext.pInput->IsActionTriggered((UINT)CharacterInputs::Up + (offset * m_PlayerID)))
	{
		move.y = 1.f;
		diagonal = 45;
		m_pModel->GetTransform()->Rotate(0, 180, 0);
	}
	else if(gameContext.pInput->IsActionTriggered((UINT)CharacterInputs::Down + (offset * m_PlayerID)))
	{
		move.y = -1.f;
		diagonal = -45;
		m_pModel->GetTransform()->Rotate(0, 0, 0);
	}

	if (gameContext.pInput->IsActionTriggered((UINT)CharacterInputs::Right + (offset * m_PlayerID)))
	{
		move.x = 1.f;
		m_pModel->GetTransform()->Rotate(0, -90 - diagonal, 0);
	}
	else if (gameContext.pInput->IsActionTriggered((UINT)CharacterInputs::Left + (offset * m_PlayerID)))
	{
		move.x = -1.f;
		m_pModel->GetTransform()->Rotate(0, 90 + diagonal, 0);
	}

	if (gameContext.pInput->IsActionTriggered((UINT)CharacterInputs::Bomb + (offset * m_PlayerID)))
		PlaceBomb();

	// Calc Velocity
	//**************
	const auto forward = XMLoadFloat3(&GetTransform()->GetForward());
	const auto right = XMLoadFloat3(&GetTransform()->GetRight());

	const auto direction = XMVector3Normalize((forward * move.y) + (right * move.x));

	float maxVel = m_MaxRunVelocity + m_SpeedBoost;
	if (move.x != 0 || move.y != 0)
	{
		m_RunVelocity += m_RunAcceleration * gameContext.pGameTime->GetElapsed();
		if (m_RunVelocity > maxVel)
			m_RunVelocity = maxVel;

		XMStoreFloat3(&m_Velocity, direction * m_RunVelocity);
		
		SetAnimation(1);
	}
	else
	{
		m_Velocity.x = 0;
		m_Velocity.z = 0;
		
		SetAnimation(0);
	}
	
	XMFLOAT3 movement{};
	XMStoreFloat3(&movement, XMLoadFloat3(&m_Velocity) * gameContext.pGameTime->GetElapsed());
	m_pController->Move(movement);

	// Check current Node
	//*******************
	CheckCurrentNode();
}

bool Bomberman::IsAlive() const
{
	return m_IsAlive;
}

UINT Bomberman::GetScore() const
{
	return m_Score;
}

UINT Bomberman::GetPlayerID() const
{
	return m_PlayerID;
}

PlayerColor Bomberman::GetPlayerColor() const
{
	return m_PlayerColor;
}

void Bomberman::Die()
{
	m_IsAlive = false;
	m_pParticleEmitter->SetActive(true);
	m_pCurrentNode->RemovePlayerFromNode(this);
	
	SetAnimation(2);
	SoundManager::GetInstance()->GetSystem()->playSound(m_pDeathSound, 0, false, &m_pSoundChannel);
	SoundManager::GetInstance()->GetSystem()->playSound(m_pCroudDeathSound, 0, false, &m_pSoundChannel);
}

void Bomberman::AddBomb()
{
	++m_BombCounter;
}

void Bomberman::AddRange()
{
	++m_BombRange;
}

void Bomberman::AddSpeed()
{
	m_SpeedBoost += 5;
}

void Bomberman::IncrementScore()
{
	++m_Score;
}

void Bomberman::PlaceBomb()
{
	if (m_BombCounter == 0
		|| m_pCurrentNode->GetHasBomb())
		return;

	--m_BombCounter;
	m_pCurrentNode->SetHasBomb(true);

	// Request 
	m_pBombManager.RequestBomb(m_pCurrentNode, this, m_BombRange, m_BombType);
}

void Bomberman::SetCurrentNode(Node* node)
{
	m_pCurrentNode = node;
}

void Bomberman::SetLocation(DirectX::XMFLOAT2 pos) const
{
	float capsuleOffset{ 3.f };
	GetTransform()->Translate(pos.x, m_CapsuleHeight - capsuleOffset, pos.y);
}

void Bomberman::SetAnimation(UINT clipNumber)
{
	if (m_AnimationIndex != clipNumber)
	{
		m_AnimationIndex = clipNumber;
		m_pModelComponent->GetAnimator()->SetAnimation(m_AnimationIndex);
	}
}

void Bomberman::ResetPlayer(bool fullReset)
{
	m_IsAlive = true;
	m_SpeedBoost = 0.f;
	m_BombType = BombType::Normal;
	m_BombRange = m_BaseBombRange;
	m_BombCounter = m_BaseBombCounter;
	m_pParticleEmitter->SetActive(false);
	if (fullReset)
	{
		m_Score = 0;
		m_pGameScene->GetHud()->SetPlayerScore(m_PlayerID, m_Score);
	}
}

void Bomberman::SetPlayerControl(bool state)
{
	m_AllowPlayerControl = state;
}

void Bomberman::SetBombType(BombType type)
{
	m_BombType = type;
}

void Bomberman::CheckCurrentNode()
{
	// ALL X & Y POSITION ARE AT THE CENTER, NOT LEFT-BOTTOM
	// X = x-position, Z = y-position
	DirectX::XMFLOAT3 position = GetTransform()->GetPosition();

	// X = x-position, Y = y-position, Z = width over X & Y
	DirectX::XMFLOAT3 nodeDimension = m_pCurrentNode->GetDimensions();

	// Right check
	if (position.x > nodeDimension.x + nodeDimension.z / 2)
		MoveToNeighboringNode(Direction::Right);
	// Left check
	if (position.x < nodeDimension.x - nodeDimension.z / 2)
		MoveToNeighboringNode(Direction::Left);
	// Up check
	if (position.z > nodeDimension.y + nodeDimension.z / 2)
		MoveToNeighboringNode(Direction::Up);
	// Down check
	if (position.z < nodeDimension.y - nodeDimension.z / 2)
		MoveToNeighboringNode(Direction::Down);
}

void Bomberman::MoveToNeighboringNode(Direction direction)
{
	// Is a bomb is present in the new node, the player shouldn't be able to enter that node
	// A player IS able to leave his current node if this node contains a bomb
	if (!m_pCurrentNode->GetNeighbor(direction)->GetHasBomb())
	{
		// Old Node
		m_pCurrentNode->RemovePlayerFromNode(this);
		m_pCurrentNode = m_pCurrentNode->GetNeighbor(direction);

		// New Node
		if (!m_pCurrentNode)
			return;
		m_pCurrentNode->AddPlayerToNode(this);
	}
	else
	{
		DirectX::XMFLOAT3 nodeDim{ m_pCurrentNode->GetDimensions() };
		DirectX::XMFLOAT3 playerPos{ GetTransform()->GetPosition() };
		switch (direction)
		{
		case Direction::Up:
			GetTransform()->Translate(playerPos.x, playerPos.y, nodeDim.y + nodeDim.z / 2);
			break;
		case Direction::Down:
			GetTransform()->Translate(playerPos.x, playerPos.y, nodeDim.y - nodeDim.z / 2);
			break;
		case Direction::Left:
			GetTransform()->Translate(nodeDim.x - nodeDim.z / 2, playerPos.y, playerPos.z);
			break;
		case Direction::Right:
			GetTransform()->Translate(nodeDim.x + nodeDim.z / 2, playerPos.y, playerPos.z);
			break;
		}
	}
}
