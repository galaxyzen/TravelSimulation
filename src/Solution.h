#pragma once
#include "TimeTable.h"
#include <QDateTime>
#include <algorithm>

struct PassengerState {
	bool isMoving;//是否在路上
	TrafficInfo routeInfo;//如果在路上,路线信息
	int cityNumber;//如果在某座城市停留,停留城市信息
};//乘客状态结构体

struct Result {
	vector<TrafficInfo> route;//由各个车次(航班)组成的路线
	QDateTime departureTime;//出发时间
	int sumTime = 0;//总用时
	int sumCost = 0;//总费用
};

struct MinTime {
	int spendTime;//从A地到B地的最短时间,包括等车时间
	int index;//从A地到B地的最短时间所对应的车次
	int arriveTime;//到达B地的时间
};

extern Result result;//声明 结果

void makeMatrix();
/*
	功能:制作邻接矩阵
*/
void solveFirst(QDateTime departureTime, int departureCity, int destinatedCity, vector<int> passCities);
/*
	功能:解决"费用最少"问题
	departureTime 出发时间
	departureCity 出发城市编号
	destinatedCity 目的城市编号
	passCities 经停城市们
*/
void solveSecond(QDateTime departureTime, int departureCity, int destinatedCity, vector<int> passCities);
/*
	功能:解决“时间最少”问题
	departureTime 出发时间
	departureCity 出发城市编号
	destinatedCity 目的城市编号
	passCities 经停城市们
*/
void solveThird(QDateTime departureTime, int departureCity, int destinatedCity, vector<int> passCities, int limitTime);
/*
	功能:解决“限时时间,费用最少”问题
	departureTime 出发时间
	departureCity 出发城市编号
	destinatedCity 目的城市编号
	passCities 经停城市们
	limitTime 限时时间
*/
Result dijkstra(int departureTime, int departureCity, int destinatedCity);
/*
	功能:给出任意两地和出发时间,给出最佳路径,被solveSecond调用,致力解决第二种问题
	departureTime 出发时间
	departureCity 始发地
	destinatedCity 目的地
*/
MinTime findMinTime(int startTime, int departureCity, int destinatedCity);
/*
	此函数用来寻找两地之间耗费时间最少的交通方式及其耗费的时间,被dijkstra调用,致力解决第二种问题
	startTime 出发时间
	departureCity 出发城市
	destinatedCity 到达城市
*/
vector<TrafficInfo> makeRouteTable(int strategyNum,int path[],MinTime time[],int departureCity, int destinatedCity);
/*
	功能:为第一种和第二种解决方案制作路线表,路线表由一个个航班组成
	strategyNum 策略号
	path 一维数组,前驱路线(用于第一种策略)
	time 一维数组,采用MinTime中的index,确定两地之间交通方式(用于第二种策略)
	departureCity 起始地
	destinatedCity 目的地
*/
int calculateTime(Result result);
/*
	功能:根据result计算总时间
*/
void permutate(vector<vector<int>>& permResult, vector<int>& passCities, int begin);
/*
	功能:计算必经城市的全排列
	permResult 全排列后的结果,二维数组
	passCities 必经城市们 一维数组
	begin 用做递归,默认为0,passCities第一个元素
*/
void DFS(TrafficInfo preTraffic, int spendTime, vector<TrafficInfo>& tempResult, Result& finalResult
	,int destinatedCity,int limitTime,int depth,int& maxCost, vector<int>&passCities,int spendCost);
/*
	功能:利用回溯法,解决第三种问题
	回溯的条件为:
	1.花费的时间大于限制时间
	2.花费的金钱大于"当前最佳路径"的金钱
	3.递归深度大于MAXDEPTH

	preTraffic 上一次递归调用所传进来的路径,用来判断此次递归调用的 出发城市,和开始时间
	spendTime 从起点调用递归开始到现在所用的总时间,判断是否已经超越时间限制
	tempResult 跟踪单次路线,每递归一层就push_back一个航班,每返回一层递归就pop_back
	finalResult 在递归过程中,每次获得一个更好的路径,便更新finalResult
	destinatedCity 最终要到达的目的地
	limitTime 限制时间,用于剪枝
	depth 递归深度,用于剪枝
	maxCost 当前最好的路径的花费,用于剪枝
	passCities 必经的城市
	spendCost  从起点递归调用开始到现在所用的总费用,判断是否已经超越maxCost
*/
bool isHasPass(vector<TrafficInfo>& tempResult, vector<int>&passCities);
/*
	功能:判断一条路线是否包含必经城市,由DFS调用,用来判断是否更新finalResult
	tempResult 路线
	passCities 毕竟城市
*/
PassengerState currentState(QDateTime pressTime);
/*
	功能:给出乘客当前状态
	pressTime 点击查询按钮时的虚拟时间
	返回值为顾客当前状态结构体
*/