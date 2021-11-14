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


//-----------------------��������-----------------------//
const int maxn = 110, maxm = 1010;
const int N = 15;//������
const int M_fight = 10, M_train = 30, M_car = 90;//���ֽ�ͨ������Ŀ
const int M = M_fight + M_train + M_car;//��ͨ��������Ŀ


//-----------------------���������Ϣ����-----------------------//
struct City {
	string name;//��������
	int id;//���б��
	int type;//�������ͣ�1:�͡�2:�С�3:�߷��ճ���
	double risk;//���գ�0.2��0.5��0.9
}city[maxn];
map<string, int> cityNameToId;//�������ƶ�Ӧ���
int dist[maxn][maxn];//��������֮��ľ��룬1~60

int mp[maxn * 24];//���֮��ÿ�����Ӧ�ĳ��б��
int remp[maxn][24];//mp�ķ���Ӧ


//-----------------------��ͨ�����Ϣ����-----------------------//
struct Edge {
	int type;//��ͨ���ͣ�1:������2:�𳵡�3:�ɻ�  0�����еȴ�
	double risk;//����ֵ,�ý�ͨ���ߵ�λʱ�����ֵ*�ð�������еĵ�λ����ֵ*����ʱ�� (2 5 9)*(fr:0.2 0.5 0.9)*tim
	int tim;// ·��������ʱ�䣬dis = dis[fr][to], tim = max(dis / 5, dis / 10, dis / 20, 1)
	int from, to, next;//��ʼ���б�ţ�Ŀ�ĳ��б�ţ��ڽӱ�
	int startTime;//����ʱ��,0~23,ÿ��ĳ���ʱ��
}e[(maxm + maxn) * 24];
int head[maxn * 24], len;

//-----------------------·�������Ϣ����-----------------------//
struct Route {
	int num;//�����ĳ���/��ͨ������Ŀ
	int id[maxn + maxm];//�����ĳ���/��ͨ���߱��
	bool tp[maxn + maxm];//0Ϊ���У�1Ϊ��ͨ����
	int tim[maxn + maxm];//�ھ�������/��ͨ����ͣ����ʱ�䣬����Ϊͣ��ʱ�䣬��ͨ����Ϊ·��ʱ��
	Route() {//�ṹ���ʼ��
		num = 0;
		memset(id, 0, sizeof(id));
		memset(tp, 0, sizeof(tp));
		memset(tim, 0, sizeof(tim));
	}
};

//-----------------------�ÿ������Ϣ����-----------------------//
struct Passenger {
	string name;//�ÿ�����
	int frId, toId;//���б��
	int startTime;//����ʱ��
	int type;//���ԣ�1�����ٷ��ղ��ԣ���ʱ�����ƣ��������ټ���
			//2:��ʱ���ٷ��ղ��ԣ��ڹ涨��ʱ���ڷ�������
	int tim;//����2�Ĺ涨ʱ��

	Route route;//���ղ��Եõ���·��

}passenger[maxm];
int PassengerNum;//�˿�����



//-----------------------·��ͼ����---------------------------//
void Insert(int fr, int to, int tp, int Start) {//����֮���һ����·��fr->to,������tp��1��2��3������ʼʱ��ΪStart,ֵΪ0~23
	len++; e[len].from = fr; e[len].to = to; e[len].next = head[fr]; head[fr] = len;//�ڽӱ�

	e[len].type = tp;
	e[len].startTime = Start;
}

void AddEdge(int fr, int to, int tp, int Start) {//�����ڲ�������ߣ�·��ʱ����1������ֵ������ʼ���з���ֵ
	len++; e[len].from = fr; e[len].to = to; e[len].next = head[fr]; head[fr] = len;

	e[len].type = tp;

	e[len].risk = 1.0*city[mp[fr]].risk;
	e[len].tim = 1;

	e[len].startTime = Start;
}

//-----------------------������Ϣ������-----------------------//
void ReadCity() {//���������Ϣ
	//���ļ�
	ifstream ifs;
	ifs.open(CITY_FILE, ios::in);

	//�ж��ļ��Ƿ����
	if (!ifs.is_open()) {
		cout << "�����ļ������ڣ�" << endl;
		ifs.close();
		return;
	}
	for (int i = 1; i <= N; i++) {
		ifs >> city[i].name;//��������
		ifs >> city[i].type;//��������

		city[i].id = i;
		cityNameToId[city[i].name] = i;//��¼���ֶ�Ӧ�ı��
		//���з���ֵ
		if (city[i].type == 1) city[i].risk = 0.2;
		else if (city[i].type == 2) city[i].risk = 0.5;
		else if (city[i].type == 3) city[i].risk = 0.9;
	}
	//��������һ�����в�ɶ�Ӧ24Сʱ��24����
	int cnt = 0;
	for (int i = 1; i <= N; i++) {
		for (int j = 0; j < 24; j++) {
			cnt++;
			mp[cnt] = i;//��ż�¼���ڵ��Ŷ�Ӧ�ĳ��б��
			remp[i][j] = cnt;//�ض������ض�ʱ�̶�Ӧ�Ľڵ���
		}
	}
	//�����ڲ����ߣ�ÿ�����е�ǰʱ������һʱ�����ߣ�ÿ��������24���ߣ�����23ʱ����0ʱ�����ߣ�����һ����
	for (int i = 1; i <= N; i++) {
		for (int j = 0; j < 24; j++) {
			AddEdge(remp[i][j], remp[i][(j + 1) % 24], 0, j);
		}
	}
	//�������֮�������ľ���
	for (int i = 1; i <= N; i++) {
		for (int j = 1; j <= N; j++) {
			ifs >> dist[i][j];
		}
	}
	ifs.close();
}



//-----------------------��ͨ��Ϣ����-----------------------//

void ReadRoad() {//��ȡ��ͨ��Ϣ
	//���ļ�
	ifstream ifs;
	ifs.open(ROAD_FILE, ios::in);

	//�ж��ļ��Ƿ����
	if (!ifs.is_open()) {
		cout << "��·�ļ������ڣ�" << endl;
		ifs.close();
		return;
	}
	for (int i = 1; i <= M; i++) {
		string sfr, sto;//��ͨ���������ơ��յ��������
		int fr, to, tp, Start;
		ifs >> sfr >> sto;
		fr = cityNameToId[sfr];//�����ж�Ӧ���
		to = cityNameToId[sto];//�յ���ж�Ӧ���
		ifs >> tp >> Start;//���ͺͳ���ʱ��

		//���㽻ͨ�ķ���ֵ��ʱ��
		int dis = dist[fr][to], baseRisk, tim;
		if (tp == 1) baseRisk = 2, tim = max(dis / 5, 1);
		else if (tp == 2) baseRisk = 5, tim = max(dis / 10, 1);
		else if (tp == 3) baseRisk = 9, tim = max(dis / 20, 1);
		double risk = (double)1.0*baseRisk * city[mp[fr]].risk*tim;
		
		//����֮�����ߣ����ض�ʱ������
		Insert(remp[fr][Start], remp[to][(Start + tim) % 24], tp, Start);
		e[len].risk = risk;
		e[len].tim = tim;

	}
	ifs.close();
}



//-----------------------ʱ��ת������--------------------//
int Day(int nowTime) {//�ܹ���Сʱ��ת����
	return nowTime / 24 + 1;
}
int Hour(int nowTime) {//�ܹ���Сʱ��ת�����Сʱ��
	return nowTime % 24;
}

//------------------------����·���滮-------------------//
//�㷨���Գ��н��в�㽨ͼ���ܶ��Ż���Dijkstra�㷨
//����1 ���ٷ��ղ��ԣ���ʱ�����ƣ��������ټ���,endTimeΪ������
//����2 ��ʱ���ٷ��ղ��ԣ��ڹ涨��ʱ���ڷ������٣�endTime=startTime+tim
//�ƶ����Բ�������·�ߴ��浽passenger[pId].route��

struct Node {//���ȶ��д洢��Ϣ�Ľڵ�
	int num,tim;//�ڵ��ţ����ýڵ��ʱ��
	double dis;//���ýڵ�ķ���ֵ
	Node() {};
	Node(int a, int c, double b) { num = a; tim = c; dis = b; }
	bool operator < (const Node & a)const {//����С�ںţ�����ʹ�ô���ѣ����Է���������
		return dis > a.dis;
	}
};
double dis[maxn*24];//��ǰ��ѯ��㵽�����ڵ����ͷ���ֵ
bool flag[maxn*24];//��־����
int Last[maxn*24];//ת�Ƶ�һ���ڵ��·�����

//ȫ���̺����㷨�����Ż���Dijkstra�㷨
bool Dijkstra(int pId,int fr,int to,int startTime,int endTime) {
	//����ֵ��bool���ͣ����Ϊfalse��ʾ�޷��ڹ涨ʱ���ڵ������Ϊtrue��ʾ���Ե���
	//������Ϊmp[fr],�յ����Ϊto���յ���Ϊremp[to][0]~remp[to][23]
	//�˿ͱ��ΪpId����ʼʱ��startTime����ֹʱ��endTime���������1��endTimeΪ������
	priority_queue<Node> q;//���ȶ���
	//��ʼ��
	for (int i = 0; i <= N * 24 + 100; i++) {
		flag[i] = false;
		dis[i] = Inf;
		Last[i] = 0;
	}

	flag[fr] = true; dis[fr] = 0;
	q.push(Node(fr, startTime,dis[fr]));

	bool arrived = false;//�жϹ涨ʱ�����ܷ񵽴�

	int T=0,S=fr;

	while (!q.empty()) {
		Node temp = q.top(); q.pop();
		int k = temp.num; flag[k] = true;
		if (mp[temp.num] == to) {//�ҵ��ɴ��Dijkstra�㷨���ԣ���һ�ε���һ������ֵ��С
			T = temp.num;
			arrived = true;
			break;
		}
		for (int i = head[k]; i; i = e[i].next) if(temp.tim+e[i].tim<=endTime){//ֻ���ڹ涨ʱ���ڲŽ����ɳڲ���
			int j = e[i].to;
			if (!flag[j] && dis[j] > dis[k] + e[i].risk) {//û�и��¹��ҿ����ɳ�
				Last[j] = i;
				dis[j] = dis[k] + e[i].risk;//�ɳڲ���
				q.push(Node(j, temp.tim+e[i].tim,dis[j]));
			}
		}
	}
	if (!arrived) {//����2�����ڹ涨ʱ�����޷�����
		return false;
	}
	//��Dijkstra�ҵ���·��ѹ����������֮��ı�����ת��Ϊһ�����в������ڳ���ͣ��ʱ��
	Route r;
	r.num++;
	r.id[r.num] = T;
	r.tp[r.num] = 0;
	r.tim[r.num] = 0;
	int now = T;
	while (Last[now]) {
		if (e[Last[now]].type == 0) {//����֮��ıߣ�ѹ���洢
			r.num++;
			r.id[r.num] = e[Last[now]].to;
			r.tp[r.num] = 0;
			while (Last[now] && e[Last[now]].type==0) {
				r.tim[r.num]++;
				now = e[Last[now]].from;
			}
		}
		else {//����֮��ıߣ�ֱ�Ӵ洢
			r.num++;
			r.id[r.num] = Last[now];
			r.tp[r.num] = 1;
			r.tim[r.num] = e[Last[now]].tim;
			now = e[Last[now]].from;
		}
	}
	if (r.tp[r.num] == 1) {//�ж�����Ƿ���Ҫ�ȴ�
		r.num++;
		r.id[r.num] = S;
		r.tp[r.num] = 0;
		r.tim[r.num] = 0;
	}
	//���������洢��r�Ƿ������ģ��洢���ÿ���Ϣ��Ҫ��r��·����ת
	for (int i = 1; i <= r.num; i++) {
		passenger[pId].route.id[i] = r.id[r.num - i + 1];
		passenger[pId].route.tp[i] = r.tp[r.num - i + 1];
		passenger[pId].route.tim[i] = r.tim[r.num - i + 1];
	}
	passenger[pId].route.num = r.num;
	return true;
}

//------------------------�����ÿ���Ϣ-------------------//
void AddPassenger(int nowTime) {//nowTimeΪ��ǰʱ��
	system("cls");
	string TIME = "ʱ��: ��" + to_string(Day(nowTime)) + "��";
	if (Hour(nowTime) < 10) TIME = TIME + "0";
	TIME = TIME +to_string(Hour(nowTime)) + ":00";
	cout << "\t-------------------------------\n";
	cout << "\t|                              |\n";
	cout << "\t|         1.�����ÿ���Ϣ       |\n";
	cout << "\t|                              |\n";
	cout << "\t|             " << TIME   << " |\n";
	cout << "\t-------------------------------\n";
	string name, fr, to;
	int tp,tim=Inf,frId,toId;
	//�����ÿ������Ϣ��������ʼ���ص㡢Ŀ�ĵص㡢���ԡ��涨ʱ��
	cout << "�������ÿ͵�����: "; cin >> name;
	
	cout << "�������ÿ͵�ʼ������: "; cin >> fr; frId = cityNameToId[fr];
	cout << "�������ÿ͵��յ����: "; cin >> to; toId = cityNameToId[to];

	cout << "���������в���:" << endl;
	cout << "1 ���ٷ��ղ��ԣ���ʱ�����ƣ��������ټ���" << endl;
	cout << "2 ��ʱ���ٷ��ղ��ԣ��ڹ涨��ʱ���ڷ�������" << endl;
	cin >> tp;

	if (tp == 2) {
		cout << "������涨ʱ��(Сʱ): "; cin >> tim;
	}
	cout << endl;

	//�洢��Ϣ
	PassengerNum++;
	passenger[PassengerNum].name = name;
	passenger[PassengerNum].frId = frId;
	passenger[PassengerNum].toId = toId;

	passenger[PassengerNum].startTime = nowTime;
	
	passenger[PassengerNum].type = tp;
	passenger[PassengerNum].tim = tim;

	bool tag=Dijkstra(PassengerNum,remp[frId][Hour(nowTime)], toId, nowTime,nowTime+tim);

	//д�ļ�
	ofstream ofs;
	ofs.open(LOG_FILE, ios::out | ios::app);

	if (tag == false) {//����2�����ڹ涨ʱ�����޷�����
		PassengerNum--;//ɾ���ÿ���Ϣ
		cout << "�����ÿ�ʧ��,�����������ÿ������Ϣ�͹涨ʱ��!" << endl<<endl;
		ofs.close();
		system("pause");
		return;
	}

	//���ÿ���Ϣ��·��д����־

	ofs << "-----------�����ÿ���Ϣ-------------" << endl;
	ofs << "�ÿͱ��: " << PassengerNum << endl;
	ofs << "����: " << name << endl;
	ofs << "������: " << fr << "   " << "Ŀ�ĵ�: " << to << endl;
	ofs << "����ʱ��: ��" << Day(nowTime) << "��" << Hour(nowTime) << ":00" <<endl;
	ofs << "ѡ�����Ϊ����" << tp;
	if (tp == 1) ofs << ": ���ٷ��ղ���"<<endl<<endl;
	else ofs << ": ��ʱ���ٷ��ղ���,ʱ��Ϊ" << tim << "Сʱ" << endl<<endl;
	
	ofs << "����·������:" << endl;
	Route r = passenger[PassengerNum].route;
	for (int i = 1; i <= r.num; i++) {
		if (i == 1) {//���
			ofs << "�ÿ�" << name << "����ʼ���� " << fr << " ͣ��" << r.tim[i] << "Сʱ,Ȼ�����;" << endl;
		}
		else if (i == passenger[PassengerNum].route.num) {//�յ�
			ofs << "�ÿ�" << name << "������� " << to << " ��"<<endl;
		}
		else {//��;
			if (r.tp[i] == 0) {//����
				ofs << "�ÿ�" << name << "������� " << city[mp[r.id[i]]].name << " ,��ͣ��" << r.tim[i] << "Сʱ;" << endl;
			}
			else {//·��
				string way;
				if (e[r.id[i]].type == 1) way="����";
				else if (e[r.id[i]].type == 2) way= "��";
				else way= "�ɻ�";
				ofs << "�ÿ�" << name << "��" << way << "�ӳ��� " << city[mp[e[r.id[i]].from]].name << " ������ " << city[mp[e[r.id[i]].to]].name;
				ofs << " ,·����Ҫ" << e[r.id[i]].tim << "Сʱ;" << endl;
			}
		}
	}

	ofs << endl;
	ofs.close();

	//���ÿ���Ϣ��·������������д���

	cout << "�ÿ���Ϣ��ӳɹ�!" << endl ;
	cout << "�ÿͱ��: " << PassengerNum << endl;
	cout << "����: " << name << endl;
	cout << "������: " << fr << "   " << "Ŀ�ĵ�: " << to << endl;
	cout << "����ʱ��: ��" << Day(nowTime) << "��" << Hour(nowTime) << ":00" <<endl;
	cout << "ѡ�����Ϊ����" << tp;
	if (tp == 1) cout << ": ���ٷ��ղ���" << endl<<endl;
	else cout << ": ��ʱ���ٷ��ղ���,ʱ��Ϊ" << tim << "Сʱ" << endl<<endl;

	cout << "����·������:" << endl;
	r = passenger[PassengerNum].route;
	for (int i = 1; i <= r.num; i++) {
		if (i == 1) {//���
			cout << "�ÿ�" << name << "����ʼ���� " << fr << " ͣ��" << r.tim[i] << "Сʱ,Ȼ�����;" << endl;
		}
		else if (i == passenger[PassengerNum].route.num) {//�յ�
			cout << "�ÿ�" << name << "������� " << to << " ��" << endl;
		}
		else {//��;
			if (r.tp[i] == 0) {//����
				cout << "�ÿ�" << name << "������� " << city[mp[r.id[i]]].name << " ,��ͣ��" << r.tim[i] << "Сʱ;" << endl;
			}
			else {//·��
				string way;
				if (e[r.id[i]].type == 1) way = "����";
				else if (e[r.id[i]].type == 2) way = "��";
				else way = "�ɻ�";
				cout << "�ÿ�" << name << "��" << way << "�ӳ��� " << city[mp[e[r.id[i]].from]].name << " ������ " << city[mp[e[r.id[i]].to]].name;
				cout << " ,·����Ҫ" << e[r.id[i]].tim << "Сʱ;" << endl;
			}
		}
	}

	cout << endl << endl;

	system("pause");

}



//------------------------��ѯ�ÿ�״̬-------------------//
void QueryPassengerStatus(int nowTime) {//nowTimeΪ��ǰʱ��
	system("cls");
	string TIME = "ʱ��: ��" + to_string(Day(nowTime)) + "��";
	if (Hour(nowTime) < 10) TIME = TIME + "0";
	TIME = TIME + to_string(Hour(nowTime)) + ":00";
	cout << "\t-------------------------------\n";
	cout << "\t|                              |\n";
	cout << "\t|         2.��ѯ�ÿ�״̬       |\n";
	cout << "\t|                              |\n";
	cout << "\t|             " << TIME   << " |\n";
	cout << "\t-------------------------------\n";
	string name;
	//������Ҫ��ѯ�ÿ͵�����
	cout << "��������Ҫ��ѯ���ÿ͵�����:" << endl; cin >> name;
	int id=0;
	for (int i = 1; i <= PassengerNum; i++) {
		if (passenger[i].name == name) {
			id = i;
			break;
		}
	}
	if (id == 0) {//û�и��ÿ�
		cout << "�������������²���!" << endl << endl;
		system("pause");
		return;
	}
	//��ѯ�ÿ�״̬
	Route r = passenger[id].route;
	int tim = nowTime-passenger[id].startTime;
	int tot = 0;
	for (int i = 1; i <= r.num; i++) {
		tot += r.tim[i];
		if (i == r.num) {//�����յ�
			cout << "�ÿ�" << name << "�Ѿ������յ���� " << city[mp[r.id[i]]].name << " ��" << endl;
			break;
		}
		if (tot >= tim) {//û�е����յ�
			if (r.tp[i] == 0) {//ͣ���ڳ���
				cout << "�ÿ�" << name << "��ǰ�ڳ��� " << city[mp[r.id[i]]].name << " ,����ͣ��" << tot - tim << "Сʱ��" << endl;
			}
			else {//��·�ϣ��������𳵡��ɻ���
				cout << "�ÿ�" << name << "��ǰ�ڴӳ��� " << city[mp[e[r.id[i]].from]].name << " ������ " << city[mp[e[r.id[i]].to]].name << " ��";
				if (e[r.id[i]].type == 1) cout << "������," ;
				else if (e[r.id[i]].type == 2) cout << "����," ;
				else cout << "�ɻ���," ;
				cout << "����" << tot - tim << "Сʱ���" << endl;
			}
			break;
		}
	}
	cout << "\n��ѯ������" << endl<<endl;
	system("pause");
}



//------------------------�����Ϣ����־-------------------//

void LogAll(int nowTime) {//ÿ��1Сʱ�Զ��������ÿ͵�״̬д�뵽��־�ļ�

	ofstream ofs;
	ofs.open(LOG_FILE, ios::out | ios::app);

	ofs << endl << "��ǰʱ��: ��" << Day(nowTime) << "��" << Hour(nowTime) << ":00"<<endl;
	int id = 0;
	for (int d = 1; d <= PassengerNum; d++) {//ѭ����������ÿ�״̬
		id = d;
		ofs << "��" << id << "���ÿ�:";
		string name = passenger[id].name;
		Route r = passenger[id].route;
		int tim = nowTime - passenger[id].startTime;
		int tot = 0;
		for (int i = 1; i <= r.num; i++) {
			tot += r.tim[i];
			if (i == r.num) {
				ofs << "�ÿ�" << name << "�Ѿ������յ���� " << city[mp[r.id[i]]].name << " ��" << endl;
				break;
			}
			if (tot >= tim) {
				if (r.tp[i] == 0) {//����
					ofs << "�ÿ�" << name << "��ǰ�ڳ��� " << city[mp[r.id[i]]].name << " ,����ͣ��" << tot - tim << "Сʱ��" << endl;
				}
				else {//·��
					ofs << "�ÿ�" << name << "��ǰ�ڴӳ��� " << city[mp[e[r.id[i]].from]].name << " ������ " << city[mp[e[r.id[i]].to]].name << " ��";
					if (e[r.id[i]].type == 1) ofs << "������,";
					else if (e[r.id[i]].type == 2) ofs << "����,";
					else ofs << "�ɻ���,";
					ofs << "����" << tot - tim << "Сʱ���" << endl;
				}
				break;
			}
		}
	}

	ofs << endl;
	ofs.close();
}