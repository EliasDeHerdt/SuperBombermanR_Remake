#include "stdafx.h"
#include "Button.h"

#include "GameScene.h"
#include "SpriteComponent.h"
#include "TransformComponent.h"
#include "DataTypes.h"
#include "SoundManager.h"
#include "TextureData.h"

bool Button::m_MouseControl{ true };
bool Button::m_MouseClicked{ false };
FMOD::Sound* Button::m_pActiveSound{ nullptr };
FMOD::Sound* Button::m_pHoverSound{ nullptr };
FMOD::Channel* Button::m_pSoundChannel{ nullptr };

Button::Button(GameScene* scene, DirectX::XMFLOAT2 position, std::wstring basicPath, std::wstring hoveredPath)
	: GameObject()
	, m_pGameScene(scene)
	, m_Active(false)
	, m_SkipUpdate(false)
	, m_HasUserFocus(false)
	, m_FocusChangeCheck(false)
	, m_pBasicButton(nullptr)
	, m_pHoveredButton(nullptr)
	, m_BasicPath(basicPath)
	, m_HoveredPath(hoveredPath)
	, m_Position(position)
	, m_TopNeighbor(nullptr)
	, m_BottomNeighbor(nullptr)
	, m_LeftNeighbor(nullptr)
	, m_RightNeighbor(nullptr)
{
}

Button::~Button()
{
}

void Button::Initialize(const GameContext& gameContext)
{
	m_pBasicButton = new GameObject();
	m_pBasicButton->AddComponent(new SpriteComponent(m_BasicPath));
	m_pBasicButton->GetTransform()->Translate(m_Position.x, m_Position.y, 0.f);

	m_pHoveredButton = new GameObject();
	m_pHoveredButton->AddComponent(new SpriteComponent(m_HoveredPath));
	m_pHoveredButton->GetTransform()->Translate(m_Position.x, m_Position.y, 0.f);

	AddChild(m_pBasicButton);
	AddChild(m_pHoveredButton);

	gameContext.pInput->AddInputAction(InputAction(90, InputTriggerState::Pressed, VK_RETURN));
	gameContext.pInput->AddInputAction(InputAction(91, InputTriggerState::Pressed, 'W'));
	gameContext.pInput->AddInputAction(InputAction(92, InputTriggerState::Pressed, VK_UP));
	gameContext.pInput->AddInputAction(InputAction(93, InputTriggerState::Pressed, 'S'));
	gameContext.pInput->AddInputAction(InputAction(94, InputTriggerState::Pressed, VK_DOWN));
	gameContext.pInput->AddInputAction(InputAction(95, InputTriggerState::Pressed, 'A'));
	gameContext.pInput->AddInputAction(InputAction(96, InputTriggerState::Pressed, VK_LEFT));
	gameContext.pInput->AddInputAction(InputAction(97, InputTriggerState::Pressed, 'D'));
	gameContext.pInput->AddInputAction(InputAction(98, InputTriggerState::Pressed, VK_RIGHT));

	// Sound
	//******
	if (!m_pHoverSound
		|| !m_pSoundChannel)
	{
		SoundManager::GetInstance()->GetSystem()->createSound("Resources/Audio/Bomberman/ButtonActive.mp3", FMOD_2D, 0, &m_pActiveSound);
		SoundManager::GetInstance()->GetSystem()->createSound("Resources/Audio/Bomberman/ButtonSelect.mp3", FMOD_2D, 0, &m_pHoverSound);
		SoundManager::GetInstance()->GetSystem()->getChannel(2, &m_pSoundChannel);
	}
}

void Button::Update(const GameContext&)
{
	// Reset checks
	m_Active = false;
	m_FocusChangeCheck = false;

	// Set the visibility of the buttons
	m_pBasicButton->SetVisibility(!m_HasUserFocus);
	m_pHoveredButton->SetVisibility(m_HasUserFocus);

	// Check if the mouse moved last frame
	auto currMousePos = m_pGameScene->GetGameContext().pInput->GetMousePosition();
	auto prevMousePos = m_pGameScene->GetGameContext().pInput->GetMousePosition(true);
	if (currMousePos.x != prevMousePos.x
		|| currMousePos.y != prevMousePos.y)
		m_MouseControl = true;

	// Check if the mouse hovers over the button
	if (!m_HasUserFocus
		&& CheckMouseInsideButton())
		SetUserFocus(true);
	
	// Make sure changing the focused button can only happen once per frame!
	// If the button has no user focus, there is no need to update any further
	if (!m_HasUserFocus
		|| m_SkipUpdate)
	{
		m_SkipUpdate = false;
		return;
	}

	// Activate the button
	if (m_pGameScene->GetGameContext().pInput->IsActionTriggered(90)
		|| (m_pGameScene->GetGameContext().pInput->IsMouseButtonDown(VK_LBUTTON)
			&& CheckMouseInsideButton()
			&& !m_MouseClicked))
	{
		m_Active = true;
		SoundManager::GetInstance()->GetSystem()->playSound(m_pActiveSound, 0, false, &m_pSoundChannel);
	}

	// Move between the buttons
		// Move Up
	else if (m_pGameScene->GetGameContext().pInput->IsActionTriggered(91)
		|| m_pGameScene->GetGameContext().pInput->IsActionTriggered(92))
		MoveToNeighbor(Direction::Up);

		// Move Down
	else if (m_pGameScene->GetGameContext().pInput->IsActionTriggered(93)
		|| m_pGameScene->GetGameContext().pInput->IsActionTriggered(94))
		MoveToNeighbor(Direction::Down);

		// Move Left
	else if (m_pGameScene->GetGameContext().pInput->IsActionTriggered(95)
		|| m_pGameScene->GetGameContext().pInput->IsActionTriggered(96))
		MoveToNeighbor(Direction::Left);

		// Move Right
	else if (m_pGameScene->GetGameContext().pInput->IsActionTriggered(97)
		|| m_pGameScene->GetGameContext().pInput->IsActionTriggered(98))
		MoveToNeighbor(Direction::Right);

	// Check if the left mouse button is down for future frames
	m_MouseClicked = m_pGameScene->GetGameContext().pInput->IsMouseButtonDown(VK_LBUTTON);
}

void Button::SetUserFocus(bool state, bool forceTake)
{
	m_HasUserFocus = state;
	m_SkipUpdate = true;
	m_FocusChangeCheck = true;
	
	if (state)
		SoundManager::GetInstance()->GetSystem()->playSound(m_pHoverSound, 0, false, &m_pSoundChannel);

	if (!forceTake)
		return;
	
	if (m_TopNeighbor && !m_TopNeighbor->m_FocusChangeCheck)
		m_TopNeighbor->SetUserFocus(false);
	
	if (m_BottomNeighbor && !m_BottomNeighbor->m_FocusChangeCheck)
		m_BottomNeighbor->SetUserFocus(false);
	
	if (m_LeftNeighbor && !m_LeftNeighbor->m_FocusChangeCheck)
		m_LeftNeighbor->SetUserFocus(false);
	
	if (m_RightNeighbor && !m_RightNeighbor->m_FocusChangeCheck)
		m_RightNeighbor->SetUserFocus(false);
}

bool Button::HasUserFocus() const
{
	return m_HasUserFocus;
}

bool Button::IsActive() const
{
	return m_Active;
}

void Button::LinkNeighbor(Button* neighbor, Direction direction)
{
	switch (direction)
	{
	case Direction::Up:
		m_TopNeighbor = neighbor;
		break;
	case Direction::Down:
		m_BottomNeighbor = neighbor;
		break;
	case Direction::Left:
		m_LeftNeighbor = neighbor;
		break;
	case Direction::Right:
		m_RightNeighbor = neighbor;
		break;
	default:
		break;
	}
}

void Button::MoveToNeighbor(Direction direction)
{
	m_MouseControl = false;
	switch (direction)
	{
	case Direction::Up:
		if (m_TopNeighbor)
			m_TopNeighbor->SetUserFocus(true);
		break;
	case Direction::Down:
		if (m_BottomNeighbor)
			m_BottomNeighbor->SetUserFocus(true);
		break;
	case Direction::Left:
		if (m_LeftNeighbor)
			m_LeftNeighbor->SetUserFocus(true);
		break;
	case Direction::Right:
		if (m_RightNeighbor)
			m_RightNeighbor->SetUserFocus(true);
		break;
	default:
		break;
	}
}

bool Button::CheckMouseInsideButton() const
{
	if (!m_MouseControl)
		return false;
	
	auto position = m_Position;
	auto dimensions = m_pBasicButton->GetComponent<SpriteComponent>()->GetTexture()->GetDimension();
	auto mousePos = m_pGameScene->GetGameContext().pInput->GetMousePosition();
	
	if (mousePos.x >= position.x
		&& mousePos.x <= position.x + dimensions.x
		&& mousePos.y >= position.y
		&& mousePos.y <= position.y + dimensions.y)
		return true;
	
	return false;
}
