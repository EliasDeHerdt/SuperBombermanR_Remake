#pragma once
#include "GameObject.h"
#include <map>

class SpriteFont;
enum class PlayerColor;
class SpriteComponent;
class BombermanScene;

struct PlayerSlot
{
	UINT playerID;
	PlayerColor playerColor;
	DirectX::XMFLOAT2 position;
	GameObject* head;
	GameObject* score;
};

struct TextSlot
{
	void UpdateTimer(float deltaTime)
	{
		m_DrawTimer += deltaTime;
		if (m_DrawTimer >= time)
		{
			m_DrawTimer = 0.f;
			draw = false;
		}
	}
	
	bool draw;
	float time;
	std::wstring text;
	DirectX::XMFLOAT2 position;

private:
	float m_DrawTimer = 0.f;
};

class HUD : public GameObject
{
public:
	HUD(BombermanScene* scene, DirectX::XMFLOAT2 playerStartPos, std::map<UINT, PlayerColor> playerColors);
	virtual ~HUD() = default;

	HUD(const HUD& other) = delete;
	HUD(HUD&& other) noexcept = delete;
	HUD& operator=(const HUD& other) = delete;
	HUD& operator=(HUD&& other) noexcept = delete;

	void Initialize(const GameContext& gameContext) override;
	void Update(const GameContext& gameContext) override;
	void Draw(const GameContext&) override;
	
	void SetPlayerScore(UINT playerID, UINT score) const;
	void DrawVictoryText(std::wstring text, float time = 2.f);

private:
	BombermanScene* m_pGameScene;

	SpriteFont* m_pFont;
	DirectX::XMFLOAT2 m_PlayerSlotStartPos;
	
	TextSlot m_VictoryText;
	std::vector<PlayerSlot> m_PlayerSlots;

	void LoadPlayerSlot(PlayerSlot& playerSlot, UINT slotNumber);
};
