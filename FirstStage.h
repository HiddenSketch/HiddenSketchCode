#ifndef _FIRSTSTAGE_H
#define _FIRSTSTAGE_H
#include <stdint.h>
#include <cstdlib>
#include <iostream>
#include "./utils/BOBHash32.h"
using namespace std;
class FirstStage {
public:
    virtual uint32_t insert(const char* key, uint32_t keylen) = 0;
    virtual uint32_t query(const char* key, uint32_t keylen) = 0;
    virtual uint32_t memory() = 0;
    virtual void init(uint32_t layer, uint32_t* counterw, uint32_t Mem) {};
};

class TowerSketch: public FirstStage {
public:
    uint32_t** counters;
    uint32_t* counterw;
    uint32_t* arrayw;
    uint32_t layer;
    uint32_t Mem;
    BOBHash32* hashes;
    uint32_t* seed;
    void init(uint32_t layer, uint32_t* counterw, uint32_t Mem);
    ~TowerSketch();
    virtual uint32_t insert(const char* key, uint32_t keylen);
    virtual uint32_t query(const char* key, uint32_t keylen);
    virtual uint32_t memory();
};

// class CMSketch: public FirstStage {
// public:
//     uint32_t** counters;
//     uint32_t layer;
//     uint32_t width;
//     uint32_t Mem;
//     BOBHash32* hashes;
//     void init(uint32_t layer, uint32_t Mem);
//     ~CMSketch();
//     virtual uint32_t insert(const char* key, uint32_t keylen);
//     virtual uint32_t query(const char* key, uint32_t keylen);
//     virtual uint32_t memory();
// };

void TowerSketch::init(uint32_t layer, uint32_t* counterw, uint32_t Mem) {
    this -> counters = new uint32_t*[layer];
    this -> layer = layer;
    this -> counterw = new uint32_t[layer];
    this -> arrayw = new uint32_t[layer];
    this -> Mem = Mem;
    this -> hashes = new BOBHash32[layer];
    this -> seed = new uint32_t[layer];
    for (uint32_t i = 0; i < layer; i++) {
        seed[i] = rand() % 1000;
        hashes[i].initialize(seed[i]);
        this -> counterw[i] = counterw[i];
        this -> arrayw[i] = Mem * 8 / layer / counterw[i];
        counters[i] = new uint32_t[arrayw[i]];
        for (uint32_t j = 0; j < arrayw[i]; j++)
            counters[i][j] = 0;
    }
}
TowerSketch::~TowerSketch() {
    for (uint32_t i = 0; i < layer; i++) {
        delete[] counters[i];
    }
    delete[] counters;
    delete[] counterw;
    delete[] arrayw;
    delete[] hashes;
    delete[] seed;
}
uint32_t TowerSketch::insert(const char* key, uint32_t keylen) {
    uint32_t ret = UINT32_MAX;
    for (uint32_t i = 0; i < layer; i++)
    {
        uint32_t pos = hashes[i].run(key, keylen) % arrayw[i];
        if (counters[i][pos] != (1U << (counterw[i])) - 1)
            ret = min(++counters[i][pos], ret);
    }
    return ret;
}

uint32_t TowerSketch::query(const char* key, uint32_t keylen) {
    uint32_t ret = UINT32_MAX;
    for (uint32_t i = 0; i < layer; i++)
    {
        uint32_t pos = hashes[i].run(key, keylen) % arrayw[i];
        if (counters[i][pos] != (1U << (counterw[i])) - 1)
            ret = min(counters[i][pos], ret);
    }
    return ret;
}

uint32_t TowerSketch::memory() {
    return Mem;
}
#endif 