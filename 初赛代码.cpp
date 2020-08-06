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

//有向图的邻接表结构
struct Node
{
	int v;
	Node* next;
	Node(int _v,Node* _next) :v(_v), next(_next) {};
};

struct Hnode
{
	Node *first;		//保存邻接表
};

struct TmpAns
{
	int a, b, c;
	TmpAns(int _a, int _b, int _c) :a(_a), b(_b), c(_c) {};
};
//拓扑排序所需的数据结构
struct Data
{
	bool isUsed;		//是否被访问
	bool Out;			//出边是否，满足要求
	bool In;			//三层搜索的入边是否满足要求
	int indg;			//入度数量
	Data() :isUsed(false),Out(false),In(false),indg(0) {};
};

unordered_map<int, Hnode> graph;					//如果存在边
unordered_map<int, Data> state;						//图中结点的状态
unordered_map<int, unordered_set<int>> cycleArc;	//记录环弧的 起点对应的终点
unordered_map<int, unordered_map<int, vector<TmpAns>>>  Answer;     //存储答案
vector<Edge> EdgeVec;								//边集合
set<int> reCycleArc;                                //记录环弧的终点（即搜索的起点）
vector<int> st;										//模拟栈
int id=0,sum = 0,mod=0;

//直接分配5个空间
vector<vector<int>> res(5,vector<int>());	
int tab[5] = { 3,4,5,6,7 };
int key_tab[8] = { 0,0,0,0,1,2,3,4 };

struct EdgeSort
{//排序边集合
	inline bool operator() (const Edge &e1, const Edge &e2) const {
		//起始点相同时，结尾点大的排在前面，因为邻接表采用前插法构建
		return e1.u == e2.u ? (e1.v > e2.v) : (e1.u < e2.u);
	}
};


void TopologySort()
{//拓扑排序，去掉多余结点,其效果和时间与Tarjan算法差不多，但更为简单
	queue<int> q;
	for (auto it = graph.begin(); it != graph.end();it++)
	{
		if (state[it->first].indg == 0)
			q.push(it->first);			    //将入度为0的结点入队,graph中结点出度无需考虑
	}
	int cur;
	while (!q.empty())
	{							//广度优先搜索
		cur = q.front();
		q.pop();				//出队队首元素
		for (Node *ptr = graph[cur].first; ptr; ptr = ptr->next)
		{						//邻接点的入度减1
			if (graph.find(ptr->v)==graph.end())  //图中不存在的点，可能是出度为0的点
				continue;		                //为已经删除的结点，无需访问
			if ((--state[ptr->v].indg) == 0)          //再次将入度为0的结点入队
				q.push(ptr->v);
		}
		graph.erase(cur);		                //出队的元素必然构不成环，将其删除
		state[cur].isUsed = true;
		if (reCycleArc.find(cur) != reCycleArc.end())   //删除不可能的起始点
			reCycleArc.erase(cur);
	}
}

void predfs(int v)
{
	if (st.size()==2 && cycleArc.find(v) != cycleArc.end())
	{//包含环弧的起始点
		for (auto it = cycleArc[v].begin(); it != cycleArc[v].end(); it++)
		{//遍历环弧的终点
			if (!state[*it].Out ||st[1]<=*it)
				continue;
			if (!state[*it].In)
				state[*it].In = true;
			Answer[*it][st[0]].emplace_back(st[0],st[1],v);		
		}
	}
	if (st.size() == 2)			//实际已经是第三层了，退出
		return;
	st.emplace_back(v);
	state[v].isUsed = true;
	for (Node *ptr = graph[v].first; ptr; ptr = ptr->next)
	{//遍历v的子节点
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


//求以v为起始点的回路
void findCircuit(int cur,const int & start)
{
	if (Answer[start].find(cur) != Answer[start].end())
	{
		for (const TmpAns & vec : Answer[start][cur])
		{
			if (state[vec.a].isUsed || state[vec.b].isUsed || state[vec.c].isUsed)
				continue;
			sum++;

			id = key_tab[st.size()+3];	//计算结果的总长度
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

	//1.读取文件内容，构造原始图
	EdgeVec.reserve(280000);
	int u, v, w;
	char c;
	while (fin >> u >> c >> v >> c >> w)
	{//边  u---->v
		if (v < u)
		{
			cycleArc[u].insert(v);			//保存环弧
			reCycleArc.insert(v);			//保存环弧的终点，是要搜索的起点
		}
		else if (!state[u].Out)
			state[u].Out = true;
		state[v].indg++;
		EdgeVec.emplace_back(u,v);		
	}
	sort(EdgeVec.begin(), EdgeVec.end(), EdgeSort());		//排序边集
	EdgeVec.shrink_to_fit();			//收缩到适当范围
	Node *q=nullptr;
	for (int i = 0,Size= EdgeVec.size(); i < Size; i++)
	{
		q = new Node(EdgeVec[i].v, graph[EdgeVec[i].u].first);//构造邻接表
		graph[EdgeVec[i].u].first = q;						//在graph中的，出度必然大于0,不用统计
	}

#ifdef DEBUG
	endTime = clock();
	cout << "Read finished: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
#endif

	//2.拓扑排序去除多余结点
	TopologySort();

#ifdef DEBUG
	endTime = clock();
	cout << "TopologySort finished: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
#endif

	//3.开始查找环算法
	st.reserve(2);
	for (auto it = graph.begin(); it != graph.end(); it++)
	{//预先进行深搜
		predfs(it->first);
	}

	//3.2 找环
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

	//4.输出结果
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

