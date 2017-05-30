#pragma once

#include <QWidget>
#include <QVector>

struct StepScore;

class CBoard : public QWidget
{
	Q_OBJECT

	typedef QWidget Base;

	enum Player
	{
		Human = 0,
		Ai = ~Human,
		None = Ai - 1,
	};

public:
	CBoard(QWidget *parent);
	~CBoard();

signals:
	void aiStep(int x, int y);

protected:
	void paintEvent(QPaintEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;

private:
	void initUi();
	// 计算某一位置的分数
	int computePositionScore(int x, int y, Player player);
	void computeAiNextStep();
	void playChess(int x, int y, Player player);

private slots:
	void onAiStep(int x, int y);

private:
	QVector<QVector<Player> > m_boardState;
};
