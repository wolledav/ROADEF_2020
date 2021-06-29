#ifndef OBJECTIVE_H
#define OBJECTIVE_H

#include <iostream>
#include "../types.hpp"

using namespace std;

class Objective {
    public:
        fitness_t mean_risk;
        fitness_t expected_excess;
        fitness_t final_objective;
        fitness_t total_resource_use;
        fitness_t workload_underuse;
        fitness_t workload_overuse;
        uint_t exclusion_penalty;
        fitness_t extended_objective;
        Objective();
        void print_state();
        bool is_valid();
};

#endif
