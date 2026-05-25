#pragma once
#include "../Core/NzWndBase.h"
#include "../Core/Utillity.h"
#include <vector>

// [CHECK] #7  전방 선언을 사용하여 헤더파일의 의존성을 줄임.
class GameTimer;
class GameObjectBase;
class GameObject;

enum class ObjectType;

namespace renderHelp {
    class BitmapInfo;
}

namespace learning {
    class Vector2f;
    class Collider;
}

enum class GameState
{
    Playing,
    GameOver
};

enum class ItemType
{
    FlowerBomb,
};

struct ItemData
{
    int objectIndex = -1;
    ItemType type = ItemType::FlowerBomb;
    int bounceCount = 0;
};

struct FlowerBombEffectData
{
    int objectIndex = -1;
    float timer = 0.0f;
    float duration = 3.0f;
};

class MyFirstWndGame : public NzWndBase
{
public:
    MyFirstWndGame() = default;
    ~MyFirstWndGame() override = default;

    bool Initialize();
    void Run();
    void Finalize();

    static float FClamp(float value, float min, float max);

private:
    void Update();
    void Render();

    void RenderObjectsByType(ObjectType type);          // 렌더링 순서

    void DrawBackground();  // 배경 그리기 함수 추가

    void OnResize(int width, int height) override;     //On은 이벤트가 이뤄졌을 때를 의미
    void OnClose() override;

    void OnMouseMove(int x, int y);
    void OnLButtonDown(int x, int y);
    void OnRButtonDown(int x, int y);
    void OnMButtonDown(int x, int y);

    void FixedUpdate();
    void LogicUpdate();

    void CreatePlayer();

    void CreateCircleEnemy();
    void CreateCircleEnemy(float x, float y);   // 오버로딩

    void CreateBoxEnemy();

    void UpdatePlayerInfo();
    void UpdateEnemyInfo();         // 적이 플레이어 추적 가능하도록 함수 추가

	void UpdateScoreAndTime();  // 점수와 시간 업데이트 함수 추가

    void UpdateEnemySpawn();        // 적 스폰 업데이트 함수

    void CheckPlayerEnemyCollision();   // 적과 충돌했는지 검사
    void RestartGame();                 // 게임 재시작

    // =============================
    // 아이템 관련 함수
    // =============================


    void UpdateItemSpawn();
    void CreateFlowerBombItem();
    void UpdateItems();
    void RemoveItemDataByIndex(int objectIndex);

    void CheckPlayerItemCollision();
    void ActivateFlowerBomb(learning::Vector2f position);

    void UpdateFlowerBombEffects();

    // =============================


	//  ============================

    int FindEmptyObjectSlot() const;
    void RemoveObject(int index);

	void RenderUI();                        // UI 렌더링 함수 추가

	void KillEnemy(int index, int score);   // 적 제거 및 점수 증가 함수 추가

    void UpdateWholeIntersect();

    void PushEnemiesEachOther();
    void PushEnemiesOutOfPlayer();

    


    void SettingBoxPos(learning::Collider* thisBox, learning::Collider* targetBox, GameObject* pThis, learning::Vector2f firstDir);
    learning::Vector2f GetBoxDir(learning::Collider* thisBox, learning::Collider* targetBox);
    void SettingCirPos(learning::Collider* thisCir, learning::Collider* targetCir, GameObject* pThis);

    GameObject* GetPlayer() const { return (GameObject*)m_GameObjectPtrTable[0]; }

private:
    HDC m_hFrontDC = nullptr;
    HDC m_hBackDC = nullptr;
    HBITMAP m_hBackBitmap = nullptr;
    HBITMAP m_hDefaultBitmap = nullptr;

    // [CHECK] #8. 게임 타이머를 사용하여 프레임을 관리하는 예시.F
    GameTimer* m_pGameTimer = nullptr;
    float m_fDeltaTime = 0.0f;
    float m_fFrameCount = 0.0f;

    // [CHECK] #8. 게임 오브젝트를 관리하는 컨테이너.
    GameObjectBase** m_GameObjectPtrTable = nullptr;

    struct MOUSE_POS
    {
        int x = 0;
        int y = 0;

        bool operator!=(const MOUSE_POS& other) const //연산자 오버로딩 
        {
            return (x != other.x || y != other.y);
        }
    };

    MOUSE_POS m_MousePos = { 0, 0 };
    MOUSE_POS m_MousePosPrev = { 0, 0 };

    MOUSE_POS m_PlayerTargetPos = { 0, 0 };
    MOUSE_POS m_CirEnemySpawnPos = { 0, 0 };
    MOUSE_POS m_BoxEnemySpawnPos = { 0, 0 };

    // 키보드 입력 상태
    bool m_keyUp = false;
    bool m_keyDown = false;
    bool m_keyLeft = false;
    bool m_keyRight = false;

    //////////////////////////////////////////////////

    // 플레이어 빙판 이동용 속도
    learning::Vector2f m_playerVelocity = { 0.0f, 0.0f };

    // 값이 클수록 키 입력 시 빠르게 가속
    float m_playerAcceleration = 0.0012f;

    // 값이 클수록 최대 이동 속도 증가
    float m_playerMaxSpeed = 0.42f;

    // 값이 클수록 빨리 멈춤
    float m_playerFriction = 0.0005f;

    //////////////////////////////////////////////////

    // 적 자동 스폰
    float m_enemySpawnTimer = 0.0f;
    float m_enemySpawnInterval = 1.5f;

    // 적 삭제
    void RemoveOutOfScreenEnemies();

    // 게임 상태
    GameState m_gameState = GameState::Playing;

    // 점수 / 시간
    float m_surviveTime = 0.0f;
    float m_score = 0.0f;
    int m_killCount = 0;

    // 점수 규칙
    float m_scorePerSecond = 5.0f;
    int m_scorePerKill = 100;

    ////////////////////////////////////////////////

    // 아이템 스폰
    float m_itemSpawnTimer = 0.0f;
    float m_itemSpawnInterval = 5.0f;
    int m_currentItemCount = 0;
    int m_maxItemCount = 3;

    std::vector<ItemData> m_items;

    // 꽃잎 폭탄 효과
    std::vector<FlowerBombEffectData> m_flowerBombEffects;


	renderHelp::BitmapInfo* m_pPlayerBitmapInfo = nullptr;  // 플레이어 이미지 정보

	renderHelp::BitmapInfo* m_pEnemyBitmapInfo = nullptr;   // 적 이미지 정보

	renderHelp::BitmapInfo* m_pBackgroundBitmapInfo = nullptr;  // 배경 이미지 정보

	renderHelp::BitmapInfo* m_pFlowerBombBitmapInfo = nullptr;  // 아이템 이미지 정보


	renderHelp::BitmapInfo* m_pFlowerBombEffectBitmapInfo = nullptr;    // 꽃잎 폭탄 효과 이미지 정보

};