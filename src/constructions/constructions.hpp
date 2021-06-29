#ifndef CONSTRUCTIONS_H
#define CONSTRUCTIONS_H

#include <stack>
#include <iostream>
#include <limits>
#include <string>
#include <string.h>
#include "../solution/solution.hpp"
#include "../instance/instance.hpp"
#include "../insertions/insertions.hpp"

using namespace std;

Solution dfs_construct(Instance *instance, std::default_random_engine *engine); /* returns solution object from passed instance using depth-first search */
Solution dfs_optimum_construct(Instance *instance, std::default_random_engine *engine); /* returns optimal solution object from passed instance usind depth-first search */

Solution random_construct(Instance *instance, std::default_random_engine *engine);
Solution cheapest_construct(Instance *instance, std::default_random_engine *engine);
Solution lrd1_construct(Instance *instance, std::default_random_engine *engine);
Solution lrd2_construct(Instance *instance, std::default_random_engine *engine);
Solution hrd_construct(Instance *instance, std::default_random_engine *engine);
Solution fixed_order_construct(Instance *instance, std::default_random_engine *engine); /* schedules intervention in fixed order to minimize solution's extended objective */
Solution longest1_construct(Instance *instance, std::default_random_engine *engine);
Solution longest2_construct(Instance *instance, std::default_random_engine *engine);
Solution most_exclusions_construct(Instance *instance, std::default_random_engine *engine);
Solution shortest1_construct(Instance *instance, std::default_random_engine *engine);
Solution shortest2_construct(Instance *instance, std::default_random_engine *engine);
Solution most_expensive_construct(Instance *instance, std::default_random_engine *engine);

#endif
