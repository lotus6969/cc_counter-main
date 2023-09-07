#ifndef _dasketch_H
#define _dasketch_H
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
#include "BOBHASH64.h"
#define CMM_d 4
#define TOP_d 4
#define rep(i,a,n) for(int i=a;i<=n;i++)
using namespace std;
class dasketch : public sketch::BaseSketch{
private:
    struct cell { //单元结构体，存储流ID，策略频率Cs和真实频率Cr
        string ID;
        int Cs;
        int Cr;
    };
    struct bucket { //一个桶内多个单元
        cell cells[TOP_d];
    };
    bucket *bk; //桶数组指针
    struct node { int C; } HK[CMM_d][MAX_MEM+10]; //下层cmm 
    BOBHash64 * bobhash; 
    int K, M2; //K,M2:上层桶数组宽度以及下层cmm宽度
    int total; //cmm总元素数量
public:
    dasketch(int M2, int K) :M2(M2), K(K) { 
        bk = new bucket[M2]; 
        bobhash = new BOBHash64(1005); 
        total = 0; 
    }
    void clear()
    {
        for (int i = 0; i < M2; i++) 
            for (int j = 0; j < TOP_d; j++) {
                bk[i].cells[j].ID = "";
                bk[i].cells[j].Cs = 0;
                bk[i].cells[j].Cr = 0;
            }
        for (int i = 0; i < CMM_d; i++) 
            for (int j = 0; j <= M2 + 5; j++)
                HK[i][j].C = 0;
        total = 0; 
    }
    unsigned long long Hash(string ST)
    {
        return (bobhash->run(ST.c_str(), ST.size()));
    }
    void Insert(const string &x)
    {
        int h = Hash(x) % M2; //桶数组索引
        bool match = false; //标记是否匹配到单元
        bool empty = false; //标记是否有空单元
        int minv = 0x7fffffff; //记录最小的Cs值
        int minp = -1; //记录最小的Cs值对应的位置
        for (int i = 0; i < TOP_d; i++) { //是否有匹配的单元
            if (bk[h].cells[i].ID == x) { //如果匹配到单元，就直接增加Cs和Cr
                bk[h].cells[i].Cs++;
                bk[h].cells[i].Cr++;
                match = true;
                break;
            }
        }
        if (!match) { //是否有空位
            for (int i = 0; i < TOP_d; i++) {
                if (bk[h].cells[i].ID == "") { //如果有空位，就将空位赋值为插入元素，并且设置Cs和Cr为1
                    bk[h].cells[i].ID = x;
                    bk[h].cells[i].Cs = 1;
                    bk[h].cells[i].Cr = 1;
                    empty = true;
                    break;
                }
            }
        }
        if (!match && !empty) { //如果没匹配到也没有空位，使用替换策略
            for(int i = 0; i < TOP_d; i++){ //寻找最小单元
			    if(bk[h].cells[i].Cs<minv){
                    minv = bk[h].cells[i].Cs;
                    minp = i;
                }
		    }
            double p = 1.0 / (minv + 1); //替换概率
            double r = rand() / double(RAND_MAX); //生成随机数
            if (r < p) { //如果替换成功，就将替换单元的流ID改为插入元素，Cs改为minv+1，Cr改为1，同时将被驱逐的元素的Cr散列到下层cmm中
                string evicted = bk[h].cells[minp].ID; //记录被驱逐的元素
                int evicted_cr = bk[h].cells[minp].Cr; //记录被驱逐的元素的Cr
                bk[h].cells[minp].ID = x; //替换流ID
                bk[h].cells[minp].Cs = minv + 1; //更新Cs
                bk[h].cells[minp].Cr = 1; //更新Cr
                unsigned long long hash[CMM_d]; //计算被驱逐的元素在下层cmm中的位置
                for (int i = 0; i < CMM_d; i++)
                    hash[i]=Hash(evicted+std::to_string(i))%(M2-(2*CMM_d)+2*i+3);
                for(int i = 0; i < CMM_d; i++) { //将被驱逐的元素的Cr散列到下层cmm中
                    HK[i][hash[i]].C += evicted_cr;
                }
                total += evicted_cr; //更新插入下层总元素数量
            }
            else { //如果替换失败，就将插入元素插入到下层cmm中
                unsigned long long hash[CMM_d]; 
                for (int i = 0; i < CMM_d; i++)
                    hash[i]=Hash(x+std::to_string(i))%(M2-(2*CMM_d)+2*i+3);
                for(int i = 0; i < CMM_d; i++) { 
                    HK[i][hash[i]].C++;
                }
                total++; 
            }
        }
    }
    struct Node { string x; int y; } q[MAX_MEM + 10]; 
    static int cmp(Node i, Node j) { return i.y > j.y; } 
    void work()
    {
        int CNT = 0;
        for (int i = 0; i < M2; i++) 
            for (int j = 0; j < TOP_d; j++) {
                if (bk[i].cells[j].ID != "") { 
                    q[CNT].x = bk[i].cells[j].ID;
                    q[CNT].y = bk[i].cells[j].Cs;
                    CNT++;
                }
            }
        sort(q, q + CNT, cmp); 
    }
    pair<string, int> Query(int k)
    {
        return make_pair(q[k].x, q[k].y); 
    }
    std::string get_name() {
        return "dasketch";
    }
};
#endif 