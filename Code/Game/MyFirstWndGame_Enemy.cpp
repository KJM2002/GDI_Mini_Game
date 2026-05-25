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

void MyFirstWndGame::CreateCircleEnemy()
{
	if (m_CirEnemySpawnPos.x == 0 && m_CirEnemySpawnPos.y == 0)
	{
		return;
	}

	CreateCircleEnemy(
		static_cast<float>(m_CirEnemySpawnPos.x),
		static_cast<float>(m_CirEnemySpawnPos.y)
	);

	m_CirEnemySpawnPos = { 0, 0 };
}


void MyFirstWndGame::CreateCircleEnemy(float x, float y)
{
	// ENEMY 타입 게임 오브젝트 생성
	GameObject* pNewObject = new GameObject(ObjectType::ENEMY);

	pNewObject->SetName("Enemy");

	pNewObject->SetPosition(x, y);

	pNewObject->SetSpeed(0.16f);

	// 적은 생성 시점의 플레이어 위치를 향해 방향만 정한다.
	GameObject* pPlayer = GetPlayer();

	if (pPlayer != nullptr)
	{
		Vector2f playerPos = pPlayer->GetPosition();
		Vector2f enemyPos = pNewObject->GetPosition();

		Vector2f enemyDir = playerPos - enemyPos;

		if (enemyDir.LengthSquared() > 0.0f)
		{
			enemyDir.Normalize();
		}

		pNewObject->SetDirection(enemyDir);
	}

	pNewObject->SetColliderCircle(35.0f);

	pNewObject->SetBitmapInfo(m_pEnemyBitmapInfo);

	pNewObject->SetFrameInfo(500, 500, 1, 1);

	pNewObject->SetRenderSize(110, 110);


	bool isInter = false;

	learning::Collider* thisCollider = nullptr;

	// 충돌 보정 루프
	// 새로 생성된 적이 기존 오브젝트와 겹치는지 검사합니다.
	int j = 0;

	// 무한 루프 방지 카운터
	int t = 0;

	while (j < MAX_GAME_OBJECT_COUNT)
	{
		thisCollider = pNewObject->GetCollider();

		GameObjectBase* target = m_GameObjectPtrTable[j];

		// 더 이상 오브젝트가 없으면 종료
		if (target == nullptr)
			break;

		learning::Collider* targetCollider = target->GetCollider();

		// 충돌 중이면 위치를 보정
		if (thisCollider->IsIntersect(targetCollider))
		{
			// 원형 충돌 보정 함수
			SettingCirPos(thisCollider, targetCollider, pNewObject);

			// 위치가 바뀌었으므로 처음부터 다시 검사
			j = 0;

			++t;

			// 특정 횟수 이상 반복되면
			// 빠져나오지 못하는 상황이라고 판단
			if (t == MAX_GAME_OBJECT_COUNT)
			{
				isInter = true;
				break;
			}

			continue;
		}

		++j;
	}

	// 빈 슬롯을 찾아 적 저장
	// 0번은 플레이어이므로 1번부터 사용
	int i = 0;

	while (++i < MAX_GAME_OBJECT_COUNT && !isInter)
	{
		if (nullptr == m_GameObjectPtrTable[i])
		{
			m_GameObjectPtrTable[i] = pNewObject;
			break;
		}
	}

	// 저장 실패 또는 충돌 해결 실패 시 삭제
	if (i == MAX_GAME_OBJECT_COUNT || isInter)
	{
		delete pNewObject;
		pNewObject = nullptr;

		isInter = false;
	}
}


void MyFirstWndGame::CreateBoxEnemy()
{
	GameObject* pNewObject = new GameObject(ObjectType::ENEMY);

	pNewObject->SetName("Enemy");

	float x = m_BoxEnemySpawnPos.x;
	float y = m_BoxEnemySpawnPos.y;

	m_BoxEnemySpawnPos = { 0, 0 };

	pNewObject->SetPosition(x, y);

	pNewObject->SetSpeed(0.3f);

	// 박스 형태 콜라이더 생성
	pNewObject->SetColliderBox(50.0f, 50.0f);

	pNewObject->SetBitmapInfo(m_pEnemyBitmapInfo);

	pNewObject->SetFrameInfo(183, 168, 14, 5);

	pNewObject->SetRenderSize(90, 83);

	bool isInter = false;

	learning::Collider* thisCollider = nullptr;

	// 처음 충돌 방향 저장용
	Vector2f firstDir;

	int j = 0;
	int t = 0;

	while (j < MAX_GAME_OBJECT_COUNT)
	{
		thisCollider = pNewObject->GetCollider();

		GameObjectBase* target = m_GameObjectPtrTable[j];

		if (target == nullptr)
			break;

		learning::Collider* targetCollider = target->GetCollider();

		if (thisCollider->IsIntersect(targetCollider))
		{
			// 첫 충돌 방향 저장
			// Box는 방향이 계속 바뀌면 튕김 현상이 심해질 수 있으므로
			// 최초 방향 기준으로 밀어냅니다.
			if (firstDir.x == 0 && firstDir.y == 0)
			{
				firstDir = GetBoxDir(thisCollider, targetCollider);
			}

			// 박스 충돌 보정
			SettingBoxPos(thisCollider, targetCollider, pNewObject, firstDir);

			// 위치가 바뀌었으므로 다시 전체 검사
			j = 0;

			++t;

			// 무한 루프 방지
			if (t == MAX_GAME_OBJECT_COUNT)
			{
				isInter = true;
				break;
			}

			continue;
		}

		++j;
	}

	// 빈 슬롯에 저장
	int i = 0;

	while (++i < MAX_GAME_OBJECT_COUNT && !isInter)
	{
		if (nullptr == m_GameObjectPtrTable[i])
		{
			m_GameObjectPtrTable[i] = pNewObject;
			break;
		}
	}

	// 실패 시 메모리 해제
	if (i == MAX_GAME_OBJECT_COUNT || isInter)
	{
		delete pNewObject;
		pNewObject = nullptr;

		isInter = false;
	}
}


void MyFirstWndGame::UpdateEnemyInfo()
{
	// 적은 생성 시점에 이동 방향이 정해진다.
   // 이후에는 플레이어의 현재 위치를 추적하지 않는다.
}


void MyFirstWndGame::KillEnemy(int index, int score)
{
	if (index <= 0 || index >= MAX_GAME_OBJECT_COUNT)
	{
		return;
	}

	GameObjectBase* enemy = m_GameObjectPtrTable[index];

	if (enemy == nullptr)
	{
		return;
	}

	if (enemy->Type() != ObjectType::ENEMY)
	{
		return;
	}

	delete enemy;
	m_GameObjectPtrTable[index] = nullptr;

	++m_killCount;
	m_score += score;
}


void MyFirstWndGame::RemoveOutOfScreenEnemies()
{
	constexpr float MARGIN = 120.0f;

	for (int i = 1; i < MAX_GAME_OBJECT_COUNT; ++i)
	{
		GameObjectBase* obj = m_GameObjectPtrTable[i];

		if (obj == nullptr)
		{
			continue;
		}

		if (obj->Type() != ObjectType::ENEMY)
		{
			continue;
		}

		Vector2f pos = obj->GetPosition();

		if (pos.x < -MARGIN ||
			pos.x > m_width + MARGIN ||
			pos.y < -MARGIN ||
			pos.y > m_height + MARGIN)
		{
			delete obj;
			m_GameObjectPtrTable[i] = nullptr;
		}
	}
}


void MyFirstWndGame::UpdateEnemySpawn()
{
	m_enemySpawnTimer += m_fDeltaTime * 0.001f;

	if (m_enemySpawnTimer < m_enemySpawnInterval)
	{
		return;
	}

	m_enemySpawnTimer = 0.0f;

	const int margin = 60;

	int edge = rand() % 4;

	float x = 0.0f;
	float y = 0.0f;

	switch (edge)
	{
	case 0: // 위쪽
		x = static_cast<float>(rand() % m_width);
		y = static_cast<float>(-margin);
		break;

	case 1: // 아래쪽
		x = static_cast<float>(rand() % m_width);
		y = static_cast<float>(m_height + margin);
		break;

	case 2: // 왼쪽
		x = static_cast<float>(-margin);
		y = static_cast<float>(rand() % m_height);
		break;

	case 3: // 오른쪽
		x = static_cast<float>(m_width + margin);
		y = static_cast<float>(rand() % m_height);
		break;
	}

	CreateCircleEnemy(x, y);
}

