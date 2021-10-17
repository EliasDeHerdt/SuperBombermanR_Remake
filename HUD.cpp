#include "stdafx.h"
#include "HUD.h"

#include "BombermanScene.h"
#include "ContentManager.h"
#include "SpriteComponent.h"
#include "PlayerManager.h"
#include "TransformComponent.h"
#include "SpriteFont.h"
#include "OverlordGame.h"
#include "TextRenderer.h"

HUD::HUD(BombermanScene* scene, DirectX::XMFLOAT2 playerStartPos, std::map<UINT, PlayerColor> playerColors)
	: GameObject()
	, m_pGameScene(scene)
	, m_pFont(nullptr)
	, m_PlayerSlotStartPos(playerStartPos)
	, m_VictoryText()
	, m_PlayerSlots()
{
	for (auto playerInfo : playerColors)
		m_PlayerSlots.push_back(PlayerSlot{ playerInfo.first, playerInfo.second });
}

void HUD::Initialize(const GameContext&)
{
	m_pFont = ContentManager::Load<SpriteFont>(L"./Resources/SpriteFonts/Bomberman_64.fnt");
	
	for (UINT i = 0; i < m_PlayerSlots.size(); ++i)
		LoadPlayerSlot(m_PlayerSlots[i], i);
}

void HUD::Update(const GameContext&)
{
}

void HUD::Draw(const GameContext&)
{
	const auto gameContext = m_pGameScene->GetGameContext();

	if (m_VictoryText.draw)
	{
		// A little dirty, but oh well...
		if (!m_pGameScene->IsGamePaused())
			m_VictoryText.UpdateTimer(gameContext.pGameTime->GetElapsed());

		TextRenderer::GetInstance()->DrawText(m_pFont, m_VictoryText.text, m_VictoryText.position, static_cast<DirectX::XMFLOAT4>(DirectX::Colors::White));
	}
}

void HUD::SetPlayerScore(UINT playerID, UINT score) const
{
	if (score > 3)
		return;
	
	auto it{ std::find_if(m_PlayerSlots.begin(), m_PlayerSlots.end(), [playerID](const PlayerSlot& val)
		{
			return val.playerID == playerID;
		}) };

	if (it == m_PlayerSlots.end())
		return;

	std::wstring scorePath{ L"./Resources/Textures/Bomberman/HUD/HUD_Score" };
	scorePath.append(std::to_wstring(score));
	scorePath.append(L".png");
	
	it->score->GetComponent<SpriteComponent>()->SetTexture(scorePath);
}

void HUD::DrawVictoryText(std::wstring text, float time)
{
	m_VictoryText.draw = true;
	m_VictoryText.text = text;
	m_VictoryText.time = time;

	float fontSize{ 64.f };
	m_VictoryText.position = { (OverlordGame::GetGameSettings().Window.Width / 2.f) - ((fontSize * text.size()) / 3.f), (OverlordGame::GetGameSettings().Window.Height / 2.f) - fontSize};
}

void HUD::LoadPlayerSlot(PlayerSlot& playerSlot, UINT slotNumber)
{
	std::wstring playerPath{ L"./Resources/Textures/Bomberman/HUD/HUD_Head" };
	playerPath.append(ToString(playerSlot.playerColor));
	playerPath.append(L".png");

	std::wstring scorePath{ L"./Resources/Textures/Bomberman/HUD/HUD_Score0.png" };

	playerSlot.head = new GameObject();
	playerSlot.score = new GameObject();

	float slot{ (float)slotNumber };
	float scale{ 0.12f };
	float slotSize{ scale * (613.f + 872.f) };
	float buffer{ ((OverlordGame::GetGameSettings().Window.Width - slotSize * 4 - m_PlayerSlotStartPos.x*2) / 4) + ((OverlordGame::GetGameSettings().Window.Width - slotSize * 4 - m_PlayerSlotStartPos.x * 2) / 12)};

	if (slotNumber == m_PlayerSlots.size() - 1)
		slot = 3;

	playerSlot.position.x = m_PlayerSlotStartPos.x + (slot * (slotSize + buffer));
	playerSlot.position.y = m_PlayerSlotStartPos.y;

	if (m_PlayerSlots.size() % 2 == 1
		&& slot == int((m_PlayerSlots.size() - 1) / 2.f))
		playerSlot.position.x = (OverlordGame::GetGameSettings().Window.Width / 2) - (slotSize / 2);
	
	playerSlot.head->AddComponent(new SpriteComponent(playerPath));
	playerSlot.head->GetTransform()->Translate(playerSlot.position.x, playerSlot.position.y, 0.f);
	playerSlot.head->GetTransform()->Scale(scale, scale, scale);
	
	playerSlot.score->AddComponent(new SpriteComponent(scorePath));
	playerSlot.score->GetTransform()->Translate(playerSlot.position.x + 613.f * scale, playerSlot.position.y, 0.f);
	playerSlot.score->GetTransform()->Scale(scale, scale, scale);
	
	AddChild(playerSlot.head);
	AddChild(playerSlot.score);
}
