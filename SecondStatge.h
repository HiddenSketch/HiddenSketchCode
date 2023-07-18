#ifndef _SECONDSTAGE_H
#define _SECONDSTAGE_H
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <deque>
#include <queue>
#include <iostream>
#include "./utils/BOBHash32.h"
using namespace std;
char *partial(const char *key, uint32_t keylen, pair<uint32_t, uint32_t> startend)
{
    assert(startend.second > startend.first);
    assert(startend.second <= keylen * 8);
    char *partialkey = new char[keylen];
    memset(partialkey, 0, keylen);
    for (uint32_t i = startend.first, j = 0; i < startend.second; i++, j++)
    {
        if (key[i / 8] & (((unsigned char)1U) << (i % 8)))
            partialkey[j / 8] |= (((unsigned char)1U) << (j % 8));
    }
    return partialkey;
}
char *mix(const char *key1, const char *key2, uint32_t len1, uint32_t len2, uint32_t keylen)
{
    char *mixkey = new char[keylen];
    memset(mixkey, 0, keylen);
    for (uint32_t i = 0; i < len1; i++)
    {
        if (key1[i / 8] & (((unsigned char)1U) << (i % 8)))
            mixkey[i / 8] |= (((unsigned char)1U) << (i % 8));
    }
    for (uint32_t i = 0; i < len2; i++)
    {
        if (key2[i / 8] & (((unsigned char)1U) << (i % 8)))
            mixkey[(i + len1) / 8] |= (((unsigned char)1U) << ((i + len1) % 8));
    }
    return mixkey;
}

void printkey(const char *key, uint32_t keylen)
{
    for (uint32_t j = 0; j < keylen; j++)
        printf("%02x ", uint8_t(key[j]));
    cout << endl;
}
struct extTreeNode
{
    uint32_t index;
    uint32_t start;
    uint32_t end;
    deque<pair<char *, uint32_t>> *candidate;
    extTreeNode *Firstchildren;
    extTreeNode *Sibling;
    extTreeNode *Parent;
    ~extTreeNode()
    {
        if (Firstchildren != NULL)
            delete Firstchildren;
        if (Sibling != NULL)
            delete Sibling;
        delete candidate;
    }
};

void insert(pair<uint32_t, uint32_t> startend, extTreeNode *root, uint32_t index)
{
    extTreeNode *now = root;
    extTreeNode *newNode = new extTreeNode;
    newNode->start = startend.first;
    newNode->end = startend.second;
    newNode->index = index;
    newNode->candidate = new deque<pair<char *, uint32_t>>;
    newNode->Firstchildren = NULL;
    newNode->Sibling = NULL;
    while (now->start <= startend.first && now->end >= startend.second)
    {
        extTreeNode *tmp = now->Firstchildren;
        extTreeNode *tt = NULL;
        while (tmp && !(tmp->start <= startend.first && tmp->end >= startend.second))
        {
            tt = tmp;
            tmp = tmp->Sibling;
        }
        if (tmp && tmp->start <= startend.first && tmp->end >= startend.second)
        {
            now = tmp;
        }
        else
        {
            if (tt)
            {
                tt->Sibling = newNode;
                newNode->Parent = now;
            }
            else
            {
                now->Firstchildren = newNode;
                newNode->Parent = now;
            }
            break;
        }
    }
}
void printTree(extTreeNode *root)
{
    if (root == NULL)
        return;
    cout << root->index << "  " << root->start << " " << root->end << endl;
    extTreeNode *head = root->Firstchildren;
    while (head != NULL)
    {
        printTree(head);
        head = head->Sibling;
    }
}
extTreeNode *establishTree(pair<uint32_t, uint32_t> *BFpartialkey, uint32_t BFnum)
{
    extTreeNode *root = new extTreeNode;
    root->candidate = new deque<pair<char *, uint32_t>>;
    root->index = 0;
    root->start = BFpartialkey[0].first;
    root->end = BFpartialkey[0].second;
    root->Parent = NULL;
    root->Sibling = NULL;
    root->Firstchildren = NULL;
    extTreeNode *nowparent = root;
    extTreeNode *nowbrother = NULL;
    for (uint32_t i = 1; i < BFnum; i++)
    {
        // extTreeNode *newNode = new extTreeNode;
        // newNode->start = BFpartialkey[i].first;
        // newNode->end = BFpartialkey[i].second;
        // newNode->index = i;
        // newNode->Sibling = NULL;
        // newNode->Firstchildren = NULL;
        // newNode->candidate = new deque<pair<char *, uint32_t>>;
        // if (nowbrother == NULL)
        // {
        //     nowparent->Firstchildren = newNode;
        //     nowbrother = newNode;
        //     newNode->Parent = nowparent;
        // }
        // if (newNode->start >= nowbrother->end)
        // {
        //     nowbrother = newNode;
        //     while (nowparent->end < newNode->end)
        //     {
        //         nowparent = nowparent->Parent;
        //     }
        // }
        // else
        // {
        //     newNode->Parent = nowparent;
        //     nowbrother->Sibling = newNode;
        // }
        insert(BFpartialkey[i], root, i);
        // printTree(root);
        // cout << "node" << newNode->start << "  " << newNode->end << endl;
        // cout << "parent" << newNode->Parent->start << "  " << newNode->Parent->end << endl;
        // cout << endl;
    }
    return root;
}

class SegBitmap
{
public:
    uint32_t **bitmaps;
    uint32_t *segkl;
    uint32_t *segbmw;
    uint32_t segsnum;
    uint32_t Mem;
    uint32_t insert(const char *key, uint32_t keylen);
    SegBitmap(uint32_t *segkl, uint32_t segnum);
    SegBitmap();
};

class BloomFilter
{
public:
    uint32_t **bits;
    BOBHash32 *hashes;
    uint32_t layer;
    uint32_t width;
    uint32_t Mem;
    uint32_t insert(const char *key, uint32_t keylen);
    uint32_t query(const char *key, uint32_t keylen);
    BloomFilter(uint32_t w, uint32_t l);
    BloomFilter(){};
};

class MiniCM
{
public:
    uint32_t **counters;
    BOBHash32 *hashes;
    uint32_t layer = 3;
    uint32_t Mem;
    uint32_t width;
    uint32_t keylen;
    MiniCM(uint32_t M, uint32_t keylen);
    uint32_t insert(const char *key, uint32_t keylen, uint32_t size);
    uint32_t query(const char *key, uint32_t keylen);
    void decode(deque<pair<char *, uint32_t>> &candidate);
};

class SecondStage
{
public:
    BOBHash32 *hash;
    SegBitmap **SBB;
    uint32_t *segkl;
    uint32_t segnum;
    uint32_t blocknum;
    BloomFilter **BF;
    uint32_t *BFwidths;
    uint32_t *BFlayers;
    uint32_t BFnum;
    pair<uint32_t, uint32_t> *BFpartialkey;
    MiniCM *CM;
    uint32_t Mem;
    uint32_t keylen;

public:
    uint32_t insert(const char *key, uint32_t keylen, uint32_t size);
    uint32_t query(const char *key, uint32_t keylen);
    void detect(deque<pair<char *, uint32_t>> &candidate);
    void extend(extTreeNode *root, uint32_t SBBindex);
    void cartesian(deque<pair<char *, uint32_t>> &c1, deque<pair<char *, uint32_t>> &c2, deque<pair<char *, uint32_t>> &candidate, uint32_t len1, uint32_t len2);
    SecondStage(pair<uint32_t, uint32_t> *BFpartialkey,
                uint32_t BFnum,
                uint32_t *segkl,
                uint32_t *ws,
                uint32_t *ls,
                uint32_t segnum,
                uint32_t blocknum,
                uint32_t M,
                uint32_t keylen);
};

MiniCM::MiniCM(uint32_t M, uint32_t keylen)
{
    this->keylen = keylen;
    Mem = M;
    layer = 3;
    width = Mem / layer / 4;
    counters = new uint32_t *[layer];
    hashes = new BOBHash32[layer];
    uint32_t seed = random() % 996;
    for (uint32_t i = 0; i < layer; i++)
    {
        counters[i] = new uint32_t[width];
        memset(counters[i], 0, width * sizeof(uint32_t));
        hashes[i].initialize(seed + i);
    }
}

uint32_t MiniCM::insert(const char *key, uint32_t keylen, uint32_t size)
{
    uint32_t ret = UINT32_MAX;
    for (uint32_t i = 0; i < layer; i++)
    {
        uint32_t pos = hashes[i].run(key, keylen) % width;
        counters[i][pos] += size;
        ret = min(ret, counters[i][pos]);
    }
    return ret;
}

uint32_t MiniCM::query(const char *key, uint32_t keylen)
{
    uint32_t ret = UINT32_MAX;
    for (uint32_t i = 0; i < layer; i++)
    {
        uint32_t pos = hashes[i].run(key, keylen) % width;
        ret = min(ret, counters[i][pos]);
    }
    return ret;
}

void MiniCM::decode(deque<pair<char *, uint32_t>> &candidate)
{
    cout << "decoding " << candidate.size() << endl;
    deque<pair<char *, uint32_t>> **bucketkeys = new deque<pair<char *, uint32_t>> *[layer];
    for (uint32_t i = 0; i < layer; i++)
        bucketkeys[i] = new deque<pair<char *, uint32_t>>[width];
    for (uint32_t i = 0; i < candidate.size(); i++)
    {
        for (uint32_t j = 0; j < layer; j++)
        {
            bucketkeys[j][hashes[j].run(candidate[i].first, keylen) % width].push_back(make_pair(candidate[i].first, i));
        }
    }
    queue<pair<uint32_t, uint32_t>> pure_que;
    for (uint32_t i = 0; i < layer; i++)
        for (uint32_t j = 0; j < width; j++)
        {
            if (bucketkeys[i][j].size() == 1 || counters[i][j] == 0)
            {
                pure_que.push(make_pair(i, j));
            }
        }
    while (!pure_que.empty())
    {
        auto bucket = pure_que.front();
        pure_que.pop();
        if (bucketkeys[bucket.first][bucket.second].size() == 0)
            continue;
        if (counters[bucket.first][bucket.second] == 0)
        {
            for (auto key : bucketkeys[bucket.first][bucket.second])
            {
                for (uint32_t i = 0; i < layer; i++)
                {
                    uint32_t pos = hashes[i].run(key.first, keylen) % width;
                    for (auto j = bucketkeys[i][pos].begin(); j != bucketkeys[i][pos].end(); j++)
                    {
                        if (j->first == key.first)
                        {
                            bucketkeys[i][pos].erase(j);
                            break;
                        }
                    }
                    if (bucketkeys[i][pos].size() == 1 && i != bucket.first)
                        pure_que.push(make_pair(i, pos));
                }
            }
        }
        uint32_t size = counters[bucket.first][bucket.second];
        char *key = bucketkeys[bucket.first][bucket.second][0].first;
        uint32_t index = bucketkeys[bucket.first][bucket.second][0].second;
        candidate[index].second = size;
        for (uint32_t i = 0; i < layer; i++)
        {
            uint32_t pos = hashes[i].run(key, keylen) % width;
            counters[i][pos] -= size;
            for (auto j = bucketkeys[i][pos].begin(); j != bucketkeys[i][pos].end(); j++)
            {
                if (j->first == key)
                {
                    bucketkeys[i][pos].erase(j);
                    break;
                }
            }
            if ((bucketkeys[i][pos].size() == 1 || counters[i][pos] == 0) && i != bucket.first)
                pure_que.push(make_pair(i, pos));
        }
    }
    for (uint32_t i = 0; i < layer; i++)
        for (uint32_t j = 0; j < width; j++)
        {
            if (counters[i][j] != 0)
            {
                printf("decode not ok\n");
                return;
            }
        }
}

uint32_t BloomFilter::insert(const char *key, uint32_t keylen)
{
    uint32_t ret = 1;
// #pragma omp parallel for
    for (uint32_t i = 0; i < layer; i++)
    {
        uint32_t pos = hashes[i].run(key, keylen) % width;
        ret = min(ret, bits[i][pos]);
        bits[i][pos] = 1;
    }
    return ret;
}

uint32_t BloomFilter::query(const char *key, uint32_t keylen)
{
    uint32_t ret = 1;
// #pragma omp parallel for
    for (uint32_t i = 0; i < layer; i++)
    {
        uint32_t pos = hashes[i].run(key, keylen) % width;
        if (bits[i][pos] == 0)
            ret = 0;
    }
    return ret;
}

uint32_t SecondStage::insert(const char *key, uint32_t keylen, uint32_t size)
{
    SBB[hash->run(key, keylen) % blocknum]->insert(key, keylen);
    for (uint32_t i = 0; i < BFnum; i++)
    {
        char *partialkey = partial(key, keylen, BFpartialkey[i]);
        BF[i]->insert(partialkey, keylen);
        delete[] partialkey;
    }
    return CM->insert(key, keylen, size);
}

uint32_t SecondStage::query(const char *key, uint32_t keylen)
{
    return CM->query(key, keylen);
}

uint32_t SegBitmap::insert(const char *key, uint32_t keylen)
{
    uint32_t ret = 0;
    uint32_t start = 0, end = segkl[0];
    for (uint32_t i = 0; i < segsnum; i++)
    {
        char *partialkey = partial(key, keylen, make_pair(start, end));
        uint32_t offset = *(uint32_t *)partialkey;
        ret = min(ret, bitmaps[i][offset]);
        bitmaps[i][offset] = 1;
        start = end;
        end += segkl[i + 1];
        delete[] partialkey;
    }
    return ret;
}

SegBitmap::SegBitmap(uint32_t *segkl, uint32_t segnum)
{
    this->segkl = new uint32_t[segnum];
    this->segbmw = new uint32_t[segnum];
    this->bitmaps = new uint32_t *[segnum];
    this->segsnum = segnum;
    this->Mem = 0;
    for (uint32_t i = 0; i < segnum; i++)
    {
        this->segkl[i] = segkl[i];
        this->segbmw[i] = 1 << segkl[i];
        this->Mem += this->segbmw[i];
        this->bitmaps[i] = new uint32_t[1 << segkl[i]];
        memset(this->bitmaps[i], 0, sizeof(uint32_t) * (1 << segkl[i]));
    }
    this->Mem /= 8;
}

BloomFilter::BloomFilter(uint32_t w, uint32_t l)
{
    layer = l;
    width = w;
    bits = new uint32_t *[l];
    hashes = new BOBHash32[l];
    uint32_t seed = random() % 997;
    for (uint32_t i = 0; i < l; i++)
    {
        hashes[i].initialize(seed + i);
        bits[i] = new uint32_t[width];
        memset(bits[i], 0, width * sizeof(uint32_t));
    }
    Mem = l * w / 8;
}

SecondStage::SecondStage(pair<uint32_t, uint32_t> *BFpartialkey,
                         uint32_t BFnum,
                         uint32_t *segkl,
                         uint32_t *ws,
                         uint32_t *ls,
                         uint32_t segnum,
                         uint32_t blocknum,
                         uint32_t M,
                         uint32_t keylen)
{
    this->blocknum = blocknum;
    this->segnum = segnum;
    this->BFnum = BFnum;
    this->keylen = keylen;
    hash = new BOBHash32;
    hash->initialize(random() % 1001);
    this->segkl = new uint32_t[segnum];
    this->BFwidths = new uint32_t[BFnum];
    this->BFlayers = new uint32_t[BFnum];
    this->BFpartialkey = new pair<uint32_t, uint32_t>[BFnum];
    this->Mem = M;
    CM = new MiniCM(M, keylen);
    for (uint32_t i = 0; i < segnum; i++)
        this->segkl[i] = segkl[i];
    this->BF = new BloomFilter *[BFnum];
    for (uint32_t i = 0; i < BFnum; i++)
    {
        this->BFwidths[i] = ws[i];
        this->BFlayers[i] = ls[i];
        this->BFpartialkey[i] = make_pair(BFpartialkey[i].first, BFpartialkey[i].second);
        this->BF[i] = new BloomFilter(this->BFwidths[i], this->BFlayers[i]);
        Mem += (this->BF[i])->Mem;
    }
    SBB = new SegBitmap *[blocknum];
    for (uint32_t i = 0; i < blocknum; i++)
    {
        this->SBB[i] = new SegBitmap(this->segkl, this->segnum);
        Mem += (this->SBB[i])->Mem;
    }
}

void SecondStage::detect(deque<pair<char *, uint32_t>> &candidate)
{
    // #pragma omp parallel for
    for (uint32_t i = 0; i < blocknum; i++)
    {
        extTreeNode *root = establishTree(BFpartialkey, BFnum);
        extend(root, i);
        for (auto j : *root->candidate)
        {
            if (hash->run(j.first, keylen) % blocknum == i)
                candidate.push_back(j);
            else
            {
                delete[] j.first;
            }
        }
        // cout << candidate.size() << endl;
        delete root;
    }
    return;
}

void SecondStage::extend(extTreeNode *root, uint32_t SBBindex)
{
    extTreeNode *child = root->Firstchildren;
    uint32_t start = root->start;
    uint32_t end = root->start;
    uint32_t childnum = 0;

    while (child)
    {
        childnum++;
        extend(child, SBBindex);
        end = child->end;
        child = child->Sibling;
    }
    deque<pair<char *, uint32_t>> cand1, cand2;
    if (childnum != 0)
    {
        deque<pair<char *, uint32_t>> *cand = new deque<pair<char *, uint32_t>>[childnum];
        child = root->Firstchildren;
        for (auto i : *child->candidate)
        {
            cand[0].push_back(i);
        }
        uint32_t len = child->end - child->start;
        child = child->Sibling;
        uint32_t i = 1;
        while (child)
        {
            cartesian(cand[i - 1], *child->candidate, cand[i], len, child->end - child->start);
            child = child->Sibling;
            i++;
        }
        for (auto i : cand[childnum - 1])
            cand1.push_back(i);
    }
    else
    {
        char *key = new char[keylen];
        cand1.push_back(make_pair(key, 0));
    }
    if (end < root->end)
    {
        uint32_t layer1 = 0, sum = 0;
        for (; sum < end; layer1++)
        {
            sum += segkl[layer1];
        }
        uint32_t layer2 = layer1;
        for (; sum < root->end; layer2++)
        {
            sum += segkl[layer2];
        }
        char *key = new char[keylen];
        uint32_t len1 = 0, len2;
        deque<pair<char *, uint32_t>> *c = new deque<pair<char *, uint32_t>>[layer2 - layer1];
        deque<pair<char *, uint32_t>> *_c = new deque<pair<char *, uint32_t>>[layer2 - layer1];

        for (uint32_t i = layer1; i < layer2; i++)
        {
            for (uint32_t j = 0; j < (1 << segkl[i]); j++)
            {
                if (SBB[SBBindex]->bitmaps[i][j])
                {
                    char *tkey = new char[keylen];
                    *(uint32_t *)tkey = j;
                    c[i - layer1].push_back(make_pair(tkey, 0));
                }
            }
        }
        for (auto i : c[0])
        {
            _c[0].push_back(i);
        }
        for (uint32_t i = layer1 + 1; i < layer2; i++)
        {
            len1 += segkl[i - 1];
            len2 = segkl[i];
            cartesian(_c[i - layer1 - 1], c[i - layer1], _c[i - layer1], len1, len2);
        }
        for (auto i : _c[layer2 - layer1 - 1])
            cand2.push_back(i);
    }
    else
    {
        char *key = new char[keylen];
        cand2.push_back(make_pair(key, 0));
    }
    deque<pair<char *, uint32_t>> candi;
    cartesian(cand1, cand2, candi, end - root->start, root->end - end);

// #pragma omp parallel for
    for (auto i : candi)
    {
        if (BF[root->index]->query(i.first, keylen))
        {
            root->candidate->push_back(i);
        }
        else
        {
            delete[] i.first;
        }
    }
    // cout << root->candidate->size() << endl;
    // for (auto i : *root->candidate)
    //     printkey(i.first, keylen);
}

void SecondStage::cartesian(deque<pair<char *, uint32_t>> &c1, deque<pair<char *, uint32_t>> &c2, deque<pair<char *, uint32_t>> &candidate, uint32_t len1, uint32_t len2)
{
    for (auto i : c1)
        for (auto j : c2)
        {
            char *mixkey = mix(i.first, j.first, len1, len2, keylen);
            candidate.push_back(make_pair(mixkey, 0));
        }
}

#endif