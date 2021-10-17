#pragma once
#include <queue>
#include <vector>

class Node;
class Bomb;
class Bomberman;
class BombermanScene;
enum class BombType;

class BombManager
{
public:
	BombManager();
	virtual ~BombManager();

	BombManager(const BombManager& other) = delete;
	BombManager(BombManager&& other) noexcept = delete;
	BombManager& operator=(const BombManager& other) = delete;
	BombManager& operator=(BombManager&& other) noexcept = delete;

	void Initialize(BombermanScene* scene, UINT poolSize);
	void Update();
	
	void RequestBomb(Node* node, Bomberman* owner, UINT range, BombType bombType);
	void RemoveAllBombs();
private:
	BombermanScene* m_pGameScene;
	
	std::queue<Bomb*> m_BombPool;
	std::queue<Bomb*> m_ActiveBombs;
	std::vector<UINT> m_TextureIDs;

	void LoadBombTextures();
};

