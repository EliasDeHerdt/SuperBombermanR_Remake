#pragma once
#include "GameObject.h"
#include <vector>

enum class Direction;
class PickUp;
class Bomberman;
class GameScene;
class PickUpManager;
class BombermanScene;
class ColliderComponent;
class ParticleBurstEmitterComponent;

enum class NodeType
{
	Empty,
	Brittle,
	Solid
};

class Node : public GameObject
{
public:
	Node(BombermanScene* scene, PickUpManager& pickUpManager, NodeType type, float width, float height, DirectX::XMFLOAT2 position, UINT brittleTextureID, UINT solidTextureID);
	virtual ~Node() = default;

	Node(const Node& other) = delete;
	Node(Node&& other) noexcept = delete;
	Node& operator=(const Node& other) = delete;
	Node& operator=(Node&& other) noexcept = delete;

	void Initialize(const GameContext& gameContext) override;
	void Update(const GameContext&) override;

	PickUp* GetPickUp() const;
	const DirectX::XMFLOAT2& GetPosition() const;
	DirectX::XMFLOAT3 GetDimensions() const;
	Node* GetNeighbor(Direction direction) const;
	void ChangeNodeType(NodeType newType);
	void NotifyDamageTaken(Direction damageDirection, int steps, bool penetrate = false);
	void SetHasBomb(bool state);
	bool GetHasBomb() const;
	void LinkToNode(Node* node, Direction direction);
	void AddPlayerToNode(Bomberman* player);
	void RemovePlayerFromNode(Bomberman* player);
	void SetPickUp(PickUp* powerUp);
private:
	// When working with models or textures, add them as a static
	BombermanScene* m_pGameScene;
	
	NodeType m_NodeType;
	PickUpManager& m_PickUpManager;
	std::vector<Bomberman*> m_PlayersInNode;
	const int m_DropChance;

	float m_Width;
	float m_Height;
	bool m_HasBomb;
	PickUp* m_pPowerUp;
	DirectX::XMFLOAT2 m_Position;
	
	GameObject* m_pSolidWall;
	GameObject* m_pBrittleWall;

	UINT m_BrittleTextureID;
	UINT m_SolidTextureID;

	ColliderComponent* m_pCollider;
	ParticleBurstEmitterComponent* m_pParticleEmitter;
	
	Node* m_pLeftNeighbor;
	Node* m_pRightNeighbor;
	Node* m_pTopNeighbor;
	Node* m_pBottomNeighbor;

	void UpdateNode() const;
};