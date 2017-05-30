#include "stdafx.h"
#include "wuziqi.h"
#include "board.h"

CWuZiQi::CWuZiQi(QObject* parent)
	: QObject(parent)
	, m_pBoard(new CBoard(nullptr))
{
}

CWuZiQi::~CWuZiQi()
{

}

void CWuZiQi::startGame()
{
	m_pBoard->show();
}
