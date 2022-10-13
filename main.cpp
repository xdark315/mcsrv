#include "args/args.h"
#include "cmd/cmd.h"
#include "json/json.hpp"
#include <iostream>
#include <string>
#include <fstream>

using namespace std;
using json = nlohmann::json;

// get string from all line in txt file
string get_string_from_file(string path) {
    ifstream file(path);
    string str;
    string buffer;
    while (getline(file, buffer)) {
        str += buffer;
    }
    return str;
}

int main(int argc, char *argv[]) {
    read_args(argc, argv);
    string s = get_string_from_file("cfg/mcsrv.json");
    json j = json::parse(s);
    cout << j.value("folder","a") << endl;
    return EXIT_SUCCESS;
}