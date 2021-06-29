//
// Created by wolledav on 01.09.20.
//

#ifndef ROADEF_LOCAL_SEARCH_H
#define ROADEF_LOCAL_SEARCH_H

#include <string.h>
#include <omp.h>
#include "../instance/instance.hpp"
#include "../solution/solution.hpp"

using namespace std;

typedef bool (*operator_t) (Solution &solution);

typedef struct estimate {
    fitness_t score;
    uint_t t1;
    uint_t t2;
} estimate;

typedef struct cand {
    uint_t i;
    uint_t t;
} cand;

void apply_shift(Solution &solution, cand &c1, cand &c2);

bool two_shift(Solution &solution, uint_t i1, uint_t i2);
estimate two_shift_estimate(Solution &solution, uint_t i1, uint_t i2);

bool one_shift(Solution &solution);
bool rand_two_shift(Solution &solution);
bool full_two_shift(Solution &solution);
bool excl_two_shift(Solution &solution);

void vnd(Solution &solution, vector<operator_t> operators);
void rvnd(Solution &solution, vector<operator_t> operators);

#endif //ROADEF_LOCAL_SEARCH_H
