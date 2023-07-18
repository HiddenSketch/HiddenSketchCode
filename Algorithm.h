#ifndef _ALGORITHM_H
#define _ALGORITHM_H
#include "FirstStage.h"
#include "SecondStage.h"
#include <deque>
#include <time.h>
enum SketchKind
{
    CM,
    Tower
};
class Algorithm
{
public:
    uint32_t thresh = 210;
    SketchKind kind;
    FirstStage *FS;
    SecondStage *SS;
    uint32_t Mem=0;
    uint32_t total=0;
    uint32_t keylen;
    deque<pair<char *, uint32_t>> heavy_hitters;
    void insert(const char *key, uint32_t keylen);
    uint32_t query(const char *key, uint32_t keylen);
    void detect(double phi);
    void init_CM(uint32_t layer, uint32_t Mem1);
    void init_Tower(uint32_t layer, uint32_t *counterw, uint32_t Mem1);
    void init_SecondStage(pair<uint32_t, uint32_t> *BFpartialkey,
                          uint32_t BFnum,
                          uint32_t *segkl,
                          uint32_t *ws,
                          uint32_t *ls,
                          uint32_t segnum,
                          uint32_t blocknum,
                          uint32_t M,
                          uint32_t keylen);
    Algorithm(){};
};
void Algorithm::init_SecondStage(pair<uint32_t, uint32_t> *BFpartialkey,
                                 uint32_t BFnum,
                                 uint32_t *segkl,
                                 uint32_t *ws,
                                 uint32_t *ls,
                                 uint32_t segnum,
                                 uint32_t blocknum,
                                 uint32_t M,
                                 uint32_t keylen)
{
    SS = new SecondStage(BFpartialkey, BFnum, segkl, ws, ls, segnum, blocknum, M, keylen);
    Mem += SS -> Mem;
    this->keylen = keylen;
}
void Algorithm::init_Tower(uint32_t layer, uint32_t *counterw, uint32_t Mem1) {
    FS = new TowerSketch;
    FS -> init(layer, counterw, Mem1);
    Mem += FS -> memory();
}

void Algorithm::insert(const char* key, uint32_t keylen) {
    uint32_t r1 = FS -> insert(key, keylen);
    total++;
    if (r1 > thresh)
    {
        SS -> insert(key, keylen, 1);
    }
}
uint32_t Algorithm::query(const char* key, uint32_t keylen) {
    uint32_t r1 = FS -> query(key, keylen);
    if (r1 > thresh)
        r1 += SS -> query(key, keylen);
    return r1;
}
void Algorithm::detect(double phi) {
    // auto start = time(NULL);
    deque<pair<char *, uint32_t>> candidate;
    SS -> detect(candidate);
    deque<pair<char*, uint32_t>> candidatenew;
    cout << "candidate.size()  " << candidate.size() << endl;
    fflush(stdout);
    for (auto i : candidate)
        if (FS->query(i.first, keylen) > thresh)
        {
            candidatenew.push_back(i);
        }
        else {
            delete[] i.first;
        }
    candidate.clear();
    cout << "candidate.size()  " << candidate.size() << endl;
    fflush(stdout);
    SS->CM->decode(candidatenew);
    for (auto i : candidatenew)
        if (i.second + thresh > phi * total)
        {
            heavy_hitters.push_back(make_pair(i.first, i.second + thresh));
        }
    auto end = time(NULL);
    // cout << "Decode use " << double(end - start) << " seconds."<< endl;
    return;
}
#endif