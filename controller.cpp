#define KEYLEN 4
#include "Algorithm.h"
#include <iostream>
#include <map>
#include <unordered_map>
#include <deque>
#include <string.h>
#include <stdio.h>
#define WINDOWNUM 12
#define TOT_MEM 55
using namespace std;
int total = 0;
struct keys_t
{
    unsigned char key[KEYLEN];
    keys_t operator=(keys_t &tmp)
    {
        for (uint32_t i = 0; i < KEYLEN; i++)
            key[i] = tmp.key[i];
        return *this;
    }
};
void read_trace(const char *filename, deque<keys_t> *keys)
{
    double starttime, nowtime;
    FILE *f = fopen(filename, "r");
    char tmp[21];
    fread(tmp, 21, 1, f);
    for (int w = 0; w < WINDOWNUM; w++)
    {
        starttime = *(double *)(tmp + 13);
        keys_t key;
        memcpy(&key, tmp, KEYLEN);
        keys[w].push_back(key);
        while (true)
        {
            if (!fread(tmp, 21, 1, f))
                break;
            nowtime = *(double *)(tmp + 13);
            if (nowtime - starttime > 5)
            {
                break;
            }
            else
            {
                memcpy(&key, tmp, KEYLEN);
                keys[w].push_back(key);
            }
        }
    }
}
bool operator<(keys_t a, keys_t b)
{
    for (uint32_t i = 0; i < KEYLEN; i++)
        if (a.key[i] < b.key[i])
            return true;
        else if (a.key[i] > b.key[i])
            return false;
    return false;
}

bool operator==(keys_t a, keys_t b)
{
    for (uint32_t i = 0; i < KEYLEN; i++)
        if (a.key[i] != b.key[i])
            return false;
    return true;
}

int main()
{
    uint32_t keylen = KEYLEN;
    deque<keys_t> *keys = new deque<keys_t>[WINDOWNUM];
    read_trace("/root/data/130000.dat", keys);
    uint32_t *counterw = new uint32_t[5];
    uint32_t BFnum = 3;
    uint32_t segnum = 4;
    uint32_t M = 10 * 1024;
    uint32_t blocknum = 16;
    pair<uint32_t, uint32_t> *BFpartialkey = new pair<uint32_t, uint32_t>[BFnum];
    // BFpartialkey[0] = {0, 104};
    // BFpartialkey[1] = {0, 56};
    // BFpartialkey[2] = {56, 104};
    // BFpartialkey[3] = {0, 33};
    // BFpartialkey[4] = {33, 56};
    // BFpartialkey[5] = {56, 80};
    // BFpartialkey[6] = {80, 104};
    // BFpartialkey[7] = {0, 22};


    // BFpartialkey[0] = {0, 64};
    // BFpartialkey[1] = {0, 32};
    // BFpartialkey[2] = {0, 16};
    // BFpartialkey[3] = {16, 32};
    // BFpartialkey[4] = {32, 64};
    // BFpartialkey[5] = {32, 48};
    // BFpartialkey[6] = {48, 64};

    BFpartialkey[0] = {0, 32};
    BFpartialkey[1] = {0, 16};
    BFpartialkey[2] = {16, 32};
    counterw[0] = 8;
    counterw[1] = 8;
    counterw[2] = 8;
    counterw[3] = 8;
    counterw[4] = 8;
    uint32_t ws[8] = {2200, 2200, 2200, 2200, 2200, 2200, 2200, 2200};
    // uint32_t ls[8] = {10, 12, 12, 12, 12, 12, 12, 12};
    uint32_t ls[8] = {7, 7, 7, 7, 7, 7, 7, 7};

    // uint32_t segkl[9] = {11, 11, 11, 11, 12, 12, 12, 12, 12};
    uint32_t segkl[8] = {8, 8, 8, 8, 8, 8, 8, 8};
    double avg = 0;
    double avg_ARE = 0;
    for (int window = 0; window < WINDOWNUM; window++)
    {
        Algorithm *algo = new Algorithm();
        algo->init_SecondStage(
            BFpartialkey,
            BFnum,
            segkl,
            ws,
            ls,
            segnum,
            blocknum,
            M,
            keylen);
        algo->init_Tower(3, counterw, TOT_MEM * 1024 - algo->Mem);
        algo->thresh = 0.0001 * keys[window].size() - 10;
        map<keys_t, uint32_t> true_result;
        map<keys_t, uint32_t> est_result;
        for (auto i : keys[window])
        {
            algo->insert((const char *)&i, keylen);
            true_result[i]++;
        }
        algo->detect(0.0001);
        for (auto i : algo->heavy_hitters)
        {
            keys_t tmp;
            for (uint32_t j = 0; j < keylen; j++)
            {
                tmp.key[j] = i.first[j];
            }
            delete[] i.first;
            est_result[tmp] = i.second;
        }
        for (auto i : true_result)
        {
            if (i.second <= 0.0001 * keys[window].size())
                continue;
        }
        double err = 0;
        double Pre = 0, Rec = 0;
        double total = 0;
        uint32_t true_count = 0;
        for (auto i : true_result) {
            if (i.second <= 0.0001 * keys[window].size())
                continue;
            else if (est_result.find(i.first) != est_result.end())
            {
                Pre += 1;
                total += 1;
            }
            else {
                total += 1;
            }
            true_count++;
            err += abs(double(i.second) - double(est_result[i.first])) / i.second;
        }
        avg_ARE += err / total;
        Pre /= total;
        total = 0;
        for (auto i : est_result) {
            if (true_result[i.first] > 0.0001 * keys[window].size())
            {
                Rec += 1;
                total += 1;
            }
            else
                total += 1;
        }
        Rec /= total;
        double F1_score = 2 * Pre * Rec / (Pre + Rec);
        printf("[WINDOW %2d]\n", window);
        cout << "total: " << keys[window].size() << endl;
        cout << "Use Memory  " << algo->Mem / 1024 << "KB" << endl;
        cout << "Pre: " << Pre << endl;
        cout << "Rec: " << Rec << endl;
        cout << "F1_score:  " << F1_score << endl;
        cout << "*************************" << endl;
        avg += F1_score;
        delete algo;
    }
    cout << "Average F1_score: " << avg / WINDOWNUM << endl;
    cout << "Average ARE: " << avg_ARE / WINDOWNUM << endl;
    return 0;
}