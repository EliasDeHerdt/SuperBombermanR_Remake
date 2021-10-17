#include "stdafx.h"
#include "BombManager.h"

#include "Bomb.h"
#include "BombermanScene.h"
#include "../../Materials/Shadow/DiffuseMaterial_Shadow.h"

BombManager::BombManager()
	: m_pGameScene(nullptr)
	, m_BombPool()
	, m_ActiveBombs()
	, m_TextureIDs()
{
}

// The active bombs shouldn't be deleted here because they are still inside the scene.
// Because of this, the scene will delete them!
BombManager::~BombManager()
{
	while (!m_BombPool.empty())
	{
		delete m_BombPool.front();
		m_BombPool.pop();
	}
}

void BombManager::Initialize(BombermanScene* scene, UINT poolSize)
{
	m_pGameScene = scene;

	LoadBombTextures();
	for (UINT i = 0; i < poolSize; ++i)
		m_BombPool.push(new Bomb(scene, m_TextureIDs[0]));
}

void BombManager::Update()
{
	while (!m_ActiveBombs.empty()
			&& !m_ActiveBombs.front()->IsActive())
	{
		m_BombPool.push(m_ActiveBombs.front());
		m_ActiveBombs.pop();
	}
}

void BombManager::RequestBomb(Node* node, Bomberman* owner, UINT range, BombType bombType)
{
	Bomb* bomb = m_BombPool.front();

	m_BombPool.pop();
	m_ActiveBombs.push(bomb);

	// Range gets increased by 1 because the original node the Bomb spawns on is also included in the range!
	// Not doing this might be confusing later on (I've already confused myself with this O_o)
	bomb->Spawn(node, owner, ++range, bombType, m_TextureIDs[(UINT)bombType]);
}

void BombManager::RemoveAllBombs()
{
	while (!m_ActiveBombs.empty())
	{
		Bomb* bomb = m_ActiveBombs.front();

		bomb->ResetBomb();
		m_BombPool.push(bomb);
		m_ActiveBombs.pop();
	}
}

void BombManager::LoadBombTextures()
{
	auto gameContext = m_pGameScene->GetGameContext();

	for (UINT i = 0; i < (UINT)BombType::SizeBuffer; ++i)
	{
		std::wstring texturePath = L"./Resources/Textures/Bomberman/Bomb/Bomb_";
		texturePath.append(ToString((BombType)i));
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
