#include<iostream>
#include<unordered_map>
#include<unordered_set>
#include<set>
#include<map>
#include<queue>
#include<vector>
#include<algorithm>
#include<fstream>
#include<ctime>
using namespace std;

//#define DEBUG

ifstream fin("/data/test_data.txt", ios::in);
FILE *fout = fopen("/projects/student/result.txt", "w");


struct Edge {
	int u;
	int v;
	Edge(int _u, int _v) :u(_u), v(_v) {};
};

//����ͼ���ڽӱ�ṹ
struct Node
{
	int v;
	Node* next;
	Node(int _v,Node* _next) :v(_v), next(_next) {};
};

struct Hnode
{
	Node *first;		//�����ڽӱ�
};

struct TmpAns
{
	int a, b, c;
	TmpAns(int _a, int _b, int _c) :a(_a), b(_b), c(_c) {};
};
//����������������ݽṹ
struct Data
{
	bool isUsed;		//�Ƿ񱻷���
	bool Out;			//�����Ƿ�����Ҫ��
	bool In;			//��������������Ƿ�����Ҫ��
	int indg;			//�������
	Data() :isUsed(false),Out(false),In(false),indg(0) {};
};

unordered_map<int, Hnode> graph;					//������ڱ�
unordered_map<int, Data> state;						//ͼ�н���״̬
unordered_map<int, unordered_set<int>> cycleArc;	//��¼������ ����Ӧ���յ�
unordered_map<int, unordered_map<int, vector<TmpAns>>>  Answer;     //�洢��
vector<Edge> EdgeVec;								//�߼���
set<int> reCycleArc;                                //��¼�������յ㣨����������㣩
vector<int> st;										//ģ��ջ
int id=0,sum = 0,mod=0;

//ֱ�ӷ���5���ռ�
vector<vector<int>> res(5,vector<int>());	
int tab[5] = { 3,4,5,6,7 };
int key_tab[8] = { 0,0,0,0,1,2,3,4 };

struct EdgeSort
{//����߼���
	inline bool operator() (const Edge &e1, const Edge &e2) const {
		//��ʼ����ͬʱ����β��������ǰ�棬��Ϊ�ڽӱ����ǰ�巨����
		return e1.u == e2.u ? (e1.v > e2.v) : (e1.u < e2.u);
	}
};


void TopologySort()
{//��������ȥ��������,��Ч����ʱ����Tarjan�㷨��࣬����Ϊ��
	queue<int> q;
	for (auto it = graph.begin(); it != graph.end();it++)
	{
		if (state[it->first].indg == 0)
			q.push(it->first);			    //�����Ϊ0�Ľ�����,graph�н��������迼��
	}
	int cur;
	while (!q.empty())
	{							//�����������
		cur = q.front();
		q.pop();				//���Ӷ���Ԫ��
		for (Node *ptr = graph[cur].first; ptr; ptr = ptr->next)
		{						//�ڽӵ����ȼ�1
			if (graph.find(ptr->v)==graph.end())  //ͼ�в����ڵĵ㣬�����ǳ���Ϊ0�ĵ�
				continue;		                //Ϊ�Ѿ�ɾ���Ľ�㣬�������
			if ((--state[ptr->v].indg) == 0)          //�ٴν����Ϊ0�Ľ�����
				q.push(ptr->v);
		}
		graph.erase(cur);		                //���ӵ�Ԫ�ر�Ȼ�����ɻ�������ɾ��
		state[cur].isUsed = true;
		if (reCycleArc.find(cur) != reCycleArc.end())   //ɾ�������ܵ���ʼ��
			reCycleArc.erase(cur);
	}
}

void predfs(int v)
{
	if (st.size()==2 && cycleArc.find(v) != cycleArc.end())
	{//������������ʼ��
		for (auto it = cycleArc[v].begin(); it != cycleArc[v].end(); it++)
		{//�����������յ�
			if (!state[*it].Out ||st[1]<=*it)
				continue;
			if (!state[*it].In)
				state[*it].In = true;
			Answer[*it][st[0]].emplace_back(st[0],st[1],v);		
		}
	}
	if (st.size() == 2)			//ʵ���Ѿ��ǵ������ˣ��˳�
		return;
	st.emplace_back(v);
	state[v].isUsed = true;
	for (Node *ptr = graph[v].first; ptr; ptr = ptr->next)
	{//����v���ӽڵ�
		if (graph.find(v)!=graph.end() && state[ptr->v].isUsed)
			continue;
		if (!state[ptr->v].isUsed)
		{
			predfs(ptr->v);
		}
	}
	st.pop_back();
	state[v].isUsed = false;
}


//����vΪ��ʼ��Ļ�·
void findCircuit(int cur,const int & start)
{
	if (Answer[start].find(cur) != Answer[start].end())
	{
		for (const TmpAns & vec : Answer[start][cur])
		{
			if (state[vec.a].isUsed || state[vec.b].isUsed || state[vec.c].isUsed)
				continue;
			sum++;

			id = key_tab[st.size()+3];	//���������ܳ���
			for (int j = 0,Size=st.size(); j < Size; j++)
			{
				res[id].emplace_back(st[j]);
			}
			res[id].emplace_back(vec.a);
			res[id].emplace_back(vec.b);
			res[id].emplace_back(vec.c);
		}
	}
	if (st.size() == 4)
		return;
	st.emplace_back(cur);
	state[cur].isUsed = true;
	for (Node *ptr = graph[cur].first; ptr; ptr = ptr->next)
	{
		if (ptr->v <= start || state[ptr->v].isUsed)
			continue;
		if (!state[ptr->v].isUsed)
		{
			findCircuit(ptr->v,start);
		}
	}
	st.pop_back();
	state[cur].isUsed = false;
}


int main()
{
#ifdef DEBUG
	clock_t startTime, endTime;
	startTime = clock();
#endif
	res[0].reserve(1200000);//3
	res[1].reserve(1600000);//4
	res[2].reserve(4500000);//5
	res[3].reserve(6000000);//6
	res[4].reserve(14000000);//7

	//1.��ȡ�ļ����ݣ�����ԭʼͼ
	EdgeVec.reserve(280000);
	int u, v, w;
	char c;
	while (fin >> u >> c >> v >> c >> w)
	{//��  u---->v
		if (v < u)
		{
			cycleArc[u].insert(v);			//���滷��
			reCycleArc.insert(v);			//���滷�����յ㣬��Ҫ���������
		}
		else if (!state[u].Out)
			state[u].Out = true;
		state[v].indg++;
		EdgeVec.emplace_back(u,v);		
	}
	sort(EdgeVec.begin(), EdgeVec.end(), EdgeSort());		//����߼�
	EdgeVec.shrink_to_fit();			//�������ʵ���Χ
	Node *q=nullptr;
	for (int i = 0,Size= EdgeVec.size(); i < Size; i++)
	{
		q = new Node(EdgeVec[i].v, graph[EdgeVec[i].u].first);//�����ڽӱ�
		graph[EdgeVec[i].u].first = q;						//��graph�еģ����ȱ�Ȼ����0,����ͳ��
	}

#ifdef DEBUG
	endTime = clock();
	cout << "Read finished: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
#endif

	//2.��������ȥ��������
	TopologySort();

#ifdef DEBUG
	endTime = clock();
	cout << "TopologySort finished: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
#endif

	//3.��ʼ���һ��㷨
	st.reserve(2);
	for (auto it = graph.begin(); it != graph.end(); it++)
	{//Ԥ�Ƚ�������
		predfs(it->first);
	}

	//3.2 �һ�
	st.reserve(4);
	for (const int &i:reCycleArc)
	{
		if (state[i].Out && state[i].In)
		{
			findCircuit(i, i);
		}
	}

#ifdef DEBUG
	endTime = clock();
	cout << "FindCircuit finished: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
#endif

	for (int i = 0; i < 5; i++)
	{
		res[i].shrink_to_fit();
	}

	//4.������
	fprintf(fout, "%d\n", sum);
	for (int i = 0; i < 5; i++)
	{
		mod = tab[i] - 1;
		for (int j = 0,Size= res[i].size(); j <Size; j++)
		{
			if (j%tab[i] == mod)
			{
				fprintf(fout, "%d\n", res[i][j]);
				continue;
			}
			fprintf(fout, "%d,", res[i][j]);
		}
	}
	fclose(fout);
	fin.close();

#ifdef DEBUG
	endTime = clock();
	cout << "Output file: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
#endif

	return 0;
}

