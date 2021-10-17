#pragma once
#include <queue>
#include <vector>

class Node;
class PickUp;
class BombermanScene;
enum class PickUpType;

class PickUpManager
{
public:
	PickUpManager();
	virtual ~PickUpManager();

	PickUpManager(const PickUpManager& other) = delete;
	PickUpManager(PickUpManager&& other) noexcept = delete;
	PickUpManager& operator=(const PickUpManager& other) = delete;
	PickUpManager& operator=(PickUpManager&& other) noexcept = delete;

	void Initialize(BombermanScene* scene, UINT poolSize);
	void Update();

	void RequestPickUp(Node* node);
	void RemoveAllPickUp();

private:
	struct PickUpDropChance
	{
		PickUpType type;
		int influence;
	};
	
	BombermanScene* m_pGameScene;
	int m_TotalDropChance;

	std::queue<PickUp*> m_PickUpPool;
	std::vector<PickUp*> m_ActivePickUps;
	std::vector<UINT> m_TextureIDs;
	std::vector<PickUpDropChance> m_DropChances;

	void SetDropChances();
	void LoadPickUpTextures();
};

