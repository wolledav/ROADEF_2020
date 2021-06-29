//
// Created by wolledav on 01.09.20.
//

#include "local_search.hpp"

#include <random>
#include <atomic>

int ls_cnt = 0;
int skipped_cnt = 0;

void apply_shift(Solution &solution, cand &c1, cand &c2) {
    ++ls_cnt;
    solution.unschedule(c1.i);
    solution.unschedule(c2.i);
    solution.schedule(c1.i, c1.t);
    solution.schedule(c2.i, c2.t);
}

/*
 * Attempts to shift one intervention for all possible interventions and start times.
 * Returns true if solution improved.
 */
bool one_shift(Solution &solution) {
#if VERBOSE_LS
    cerr << "ONE_SHIFT" << endl;
#endif
    // Shared variables
    bool improved = false;
    atomic<fitness_t> best_score { solution.extended_objective };
    uint_t cand_i;
    uint_t cand_t;

    // Select a random subset of interventions of size |I| * ONE_SHIFT_DEPTH
    auto interventions = solution.instance->get_interventions();
    std::shuffle(interventions.begin(), interventions.end(), *solution.engine);
    uint_t depth = max(1, (int)(interventions.size() * ONE_SHIFT_DEPTH));
    interventions = vector<uint_t> (interventions.begin(), interventions.begin() + depth);


    if (LS_FIRST_IMPROVE) {
        for (int id = 0; id < interventions.size(); id++) {
            auto i = interventions[id];
            if (!improved) {
                Solution cur_sol = solution;
                cur_sol.unschedule(i);
                if (best_score - cur_sol.extended_objective > ACCEPT_TOLERANCE) { // bound by best_score
                    for (uint_t t = 1; t <= solution.instance->get_t_max(i); ++t) {
                        if (improved) break;
                        auto cur_score = cur_sol.estimate_schedule(i, t).extended_objective;
                        if (best_score - cur_score > ACCEPT_TOLERANCE) {
                            improved = true;
                            best_score = cur_score;
                            cand_i = i;
                            cand_t = t;
                        }
                    }
                }
            }
        }
    } else {
        vector<vector<fitness_t>> objectives(interventions.size());

        #pragma omp parallel for shared(objectives) schedule(dynamic)
        for (int idx = 0; idx < interventions.size(); ++idx) {
            // retrieve info about intervention
            auto i = interventions[idx];
            uint_t times = solution.instance->get_t_max(i);
            // alloc local vector for estimates
            vector<fitness_t> temp;
            temp.reserve(times);
            temp.push_back(0.0);
            Solution cur_sol = solution;
            cur_sol.unschedule(i);
            fitness_t val = cur_sol.extended_objective;
            if (best_score - val > 10 * ACCEPT_TOLERANCE) {
                for (uint_t t = 1; t <= times; ++t) {
                    fitness_t cur_score = cur_sol.estimate_schedule(i, t).extended_objective;
                    temp.push_back(cur_score);
                    fitness_t x = best_score.load();
                    while (x > cur_score && !best_score.compare_exchange_strong(x, cur_score));
                }
            }
            // swap with shared vector to avoid false sharing
            swap(temp, objectives[idx]);
        }

        fitness_t new_best_score = solution.extended_objective;
        for (int idx = 0; idx < interventions.size(); ++idx) {
            auto i = interventions[idx];
            for (uint_t t = 1; t < objectives[idx].size(); ++t) {
                fitness_t cur_score = objectives[idx][t];
                if (new_best_score - cur_score > ACCEPT_TOLERANCE) {
                    improved = true;
                    new_best_score = cur_score;
                    cand_i = i;
                    cand_t = t;
                }
            }
        }

    }

    if (improved) {
        solution.unschedule(cand_i);
        solution.schedule(cand_i, cand_t);
    }

    return improved;
}

/*
 * Returns best extended objective obtainable by performing two_shift on the given interventions
 */
estimate two_shift_estimate(Solution &solution, uint_t i1, uint_t i2) {
    /* auto best_score = solution.extended_objective; */
    estimate best = { .score = solution.extended_objective, .t1 = 0, .t2 = 0 };
    bool improved = false;

    auto cur_sol = solution;
    cur_sol.unschedule(i1);
    cur_sol.unschedule(i2);

    if (best.score - cur_sol.extended_objective > ACCEPT_TOLERANCE) { // bound cur_sol without i1, i2
        for (uint_t t1 = 1; t1 <= solution.instance->get_t_max(i1); ++t1) {
            if (LS_FIRST_IMPROVE && improved) break;
            cur_sol.schedule(i1, t1);
            if (best.score - cur_sol.extended_objective > ACCEPT_TOLERANCE) { // bound cur_sol without i2
                for (uint_t t2 = 1; t2 <= solution.instance->get_t_max(i2); ++t2) {
                    if (LS_FIRST_IMPROVE && improved) break;
                    auto cur_score = cur_sol.estimate_schedule(i2, t2).extended_objective;
                    if (best.score - cur_score > ACCEPT_TOLERANCE) {
                        improved = true;
                        best.score = cur_score;
                        best.t1 = t1;
                        best.t2 = t2;
                    }
                }
            }
            cur_sol.unschedule(i1);
        }
    }

    return best;
}

/*
 * Selects two distinct random scheduled interventions and performs two_shift
 */
bool rand_two_shift(Solution &solution) {
#if VERBOSE_LS
    cerr << "RAND_TWO_SHIFT" << endl;
#endif
    // Shared variables
    auto best_score = solution.extended_objective;
    cand c1;
    cand c2;
    bool improved = false;

    // Get TWO_SHIFT_LIMIT pairs of i1, i2
    auto interventions = solution.instance->get_interventions();
    std::uniform_int_distribution<uint_t> distribution(1, interventions.size());
    vector<pair<uint_t, uint_t>> int_pairs;
    for (int i = 0; i < TWO_SHIFT_LIMIT; i++) {
        uint_t i1 = distribution(*solution.engine);
        uint_t i2 = distribution(*solution.engine);
        while (i2 == i1) i2 = distribution(*solution.engine);
        int_pairs.emplace_back(i1, i2);
    }

    uint_t size = int_pairs.size();

    if (LS_FIRST_IMPROVE) {
        for (int id = 0; id < size; id++) {
            auto p = int_pairs[id];
            if (!(LS_FIRST_IMPROVE && improved)) {
                uint_t i1 = p.first;
                uint_t i2 = p.second;
                estimate cur_score = two_shift_estimate(solution, i1, i2);
                if (best_score - cur_score.score > ACCEPT_TOLERANCE) {
                    improved = true;
                    best_score = cur_score.score;
                    c1.i = i1;
                    c2.i = i2;
                    c1.t = cur_score.t1;
                    c2.t = cur_score.t2;
                }
            }
        }
    } else {

        vector<estimate> estimates;
        estimates.resize(size);

        #pragma omp parallel for schedule(dynamic)
        for (int id = 0; id < size; ++id) {
            auto p = int_pairs[id];
            estimate e = two_shift_estimate(solution, p.first, p.second);
            #pragma omp critical
            estimates[id] = e;
        }

        for (int id = 0; id < size; ++id) {
            estimate e = estimates[id];
            if (best_score - e.score > ACCEPT_TOLERANCE) {
                auto p = int_pairs[id];
                improved = true;
                best_score = e.score;
                c1.i = p.first;
                c2.i = p.second;
                c1.t = e.t1;
                c2.t = e.t2;
            }
        }
    }

    if (improved) {
        apply_shift(solution, c1, c2);
    }

    return improved;
}

/*
 * Performs two_shift on all pairs of interventions in exclusion
 */
bool excl_two_shift(Solution &solution) {
    #if VERBOSE_LS
    cerr << "EXCL_TWO_SHIFT" << endl;
    #endif

    if (solution.exclusion_penalty > 0) {
        auto exclusions = solution.instance->get_exclusion_pairs();
        std::shuffle(exclusions.begin(), exclusions.end(), *solution.engine);
        // Shared variables
        auto best_score = solution.extended_objective;
        cand c1;
        cand c2;
        bool improved = false;

        uint_t size = min(TWO_SHIFT_LIMIT, (uint_t)exclusions.size());

        if (LS_FIRST_IMPROVE) {
            for (uint_t i = 0; i < size; i++) {
                auto e = exclusions[i];
                if (!improved) {
                    estimate cur_score = two_shift_estimate(solution, e.first, e.second);
                    if (best_score - cur_score.score > ACCEPT_TOLERANCE) {
                        improved = true;
                        best_score = cur_score.score;
                        c1.i = e.first;
                        c2.i = e.second;
                        c1.t = cur_score.t1;
                        c2.t = cur_score.t2;
                    }
                }
            }
        } else {
            vector<estimate> estimates;
            estimates.resize(size);

            #pragma omp parallel for schedule(dynamic)
            for (uint_t i = 0; i < size; ++i) {
                auto ex = exclusions[i];
                estimate e = two_shift_estimate(solution, ex.first, ex.second);
                #pragma omp critical
                estimates[i] = e;
            }
            for (uint_t i = 0; i < size; ++i) {
                estimate e = estimates[i];
                if (best_score - e.score > ACCEPT_TOLERANCE) {
                    improved = true;
                    auto ex = exclusions[i];
                    best_score = e.score;
                    c1.i = ex.first;
                    c2.i = ex.second;
                    c1.t = e.t1;
                    c2.t = e.t2;
                }
            }
        }

        if (improved) {
            apply_shift(solution, c1, c2);
        }
        return improved;
    } else {
        return false;
    }
}

/*
 * Performs two_shift on all pairs of interventions.
 */
bool full_two_shift(Solution &solution) {
    auto interventions = solution.instance->get_interventions();
    auto best_score = solution.extended_objective;
    cand c1;
    cand c2;
    bool improved = false;
    for (auto i1:interventions) {
        if (LS_FIRST_IMPROVE && improved) break;
        for (auto i2:interventions) {
            cout << i1 << " " << i2 << endl;
            if (LS_FIRST_IMPROVE && improved) break;
            if (i1 < i2) {
                estimate cur_score = two_shift_estimate(solution, i1, i2);
                if (best_score - cur_score.score > ACCEPT_TOLERANCE) {
                    improved = true;
                    best_score = cur_score.score;
                    c1.i = i1;
                    c2.i = i2;
                    c1.t = cur_score.t1;
                    c2.t = cur_score.t2;
                }
            }
        }
    }

    if (improved) {
        apply_shift(solution, c1, c2);
    }

    return improved;
}

/*
 * Performs variable neighborhood descent using the given list of operators
 */
void vnd(Solution &solution, vector<operator_t> operators) {
    int i = 0;
    bool improved = true;
    while (improved) {
        improved = false;

        for (auto op:operators) {
            improved = op(solution);
            if (improved) break;
        }

        #if VERBOSE_LS
        cerr << "VND iter. " << ++i << " finished" << endl;
        #endif
    }
}

int get_random_among_available(unsigned availableCount, const vector<bool> &available, std::default_random_engine *engine) {
    uniform_int_distribution<int> distribution(0,availableCount - 1);
    auto r = distribution(*engine);
    for (unsigned i = 0; i < available.size(); ++i) {
        if (available[i]) {
            if (r == 0) {
                return i;
            }
            --r;
        }
    }
    return -1;

}

/*
 * Performs randomized variable neighborhood descent using the given list of operators
 */
void rvnd(Solution &solution, vector<operator_t> operators) {
    int i = 0;

    // nlSize ~ number of available neighborhoods
    auto nlSize = static_cast<unsigned>(operators.size());

    // available[i] == true ~ neighborhood i is available
    std::vector<bool> available(operators.size(), true);

    // Iterate until there are available neighborhoods
    while (nlSize > 0 && !stop()) {
        // Pick neighborhood index randomly among available neighborhoods
        auto operator_id = get_random_among_available(nlSize, available, solution.engine);
        // Attempt to improve the tour in the selected neighborhood
        if ((*operators[operator_id])(solution)) {
            #if VERBOSE_LS
            cerr << "RVND iter. " << ++i << " finished with improvement, extended_objective: " << solution.extended_objective << endl;
            #endif
            // If success, then set all neighborhoods to be available again
            nlSize = static_cast<unsigned>(operators.size());
            std::fill(available.begin(), available.end(), true);
        } else {

            // If not success, then make the current neighborhood unavailable and decrease nlSize
            available[operator_id] = false;
            --nlSize;
        }
    }

    #if VERBOSE_LS
    cerr << "RVND iter. " << ++i << " finished without improvement, extended_objective: " << solution.extended_objective << endl;
    #endif
}






