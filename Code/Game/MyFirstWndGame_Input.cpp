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

void MyFirstWndGame::OnMouseMove(int x, int y)
{
	/*   std::cout << __FUNCTION__ << std::endl;
	   std::cout << "x: " << x << ", y: " << y << std::endl;*/
	m_MousePosPrev = m_MousePos;
	m_MousePos = { x, y };
}


void MyFirstWndGame::OnLButtonDown(int x, int y)
{
	/*  std::cout << __FUNCTION__ << std::endl;
 std::cout << "x: " << x << ", y: " << y << std::endl;*/

	m_PlayerTargetPos.x = x;
	m_PlayerTargetPos.y = y;

}


void MyFirstWndGame::OnRButtonDown(int x, int y)
{
	/*  std::cout << __FUNCTION__ << std::endl;
   std::cout << "x: " << x << ", y: " << y << std::endl;*/

	m_CirEnemySpawnPos.x = x;
	m_CirEnemySpawnPos.y = y;
}


void MyFirstWndGame::OnMButtonDown(int x, int y)
{
	m_BoxEnemySpawnPos.x = x;
	m_BoxEnemySpawnPos.y = y;
}

