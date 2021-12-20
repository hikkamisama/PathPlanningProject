#include "environmentoptions.h"

EnvironmentOptions::EnvironmentOptions()
{
    searchtype = CN_SP_ST_ASTAR;
    hweight = 1;
    metrictype = CN_SP_MT_EUCL;
    allowsqueeze = false;
    allowdiagonal = true;
    cutcorners = false;
}

EnvironmentOptions::EnvironmentOptions(bool AS, bool AD, bool CC, int MT, int ST, double HW)
{
    metrictype = MT;
    allowsqueeze = AS;
    allowdiagonal = AD;
    cutcorners = CC;
    searchtype = ST;
    hweight = HW;
}

