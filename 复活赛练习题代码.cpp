#include<iostream>
#include<unordered_map>
#include<unordered_set>
#include<list>
#include<set>
#include<map>
#include<vector>
#include<algorithm>
#include<fstream>
#include<string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include<ctime>
using namespace std;


#define DEBUG
#define T(a,b)	(((uint64_t)a<< 31) | (b))	//����ý���ǿת����������

char fin[]="/data/test_data.txt";
char fout[]="/projects/student/result.txt";

struct vectorComp {
	inline bool operator() (const vector<int> &v1, const vector<int> &v2) const {
		if (v1.size() == v2.size())
			return lexicographical_compare(v1.begin(), v1.end(), v2.begin(), v2.end());
		return v1.size() < v2.size();
	}
};
//Ԥ�����Ľ��
struct TmpAns
{
	int a, b, c;
	TmpAns(int _a, int _b, int _c) :a(_a), b(_b), c(_c) {};
};


vector<int> vertex;
vector<int> temp;
vector<bool> isUsed;
vector<int> aidvec(10,0);
vector<vector<int>> graph;			//�ڽӾ���
vector<vector<int>> regraph;		//���ڽӾ���
unordered_map<int, int> mp;			//��ϣӳ��
unordered_map<uint64_t, float> table;	//��ֵ

unordered_map<int,set<int>> cycleArc;	//��¼������ ����Ӧ��С���յ�
unordered_map<int, unordered_map<int, vector<TmpAns>>>  Answer;     //�洢Ԥ�����Ľ��
unordered_set<int> startpoint;
vector<int> st;										//ģ��ջ
bool flag;

vector<vector<int>>  res;

void readfile(char *file)
{
    int fd=open(file,O_RDONLY);
    if(fd<0)
    {
        perror("open inputfile error!");
        exit(1);
    }
    int len= lseek(fd,0,SEEK_END);
    char *p=(char*)mmap(NULL,len,PROT_READ,MAP_PRIVATE,fd,0);
    char *s=strdup(p);
    int u, v;
    for(char *token = strsep(&s, "\n"); token != NULL  && s!=NULL; token = strsep(&s, "\n"))
    {
        u=atoi(strsep(&token,","));
        v=atoi(strsep(&token,","));
        vertex.push_back(u);
		vertex.push_back(v);
		table[T(u,v)] =atof(strsep(&token,","));
    }
    munmap(p,len);
    close(fd);

	temp.assign(vertex.begin(), vertex.end());
	sort(temp.begin(), temp.end());					//��ɢ��
	temp.erase(unique(temp.begin(), temp.end()), temp.end());

	int nodeNum = (int)temp.size();					//���õĽ������

	graph.resize(nodeNum);
	regraph.resize(nodeNum);
	isUsed.resize(nodeNum);

	for (int i = 0; i < nodeNum; i++)
	{
		mp.insert(make_pair(temp[i], i));			//���ֵ��������ӳ��
	}
	for (int i = 0; i < (int)vertex.size(); i += 2)
	{
		u = mp[vertex[i]], v = mp[vertex[i + 1]];	//��ȡ���ֵ
		if (v < u)
		{
			cycleArc[u].insert(v);				//����Ԫ��
			startpoint.insert(v);				//��¼���ܵ���ʼ��
		}
		graph[u].push_back(v);					//����ͼ
		regraph[v].push_back(u);

	}
	for (int i = 0; i < nodeNum; i++)
	{
		sort(graph[i].begin(), graph[i].end());			//�����ڽӵı�
		sort(regraph[i].begin(), regraph[i].end());		//�������ڽӵı�
	}
}

inline void intToChar(char *p,int & pos,int num)
{
    if(num==0)
    {
        p[pos++]=0+'0';
        return;
    }
    int id=0;
    while(num)
    {
        aidvec[id++]= num % 10;
        num /= 10;
    }
    for(int i=id-1;i>=0;i--)
    {
        p[pos++]=aidvec[i]+'0';
    }
    aidvec.clear();
    return;
}


void writefile(char *file)
{
    int fd=open(file,O_RDWR|O_CREAT|O_TRUNC,0666);
    int flen=1e9;
    int ret=ftruncate(fd,flen);
    if(ret<0)
    {
        perror("ftruncate error!");
        exit(1);
    }

    char *p=(char*)mmap(NULL,flen,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
    sort(res.begin(), res.end(), vectorComp());

    int idx=0;          //д�������
    intToChar(p,idx,res.size());
    p[idx++]='\n';
	for (int i = 0; i < (int)res.size(); i++)
	{
		int j = 0;
		for (; j+1 < (int)res[i].size(); j++)
		{
			intToChar(p,idx, temp[res[i][j]]);
			p[idx++]=',';
		}
		intToChar(p,idx, temp[res[i][j]]);
		p[idx++]='\n';
	}

    ret=ftruncate(fd,idx);
    munmap(p,flen);
    close(fd);
}





//�ж��� d1-->d2-->d3 �Ƿ�����Ҫ��
inline bool isProper(const int & d1,const  int &d2, const int & d3)
{
	double val = (double)table[T(temp[d2],temp[d3])] / table[T(temp[d1],temp[d2])];
	return val < 0.2 || val > 3;
}

//�����ڽӱ��з�������
void redfs(int v,const int & start)
{
	if (st.size() == 2)			//ʵ���Ѿ��ǵ������ˣ��˳�
	{
		for (auto pt = cycleArc[start].begin(); pt != cycleArc[start].end(); pt++)
		{//v-->st[1]-->st[0]-->*it
			if (v < *pt || st[1] < *pt)
				continue;
			if (isProper(v, st[1], st[0]))
				continue;
			if (isProper(st[1], st[0], *pt))
				continue;
			Answer[*pt][v].emplace_back(v, st[1], st[0]);
		}
		return;
	}
	st.push_back(v);
	isUsed[v] = true;
	auto it = lower_bound(regraph[v].begin(), regraph[v].end(), *cycleArc[start].begin());
	for (; it != regraph[v].end(); it++)
	{
		if (!isUsed[*it])
			redfs(*it,start);
	}
	st.pop_back();
	isUsed[v] = false;
}


//����vΪ��ʼ��Ļ�·
void dfs(int cur, const int & start)
{
	if (Answer[start].find(cur) != Answer[start].end())
	{
		for (const TmpAns & vec : Answer[start][cur])
		{
			if (isUsed[vec.a] || isUsed[vec.b] || isUsed[vec.c])
				continue;
			int flag = true;
			//�ж�ת�˽����Ƿ�Ϸ�
			switch ((int)st.size())
			{
			case 0:	//vec.a-->vec.b-->vec.c-->vec.a
				if (isProper(vec.c,vec.a,vec.b))
					flag = false;
				break;
			case 1://st[0]-->vec.a-->vec.b-->vec.c-->st[0]
				if (isProper(vec.c,st[0],vec.a))
				{
					flag = false;
					break;
				}
				if (isProper(st[0], vec.a,vec.b))
					flag = false;
				break;
			case 2://st[0]-->st[1]-->vec.a-->vec.b-->vec.c-->st[0]
				if (isProper(vec.c,st[0],st[1]))
				{
					flag = false;
					break;
				}
				if (isProper(st[0], st[1],vec.a))
				{
					flag = false;
					break;
				}
				if (isProper(st[1], vec.a,vec.b))
				{
					flag = false;
				}
				break;
			case 3://st[0]-->st[1]-->st[2]-->vec.a-->vec.b-->vec.c-->st[0]
				if (isProper(vec.c,st[0],st[1]))
				{
					flag = false;
					break;
				}
				if (isProper(st[0], st[1],st[2]))
				{
					flag = false;
					break;
				}
				if (isProper(st[1], st[2],vec.a))
				{
					flag = false;
					break;
				}
				if (isProper(st[2], vec.a,vec.b))
				{
					flag = false;
				}
				break;
			case 4://st[0]-->st[1]-->st[2]-->st[3]-->vec.a-->vec.b-->vec.c-->st[0]
				if (isProper(vec.c,st[0],st[1]))
				{
					flag = false;
					break;
				}
				if (isProper(st[0],st[1],st[2]))
				{
					flag = false;
					break;
				}
				if (isProper(st[1],st[2],st[3]))
				{
					flag = false;
					break;
				}
				if (isProper(st[2], st[3],vec.a))
				{
					flag = false;
					break;
				}
				if (isProper(st[3],vec.a,vec.b))
				{
					flag = false;
				}
				break;
            case 5://st[0]-->st[1]-->st[2]-->st[3]-->st[4]-->vec.a-->vec.b-->vec.c-->st[0]
            if (isProper(vec.c,st[0],st[1]))
				{
					flag = false;
					break;
				}
				if (isProper(st[0],st[1],st[2]))
				{
					flag = false;
					break;
				}
				if (isProper(st[1],st[2],st[3]))
				{
					flag = false;
					break;
				}
				if (isProper(st[2], st[3],st[4]))
				{
					flag = false;
					break;
				}
				if (isProper(st[3],st[4],vec.a))
				{
					flag = false;
					break;
				}
				if(isProper(st[4],vec.a,vec.b))
                    flag=false;
                break;
			}
			if (flag == false)
				continue;

			vector<int> tmp(3+st.size(),0);		//�����ڴ�
			int k = 0;
			for (int j = 0, Size = st.size(); j < Size; j++,k++)
			{
				tmp[k]=st[j];
			}
			tmp[k++] = vec.a;
			tmp[k++] = vec.b;
			tmp[k++] = vec.c;
			res.emplace_back(tmp);
		}
	}
	if (st.size() ==5 )
		return;

	st.emplace_back(cur);
	isUsed[cur] = true;

	auto it = lower_bound(graph[cur].begin(), graph[cur].end(), start);
	for (; it != graph[cur].end(); it++)
	{//ֱ�Ӷ��ֲ��ң�������С�ļ�ֵ
		if (!isUsed[*it])
			dfs(*it, start);
	}
	st.pop_back();
	isUsed[cur]= false;
}


int main()
{

#ifdef DEBUG
	clock_t startTime, endTime;
	startTime = clock();
#endif
	//1.mmap�������ݲ�����ɢ��
	readfile(fin);

#ifdef DEBUG
	endTime = clock();
	cout << "normalized finished: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
#endif

	//2.�ӽ��������������dfs����
	st.reserve(4);
	for (auto it = cycleArc.begin(); it != cycleArc.end(); it++)
	{
        redfs(it->first,it->first);
	}
    //����ʼ���������dfs����
	for (const int &i : startpoint)
	{
        dfs(i,i);
	}

#ifdef DEBUG
	endTime = clock();
	cout << "FindCircuit finished: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
#endif

	//3.������
	writefile(fout);

#ifdef DEBUG
	endTime = clock();
	cout << "Output file: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
#endif

	return 0;
}

