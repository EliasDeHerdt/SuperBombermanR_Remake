#include "stdafx.h"
#include "PlayerManager.h"

#include "HUD.h"
#include "Bomberman.h"
#include "BombermanScene.h"
#include "GameScene.h"
#include "NodeGrid.h"
#include "Node.h"
#include "SceneManager.h"
#include "SoundManager.h"
#include "../../Materials/Shadow/SkinnedDiffuseMaterial_Shadow.h"

FMOD::Sound* PlayerManager::m_pAnnouncerReadySound{ nullptr };
FMOD::Sound* PlayerManager::m_pAnnouncerGoSound{ nullptr };
FMOD::Sound* PlayerManager::m_pCroudWinSound{ nullptr };
FMOD::Channel* PlayerManager::m_pSoundChannel{ nullptr };

PlayerManager::PlayerManager()
	: m_pGameScene(nullptr)
	, m_BombManager()
	, m_Players()
	, m_TextureIDs()
	, m_GameState(GameState::WarmUp)
	, m_WarmUpDelay(2.f)
	, m_GameOverTimer(0.f)
	, m_RoundOverDelay(3.f)
	, m_GameOverDelay(5.f)
{
}

void PlayerManager::Initialize(BombermanScene* scene, UINT bombPoolSize)
{
	m_pGameScene = scene;

	LoadPlayerTextures();
	m_BombManager.Initialize(scene, bombPoolSize);

	m_pGameScene->GetGameContext().pInput->AddInputAction(InputAction(99, InputTriggerState::Pressed, VK_ESCAPE));

	// Sound
	//******
	if (!m_pAnnouncerReadySound
		|| !m_pAnnouncerGoSound
		|| !m_pCroudWinSound
		|| !m_pSoundChannel)
	{
		SoundManager::GetInstance()->GetSystem()->createStream("Resources/Audio/Bomberman/AnnouncerReady.mp3", FMOD_2D, 0, &m_pAnnouncerReadySound);
		SoundManager::GetInstance()->GetSystem()->createStream("Resources/Audio/Bomberman/AnnouncerGo.mp3", FMOD_2D, 0, &m_pAnnouncerGoSound);
		SoundManager::GetInstance()->GetSystem()->createStream("Resources/Audio/Bomberman/CroudPlayerWins.mp3", FMOD_2D, 0, &m_pCroudWinSound);
		SoundManager::GetInstance()->GetSystem()->getChannel(1, &m_pSoundChannel);
	}
	SoundManager::GetInstance()->GetSystem()->playSound(m_pAnnouncerReadySound, 0, false, &m_pSoundChannel);
}

void PlayerManager::Update(float deltaTime)
{
	// Input Check
	if (m_pGameScene->GetGameContext().pInput->IsActionTriggered(99))
		m_pGameScene->ToggleGamePaused();
	
	if (m_pGameScene->IsGamePaused())
		return;
	
	m_BombManager.Update();

	// Gamestate Check
	if (m_GameState == GameState::WarmUp)
	{
		m_GameOverTimer += deltaTime;
		m_pGameScene->GetHud()->DrawVictoryText(L"Ready?", 0.f);
		if (m_GameOverTimer >= m_WarmUpDelay)
		{
			for (auto player : m_Players)
				player->SetPlayerControl(true);

			m_pGameScene->GetHud()->DrawVictoryText(L"Go!", m_WarmUpDelay);
			SoundManager::GetInstance()->GetSystem()->playSound(m_pAnnouncerGoSound, 0, false, &m_pSoundChannel);

			m_GameOverTimer = 0.f;
			m_GameState = GameState::Running;
		}
	}
	if (m_GameState == GameState::Running)
	{
		UINT GameWonCheck{ 0 };
		Bomberman* lastAlive{};
		for (auto player : m_Players)
			if (player->IsAlive())
			{
				++GameWonCheck;
				lastAlive = player;
			}

		if (GameWonCheck == 1)
			m_GameState = GameState::RoundOver;
		else if (GameWonCheck == 0)
			m_GameState = GameState::TieGame;
		
		// Happens only once since m_RoundOver is now true
		if (m_GameState == GameState::RoundOver)
		{
			lastAlive->IncrementScore();
			m_BombManager.RemoveAllBombs();
			m_pGameScene->GetHud()->SetPlayerScore(lastAlive->GetPlayerID(), lastAlive->GetScore());
			
			if (lastAlive->GetScore() >= 3)
			{
				std::wstring text{L"Player "};
				text.append(std::to_wstring(lastAlive->GetPlayerID()));
				text.append(L" Wins!");
				
				m_GameState = GameState::GameOver;
				m_RoundOverDelay = m_GameOverDelay;
				m_pGameScene->GetHud()->DrawVictoryText(text, m_GameOverDelay);
				SoundManager::GetInstance()->GetSystem()->playSound(m_pCroudWinSound, 0, false, &m_pSoundChannel);
			}
		}
	}

	// Happens every tick
	if (m_GameState != GameState::WarmUp
		&& m_GameState != GameState::Running)
		HandleRoundOver(deltaTime);
}

bool PlayerManager::AddPlayer(UINT playerID, PlayerColor color, PlayerInput inputLayout)
{
	if (std::find_if(m_Players.begin(), m_Players.end(), [playerID](Bomberman* val)
		{
			return (val->GetPlayerID() == playerID);
		}) != m_Players.end())
	{
		Logger::LogWarning(L"PlayerManager: A player with the given ID already exists, add failed!");
		return false;
	}

	Bomberman* player = new Bomberman{ m_pGameScene, m_BombManager, playerID, color, m_TextureIDs[(UINT)color], inputLayout };
	m_Players.push_back(player);
	m_pGameScene->AddChild(player);
	return true;
}

void PlayerManager::MovePlayersToSpawns(const std::vector<PlayerSpawn>& spawns)
{
	for (auto player : m_Players)
		for (auto spawnPoint : spawns)
			if (player->GetPlayerID() == spawnPoint.playerID)
			{
				player->SetCurrentNode(spawnPoint.node);
				player->SetLocation(spawnPoint.node->GetPosition());
				spawnPoint.node->AddPlayerToNode(player);
			}
}

Bomberman* PlayerManager::GetPlayer(UINT playerID) const
{
	return *(std::find_if(m_Players.begin(), m_Players.end(), [&playerID](Bomberman* val)
		{
			return val->GetPlayerID() == playerID;
		}));
}

void PlayerManager::ResetPlayers(bool fullReset)
{
	m_GameState = GameState::Running;
	m_BombManager.RemoveAllBombs();
	
	for (auto player : m_Players)
		player->ResetPlayer(fullReset);
}

std::map<UINT, PlayerColor>* PlayerManager::GetPlayersForHUD()
{
	std::map<UINT, PlayerColor>* output{new std::map<UINT, PlayerColor>()};
	for (auto player : m_Players)
		output->emplace(player->GetPlayerID(), player->GetPlayerColor());
	
	return output;
}

void PlayerManager::LoadPlayerTextures()
{
	auto gameContext = m_pGameScene->GetGameContext();

	for (UINT i = 0; i < (UINT)PlayerColor::SizeBuffer; ++i)
	{
		std::wstring texturePath = L"./Resources/Textures/Bomberman/Character/Character_";
		texturePath.append(ToString((PlayerColor)i));
		texturePath.append(L".tga");

		auto diffMat = new SkinnedDiffuseMaterial_Shadow();
		diffMat->Initialize(gameContext);
		diffMat->SetDiffuseTexture(texturePath);
		diffMat->SetLightDirection(gameContext.pShadowMapper->GetLightDirection());

		int index = gameContext.pMaterialManager->AddMaterial(diffMat);
		if (index != -1)
			m_TextureIDs.push_back(index);
	}
}

void PlayerManager::HandleRoundOver(float deltaTime)
{
	m_GameOverTimer += deltaTime;
	if (m_GameOverTimer >= m_RoundOverDelay)
	{
		m_GameOverTimer = 0;
		
		if (m_GameState == GameState::GameOver)
			SceneManager::GetInstance()->SetActiveGameScene(L"MainMenuScene");
		else
			m_pGameScene->RestartLevel();
	}
}