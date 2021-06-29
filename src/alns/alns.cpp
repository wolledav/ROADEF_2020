#include "alns.hpp"
#include "../local_search/local_search.hpp"
#include "../constructions/constructions.hpp"

void (*insertions[INSERT_COUNT])(Solution &solution) {
        [RANDOM_INSERT] = random_insert,
        [FIXED_INSERT] = fixed_order_insert,
        [CHEAPEST_INSERT] = cheapest_insert,
        [MOST_EXPENSIVE_INSERT] = most_expensive_insert,
        [LRD1_INSERT] = lrd1_insert,
        [LRD2_INSERT] = lrd2_insert,
        [HRD_INSERT] = hrd_insert,
        [LONGEST1_INSERT] = longest1_insert,
        [LONGEST2_INSERT] = longest2_insert,
        [SHORTEST1_INSERT] = shortest1_insert,
        [SHORTEST2_INSERT] = shortest2_insert,
        [MOST_EXCLUSIONS_INSERT] = most_exclusions_insert,
        [LEAST_EXCLUSIONS_INSERT] = least_exclusions_insert,
        [MOST_USED_INSERT] = most_used_insert,
        [LEAST_USED_INSERT] = least_used_insert,

        [N1_RANDOM_INSERT] = n1_random_insert,
        [N1_FIXED_INSERT] = n1_fixed_order_insert,
        [N1_CHEAPEST_INSERT] = n1_cheapest_insert,
        [N1_MOST_EXPENSIVE_INSERT] = n1_most_expensive_insert,
        [N1_LRD1_INSERT] = n1_lrd1_insert,
        [N1_HRD_INSERT] = n1_hrd_insert,
        [N1_LONGEST1_INSERT] = n1_longest1_insert,
        [N1_SHORTEST1_INSERT] = n1_shortest1_insert,
        [N1_MOST_EXCLUSIONS_INSERT] = n1_most_exclusions_insert,
        [N1_LEAST_EXCLUSIONS_INSERT] = n1_least_exclusions_insert,
        [N1_MOST_USED_INSERT] = n1_most_used_insert,
        [N1_LEAST_USED_INSERT] = n1_least_used_insert,

        [N2_CHEAPEST_INSERT] = n2_cheapest_insert,
        [N2_MOST_EXPENSIVE_INSERT] = n2_most_expensive_insert,
        [N2_LRD1_INSERT] = n2_lrd1_insert,
        [N2_HRD_INSERT] = n2_hrd_insert,
        [N2_LONGEST1_INSERT] = n2_longest1_insert,
        [N2_SHORTEST1_INSERT] = n2_shortest1_insert,
        [N2_MOST_EXCLUSIONS_INSERT] = n2_most_exclusions_insert,
        [N2_LEAST_EXCLUSIONS_INSERT] = n2_least_exclusions_insert,
        [N2_MOST_USED_INSERT] = n2_most_used_insert,
        [N2_LEAST_USED_INSERT] = n2_least_used_insert,

        [N3_CHEAPEST_INSERT] = n3_cheapest_insert,
        [N3_MOST_EXPENSIVE_INSERT] = n3_most_expensive_insert,
        [N3_LRD1_INSERT] = n3_lrd1_insert,
        [N3_HRD_INSERT] = n3_hrd_insert,
        [N3_LONGEST1_INSERT] = n3_longest1_insert,
        [N3_SHORTEST1_INSERT] = n3_shortest1_insert,
        [N3_MOST_EXCLUSIONS_INSERT] = n3_most_exclusions_insert,
        [N3_LEAST_EXCLUSIONS_INSERT] = n3_least_exclusions_insert,
        [N3_MOST_USED_INSERT] = n3_most_used_insert,
        [N3_LEAST_USED_INSERT] = n3_least_used_insert
};

void (*removals[REMOVE_COUNT])(Solution &solution) {
        [RANDOM_REMOVE] = random_remove,
        [CHEAPEST_REMOVE] = cheapest_remove,
        [MOST_EXPENSIVE_REMOVE] = most_expensive_remove,
        [LRD_REMOVE] = lrd_remove,
        [HRD_REMOVE] = hrd_remove,
        [LONGEST_REMOVE] = longest_remove,
        [SHORTEST_REMOVE] = shortest_remove,
        [MOST_EXCLUSIONS_REMOVE] = most_exclusions_remove,
        [LEAST_EXCLUSIONS_REMOVE] = least_exclusions_remove,
        [MOST_USED_REMOVE] = most_used_remove,
        [LEAST_USED_REMOVE] = least_used_remove
};

Solution (*constructions[CONSTRUCTION_COUNT])(Instance *instance, std::default_random_engine *engine) {
        [RANDOM_CONSTRUCT] = random_construct,
        [LONGEST1_CONSTRUCT] = longest1_construct,
        [HRD_CONSTRUCT] = hrd_construct,
        [MOST_EXPENSIVE_CONSTRUCT] = most_expensive_construct,
        [CHEAPEST_CONSTRUCT] = cheapest_construct,
        [LRD1_CONSTRUCT] = lrd1_construct,
        [LRD2_CONSTRUCT] = lrd2_construct,
        [FIXED_CONSTRUCT] = fixed_order_construct,
        [LONGEST2_CONSTRUCT] = longest2_construct,
        [MOST_EXCLUSIONS_CONSTRUCT] = most_exclusions_construct,
        [SHORTEST1_CONSTRUCT] = shortest1_construct,
        [SHORTEST2_CONSTRUCT] = shortest2_construct
};

bool (*ls[LS_COUNT])(Solution &solution) {
        [ONE_SHIFT] = one_shift,
        [EXCL_TWO_SHIFT] = excl_two_shift,
        [RAND_TWO_SHIFT] = rand_two_shift
};


void ALNS::adjust_repair_weight(uint_t idx, double psi) {
    this->repair_weights[idx] = LAMBDA * this->repair_weights[idx] + (1 - LAMBDA) * psi;
}

void ALNS::adjust_destroy_weight(uint_t idx, double psi) {
    this->destroy_weights[idx] = LAMBDA * this->destroy_weights[idx] + (1 - LAMBDA) * psi;
}

double ALNS::compute_repair_method_probability(uint_t idx) {
    return this->repair_weights[idx] / this->repair_weights_sum;
}

double ALNS::compute_destroy_method_probability(uint_t idx) {
    return this->destroy_weights[idx] / this->destroy_weights_sum;
}

bool ALNS::iteration() {
    bool improved = false;

//    cur_sol_mutex.lock();
    Solution solution_ = cur_solution;
//    cur_sol_mutex.unlock();

    /* select destroy and repair methods (indices to vectors in ALNS) */
    uint_t d_idx = this->select_destroy_idx();
    uint_t r_idx = this->select_repair_idx();
    uint_t ub = max(1, int(ALNS_DEPTH * solution_.scheduled.size()));
    uniform_int_distribution<int> count(1, ub);

    /* destroy temporary solution with randomly selected destroy method */
    for (uint_t i = 0; i < count(alns_engine); ++i) {
        (*(this->destroy_methods[d_idx]))(solution_);
    }

    /* repair temporary solution with randomly selected repair method */
    while (solution_.has_unscheduled()) {
        if (!stop()) {
            (*(this->repair_methods[r_idx]))(solution_);
        } else {
            fixed_order_insert(solution_);
        }
    }

    /* perform local search */
    rvnd(solution_, ls_operators);

    /* Adjust omega parameters to influence weights */
    double omega_1 = 0;
    double omega_2 = 0;
    double omega_3 = 0;
    double omega_4 = 0;
    if (best_solution.extended_objective - solution_.extended_objective > ACCEPT_TOLERANCE) { // better than global best
        best_solution = solution_;
        omega_1 = OMEGA_1;
    }

//    cur_sol_mutex.lock();
    if (cur_solution.extended_objective - solution_.extended_objective > ACCEPT_TOLERANCE) { // better than current & accepted
        cur_solution = solution_;
        improved = true;
        omega_2 = OMEGA_2;
    }
//    if (accept_solution(cur_solution.extended_objective, solution_.extended_objective)) { // accepted
//        omega_3 = OMEGA_3;
//    }
    else { // rejected
        omega_4 = OMEGA_4;
    }
//    cur_sol_mutex.unlock();

    this->adjust_weights(r_idx, d_idx, this->get_psi(omega_1, omega_2, omega_3, omega_4));

#if VERBOSE_ALNS
    cerr << "WEIGHT ADJUSTMENT" << endl;
    cerr << "psi: "<< this->get_psi(omega_1, omega_2, omega_3, omega_4) << endl;
    cerr << "repair weights: ";
    for (auto w:this->repair_weights) cerr << w << " ";
    cerr << endl;
    cerr << "destroy weights: ";
    for (auto w:this->destroy_weights) cerr << w << " ";
    cerr << endl;
#endif

    return improved;
}

ALNS::ALNS(Instance *instance, int seed) {
    this->instance = instance;
    this->alns_engine.seed(seed);
    this->ls_engine.seed(seed);
    this->range = uniform_real_distribution<double>(0, 1);
    methods.check_setup();
    for (int i = 0; i < INSERT_COUNT; ++i)
        if (methods.insertions[i])
            this->add_repair_method(insertions[i], insertions_labels[i]);
    for (int i = 0; i < REMOVE_COUNT; ++i)
        if (methods.removals[i])
            this->add_destroy_method(removals[i], removals_labels[i]);
    for (int i = 0; i < LS_COUNT; ++i) if (methods.ls[i]) this->add_ls_operator(ls[i], ls_labels[i]);
    this->add_construction(constructions[methods.construction], constructions_labels[methods.construction]);
    /* set initial weight for each method and add it to sum */
    this->repair_weights = vector<double>(this->repair_methods.size(), INITIAL_WEIGHT);
    this->repair_weights_sum = this->repair_methods.size() * INITIAL_WEIGHT;
    /* set initial weight for each method and add it to sum */
    this->destroy_weights = vector<double>(this->destroy_methods.size(), INITIAL_WEIGHT);
    this->destroy_weights_sum = this->destroy_methods.size() * INITIAL_WEIGHT;
    this->restarts_cnt = 0;
#if VERBOSE_CONFIG
    this->dump_methods();
#endif

}

void ALNS::adjust_weights(uint_t repair_idx, uint_t destroy_idx, double psi) {
    this->repair_weights_sum -= this->repair_weights[repair_idx];
    this->adjust_repair_weight(repair_idx, psi);
    this->repair_weights_sum += this->repair_weights[repair_idx];
    this->destroy_weights_sum -= this->destroy_weights[destroy_idx];
    this->adjust_destroy_weight(destroy_idx, psi);
    this->destroy_weights_sum += this->destroy_weights[destroy_idx];
}

double ALNS::get_psi(double omega_1, double omega_2, double omega_3, double omega_4) {
    double max = omega_1;
    if (omega_2 > max) max = omega_2;
    if (omega_3 > max) max = omega_3;
    if (omega_4 > max) max = omega_4;
    return max;
}

uint_t ALNS::select_repair_idx() {
    vector<double> thresholds(this->repair_weights.size() + 1, 0);
    /* compute probability thresholds (normalize in range <0, 1>) */
    for (uint_t idx = 1; idx < thresholds.size() - 1; ++idx) {
        thresholds[idx] = thresholds[idx - 1] + this->compute_repair_method_probability(idx - 1);
    }
    thresholds[thresholds.size() - 1] = 1;
    double rand = this->range(alns_engine); /* get rand number in <0, 1> */
    uint_t ret = 0; /* default repair method in case of selection error */
    /* compare rand number with thresholds and select index of repair method */
    for (uint_t idx = 1; idx < thresholds.size(); ++idx) {
        if (rand <= thresholds[idx]) {
            ret = idx - 1;
            break;
        }
    }
    return ret;
}

uint_t ALNS::select_destroy_idx() {
    vector<double> thresholds(this->destroy_weights.size() + 1, 0);
    /* compute probability thresholds (normalize in range <0, 1>) */
    for (uint_t idx = 1; idx < thresholds.size() - 1; ++idx) {
        thresholds[idx] = thresholds[idx - 1] + this->compute_destroy_method_probability(idx - 1);
    }
    thresholds[thresholds.size() - 1] = 1;
    double rand = this->range(alns_engine); /* get rand number in <0, 1> */
    uint_t ret = 0; /* default repair method in case of selection error */
    /* compare rand number with thresholds and select index of repair method */
    for (uint_t idx = 1; idx < thresholds.size(); ++idx) {
        if (rand <= thresholds[idx]) {
            ret = idx - 1;
            break;
        }
    }
    return ret;
}

Solution ALNS::greedy_search() {
#if VERBOSE_CONFIG
    cout << "ALNS::greedy_search" << endl;
#endif
    auto init_solution = construction(instance, &alns_engine);

//    cur_sol_mutex.lock();
    cur_solution = init_solution;
    best_solution = cur_solution;
//    cur_sol_mutex.unlock();

//    thread ls_thread(&ALNS::parallel_local_search, this);

    restarts_cnt = 0;
    accept_temperature = 0;
    uint_t iter_cnt = 0;

    while (!stop()) {
        if (this->iteration()) {
            iter_cnt = 0;
        } else {
            iter_cnt++;
        }

#if VERBOSE_ALNS
        cerr << "ITERATION" << endl;
        cerr << "iter_cnt: " << iter_cnt << "/" << iters_max << ", restart_cnt: " << restarts_cnt << ", cur_cost: " << cur_solution.extended_objective << ", best_cost: " << best_solution.extended_objective << endl << endl;
#endif

        // Restart
        if (iter_cnt == ITERS_MAX) {
            iter_cnt = 0;

//            cur_sol_mutex.lock();
            restarts_cnt++;
            if (construction_name[construction] == "random") {
                cur_solution = construction(instance, &alns_engine);
            } else { // Other constructions are assumed to be deterministic
                cur_solution = init_solution;
            }
//            cur_sol_mutex.unlock();
        }
    }

//    ls_thread.join();

//    cur_sol_mutex.lock();
    if (best_solution.extended_objective - cur_solution.extended_objective > ACCEPT_TOLERANCE) {
        best_solution = cur_solution;
    }
//    cur_sol_mutex.unlock();

    best_solution.restarts_cnt = restarts_cnt;
    return best_solution;
}

void ALNS::add_repair_method(func_t method, string method_name) {
    this->repair_methods.push_back(method);
    this->repair_methods_names[method] = method_name;
}

void ALNS::add_destroy_method(func_t method, string method_name) {
    this->destroy_methods.push_back(method);
    this->destroy_methods_names[method] = method_name;
}

string ALNS::get_repair_name(func_t method) {
    return this->repair_methods_names[method];
}

string ALNS::get_destroy_name(func_t method) {
    return this->destroy_methods_names[method];
}

void ALNS::dump_methods() {
    cout << "ALNS {" << endl;
    cout << "\tconstruction [" << endl;
    cout << "\t\t" << construction_name.begin()->second << endl;
    cout << "\t]" << endl;
    cout << "\trepair [" << endl;
    for (func_to_name_t::iterator it = this->repair_methods_names.begin(); it != this->repair_methods_names.end();) {
        cout << "\t\t" << it->second;
        if (++it != this->repair_methods_names.end()) cout << ",";
        cout << endl;
    }
    cout << "\t]" << endl;
    cout << "\tdestroy [" << endl;
    for (func_to_name_t::iterator it = this->destroy_methods_names.begin(); it != this->destroy_methods_names.end();) {
        cout << "\t\t" << it->second;
        if (++it != this->destroy_methods_names.end()) cout << ",";
        cout << endl;
    }
    cout << "\t]" << endl;
    cout << "\tlocal_search [" << endl;
    for (auto it = this->ls_operators_names.begin(); it != this->ls_operators_names.end();) {
        cout << "\t\t" << it->second;
        if (++it != this->ls_operators_names.end()) cout << ",";
        cout << endl;
    }
    cout << "\t]" << endl;

    cout << "}" << endl;
}

void ALNS::add_ls_operator(operator_t op, string op_name) {
    this->ls_operators.push_back(op);
    this->ls_operators_names[op] = op_name;
}

void ALNS::add_construction(cons_t cons, string cons_name) {
    this->construction = cons;
    this->construction_name[cons] = cons_name;
}

bool ALNS::accept_solution(fitness_t cur_cost, fitness_t new_cost) {
    double p_accept = min(exp((cur_cost - new_cost) / accept_temperature), 1.0);
    discrete_distribution<int> distribution{1 - p_accept, p_accept};
    bool accept = distribution(alns_engine);
    accept_temperature = cooling_rate * accept_temperature;

#if VERBOSE_ALNS
    cerr << "ACCEPTANCE" << endl;
    cerr << "cur_cost: " << cur_cost << ", new_cost: " << new_cost << endl << "prob_accept: " << p_accept << ", accepted: " << accept << endl << "temperature: " << accept_temperature << endl;
#endif

    return accept;
}

double ALNS::init_temperature(fitness_t cur_cost, double initial_acceptance) {
    return cur_cost * (1 - initial_acceptance) / log(0.5);
}

double ALNS::init_cooling_rate(double initial_acceptance, double final_acceptance, uint_t num_iterations) {
    return pow((1 - final_acceptance) / (1 - initial_acceptance), (1.0 / num_iterations));
}

void ALNS::parallel_local_search() {
#if VERBOSE_CONFIG
    cout << "ALNS::parallel_local_search" << endl;
#endif

    while (!stop()) {
        uint_t restarts_cnt_ = restarts_cnt;
        cur_sol_mutex.lock();
        Solution solution_ = cur_solution;
        cur_sol_mutex.unlock();
        solution_.engine = &this->ls_engine;

        for (int i = 0; i < 10; i++) {
            rvnd(solution_, ls_operators);
        }

        cur_sol_mutex.lock();
        if ((cur_solution.extended_objective - solution_.extended_objective > ACCEPT_TOLERANCE) && restarts_cnt_ == restarts_cnt) {
            cur_solution = solution_;
        }
        cur_sol_mutex.unlock();
    }
}




