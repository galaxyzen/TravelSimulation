#include "TimeTable.h"

vector<TrafficInfo> timeTable;//定义 列车时刻表
vector<CityInfo> cityInfoList;//定义 城市信息(编号,名字,坐标)

void readFromFile()
{
	makeCityList();//制作城市表

	fstream file;
	file.open("traffic.txt", ios::in);//只读方式打开文件
	if (file.is_open() == false) {	//文件打开失败
		exit(0);//退出
	}
	
	TrafficInfo tempInfo;
	string tempStr;
	int tempInt;
	while (file >> tempStr) {
		tempInfo.origin = cityToNumber(tempStr);//始发地
		file >> tempStr;
		tempInfo.destination = cityToNumber(tempStr);//目的地
		file >> tempStr;
		tempInfo.trafficName = tempStr;
		file >> tempInt;
		tempInfo.departureTime = tempInt;//出发时间
		file >> tempInt;
		tempInfo.arrivalTime = tempInt;//到达时间
		file >> tempInt;
		tempInfo.cost = tempInt;//花费
		file >> tempInt;
		tempInfo.trafficType = tempInt;//交通工具类型

		timeTable.push_back(tempInfo);//加入到timeTable里
	}
	file.close();
}

void makeCityList()
{
	//先不管坐标
	CityInfo temp;

	temp.cityName = "Beijing";
	temp.cityPoint = QPoint(718, 305);
	cityInfoList.push_back(temp);

	temp.cityName = "Shanghai";
	temp.cityPoint = QPoint(820, 499);
	cityInfoList.push_back(temp);

	temp.cityName = "Harbin";
	temp.cityPoint = QPoint(856, 150);
	cityInfoList.push_back(temp);

	temp.cityName = "Kunming";
	temp.cityPoint = QPoint(477, 647);
	cityInfoList.push_back(temp);

	temp.cityName = "Lhasa";
	temp.cityPoint = QPoint(271, 525);
	cityInfoList.push_back(temp);

	temp.cityName = "Haikou";
	temp.cityPoint = QPoint(634, 757);
	cityInfoList.push_back(temp);

	temp.cityName = "Zhengzhou";
	temp.cityPoint = QPoint(684, 433);
	cityInfoList.push_back(temp);

	temp.cityName = "Nanjing";
	temp.cityPoint = QPoint(783, 477);
	cityInfoList.push_back(temp);

	temp.cityName = "Changsha";
	temp.cityPoint = QPoint(674, 568);
	cityInfoList.push_back(temp);

	temp.cityName = "Huhhot";
	temp.cityPoint = QPoint(634, 301);
	cityInfoList.push_back(temp);

	temp.cityName = "Chengdu";
	temp.cityPoint = QPoint(512, 528);
	cityInfoList.push_back(temp);

	temp.cityName = "Wuhan";
	temp.cityPoint = QPoint(698, 530);
	cityInfoList.push_back(temp);
}

int cityToNumber(string cityName)
{
	for (unsigned int i = 0; i < cityInfoList.size(); i++) {
		if (cityName == cityInfoList[i].cityName) {
			return i;
		}
	}
}