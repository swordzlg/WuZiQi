#include "stdafx.h"
#include "board.h"
#include <QtConcurrent>

namespace
{
	const int kCellSideLength = 30;
	const int kLuCount = 20;
	const int kBoardMargin = 40;
	const int kChessDiameter = 16;
}

struct StepScore
{
	int posX;
	int posY;
	int score;

	StepScore()
		: posX(-1), posY(-1), score(-1)
	{

	}

	StepScore(int x, int y, int s)
		: posX(x), posY(y), score(s)
	{

	}

	QPoint pos()
	{
		return QPoint(posX, posY);
	}

	operator QString() const
	{
		return QString("{(%1, %2), %3}")
			.arg(posX).arg(posY).arg(score);
	}
};

bool operator < (const StepScore& s1, const StepScore& s2)
{
	return s1.score < s2.score;
}

CBoard::CBoard(QWidget *parent)
	: QWidget(parent)
{
	initUi();

	connect(this, &CBoard::aiStep, this, &CBoard::onAiStep);

	m_boardState.resize(kLuCount);
	for (int i = 0; i < m_boardState.size(); ++i)
	{
		m_boardState[i].resize(kLuCount);
		std::fill(m_boardState[i].begin(), m_boardState[i].end(), None);
	}
}

CBoard::~CBoard()
{

}

void CBoard::paintEvent(QPaintEvent* event)
{
	Base::paintEvent(event);

	const int beginX = kBoardMargin;
	const int beginY = kBoardMargin;
	const int endX = width() - kBoardMargin;
	const int endY = height() - kBoardMargin;

	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	int x = beginX, y = beginY;
	for (int i = 0; i < kLuCount; ++i)
	{
		painter.drawLine(x, beginY, x, endY);
		x += kCellSideLength;

		painter.drawLine(beginX, y, endX, y);
		y += kCellSideLength;
	}

	const int chessRadius = kChessDiameter / 2;
	for (int i = 0; i < kLuCount; ++i)
	{
		for (int j = 0; j < kLuCount; ++j)
		{
			const Player player = m_boardState.at(i).at(j);
			if (player != None)
			{
				const QColor color = (player == Human) ? Qt::black : Qt::white;
				painter.setPen(color);
				painter.setBrush(QBrush(color));
				painter.drawEllipse(kBoardMargin + kCellSideLength * j - chessRadius,
					kBoardMargin + kCellSideLength * i - chessRadius,
					kChessDiameter,
					kChessDiameter);
			}
		}
	}
}

void CBoard::mouseReleaseEvent(QMouseEvent* event)
{
	Base::mouseReleaseEvent(event);

	const int cursorX = event->pos().x();
	const int cursorY = event->pos().y();
	const int range = kChessDiameter / 2;

	int columnNo = (cursorX - kBoardMargin) / kCellSideLength;
	const int x = columnNo * kCellSideLength + kBoardMargin;
	if (x + kCellSideLength - cursorX < range)
		++columnNo;
	else if (cursorX - x > range)
		return;

	int lineNo = (cursorY - kBoardMargin) / kCellSideLength;
	const int y = lineNo * kCellSideLength + kBoardMargin;
	if (y + kCellSideLength - cursorY < range)
		++lineNo;
	else if (cursorY - y > range)
		return;

	playChess(columnNo, lineNo, Human);

	QtConcurrent::run(QThreadPool::globalInstance(), this,
		&CBoard::computeAiNextStep);
}

void CBoard::initUi()
{
	const int sideLength = kCellSideLength * (kLuCount - 1) + 2 * kBoardMargin;
	setFixedSize(sideLength, sideLength);
}

int CBoard::computePositionScore(int posX, int posY, Player player)
{
	QVector<qint8> scores;

	// 纵
	int sameCount = 0;
	int emptyCount = 0;
	for (int y0 = posY - 1; y0 >= 0 && y0 > posY - 5; --y0)
	{
		if (m_boardState.at(y0).at(posX) == player)
			++sameCount;
		else if (m_boardState.at(y0).at(posX) == None)
			++emptyCount;
		else
			break;
	}
	for (int y1 = posY + 1; y1 < kLuCount && y1 < posY + 5; ++y1)
	{
		if (m_boardState.at(y1).at(posX) == player)
			++sameCount;
		else if (m_boardState.at(y1).at(posX) == None)
			++emptyCount;
		else
			break;
	}
	if (emptyCount + sameCount >= 5)
		scores.push_back(sameCount);
	else
		scores.push_back(0);

	// 横
	sameCount = 0;
	emptyCount = 0;
	for (int x0 = posX - 1; x0 >= 0 && x0 > posX - 5; --x0)
	{
		if (m_boardState.at(posY).at(x0) == player)
			++sameCount;
		else if (m_boardState.at(posY).at(x0) == None)
			++emptyCount;
		else
			break;
	}
	for (int x1 = posX + 1; x1 < kLuCount && x1 < posX + 5; ++x1)
	{
		if (m_boardState.at(posY).at(x1) == player)
			++sameCount;
		else if (m_boardState.at(posY).at(x1) == None)
			++emptyCount;
		else
			break;
	}
	if (emptyCount + sameCount >= 5)
		scores.push_back(sameCount);
	else
		scores.push_back(0);
	
	// 左下至右上
	scores.push_back(0);

	// 左上至右下
	scores.push_back(0);

	//qDebug() << QString("position (%1,%2) score: ").arg(posX).arg(posY);
	//foreach (auto s, scores)
	//	qDebug() << s;

	std::sort(scores.begin(), scores.end(), std::greater<qint8>());
	int score = 0;
	foreach (auto s, scores)
		score = score * 10 + s;

	//qDebug() << "score" << score;
	return score;
}

void CBoard::computeAiNextStep()
{
	StepScore humanBestScore, aiBestScore;
	for (int i = 0; i < kLuCount; ++i)
	{
		for (int j = 0; j < kLuCount; ++j)
		{
			if (m_boardState[i][j] == None)
			{
				int score = computePositionScore(j, i, Human);
				if (score > humanBestScore.score)
				{
					humanBestScore.score = score;
					humanBestScore.posX = j;
					humanBestScore.posY = i;
				}

				score = computePositionScore(j, i, Ai);
				if (score > aiBestScore.score)
				{
					aiBestScore.score = score;
					aiBestScore.posX = j;
					aiBestScore.posY = i;
				}
			}
		}
	}

	StepScore bestStep = qMax(aiBestScore, humanBestScore);
	qDebug() << "Best step" << bestStep;
	emit aiStep(bestStep.posX, bestStep.posY);
}

void CBoard::playChess(int x, int y, Player player)
{
	Q_ASSERT(x >= 0 && x < kLuCount);
	Q_ASSERT(y >= 0 && y < kLuCount);

	m_boardState[y][x] = player;
	update();
}

void CBoard::onAiStep(int x, int y)
{
	playChess(x, y, Ai);
}
