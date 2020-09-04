#include "TravelWindow.h"
#include "Solution.h"
#include "Log.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	readFromFile();//从文件里读取信息
	makeMatrix();//生成邻接矩阵
	if (openFile() == false) {
		exit(0);
	}
	QApplication a(argc, argv);
	TravelWindow w;
	w.show();
	return a.exec();
}
