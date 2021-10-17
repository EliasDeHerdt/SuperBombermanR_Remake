#include "stdafx.h"
#include "PickUpManager.h"

#include "PickUp.h"
#include "BombermanScene.h"
#include "../../Materials/Shadow/DiffuseMaterial_Shadow.h"

PickUpManager::PickUpManager()
	: m_pGameScene(nullptr)
	, m_TotalDropChance(0)
	, m_PickUpPool()
	, m_ActivePickUps()
	, m_TextureIDs()
	, m_DropChances()
{
}

// The active PickUps shouldn't be deleted here because they are still inside the scene.
// Because of this, the scene will delete them!
PickUpManager::~PickUpManager()
{
	while (!m_PickUpPool.empty())
	{
		delete m_PickUpPool.front();
		m_PickUpPool.pop();
	}
}

void PickUpManager::Initialize(BombermanScene* scene, UINT poolSize)
{
	m_pGameScene = scene;

	LoadPickUpTextures();
	SetDropChances();
	for (UINT i = 0; i < poolSize; ++i)
		m_PickUpPool.push(new PickUp(scene, m_TextureIDs[0]));

}

void PickUpManager::Update()
{
	if (m_ActivePickUps.empty())
		return;
	
	std::vector<PickUp*> buffer{};
	for (auto pickUp : m_ActivePickUps)
	{
		// If a pickup is no longer active, push it back in the pool
		if (!pickUp->IsActive())
			m_PickUpPool.push(pickUp);
		// If it is still active, push it in the buffer
		else
			buffer.push_back(pickUp);
	}

	// Assign the buffer with only active pickups to m_ActivePickUps
	m_ActivePickUps = buffer;
}

void PickUpManager::RequestPickUp(Node* node)
{
	PickUp* pickUp = m_PickUpPool.front();

	m_PickUpPool.pop();
	m_ActivePickUps.push_back(pickUp);

	PickUpType typeToSpawn{};
	int currentChance{};
	int random = rand() % m_TotalDropChance;
	for (auto dropChance : m_DropChances)
	{
		currentChance += dropChance.influence;
		if (random <= currentChance)
		{
			typeToSpawn = dropChance.type;
			break;
		}
	}
	
	pickUp->Spawn(node, typeToSpawn, m_TextureIDs[(UINT)typeToSpawn]);
}

void PickUpManager::RemoveAllPickUp()
{
	for (auto pickUp : m_ActivePickUps)
	{
		pickUp->ResetPickUp();
		m_PickUpPool.push(pickUp);
	}

	m_ActivePickUps.clear();
}

void PickUpManager::SetDropChances()
{
	m_DropChances.push_back({PickUpType::Bombs, 50});
	m_DropChances.push_back({PickUpType::Range, 30});
	m_DropChances.push_back({PickUpType::Penetrating, 5});
	m_DropChances.push_back({PickUpType::Speed, 15});

	for (auto dropChance : m_DropChances)
		m_TotalDropChance += dropChance.influence;
}

void PickUpManager::LoadPickUpTextures()
{
	auto gameContext = m_pGameScene->GetGameContext();

	for (UINT i = 0; i < (UINT)PickUpType::SizeBuffer; ++i)
	{
		std::wstring texturePath = L"./Resources/Textures/Bomberman/PickUps/PickUp_";
		texturePath.append(ToString((PickUpType)i));
		texturePath.append(L".png");

		auto diffMat = new DiffuseMaterial_Shadow();
		diffMat->Initialize(gameContext);
		diffMat->SetDiffuseTexture(texturePath);
		diffMat->SetLightDirection(gameContext.pShadowMapper->GetLightDirection());
		
		int index = gameContext.pMaterialManager->AddMaterial(diffMat);
		if (index != -1)
			m_TextureIDs.push_back(index);
	}
}
