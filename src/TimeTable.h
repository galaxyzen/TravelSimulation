#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <QPoint>
using namespace std;
#define CITYNUM 12
#define MAXCOST 100000

struct TrafficInfo {
	int origin = -1;//此(车次)航班始发地
	int destination = -1;//此(车次)航班目的地
	string trafficName = "Noon";//交通工具的具体名称 G9003
	int departureTime;//出发时间,精确到小时
	int arrivalTime;//到达时间,精确到小时
	int cost;//此车次花费多少钱
	int trafficType;//交通工具的类型	0->汽车 1->火车 2->飞机
};

struct CityInfo {
	string cityName;//城市名称
	QPoint cityPoint;//城市坐标
};
//城市编号(vector下标代表编号),名字,坐标

extern vector<CityInfo> cityInfoList;//vector中的下标代表城市编号
extern vector<TrafficInfo> timeTable;//声明 列车时刻表

void readFromFile();//从文件读取各车次的信息
void makeCityList();//生成城市信息表
int cityToNumber(string cityName);
/*
	功能:把城市转化为相应的编号
	cityName 城市名称
*/