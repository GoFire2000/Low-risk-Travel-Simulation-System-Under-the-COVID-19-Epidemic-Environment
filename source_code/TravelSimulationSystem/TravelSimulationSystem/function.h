#pragma once
#include "definitions.h"

//城市信息读入
void ReadCity();

//交通信息读入
void ReadRoad();

//新增乘客信息
void AddPassenger(int);

//查询旅客状态
void QueryPassengerStatus(int);

//每小时日志写入
void LogAll(int);

