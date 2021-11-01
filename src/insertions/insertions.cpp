#include "insertions.hpp"

/*
 * Determine cheapest start time for scheduling intervention i.
 * Parameter nu adds additive noise to cost_insert = (1 + nu) * cost; cost = cost(after) - cost(before).
 * Returns tuple<cheapest_t, best_cost, objective>, where cheapest_t and best cost are "noisy" values and objective is exact
 */
tuple<uint_t, fitness_t, Objective> get_cheapest_time(Solution &sol, uint_t i, fitness_t nu) {
    uint_t cheapest_t;
    fitness_t cost;
    fitness_t best_cost = numeric_limits<fitness_t>::max();
    Objective o;
    Objective best_o;
    uniform_real_distribution<fitness_t> dist(0, nu);
    for (uint_t t = 1; t <= sol.instance->get_t_max(i); ++t) {
        o = sol.estimate_schedule(i, t);
        cost = (1 + dist(*sol.engine)) * (o.extended_objective - sol.extended_objective);
        if (best_cost - cost > ACCEPT_TOLERANCE) {
            best_cost = cost;
            cheapest_t = t;
            best_o = o;
        }
    }
    return make_tuple(cheapest_t, best_cost, best_o);
}

/*
 * For each unscheduled intervention, determine value of the given property in the cheapest start time.
 * Cheapest start time selection is randomized by nu: cost = (1 + nu)*nu
 * Intervention is selected according to the distribution [mu^0, mu^1, ... mu^unscheduled.size()-1]
 * mu = 0 corresponds to lowest value of property, mu = INFINITY to highest, hybrid in between
 */
void property_at_cheapest_time_based_insert(Solution &solution, fitness_t nu, fitness_t mu, const string& property) {
    auto subset = get_unscheduled_subset(solution, property, mu);
    uint_t size = subset.size();
    uint_t id = 0;
//    uint_t size = solution.unscheduled.size();
//    uint_t id = 0;
//    cout << size << endl;

    vector<tuple_t> property_tuples(size);
    vector<fitness_t> probabilities(size);

    for (uint_t i : subset) {
//    for (uint_t i : solution.unscheduled) {
            auto p = get_cheapest_time(solution, i, nu);
        if (property == "length") {
            property_tuples[id] = tuple_t (solution.instance->delta[solution.instance->get_delta_index(i, get<0>(p))], i, get<0>(p)); // property = length, intervention, start time
        } else if (property == "cost") {
            property_tuples[id] = tuple_t (get<1>(p), i, get<0>(p)); // property = cost, intervention, start time
        } else if (property == "rd") {
            property_tuples[id] = tuple_t (get<2>(p).total_resource_use, i, get<0>(p)); // property = resource usage, intervention, start time
        } else {
            cerr << "UNKNOWN_PROPERTY: " << property << endl;
            exit(1);
        }

        probabilities[id] = pow(mu, id);
        if (probabilities[id] == INFINITY) probabilities[id] = numeric_limits<fitness_t>::max(); // distribution will always return last index
        id++;
    }

    std::discrete_distribution<> distribution(probabilities.begin(), probabilities.end());
    id = distribution(*solution.engine);
    nth_element(property_tuples.begin(), property_tuples.begin() + id, property_tuples.end());

    auto property_tuple = *(property_tuples.begin() + id);
    auto i = get<1>(property_tuple);
    auto t = get<2>(property_tuple);

#if VERBOSE_HEURISTICS
    cerr << "PACTB_INSERT(nu = " << nu << ", mu = " << mu << ", property = " << property << "): scheduled intervention " << i << " (" << solution.instance->get_intervention(i) << ") at time " << t << ", property value = " << get<0>(property_tuple) << endl;
#endif

    solution.schedule(i, t);
}

void static_property_based_insert(Solution &solution, fitness_t nu, fitness_t mu, const string &property) {
    uint_t size = solution.unscheduled.size();
    uint_t id = 0;

    vector<tuple_t> property_tuples(size);
    vector<fitness_t> probabilities(size);

    for (uint_t i : solution.unscheduled) {
        if (property == "exclusions_cnt") {
            property_tuples[id] = tuple_t(solution.instance->get_excluded(i).size(), i, 0);
        } else if (property == "usage") {
            property_tuples[id] = tuple_t(solution.unscheduled_cnt[i], i, 0);
        }
        else {
            cerr << "UNKNOWN_PROPERTY: " << property << endl;
            exit(1);
        }

        probabilities[id] = pow(mu, id);
        if (probabilities[id] == INFINITY) probabilities[id] = numeric_limits<fitness_t>::max(); // distribution will always return last index
        id++;
    }

    std::discrete_distribution<> distribution(probabilities.begin(), probabilities.end());
    id = distribution(*solution.engine);
    nth_element(property_tuples.begin(), property_tuples.begin() + id, property_tuples.end());

    auto property_tuple = *(property_tuples.begin() + id);
    auto i = get<1>(property_tuple);
    auto t = get<0>(get_cheapest_time(solution, i, nu));

#if VERBOSE_HEURISTICS
    cerr << "SPB_INSERT(nu = " << nu << ", mu = " << mu << ", property = " << property << "): scheduled intervention " << i << " (" << solution.instance->get_intervention(i) << ") at time " << t << endl;
#endif

    solution.schedule(i, t);
}

/*
 * Select a random unscheduled intervention in solution and schedule it to the cheapest start time.
 */
void general_random_insert(Solution &solution, fitness_t nu) {
    // Get random intervention
    std::uniform_int_distribution<uint_t> range(0, solution.unscheduled.size() - 1);
    auto it = solution.unscheduled.begin();
    std::advance(it, range(*solution.engine));
    uint_t i = *it;
    // Get cheapest start time
    uint_t t = get<0>(get_cheapest_time(solution, i, nu));

#if VERBOSE_HEURISTICS
    cerr << "RANDOM_INSERT: scheduled intervention " << solution.instance->get_intervention(i) << " at time " << t << endl;
#endif

    solution.schedule(i, t);
}

/*
 * find best start time for first intervention in unscheduled and schedule it
 */
void general_fixed_order_insert(Solution &solution, fitness_t nu) {
    uint_t i = solution.get_first_unscheduled();

    auto p = get_cheapest_time(solution, i, nu);

#if VERBOSE_HEURISTICS
    cerr << "FIXED_ORDER_INSERT: scheduled intervention " << solution.instance->get_intervention(i) << " at time " << get<0>(p) << endl;
#endif

    solution.schedule(i, get<0>(p));
}

/*
 * For each unscheduled intervention, determine the lowest absolute increase in total resource use.
 * Schedule the intervention with the lowest increase overall (to the lowest increase start time, no matter the validity)
 */
void lrd2_insert(Solution &solution) {
    uint_t i;
    uint_t t;
    uint_t ut;
    fitness_t lowest_increase;
    fitness_t lowest_increase_overall = numeric_limits<fitness_t>::max();
    fitness_t increase;
    Objective o;
    for (uint_t ui : solution.unscheduled) {
        lowest_increase = numeric_limits<fitness_t>::max();
        for (uint_t uut = 1; uut <= solution.instance->get_t_max(ui); ++uut) {
            o = solution.estimate_schedule(ui, uut);
            increase = o.total_resource_use - solution.total_resource_use;
            if (increase + ACCEPT_TOLERANCE < lowest_increase) {
                ut = uut;
                lowest_increase = increase;
            }
        }
        if (lowest_increase + ACCEPT_TOLERANCE < lowest_increase_overall) {
            i = ui;
            t = ut;
            lowest_increase_overall = lowest_increase;
        }
    }
    #if VERBOSE_HEURISTICS
    cerr << "LRD2_INSERT: scheduled intervention " << solution.instance->get_intervention(i) << " at time " << t << endl;
    #endif
    solution.schedule(i, t);
}

/*
 * for each unscheduled intervention find start time with shortest duration
 * from these schedule the longest
 */
void longest2_insert(Solution &solution) {
    uint_t final_i;
    uint_t final_t;
    uint_t max_length = 0;
    uint_t cur_min_length;
    uint_t length;
    uint_t delta_idx;

    // for all interventions
    for (uint_t i : solution.unscheduled) {
        cur_min_length = numeric_limits<uint_t>::max();
        delta_idx = solution.instance->get_delta_index(i, 1);

        // for all start times
        for (uint_t t = 1; t <= solution.instance->get_t_max(i); ++t) {
            length = solution.instance->delta[delta_idx++];
            if (length < cur_min_length) {
                cur_min_length = length;
                if (cur_min_length > max_length) {
                    final_i = i;
                    final_t = t;
                    max_length = cur_min_length;
                }
            }
        }
    }

    #if VERBOSE_HEURISTICS
    cerr << "LONGEST_INSERT: scheduled intervention " << solution.instance->get_intervention(final_i) << " at time " << final_t << endl;
    #endif

    solution.schedule(final_i, final_t);
}

/*
 * for each unscheduled intervention find start time with shortest duration
 * from these schedule the shortest
 */
void shortest2_insert(Solution &solution) {
    uint_t i_final;
    uint_t t_final;
    uint_t length_min = numeric_limits<uint_t>::max();
    uint_t length_cur;
    uint_t delta_idx;

    // for all interventions
    for (uint_t i : solution.unscheduled) {
        delta_idx = 1;

        // determine start time with shortest duration overall
        for (uint_t t = 1; t <= solution.instance->get_t_max(i); ++t) {
            length_cur = solution.instance->delta[delta_idx++];
            if (length_cur < length_min) {
                length_min = length_cur;
                i_final = i;
                t_final = t;
            }
        }
    }

    #if VERBOSE_HEURISTICS
    cerr << "SHORTEST2_INSERT: scheduled intervention " << solution.instance->get_intervention(i_final) << " at time " << t_final << endl;
    #endif

    solution.schedule(i_final, t_final);
}

/*
 * Returns a vector of first batch_size interventions unscheduled in solution.
 * The interventions are ordered according to their average property, in increasing order.
 */
vector<uint_t> get_unscheduled_subset(Solution &solution, const string& property, fitness_t mu) {
    vector<uint_t> subset;
    uint_t cnt = 0;
    uint_t batch_size;
    vector<pair<uint_t, fitness_t>> avg_properties;

    if (property == "length") {
        avg_properties = solution.instance->get_avg_deltas();
        batch_size = max(1, int(LENGTH_BATCH * solution.instance->get_interventions().size()));
    } else if (property == "cost") {
        avg_properties = solution.instance->get_avg_costs();
        batch_size = max(1, int(COST_BATCH * solution.instance->get_interventions().size()));
    } else if (property == "rd") {
        avg_properties = solution.instance->get_avg_rds();
        batch_size = max(1, int(RD_BATCH * solution.instance->get_interventions().size()));
    } else {
        cerr << "UNKNOWN_PROPERTY: " << property << endl;
        exit(1);
    }

    if (mu > 1) { // Reorder properties high to low
        std::reverse(avg_properties.begin(), avg_properties.end());
    }

    for (auto p:avg_properties) {
        auto i = p.first;
        if (solution.unscheduled.find(i) != solution.unscheduled.end()) { // i is unscheduled
            subset.push_back(i);
            cnt++;
        }
        if (cnt == batch_size) break;
    }

    return subset;
}

void random_with_me_violations(Solution &solution) {
    // Get random intervention
    std::uniform_int_distribution<uint_t> range(0, solution.unscheduled.size() - 1);
    auto it = solution.unscheduled.begin();
    std::advance(it, range(*solution.engine));
    uint_t i = *it;
    // Select t with highest exclusion penalty
    Objective o;
    uint_t max_e_penalty = 0;
    uint_t max_t = 1;
    for (uint_t t = 1; t <= solution.instance->get_t_max(i); ++t) {
        o = solution.estimate_schedule(i, t);
        if (o.exclusion_penalty > max_e_penalty) {
            max_e_penalty = o.exclusion_penalty;
            max_t = t;
        }
    }
//    cout << i << " " << max_t << endl;
    solution.schedule(i, max_t);
}


