#pragma once
#include "GameScene.h"

class Button;

class MainMenuScene : public GameScene
{
public:
	MainMenuScene();
	virtual ~MainMenuScene() = default;

	MainMenuScene(const MainMenuScene& other) = delete;
	MainMenuScene(MainMenuScene&& other) noexcept = delete;
	MainMenuScene& operator=(const MainMenuScene& other) = delete;
	MainMenuScene& operator=(MainMenuScene&& other) noexcept = delete;

protected:
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void SceneActivated() override;

private:
	Button* m_ButtonStart;
	Button* m_ButtonQuit;
	
	static FMOD::Sound* m_pMainMenuMusic;
	static FMOD::Channel* m_pSoundChannel;
};

