#include "stdafx.h"
#include "MainMenuScene.h"

#include "Button.h"
#include "CharacterPickerScene.h"
#include "DataTypes.h"
#include "SceneManager.h"
#include "SpriteComponent.h"
#include "SoundManager.h"

FMOD::Sound* MainMenuScene::m_pMainMenuMusic{ nullptr };
FMOD::Channel* MainMenuScene::m_pSoundChannel{ nullptr };

MainMenuScene::MainMenuScene()
	: GameScene(L"MainMenuScene")
{
}

void MainMenuScene::Initialize()
{
	// Buttons
	//********
	m_ButtonStart = new Button(this, { 428, 377 }, L"Resources/Textures/Bomberman/Buttons/Button_Start_Basic.png", L"Resources/Textures/Bomberman/Buttons/Button_Start_Hover.png");
	m_ButtonQuit = new Button(this, { 428, 471 }, L"Resources/Textures/Bomberman/Buttons/Button_Quit_Basic.png", L"Resources/Textures/Bomberman/Buttons/Button_Quit_Hover.png");
	
	m_ButtonStart->LinkNeighbor(m_ButtonQuit, Direction::Down);
	m_ButtonStart->SetUserFocus(true);
	
	m_ButtonQuit->LinkNeighbor(m_ButtonStart, Direction::Up);

	AddChild(m_ButtonStart);
	AddChild(m_ButtonQuit);

	// Background
	//***********
	auto background = new GameObject();
	background->AddComponent(new SpriteComponent(L"Resources/Textures/Bomberman/HUD/GP2Exam2021_MainMenu.png"));

	AddChild(background);

	// Sound
	//******
	if (!m_pMainMenuMusic 
		|| !m_pSoundChannel)
	{
		SoundManager::GetInstance()->GetSystem()->createStream("Resources/Audio/Bomberman/MainMenuSong.mp3", FMOD_2D, 0, &m_pMainMenuMusic);
		SoundManager::GetInstance()->GetSystem()->getChannel(0, &m_pSoundChannel);
	}
	SoundManager::GetInstance()->GetSystem()->playSound(m_pMainMenuMusic, 0, false, &m_pSoundChannel);

	// Set Loop Count
	m_pSoundChannel->setLoopCount(-1);
	m_pSoundChannel->setMode(FMOD_LOOP_NORMAL);
}

void MainMenuScene::Update()
{
	if (m_ButtonStart->IsActive())
	{
		CharacterPickerScene* scene = reinterpret_cast<CharacterPickerScene*>(SceneManager::GetInstance()->GetScene(L"CharacterPickerScene"));
		if (scene == nullptr)
			scene = new CharacterPickerScene();
			SceneManager::GetInstance()->AddGameScene(scene);

		scene->UpdateSoundData(m_pMainMenuMusic, m_pSoundChannel);
		SceneManager::GetInstance()->SetActiveGameScene(L"CharacterPickerScene");
	}
	if (m_ButtonQuit->IsActive())
		PostQuitMessage(WM_QUIT);
}

void MainMenuScene::Draw()
{
}

void MainMenuScene::SceneActivated()
{
	bool isPlaying{};
	m_pSoundChannel->isPlaying(&isPlaying);
	
	if (!isPlaying)
		SoundManager::GetInstance()->GetSystem()->playSound(m_pMainMenuMusic, 0, false, &m_pSoundChannel);
}