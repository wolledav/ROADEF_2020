#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <fstream>
#include "json.hpp"
#include "../instance/instance.hpp"
#include "json_objnames.hpp"
#include "../ranges.hpp"
#include "../util.hpp"
#include "../types.hpp"

using namespace std;
using json = nlohmann::json;

/* hanles parsing json file and storing the values to instance object */
class Parser {
    private:
        const string path_to_file; /* path to parsed json file */
        json j; /* library object which parses the file */
        void process_exclusions(Instance *instance); /* extracts exclusions from json to Instance object */
        static void process_avg_properties(Instance *instance); /* determine average values of some intervention properties */
    public:
        Parser(string path_to_file);
        void load(); /* reads file and fills j object */
        void process(Instance *instance); /* reads j object and translates data to instance object */
};

#endif
