#ifndef _cuckoocounter_H
#define _cuckoocounter_H

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <string>
#include <cstring>
#include <unordered_map>

#include "BaseSketch.h"
#include "BOBHASH32.h"
#include "params.h"
#include "ssummary.h"
#include "BOBHASH64.h"
#define CC_d 2
#define BN 4
#define rep(i,a,n) for(int i=a;i<=n;i++)
using namespace std;
class cuckoocounter : public sketch::BaseSketch
{
private:
	ssummary *ss;
	struct node { int C, FP; } HK[CC_d][MAX_MEM + 10][BN];
	BOBHash64 * bobhash;
	int K, M2;
	int thresh;
	double epsilon;
public:
	cuckoocounter(int M2, int K, int t, double e) :M2(M2), K(K), thresh(t), epsilon(e)
	{ ss = new ssummary(K); ss->clear(); bobhash = new BOBHash64(1005); }
	void clear()
	{
		for (int i = 0; i < CC_d; i++)
			for (int j = 0; j <= M2 + 5; j++)
				for(int r=0;r<BN;r++)HK[i][j][r].C = HK[i][j][r].FP = 0;
	}
	unsigned long long Hash(string ST)
	{
		return (bobhash->run(ST.c_str(), ST.size()));
	}

	void rehash(node hash_entry, int loop_times, int j, unsigned long long hash) {
		int k=1-j;
		unsigned long long re_hash = hash ^ Hash(std::to_string(hash_entry.FP));
		int Hsh = re_hash % (M2 - (2 * CC_d) + 2 *(1-j) + 3);
		for (int r = 0; r < BN; r++) {
			if (HK[k][Hsh][r].FP == 0) {
				HK[k][Hsh][r].FP = hash_entry.FP;
				HK[k][Hsh][r].C = hash_entry.C;
				return;
			}
		}
		if (loop_times == 0) {//if the loop times bigger than the thresh
			HK[k][Hsh][1].FP = hash_entry.FP;//no matter rehash the fingerprint or not 
			if (hash_entry.C<HK[k][Hsh][1].C)
				HK[k][Hsh][1].C = hash_entry.C;//replace entry2 with min count
				return;	
		}
		node tmp;
		tmp.FP = HK[k][Hsh][1].FP;
		tmp.C = HK[k][Hsh][1].C;
		HK[k][Hsh][1].FP = hash_entry.FP;
		HK[k][Hsh][1].C = hash_entry.C;
		hash_entry.FP = tmp.FP;
		hash_entry.C = tmp.C;
		loop_times--;
		//printf("the loop_times = %6d\n", loop_times);
		rehash(hash_entry, loop_times, k, re_hash);
	}

	void Insert(const string &x)
	{
		bool mon = false;
		int p = ss->find(x);
		if (p) mon = true;
		int maxv = 0;
		int max_loop = 1;
		node temp;
		unsigned long long H1 = Hash(x); int FP = (H1 >> 56);
		unsigned long long H2 = H1^FP;//Hash(std::to_string(FP));//XOR CUCKOO HASHING
	//	unsigned long long H2 = H1^Hash(std::to_string(FP)); //XOR CUCKOO HASHING
		int Hsh1 = H1 % (M2 - (2 * CC_d) + 2 * 0 + 3);
		int Hsh2 = H2 % (M2 - (2 * CC_d) + 2 * 1 + 3);
		int count = 0;
		
		int hash[2] = { Hsh1, Hsh2 };
		unsigned long long hashHH[2] = { H1, H2 };
		
		int ii, jj, mi=(1<<25);	
		for(int i = 0; i < CC_d; i++)
			for (int j = 0; j < BN; j++)
			{	
				if (mi > HK[i][hash[i]][j].C){
					ii=i; jj=j; mi=HK[i][hash[i]][j].C;
				}	
				if (HK[i][hash[i]][j].FP == FP) {
					int c = HK[i][hash[i]][j].C;
					if (mon || c <= ss->getmin())
						HK[i][hash[i]][j].C++;
					maxv = max(maxv, HK[i][hash[i]][j].C);
					count = 1;
					break;
				}
				if(HK[i][hash[i]][j].FP == 0)
				{
					HK[i][hash[i]][j].FP=FP;
					HK[i][hash[i]][j].C=1;
					maxv=max(maxv,1);
					count = 1;
					break;
				}
			}

		if (count == 0) {	//mean can not insert normally
			HK[ii][hash[ii]][jj].FP = FP;
			HK[ii][hash[ii]][jj].C = 1;
			maxv=max(maxv, 1);
		//	rehash(temp, max_loop, ii, hashHH[ii]);
		}

		if (!mon)
		{
			if (maxv - (ss->getmin()) == 1 || ss->tot < K)
			{
				int i = ss->getid();
				ss->add2(ss->location(x), i);
				ss->str[i] = x;
				ss->sum[i] = maxv;
				ss->tmp[i] = maxv;
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
	struct Node { string x; int y; int thre;} q[MAX_MEM + 10];
	static int cmp(Node i, Node j) { return i.y > j.y; }
	struct Node2 {int FP; int tmpFQ; int fnlFQ; } p[MAX_MEM + 10];
	static int cmp2(Node2 i, Node2 j) { 
		return i.FP < j.FP;
	}
	unordered_map<int, int> mp;
	void work()
	{
		int CNT = 0;
		for (int i = N; i; i = ss->Left[i])
			for (int j = ss->head[i]; j; j = ss->Next[j]) 
			{ 
				q[CNT].x = ss->str[j]; q[CNT].y = ss->sum[j]; CNT++; 
				q[CNT].thre=ss->sum[j]-ss->tmp[j];
				mp[Hash(ss->str[j]) >> 56]++;
			}
		sort(q, q + CNT, cmp);
	}
	int jump=0, x_num=0;
	pair<string, int> Query(int k)
	{
		while(mp[Hash(q[k+jump].x)]>1 && q[k+jump].thre<thresh && x_num<20){
			x_num++;
			jump++;
		}
		return make_pair(q[k+jump].x, q[k+jump].y);
	}
	std::string get_name() override {
		return "CuckooCounter";
	}
};
#endif
