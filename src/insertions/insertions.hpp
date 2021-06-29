#ifndef ROADEF_INSERTIONS_H
#define ROADEF_INSERTIONS_H

#include <limits>
#include <unordered_set>
#include <iostream>
#include <string>
#include <cstring>
#include "../instance/instance.hpp"
#include "../solution/solution.hpp"
#include "../types.hpp"
#include "../params.hpp"

using namespace std;

tuple<uint_t, fitness_t, Objective> get_cheapest_time(Solution &sol, uint_t i, fitness_t nu);

// Both intervention and start time selection randomized
void property_at_cheapest_time_based_insert(Solution &solution, fitness_t nu, fitness_t mu, const string& property);
// Exact heuristics - nu = 0, mu = 0 or mu = INFINITY
inline void cheapest_insert(Solution &solution) { property_at_cheapest_time_based_insert(solution, 0, 0, "cost");}
inline void most_expensive_insert(Solution &solution) { property_at_cheapest_time_based_insert(solution, 0, INFINITY, "cost");}
inline void lrd1_insert(Solution &solution) { property_at_cheapest_time_based_insert(solution, 0, 0, "rd");}
inline void hrd_insert(Solution &solution) { property_at_cheapest_time_based_insert(solution, 0, INFINITY, "rd");}
inline void shortest1_insert(Solution &solution) { property_at_cheapest_time_based_insert(solution, 0, 0, "length");}
inline void longest1_insert(Solution &solution) { property_at_cheapest_time_based_insert(solution, 0, INFINITY, "length");}
// Noisy cost, exact int. selection - nu = NU, mu = 0 or mu = INFINITY
inline void n1_cheapest_insert(Solution &solution) { property_at_cheapest_time_based_insert(solution, NU, 0, "cost");}
inline void n1_most_expensive_insert(Solution &solution) {property_at_cheapest_time_based_insert(solution, NU, INFINITY, "cost");}
inline void n1_lrd1_insert(Solution &solution) { property_at_cheapest_time_based_insert(solution, NU, 0, "rd");}
inline void n1_hrd_insert(Solution &solution) { property_at_cheapest_time_based_insert(solution, NU, INFINITY, "rd");}
inline void n1_shortest1_insert(Solution &solution) { property_at_cheapest_time_based_insert(solution, NU, 0, "length");}
inline void n1_longest1_insert(Solution &solution) { property_at_cheapest_time_based_insert(solution, NU, INFINITY, "length");}
// Exact cost, noisy int. selection - nu = 0, mu = MU1 or mu = MU2
inline void n2_cheapest_insert(Solution &solution) { property_at_cheapest_time_based_insert(solution, 0, MU1, "cost");}
inline void n2_most_expensive_insert(Solution &solution) {property_at_cheapest_time_based_insert(solution, 0, MU2, "cost");}
inline void n2_lrd1_insert(Solution &solution) { property_at_cheapest_time_based_insert(solution, 0, MU1, "rd");}
inline void n2_hrd_insert(Solution &solution) { property_at_cheapest_time_based_insert(solution, 0, MU2, "rd");}
inline void n2_shortest1_insert(Solution &solution) { property_at_cheapest_time_based_insert(solution, 0, MU1, "length");}
inline void n2_longest1_insert(Solution &solution) { property_at_cheapest_time_based_insert(solution, 0, MU2, "length");}
// Noisy heuristics - nu = NU, mu = MU1 or mu = MU2
inline void n3_cheapest_insert(Solution &solution) { property_at_cheapest_time_based_insert(solution, NU, MU1, "cost");}
inline void n3_most_expensive_insert(Solution &solution) {property_at_cheapest_time_based_insert(solution, NU, MU2, "cost");}
inline void n3_lrd1_insert(Solution &solution) { property_at_cheapest_time_based_insert(solution, NU, MU1, "rd");}
inline void n3_hrd_insert(Solution &solution) { property_at_cheapest_time_based_insert(solution, NU, MU2, "rd");}
inline void n3_shortest1_insert(Solution &solution) { property_at_cheapest_time_based_insert(solution, NU, MU1, "length");}
inline void n3_longest1_insert(Solution &solution) { property_at_cheapest_time_based_insert(solution, NU, MU2, "length");}


void static_property_based_insert(Solution &solution, fitness_t nu, fitness_t mu, const string& property);

inline void least_exclusions_insert(Solution &solution) {static_property_based_insert(solution, 0, 0, "exclusions_cnt");}
inline void most_exclusions_insert(Solution &solution) {static_property_based_insert(solution, 0, INFINITY, "exclusions_cnt");}
inline void least_used_insert(Solution &solution) {static_property_based_insert(solution, 0, 0, "usage");}
inline void most_used_insert(Solution &solution) {static_property_based_insert(solution, 0, INFINITY, "usage");}

inline void n1_least_exclusions_insert(Solution &solution) {static_property_based_insert(solution, NU, 0, "exclusions_cnt");}
inline void n1_most_exclusions_insert(Solution &solution) {static_property_based_insert(solution, NU, INFINITY, "exclusions_cnt");}
inline void n1_least_used_insert(Solution &solution) {static_property_based_insert(solution, NU, 0, "usage");}
inline void n1_most_used_insert(Solution &solution) {static_property_based_insert(solution, NU, INFINITY, "usage");}

inline void n2_least_exclusions_insert(Solution &solution) {static_property_based_insert(solution, 0, MU1, "exclusions_cnt");}
inline void n2_most_exclusions_insert(Solution &solution) {static_property_based_insert(solution, 0, MU2, "exclusions_cnt");}
inline void n2_least_used_insert(Solution &solution) {static_property_based_insert(solution, 0, MU1, "usage");}
inline void n2_most_used_insert(Solution &solution) {static_property_based_insert(solution, 0, MU2, "usage");}

inline void n3_least_exclusions_insert(Solution &solution) {static_property_based_insert(solution, NU, MU1, "exclusions_cnt");}
inline void n3_most_exclusions_insert(Solution &solution) {static_property_based_insert(solution, NU, MU2, "exclusions_cnt");}
inline void n3_least_used_insert(Solution &solution) {static_property_based_insert(solution, NU, MU1, "usage");}
inline void n3_most_used_insert(Solution &solution) {static_property_based_insert(solution, NU, MU2, "usage");}


// Start time selection randomized
void general_random_insert(Solution &solution, fitness_t nu);
inline void random_insert(Solution &solution) {general_random_insert(solution, 0);}
inline void n1_random_insert(Solution &solution) {general_random_insert(solution, NU);}

void general_fixed_order_insert(Solution &solution, fitness_t nu);
inline void fixed_order_insert(Solution &solution) {general_fixed_order_insert(solution, 0);}
inline void n1_fixed_order_insert(Solution &solution) {general_fixed_order_insert(solution, NU);}


// Not hybridized, todo when previous hybridization successful
void lrd2_insert(Solution &solution);
void longest2_insert(Solution &solution);
void shortest2_insert(Solution &solution);

vector<uint_t> get_unscheduled_subset(Solution &solution, const string& property, fitness_t mu);

#endif //ROADEF_INSERTIONS_H
