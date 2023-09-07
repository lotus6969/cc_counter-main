#ifndef _ASketch_H
#define _ASketch_H

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
#define COUNTER_SIZE 16
#define MAX_HASH_NUM 20
#define rep(i,a,n) for(int i=a;i<=n;i++)
using namespace std;

class ASketch : public sketch::BaseSketch
{
    private:
		int w, d;
		int *new_count;
		int *old_count;
		char **items;
		int *counter[MAX_HASH_NUM];
		int MAX_CNT;
		int Myindex[MAX_HASH_NUM];
		BOBHash64 * bobhash[MAX_HASH_NUM];
		int K,M2;
	       	int FILTER_SIZE;	
    public:
        ASketch(int M2,int K):M2(M2),K(K)
        {
        	w = M2+5;
			d = 1;
			FILTER_SIZE = K;
			for(int i = 0; i < d; i++)	
			{
				counter[i] = new int[w];
				memset(counter[i], 0, sizeof(int) * w);
			}
	
			MAX_CNT = 1<<30;//(1 << COUNTER_SIZE) - 1;
	
			for(int i = 0; i < d; i++)
			{
				bobhash[i] = new BOBHash64(i + 1000);
			}
	
			items = new char *[FILTER_SIZE];
			for(int i = 0; i < FILTER_SIZE; i++)
			{
				items[i] = new char[100];
				items[i][0] = '\0';
			}
	
			new_count = new int[FILTER_SIZE];
			old_count = new int[FILTER_SIZE];
			memset(new_count, 0, sizeof(int) * FILTER_SIZE);
			memset(old_count, 0, sizeof(int) * FILTER_SIZE);
		}
		void clear() {

		}
        ~ASketch()
		{
			for(int i = 0; i < d; i++)	
			{
				delete []counter[i];
			}
	
	
			for(int i = 0; i < d; i++)
			{
				delete bobhash[i];
			}
			for(int i = 0; i < FILTER_SIZE; i++)
			{
				delete []items[i];
			}
			
			delete old_count;
			delete new_count;
		}
        
		int find_element_in_filter(string str)
		{
			for (int i = 0; i < FILTER_SIZE; i++) {
				if(strcmp(str.c_str(), items[i]) == 0)
					return i;
			}
			return -1;
		}
		//can finish in finding element in filter
		int find_empty_in_filter()
		{
			for(int i = 0; i < FILTER_SIZE; i++)
			{
				if(strlen(items[i]) == 0)
					return i;
			}
			return -1;
		}
		void Insert(const string &x)
		{	
			int index = find_element_in_filter(x);
			int index_empty = find_empty_in_filter();
			int estimate_value, min_index, min_value, hash_value, temp;
			if(index != -1)
			{
				new_count[index] += 1;
				return;
			}
			else if(index_empty != -1)
			{
				strcpy(items[index_empty], x.c_str());
				new_count[index_empty] = 1;
				old_count[index_empty] = 0;
			}
			else
			{
				estimate_value = (1 << 30);
				for(int i = 0; i < d; i++)
				{
					hash_value = (bobhash[i]->run(x.c_str(), x.length())) % w;
					if(counter[i][hash_value] != MAX_CNT)
					{
						counter[i][hash_value] ++;
						estimate_value = estimate_value < counter[i][hash_value] ? estimate_value : counter[i][hash_value];
					}
				}
				min_index = 0;
				min_value = (1 << 30);
				for(int i = 0; i < FILTER_SIZE; i++)
				{
					if(strlen(items[i]) != 0 && min_value > new_count[i])
					{
						min_value = new_count[i];
						min_index = i;
					}
				}
				if(estimate_value > min_value)
				{
					temp = new_count[min_index] - old_count[min_index];
					if(temp > 0)
					{
						for(int i = 0; i < d; i++)
						{
							hash_value = (bobhash[i]->run(items[min_index], strlen(items[min_index]))) % w;
							if(counter[i][hash_value] != MAX_CNT)
							{
								counter[i][hash_value] += temp;
							}
						}
					}
					strcpy(items[min_index], x.c_str());
					new_count[min_index] = estimate_value;
					old_count[min_index] = estimate_value;
				}
			}
		}
        struct Node {string x; int y;} q[MAX_MEM+10];
        static int cmp(Node i,Node j) {return i.y>j.y;}
        void work()
        {
            int CNT=0;
            for(int i=0;i<K;i++) {string tmp=items[i];q[CNT].x=tmp; q[CNT].y=new_count[i]; CNT++; }
            sort(q,q+CNT,cmp);
        }
        pair<string ,int> Query(int k)
        {
            return make_pair(q[k].x,q[k].y);
        }
		std::string get_name() override {
			return "ASketch";
		}
};
#endif
