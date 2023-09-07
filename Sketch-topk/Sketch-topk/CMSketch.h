#ifndef _cmsketch_H
#define _cmsketch_H
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <string>
#include <cstring>
#include "BaseSketch.h"
#include "BOBHASH32.h"
#include "params.h"
#include "ssummary.h"
#include "BOBHASH64.h"
#define CM_d 4
#define rep(i,a,n) for(int i=a;i<=n;i++)
using namespace std;
class cmsketch : public sketch::BaseSketch{
private:
	ssummary *ss;
	struct node { int C; } HK[CM_d][MAX_MEM+10];
	BOBHash64 * bobhash;
	int K, M2;
public:
	cmsketch(int M2, int K) :M2(M2), K(K) { ss = new ssummary(K); ss->clear(); bobhash = new BOBHash64(1005); }
	void clear()
	{
		for (int i = 0; i < CM_d; i++)
			for (int j = 0; j <= M2 + 5; j++)
				HK[i][j].C = 0;
	}
	unsigned long long Hash(string ST)
	{
		return (bobhash->run(ST.c_str(), ST.size()));
	}
	void Insert(const string &x)
	{
		bool mon = false;
		int p = ss->find(x);
		if (p) mon = true;
		int minv = 0x7fffffff;
		unsigned long long hash[CM_d];
		for (int i = 0; i < CM_d; i++)
			hash[i]=Hash(x+std::to_string(i))%(M2-(2*CM_d)+2*i+3);
			
		for(int i = 0; i < CM_d; i++)
		{
			HK[i][hash[i]].C++;
			minv=min(minv,HK[i][hash[i]].C);
		}
		
		if (!mon)
		{
			if (minv - (ss->getmin()) > 0 || ss->tot < K)
			{
				int i = ss->getid();
				ss->add2(ss->location(x), i);
				ss->str[i] = x;
				ss->sum[i] = minv;
				ss->link(i, 0);
				while (ss->tot > K)
				{
					int t = ss->Right[0];
					int tmp = ss->head[t];
					ss->cut(ss->head[t]);
					ss->recycling(tmp);
				}
			}
		}
		else
			if (minv > ss->sum[p])
			{
				int tmp = ss->Left[ss->sum[p]];
				ss->cut(p);
				if (ss->head[ss->sum[p]]) tmp = ss->sum[p];
				ss->sum[p] = minv;
				ss->link(p, tmp);
			}
	}
	struct Node { string x; int y; } q[MAX_MEM + 10];
	static int cmp(Node i, Node j) { return i.y > j.y; }
	void work()
	{
		int CNT = 0;
		for (int i = N; i; i = ss->Left[i])
			for (int j = ss->head[i]; j; j = ss->Next[j]) { q[CNT].x = ss->str[j]; q[CNT].y = ss->sum[j]; CNT++; }
		sort(q, q + CNT, cmp);
	}
	pair<string, int> Query(int k)
	{
		return make_pair(q[k].x, q[k].y);
	}
	std::string get_name() {
		return "cmsketch";
	}
};
#endif
