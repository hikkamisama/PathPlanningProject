#ifndef SEARCH_H
#define SEARCH_H
#include "ilogger.h"
#include "searchresult.h"
#include "environmentoptions.h"
#include <list>
#include <vector>
#include <math.h>
#include <limits>
#include <chrono>
#include <unordered_map>
#include <set>

class Search
{
    public:
        Search();
        ~Search(void);
        SearchResult startSearch(ILogger *Logger, const Map &Map, const EnvironmentOptions &options);

    protected:
        struct OPEN {
            std::set<std::pair<double, long long>> data;
            std::unordered_map<long long, Node*> real_open;
            Node*& operator[](long long h) {
                return real_open[h];
            }
            void Erase(long long it) {
                double f = real_open[it]->F;
                real_open.erase(it);
                data.erase({f, it});
                // std::cerr << "erasing hash " << it << " , F = " << f << std::endl;
            }
        } OPEN;

        std::unordered_map<long long, Node*> CLOSE;

        bool Condition();
        long long Optimal();
        std::vector<std::pair<int, int>> Successors(Node* curr, const Map &Map, const EnvironmentOptions &options);
        long long FindOpen(int i, int j, const Map &Map);
        long long FindClosed(int i, int j, const Map &Map);
        void Update(Node* parent, Node* child, const Map &map);
        double Heuristic(int i, int j, const Map &map, const EnvironmentOptions &options);
        void makePrimaryPath(Node* curNode);
        void makeSecondaryPath();
        //CODE HERE

        //Hint 1. You definetely need class variables for OPEN and CLOSE

        //Hint 2. It's a good idea to define a heuristic calculation function, that will simply return 0
        //for non-heuristic search methods like Dijkstra

        //Hint 3. It's a good idea to define function that given a node (and other stuff needed)
        //will return it's sucessors, e.g. unordered list of nodes

        //Hint 4. working with OPEN and CLOSE is the core
        //so think of the data structures that needed to be used, about the wrap-up classes (if needed)
        //Start with very simple (and ineffective) structures like list or vector and make it work first
        //and only then begin enhancement!


        SearchResult                    sresult; //This will store the search result
        std::list<Node>                 lppath, hppath; //

        //CODE HERE to define other members of the class
};
#endif
