#include "removals.hpp"

/*
 * Find the most expensive scheduled intervention and unschedule it.
 */
void most_expensive_remove(Solution &solution) {
    Objective o;
    fitness_t highest_decrease = numeric_limits<fitness_t>::max();
    uint_t i;
    for (uint_t si : solution.scheduled) {
        o = solution.estimate_unschedule(si);
        if (o.extended_objective < highest_decrease) {
            i = si;
            highest_decrease = o.extended_objective;
        }
    }
    #if VERBOSE_HEURISTICS
    cerr << "ME_REMOVE: removed intervention " << solution.instance->get_intervention(i) << endl;
    #endif
    solution.unschedule(i);
    solution.unscheduled_cnt[i]++;
}

/*
 * Randomly select a scheduled intervention and unschedule it.
 */
void random_remove(Solution &solution) {
    std::uniform_int_distribution<uint_t> range(0, solution.scheduled.size() - 1);
    auto it = solution.scheduled.begin();
    std::advance(it, range(*solution.engine));
    #if VERBOSE_HEURISTICS
    cerr << "RANDOM_REMOVE: removed intervention " << solution.instance->get_intervention(*it) << endl;
    #endif
    solution.unscheduled_cnt[*it]++;
    solution.unschedule(*it);
}

/*
 * Find the intervention with the highest resource demand (=highest decrease on unschedule in total resource demand) and unschedule it.
 */
void hrd_remove(Solution &solution) {
    Objective o;
    fitness_t highest_decrease = numeric_limits<fitness_t>::max();
    uint_t i;
    for (uint_t si : solution.scheduled) {
        o = solution.estimate_unschedule(si);
        if (o.total_resource_use < highest_decrease) {
            i = si;
            highest_decrease = o.total_resource_use;
        }
    }
    #if VERBOSE_HEURISTICS
    cerr << "HRD_REMOVE: removed intervention " << solution.instance->get_intervention(i) << endl;
    #endif
    solution.unschedule(i);
    solution.unscheduled_cnt[i]++;
}

/**
 * find longest schedule intervention and unschedule it
 */
void longest_remove(Solution &solution) {
    uint_t i;
    uint_t d = 0;
    uint_t sd;
    for (uint_t si : solution.scheduled) {
        sd = solution.instance->delta[solution.instance->get_delta_index(si, solution.get_start_time(si))];
        if (sd > d) {
            d = sd;
            i = si;
        }
    }
    #if VERBOSE_HEURISTICS
    cerr << "LONGEST_REMOVE: removed intervention " << solution.instance->get_intervention(i) << endl;
    #endif
    solution.unschedule(i);
    solution.unscheduled_cnt[i]++;
}

/*
 * Finds a scheduled intervention with most exclusions and unschedules it.
 */
void most_exclusions_remove(Solution &solution) {
    uint_t e_max_cnt = 0;
    uint_t i_cand;
    uint_t e_cur_cnt;

    for (uint i : solution.scheduled) {
        e_cur_cnt = solution.instance->get_excluded(i).size();
        if (e_cur_cnt >= e_max_cnt) {
            e_max_cnt = e_cur_cnt;
            i_cand = i;
        }
    }

    solution.unschedule(i_cand);
    solution.unscheduled_cnt[i_cand]++;

#if VERBOSE_HEURISTICS
    cerr << "MOST_EXCLUSIONS_REMOVE: removed intervention " << solution.instance->get_intervention(i_cand) << " with " << e_max_cnt << " exclusions" << endl;
    #endif
}

void shortest_remove(Solution &solution) {
    uint_t i;
    uint_t d = numeric_limits<uint_t>::max();
    uint_t sd;
    for (uint_t si : solution.scheduled) {
        sd = solution.instance->delta[solution.instance->get_delta_index(si, solution.get_start_time(si))];
        if (sd < d) {
            d = sd;
            i = si;
        }
    }
    #if VERBOSE_HEURISTICS
    cerr << "SHORTEST_REMOVE: removed intervention " << solution.instance->get_intervention(i) << endl;
    #endif
    solution.unschedule(i);
    solution.unscheduled_cnt[i]++;

}

void cheapest_remove(Solution &solution) {
    Objective o;
    fitness_t lowest_decrease = 0;
    uint_t i;
    for (uint_t si : solution.scheduled) {
        o = solution.estimate_unschedule(si);
        if (o.extended_objective > lowest_decrease) {
            i = si;
            lowest_decrease = o.extended_objective;
        }
    }
#if VERBOSE_HEURISTICS
    cerr << "CHEAPEST_REMOVE: removed intervention " << solution.instance->get_intervention(i) << endl;
#endif
    solution.unschedule(i);
    solution.unscheduled_cnt[i]++;

}

/*
 * Find the intervention with the highest resource demand (=highest decrease on unschedule in total resource demand) and unschedule it.
 */
void lrd_remove(Solution &solution) {
    Objective o;
    fitness_t lowest_decrease = numeric_limits<fitness_t>::max();
    uint_t cand_i;
    for (uint_t si:solution.scheduled) {
        o = solution.estimate_unschedule(si);
        if (o.total_resource_use < lowest_decrease) {
            cand_i = si;
            lowest_decrease = o.total_resource_use;
        }
    }
    #if VERBOSE_HEURISTICS
    cerr << "LRD_REMOVE: removed intervention " << solution.instance->get_intervention(cand_i) << endl;
    #endif

    solution.unschedule(cand_i);
    solution.unscheduled_cnt[cand_i]++;

}

void least_exclusions_remove(Solution &solution) {
    uint_t e_min_cnt = numeric_limits<uint_t>::max();
    uint_t i_cand;
    uint_t e_cur_cnt;

    for (auto i:solution.scheduled) {
        e_cur_cnt = solution.instance->get_excluded(i).size();
        if (e_cur_cnt < e_min_cnt) {
            e_min_cnt = e_cur_cnt;
            i_cand = i;
        }
        if (e_min_cnt == 0) {
            break;
        }
    }

    #if VERBOSE_HEURISTICS
    cerr << "LEAST_EXCLUSIONS_REMOVE: removed intervention " << solution.instance->get_intervention(i_cand) << " with " << e_min_cnt << " exclusions" << endl;
    #endif

    solution.unschedule(i_cand);
    solution.unscheduled_cnt[i_cand]++;

}

void least_used_remove(Solution &solution) {
    uint_t min_cnt = numeric_limits<uint_t>::max();
    uint_t i_cand;
    uint_t cur_cnt;

    for (auto i:solution.scheduled) {
        cur_cnt = solution.unscheduled_cnt[i];
        if (cur_cnt < min_cnt) {
            min_cnt = cur_cnt;
            i_cand = i;
        }

        if (min_cnt == 0) break;
    }

    #if VERBOSE_HEURISTICS
    cerr << "LEAST_USED_REMOVE: removed intervention " << solution.instance->get_intervention(i_cand) << " with " << min_cnt << " previous removals" << endl;
    #endif

    solution.unschedule(i_cand);
    solution.unscheduled_cnt[i_cand]++;

}

void most_used_remove(Solution &solution) {
    uint_t max_cnt = 0;
    uint_t i_cand;
    uint_t cur_cnt;

    for (auto i:solution.scheduled) {
        cur_cnt = solution.unscheduled_cnt[i];
        if (cur_cnt >= max_cnt) {
            max_cnt = cur_cnt;
            i_cand = i;
        }
    }

    #if VERBOSE_HEURISTICS
    cerr << "MOST_USED_REMOVE: removed intervention " << solution.instance->get_intervention(i_cand) << " with " << max_cnt << " previous removals" << endl;
    #endif

    solution.unschedule(i_cand);
    solution.unscheduled_cnt[i_cand]++;

}
