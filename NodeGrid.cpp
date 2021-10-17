#include "stdafx.h"
#include "NodeGrid.h"
#include "Node.h"

#include <string>
#include <istream>
#include <regex>

#include "BombermanScene.h"
#include "DataTypes.h"
#include "../../Materials/Shadow/DiffuseMaterial_Shadow.h"

// A basic level should always exist of uneven numbers!
// Maybe load this in from a file
NodeGrid::NodeGrid()
	: m_pGameScene()
	, m_PickUpManager()
	, m_NodeWidth()
	, m_NodeHeight()
	, m_LevelCols()
	, m_LevelRows()
	, m_levelFile()
	, m_PlayerSpawns()
	, m_Grid()
{
}

// Nodes are added as gameobjects, causing the scene to clean the grid up for us
NodeGrid::~NodeGrid()
{
}

void NodeGrid::Initialize(BombermanScene* scene, float nodeWidth, float nodeHeight, std::string levelFile)
{
	m_pGameScene = scene;
	m_NodeWidth = nodeWidth;
	m_NodeHeight = nodeHeight;
	m_levelFile = levelFile;

	m_PickUpManager.Initialize(scene, 200);
	
	LoadNodeTextures();
	LoadLevel();
}

void NodeGrid::Update(float)
{
	if (m_pGameScene->IsGamePaused())
		return;
	
	m_PickUpManager.Update();
}

void NodeGrid::LoadLevel()
{
	std::ifstream input{ m_levelFile };
	int rowIndex{ 0 }, colIndex{ 0 };

	std::string headerCheck{};
	std::regex sizeCheck{ "^[CR]=(\\d+)" };
	std::smatch match{};

	// Check if the header contains the information needed for the width & the height
	std::getline(input, headerCheck);
	if (std::regex_match(headerCheck, match, sizeCheck))
		m_LevelCols = std::stof(match[1]);

	std::getline(input, headerCheck);
	if (std::regex_match(headerCheck, match, sizeCheck))
		m_LevelRows = std::stof(match[1]);

	//Push the first layer and set the start pos for the grid nodes
	ClearLevel();
	m_Grid.push_back(std::vector<Node*>{});
	DirectX::XMFLOAT2 startPos = {-int(m_LevelCols / 2.f) * m_NodeWidth, int(m_LevelRows / 2.f) * m_NodeWidth };
	
	do
	{
		NodeType type{ };
		char character = (char)input.get();
		
		if (!input)
			break;
		
		switch (character)
		{
		case 'X':
			type = NodeType::Solid;
			break;
		case 'O':
			type = NodeType::Brittle;
			break;
		case '\n':
			// Go to the next line and skip adding a node
			m_Grid.push_back(std::vector<Node*>{});
			++rowIndex;
			colIndex = 0;
			continue;
		case '-':
		default:
			type = NodeType::Empty;
			break;
		}

		Node* node = new Node{ m_pGameScene, m_PickUpManager, type, m_NodeWidth, m_NodeHeight, {startPos.x + (float)colIndex * m_NodeWidth, startPos.y - (float)rowIndex * m_NodeWidth}, m_TextureIDs[0], m_TextureIDs[1] };
		m_Grid[rowIndex].push_back(node);

		// If a char is a number and you subtract it by '0', you'll get the actual number (only works for 0 - 9!)
		if (std::isdigit(character))
			m_PlayerSpawns.push_back(PlayerSpawn{ UINT(character - '0'), node });
		
		++colIndex;
		
	} while (true);

	LinkNodes();
}

void NodeGrid::SetLevelPath(std::string levelPath)
{
	m_levelFile = levelPath;
}

const std::vector<PlayerSpawn>& NodeGrid::GetSpawns() const
{
	return m_PlayerSpawns;
}

void NodeGrid::LinkNodes() const
{
	for (int i = 0; i < m_LevelRows; ++i)
	{
		for (int j = 0; j < m_LevelCols; ++j)
		{
			if (i > 0)
				m_Grid[i][j]->LinkToNode(m_Grid[i - 1][j], Direction::Up);
			if (i < m_LevelRows - 1)
				m_Grid[i][j]->LinkToNode(m_Grid[i + 1][j], Direction::Down);
			if (j > 0)
				m_Grid[i][j]->LinkToNode(m_Grid[i][j - 1], Direction::Left);
			if (j < m_LevelCols - 1)
				m_Grid[i][j]->LinkToNode(m_Grid[i][j + 1], Direction::Right);
			m_pGameScene->AddChild(m_Grid[i][j]);
		}
	}
}

void NodeGrid::ClearLevel()
{
	m_PickUpManager.RemoveAllPickUp();
	
	for (auto row : m_Grid)
		for (auto col : row)
			m_pGameScene->RemoveChild(col);
	
	m_Grid.clear();
	m_PlayerSpawns.clear();
}

void NodeGrid::LoadNodeTextures()
{
	auto gameContext = m_pGameScene->GetGameContext();

	auto diffMat = new DiffuseMaterial_Shadow();
	diffMat->Initialize(gameContext);
	diffMat->SetDiffuseTexture(L"./Resources/Textures/Bomberman/Level/BrittleWall.png");
	diffMat->SetLightDirection(gameContext.pShadowMapper->GetLightDirection());

	int index = gameContext.pMaterialManager->AddMaterial(diffMat);
	if (index != -1)
		m_TextureIDs.push_back(index);

	diffMat = new DiffuseMaterial_Shadow();
	diffMat->Initialize(gameContext);
	diffMat->SetDiffuseTexture(L"./Resources/Textures/Bomberman/Level/SolidWall.png");
	diffMat->SetLightDirection(gameContext.pShadowMapper->GetLightDirection());

	index = gameContext.pMaterialManager->AddMaterial(diffMat);
	if (index != -1)
		m_TextureIDs.push_back(index);
}
