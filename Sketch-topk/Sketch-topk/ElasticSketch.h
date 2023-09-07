#ifndef _elasticsketch_H
#define _elasticsketch_H

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
#define ES_d 1
#define BN 7
#define lambda 8
#define rep(i,a,n) for(int i=a;i<=n;i++)
using namespace std;
class elasticsketch : public sketch::BaseSketch
{
private:
	ssummary *ss;
	struct heavy { int FP, pvote, nvote, Flag; } HK[MAX_MEM+10][BN];
	struct light { int C; } LK[ES_d][MAX_MEM+10];
	BOBHash64 * bobhash;
	int K, M1, M2;

public:
	elasticsketch(int M1, int M2, int K) :M1(M1), M2(M2), K(K) { ss = new ssummary(K); ss->clear(); bobhash = new BOBHash64(1005); }
	void clear()
	{
		for (int i = 0; i < M1 + 5; i++)
			for (int j = 0; j <= BN; j++)
				HK[i][j].FP = HK[i][j].pvote = HK[i][j].nvote = HK[i][j].Flag = 0;
		for (int i = 0; i < ES_d; i++)
			for (int j = 0; j < M2 + 5; j++)
				LK[i][j].C = 0;
			
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
		int tmpmaxv = N;
		int maxv = 0;
		unsigned long long H = Hash(x); int FP = (H>>32);
		unsigned long long H1 = H % M1;
		unsigned long long hash[ES_d];
		for (int i = 0; i < ES_d; i++)
			hash[i]=Hash(std::to_string(FP+i))%(M2-(2*ES_d)+2*i+3);
		int min_size = N;
		int min_pos = -1;
		int flag = 0;
		for (int i = 0; i < BN; i++) {
			if (HK[H1][i].FP == FP) {
				HK[H1][i].pvote++;
				flag = 1;
				break;
			}
			else if (HK[H1][i].pvote == 0) {
				HK[H1][i].pvote = 1;
				HK[H1][i].FP = FP;
				flag = 1;
				break;
			}
			if (min_size > HK[H1][i].pvote) {
				min_pos = i;
				min_size = HK[H1][i].pvote;
			}
		}
		if (!flag) {
			HK[H1][min_pos].nvote++;
			if (HK[H1][min_pos].nvote / HK[H1][min_pos].pvote >= lambda) {
				for (int i = 0; i < ES_d; i++) {
					int tmphash = Hash(std::to_string(HK[H1][min_pos].FP+i))%(M2-(2*ES_d)+2*i+3);
					LK[i][tmphash].C += HK[H1][min_pos].pvote;
				}
				HK[H1][min_pos].FP = FP;
				HK[H1][min_pos].Flag = 1;
				HK[H1][min_pos].nvote = 1;
				HK[H1][min_pos].pvote = 1;
				flag = 2;
			}
		}
		if (!flag) {
			for (int i = 0; i < ES_d; i++) {
				LK[i][hash[i]].C++;
			}
		}
		for (int i = 0; i < BN; i++) {
			if (HK[H1][i].FP == FP) {
				maxv = HK[H1][i].pvote;
				if (HK[H1][i].Flag == 1) {
					for (int i = 0; i < ES_d; i++) {
						tmpmaxv = min(tmpmaxv, LK[i][hash[i]].C);
					}
					maxv += tmpmaxv;
					break;
				}
			}
		}
		if (maxv == 0) {
			for (int i = 0; i < ES_d; i++) {
				tmpmaxv = min(tmpmaxv, LK[i][hash[i]].C);
			}
			maxv = tmpmaxv;
		}
		
		if (!mon)
		{
			if (maxv - (ss->getmin()) > 0 || ss->tot < K)
			{
				int i = ss->getid();
				ss->add2(ss->location(x), i);
				ss->str[i] = x;
				ss->sum[i] = maxv;
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
			if (maxv > ss->sum[p])
			{
				int tmp = ss->Left[ss->sum[p]];
				ss->cut(p);
				if (ss->head[ss->sum[p]]) tmp = ss->sum[p];
				ss->sum[p] = maxv;
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

	std::string get_name() override {
		return "ElasticSketch";
	}
};
#endif
