#pragma once
#include <fstream>
#include <string>
using namespace std;

extern fstream logFile;

bool openFile();//打开日志文件
void updateFile(string content);
/*
	功能:更新日志文件
	content 需要追加的内容
*/