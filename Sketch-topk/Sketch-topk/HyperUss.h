#ifndef _hyperuss_H
#define _hyperuss_H
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <string>
#include <cstring>
#include "BaseSketch.h"
#include "BOBHASH64.h"
#include "params.h"
#define HU_d 4
#define rep(i,a,n) for(int i=a;i<=n;i++)
using namespace std;
class hyperuss : public sketch::BaseSketch{
private:
	struct node { string ID; int C; } HK[HU_d][MAX_MEM+10];
	BOBHash64 * bobhash;
	int K, M2;
public:
	hyperuss(int M2, int K) :M2(M2), K(K) { bobhash = new BOBHash64(1005); }
	void clear()	{
		for (int i = 0; i < HU_d; i++)
			for (int j = 0; j <= M2 + 5; j++)
				HK[i][j].C = 0,HK[i][j].ID ="";
	}
	unsigned long long Hash(string ST)	{
		return (bobhash->run(ST.c_str(), ST.size()));
	}
	void Insert(const string &x)	{
		int minv = 0x7fffffff;
		unsigned long long hash[HU_d];
		for (int i = 0; i < HU_d; i++)
			hash[i]=Hash(x+std::to_string(i))%(M2-(2*HU_d)+2*i+3);
        bool flag0 = false,flag1 = false;
        for(int i = 0; i < HU_d; i++)
		{
			if (HK[i][hash[i]].ID == x) {
				HK[i][hash[i]].C++;
                flag0 = true;
                break;
			}
        }
        if(!flag0){
            for(int i = 0; i < HU_d; i++){
			    if (HK[i][hash[i]].ID == "") {
				    HK[i][hash[i]].ID=x;
				    HK[i][hash[i]].C=1;
                    flag1 = true;
                    break;
			    }
            }
        }
        if(!flag0 && !flag1){
            int mini;
            for(int i=0;i<HU_d;i++){
                if(HK[i][hash[i]].C<minv){
                    minv = HK[i][hash[i]].C;
                    mini = i;
                }
            }
            double p = 1.0 / (minv + 1);
			double q = 1.0 - p;
			double r = (double)rand() / RAND_MAX;
			if (r < p) {
				HK[mini][hash[mini]].ID = x;
				HK[mini][hash[mini]].C = 1 / p;
			}
			else {
				HK[mini][hash[mini]].C = minv / q;
			}
        }
	}
	struct Node { string x; int y; } q[MAX_MEM + 10]; 
	static bool cmp(Node i, Node j) { return i.y > j.y; } 
    void work()
	{
		int CNT = 0;
		for (int i = 0; i<HU_d; i++){
			for(int j=0;j<M2;j++){
				if(HK[i][j].ID!=""){
					q[CNT].x = HK[i][j].ID; 
					q[CNT].y = HK[i][j].C; 
					CNT++;
				}
			}		
		}
		sort(q, q + CNT, cmp);	
	}
	pair<string, int> Query(int k)
	{
		return make_pair(q[k].x, q[k].y);
	}
	std::string get_name() {
		return "hyperuss";
	}
};
#endif
