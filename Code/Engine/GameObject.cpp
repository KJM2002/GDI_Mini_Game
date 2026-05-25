#include "../Core/INC_Windows.h"
#include "renderHelp.h"
#include "Collider.h"
#include "GameObject.h"
#include <assert.h>

////////////////////////////////////////////////////////
GameObject::~GameObject()
{
    if (myCollider)
    {
        delete myCollider;
        myCollider = nullptr;
    }
}

////////////////////////////////////////////////////////
void GameObject::Update(float deltaTime)
{
    Move(deltaTime);
    UpdateFrame(deltaTime);
    // Collider 업데이트
    if (myCollider)
    {
        myCollider->center = m_pos;
    }
}

////////////////////////////////////////////////////////
void GameObject::Render(HDC hdc)
{
    //비트맵 그리기

    DrawBitmap(hdc);
    DrawCollider(hdc);
}


////////////////////////////////////////////////////////
void GameObject::SetColliderCircle(float radius)
{
    if (myCollider)
    {
        delete myCollider;
        myCollider = nullptr;
    }

    learning::ColliderCircle* circleP = new learning::ColliderCircle;
    myCollider = dynamic_cast<learning::Collider*>(circleP);

    assert(circleP != nullptr && "Failed to create ColliderCircle!");
    assert(myCollider != nullptr && "Failed to create ColliderCircle!");

    circleP->radius = radius;
    circleP->center = m_pos;
}


////////////////////////////////////////////////////////
void GameObject::SetColliderBox(float width, float height)
{
    if (myCollider)
    {
        delete myCollider;
        myCollider = nullptr;
    }

    learning::ColliderBox* boxP = new learning::ColliderBox;
    myCollider = dynamic_cast<learning::Collider*>(boxP);

    assert(boxP != nullptr && "Failed to create ColliderBox!");
    assert(myCollider != nullptr && "Failed to create ColliderBox!");

    boxP->center = m_pos;
    boxP->halfSize.x = width / 2.0f;
    boxP->halfSize.y = height / 2.0f;
}


////////////////////////////////////////////////////////
void GameObject::DrawCollider(HDC hdc)
{
    if (myCollider == nullptr) return;

    HPEN hPenR = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
    HPEN hPenB = CreatePen(PS_SOLID, 2, RGB(0, 0, 255));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPenR); // 빨간팬 선택 기존 팬 들어있음
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

    if (myCollider->isIntersect) {
        SelectObject(hdc, hPenB);
    }
    else {
        SelectObject(hdc, hPenR);
    }

    myCollider->Draw(hdc);

    // 이전 객체 복원 및 펜 삭제
    SelectObject(hdc, hOldPen);
    SelectObject(hdc, hOldBrush);
    DeleteObject(hPenR);
    DeleteObject(hPenB);
}

////////////////////////////////////////////////////////
void GameObject::SetBitmapInfo(BitmapInfo* bitmapInfo) {
    assert(m_pBitmapInfo == nullptr && "BitmapInfo must be null!");

    m_pBitmapInfo = bitmapInfo;
}

////////////////////////////////////////////////////////
// 현재 DrawBitmap()은 m_frameXY[m_frameIndex]에서 srcX, srcY를 가져오고, m_frameWidth, m_frameHeight만큼 잘라서 AlphaBlend()로 그림
// 그러니까 이 값들을 이미지마다 다르게 세팅해줘야 함
void GameObject::SetFrameInfo(int frameWidth, int frameHeight, int frameCount, int columnCount)
{

    // player   :   102, 94, 14, 5
    // Enemy    :   183, 168, 14, 5

    m_frameWidth = frameWidth;
    m_frameHeight = frameHeight;
    m_frameCount = frameCount;
    m_frameColumnCount = columnCount;

    m_frameIndex = 0;
    m_frameTime = 0.0f;

    for (int i = 0; i < frameCount; ++i)
    {
        int col = i % columnCount;
        int row = i / columnCount;

        m_frameXY[i].x = col * frameWidth;
        m_frameXY[i].y = row * frameHeight;
    }
}

////////////////////////////////////////////////////////
void GameObject::SetRenderSize(int width, int height)
{
    m_width = width;
    m_height = height;
}

////////////////////////////////////////////////////////
void GameObject::DrawBitmap(HDC hdc)
{
    if (m_pBitmapInfo == nullptr) return;
    if (m_pBitmapInfo->GetBitmapHandle() == nullptr) return;

    HDC hBitmapDC = CreateCompatibleDC(hdc);

    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hBitmapDC, m_pBitmapInfo->GetBitmapHandle());
    // BLENDFUNCTION 설정 (알파 채널 처리)
    BLENDFUNCTION blend = { 0 };
    blend.BlendOp = AC_SRC_OVER;
    blend.SourceConstantAlpha = 255;  // 원본 알파 채널 그대로 사용
    blend.AlphaFormat = AC_SRC_ALPHA;

    const int x = m_pos.x - m_width / 2;
    const int y = m_pos.y - m_height / 2;

    const int srcX = m_frameXY[m_frameIndex].x;
    const int srcY = m_frameXY[m_frameIndex].y;

    //실제 비트맵에 그림
    AlphaBlend(hdc, x, y, m_width, m_height,
        hBitmapDC, srcX, srcY, m_frameWidth, m_frameHeight, blend);

    // 비트맵 핸들 복원
    SelectObject(hBitmapDC, hOldBitmap);
    DeleteDC(hBitmapDC);

}

////////////////////////////////////////////////////////
// 가져오는 그림 처리 
void GameObject::UpdateFrame(float deltaTime)
{
    m_frameTime += deltaTime;
    if (m_frameTime >= m_frameDuration)
    {
        m_frameTime = 0.0f;
        m_frameIndex = (m_frameIndex + 1) % (m_frameCount);
    }
}





////////////////////////////////////////////////////////
learning::Collider* GameObject::GetCollider() {
    return myCollider;
}

////////////////////////////////////////////////////////
void GameObject::Move(float deltaTime)
{
    GameObjectBase::Move(deltaTime);
}

////////////////////////////////////////////////////////
void GameObjectBase::SetName(const char* name)
{
    if (name == nullptr) return;

    strncpy_s(m_name, name, OBJECT_NAME_LEN_MAX - 1);
    m_name[OBJECT_NAME_LEN_MAX - 1] = '\0';
}