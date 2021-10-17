#pragma once
#include <vector>
#include <map>
#include "BombManager.h"

class HUD;
class Bomberman;
class BombermanScene;
struct PlayerSpawn;
struct PlayerInput;
enum class PlayerColor;

// Always keep SizeBuffer Last!
// Breaks if you start changing the entry values!
enum class PlayerColor
{
	White = 0,
	Black,
	Red,
	Green,
	SizeBuffer
};

class PlayerManager
{
public:
	PlayerManager();
	virtual ~PlayerManager() = default;

	PlayerManager(const PlayerManager& other) = delete;
	PlayerManager(PlayerManager&& other) noexcept = delete;
	PlayerManager& operator=(const PlayerManager& other) = delete;
	PlayerManager& operator=(PlayerManager&& other) noexcept = delete;

	void Initialize(BombermanScene* scene, UINT bombPoolSize);
	void Update(float deltaTime);
	bool AddPlayer(UINT playerID, PlayerColor color, PlayerInput inputLayout);
	void MovePlayersToSpawns(const std::vector<PlayerSpawn>& spawns);
	Bomberman* GetPlayer(UINT playerID) const;
	void ResetPlayers(bool fullReset = false);
	std::map<UINT, PlayerColor>* GetPlayersForHUD();

private:
	enum class GameState
	{
		WarmUp,
		Running,
		RoundOver,
		GameOver,
		TieGame
	};
	BombermanScene* m_pGameScene;
	
	BombManager m_BombManager;
	std::vector<Bomberman*> m_Players;
	std::vector<UINT> m_TextureIDs;

	GameState m_GameState;
	float m_WarmUpDelay;
	float m_GameOverTimer;
	float m_RoundOverDelay;
	float m_GameOverDelay;

	static FMOD::Sound* m_pAnnouncerReadySound;
	static FMOD::Sound* m_pAnnouncerGoSound;
	static FMOD::Sound* m_pCroudWinSound;
	static FMOD::Channel* m_pSoundChannel;
	
	void LoadPlayerTextures();
	void HandleRoundOver(float deltaTime);
};

// Used the following source to make this:
// https://stackoverflow.com/questions/5093460/how-to-convert-an-enum-type-variable-to-a-string
inline const wchar_t* ToString(PlayerColor v)
{
	switch (v)
	{
	case PlayerColor::White:
		return L"White";
	case PlayerColor::Black:
		return L"Black";
	case PlayerColor::Red:
		return L"Red";
	case PlayerColor::Green:
		return L"Green";
	default:
		return L"White";
	}
}