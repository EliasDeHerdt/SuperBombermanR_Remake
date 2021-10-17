#pragma once
#include "GameScene.h"
#include "NodeGrid.h"
#include "PlayerManager.h"
#include <vector>

class Button;
class HUD;
class Bomberman;
class ModelComponent;
class PostChromaticAberration;
struct PlayerInput;

class BombermanScene : public GameScene
{
public:
	BombermanScene(UINT playerCount);
	virtual ~BombermanScene();

	BombermanScene(const BombermanScene& other) = delete;
	BombermanScene(BombermanScene&& other) noexcept = delete;
	BombermanScene& operator=(const BombermanScene& other) = delete;
	BombermanScene& operator=(BombermanScene&& other) noexcept = delete;

	void RestartLevel(bool fullReset = false);
	void ToggleGamePaused();
	void SetGamePaused(bool state);
	bool IsGamePaused() const;
	HUD* GetHud() const;
	PostChromaticAberration* GetChromaticEffect() const;

protected:
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void SceneDeactivated() override;

private:
	bool m_Paused;
	UINT m_LevelCount;
	UINT m_PlayerCount;
	NodeGrid m_Grid;
	PlayerManager m_PlayerManager;
	std::vector<PlayerInput> m_PlayerInputs;

	Button* m_ButtonMainMenu;
	Button* m_ButtonRestart;
	Button* m_ButtonQuit;
	
	HUD* m_pHUD;
	GameObject* m_pPauseScreen;
	PostChromaticAberration* m_ChromaticAberration;

	static FMOD::Sound* m_pGameplayMusic;
	static FMOD::Channel* m_pSoundChannel;

	std::string GenerateRandomLevelPath();
};