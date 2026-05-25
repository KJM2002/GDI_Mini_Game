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

void MyFirstWndGame::RemoveItemDataByIndex(int objectIndex)
{
	for (int i = static_cast<int>(m_items.size()) - 1; i >= 0; --i)
	{
		if (m_items[i].objectIndex == objectIndex)
		{
			m_items.erase(m_items.begin() + i);
			return;
		}
	}
}


void MyFirstWndGame::UpdateItemSpawn()
{
	if (m_currentItemCount >= m_maxItemCount)
	{
		return;
	}

	m_itemSpawnTimer += m_fDeltaTime * 0.001f;

	if (m_itemSpawnTimer < m_itemSpawnInterval)
	{
		return;
	}

	m_itemSpawnTimer = 0.0f;

	CreateFlowerBombItem();
}


void MyFirstWndGame::CreateFlowerBombItem()
{
	int slot = FindEmptyObjectSlot();

	if (slot == -1)
	{
		return;
	}

	if (m_pFlowerBombBitmapInfo == nullptr)
	{
		return;
	}

	constexpr int ITEM_RENDER_SIZE = 45;
	constexpr float ITEM_RADIUS = ITEM_RENDER_SIZE * 0.5f;
	constexpr float ITEM_SPEED = 0.18f;
	constexpr int SPAWN_MARGIN = 60;

	int edge = rand() % 4;

	float x = 0.0f;
	float y = 0.0f;

	switch (edge)
	{
	case 0: // 위쪽 밖
		x = static_cast<float>(rand() % m_width);
		y = static_cast<float>(-SPAWN_MARGIN);
		break;

	case 1: // 아래쪽 밖
		x = static_cast<float>(rand() % m_width);
		y = static_cast<float>(m_height + SPAWN_MARGIN);
		break;

	case 2: // 왼쪽 밖
		x = static_cast<float>(-SPAWN_MARGIN);
		y = static_cast<float>(rand() % m_height);
		break;

	case 3: // 오른쪽 밖
		x = static_cast<float>(m_width + SPAWN_MARGIN);
		y = static_cast<float>(rand() % m_height);
		break;
	}

	// 아이템이 향할 화면 안쪽 목표 지점
	float targetX = static_cast<float>(ITEM_RENDER_SIZE + rand() % (m_width - ITEM_RENDER_SIZE * 2));
	float targetY = static_cast<float>(ITEM_RENDER_SIZE + rand() % (m_height - ITEM_RENDER_SIZE * 2));

	Vector2f spawnPos(x, y);
	Vector2f targetPos(targetX, targetY);

	Vector2f itemDir = targetPos - spawnPos;

	if (itemDir.LengthSquared() > 0.0f)
	{
		itemDir.Normalize();
	}

	GameObject* pItem = new GameObject(ObjectType::ITEM);

	pItem->SetName("FlowerItem");
	pItem->SetPosition(x, y);
	pItem->SetSpeed(ITEM_SPEED);
	pItem->SetDirection(itemDir);

	pItem->SetColliderCircle(ITEM_RADIUS);

	pItem->SetBitmapInfo(m_pFlowerBombBitmapInfo);

	// 실제 꽃 이미지 원본 크기에 맞춰 수정
	pItem->SetFrameInfo(500, 500, 1, 1);

	pItem->SetRenderSize(ITEM_RENDER_SIZE, ITEM_RENDER_SIZE);

	m_GameObjectPtrTable[slot] = pItem;

	ItemData data;
	data.objectIndex = slot;
	data.type = ItemType::FlowerBomb;
	data.bounceCount = 0;

	m_items.push_back(data);

	++m_currentItemCount;
}


void MyFirstWndGame::UpdateItems()
{
	constexpr float ITEM_RADIUS = 22.5f;
	constexpr int MAX_BOUNCE_COUNT = 5;

	for (int i = static_cast<int>(m_items.size()) - 1; i >= 0; --i)
	{
		ItemData& itemData = m_items[i];

		int objectIndex = itemData.objectIndex;

		if (objectIndex < 0 || objectIndex >= MAX_GAME_OBJECT_COUNT)
		{
			m_items.erase(m_items.begin() + i);
			continue;
		}

		GameObjectBase* itemObj = m_GameObjectPtrTable[objectIndex];

		if (itemObj == nullptr)
		{
			m_items.erase(m_items.begin() + i);
			continue;
		}

		Vector2f pos = itemObj->GetPosition();
		Vector2f dir = itemObj->GetDirection();

		bool bounced = false;

		if (pos.x <= ITEM_RADIUS)
		{
			pos.x = ITEM_RADIUS;
			dir.x *= -1.0f;
			bounced = true;
		}
		else if (pos.x >= m_width - ITEM_RADIUS)
		{
			pos.x = m_width - ITEM_RADIUS;
			dir.x *= -1.0f;
			bounced = true;
		}

		if (pos.y <= ITEM_RADIUS)
		{
			pos.y = ITEM_RADIUS;
			dir.y *= -1.0f;
			bounced = true;
		}
		else if (pos.y >= m_height - ITEM_RADIUS)
		{
			pos.y = m_height - ITEM_RADIUS;
			dir.y *= -1.0f;
			bounced = true;
		}

		if (bounced)
		{
			++itemData.bounceCount;

			itemObj->SetPosition(pos.x, pos.y);
			itemObj->SetDirection(dir);

			if (itemData.bounceCount >= MAX_BOUNCE_COUNT)
			{
				RemoveObject(objectIndex);

				m_items.erase(m_items.begin() + i);

				if (m_currentItemCount > 0)
				{
					--m_currentItemCount;
				}
			}
		}
	}
}


void MyFirstWndGame::CheckPlayerItemCollision()
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
		GameObjectBase* obj = m_GameObjectPtrTable[i];

		if (obj == nullptr)
		{
			continue;
		}

		if (obj->Type() != ObjectType::ITEM)
		{
			continue;
		}

		learning::Collider* itemCollider = obj->GetCollider();

		if (itemCollider == nullptr)
		{
			continue;
		}

		if (playerCollider->IsIntersect(itemCollider))
		{
			learning::Vector2f itemPos = obj->GetPosition();

			RemoveObject(i);
			RemoveItemDataByIndex(i);

			if (m_currentItemCount > 0)
			{
				--m_currentItemCount;
			}

			ActivateFlowerBomb(itemPos);

			return;
		}
	}
}


void MyFirstWndGame::ActivateFlowerBomb(learning::Vector2f position)
{
	int slot = FindEmptyObjectSlot();

	if (slot == -1)
	{
		return;
	}

	constexpr int EFFECT_RENDER_SIZE = 220;
	constexpr float EFFECT_RADIUS = EFFECT_RENDER_SIZE * 0.5f;

	GameObject* pEffect = new GameObject(ObjectType::BULLET);

	pEffect->SetName("FlowerBombEffect");
	pEffect->SetPosition(position.x, position.y);
	pEffect->SetSpeed(0.0f);

	pEffect->SetColliderCircle(EFFECT_RADIUS);

	pEffect->SetBitmapInfo(m_pFlowerBombEffectBitmapInfo);

	// 꽃 이미지 단일 프레임
	pEffect->SetFrameInfo(500, 500, 4, 4);

	pEffect->SetRenderSize(EFFECT_RENDER_SIZE, EFFECT_RENDER_SIZE);

	m_GameObjectPtrTable[slot] = pEffect;

	FlowerBombEffectData data;
	data.objectIndex = slot;
	data.timer = 0.0f;
	data.duration = 3.0f;

	m_flowerBombEffects.push_back(data);
}


void MyFirstWndGame::UpdateFlowerBombEffects()
{
	for (int e = static_cast<int>(m_flowerBombEffects.size()) - 1; e >= 0; --e)
	{
		FlowerBombEffectData& effectData = m_flowerBombEffects[e];

		effectData.timer += m_fDeltaTime * 0.001f;

		int effectIndex = effectData.objectIndex;

		if (effectIndex < 0 || effectIndex >= MAX_GAME_OBJECT_COUNT)
		{
			m_flowerBombEffects.erase(m_flowerBombEffects.begin() + e);
			continue;
		}

		GameObjectBase* effectObj = m_GameObjectPtrTable[effectIndex];

		if (effectObj == nullptr)
		{
			m_flowerBombEffects.erase(m_flowerBombEffects.begin() + e);
			continue;
		}

		learning::Collider* effectCollider = effectObj->GetCollider();

		if (effectCollider == nullptr)
		{
			continue;
		}

		for (int i = 1; i < MAX_GAME_OBJECT_COUNT; ++i)
		{
			GameObjectBase* enemy = m_GameObjectPtrTable[i];

			if (enemy == nullptr)
			{
				continue;
			}

			if (enemy->Type() != ObjectType::ENEMY)
			{
				continue;
			}

			learning::Collider* enemyCollider = enemy->GetCollider();

			if (enemyCollider == nullptr)
			{
				continue;
			}

			if (effectCollider->IsIntersect(enemyCollider))
			{
				KillEnemy(i, m_scorePerKill);
			}
		}

		if (effectData.timer >= effectData.duration)
		{
			RemoveObject(effectIndex);
			m_flowerBombEffects.erase(m_flowerBombEffects.begin() + e);
		}
	}
}

