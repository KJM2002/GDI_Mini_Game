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

bool MyFirstWndGame::Initialize()
{
	srand(static_cast<unsigned int>(time(nullptr)));	// 실행할 때마다 적 생성 위치를 다르게

	m_pGameTimer = new GameTimer(); // 게임 시간과 delta time 계산용 타이머 생성
	m_pGameTimer->Reset();			// 타이머 기준 시간을 현재 시점으로 초기화

	const wchar_t* className = L"MyFirstWndGame"; //L은 유니코드로 하겠다는 거
	const wchar_t* windowName = L"MyFirstWndGame";

	// 부모 클래스의 Create 함수를 호출해 윈도우를 생성합니다.
	// 실패하면 초기화 실패로 false를 반환합니다.
	if (false == __super::Create(className, windowName, 720, 1080))
	{
		return false;
	}


	RECT rcClient = {};

	// 클라이언트 영역 크기를 얻습니다.
	// 클라이언트 영역은 제목 표시줄과 테두리를 제외한 실제 그리기 영역입니다.
	GetClientRect(m_hWnd, &rcClient);

	m_width = rcClient.right - rcClient.left;
	m_height = rcClient.bottom - rcClient.top;

	// 실제 윈도우 화면에 출력되는 DC입니다.
	m_hFrontDC = GetDC(m_hWnd);

	// 화면에 바로 그리면 깜빡임이 생길 수 있으므로,
	// 먼저 그릴 메모리 DC를 만듭니다.
	m_hBackDC = CreateCompatibleDC(m_hFrontDC);

	// 백 버퍼 역할을 할 Bitmap입니다.
	// 화면 크기와 같은 크기로 생성합니다.
	m_hBackBitmap = CreateCompatibleBitmap(m_hFrontDC, m_width, m_height);

	// 메모리 DC에 백 버퍼 Bitmap을 선택합니다.
	// 반환값은 원래 선택되어 있던 Bitmap이므로,
	// 종료 시 다시 복구해야 합니다.
	m_hDefaultBitmap = (HBITMAP)SelectObject(m_hBackDC, m_hBackBitmap);

	// 게임 오브젝트를 저장할 포인터 배열을 동적 생성합니다.
	// 실제 객체는 따로 new로 생성하고,
	// 이 배열에는 객체 주소만 저장합니다.
	m_GameObjectPtrTable = new GameObjectBase * [MAX_GAME_OBJECT_COUNT];

	// 처음에는 아무 오브젝트도 들어 있지 않으므로 nullptr로 초기화합니다
	for (int i = 0; i < MAX_GAME_OBJECT_COUNT; ++i)
	{
		m_GameObjectPtrTable[i] = nullptr;
	}


	//필요 리소스 로드
#pragma region resource

	// 주의:
	// IDE에서 보이는 현재 경로와 실제 실행 파일 기준 경로가 다를 수 있습니다.
	// 따라서 리소스 경로가 맞지 않으면 실행 위치를 확인해야 합니다.
	m_pPlayerBitmapInfo = renderHelp::CreateBitmapInfo(L"../Resource/LadyBug.png");		// 플레이어 스프라이트 이미지 로드
	m_pEnemyBitmapInfo = renderHelp::CreateBitmapInfo(L"../Resource/Enemy.png");		// 적 스프라이트 이미지 로드
	m_pBackgroundBitmapInfo = renderHelp::CreateBitmapInfo(L"../Resource/Background.png");	// 배경 이미지 로드
	m_pFlowerBombBitmapInfo = renderHelp::CreateBitmapInfo(L"../Resource/flower_bomb.png");
	m_pFlowerBombEffectBitmapInfo = renderHelp::CreateBitmapInfo(L"../Resource/flower_bomb_effect_sheet.png");
#pragma endregion

	// [CHECK]. 첫 번째 게임 오브젝트는 플레이어 캐릭터로 고정!
	// 즉, m_GameObjectPtrTable[0]은 항상 플레이어
	CreatePlayer();

	return true;

}


void MyFirstWndGame::Run()
{
	MSG msg = { 0 };     //디스패치하지 않고(프로시져를 거치지 않고) 이렇게 처리할 수도 있다.
	while (msg.message != WM_QUIT)
	{
		// 메시지 큐에 처리할 메시지가 있는지 확인합니다.
		// PM_REMOVE는 메시지를 꺼내면서 큐에서 제거한다는 의미입니다.
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_KEYDOWN)
			{
				switch (msg.wParam)
				{
				case 'W':
				case VK_UP:
					m_keyUp = true;
					break;

				case 'S':
				case VK_DOWN:
					m_keyDown = true;
					break;

				case 'A':
				case VK_LEFT:
					m_keyLeft = true;
					break;

				case 'D':
				case VK_RIGHT:
					m_keyRight = true;
					break;
				/*case 'K':
					for (int i = 1; i < MAX_GAME_OBJECT_COUNT; ++i)
					{
						if (m_GameObjectPtrTable[i] != nullptr &&
							m_GameObjectPtrTable[i]->Type() == ObjectType::ENEMY)
						{
							KillEnemy(i, m_scorePerKill);
							break;
						}
					}
					break;*/

				case VK_SPACE:
					if (m_gameState == GameState::GameOver)
					{
						RestartGame();
					}
					break;
				}
			}
			else if (msg.message == WM_KEYUP)
			{
				switch (msg.wParam)
				{
				case 'W':
				case VK_UP:
					m_keyUp = false;
					break;

				case 'S':
				case VK_DOWN:
					m_keyDown = false;
					break;

				case 'A':
				case VK_LEFT:
					m_keyLeft = false;
					break;

				case 'D':
				case VK_RIGHT:
					m_keyRight = false;
					break;
				}
			}
			else if (msg.message == WM_LBUTTONDOWN)
			{
				MyFirstWndGame::OnLButtonDown(LOWORD(msg.lParam), HIWORD(msg.lParam));
			}
			else if (msg.message == WM_RBUTTONDOWN)
			{
				MyFirstWndGame::OnRButtonDown(LOWORD(msg.lParam), HIWORD(msg.lParam));
			}
			else if (msg.message == WM_MBUTTONDOWN)
			{
				MyFirstWndGame::OnMButtonDown(LOWORD(msg.lParam), HIWORD(msg.lParam));
			}
			else if (msg.message == WM_MOUSEMOVE)
			{
				MyFirstWndGame::OnMouseMove(LOWORD(msg.lParam), HIWORD(msg.lParam));
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			// 처리할 메시지가 없을 때 게임 로직과 렌더링을 수행합니다.
			Update();
			Render();
		}
	}
}


void MyFirstWndGame::Finalize()
{
	delete m_pGameTimer;
	m_pGameTimer = nullptr;

	delete m_pPlayerBitmapInfo;
	m_pPlayerBitmapInfo = nullptr;

	delete m_pEnemyBitmapInfo;
	m_pEnemyBitmapInfo = nullptr;

	delete m_pBackgroundBitmapInfo;
	m_pBackgroundBitmapInfo = nullptr;

	delete m_pFlowerBombBitmapInfo;
	m_pFlowerBombBitmapInfo = nullptr;

	delete m_pFlowerBombEffectBitmapInfo;
	m_pFlowerBombEffectBitmapInfo = nullptr;

	if (m_GameObjectPtrTable)
	{
		for (int i = 0; i < MAX_GAME_OBJECT_COUNT; ++i)
		{
			if (m_GameObjectPtrTable[i])
			{
				// 각 슬롯에 들어 있는 게임 오브젝트 해제
				delete m_GameObjectPtrTable[i];
				m_GameObjectPtrTable[i] = nullptr;
			}
		}
		// 주의:
		// m_GameObjectPtrTable은 new []로 생성했으므로
		// 원칙적으로 delete[] m_GameObjectPtrTable; 을 사용하는 것이 더 안전합니다.
		delete[] m_GameObjectPtrTable;
	}
	// 부모 클래스의 윈도우/시스템 자원 정리 호출
	__super::Destroy();
}


void MyFirstWndGame::FixedUpdate()
{
	// 플레이어와 적 전체의 충돌 상태를 먼저 갱신합니다.
	UpdateWholeIntersect();

	// 오른쪽 클릭으로 원형 적 생성 위치가 저장되어 있으면 원형 적 생성
	if (m_CirEnemySpawnPos.x != 0 && m_CirEnemySpawnPos.y != 0)
	{
		CreateCircleEnemy();
	}
	// 가운데 클릭으로 박스 적 생성 위치가 저장되어 있으면 박스 적 생성
	else if (m_BoxEnemySpawnPos.x != 0 && m_BoxEnemySpawnPos.y != 0) {

		CreateBoxEnemy();
	}
}


void MyFirstWndGame::LogicUpdate()
{
	if (m_gameState == GameState::GameOver)
	{
		return;
	}

	UpdateScoreAndTime();

	UpdatePlayerInfo();

	UpdateEnemySpawn();

	UpdateEnemyInfo();

	UpdateItemSpawn();

	for (int i = 0; i < MAX_GAME_OBJECT_COUNT; ++i)
	{
		if (m_GameObjectPtrTable[i] == nullptr)
		{
			continue;
		}

		m_GameObjectPtrTable[i]->Update(m_fDeltaTime);
	}

	//PushEnemiesEachOther();
	// 적들끼리 서로 충돌하지 않는다
	UpdateItems();


	CheckPlayerItemCollision();

	UpdateFlowerBombEffects();

	CheckPlayerEnemyCollision();

	RemoveOutOfScreenEnemies();

	RemoveOutOfScreenEnemies();
}


void MyFirstWndGame::Update()
{
	m_pGameTimer->Tick();

	m_fDeltaTime = m_pGameTimer->DeltaTimeMS();
	m_fFrameCount += m_fDeltaTime;

	LogicUpdate();

	while (m_fFrameCount >= 200.0f)
	{
		FixedUpdate();
		m_fFrameCount -= 200.0f;
	}
}


void MyFirstWndGame::UpdateScoreAndTime()
{
	float deltaTimeSec = m_fDeltaTime * 0.001f;

	m_surviveTime += deltaTimeSec;

	// 생존 시간에 따라 점수가 천천히 증가
	m_score += deltaTimeSec * m_scorePerSecond;
}


float MyFirstWndGame::FClamp(float value, float min, float max)
{
	if (value < min)
		return min;

	if (value > max)
		return max;

	return value;
}


int MyFirstWndGame::FindEmptyObjectSlot() const
{
	for (int i = 1; i < MAX_GAME_OBJECT_COUNT; ++i)
	{
		if (m_GameObjectPtrTable[i] == nullptr)
		{
			return i;
		}
	}

	return -1;
}


void MyFirstWndGame::RemoveObject(int index)
{
	if (index < 0 || index >= MAX_GAME_OBJECT_COUNT)
	{
		return;
	}

	if (m_GameObjectPtrTable[index] != nullptr)
	{
		delete m_GameObjectPtrTable[index];
		m_GameObjectPtrTable[index] = nullptr;
	}
}


void MyFirstWndGame::RestartGame()
{
	for (int i = 1; i < MAX_GAME_OBJECT_COUNT; ++i)
	{
		if (m_GameObjectPtrTable[i] != nullptr)
		{
			delete m_GameObjectPtrTable[i];
			m_GameObjectPtrTable[i] = nullptr;
		}
	}

	GameObject* pPlayer = GetPlayer();

	if (pPlayer != nullptr)
	{
		pPlayer->SetPosition(m_width * 0.5f, m_height * 0.5f);
		pPlayer->SetDirection(Vector2f(0.0f, 0.0f));
	}

	m_keyUp = false;
	m_keyDown = false;
	m_keyLeft = false;
	m_keyRight = false;

	m_enemySpawnTimer = 0.0f;

	m_surviveTime = 0.0f;
	m_score = 0.0f;
	m_killCount = 0;

	m_itemSpawnTimer = 0.0f;
	m_currentItemCount = 0;
	m_flowerBombEffects.clear();

	m_gameState = GameState::Playing;
}

