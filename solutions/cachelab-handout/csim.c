#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#define DEBUG 0

static int s;
static int E;
static int B;
static int hit;
static int miss;
static int eviction;
static int cur_time = 0;

static struct cache{
    unsigned vaild;
    unsigned tag;
    int accessTime;   
}cache[32][32];

void parseOption(int argc, char** argv, char *fileName);
void parseTraceAndExecute(char *fileName);
void exec(unsigned address);

void parseOption(int argc, char** argv, char *fileName) {
    int option;
    while ((option = getopt(argc, argv, "s:E:b:t:")) != -1) {
        switch (option) {
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                B = atoi(optarg);
                break;
            case 't':
                strcpy(fileName, optarg);
                break;
        }
    }
}

void parseTraceAndExecute(char *fileName) {
    freopen(fileName, "r", stdin);
    char cmd;
    unsigned address;
    int blocksize;
    while (~scanf(" %c %x,%d", &cmd, &address, &blocksize)) {
        if (DEBUG) printf("%c, %x\n", cmd, address);
        switch (cmd) {
            case 'L':
                exec(address);
                break;
            case 'M':
                exec(address);
            case 'S':
                exec(address);
                break;
        }
    }
}

void exec(unsigned address) {
    cur_time++;

    unsigned mask = 0xffffffff >> (32 - s);
    unsigned cur_set = (address >> B) & mask;
    unsigned cur_tag = address >> (s + B);

   if (DEBUG) printf ("%x, %x\n", cur_set, cur_tag);

    for (int i = 0; i < E; i++) {
        if (cache[cur_set][i].vaild && cache[cur_set][i].tag == cur_tag) {
            hit++;
            if (DEBUG) printf("hit ++ !\n");
            cache[cur_set][i].accessTime = cur_time;
            return;
        }
    }
    miss++;
    if (DEBUG) printf("miss ++ !\n");
    for (int i = 0; i < E; i++) {
        if (!cache[cur_set][i].vaild) {
            cache[cur_set][i].accessTime = cur_time;
            cache[cur_set][i].vaild = 1;
            cache[cur_set][i].tag = cur_tag;
            return;
        }
    }
    eviction++;
    if (DEBUG) printf("eviction ++ !\n");
    int min_time = cache[cur_set][0].accessTime;
    int min_time_id = 0;
    for (int i = 1; i < E; i++) {
        if (cache[cur_set][i].accessTime < min_time) {
            min_time = cache[cur_set][i].accessTime;
            min_time_id = i;
        }
    }
    cache[cur_set][min_time_id].tag = cur_tag;
    cache[cur_set][min_time_id].accessTime = cur_time;
}

int main(int argc, char** argv) {
    // 读入命令行参数并分解参数
    char *fileName = malloc(30 * sizeof(char));
    parseOption(argc, argv, fileName);

    // 读入trace文件并模拟执行每行
    parseTraceAndExecute(fileName);    

    printSummary(hit, miss, eviction);
    return 0;
}
