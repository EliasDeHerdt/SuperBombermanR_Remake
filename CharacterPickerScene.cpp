#include "stdafx.h"
#include "CharacterPickerScene.h"

#include "BombermanScene.h"
#include "Button.h"
#include "DataTypes.h"
#include "OverlordGame.h"
#include "SceneManager.h"
#include "SoundManager.h"
#include "SpriteComponent.h"
#include "TransformComponent.h"

CharacterPickerScene::CharacterPickerScene()
	: GameScene(L"CharacterPickerScene")
	, m_ActivePlayers(2)
	, m_MinPlayers(2)
	, m_MaxPlayers(4)
	, m_ButtonContinue(nullptr)
	, m_ButtonAddPlayer(nullptr)
	, m_ButtonRemovePlayer(nullptr)
	, m_Players((const UINT)m_MaxPlayers)
	, m_pMainMenuMusic(nullptr)
	, m_pSoundChannel(nullptr)
{
}

void CharacterPickerScene::Initialize()
{
	// Buttons
	//********
	float offsetY{ 530.f };
	float buttonSize{ 225.f };
	float centerButtonX{ buttonSize / 2.f };
	float centerScreenX{ OverlordGame::GetGameSettings().Window.Width / 2.f };
	m_ButtonContinue = new Button(this, { centerScreenX - centerButtonX, offsetY }, L"Resources/Textures/Bomberman/Buttons/Button_Start_Basic.png", L"Resources/Textures/Bomberman/Buttons/Button_Start_Hover.png");
	m_ButtonAddPlayer = new Button(this, { centerScreenX - centerButtonX + buttonSize + 20, offsetY }, L"Resources/Textures/Bomberman/Buttons/Button_Add_Basic.png", L"Resources/Textures/Bomberman/Buttons/Button_Add_Hover.png");
	m_ButtonRemovePlayer = new Button(this, { centerScreenX - centerButtonX - buttonSize - 20, offsetY }, L"Resources/Textures/Bomberman/Buttons/Button_Remove_Basic.png", L"Resources/Textures/Bomberman/Buttons/Button_Remove_Hover.png");

	m_ButtonContinue->LinkNeighbor(m_ButtonRemovePlayer, Direction::Left);
	m_ButtonContinue->LinkNeighbor(m_ButtonAddPlayer, Direction::Right);
	m_ButtonContinue->SetUserFocus(true);

	m_ButtonAddPlayer->LinkNeighbor(m_ButtonContinue, Direction::Left);
	m_ButtonRemovePlayer->LinkNeighbor(m_ButtonContinue, Direction::Right);

	AddChild(m_ButtonContinue);
	AddChild(m_ButtonAddPlayer);
	AddChild(m_ButtonRemovePlayer);

	// Player Picker
	//**************
	float xPos{ 100 };
	float slotSize{ 225 };
	float buffer{ ((OverlordGame::GetGameSettings().Window.Width - slotSize * 4 - xPos * 2) / 4) + ((OverlordGame::GetGameSettings().Window.Width - slotSize * 4 - xPos * 2) / 12) };

	for (UINT i = 0; i < m_MaxPlayers; ++i)
	{
		m_Players[i] = new SpriteComponent(L"Resources/Textures/Bomberman/HUD/SelectScreen_Player1.png");

		
		auto obj = new GameObject();
		obj->AddComponent(m_Players[i]);
		obj->GetTransform()->Translate(xPos + (i * (slotSize + buffer)), 120, 0);

		AddChild(obj);
	}
	UpdatePlayerIcons();

	auto obj = new GameObject();
	obj->AddComponent(new SpriteComponent(L"Resources/Textures/Bomberman/HUD/SelectScreen_PauseWidget.png"));
	obj->GetTransform()->Translate(centerScreenX - centerButtonX, 30.f, 0.f);
	
	AddChild(obj);
	
	// Background
	//***********
	auto background = new GameObject();
	background->AddComponent(new SpriteComponent(L"Resources/Textures/Bomberman/HUD/GP2Exam2021_Controls.png"));

	AddChild(background);
}

void CharacterPickerScene::Update()
{
	if (m_ButtonContinue->IsActive())
		LoadGame();
	if (m_ButtonAddPlayer->IsActive() 
		&& m_ActivePlayers < m_MaxPlayers)
	{
		++m_ActivePlayers;
		UpdatePlayerIcons();
	}
	if (m_ButtonRemovePlayer->IsActive() 
		&& m_ActivePlayers > m_MinPlayers)
	{
		--m_ActivePlayers;
		UpdatePlayerIcons();
	}
}

void CharacterPickerScene::Draw()
{
}

void CharacterPickerScene::SceneActivated()
{
	bool isPlaying{};
	m_pSoundChannel->isPlaying(&isPlaying);

	if (!isPlaying)
		SoundManager::GetInstance()->GetSystem()->playSound(m_pMainMenuMusic, 0, false, &m_pSoundChannel);
}

void CharacterPickerScene::UpdateSoundData(FMOD::Sound* mainMenuMusic, FMOD::Channel* soundChannel)
{
	m_pMainMenuMusic = mainMenuMusic;
	m_pSoundChannel = soundChannel;
}

void CharacterPickerScene::UpdatePlayerIcons() const
{
	std::wstring filepath{ L"Resources/Textures/Bomberman/HUD/SelectScreen_Player" };
	
	for (UINT i = 0; i < (UINT)m_ActivePlayers; ++i)
	{
		std::wstring activePath{ filepath };
		activePath.append(std::to_wstring(i + 1));
		activePath.append(L"_Active.png");

		m_Players[i]->SetTexture(activePath);
	}
	for (UINT j = UINT(m_MaxPlayers - 1); j >= (UINT)m_ActivePlayers; --j)
	{
		std::wstring activePath{ filepath };
		activePath.append(std::to_wstring(j + 1));
		activePath.append(L".png");

		m_Players[j]->SetTexture(activePath);
	}
}

void CharacterPickerScene::LoadGame() const
{
	auto scenemanager = SceneManager::GetInstance();
	
	GameScene* scene = scenemanager->GetScene(L"BombermanScene");
	if (scene)
	{
		scenemanager->RemoveGameScene(scene);
		SafeDelete(scene);
	}
	
	m_pSoundChannel->stop();
	scenemanager->AddGameScene(new BombermanScene(m_ActivePlayers));
	scenemanager->SetActiveGameScene(L"BombermanScene");
}
