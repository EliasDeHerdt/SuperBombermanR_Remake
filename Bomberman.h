#pragma once
#include "GameObject.h"

class GameScene;
class ModelComponent;
class ControllerComponent;
class ParticleEmitterComponent;
class Node;
class BombManager;
class BombermanScene;
enum class BombType;
enum class Direction;
enum class PlayerColor;

struct PlayerInput
{
	char Up, Down, Left, Right, Bomb;
};

class Bomberman : public GameObject
{
public:
	enum class CharacterInputs
	{
		Left = 0,
		Right,
		Up,
		Down,
		Bomb,
		SizeBuffer
	};
	
	Bomberman(BombermanScene* scene, BombManager& bombManager, UINT playerID, PlayerColor color, UINT materialID, PlayerInput inputLayout);
	virtual ~Bomberman() = default;

	Bomberman(const Bomberman& other) = delete;
	Bomberman(Bomberman&& other) noexcept = delete;
	Bomberman& operator=(const Bomberman& other) = delete;
	Bomberman& operator=(Bomberman&& other) noexcept = delete;

	void Initialize(const GameContext& gameContext) override;
	void Update(const GameContext& gameContext) override;
	
	bool IsAlive() const;
	UINT GetScore() const;
	UINT GetPlayerID() const;
	PlayerColor GetPlayerColor() const;
	void Die();
	void AddBomb();
	void AddRange();
	void AddSpeed();
	void IncrementScore();
	void PlaceBomb();
	void SetCurrentNode(Node* node);
	void SetLocation(DirectX::XMFLOAT2 pos) const;
	void SetAnimation(UINT clipNumber);
	void ResetPlayer(bool fullReset = false);
	void SetPlayerControl(bool state);
	void SetBombType(BombType type);
private:
	BombermanScene* m_pGameScene;
	
	GameObject* m_pModel;
	ModelComponent* m_pModelComponent;
	ControllerComponent* m_pController;
	ParticleEmitterComponent* m_pParticleEmitter;
	
	Node* m_pCurrentNode;
	BombManager& m_pBombManager;

	UINT m_PlayerID;
	UINT m_MaterialID;
	UINT m_AnimationIndex;
	PlayerInput m_InputLayout;
	PlayerColor m_PlayerColor;
	
	UINT m_Score;
	UINT m_BombRange;
	UINT m_BaseBombRange;
	UINT m_BombCounter;
	UINT m_BaseBombCounter;
	BombType m_BombType;
	bool m_IsAlive;
	bool m_AllowPlayerControl;
	float m_SpeedBoost;

	float m_DeathJumpSpeed;
	float m_MaxRunVelocity;
	float m_RunAccelerationTime;
	float m_RunAcceleration;
	float m_RunVelocity;
	float m_CapsuleHeight;

	DirectX::XMFLOAT3 m_Velocity;

	static FMOD::Sound* m_pDeathSound;
	static FMOD::Sound* m_pCroudDeathSound;
	static FMOD::Channel* m_pSoundChannel;

	void CheckCurrentNode();
	void MoveToNeighboringNode(Direction direction);
};

	