#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include <algorithm>
#include <deque>
#include <string>
#include <vector>
using namespace std;

// ── Constants (match main.cpp) ────────────────────────────────────────────────
const int   COLS=20, ROWS=13, LB_MAX=5, APL=5;
const float BASE=0.140f, STEP=0.012f, MINSPD=0.055f;

// ── Minimal types ─────────────────────────────────────────────────────────────
struct Entry { string name; int score; };
int                  level = 1;
deque<pair<int,int>> snakePos;
vector<Entry>        lb;

// ── Logic (mirrors main.cpp) ──────────────────────────────────────────────────
bool hasSnake(pair<int,int> p) {
    for (auto& s : snakePos) if (s==p) return true; return false;
}
float delay() { return max(BASE-(level-1)*STEP, MINSPD); }
void  addLB(const string& nm, int sc) {
    lb.push_back({nm,sc});
    sort(lb.begin(),lb.end(),[](auto&a,auto&b){return a.score>b.score;});
    if ((int)lb.size()>LB_MAX) lb.resize(LB_MAX);
}
int calcLevel(int a) { return a/APL+1; }
int calcScore(int s, int l) { return s+10*l; }

// ── Tests ─────────────────────────────────────────────────────────────────────
TEST_CASE("hasSnake — positive: head and body") {
    snakePos = {{5,5},{4,5},{3,5}};
    CHECK(hasSnake({5,5}));
    CHECK(hasSnake({4,5}));
    CHECK(hasSnake({3,5}));
}
TEST_CASE("hasSnake — negative: empty and unoccupied") {
    snakePos.clear();
    CHECK_FALSE(hasSnake({5,5}));
    snakePos = {{5,5},{4,5}};
    CHECK_FALSE(hasSnake({0,0}));
    CHECK_FALSE(hasSnake({5,6}));
    CHECK_FALSE(hasSnake({6,5}));
}
TEST_CASE("delay — level 1 is base") {
    level=1; CHECK(delay()==doctest::Approx(0.140f));
}
TEST_CASE("delay — decreases with level") {
    level=2; CHECK(delay()==doctest::Approx(0.128f));
    level=3; CHECK(delay()==doctest::Approx(0.116f));
}
TEST_CASE("delay — clamped at high level") {
    level=100; CHECK(delay()==doctest::Approx(MINSPD));
}
TEST_CASE("addLB — sorted descending") {
    lb.clear();
    addLB("Alice",50); addLB("Bob",200); addLB("Carol",100);
    CHECK(lb[0].name=="Bob"); CHECK(lb[1].name=="Carol"); CHECK(lb[2].name=="Alice");
}
TEST_CASE("addLB — capped at LB_MAX") {
    lb.clear();
    for (int i=0;i<LB_MAX+3;++i) addLB("P"+to_string(i),i*10);
    CHECK((int)lb.size()==LB_MAX);
}
TEST_CASE("addLB — low score dropped from full board") {
    lb.clear();
    for (int i=0;i<LB_MAX;++i) addLB("Top"+to_string(i),1000+i*10);
    addLB("Loser",1);
    CHECK((int)lb.size()==LB_MAX);
    for (auto& e:lb) CHECK(e.name!="Loser");
}
TEST_CASE("calcLevel — advances every APL apples") {
    CHECK(calcLevel(0)==1); CHECK(calcLevel(4)==1);
    CHECK(calcLevel(5)==2); CHECK(calcLevel(9)==2);
    CHECK(calcLevel(10)==3);
}
TEST_CASE("calcScore — multiplied by level") {
    CHECK(calcScore(0,1)==10);
    CHECK(calcScore(10,2)==30);
    CHECK(calcScore(30,3)==60);
}
