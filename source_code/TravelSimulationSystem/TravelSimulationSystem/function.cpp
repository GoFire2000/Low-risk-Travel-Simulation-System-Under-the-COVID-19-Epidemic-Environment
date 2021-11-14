#include <map>
#include <ctime>
#include <cstdio>
#include <string>
#include <queue>
#include <cstring>
#include <fstream>
#include <iostream>
#include <algorithm>
#include "function.h"
using namespace std;


//-----------------------常量定义-----------------------//
const int maxn = 110, maxm = 1010;
const int N = 15;//城市数
const int M_fight = 10, M_train = 30, M_car = 90;//各种交通工具数目
const int M = M_fight + M_train + M_car;//交通工具总数目


//-----------------------城市相关信息定义-----------------------//
struct City {
	string name;//城市名称
	int id;//城市编号
	int type;//城市类型，1:低、2:中、3:高风险城市
	double risk;//风险，0.2、0.5、0.9
}city[maxn];
map<string, int> cityNameToId;//城市名称对应编号
int dist[maxn][maxn];//两两城市之间的距离，1~60

int mp[maxn * 24];//拆点之后每个点对应的城市编号
int remp[maxn][24];//mp的反对应


//-----------------------交通相关信息定义-----------------------//
struct Edge {
	int type;//交通类型：1:汽车、2:火车、3:飞机  0：城市等待
	double risk;//风险值,该交通工具单位时间风险值*该班次起点城市的单位风险值*乘坐时间 (2 5 9)*(fr:0.2 0.5 0.9)*tim
	int tim;// 路上所花费时间，dis = dis[fr][to], tim = max(dis / 5, dis / 10, dis / 20, 1)
	int from, to, next;//起始城市编号，目的城市编号，邻接表
	int startTime;//出发时间,0~23,每天的出发时间
}e[(maxm + maxn) * 24];
int head[maxn * 24], len;

//-----------------------路线相关信息定义-----------------------//
struct Route {
	int num;//经过的城市/交通工具数目
	int id[maxn + maxm];//经过的城市/交通工具编号
	bool tp[maxn + maxm];//0为城市，1为交通工具
	int tim[maxn + maxm];//在经过城市/交通工具停留的时间，城市为停留时间，交通工具为路程时间
	Route() {//结构体初始化
		num = 0;
		memset(id, 0, sizeof(id));
		memset(tp, 0, sizeof(tp));
		memset(tim, 0, sizeof(tim));
	}
};

//-----------------------旅客相关信息定义-----------------------//
struct Passenger {
	string name;//旅客姓名
	int frId, toId;//城市编号
	int startTime;//出发时间
	int type;//策略，1：最少风险策略：无时间限制，风险最少即可
			//2:限时最少风险策略：在规定的时间内风险最少
	int tim;//策略2的规定时长

	Route route;//按照策略得到的路线

}passenger[maxm];
int PassengerNum;//乘客数量



//-----------------------路线图建立---------------------------//
void Insert(int fr, int to, int tp, int Start) {//城市之间的一条线路，fr->to,类型是tp（1、2、3），起始时间为Start,值为0~23
	len++; e[len].from = fr; e[len].to = to; e[len].next = head[fr]; head[fr] = len;//邻接表

	e[len].type = tp;
	e[len].startTime = Start;
}

void AddEdge(int fr, int to, int tp, int Start) {//城市内部拆点连线，路上时间是1，风险值就是起始城市风险值
	len++; e[len].from = fr; e[len].to = to; e[len].next = head[fr]; head[fr] = len;

	e[len].type = tp;

	e[len].risk = 1.0*city[mp[fr]].risk;
	e[len].tim = 1;

	e[len].startTime = Start;
}

//-----------------------城市信息及读入-----------------------//
void ReadCity() {//读入城市信息
	//读文件
	ifstream ifs;
	ifs.open(CITY_FILE, ios::in);

	//判断文件是否存在
	if (!ifs.is_open()) {
		cout << "城市文件不存在！" << endl;
		ifs.close();
		return;
	}
	for (int i = 1; i <= N; i++) {
		ifs >> city[i].name;//城市名称
		ifs >> city[i].type;//城市类型

		city[i].id = i;
		cityNameToId[city[i].name] = i;//记录名字对应的编号
		//城市风险值
		if (city[i].type == 1) city[i].risk = 0.2;
		else if (city[i].type == 2) city[i].risk = 0.5;
		else if (city[i].type == 3) city[i].risk = 0.9;
	}
	//拆点操作，一个城市拆成对应24小时的24个点
	int cnt = 0;
	for (int i = 1; i <= N; i++) {
		for (int j = 0; j < 24; j++) {
			cnt++;
			mp[cnt] = i;//编号记录，节点标号对应的城市编号
			remp[i][j] = cnt;//特定城市特定时刻对应的节点编号
		}
	}
	//城市内部连线，每个城市当前时刻向下一时刻连边，每个城市连24条边，其中23时刻向0时刻连边，构成一个环
	for (int i = 1; i <= N; i++) {
		for (int j = 0; j < 24; j++) {
			AddEdge(remp[i][j], remp[i][(j + 1) % 24], 0, j);
		}
	}
	//读入城市之间两两的距离
	for (int i = 1; i <= N; i++) {
		for (int j = 1; j <= N; j++) {
			ifs >> dist[i][j];
		}
	}
	ifs.close();
}



//-----------------------交通信息读入-----------------------//

void ReadRoad() {//读取交通信息
	//读文件
	ifstream ifs;
	ifs.open(ROAD_FILE, ios::in);

	//判断文件是否存在
	if (!ifs.is_open()) {
		cout << "道路文件不存在！" << endl;
		ifs.close();
		return;
	}
	for (int i = 1; i <= M; i++) {
		string sfr, sto;//交通起点城市名称、终点城市名称
		int fr, to, tp, Start;
		ifs >> sfr >> sto;
		fr = cityNameToId[sfr];//起点城市对应编号
		to = cityNameToId[sto];//终点城市对应编号
		ifs >> tp >> Start;//类型和出发时间

		//计算交通的风险值和时间
		int dis = dist[fr][to], baseRisk, tim;
		if (tp == 1) baseRisk = 2, tim = max(dis / 5, 1);
		else if (tp == 2) baseRisk = 5, tim = max(dis / 10, 1);
		else if (tp == 3) baseRisk = 9, tim = max(dis / 20, 1);
		double risk = (double)1.0*baseRisk * city[mp[fr]].risk*tim;
		
		//城市之间连边，按特定时刻连边
		Insert(remp[fr][Start], remp[to][(Start + tim) % 24], tp, Start);
		e[len].risk = risk;
		e[len].tim = tim;

	}
	ifs.close();
}



//-----------------------时间转换函数--------------------//
int Day(int nowTime) {//总共的小时数转天数
	return nowTime / 24 + 1;
}
int Hour(int nowTime) {//总共的小时数转当天的小时数
	return nowTime % 24;
}

//------------------------旅游路径规划-------------------//
//算法：对城市进行拆点建图，跑堆优化的Dijkstra算法
//策略1 最少风险策略：无时间限制，风险最少即可,endTime为正无穷
//策略2 限时最少风险策略：在规定的时间内风险最少，endTime=startTime+tim
//制定策略并将旅行路线储存到passenger[pId].route中

struct Node {//优先队列存储信息的节点
	int num,tim;//节点编号，到该节点的时间
	double dis;//到该节点的风险值
	Node() {};
	Node(int a, int c, double b) { num = a; tim = c; dis = b; }
	bool operator < (const Node & a)const {//重载小于号，由于使用大根堆，所以反方向重载
		return dis > a.dis;
	}
};
double dis[maxn*24];//当前查询起点到其他节点的最低风险值
bool flag[maxn*24];//标志数组
int Last[maxn*24];//转移到一个节点的路径编号

//全工程核心算法：堆优化的Dijkstra算法
bool Dijkstra(int pId,int fr,int to,int startTime,int endTime) {
	//返回值是bool类型，如果为false表示无法在规定时间内到达，返回为true表示可以到达
	//起点城市为mp[fr],终点城市为to，终点编号为remp[to][0]~remp[to][23]
	//乘客编号为pId，起始时间startTime，终止时间endTime，如果策略1，endTime为正无穷
	priority_queue<Node> q;//优先队列
	//初始化
	for (int i = 0; i <= N * 24 + 100; i++) {
		flag[i] = false;
		dis[i] = Inf;
		Last[i] = 0;
	}

	flag[fr] = true; dis[fr] = 0;
	q.push(Node(fr, startTime,dis[fr]));

	bool arrived = false;//判断规定时限内能否到达

	int T=0,S=fr;

	while (!q.empty()) {
		Node temp = q.top(); q.pop();
		int k = temp.num; flag[k] = true;
		if (mp[temp.num] == to) {//找到可达，由Dijkstra算法特性，第一次到达一定风险值最小
			T = temp.num;
			arrived = true;
			break;
		}
		for (int i = head[k]; i; i = e[i].next) if(temp.tim+e[i].tim<=endTime){//只有在规定时限内才进行松弛操作
			int j = e[i].to;
			if (!flag[j] && dis[j] > dis[k] + e[i].risk) {//没有更新过且可以松弛
				Last[j] = i;
				dis[j] = dis[k] + e[i].risk;//松弛操作
				q.push(Node(j, temp.tim+e[i].tim,dis[j]));
			}
		}
	}
	if (!arrived) {//策略2，且在规定时间内无法到达
		return false;
	}
	//将Dijkstra找到的路径压缩，将城市之间的边重新转化为一个城市并计算在城市停留时间
	Route r;
	r.num++;
	r.id[r.num] = T;
	r.tp[r.num] = 0;
	r.tim[r.num] = 0;
	int now = T;
	while (Last[now]) {
		if (e[Last[now]].type == 0) {//城市之间的边，压缩存储
			r.num++;
			r.id[r.num] = e[Last[now]].to;
			r.tp[r.num] = 0;
			while (Last[now] && e[Last[now]].type==0) {
				r.tim[r.num]++;
				now = e[Last[now]].from;
			}
		}
		else {//城市之间的边，直接存储
			r.num++;
			r.id[r.num] = Last[now];
			r.tp[r.num] = 1;
			r.tim[r.num] = e[Last[now]].tim;
			now = e[Last[now]].from;
		}
	}
	if (r.tp[r.num] == 1) {//判断起点是否需要等待
		r.num++;
		r.id[r.num] = S;
		r.tp[r.num] = 0;
		r.tim[r.num] = 0;
	}
	//由于上述存储到r是反向来的，存储到旅客信息需要将r的路径翻转
	for (int i = 1; i <= r.num; i++) {
		passenger[pId].route.id[i] = r.id[r.num - i + 1];
		passenger[pId].route.tp[i] = r.tp[r.num - i + 1];
		passenger[pId].route.tim[i] = r.tim[r.num - i + 1];
	}
	passenger[pId].route.num = r.num;
	return true;
}

//------------------------新增旅客信息-------------------//
void AddPassenger(int nowTime) {//nowTime为当前时间
	system("cls");
	string TIME = "时间: 第" + to_string(Day(nowTime)) + "天";
	if (Hour(nowTime) < 10) TIME = TIME + "0";
	TIME = TIME +to_string(Hour(nowTime)) + ":00";
	cout << "\t-------------------------------\n";
	cout << "\t|                              |\n";
	cout << "\t|         1.新增旅客信息       |\n";
	cout << "\t|                              |\n";
	cout << "\t|             " << TIME   << " |\n";
	cout << "\t-------------------------------\n";
	string name, fr, to;
	int tp,tim=Inf,frId,toId;
	//输入旅客相关信息：姓名、始发地点、目的地点、策略、规定时限
	cout << "请输入旅客的姓名: "; cin >> name;
	
	cout << "请输入旅客的始发城市: "; cin >> fr; frId = cityNameToId[fr];
	cout << "请输入旅客的终点城市: "; cin >> to; toId = cityNameToId[to];

	cout << "请输入旅行策略:" << endl;
	cout << "1 最少风险策略：无时间限制，风险最少即可" << endl;
	cout << "2 限时最少风险策略：在规定的时间内风险最少" << endl;
	cin >> tp;

	if (tp == 2) {
		cout << "请输入规定时限(小时): "; cin >> tim;
	}
	cout << endl;

	//存储信息
	PassengerNum++;
	passenger[PassengerNum].name = name;
	passenger[PassengerNum].frId = frId;
	passenger[PassengerNum].toId = toId;

	passenger[PassengerNum].startTime = nowTime;
	
	passenger[PassengerNum].type = tp;
	passenger[PassengerNum].tim = tim;

	bool tag=Dijkstra(PassengerNum,remp[frId][Hour(nowTime)], toId, nowTime,nowTime+tim);

	//写文件
	ofstream ofs;
	ofs.open(LOG_FILE, ios::out | ios::app);

	if (tag == false) {//策略2，且在规定时间内无法到达
		PassengerNum--;//删除旅客信息
		cout << "新增旅客失败,请重新输入旅客相关信息和规定时限!" << endl<<endl;
		ofs.close();
		system("pause");
		return;
	}

	//将旅客信息的路径写入日志

	ofs << "-----------新增旅客信息-------------" << endl;
	ofs << "旅客编号: " << PassengerNum << endl;
	ofs << "姓名: " << name << endl;
	ofs << "出发地: " << fr << "   " << "目的地: " << to << endl;
	ofs << "出发时间: 第" << Day(nowTime) << "天" << Hour(nowTime) << ":00" <<endl;
	ofs << "选择策略为策略" << tp;
	if (tp == 1) ofs << ": 最少风险策略"<<endl<<endl;
	else ofs << ": 限时最少风险策略,时限为" << tim << "小时" << endl<<endl;
	
	ofs << "旅行路线如下:" << endl;
	Route r = passenger[PassengerNum].route;
	for (int i = 1; i <= r.num; i++) {
		if (i == 1) {//起点
			ofs << "旅客" << name << "在起始城市 " << fr << " 停留" << r.tim[i] << "小时,然后出发;" << endl;
		}
		else if (i == passenger[PassengerNum].route.num) {//终点
			ofs << "旅客" << name << "到达城市 " << to << " 。"<<endl;
		}
		else {//中途
			if (r.tp[i] == 0) {//城市
				ofs << "旅客" << name << "到达城市 " << city[mp[r.id[i]]].name << " ,并停留" << r.tim[i] << "小时;" << endl;
			}
			else {//路上
				string way;
				if (e[r.id[i]].type == 1) way="汽车";
				else if (e[r.id[i]].type == 2) way= "火车";
				else way= "飞机";
				ofs << "旅客" << name << "坐" << way << "从城市 " << city[mp[e[r.id[i]].from]].name << " 到城市 " << city[mp[e[r.id[i]].to]].name;
				ofs << " ,路上需要" << e[r.id[i]].tim << "小时;" << endl;
			}
		}
	}

	ofs << endl;
	ofs.close();

	//将旅客信息和路径输出到命令行窗口

	cout << "旅客信息添加成功!" << endl ;
	cout << "旅客编号: " << PassengerNum << endl;
	cout << "姓名: " << name << endl;
	cout << "出发地: " << fr << "   " << "目的地: " << to << endl;
	cout << "出发时间: 第" << Day(nowTime) << "天" << Hour(nowTime) << ":00" <<endl;
	cout << "选择策略为策略" << tp;
	if (tp == 1) cout << ": 最少风险策略" << endl<<endl;
	else cout << ": 限时最少风险策略,时限为" << tim << "小时" << endl<<endl;

	cout << "旅行路线如下:" << endl;
	r = passenger[PassengerNum].route;
	for (int i = 1; i <= r.num; i++) {
		if (i == 1) {//起点
			cout << "旅客" << name << "在起始城市 " << fr << " 停留" << r.tim[i] << "小时,然后出发;" << endl;
		}
		else if (i == passenger[PassengerNum].route.num) {//终点
			cout << "旅客" << name << "到达城市 " << to << " 。" << endl;
		}
		else {//中途
			if (r.tp[i] == 0) {//城市
				cout << "旅客" << name << "到达城市 " << city[mp[r.id[i]]].name << " ,并停留" << r.tim[i] << "小时;" << endl;
			}
			else {//路上
				string way;
				if (e[r.id[i]].type == 1) way = "汽车";
				else if (e[r.id[i]].type == 2) way = "火车";
				else way = "飞机";
				cout << "旅客" << name << "坐" << way << "从城市 " << city[mp[e[r.id[i]].from]].name << " 到城市 " << city[mp[e[r.id[i]].to]].name;
				cout << " ,路上需要" << e[r.id[i]].tim << "小时;" << endl;
			}
		}
	}

	cout << endl << endl;

	system("pause");

}



//------------------------查询旅客状态-------------------//
void QueryPassengerStatus(int nowTime) {//nowTime为当前时间
	system("cls");
	string TIME = "时间: 第" + to_string(Day(nowTime)) + "天";
	if (Hour(nowTime) < 10) TIME = TIME + "0";
	TIME = TIME + to_string(Hour(nowTime)) + ":00";
	cout << "\t-------------------------------\n";
	cout << "\t|                              |\n";
	cout << "\t|         2.查询旅客状态       |\n";
	cout << "\t|                              |\n";
	cout << "\t|             " << TIME   << " |\n";
	cout << "\t-------------------------------\n";
	string name;
	//输入所要查询旅客的姓名
	cout << "请输入所要查询的旅客的姓名:" << endl; cin >> name;
	int id=0;
	for (int i = 1; i <= PassengerNum; i++) {
		if (passenger[i].name == name) {
			id = i;
			break;
		}
	}
	if (id == 0) {//没有该旅客
		cout << "输入有误，请重新操作!" << endl << endl;
		system("pause");
		return;
	}
	//查询旅客状态
	Route r = passenger[id].route;
	int tim = nowTime-passenger[id].startTime;
	int tot = 0;
	for (int i = 1; i <= r.num; i++) {
		tot += r.tim[i];
		if (i == r.num) {//到达终点
			cout << "旅客" << name << "已经到达终点城市 " << city[mp[r.id[i]]].name << " 。" << endl;
			break;
		}
		if (tot >= tim) {//没有到达终点
			if (r.tp[i] == 0) {//停留在城市
				cout << "旅客" << name << "当前在城市 " << city[mp[r.id[i]]].name << " ,还需停留" << tot - tim << "小时。" << endl;
			}
			else {//在路上（汽车、火车、飞机）
				cout << "旅客" << name << "当前在从城市 " << city[mp[e[r.id[i]].from]].name << " 到城市 " << city[mp[e[r.id[i]].to]].name << " 的";
				if (e[r.id[i]].type == 1) cout << "汽车上," ;
				else if (e[r.id[i]].type == 2) cout << "火车上," ;
				else cout << "飞机上," ;
				cout << "还需" << tot - tim << "小时到达。" << endl;
			}
			break;
		}
	}
	cout << "\n查询结束！" << endl<<endl;
	system("pause");
}



//------------------------输出信息到日志-------------------//

void LogAll(int nowTime) {//每隔1小时自动将所有旅客的状态写入到日志文件

	ofstream ofs;
	ofs.open(LOG_FILE, ios::out | ios::app);

	ofs << endl << "当前时间: 第" << Day(nowTime) << "天" << Hour(nowTime) << ":00"<<endl;
	int id = 0;
	for (int d = 1; d <= PassengerNum; d++) {//循环输出所有旅客状态
		id = d;
		ofs << "第" << id << "号旅客:";
		string name = passenger[id].name;
		Route r = passenger[id].route;
		int tim = nowTime - passenger[id].startTime;
		int tot = 0;
		for (int i = 1; i <= r.num; i++) {
			tot += r.tim[i];
			if (i == r.num) {
				ofs << "旅客" << name << "已经到达终点城市 " << city[mp[r.id[i]]].name << " 。" << endl;
				break;
			}
			if (tot >= tim) {
				if (r.tp[i] == 0) {//城市
					ofs << "旅客" << name << "当前在城市 " << city[mp[r.id[i]]].name << " ,还需停留" << tot - tim << "小时。" << endl;
				}
				else {//路上
					ofs << "旅客" << name << "当前在从城市 " << city[mp[e[r.id[i]].from]].name << " 到城市 " << city[mp[e[r.id[i]].to]].name << " 的";
					if (e[r.id[i]].type == 1) ofs << "汽车上,";
					else if (e[r.id[i]].type == 2) ofs << "火车上,";
					else ofs << "飞机上,";
					ofs << "还需" << tot - tim << "小时到达。" << endl;
				}
				break;
			}
		}
	}

	ofs << endl;
	ofs.close();
}