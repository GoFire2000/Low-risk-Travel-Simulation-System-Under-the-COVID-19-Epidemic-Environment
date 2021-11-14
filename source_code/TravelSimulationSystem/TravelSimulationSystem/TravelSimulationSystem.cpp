#include <map>
#include <ctime>
#include <cstdio>
#include <string>
#include <conio.h>
#include <iostream>
#include <algorithm>
#include <windows.h>
#include "function.h"
using namespace std;

char typeSelect;//用户操作类型选择

int main() {
	ReadCity();//读入城市信息
	ReadRoad();//读入交通信息

	int secondNum = 0,hourNum=0;//时间中的程序运行时间（秒）和模拟推进时间（小时），2s表示1h
	string TIME;

	TIME = "时间:" + to_string(hourNum / 24 + 1) + "天" + to_string(hourNum % 24) + "时";
	cout << "\n\n\n\n\n\n\n\n";
	cout << "\t\t\t\t-------------------------------------------------\n";
	cout << "\t\t\t\t|                                                |\n";
	cout << "\t\t\t\t|       COVID-19疫情环境下低风险旅行模拟系统     |\n";
	cout << "\t\t\t\t|                                                |\n";
	cout << "\t\t\t\t|                             Designer: XuZikang |\n";
	cout << "\t\t\t\t|                                 Time: 2020.7.6 |\n";
	cout << "\t\t\t\t-------------------------------------------------\n";
	Sleep(3000);


	time_t readTime = 0;
	time_t startTime = time(NULL);
	time_t nowTime;

	while (true) {
		system("cls");
		string TIME = "时间: 第" + to_string(hourNum / 24 + 1) + "天";
		if (hourNum % 24 < 10) TIME = TIME + "0";
		TIME = TIME + to_string(hourNum % 24) + ":00";
		
		cout << "欢迎来到COVID-19疫情环境下低风险旅行模拟系统" << endl;
		cout << endl << "请选择操作模式" << endl;
		cout << "\t\t-------------------------------\n";
		cout << "\t\t|                              |\n";
		cout << "\t\t|         1.新增旅客信息       |\n";
		cout << "\t\t|                              |\n";
		cout << "\t\t|         2.查询旅客状态       |\n";
		cout << "\t\t|                              |\n";
		cout << "\t\t|         0.退出               |\n";
		cout << "\t\t|                              |\n";
		cout << "\t\t|             "   <<TIME<<   " |\n";
		cout << "\t\t-------------------------------\n";
		cout << "请输入您的选择: ";

		bool tag = false;
		while (true) {
			if (kbhit()) {//用户有输入，接收输入
				typeSelect = getch();
				cout << typeSelect << endl; Sleep(500);
				break;
			}
			if (time(NULL) - startTime - readTime == secondNum) {//每隔2s会进入此处语句，语句内调用LogAll函数，执行写日志操作
				LogAll(hourNum);
				secondNum += 2;
				hourNum++;
				tag = true;
				break;
			}
		}
		if (tag) continue;

		time_t beTime = time(NULL);//用户操作的开始时间

		switch (typeSelect) {
		case '1'://新增旅客信息
			AddPassenger(hourNum);
			break;
		case '2'://查询旅客状态
			QueryPassengerStatus(hourNum);
			break;
		case '0'://退出系统
			cout << "欢迎下一次使用!" << endl<<endl;
			system("pause");
			return 0;
			break;
		default://错误输入
			cout << "输入有误，请重新选择!" << endl;
			system("pause");
			break;
		}

		time_t enTime = time(NULL);//用户操作的结束时间
		readTime += enTime - beTime;//更新用户操作时间
	}
	return 0;
}
