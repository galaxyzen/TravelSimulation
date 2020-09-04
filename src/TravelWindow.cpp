#pragma execution_character_set("utf-8")
#include "TravelWindow.h"

TravelWindow::TravelWindow(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	this->setFixedSize(1285, 800);//固定窗口大小为 宽1285 长800
	this->setWindowTitle("Travel");
	tipWindow = new TipWindow;//提示窗
	/*-----------------------------动画设置-----------------------------*/
	for (unsigned int i = 0; i < 4; i++) {
		vehicle[i] = new QLabel(this);
		vehicle[i]->setFixedSize(34, 34);
		vehicle[i]->setScaledContents(true);
		vehicle[i]->hide();//先隐藏
	}
	QPixmap vehiclePic;
	vehiclePic.load(":/TravelWindow/bus");//加载客车图片
	vehicle[0]->setPixmap(vehiclePic);
	vehiclePic.load(":/TravelWindow/train");//加载火车图片
	vehicle[1]->setPixmap(vehiclePic);
	vehiclePic.load(":/TravelWindow/airplan");//加载飞机图片
	vehicle[2]->setPixmap(vehiclePic);
	vehiclePic.load(":/TravelWindow/wait");//加载等待图片
	vehicle[3]->setPixmap(vehiclePic);

	animations = new QSequentialAnimationGroup(this);
	/*----------------------------------答案框设置--------------------------------*/
	ui.answerEdit->setWordWrapMode(QTextOption::WordWrap);//非自动换行
	ui.stateEdit->setWordWrapMode(QTextOption::WordWrap);//非自动换行

	/*-------------------------------选择途经城市UI-------------------------------*/
	for (unsigned int i = 0; i < CITYNUM; i++) {
		selectPass_[i] = new QCheckBox(this);
	}
	setCityNames();

	/*--------------"输入限时时间"的权限设置(只有在选择策略三的时候才能输入)-------------*/
	setTimeLimit();

	/*---------------------------------地图设置--------------------------------------*/
	backPix.load(":/TravelWindow/map");//加载地图图片

	/*--------------------------------按键设置---------------------------------------*/
	ui.stateButton->setEnabled(false);//开始查询按钮不可点击

	/*-------------------------------时间线设置--------------------------------------*/
	simulateTimer = new QTimer(this);
	transTimer = new QTimer(this);
	dateTime = dateTime.currentDateTime();//获取系统时间
	ui.startTime->setDateTime(dateTime);//显示当前时间
	
	updateFile(dateTime.toString("#yyyy/MM/dd hh:mm:ss#").toStdString() 
		+ dateTime.toString(" #yyyy/MM/dd hh:mm:ss#").toStdString()
		+ " 启动系统");//用户启动系统时间

	connect(transTimer, &QTimer::timeout, this, [=]()mutable {
		dateTime = dateTime.addSecs(36 * PERSECONDHOURS);
		ui.startTime->setDateTime(dateTime);//显示当前时间
	});
	transTimer->start(10);//1s->PERSECONDHOURS * 3600s 10ms->PERSECONDHOURS * 36s

	/*--------------------------------生成路线按钮响应设置-----------------------------*/

	connect(ui.startButton, &QPushButton::clicked, this, [=]()mutable {
		simulateTimer->stop();//停止上次模拟
		animations->stop();//停止上次动画
		animations->clear();//删除上次动画
		vehicle[3]->hide();
		ui.startButton->setEnabled(false);//未处理完的时候不可以再点生成路线按钮
		result.route.clear();//清除上次路线
		result.sumCost = 0;//清除耗时
		result.sumTime = 0;//清除金钱
		update();//清除图像
		ui.answerEdit->clear();//清除路线文字框
		ui.sumCost->clear();//清除统计时间框
		ui.sumTime->clear();//清除统计金钱框
		ui.stateEdit->clear();//清除状态文字框
		ui.endTime->setDateTime(dateTime);//刷新到达时间(刚开始和出发时间一样)

		QString tempStr = QDateTime::currentDateTime().toString("#yyyy/MM/dd hh:mm:ss#")
			+ dateTime.toString(" #yyyy/MM/dd hh:mm:ss#")
			+ QString(" 用户查询%1到%2的路线,采取策略%3").arg(cityInfoList[ui.startCity->currentIndex()].cityName.data())
			.arg(cityInfoList[ui.endCity->currentIndex()].cityName.data())
			.arg(ui.strategy->currentIndex() + 1);
		vector<int> passCities;//经停城市
		for (unsigned int i = 0; i < CITYNUM; i++) {	//获取经停城市
			if (selectPass_[i]->isChecked() == true) {
				passCities.push_back(i);//加入到经停城市vector
			}
		}
		if (passCities.size() == 0) {
			tempStr += QString(",未选择必经城市");
		}
		else {
			tempStr += QString(",选择的必经城市为:");
			for (unsigned int i = 0; i < passCities.size(); i++) {
				tempStr += QString(cityInfoList[passCities[i]].cityName.data());
				tempStr += QString(" ");
			}
		}
		updateFile(tempStr.toStdString());

		if (ui.strategy->currentIndex() == 2) {	//选择第三个策略
			if (isTimeLimitRight() == false) {	//时间限制输入不合法
				updateFile("错误,时间限制不合法");
				updateFile("-----------------------------------------------------------------------------");
				return;
			}
		}
		if (isPassCityRight() == false) {
			updateFile("错误,必经城市选择不合法");
			updateFile("-----------------------------------------------------------------------------");
			return;
		}
		if (ui.startCity->currentIndex() == ui.endCity->currentIndex()) {
			QMessageBox::critical(this, "错误", "始发地和目的地不能一样");
			updateFile("错误,始发地和目的地选择重复");
			updateFile("-----------------------------------------------------------------------------");
			return;
		}
		else {	//时间限制合法,必经城市选择合法,出发地和目的地不同
			transTimer->stop();//停止计时
			QDateTime departureTime = dateTime;//获取出发时间

			int strategyNum = ui.strategy->currentIndex();//获取策略编号
			int departureCity = ui.startCity->currentIndex();//获取出发城市编号
			int destinatedCity = ui.endCity->currentIndex();//获取目的地城市编号
		
			//下面开始计算
			if (strategyNum == 0) {
				solveFirst(departureTime, departureCity, destinatedCity, passCities);
			}
			else if (strategyNum == 1) {
				solveSecond(departureTime, departureCity, destinatedCity, passCities);
			}
			else {
				tipWindow->show();
				int limitTime = ui.limitEdit->text().toInt();//获取限制时间
				solveThird(departureTime, departureCity, destinatedCity, passCities, limitTime);
				tipWindow->hide();
			}
			if (result.route.size() > 0) {	//说明存在解决方案
				printRoute();
				update();//刷新画面,画图
				ui.stateButton->setEnabled(true);//现在查询按钮可以点
				ui.stateButton->setFocus();//设置焦点
				analogAnimation();//调用动画
				transTimer->start(10);//开始计时

				simulateTimer->start(result.sumTime * 1000 / PERSECONDHOURS);
				connect(simulateTimer, &QTimer::timeout, this, [=]() {
					simulateTimer->stop();
					vehicle[3]->move(cityInfoList[result.route[result.route.size() - 1].destination].cityPoint.x() - (vehicle[3]->width() / 2)
						, cityInfoList[result.route[result.route.size() - 1].destination].cityPoint.y() - (vehicle[3]->height() / 2));
					vehicle[3]->show();
					ui.stateEdit->setText("到达目的地");
					ui.stateButton->setEnabled(false);//经过 sumTime 时间后查询按钮不可以点
					ui.startButton->setEnabled(true);//经过 sumTime 时间后生成按钮可以点
				});
				connect(ui.stateButton, &QPushButton::clicked, this, [=]() {
					QString tempStr = QDateTime::currentDateTime().toString("# yyyy/MM/dd hh:mm:ss #")
						+ dateTime.toString(" #yyyy/MM/dd hh:mm:ss#")
						+ QString(" 用户查询当前状态");
					updateFile(tempStr.toStdString());
					PassengerState state = currentState(dateTime);//传入当前时间
					//这里先不暂停了,因为算的巨快
					printState(state);//打印状态
				});
			}
			else {
				updateFile("按上述要求无解决方案");
				updateFile("-----------------------------------------------------------------------------");
				QMessageBox::critical(this, "错误", "抱歉,此情况不存在解决方案");
				transTimer->start();//继续计时
				ui.stateButton->setEnabled(false);//查询按钮不可以点
				ui.startButton->setEnabled(true);//生成按钮可以点
			}
		}
		ui.startButton->setEnabled(true);
	});
}

void TravelWindow::printRoute()
{
	QDateTime arrivalTime = result.departureTime.addSecs(result.sumTime * 3600);
	ui.endTime->setDateTime(arrivalTime);//显示到达时间
	ui.sumTime->setText(QString("%1小时").arg(result.sumTime));//显示耗时
	ui.sumCost->setText(QString("%1元").arg(result.sumCost));//显示花费
	QString tempStr;
	QString start;//起始城市
	QString end;//目的城市
	QString type;//交通工具类型
	QDateTime currentTime = result.departureTime;//当前时间
	
	for (unsigned int i = 0; i < result.route.size(); i++) {
		start = cityInfoList[result.route[i].origin].cityName.data();//起始城市
		end = cityInfoList[result.route[i].destination].cityName.data();//目的城市
		switch (result.route[i].trafficType) {
			case 0: {
				type = "汽车";
				break;
			}
			case 1: {
				type = "火车";
				break;
			}
			default: {
				type = "飞机";
				break;
			}
		}//获取交通方式
		if (i == 0) {	//刚到车站
			QString final = cityInfoList[result.route[result.route.size() - 1].destination].cityName.data();//最终目的地
			tempStr = QString("[%1] 在%2准备出发前往%3").arg(currentTime.toString("yyyy/MM/dd hh时")).arg(start)
				.arg(final);
			ui.answerEdit->append(tempStr);
			updateFile(tempStr.toStdString());
		}
		
		int waitTime = result.route[i].departureTime - currentTime.time().hour();
		if (waitTime < 0) {	//晚点了
			waitTime += 24;
		}
		currentTime = currentTime.addSecs(waitTime * 3600);
		tempStr = QString("[%1] 在%2乘坐%3号").arg(currentTime.toString("yyyy/MM/dd hh时")).arg(start).arg(result.route[i].trafficName.data())
			+ type + QString("前往%1").arg(end);
		ui.answerEdit->append(tempStr);
		updateFile(tempStr.toStdString());
		int moveTime = result.route[i].arrivalTime - result.route[i].departureTime;
		if (moveTime < 0) {
			moveTime += 24;
		}
		currentTime = currentTime.addSecs(moveTime * 3600);//航班到达时间
		tempStr = QString("[%1] 到达%2").arg(currentTime.toString("yyyy/MM/dd hh时")).arg(end);
		ui.answerEdit->append(tempStr);
		updateFile(tempStr.toStdString());
	}
	updateFile("-----------------------------------------------------------------------------");
}

void TravelWindow::setCityNames()//为选择途经城市各个按钮设置上城市的名字
{
	unsigned int x = ui.selectLabel->x() + 10;
	unsigned int y = ui.selectLabel->y() + 8;
	for (unsigned int i = 0; i < CITYNUM; i++) {
		selectPass_[i]->move(x, y);
		if ((i + 1) % 3 == 0 && i != 0) {
			y += 30;
			x = ui.selectLabel->x() + 10;
		}
		else {
			x += 100;
		}
	}
	selectPass_[0]->setText("北京");
	selectPass_[1]->setText("上海");
	selectPass_[2]->setText("哈尔滨");
	selectPass_[3]->setText("昆明");
	selectPass_[4]->setText("拉萨");
	selectPass_[5]->setText("海口");
	selectPass_[6]->setText("郑州");
	selectPass_[7]->setText("南京");
	selectPass_[8]->setText("长沙");
	selectPass_[9]->setText("呼和浩特");
	selectPass_[10]->setText("成都");
	selectPass_[11]->setText("武汉");
}

void TravelWindow::setTimeLimit()//"输入限时时间"的权限设置(只有在选择策略三的时候才能输入)
{
	ui.limitEdit->setMaxLength(3);//最长为999小时
	ui.limitEdit->setReadOnly(true);//默认设置为只读
	ui.limitEdit->setPlaceholderText("不可输入");//默认显示不可输入
	connect(ui.strategy, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, [=](int index) {
		if (index == 2) {	//如果选择的是第三个策略
			ui.limitEdit->setPlaceholderText("整数,单位:小时");//默认显示清除
			ui.limitEdit->setReadOnly(false);//可写
			ui.limitEdit->setFocus();//自动定位光标到编辑框处
		}
		else {
			ui.limitEdit->clear();//清除编辑框中的内容
			ui.limitEdit->setReadOnly(true);//设置为只读
			ui.limitEdit->setPlaceholderText("不可输入");//默认显示不可输入
		}
	});
}


void TravelWindow::paintEvent(QPaintEvent* ev)
{
	QPainter painter(this);
	QRect mapRect(ui.mapLabel->x(), ui.mapLabel->y(), ui.mapLabel->width(), ui.mapLabel->height());
	painter.drawPixmap(mapRect, backPix);
	QPen pen;
	pen.setColor(QColor("red"));
	pen.setWidth(3);//笔宽为5
	painter.setPen(pen);//绑定笔
	if (result.route.size() > 0) {
		QPoint start;
		QPoint end;
		for (unsigned int i = 0; i < result.route.size(); i++) {
			start = cityInfoList[result.route[i].origin].cityPoint;
			end = cityInfoList[result.route[i].destination].cityPoint;
			painter.drawLine(start, end);
		}
		//下面把经过的点变色
		pen.setWidth(15);
		painter.setPen(pen);
		for (unsigned int i = 0; i < result.route.size(); i++) {
			painter.drawPoint(cityInfoList[result.route[i].origin].cityPoint);
			if (i == result.route.size() - 1) {
				painter.drawPoint(cityInfoList[result.route[i].destination].cityPoint);
			}
		}
	}
}

bool TravelWindow::isTimeLimitRight()
{
	QString timeLimit = ui.limitEdit->text();
	if (timeLimit.isEmpty() == true) {	//未输入
		QMessageBox::critical(this, "错误", "请输入时间限制");
		return false;
	}
	else {	//输入,判读输入是否正确(纯数字)
		for (unsigned int i = 0; i < timeLimit.size(); i++) {
			if (timeLimit[i] > '9' || timeLimit[i] < '0') {
				QMessageBox::critical(this, "错误", "时间限制请输入整数");
				return false;
			}
		}
		return true;
	}
}

bool TravelWindow::isPassCityRight()
{
	int count = 0;
	for (unsigned int i = 0; i < CITYNUM; i++) {
		if (selectPass_[i]->isChecked() == true) {	//被选中
			if (i == ui.startCity->currentIndex() || i == ui.endCity->currentIndex()) {
				QMessageBox::critical(this, "错误", "必经城市不可选择起点和终点");
				return false;
			}
			else {
				count++;
				if (count > MAXPASSCITY) {
					QMessageBox::critical(this, "错误", QString("必经城市最多选择%1个").arg(MAXPASSCITY));
					return false;
				}
			}
		}
	}
	return true;
}

void TravelWindow::analogAnimation()
{
	//串行动画
	QPropertyAnimation* perAnimation = nullptr;
	int waitTime = 0;
	int moveTime = 0;
	QPoint start;
	QPoint end;
	for (unsigned int i = 0; i < result.route.size(); i++) {
		//算等待时间
		if (i == 0) {	//第一趟航班
			waitTime = result.route[i].departureTime - result.departureTime.time().hour();
			if (waitTime < 0) {
				waitTime += 24;
			}
		}
		else {
			waitTime = result.route[i].departureTime - result.route[i - 1].arrivalTime;//此航班发车时间-上次航班到达时间
			if (waitTime < 0) {
				waitTime += 24;
			}
		}
		//制作等待动画
		perAnimation = new QPropertyAnimation(vehicle[3], "pos",this);
		perAnimation->setDuration(waitTime * 1000 / PERSECONDHOURS);//转换成毫秒
		start = QPoint(cityInfoList[result.route[i].origin].cityPoint.x() - (vehicle[3]->width() / 2),
			cityInfoList[result.route[i].origin].cityPoint.y() - (vehicle[3]->height() / 2));
		end = QPoint(cityInfoList[result.route[i].origin].cityPoint.x() - (vehicle[3]->width() / 2),
			cityInfoList[result.route[i].origin].cityPoint.y() - (vehicle[3]->height() / 2));
		perAnimation->setStartValue(start);//动画开始坐标
		perAnimation->setEndValue(end);//动画结束坐标
		//开始坐标和结束坐标一致,代表等待
		animations->addAnimation(perAnimation);
		connect(perAnimation, &QAbstractAnimation::stateChanged, this, [=](int newState, int oldState) {
			if (newState == 2 && oldState == 0) {	//动画由stop转为run,显示图片
				vehicle[3]->show();
			}
			else if (newState == 0 && oldState == 2) {	//动画由run转为stop,隐藏图片
				vehicle[3]->hide();
			}
		});
		//算移动时间
		moveTime =  result.route[i].arrivalTime - result.route[i].departureTime;//到达时间减去出发时间
		if (moveTime <= 0) {
			moveTime += 24;
		}
		perAnimation = new QPropertyAnimation(vehicle[result.route[i].trafficType], "pos",this);
		perAnimation->setDuration(moveTime * 1000 / PERSECONDHOURS);//转换成毫秒
		start = QPoint(cityInfoList[result.route[i].origin].cityPoint.x() - (vehicle[result.route[i].trafficType]->width() / 2),
			cityInfoList[result.route[i].origin].cityPoint.y() - (vehicle[result.route[i].trafficType]->height() / 2));
		end = QPoint(cityInfoList[result.route[i].destination].cityPoint.x() - (vehicle[result.route[i].trafficType]->width() / 2),
			cityInfoList[result.route[i].destination].cityPoint.y() - (vehicle[result.route[i].trafficType]->height() / 2));
		perAnimation->setStartValue(start);//动画开始坐标
		perAnimation->setEndValue(end);//动画结束坐标
		animations->addAnimation(perAnimation);
		connect(perAnimation, &QAbstractAnimation::stateChanged, this, [=](int newState, int oldState) {
			if (newState == 2 && oldState == 0) {	//动画由stop转为run,显示图片
				vehicle[result.route[i].trafficType]->show();
			}
			else if (newState == 0 && oldState == 2) {	//动画由run转为stop,隐藏图片
				vehicle[result.route[i].trafficType]->hide();
			}
		});
	}
	animations->start();
}


void TravelWindow::printState(PassengerState state)
{
	ui.stateEdit->clear();//清除上一次的状态打印
	if (state.isMoving == false) {	//在车站等车
		QString tempStr = QString("在%1站等待").arg(cityInfoList[state.cityNumber].cityName.data());
		ui.stateEdit->append(tempStr);
		updateFile(tempStr.toStdString());
	}
	else {
		QString type;//交通工具类型
		switch (state.routeInfo.trafficType) {
		case 0: {
			type = "汽车上";
			break;
		}
		case 1: {
			type = "火车上";
			break;
		}
		default: {
			type = "飞机上";
			break;
		}
		}
		QString tempStr = QString("在从%1前往%2的%3号").arg(cityInfoList[state.routeInfo.origin].cityName.data())
			.arg(cityInfoList[state.routeInfo.destination].cityName.data())
			.arg(state.routeInfo.trafficName.data()) + type;
		ui.stateEdit->append(tempStr);
		updateFile(tempStr.toStdString());
	}
	updateFile("-----------------------------------------------------------------------------");
}


void TravelWindow::closeEvent(QCloseEvent* ev)
{
	animations->stop();
	animations->clear();
	delete tipWindow;
	updateFile(QDateTime::currentDateTime().toString("#yyyy/MM/dd hh:mm:ss#").toStdString() 
		+ dateTime.toString(" #yyyy/MM/dd hh:mm:ss#").toStdString()
		+ " 退出系统");
	updateFile("******************************************************************************");
	logFile.close();
}