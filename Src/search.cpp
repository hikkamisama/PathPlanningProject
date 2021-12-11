#include "search.h"
#include <utility>
#include <limits>
#include <chrono>
#include <iostream>
#include <cmath>
#include <unordered_map>
#include <cassert>

Search::Search()
{
//set defaults here
}

Search::~Search() {}

long long hash_pair(int i, int j, const Map &Map) {
    return (1LL * i * Map.getMapWidth() + j);
}

bool Search::Condition() {
    return !OPEN.data.empty();
}

long long Search::Optimal() {
    return OPEN.data.begin()->second;
}

long long Search::FindOpen(int i, int j, const Map &Map) {
    if (OPEN.real_open.count(hash_pair(i, j, Map))) {
        return hash_pair(i, j, Map);
    }
    return -1;
}

long long Search::FindClosed(int i, int j, const Map &Map) {
    if (CLOSE.count(hash_pair(i, j, Map))) {
        return hash_pair(i, j, Map);
    }
    return -1;
}

std::vector<std::pair<int, int>> Search::Successors(Node* curr, const Map &Map, const EnvironmentOptions &options) {
    std::vector<std::pair<int, int>> moves;
    if (options.allowdiagonal) {
        for (int i = -1; i < 2; ++i) {
            for (int j = -1; j < 2; ++j) {
                if (i || j) {
                    moves.emplace_back(i, j);
                }
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
    // std::cerr << "cost: " << std::fixed << cost << std::endl;
    // std::cerr << parent->g << " " << child->g << std::endl;
    if (child->g > parent->g + cost) {
        auto it = OPEN.data.find({child->F, hash_pair(child->i, child->j, map)});
        assert(it != OPEN.data.end());
        OPEN.data.erase(it);
        child->g = parent->g + cost;
        child->F = child->g + child->H;
        child->parent = parent;
        OPEN.data.insert({child->F, hash_pair(child->i, child->j, map)});
        // std::cerr << "updated " << child->i << " " << child->j << "\n";
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
    ++nodescreated;
    long long st_hash = hash_pair(start->i, start->j, map);
    OPEN.data.insert({start->F, st_hash});
    OPEN[st_hash] = start;
    while (Condition()) {
        ++numberofsteps;
        long long curr = Optimal();
        // std::cerr << "now looking at " << OPEN[curr]->i << " " << OPEN[curr]->j << "\n";
        if (OPEN[curr]->i == gi && OPEN[curr]->j == gj) {
            // std::cerr << "goal reached" << std::endl;
            break;
        }
        for (auto& [succi, succj] : Successors(OPEN[curr], map, options)) {
            ++numberofsteps;
            // std::cerr << "next successor " << succi << " " << succj << "\n";
            long long ind = FindOpen(succi, succj, map);
            if (ind != -1) {
                // std::cerr << "is in open\n";
                Update(OPEN[curr], OPEN[ind], map);
            } else {  
                auto indback = FindClosed(succi, succj, map);
                if (indback == -1) {
                    Node* child = new Node{
                        succi, succj, __INT_MAX__, __INT_MAX__, Heuristic(succi, succj, map, options), OPEN[curr]
                    };
                    ++nodescreated;
                    // std::cerr << "is a new node\n";
                    long long chh = hash_pair(succi, succj, map);
                    // std::cerr << chh << std::endl;
                    OPEN[chh] = child;
                    OPEN.data.insert({__INT_MAX__, chh});
                    Update(OPEN[curr], OPEN[chh], map);
                } // else {
                //     std::cerr << "is in closed\n";
                //     Update(OPEN[curr], CLOSE[indback], map);
                // }
            }
        }
        CLOSE[hash_pair(OPEN[curr]->i, OPEN[curr]->j, map)] = OPEN[curr];
        OPEN.Erase(curr);
    }
    auto end_time = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;
    sresult.time = elapsed.count();
    sresult.numberofsteps = numberofsteps;
    sresult.pathlength = OPEN[Optimal()]->g;
    sresult.nodescreated = nodescreated;
    if (!OPEN.data.empty()) {
        sresult.pathfound = true;
        makePrimaryPath(OPEN[Optimal()]);
        makeSecondaryPath();
    }
    for (auto i : OPEN.real_open) {
        delete i.second;
    }
    OPEN.real_open.clear();
    OPEN.data.clear();
    for (auto i : CLOSE) {
        delete i.second;
    }
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
    for (auto N : lppath) {
        if (hppath.size() >= 2) {
            auto N1 = hppath.back();
            auto N2 = *prev(prev(hppath.end()));
            std::pair<int, int> vec1 = {N1.i - N2.i, N1.j - N2.j};
            std::pair<int, int> vec2 = {N.i - N1.i, N.j - N1.j};
            int prod = vec1.first * vec2.second - vec1.second * vec2.first;
            if (!prod) {
                hppath.pop_back();
            }
        }
        hppath.push_back(N);
    }
}
