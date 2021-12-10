#include "search.h"
#include <utility>
#include <limits>
#include <chrono>
#include <iostream>
#include <cmath>

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
    std::vector<std::pair<int, int>> moves;
    if (options.allowdiagonal) {
        for (int i = -1; i < 2; ++i) {
            for (int j = -1; j < 2; ++j) {
                moves.emplace_back(i, j);
            }
        }
    } else {
        moves = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};
    }
    std::vector<std::pair<int, int>> res;
    for (auto [di, dj] : moves) {
        int succi = curr->i + di;
        int succj = curr->j + dj;
        if (Map.getValue(succi, succj)) {
            continue;
        }
        if (std::abs(di) + std::abs(dj) == 2) {
            if (Map.getValue(curr->i + di, curr->j) && Map.getValue(curr->i, curr->j + dj)) {
                if (options.allowsqueeze) {
                    res.emplace_back(succi, succj);
                }
            } else if (Map.getValue(curr->i + di, curr->j) || Map.getValue(curr->i, curr->j + dj)) {
                if (options.cutcorners) {
                    res.emplace_back(succi, succj);
                }
            } else {
                res.emplace_back(succi, succj);
            }
        } else {
            res.emplace_back(succi, succj);
        }
    }
    return res;
}

double Search::Heuristic(int i, int j, const Map &map, const EnvironmentOptions &options) {
    auto [gi, gj] = map.GetGoalPoint();
    int dx = abs(gi - i);
    int dy = abs(gj - j);
    if (options.metrictype == 0) { // diag
        return std::abs(dx - dy) + std::min(dx, dy);
    } else if (options.metrictype == 1) { // manh
        return dx + dy;
    } else if (options.metrictype == 2) { // eucl
        return std::pow(dx * dx + dy * dy, 0.5);
    } else if (options.metrictype == 3) { // cheb
        return std::max(dx, dy);
    }
    return 0;
}

void Search::Update(Node* parent, Node* child, const Map &map) {
    double cost = 1;
    int dx = abs(parent->i - child->i);
    int dy = abs(parent->j - child->j);
    if (dx + dy == 2) {
        cost *= CN_SQRT_TWO;
    }
    if (child->g > parent->g + cost) {
        child->g = parent->g + cost;
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
        si, sj, 0, 0, Heuristic(si, sj, map, options), nullptr
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
                        succi, succj, __INT_MAX__, __INT_MAX__, Heuristic(succi, succj, map, options), OPEN[curr]
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
    sresult.hppath = &hppath;
    sresult.lppath = &lppath;
    return sresult;
}

void Search::makePrimaryPath(Node* curNode) {
    lppath.push_front(*curNode);
    auto temp = curNode;
    while (temp->parent) {
        lppath.push_front(*(temp->parent));
        temp = temp->parent;
    }
    lppath.push_front(*(temp));
}

void Search::makeSecondaryPath() {
    
}
