#ifndef ALNS_H
#define ALNS_H

#include <vector>
#include <unordered_map>
#include <random>
#include <iostream>
#include <chrono>
#include <string>
#include <mutex>
#include <thread>
#include <atomic>
#include "../solution/solution.hpp"
#include "../insertions/insertions.hpp"
#include "../removals/removals.hpp"
#include "../types.hpp"
#include "../params.hpp"

using namespace std;

typedef void (*func_t) (Solution &solution);
typedef bool (*operator_t) (Solution &solution);
typedef Solution(*cons_t) (Instance *instance, std::default_random_engine *engine);

typedef unordered_map<func_t, string> func_to_name_t;
typedef unordered_map<operator_t, string> op_to_name_t;
typedef unordered_map<cons_t, string> cons_to_name_t;

class ALNS {
    private:
        Instance *instance;
        std::default_random_engine alns_engine;
        std::default_random_engine ls_engine;
        Solution cur_solution;  // best in current restart, initial solution in every iteration
        Solution best_solution; // best over all restarts
        uniform_real_distribution<double> range;
        std::mutex cur_sol_mutex;

    vector<func_t> repair_methods; /* pointers to methods used to repair solution */
        func_to_name_t repair_methods_names;
        vector<func_t> destroy_methods; /* pointers to methods used to destroy solution */
        func_to_name_t destroy_methods_names;
        vector<operator_t> ls_operators;
        op_to_name_t ls_operators_names;
        cons_t construction;
        cons_to_name_t construction_name;

        vector<double> destroy_weights; /* weights of individual methods that are adujusted during the run of the program */
        double destroy_weights_sum; /* current sum of destroy weights used to calculate selection probability */
        vector<double> repair_weights; /* weights of individual methods that are adjusted during the run of the program */
        double repair_weights_sum; /* current sum of repair weights used to calculate selection probability */
        atomic<uint_t> restarts_cnt;
        double accept_temperature;
        double cooling_rate;
        void adjust_repair_weight(uint_t idx, double psi); /* adjust weights for repair method on passed index with psi parameter */
        void adjust_destroy_weight(uint_t idx, double psi); /* adjust weights for destroy method on passed index with psi parameter */
        double compute_repair_method_probability(uint_t idx); /* compute probability of selecting repair method on index idx based on its weight */
        double compute_destroy_method_probability(uint_t idx); /* compute probability of selecting destroy method on index idx based on its weight */
        bool iteration(); /* one iteration of the search */
    public:
//        ALNS(Solution *solution);
        ALNS(Instance *instance, int seed);
        void adjust_weights(uint_t repair_idx, uint_t destroy_idx, double psi); /* public wrapper that calls adjust method for repair and destroy function and psi */
        double get_psi(double omega_1, double omega_2, double omega_3, double omega_4); /* get max of 4 change parameters */
        uint_t select_repair_idx(); /* randomly selects repair method considering probabilities computed from weights */
        uint_t select_destroy_idx(); /* randomly selects destroy method considering probabilities computed from weights */
        Solution greedy_search(); /* search neigborhood */
        void add_repair_method(func_t method, string method_name);
        void add_destroy_method(func_t method, string method_name);
        void add_ls_operator(operator_t op, string op_name);
        string get_repair_name(func_t method);
        string get_destroy_name(func_t method);
        void add_construction(cons_t cons, string cons_name);
        void dump_methods();
        bool accept_solution(fitness_t cur_cost, fitness_t new_cost); /* Acceptance criterion based on simulated annealing */

        double init_temperature(fitness_t cur_cost, double initial_acceptance);
        double init_cooling_rate(double initial_acceptance, double final_acceptance, uint_t num_iterations);

        void parallel_local_search();
};

#endif
