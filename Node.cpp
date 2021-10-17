#include "stdafx.h"
#include "Node.h"
#include "Bomberman.h"
#include "BombermanScene.h"
#include "PhysxManager.h"
#include "DataTypes.h"
#include "MeshDrawComponent.h"
#include "ModelComponent.h"
#include "PickUpManager.h"
#include "ParticleBurstEmitterComponent.h"
#include "PickUp.h"
#include "RigidBodyComponent.h"
#include "ColliderComponent.h"
#include "TransformComponent.h"

Node::Node(BombermanScene* scene, PickUpManager& pickUpManager, NodeType type, float width, float height, DirectX::XMFLOAT2 position, UINT brittleTextureID, UINT solidTextureID)
	: m_pGameScene(scene)
	, m_NodeType(type)
	, m_PickUpManager(pickUpManager)
	, m_PlayersInNode()
	, m_DropChance(22)
	, m_Width(width)
	, m_Height(height)
	, m_HasBomb(false)
	, m_pPowerUp(nullptr)
	, m_Position(position)
	, m_pSolidWall(nullptr)
	, m_pBrittleWall(nullptr)
	, m_BrittleTextureID(brittleTextureID)
	, m_SolidTextureID(solidTextureID)
	, m_pLeftNeighbor(nullptr)
	, m_pRightNeighbor(nullptr)
	, m_pTopNeighbor(nullptr)
	, m_pBottomNeighbor(nullptr)
{
}

void Node::Initialize(const GameContext&)
{
	auto physx = PhysxManager::GetInstance()->GetPhysics();

	m_pParticleEmitter = new ParticleBurstEmitterComponent(L"./Resources/Textures/Bomberman/Particle/Particle_Explosion.png", 20);
	m_pParticleEmitter->SetVelocity(DirectX::XMFLOAT3(0, 0.0f, 0));
	m_pParticleEmitter->SetMinSize(10.0f);
	m_pParticleEmitter->SetMaxSize(24.0f);
	m_pParticleEmitter->SetMinEnergy(0.4f);
	m_pParticleEmitter->SetMaxEnergy(0.6f);
	m_pParticleEmitter->SetMinSizeGrow(10.f);
	m_pParticleEmitter->SetMaxSizeGrow(15.f);
	m_pParticleEmitter->SetMinEmitterRange(0.1f);
	m_pParticleEmitter->SetMaxEmitterRange(1.f);
	m_pParticleEmitter->SetColor(DirectX::XMFLOAT4(1.f, 1.f, 1.f, 0.6f));
	AddComponent(m_pParticleEmitter);
	GetTransform()->Translate(m_Position.x, m_Height / 2, m_Position.y);

	auto pRigidBody = new RigidBodyComponent(true);
	pRigidBody->SetCollisionGroup(CollisionGroupFlag::Group0);
	AddComponent(pRigidBody);

	DirectX::XMFLOAT3 boxDim{ m_Width, m_Height, m_Width };
	physx::PxMaterial* boxMaterial = physx->createMaterial(0.f, 0.f, 1.f);
	std::shared_ptr<physx::PxGeometry> cubeGeom(new physx::PxBoxGeometry(boxDim.x / 2, boxDim.y / 2, boxDim.z / 2));
	m_pCollider = new ColliderComponent(cubeGeom, *boxMaterial);
	AddComponent(m_pCollider);
	
	m_pSolidWall = new GameObject();
	auto modelComponent = new ModelComponent(L"./Resources/Meshes/Bomberman/Block.ovm");
	modelComponent->SetMaterial(m_SolidTextureID);
	m_pSolidWall->AddComponent(modelComponent);
	m_pSolidWall->GetTransform()->Scale(m_Width, m_Height, m_Width);
	
	m_pBrittleWall = new GameObject();
	modelComponent = new ModelComponent(L"./Resources/Meshes/Bomberman/Block.ovm");
	modelComponent->SetMaterial(m_BrittleTextureID);
	m_pBrittleWall->AddComponent(modelComponent);
	m_pBrittleWall->GetTransform()->Scale(m_Width , m_Height, m_Width );
	m_pBrittleWall->GetTransform()->Translate(0, -m_Height / 3.25f, 0);
	
	AddChild(m_pSolidWall);
	AddChild(m_pBrittleWall);
	UpdateNode();
}

void Node::Update(const GameContext&)
{
	if (m_pGameScene->IsGamePaused())
		m_pParticleEmitter->SetPauseFrames(1);
}

PickUp* Node::GetPickUp() const
{
	return m_pPowerUp;
}

const DirectX::XMFLOAT2& Node::GetPosition() const
{
	return m_Position;
}

// returns positions x & y and the width (width over x & y)
DirectX::XMFLOAT3 Node::GetDimensions() const
{
	return DirectX::XMFLOAT3(m_Position.x, m_Position.y, m_Width);
}

Node* Node::GetNeighbor(Direction direction) const
{
	switch (direction)
	{
	case Direction::Up:
		return m_pTopNeighbor;
	case Direction::Down:
		return m_pBottomNeighbor;
	case Direction::Left:
		return m_pLeftNeighbor;
	case Direction::Right:
		return m_pRightNeighbor;
	}

	return nullptr;
}

void Node::ChangeNodeType(NodeType newType)
{
	m_NodeType = newType;
	UpdateNode();
}

void Node::NotifyDamageTaken(Direction damageDirection, int steps, bool penetrate)
{
	if (steps <= 0)
		return;

	bool forceStop = false;
	switch (m_NodeType)
	{
	case NodeType::Empty:
		if (m_pParticleEmitter)
			m_pParticleEmitter->Burst();
		
		for (auto player : m_PlayersInNode)
			player->Die();
		break;
	case NodeType::Brittle:
		if (m_pParticleEmitter)
			m_pParticleEmitter->Burst();

		if (rand() % 100 <= m_DropChance)
			m_PickUpManager.RequestPickUp(this);
		
		ChangeNodeType(NodeType::Empty);
		forceStop = !penetrate;
		break;
	case NodeType::Solid:
		return;
	}

	if (forceStop)
		return;
	
	switch (damageDirection)
	{
	case Direction::Left:
		if (m_pLeftNeighbor)
			m_pLeftNeighbor->NotifyDamageTaken(damageDirection, --steps, penetrate);
		break;
	case Direction::Right:
		if (m_pRightNeighbor)
			m_pRightNeighbor->NotifyDamageTaken(damageDirection, --steps, penetrate);
		break;
	case Direction::Up:
		if (m_pTopNeighbor)
			m_pTopNeighbor->NotifyDamageTaken(damageDirection, --steps, penetrate);
		break;
	case Direction::Down:
		if (m_pBottomNeighbor)
			m_pBottomNeighbor->NotifyDamageTaken(damageDirection, --steps, penetrate);
		break;
	case Direction::All:
		--steps;
		if (m_pLeftNeighbor)
			m_pLeftNeighbor->NotifyDamageTaken(Direction::Left, steps, penetrate);
		if (m_pRightNeighbor)
			m_pRightNeighbor->NotifyDamageTaken(Direction::Right, steps, penetrate);
		if (m_pTopNeighbor)
			m_pTopNeighbor->NotifyDamageTaken(Direction::Up, steps, penetrate);
		if (m_pBottomNeighbor)
			m_pBottomNeighbor->NotifyDamageTaken(Direction::Down, steps, penetrate);
		break;
	}
}

void Node::SetHasBomb(bool state)
{
	m_HasBomb = state;
}

bool Node::GetHasBomb() const
{
	return m_HasBomb;
}

void Node::LinkToNode(Node* node, Direction direction)
{
	switch (direction)
	{
	case Direction::Up:
		m_pTopNeighbor = node;
		break;
	case Direction::Down:
		m_pBottomNeighbor = node;
		break;
	case Direction::Left:
		m_pLeftNeighbor = node;
		break;
	case Direction::Right:
		m_pRightNeighbor = node;
		break;
	}
}

void Node::AddPlayerToNode(Bomberman* player)
{
	if (std::find(m_PlayersInNode.begin(), m_PlayersInNode.end(), player) == m_PlayersInNode.end())
	{
		m_PlayersInNode.push_back(player);
		if (m_pPowerUp != nullptr)
			m_pPowerUp->PickedUp(player);
	}
}

void Node::RemovePlayerFromNode(Bomberman* player)
{
	m_PlayersInNode.erase(std::remove(m_PlayersInNode.begin(), m_PlayersInNode.end(), player), m_PlayersInNode.end());
}

void Node::SetPickUp(PickUp* powerUp)
{
	m_pPowerUp = powerUp;
}

void Node::UpdateNode() const
{
	switch (m_NodeType)
	{
	case NodeType::Empty:
		m_pSolidWall->SetVisibility(false);
		m_pBrittleWall->SetVisibility(false);
		
		m_pCollider->EnableTrigger(true);
		break;
	case NodeType::Brittle:
		m_pSolidWall->SetVisibility(false);
		m_pBrittleWall->SetVisibility(true);
		
		m_pCollider->EnableTrigger(false);
		break;
	case NodeType::Solid:
		m_pSolidWall->SetVisibility(true);
		m_pBrittleWall->SetVisibility(false);
		
		m_pCollider->EnableTrigger(false);
		break;
	}
}
