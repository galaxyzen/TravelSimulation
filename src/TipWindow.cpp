#pragma execution_character_set("utf-8")
#include "TipWindow.h"

TipWindow::TipWindow(QWidget *parent)
	: QMainWindow(parent)
{
	setWindowFlags(
		Qt::Window | Qt::WindowTitleHint
		| Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint
	);
	this->setFixedSize(120, 1);
	this->setWindowTitle("º∆À„÷–,«Î…‘∫Û...");
}