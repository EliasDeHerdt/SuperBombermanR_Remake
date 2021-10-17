#pragma once
#include "PickUpManager.h"
#include <vector>


class Node;
struct PlayerSpawn
{
	UINT playerID{ 0 };
	Node* node{ nullptr };
};

class NodeGrid
{
public:
	NodeGrid();
	virtual ~NodeGrid();

	NodeGrid(const NodeGrid& other) = delete;
	NodeGrid(NodeGrid&& other) noexcept = delete;
	NodeGrid& operator=(const NodeGrid& other) = delete;
	NodeGrid& operator=(NodeGrid&& other) noexcept = delete;

	void Initialize(BombermanScene* scene, float nodeWidth, float nodeHeight, std::string levelFile);
	void Update(float deltaTime);

	void LoadLevel();
	void SetLevelPath(std::string levelPath);
	const std::vector<PlayerSpawn>& GetSpawns() const;

private:
	BombermanScene* m_pGameScene;
	PickUpManager m_PickUpManager;
	
	float m_NodeWidth;
	float m_NodeHeight;
	float m_LevelCols;
	float m_LevelRows;

	std::string m_levelFile;
	std::vector<PlayerSpawn> m_PlayerSpawns;
	std::vector<std::vector<Node*>> m_Grid;
	std::vector<UINT> m_TextureIDs;
	
	void LinkNodes() const;
	void ClearLevel();
	void LoadNodeTextures();
};

