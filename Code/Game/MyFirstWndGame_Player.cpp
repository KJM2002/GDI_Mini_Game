#include "../Core/INC_Windows.h"
#include "MyFirstWndGame.h"
#include "GameConstants.h"
#include "../Core/GameTimer.h"
#include "../Engine/Collider.h"
#include "../Engine/GameObject.h"
#include "../Engine/renderHelp.h"
#include <iostream>
#include <assert.h>
#include <cstdlib>
#include <ctime>

using namespace learning;

void MyFirstWndGame::CreatePlayer()
{
	// assert 함수는 디버깅 모드에서 개발자가 오류가 생기면 치명적일 것이라는 곳에 심어 놓는 에러 검출용 코드
	assert(m_GameObjectPtrTable[0] == nullptr && "Player object already exists!");

	// 플레이어 타입 게임 오브젝트 생성
	GameObject* pNewObject = new GameObject(ObjectType::PLAYER);

	pNewObject->SetName("Player");
	pNewObject->SetPosition(m_width * 0.5f, m_height * 0.5f);
	pNewObject->SetSpeed(0.3f);

	
	const float PLAYER_RADIUS = 35.0f;
	//const int PLAYER_RENDER_SIZE = 100;
	// 가로 세로 비율이 달라서 이렇게 하니까 이미지가 찌그러지는 문제 발생

	pNewObject->SetColliderCircle(PLAYER_RADIUS);

	pNewObject->SetBitmapInfo(m_pPlayerBitmapInfo);
	pNewObject->SetFrameInfo(532, 469, 1, 1);

	pNewObject->SetRenderSize(100, 90);

	m_GameObjectPtrTable[0] = pNewObject;				// 플레이어는 항상 0번 인덱스에 저장합니다.
}


void MyFirstWndGame::UpdatePlayerInfo()
{
	GameObject* pPlayer = GetPlayer();

	assert(pPlayer != nullptr);

	float deltaTime = m_fDeltaTime;

	Vector2f inputDir(0.0f, 0.0f);

	if (m_keyUp)
	{
		inputDir.y -= 1.0f;
	}

	if (m_keyDown)
	{
		inputDir.y += 1.0f;
	}

	if (m_keyLeft)
	{
		inputDir.x -= 1.0f;
	}

	if (m_keyRight)
	{
		inputDir.x += 1.0f;
	}

	if (inputDir.LengthSquared() > 0.0f)
	{
		inputDir.Normalize();

		// 키를 누르고 있으면 해당 방향으로 가속
		m_playerVelocity += inputDir * (m_playerAcceleration * deltaTime);
	}
	else
	{
		// 키를 떼면 마찰로 서서히 감속
		float speed = m_playerVelocity.Length();

		if (speed > 0.0f)
		{
			float drop = m_playerFriction * deltaTime;
			float newSpeed = speed - drop;

			if (newSpeed < 0.0f)
			{
				newSpeed = 0.0f;
			}

			if (speed > 0.0f)
			{
				m_playerVelocity = m_playerVelocity.Normalized() * newSpeed;
			}
		}
	}

	// 최대 속도 제한
	float currentSpeed = m_playerVelocity.Length();

	if (currentSpeed > m_playerMaxSpeed)
	{
		m_playerVelocity = m_playerVelocity.Normalized() * m_playerMaxSpeed;
	}

	Vector2f playerPos = pPlayer->GetPosition();

	playerPos += m_playerVelocity * deltaTime;

	constexpr float PLAYER_RADIUS = 35.0f;

	// 화면 밖으로 나가지 않도록 제한
	if (playerPos.x < PLAYER_RADIUS)
	{
		playerPos.x = PLAYER_RADIUS;
		m_playerVelocity.x = 0.0f;
	}

	if (playerPos.x > m_width - PLAYER_RADIUS)
	{
		playerPos.x = m_width - PLAYER_RADIUS;
		m_playerVelocity.x = 0.0f;
	}

	if (playerPos.y < PLAYER_RADIUS)
	{
		playerPos.y = PLAYER_RADIUS;
		m_playerVelocity.y = 0.0f;
	}

	if (playerPos.y > m_height - PLAYER_RADIUS)
	{
		playerPos.y = m_height - PLAYER_RADIUS;
		m_playerVelocity.y = 0.0f;
	}

	pPlayer->SetPosition(playerPos.x, playerPos.y);

	// 기존 GameObject::Move()가 또 이동시키지 않도록 방향은 0으로 둔다.
	pPlayer->SetDirection(Vector2f(0.0f, 0.0f));
}

