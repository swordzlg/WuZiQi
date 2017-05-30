#include "stdafx.h"
#include "wuziqi.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	CWuZiQi w;
	w.startGame();

	return a.exec();
}
