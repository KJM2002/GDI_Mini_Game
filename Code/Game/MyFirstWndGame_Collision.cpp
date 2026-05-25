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

void MyFirstWndGame::SettingCirPos(
	learning::Collider* thisCir,
	learning::Collider* targetCir,
	GameObject* pThis)
{
	// Collider를 ColliderCircle 타입으로 변환
	auto a = dynamic_cast<ColliderCircle*>(thisCir);

	float radius = a->radius;

	Vector2f thisPos = thisCir->center;
	Vector2f targetPos = targetCir->center;

	// 타겟 -> 현재 오브젝트 방향 벡터
	Vector2f dir = thisPos - targetPos;

	/*
		radius * 2 :
		두 원이 딱 닿아 있어야 하는 최소 거리

		dir.Length():
		현재 두 원 중심 사이 거리

		(1 - 현재거리 / 최소거리):
		얼마나 겹쳤는지 비율 계산
	*/

	// 0.5f는 float 오차 보정
	thisCir->center +=
		dir.Normalized() *
		((radius * 2 + 0.5f) *
			(1 - (dir.Length() / (radius * 2))));

	// 실제 게임 오브젝트 위치도 갱신
	pThis->SetPosition(thisCir->center.x, thisCir->center.y);
}


Vector2f MyFirstWndGame::GetBoxDir(
	learning::Collider* thisBox,
	learning::Collider* targetBox)
{
	auto a = dynamic_cast<ColliderBox*>(thisBox);

	float boxSize = a->halfSize.x * 2;

	Vector2f thisPos = thisBox->center;
	Vector2f targetPos = targetBox->center;

	// 타겟 기준 현재 오브젝트 방향
	Vector2f dir = thisPos - targetPos;

	// 방향 벡터만 반환
	return dir.Normalized();
}


void MyFirstWndGame::SettingBoxPos(
	learning::Collider* thisBox,
	learning::Collider* targetBox,
	GameObject* pThis,
	Vector2f firstDir)
{
	auto a = dynamic_cast<ColliderBox*>(thisBox);

	float boxSize = a->halfSize.x * 2;

	Vector2f thisPos = thisBox->center;
	Vector2f targetPos = targetBox->center;

	Vector2f dir = thisPos - targetPos;

	// 두 박스 중심 거리
	float mScala = dir.Length();

	float x = targetBox->center.x;
	float y = targetBox->center.y;

	/*
		1.41f는 sqrt(2)에 가까운 값입니다.
		정사각형 대각선 길이를 근사하기 위해 사용합니다.
	*/

	thisBox->center +=
		firstDir *
		((boxSize * 1.41f) *
			(1 - (mScala / (boxSize * 1.41f))));

	/*
		위치가 너무 멀리 튀지 않도록 Clamp 처리
	*/

	thisBox->center.x =
		FClamp(thisBox->center.x,
			x - boxSize,
			x + boxSize);

	thisBox->center.y =
		FClamp(thisBox->center.y,
			y - boxSize,
			y + boxSize);

	// 실제 게임 오브젝트 위치 반영
	pThis->SetPosition(thisBox->center.x, thisBox->center.y);
}


void MyFirstWndGame::UpdateWholeIntersect() {
	static GameObject* pPlayer = GetPlayer();

	auto playerCollider = pPlayer->GetCollider();

	//전체 초기화
	for (int i = 0; i < MAX_GAME_OBJECT_COUNT; i++) {
		GameObjectBase* target = m_GameObjectPtrTable[i];
		if (target == nullptr) break;
		target->GetCollider()->isIntersect = false;
	}

	int j = 1;
	while (j < MAX_GAME_OBJECT_COUNT) {
		GameObjectBase* target = m_GameObjectPtrTable[j];
		if (target == nullptr) break;
		learning::Collider* targetCollider = target->GetCollider();
		if (playerCollider->IsIntersect(targetCollider)) {
			targetCollider->isIntersect = true;
			playerCollider->isIntersect = true;
		}
		++j;
	}
}


void MyFirstWndGame::CheckPlayerEnemyCollision()
{
	GameObject* pPlayer = GetPlayer();

	if (pPlayer == nullptr)
	{
		return;
	}

	learning::Collider* playerCollider = pPlayer->GetCollider();

	if (playerCollider == nullptr)
	{
		return;
	}

	for (int i = 1; i < MAX_GAME_OBJECT_COUNT; ++i)
	{
		GameObjectBase* target = m_GameObjectPtrTable[i];

		if (target == nullptr)
		{
			continue;
		}

		if (target->Type() != ObjectType::ENEMY)
		{
			continue;
		}

		learning::Collider* enemyCollider = target->GetCollider();

		if (enemyCollider == nullptr)
		{
			continue;
		}

		if (playerCollider->IsIntersect(enemyCollider))
		{
			m_gameState = GameState::GameOver;

			pPlayer->SetDirection(Vector2f(0.0f, 0.0f));

			for (int j = 1; j < MAX_GAME_OBJECT_COUNT; ++j)
			{
				if (m_GameObjectPtrTable[j] != nullptr)
				{
					m_GameObjectPtrTable[j]->SetDirection(Vector2f(0.0f, 0.0f));
				}
			}

			return;
		}
	}
}


void MyFirstWndGame::PushEnemiesEachOther()
{
	// 적 오브젝트들을 순회하면서 서로 겹치면 밀어내기

  /*
	  적의 콜라이더 반지름을 가져오고 적을 추가할 때마다 각 적들의 콜라이더를 인식해야 함
	  게임 오브젝트에 1번부터 적
	  반복문에서 적의 최대 개수만큼 순회
	  적과 적의 위치를 빼서 방향을 알아내기
  */

  //최소 거리라는게, 적의 반지름에 2를 곱한 값, 70
	constexpr float ENEMY_RADIUS = 35.0f;
	constexpr float MIN_DISTANCE = ENEMY_RADIUS * 2.0f;

	// 1부터 1000까지 순회하면서 pEnemyA를 넣되, nullptr이면 break
	for (int i = 1; i < MAX_GAME_OBJECT_COUNT; ++i)
	{
		GameObjectBase* pEnemyA = m_GameObjectPtrTable[i];

		if (pEnemyA == nullptr)
		{
			break;
		}

		// posA에 pEnemyA의 위치를 넣기
		Vector2f posA = pEnemyA->GetPosition();

		// j를 i에 1을 더해서 순회
		// pEnemyB를 넣어서 pEnemyA와 비교하기 위함
		// 근데 Enemy가 3개 이상이면 어떻게 비교하지?
		// 이미 이 경우에서는 모든 적 조합을 비교하고 있음
		// 적이 4마리 있으면
			// 1번 인덱스가 2, 3, 4와 비교
			// 2번 인덱스가 3, 4와 비교
			// 3번 인덱스가 4와 비교

		for (int j = i + 1; j < MAX_GAME_OBJECT_COUNT; ++j)
		{
			GameObjectBase* pEnemyB = m_GameObjectPtrTable[j];

			if (pEnemyB == nullptr)
			{
				break;
			}

			Vector2f posB = pEnemyB->GetPosition();

			// B의 위치와 A의 위치의 차이 구하기
			Vector2f diff = posB - posA;

			// 차이의 거리를 구하기
			float distance = diff.Length();

			// 만약에 거리가 0보다 크고, MIN_DISTANCE보다는 작다면
			if (distance > 0.0f && distance < MIN_DISTANCE)
			{
				// 차이를 정규화
				// 왜 정규화를 할까
				// diff는 방향 뿐만 아니라 거리값도 가지고 있어서, 바로 overlap과 곱하면 너무 크기가 커짐
				diff.Normalize();

				// 최소 거리에서 거리의 차를 overlap으로 설정
				float overlap = MIN_DISTANCE - distance;

				// 위치 차이와 ( overlap에 0.5를 곱한 값 )을 곱한 걸 push로 설정
				// push는 미는 힘
				// 겹친 거리에 0.5를 곱해서 거리 차이만큼 밀어내는 것
				Vector2f push = diff * (overlap * 0.5f);

				// EnemyA는 push만큼 빼서 위치를 옮기고, EnemyB는 push만큼 더해서 위치를 옮기기
				// 결국 서로가 밀려나는 것을 구현
				posA -= push;
				posB += push;

				//밀린 위치 좌표를 SetPosition으로 설정
				pEnemyA->SetPosition(posA.x, posA.y);
				pEnemyB->SetPosition(posB.x, posB.y);
			}
			else if (distance == 0.0f)
			{
				// 완전히 같은 위치에 있을 때는 방향을 계산할 수 없으므로 임의로 분리
				posA.x -= MIN_DISTANCE * 0.5f;
				posB.x += MIN_DISTANCE * 0.5f;

				pEnemyA->SetPosition(posA.x, posA.y);
				pEnemyB->SetPosition(posB.x, posB.y);
			}
		}
	}


}


void MyFirstWndGame::PushEnemiesOutOfPlayer()
{
	// 플레이어 오브젝트 가져오기
	GameObjectBase* pPlayer = m_GameObjectPtrTable[0];

	// 플레이어가 존재하지 않으면 함수 종료
	if (pPlayer == nullptr)
	{
		return;
	}

	constexpr float PLAYER_RADIUS = 50.0f;							// 플레이어와 적의 반지름 설정
	constexpr float ENEMY_RADIUS = 35.0f;

	constexpr float MIN_DISTANCE = PLAYER_RADIUS + ENEMY_RADIUS;	// 두 원이 겹치지 않기 위한 최소 거리

	Vector2f playerPos = pPlayer->GetPosition();					// 플레이어 현재 위치 저장

	for (int i = 1; i < MAX_GAME_OBJECT_COUNT; ++i)					// 1번 인덱스부터 적 오브젝트 검사, 0번은 플레이어라고 가정
	{
		GameObjectBase* pEnemy = m_GameObjectPtrTable[i];

		if (pEnemy == nullptr)										// 적이 없으면 이후도 비어있다고 가정하고 종료
		{
			break;
		}

		Vector2f enemyPos = pEnemy->GetPosition();					// 적 현재 위치 저장

		Vector2f diff = enemyPos - playerPos;						// 플레이어 → 적 방향 벡터 계산
		float distance = diff.Length();								// 플레이어와 적 사이 거리 계산

		if (distance > 0.0f && distance < MIN_DISTANCE)				// 거리가 0보다 크고 최소 거리보다 작으면 충돌 상태
		{
			diff.Normalize();										// 방향 벡터 정규화 (길이를 1로 만듦)

			Vector2f fixedPos = playerPos + diff * MIN_DISTANCE;	// 플레이어로부터 최소 거리만큼 떨어진 위치 계산

			pEnemy->SetPosition(fixedPos.x, fixedPos.y);			// 적 위치 보정
			pEnemy->SetDirection(Vector2f(0.0f, 0.0f));				// 적 이동 방향 초기화
		}
		else if (distance == 0.0f)									// 플레이어와 적 위치가 완전히 동일한 경우
		{
			Vector2f fixedPos = playerPos + Vector2f(MIN_DISTANCE, 0.0f);	// x축 방향으로 강제로 이동시켜 겹침 해제

			pEnemy->SetPosition(fixedPos.x, fixedPos.y);					// 적 위치 보정
			pEnemy->SetDirection(Vector2f(0.0f, 0.0f));						// 적 이동 방향 초기화
		}
	}
}

