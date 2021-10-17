#pragma once
#include "GameScene.h"
#include <vector>

class Button;
class SpriteComponent;
class CharacterPickerScene : public GameScene
{
public:
	CharacterPickerScene();
	virtual ~CharacterPickerScene() = default;

	CharacterPickerScene(const CharacterPickerScene& other) = delete;
	CharacterPickerScene(CharacterPickerScene&& other) noexcept = delete;
	CharacterPickerScene& operator=(const CharacterPickerScene& other) = delete;
	CharacterPickerScene& operator=(CharacterPickerScene&& other) noexcept = delete;

	void UpdateSoundData(FMOD::Sound* mainMenuMusic, FMOD::Channel* soundChannel);

protected:
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void SceneActivated() override;
	
private:
	UINT m_ActivePlayers;
	const UINT m_MinPlayers;
	const UINT m_MaxPlayers;

	Button* m_ButtonContinue;
	Button* m_ButtonAddPlayer;
	Button* m_ButtonRemovePlayer;
	std::vector<SpriteComponent*> m_Players;
	
	FMOD::Sound* m_pMainMenuMusic;
	FMOD::Channel* m_pSoundChannel;
	
	void UpdatePlayerIcons() const;
	void LoadGame() const;
};