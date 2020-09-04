#pragma once

#define MAXPASSCITY 3
#define PERSECONDHOURS 2 //每秒对应的小时数
#include <QtWidgets/QMainWindow>
#include "ui_TravelWindow.h"
#include <QDebug>
#include <QPushButton>
#include <QLabel>
#include <QPixMap>
#include <QTimer>
#include <QDateTime>
#include <QPainter>
#include <QCheckBox>
#include <QString>
#include <QMessageBox>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QLineEdit>
#include "TimeTable.h"
#include "Solution.h"
#include "TipWindow.h"
#include "Log.h"
using namespace std;

class TravelWindow : public QMainWindow
{
	Q_OBJECT

public:
	TravelWindow(QWidget *parent = Q_NULLPTR);
	void setCityNames();//为选择途经城市各个按钮设置上城市的名字
	void setTimeLimit();//设置 "设置限时时间" 的权限
	void paintEvent(QPaintEvent* event);//绘画
	bool isTimeLimitRight();//时间限制设置是否正确
	bool isPassCityRight();//必经城市是否包含起点和终点,数量是否太多
	void printRoute();//打印路线
	void analogAnimation();//模拟动画
	void closeEvent(QCloseEvent* ev);//拦截窗口关闭事件
	void printState(PassengerState state);//打印状态
private:
	Ui::TravelWindowClass ui;
	QCheckBox* selectPass_[CITYNUM];//CITYNUM个途经城市
	QPixmap backPix;//背景
	QTimer* transTimer;//用户转换时间的定时器
	QTimer* simulateTimer;//模拟所用计时器
	QDateTime dateTime;//用于时间控制
	QLabel* vehicle[4];//交通工具,用作动画
	QSequentialAnimationGroup* animations;//动画群
	TipWindow* tipWindow;//提示框
};
