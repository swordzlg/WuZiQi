#pragma once

class CBoard;

class CWuZiQi : public QObject
{
	Q_OBJECT

public:
	CWuZiQi(QObject* parent = 0);
	~CWuZiQi();

	void startGame();

private:
	CBoard* m_pBoard;
};
