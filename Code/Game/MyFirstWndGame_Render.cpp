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

void MyFirstWndGame::Render()
{
	::PatBlt(m_hBackDC, 0, 0, m_width, m_height, WHITENESS);

	DrawBackground();

	RenderObjectsByType(ObjectType::ITEM);
	RenderObjectsByType(ObjectType::BULLET);   // 꽃 효과
	RenderObjectsByType(ObjectType::ENEMY);
	RenderObjectsByType(ObjectType::PLAYER);   // 플레이어를 마지막 쪽에 그림

	RenderUI();

	BitBlt(m_hFrontDC, 0, 0, m_width, m_height, m_hBackDC, 0, 0, SRCCOPY);
}

void MyFirstWndGame::RenderObjectsByType(ObjectType type)
{
	for (int i = 0; i < MAX_GAME_OBJECT_COUNT; ++i)
	{
		GameObjectBase* obj = m_GameObjectPtrTable[i];

		if (obj == nullptr)
		{
			continue;
		}

		if (obj->Type() != type)
		{
			continue;
		}

		obj->Render(m_hBackDC);
	}
}


void MyFirstWndGame::RenderUI()
{
	SetBkMode(m_hBackDC, TRANSPARENT);
	SetTextColor(m_hBackDC, RGB(255, 255, 255));

	// 글자 크기 설정
	HFONT hFont = CreateFont(
		32,                 // 글자 높이. 값이 클수록 글자가 커짐
		0,                  // 글자 너비. 0이면 자동
		0,
		0,
		FW_BOLD,            // 굵기. FW_NORMAL 또는 FW_BOLD
		FALSE,              // Italic
		FALSE,              // Underline
		FALSE,              // StrikeOut
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE,
		L"Arial"
	);

	HFONT hOldFont = (HFONT)SelectObject(m_hBackDC, hFont);

	wchar_t buffer[128] = {};

	swprintf_s(buffer, L"TIME  : %.1f", m_surviveTime);
	TextOut(m_hBackDC, 20, 20, buffer, lstrlen(buffer));

	swprintf_s(buffer, L"SCORE : %d", static_cast<int>(m_score));
	TextOut(m_hBackDC, 20, 60, buffer, lstrlen(buffer));

	swprintf_s(buffer, L"KILL  : %d", m_killCount);
	TextOut(m_hBackDC, 20, 100, buffer, lstrlen(buffer));

	if (m_gameState == GameState::GameOver)
	{
		SetTextColor(m_hBackDC, RGB(255, 0, 0));

		const wchar_t* gameOverText = L"GAME OVER - Press SPACE";

		TextOut(
			m_hBackDC,
			m_width / 2 - 220,
			m_height / 2 - 30,
			gameOverText,
			lstrlen(gameOverText)
		);
	}

	// 원래 폰트로 복구 후 생성한 폰트 삭제
	SelectObject(m_hBackDC, hOldFont);
	DeleteObject(hFont);
}


void MyFirstWndGame::DrawBackground()
{
	if (m_pBackgroundBitmapInfo == nullptr)
	{
		return;
	}

	if (m_pBackgroundBitmapInfo->GetBitmapHandle() == nullptr)
	{
		return;
	}

	HDC hBitmapDC = CreateCompatibleDC(m_hBackDC);

	HBITMAP hOldBitmap = (HBITMAP)SelectObject(
		hBitmapDC,
		m_pBackgroundBitmapInfo->GetBitmapHandle()
	);

	BLENDFUNCTION blend = {};
	blend.BlendOp = AC_SRC_OVER;
	blend.SourceConstantAlpha = 255;
	blend.AlphaFormat = AC_SRC_ALPHA;

	AlphaBlend(
		m_hBackDC,
		0,
		0,
		m_width,
		m_height,
		hBitmapDC,
		0,
		0,
		m_pBackgroundBitmapInfo->GetWidth(),
		m_pBackgroundBitmapInfo->GetHeight(),
		blend
	);

	SelectObject(hBitmapDC, hOldBitmap);
	DeleteDC(hBitmapDC);
}


void MyFirstWndGame::OnResize(int width, int height)
{
	std::cout << __FUNCTION__ << std::endl;

	learning::SetScreenSize(width, height);

	__super::OnResize(width, height);

	m_hBackBitmap = CreateCompatibleBitmap(m_hFrontDC, m_width, m_height);

	HANDLE hPrevBitmap = (HBITMAP)SelectObject(m_hBackDC, m_hBackBitmap);

	DeleteObject(hPrevBitmap);
}


void MyFirstWndGame::OnClose()
{
	std::cout << __FUNCTION__ << std::endl;

	SelectObject(m_hBackDC, m_hDefaultBitmap);

	DeleteObject(m_hBackBitmap);
	DeleteDC(m_hBackDC);

	ReleaseDC(m_hWnd, m_hFrontDC);
}

