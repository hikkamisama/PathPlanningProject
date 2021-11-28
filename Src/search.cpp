#include "search.h"
#include <utility>
#include <limits>
#include <chrono>
#include <iostream>

Search::Search()
{
//set defaults here
}

Search::~Search() {}

bool Search::Condition() {
    return !OPEN.empty();
}

int Search::Optimal() {
    int ind = 0;
    for (int i = 0; i < OPEN.size(); ++i) {
        if (OPEN[ind]->F > OPEN[i]->F) {
            ind = i;
        }
    }
    return ind;
}

int Search::Find(int i, int j) {
    for (int k = 0; k < OPEN.size(); ++k) {
        if (OPEN[k]->i == i && OPEN[k]->j == j) {
            return k;
        }
    }
    return -1;
}

int Search::FindClosed(int i, int j) {
    for (int k = 0; k < CLOSE.size(); ++k) {
        if (CLOSE[k]->i == i && CLOSE[k]->j == j) {
            return k;
        }
    }
    return -1;
}

std::vector<std::pair<int, int>> Search::Successors(Node* curr, const Map &Map, const EnvironmentOptions &options) {
    std::vector<std::pair<int, int>> moves = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};
    std::vector<std::pair<int, int>> res;
    for (auto [di, dj] : moves) {
        if (Map.getValue(curr->i + di, curr->j + dj) == 0) {
            res.emplace_back(curr->i + di, curr->j + dj);
        }
    }
    return res;
}

double Search::Heuristic(int i, int j) {
    return 0;
}

void Search::Update(Node* parent, Node* child, const Map &map) {
    if (child->g > parent->g + map.getCellSize()) {
        child->g = parent->g + map.getCellSize();
        child->F = child->g + child->H;
        child->parent = parent;
    }
}

SearchResult Search::startSearch(ILogger *Logger, const Map &map, const EnvironmentOptions &options)
{
    int nodescreated = 0;
    int numberofsteps = 0;
    auto start_time = std::chrono::steady_clock::now();
    auto [si, sj] = map.GetStartPoint();
    auto [gi, gj] = map.GetGoalPoint();
    Node* start = new Node{
        si, sj, 0, 0, Heuristic(si, sj), nullptr
    };
    OPEN.push_back(start);
    while (Condition()) {
        ++numberofsteps;
        int curr = Optimal();
        //std::cerr << "now looking at " << OPEN[curr]->i << " " << OPEN[curr]->j << "\n";
        if (OPEN[curr]->i == gi && OPEN[curr]->j == gj) {
            //std::cerr << "goal reached\n";
            break;
        }
        for (auto& [succi, succj] : Successors(OPEN[curr], map, options)) {
            ++numberofsteps;
            //std::cerr << "next successor " << succi << " " << succj << "\n";
            int ind = Find(succi, succj);
            if (ind != -1) {
                //std::cerr << "is in open\n";
                Update(OPEN[curr], OPEN[ind], map);
            } else {  
                int indback = FindClosed(succi, succj);
                if (indback == -1) {
                    Node* child = new Node{
                        succi, succj, __INT_MAX__, __INT_MAX__, Heuristic(succi, succj), OPEN[curr]
                    };
                    //std::cerr << "is a new node\n";
                    OPEN.push_back(child);
                    Update(OPEN[curr], OPEN.back(), map);
                } else {
                    //std::cerr << "is in closed\n";
                    Update(OPEN[curr], CLOSE[indback], map);
                }
            }
        }
        CLOSE.push_back(OPEN[curr]);
        OPEN.erase(OPEN.begin() + curr);
    }
    auto end_time = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;
    sresult.time = elapsed.count();
    sresult.numberofsteps = numberofsteps;
    sresult.pathlength = OPEN[Optimal()]->g;
    sresult.nodescreated = OPEN.size() + CLOSE.size();
    if (!OPEN.empty()) {
        sresult.pathfound = true;
        makePrimaryPath(OPEN[Optimal()]);
        makeSecondaryPath();
    }
    for (auto i : OPEN) {
        delete i;
    }
    for (auto i : CLOSE) {
        delete i;
    }
    OPEN.clear();
    CLOSE.clear();
    //need to implement

    /*sresult.pathfound = ; ok
    sresult.nodescreated =  ; ok
    sresult.numberofsteps = ; ok 
    sresult.time = ; ok */
    sresult.hppath = &hppath;
    sresult.lppath = &lppath;
    return sresult;
}

void Search::makePrimaryPath(Node* curNode) {
    lppath.push_front(*curNode);
    auto temp = curNode;
    while (temp->parent) {
        //std::cerr << temp->i << " " << temp->j << std::endl;
        lppath.push_front(*(temp->parent));
        temp = temp->parent;
    }
    lppath.push_front(*(temp));
}

void Search::makeSecondaryPath() {
    
}
