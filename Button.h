#pragma once
#include "GameObject.h"

enum class Direction;
class BombermanScene;
class Button : public GameObject
{
public:
	Button(GameScene* scene, DirectX::XMFLOAT2 position, std::wstring basicPath, std::wstring hoveredPath);
	virtual ~Button() override;

	Button(const Button& other) = delete;
	Button(Button&& other) noexcept = delete;
	Button& operator=(const Button& other) = delete;
	Button& operator=(Button&& other) noexcept = delete;

	void Initialize(const GameContext & gameContext) override;
	void Update(const GameContext & gameContext) override;

	void SetUserFocus(bool state, bool forceTake = true);
	bool HasUserFocus() const;
	bool IsActive() const;

	void LinkNeighbor(Button* neighbor, Direction direction);
	void MoveToNeighbor(Direction direction);
private:
	GameScene* m_pGameScene;

	bool m_Active;
	bool m_SkipUpdate;
	bool m_HasUserFocus;
	bool m_FocusChangeCheck;
	GameObject* m_pBasicButton;
	GameObject* m_pHoveredButton;
	std::wstring m_BasicPath;
	std::wstring m_HoveredPath;
	DirectX::XMFLOAT2 m_Position;

	Button* m_TopNeighbor;
	Button* m_BottomNeighbor;
	Button* m_LeftNeighbor;
	Button* m_RightNeighbor;
	
	static bool m_MouseControl;
	static bool m_MouseClicked;
	static FMOD::Sound* m_pActiveSound;
	static FMOD::Sound* m_pHoverSound;
	static FMOD::Channel* m_pSoundChannel;

	bool CheckMouseInsideButton() const;
};
