#ifndef _wavingsketch_H
#define _wavingsketch_H
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
#define WS_d 4 // waving sketch的heavy part的宽度
#define rep(i,a,n) for(int i=a;i<=n;i++)
using namespace std;
class wavingsketch : public sketch::BaseSketch {
private:
    struct item { string ID;int freq, flag; }; // 下层waving sketch中的元素
    struct bucket { int count; item heavy[WS_d]; }; // 下层waving sketch中的桶
    bucket WS[MAX_MEM+10]; // 下层waving sketch
    BOBHash64 * bobhash; // 哈希函数对象
    int K, M2; // topk参数，桶的个数
public:
    wavingsketch(int M2, int K): K(K), M2(M2) {
        bobhash = new BOBHash64(1005);
    }
    void clear() {
        memset(WS, 0, sizeof(WS));
    }
    unsigned long long Hash(string ST)
	{
		return (bobhash->run(ST.c_str(), ST.size()));
	}
    void Insert(const string &x) {
        int hash = Hash(x) % M2;
        int sign = (Hash(x) & 1) ? 1 : -1;
        bool found = false;
        rep(i, 0, WS_d-1) {
            if (WS[hash].heavy[i].ID == x) {
                found = true;
                WS[hash].heavy[i].freq++;
                if (!WS[hash].heavy[i].flag) WS[hash].count += sign;
                break;
            }
        }
        if (!found) {
            bool empty = false;
            rep(i, 0, WS_d-1) {
                if (WS[hash].heavy[i].ID == "") {
                    empty = true;
                    WS[hash].heavy[i].ID = x;
                    WS[hash].heavy[i].freq = 1;
                    WS[hash].heavy[i].flag = true;
                    break;
                }
            }
            if (!empty) {
                int min_freq = 0x7fffffff, min_pos = -1;
                rep(i, 0, WS_d-1) {
                    if (WS[hash].heavy[i].freq < min_freq) {
                        min_freq = WS[hash].heavy[i].freq;
                        min_pos = i;
                    }
                }
                if (WS[hash].count * sign >= min_freq) {
                    string temp = WS[hash].heavy[min_pos].ID;
                    WS[hash].heavy[min_pos].ID = x;
                    WS[hash].heavy[min_pos].freq = WS[hash].count * sign + 1;
                    if (WS[hash].heavy[min_pos].flag) WS[hash].count += min_freq * ((Hash(temp) & 1) ? 1 : -1);
                    WS[hash].heavy[min_pos].flag = false;
                }
                WS[hash].count += sign;
            }
        }
	}
	struct Node { string x; int y; } q[MAX_MEM + 10];
	static int cmp(Node i, Node j) { return i.y > j.y; }
	void work()
	{
		int CNT = 0;
		rep(i,0,M2-1){
            rep(j,0,WS_d-1){
                if(WS[i].heavy[j].ID!=""){
                    q[CNT].x = WS[i].heavy[j].ID;
                    q[CNT].y = WS[i].heavy[j].freq;
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
		return "wavingsketch";
	}
};
#endif