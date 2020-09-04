#include "Solution.h"

static vector<TrafficInfo> allAdjMatrix[CITYNUM][CITYNUM];//包含所有车次(航班)信息的邻接矩阵
static TrafficInfo firstAdjMatrix[CITYNUM][CITYNUM];//只包含两地之间最便宜交通方式的邻接矩阵
static int MAXDEPTH = 6;//基础递归深度,未考虑必经城市,后面会加上必经城市
Result result;//定义 结果

void makeMatrix()
{
	/*
		allAdjMatrix[i][j]处的vector的size为0,说明 i 地到 j 地 无列车
		firstAdjMatrix[i][j]处的cost 为MAXCOST,说明 i 地到 j 地 无列车
	*/

	int start, end;//始发地 目的地
	TrafficInfo temp;
	temp.cost = MAXCOST;
	for (unsigned int i = 0; i < CITYNUM; i++) {
		for (unsigned int j = 0; j < CITYNUM; j++) {
			firstAdjMatrix[i][j] = temp;//初始化第一种策略的邻接矩阵
		}
	}

	for (unsigned int i = 0; i < timeTable.size(); i++) {
		start = timeTable[i].origin;//始发地
		end = timeTable[i].destination;//目的地
		allAdjMatrix[start][end].push_back(timeTable[i]);//加入到allAdjMatrix[start][end]的vector中
		if (timeTable[i].cost < firstAdjMatrix[start][end].cost) {	//如果此花费较小,替换先前交通方式
			firstAdjMatrix[start][end] = timeTable[i];
		}
	}

}

void solveFirst(QDateTime departureTime, int departureCity, int destinatedCity, vector<int> passCities)
{
	//弗洛伊德算法
	unsigned int cost[CITYNUM][CITYNUM];//两地之间最小花费
	int path[CITYNUM][CITYNUM];//最短路径记录

	for (unsigned int i = 0; i < CITYNUM; i++) {
		for (unsigned j = 0; j < CITYNUM; j++) {
			cost[i][j] = firstAdjMatrix[i][j].cost;
			if (cost[i][j] < MAXCOST && i != j) {	//i->j 有弧
				path[i][j] = i;//j的前驱为i
			}
			else {
				path[i][j] = -1;//i->j无弧
			}
		}
	}
	for (unsigned int k = 0; k < CITYNUM; k++) {
		for (unsigned int i = 0; i < CITYNUM; i++) {
			for (unsigned int j = 0; j < CITYNUM; j++) {
				if (cost[i][k] + cost[k][j] < cost[i][j]) {	//i->j的花费大于i->k->j的花费
					cost[i][j] = cost[i][k] + cost[k][j];
					path[i][j] = path[k][j];//j的前驱改为i
				}
			}
		}
	}

	if (passCities.size() > 0) {	//选择了途经城市
		vector<vector<int>> permResult;//储存全排列结果
		permutate(permResult, passCities, 0);//全排列
		//下面把起始地和目的地加入到全排列中,组成完整的路线
		for (unsigned int i = 0; i < permResult.size(); i++) {
			permResult[i].insert(permResult[i].begin(), departureCity);//在头部插入起始地
			permResult[i].push_back(destinatedCity);//在尾部加入目的地
		}

		unsigned int minCostIndex = 0;//最小费用方案在n!个排列中的编号
		unsigned int minCost = INT_MAX;//最小费用
		unsigned int start;//起始地
		unsigned int end;//目的地

		//选取最优策略
		for (unsigned int i = 0; i < permResult.size(); i++) {
			unsigned int realCost = 0;//每个方案的实际花费
			for (unsigned int j = 0; j < permResult[i].size() - 1; j++) {
				start = permResult[i][j];//本段起始地
				end = permResult[i][j + 1];//本段目的地
				realCost += cost[start][end];//本段花费			         
			}
			if (realCost < minCost) {
				minCost = realCost;
				minCostIndex = i;
			}
		}

		//minCostIndex即为最优秀的策略,下面需要拼接
		vector<TrafficInfo> directRoute;//离散的路径
		MinTime time[CITYNUM];//无用
		for (unsigned int i = 0; i < permResult[minCostIndex].size() - 1; i++) {
			start = permResult[minCostIndex][i];//本段起始地
			end = permResult[minCostIndex][i + 1];//本段目的地
			directRoute = makeRouteTable(0, path[start], time, start, end);//获取本段航班表
			result.route.insert(result.route.end(), directRoute.begin(), directRoute.end());//加入到真正结果的路线表中
		}

		result.sumCost = minCost;//总花费
		result.departureTime = departureTime;//实际出发时间(不是第一个航班的出发时间)
	}
	else {	//未选择必经城市
		MinTime time[CITYNUM];//无用
		result.route = makeRouteTable(0, path[departureCity], time, departureCity, destinatedCity);
		result.sumCost = cost[departureCity][destinatedCity];//总费用
		result.departureTime = departureTime;//实际出发时间(不是第一个航班的出发时间)
	}

	result.sumTime = calculateTime(result);//计算时间

}

void solveSecond(QDateTime departureTime, int departureCity, int destinatedCity, vector<int> passCities)
{
	if (passCities.size() > 0) {
		vector<vector<int>> permResult;//储存全排列结果
		permutate(permResult, passCities, 0);//全排列
		//下面把起始地和目的地加入到全排列中,组成完整的路线
		for (unsigned int i = 0; i < permResult.size(); i++) {
			permResult[i].insert(permResult[i].begin(), departureCity);//在头部插入起始地
			permResult[i].push_back(destinatedCity);//在尾部加入目的地
		}

		unsigned int minTimeIndex = 0;//最小时间方案在n!个排列中的编号
		unsigned int minTime = INT_MAX;//最小时间
		unsigned int start;//起始地
		unsigned int end;//目的地
		int  directStartTime = 0;
		for (unsigned int i = 0; i < permResult.size(); i++) {	//每一种方案
			directStartTime = departureTime.time().hour();//每段路径的开始时间
			Result tempResult;
			unsigned int realTime = 0;//每个方案的实际时间
			for (unsigned int j = 0; j < permResult[i].size() - 1; j++) {
				start = permResult[i][j];//本段起始地
				end = permResult[i][j + 1];//本段目的地
				tempResult = dijkstra(directStartTime, start, end);//缓存本段结果
				directStartTime = tempResult.route[tempResult.route.size() - 1].arrivalTime;//更新开始时间
				realTime += tempResult.sumTime;//累计总时间
			}
			if (realTime < minTime) {
				minTime = realTime;
				minTimeIndex = i;
			}
		}
		//minTimeIndex即为最优秀的策略,下面需要拼接
		vector<TrafficInfo> directRoute;//离散的路径
		directStartTime = departureTime.time().hour();//每段路径的开始时间
		for (unsigned int i = 0; i < permResult[minTimeIndex].size() - 1; i++) {
			start = permResult[minTimeIndex][i];//本段起始地
			end = permResult[minTimeIndex][i + 1];//本段目的地
			directRoute = dijkstra(directStartTime, start, end).route;//获取本段航班表
			directStartTime = directRoute[directRoute.size() - 1].arrivalTime;//更新出发时间
			result.route.insert(result.route.end(), directRoute.begin(), directRoute.end());//加入到真正结果的路线表中
		}

		//算费用
		for (unsigned int i = 0; i < result.route.size(); i++) {
			result.sumCost += result.route[i].cost;
		}
		result.departureTime = departureTime;//实际出发时间(不是第一个航班的出发时间)
		result.sumTime = minTime;
	}
	else {	//未选择途经城市
		result = dijkstra(departureTime.time().hour(), departureCity, destinatedCity);
		//算费用
		for (unsigned int i = 0; i < result.route.size(); i++) {
			result.sumCost += result.route[i].cost;
		}
		result.departureTime = departureTime;//实际出发时间(不是第一个航班的出发时间)
	}
}

void solveThird(QDateTime departureTime, int departureCity, int destinatedCity, vector<int> passCities, int limitTime)
{
	solveFirst(departureTime, departureCity, destinatedCity, passCities);
	if (result.sumTime > limitTime) {	//最小费用不满足限制时间
		result.departureTime = departureTime;
		result.route.clear();
		result.sumCost = 0;
		result.sumTime = 0;
		solveSecond(departureTime, departureCity, destinatedCity, passCities);
		if (result.sumTime > limitTime) {	//最小时间都大于限制时间了说明无路径
			result.departureTime = departureTime;
			result.route.clear();
			result.sumCost = 0;
			result.sumTime = 0;
		}
		else {
			int maxCost = result.sumCost;//“当前最优路径”花费的金钱,最优路径指的是在限定时间内,金钱花费最少的路径
			result.departureTime = departureTime;
			result.route.clear();
			result.sumCost = 0;
			result.sumTime = 0;
			vector<TrafficInfo> tempResult;//跟踪单次路线
			Result finalResult;//“当前”最好路径
			int spendTime = 0;//当前花费的时间
			int depth = 0;//当前递归深度
			int spendCost = 0;//当前花费金钱
			
			for (unsigned int i = 0; i < CITYNUM; i++) {	//尝试从depatureCity前往下一个城市
				if (i != departureCity) {
					for (unsigned int j = 0; j < allAdjMatrix[departureCity][i].size(); j++) {//depatureCity前往下一个城市的所有交通方式
							//先算spendTime
						int waitTime = allAdjMatrix[departureCity][i][j].departureTime - departureTime.time().hour();
						if (waitTime < 0) {
							waitTime += 24;
						}
						int moveTime = allAdjMatrix[departureCity][i][j].arrivalTime
							- allAdjMatrix[departureCity][i][j].departureTime;
						if (moveTime < 0) {
							moveTime += 24;
						}
						spendTime += waitTime;
						spendTime += moveTime;
						//和spendCost
						spendCost += allAdjMatrix[departureCity][i][j].cost;
						//将此航班push_back到tempResult
						tempResult.push_back(allAdjMatrix[departureCity][i][j]);
						//调用DFS
						DFS(allAdjMatrix[departureCity][i][j], spendTime, tempResult,
							finalResult, destinatedCity, limitTime, depth++, maxCost, passCities, spendCost);
						tempResult.pop_back();//以航班开始的路线递归完毕,将其pop_back出去开启下一次递归
						depth--;//返回后 递归深度减一
						spendTime -= waitTime;//花费时间恢复
						spendTime -= moveTime;//花费时间恢复
						spendCost -= allAdjMatrix[departureCity][i][j].cost;//花费金钱恢复
					}
				}
			}
			result = finalResult;
			result.departureTime = departureTime;
		}
	}	
}

Result dijkstra(int departureTime, int departureCity, int destinatedCity)
{
	bool isJoin[CITYNUM];//被加入的点的集合
	MinTime time[CITYNUM];// 花费的时间 所用班次编号 到达的时间
	int path[CITYNUM];//生成的前驱路径
	unsigned int MAXTIME = SHRT_MAX;//最大时间
	for (unsigned int i = 0; i < CITYNUM; i++) {
		isJoin[i] = false;//除了源点都没加入
		if (i != departureCity) {
			time[i] = findMinTime(departureTime, departureCity, i); //确认源点到某个点是否有直接路径 确定交通方式

			if (time[i].spendTime < MAXTIME) {
				path[i] = departureCity;
			}
			else {
				path[i] = -1;//无直接路径
			}
		}
	}

	isJoin[departureCity] = true;//将源点加入集合
	//time[departureCity].spendTime = 0;
	//初始化结束

	for (unsigned int i = 0; i < CITYNUM - 1; i++) {	//i用于循环,即经过 CITYNUM - 1次循环将剩下CITYNUM - 1个城市加进来
		unsigned int minTime = SHRT_MAX;
		int joinCity;
		for (unsigned int j = 0; j < CITYNUM; j++) {
			if (isJoin[j] == false && time[j].spendTime < minTime) {
				joinCity = j;
				minTime = time[j].spendTime;
			}
		}
		isJoin[joinCity] = true;//将此点加入进来
		for (unsigned int j = 0; j < CITYNUM; j++) {	//更新
			if (isJoin[j] == false) {	//不是集合中的点
				MinTime temp = findMinTime(time[joinCity].arriveTime, joinCity, j);//寻找此点到j点的最少时间及其交通方式
				if (time[joinCity].spendTime + temp.spendTime < time[j].spendTime) {
					time[j].spendTime = time[joinCity].spendTime + temp.spendTime;//更新时间
					time[j].index = temp.index;//获取车次,到达它的车次
					time[j].arriveTime = temp.arriveTime;
					path[j] = joinCity;//更新前驱
				}
			}
		}
	}
	Result tempResult;
	tempResult.route = makeRouteTable(1, path, time, departureCity, destinatedCity);
	QDateTime tempDateTime;
	tempDateTime.setTime(QTime(departureTime, 0));
	tempResult.departureTime = tempDateTime;

	//qDebug() << tempDateTime.time().hour() << endl;

	tempResult.sumTime = calculateTime(tempResult);//计算本段时间
	return tempResult;
}

MinTime findMinTime(int startTime, int departureCity, int destinatedCity)
{
	MinTime time;
	unsigned int minTime = SHRT_MAX;
	unsigned int minTimeIndex = 0;
	for (int i = 0; i < allAdjMatrix[departureCity][destinatedCity].size(); i++) {
		int waitTime = 0;
		int moveTime = 0;
		int realTime = 0;
		waitTime = allAdjMatrix[departureCity][destinatedCity][i].departureTime - startTime;
		if (waitTime < 0) {
			waitTime += 24;
		}
		moveTime = allAdjMatrix[departureCity][destinatedCity][i].arrivalTime
			- allAdjMatrix[departureCity][destinatedCity][i].departureTime;//到达时间减去出发时间
		if (moveTime < 0) {
			moveTime += 24;
		}
		realTime += waitTime;//时间累计
		realTime += moveTime;//时间累计
		if (realTime < minTime) {
			minTime = realTime;
			minTimeIndex = i;
		}
	}

	time.spendTime = minTime;
	//这里要做判断的...
	//因为有可能两地之间无直达列车
	//那么allAdjMatrix[departureCity][destinatedCity][minTimeIndex]访问越界
	if (allAdjMatrix[departureCity][destinatedCity].size() > 0) {
		time.index = minTimeIndex;
		time.arriveTime = allAdjMatrix[departureCity][destinatedCity][minTimeIndex].arrivalTime;
	}

	return time;
}

vector<TrafficInfo> makeRouteTable(int strategyNum,int path[],MinTime time[],int departureCity, int destinatedCity)
{
	vector<TrafficInfo> tempRoute;
	unsigned int resultPath[CITYNUM];//最终路径
	resultPath[CITYNUM - 1] = destinatedCity;//目的地
	int count = CITYNUM - 1;
	while (path[destinatedCity] != departureCity) {	//还没回溯到起点
		resultPath[--count] = path[destinatedCity];
		destinatedCity = path[destinatedCity];
	}
	resultPath[--count] = departureCity;//起始地
	for (unsigned int i = count; i < CITYNUM - 1; i++) {
		unsigned int start = resultPath[i];//航班起始地
		unsigned int end = resultPath[i + 1];//航班目的地
		if (strategyNum == 0) {
			tempRoute.push_back(firstAdjMatrix[start][end]);//航班加入到结果vector
		}
		else if (strategyNum == 1) {
			tempRoute.push_back(allAdjMatrix[start][end][time[end].index]);//航班加入到结果vector
		}
	}
	return tempRoute;
}

int calculateTime(Result tempResult)
{
	int waitTime = 0;//在某车站等待的时间
	int moveTime = 0;//在某段路程行走的时间
	for (unsigned int i = 0; i < tempResult.route.size(); i++) {
		if (i == 0) {	//第一趟航班
			waitTime = tempResult.route[i].departureTime - tempResult.departureTime.time().hour();//第一个航班出发时间减去到达车站时间
		}
		else {
			waitTime = tempResult.route[i].departureTime - tempResult.route[i - 1].arrivalTime;//此航班出发时间减去上次航班到达时间
		}
		if (waitTime < 0) {	//晚点到机场(车站)或者跨天
			waitTime = waitTime + 24;//再等一天或者跨天
		}
		moveTime = tempResult.route[i].arrivalTime - tempResult.route[i].departureTime;//到达时间减去出发时间,即移动时间
		if (moveTime <= 0) {
			moveTime = moveTime + 24;//跨天
		}
		tempResult.sumTime += waitTime;//时间累计
		tempResult.sumTime += moveTime;//时间累计
	}
	return tempResult.sumTime;
}

void permutate(vector<vector<int>>& permResult, vector<int>& passCities, int begin)
{
	if (begin + 1 == passCities.size()) {	//此次递归结束
		permResult.push_back(passCities);
	}
	else {
		for (unsigned int i = begin; i < passCities.size(); i++) {
			swap(passCities[begin], passCities[i]);//交换
			permutate(permResult, passCities, begin + 1);//递归
			swap(passCities[begin], passCities[i]);//递归完毕换回,进行下一次交换
		}
	}
}

void DFS(TrafficInfo preTraffic, int spendTime, vector<TrafficInfo>& tempResult, Result& finalResult,
	int destinatedCity,int limitTime,int depth, int& maxCost, vector<int>& passCities, int spendCost)
{
	if (spendTime > limitTime || depth > MAXDEPTH || spendCost >= maxCost) {	//超过时间限制或者超过递归深度或者金钱限制
		return;//返回上一层
	}
	else if (preTraffic.destination == destinatedCity) {	//到达终点
		//此处判断tempResult是否满足必经和小于maxCost
		if (passCities.size() > 0) {	//选择了必经城市
			//先判断是否满足包含条件
			if (isHasPass(tempResult, passCities) == true) {
				maxCost = spendCost;
				finalResult.route = tempResult;
				finalResult.sumTime = spendTime;
				finalResult.sumCost = spendCost;//更新final
			}
		}
		else {	//没选必经城市
			maxCost = spendCost;
			finalResult.route = tempResult;
			finalResult.sumTime = spendTime;
			finalResult.sumCost = spendCost;//更新final
		}
		return;
	}
	else {	//其他
		int departureCity = preTraffic.destination;//此次的出发城市,是上次航班的到达城市
		int departureTime = preTraffic.arrivalTime;//此次出发的时间,是上次航班的到达时间
		for (unsigned int i = ((departureCity + 1) % CITYNUM); i != departureCity; (i = (i + 1) % CITYNUM)) {
			for (unsigned j = 0; j < allAdjMatrix[departureCity][i].size(); j++) {
				//先算spendTime
				///和spendCost
				int waitTime = allAdjMatrix[departureCity][i][j].departureTime - departureTime;
				if (waitTime < 0) {
					waitTime += 24;
				}
				int moveTime = allAdjMatrix[departureCity][i][j].arrivalTime
					- allAdjMatrix[departureCity][i][j].departureTime;
				if (moveTime < 0) {
					moveTime += 24;
				}
				spendTime += waitTime;
				spendTime += moveTime;
				//算spendCost
				spendCost += allAdjMatrix[departureCity][i][j].cost;
				//将此航班push_back到tempResult
				tempResult.push_back(allAdjMatrix[departureCity][i][j]);
				//调用DFS
				DFS(allAdjMatrix[departureCity][i][j], spendTime,
					tempResult, finalResult, destinatedCity, limitTime, depth++, maxCost, passCities, spendCost);
				tempResult.pop_back();//以航班开始的路线递归完毕,将其pop_back出去开启下一次递归
				depth--;//返回后 递归深度减一
				spendTime -= waitTime;//花费时间恢复
				spendTime -= moveTime;//花费时间恢复
				spendCost -= allAdjMatrix[departureCity][i][j].cost;//花费金钱恢复
			}
		}
	}
}

bool isHasPass(vector<TrafficInfo>& tempResult, vector<int>&passCities)
{

	unsigned int j = 0;
	for (unsigned int i = 0; i < passCities.size(); i++) {
		for (j = 0; j < tempResult.size(); j++) {
			if (passCities[i] == tempResult[j].destination) {
				break;
			}
		}
		if (j == tempResult.size()) {
			return false;
		}
	}
	return true;
}

PassengerState currentState(QDateTime pressTime)
{
	//首先计算pressTime 和 result里面的 depaturetime
	int days = result.departureTime.daysTo(pressTime);//daysTo返回 depatureTime到pressTime的天数,纯粹的天数相减
	int hours = pressTime.time().hour() - result.departureTime.time().hour() + days * 24;//距离出发时间过去了多长时间
	int spendTime = 0;//实际旅行的时间
	int waitTime = 0;//等待的时间
	int moveTime = 0;//在路上的时间
	for (unsigned int i = 0; i < result.route.size(); i++) {
		if (i == 0) {
			waitTime = result.route[i].departureTime - result.departureTime.time().hour();//准备出发等待第一班车
		}
		else {
			waitTime = result.route[i].departureTime - result.route[i - 1].arrivalTime;//在某站等下一班车
		}
		if (waitTime < 0) {
			waitTime += 24;
		}
		spendTime += waitTime;
		if (spendTime > hours) {
			PassengerState state;
			state.isMoving = false;
			state.cityNumber = result.route[i].origin;
			return state;
		}
		moveTime = result.route[i].arrivalTime - result.route[i].departureTime;//在路上的时间
		if (moveTime < 0) {
			moveTime += 24;
		}
		spendTime += moveTime;
		if (spendTime >= hours) {
			PassengerState state;
			state.isMoving = true;
			state.routeInfo = result.route[i];
			return state;
		}
	}
}