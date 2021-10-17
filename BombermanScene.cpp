#include "stdafx.h"
#include "BombermanScene.h"
#include "GameObject.h"

#include "ContentManager.h"
#include "Components.h"

#include "../../Materials/Shadow/DiffuseMaterial_Shadow.h"

#include <ctime>
#include "Bomberman.h"
#include "Button.h"
#include "DataTypes.h"
#include "PlayerManager.h"
#include "PostChromaticAberration.h"
#include "OverlordGame.h"
#include "HUD.h"
#include "PhysxProxy.h"
#include "SceneManager.h"
#include "SoundManager.h"

FMOD::Sound* BombermanScene::m_pGameplayMusic{ nullptr };
FMOD::Channel* BombermanScene::m_pSoundChannel{ nullptr };

BombermanScene::BombermanScene(UINT playerCount)
	: GameScene(L"BombermanScene")
	, m_Paused{ false }
	, m_LevelCount{ 3 }
	, m_PlayerCount{ playerCount }
	, m_Grid{ }
	, m_PlayerManager{ }
	, m_PlayerInputs{ }
	, m_pHUD{ nullptr }
	, m_pPauseScreen{ nullptr }
	, m_ChromaticAberration{ nullptr }
{
}

BombermanScene::~BombermanScene()
{
}

void BombermanScene::Initialize()
{
	const auto gameContext = GetGameContext();
	gameContext.pShadowMapper->SetLight({ -100.f, 66.f, -100.f }, { 0.3f, -0.8f, 1.f });
	srand(UINT(std::time(0)));
	
	// Player Inputs
	//**************
	m_PlayerInputs.push_back({ 'W', 'S', 'A', 'D', 'E' });
	m_PlayerInputs.push_back({ VK_UP, VK_DOWN, VK_LEFT, VK_RIGHT, VK_NUMPAD0 });
	m_PlayerInputs.push_back({ 'I', 'K', 'J', 'L', 'O' });
	m_PlayerInputs.push_back({ VK_NUMPAD8, VK_NUMPAD5, VK_NUMPAD4, VK_NUMPAD6, VK_NUMPAD9 });
	
	// Materials
	//**********
	auto diffMat = new DiffuseMaterial_Shadow();
	diffMat->SetDiffuseTexture(L"./Resources/Textures/Bomberman/Ground/Ground_Grass.jpg");
	diffMat->SetLightDirection(gameContext.pShadowMapper->GetLightDirection());
	gameContext.pMaterialManager->AddMaterial(diffMat, 0);
	
	m_Grid.Initialize(this, 18, 12, GenerateRandomLevelPath());
	
	m_PlayerManager.Initialize(this, 100);
	for (UINT i = 0; i < m_PlayerCount; ++i)
		m_PlayerManager.AddPlayer(i + 1, PlayerColor(i), m_PlayerInputs[i]);

	m_PlayerManager.MovePlayersToSpawns(m_Grid.GetSpawns());

	DirectX::XMFLOAT2 playerSlotPos{};
	playerSlotPos.x = 50.f;
	playerSlotPos.y = OverlordGame::GetGameSettings().Window.Height - 65.f;
	
	m_pHUD = new HUD(this, playerSlotPos, *m_PlayerManager.GetPlayersForHUD());
	AddChild(m_pHUD);

	// Buttons
	//********
	float centerButtonX{ 225.f / 2.f };
	float centerScreenX{ OverlordGame::GetGameSettings().Window.Width / 2.f };
	float centerScreenY{ OverlordGame::GetGameSettings().Window.Height / 2.f };
	m_ButtonMainMenu = new Button(this, { centerScreenX - centerButtonX, 250 }, L"Resources/Textures/Bomberman/Buttons/Button_MainMenu_Basic.png", L"Resources/Textures/Bomberman/Buttons/Button_MainMenu_Hover.png");
	m_ButtonRestart = new Button(this, { centerScreenX - centerButtonX, 350 }, L"Resources/Textures/Bomberman/Buttons/Button_Restart_Basic.png", L"Resources/Textures/Bomberman/Buttons/Button_Restart_Hover.png");
	m_ButtonQuit = new Button(this, { centerScreenX - centerButtonX, 450 }, L"Resources/Textures/Bomberman/Buttons/Button_Quit_Basic.png", L"Resources/Textures/Bomberman/Buttons/Button_Quit_Hover.png");

	m_ButtonMainMenu->LinkNeighbor(m_ButtonRestart, Direction::Down);
	
	m_ButtonRestart->LinkNeighbor(m_ButtonMainMenu, Direction::Up);
	m_ButtonRestart->LinkNeighbor(m_ButtonQuit, Direction::Down);
	
	m_ButtonQuit->LinkNeighbor(m_ButtonRestart, Direction::Up);

	m_ButtonMainMenu->SetVisibility(m_Paused);
	m_ButtonRestart->SetVisibility(m_Paused);
	m_ButtonQuit->SetVisibility(m_Paused);

	AddChild(m_ButtonMainMenu);
	AddChild(m_ButtonRestart);
	AddChild(m_ButtonQuit);

	// Pause Screen
	//*************
	m_pPauseScreen = new GameObject();
	m_pPauseScreen->AddComponent(new SpriteComponent(L"Resources/Textures/Bomberman/HUD/PauseMenu.png"));
	m_pPauseScreen->GetTransform()->Translate(centerScreenX - (375.f / 2.f), centerScreenY - (450.f / 2.f), 0.f);
	m_pPauseScreen->SetVisibility(m_Paused);
	
	AddChild(m_pPauseScreen);
	
	// Camera
	//*******
	auto camObject = new GameObject();
	auto camera = new CameraComponent();
	camObject->GetTransform()->Translate(0, 280, -67);
	camObject->GetTransform()->Rotate(78, 0, 0);
	camObject->AddComponent(camera);
	AddChild(camObject);
	SetActiveCamera(camera);
	
	//Ground Mesh
	//***********
	auto pGroundObj = new GameObject();
	auto pGroundModel = new ModelComponent(L"./Resources/Meshes/UnitPlane.ovm");
	pGroundModel->SetMaterial(0);

	pGroundObj->AddComponent(pGroundModel);
	pGroundObj->GetTransform()->Scale(48.0f, 48.0f, 48.0f);

	AddChild(pGroundObj);

	//Post Processing
	//***************
	m_ChromaticAberration = new PostChromaticAberration(true);
	m_ChromaticAberration->SetScale(0.f);
	m_ChromaticAberration->SetFadeTime(0.2f);
	
	AddPostProcessingEffect(m_ChromaticAberration);

	// Sound
	//******
	if (!m_pGameplayMusic 
		|| !m_pSoundChannel)
	{
		SoundManager::GetInstance()->GetSystem()->createStream("Resources/Audio/Bomberman/GameplaySong.mp3", FMOD_2D, 0, &m_pGameplayMusic);
		SoundManager::GetInstance()->GetSystem()->getChannel(0, &m_pSoundChannel);
	}
	SoundManager::GetInstance()->GetSystem()->playSound(m_pGameplayMusic, 0, false, &m_pSoundChannel);

	// Set Loop Count
	m_pSoundChannel->setLoopCount(-1);
	m_pSoundChannel->setMode(FMOD_LOOP_NORMAL);
}

void BombermanScene::Update()
{
	const auto gameContext = GetGameContext();
	const float deltaTime = gameContext.pGameTime->GetElapsed();

	m_Grid.Update(deltaTime);
	m_PlayerManager.Update(deltaTime);
	m_ChromaticAberration->SetElapsedTime(deltaTime);

	if (m_Paused)
	{
		if (m_ButtonMainMenu->IsActive())
			SceneManager::GetInstance()->SetActiveGameScene(L"MainMenuScene");
		if (m_ButtonRestart->IsActive())
			SceneManager::GetInstance()->SetActiveGameScene(L"CharacterPickerScene");
		if (m_ButtonQuit->IsActive())
			PostQuitMessage(WM_QUIT);
	}
}

void BombermanScene::Draw()
{}

void BombermanScene::SceneDeactivated()
{
	m_pSoundChannel->stop();
}

std::string BombermanScene::GenerateRandomLevelPath()
{
	std::string levelPath{ "./Resources/Levels/StandardLevel" };
	levelPath.append(std::to_string((rand() % m_LevelCount) + 1));
	if (m_PlayerCount > 2)
		levelPath.append("_Full");

	levelPath.append(".txt");

	return levelPath;
}

void BombermanScene::RestartLevel(bool fullReset)
{
	m_Grid.SetLevelPath(GenerateRandomLevelPath());
	
	m_Grid.LoadLevel();
	m_PlayerManager.ResetPlayers(fullReset);
	m_PlayerManager.MovePlayersToSpawns(m_Grid.GetSpawns());
}

void BombermanScene::ToggleGamePaused()
{
	m_Paused = !m_Paused;

	// Toggle Pause Menu
	//******************
	m_pPauseScreen->SetVisibility(m_Paused);
	m_ButtonMainMenu->SetVisibility(m_Paused);
	m_ButtonRestart->SetVisibility(m_Paused);
	m_ButtonQuit->SetVisibility(m_Paused);

	// Set User Focus
	if (m_Paused)
	{
		m_ButtonMainMenu->SetUserFocus(true);
		m_ButtonRestart->SetUserFocus(false);
		m_ButtonQuit->SetUserFocus(false);
	}
}

void BombermanScene::SetGamePaused(bool state)
{
	m_Paused = state;
}

bool BombermanScene::IsGamePaused() const
{
	return m_Paused;
}

HUD* BombermanScene::GetHud() const
{
	return m_pHUD;
}

PostChromaticAberration* BombermanScene::GetChromaticEffect() const
{
	return m_ChromaticAberration;
}
