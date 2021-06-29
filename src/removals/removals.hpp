#ifndef ROADEF_REMOVALS_H
#define ROADEF_REMOVALS_H

#include <limits>
#include <string>
#include <string.h>
#include "../solution/solution.hpp"
#include "../solution/objective.hpp"
#include "../types.hpp"

using namespace std;

void random_remove(Solution &solution);

void cheapest_remove(Solution &solution);
void most_expensive_remove(Solution &solution);

void lrd_remove(Solution &solution);
void hrd_remove(Solution &solution);

void longest_remove(Solution &solution);
void shortest_remove(Solution &solution);

void most_exclusions_remove(Solution &solution);
void least_exclusions_remove(Solution &solution);

void least_used_remove(Solution &solution);
void most_used_remove(Solution &solution);

#endif //ROADEF_REMOVALS_H
