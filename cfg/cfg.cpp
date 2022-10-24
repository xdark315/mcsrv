#include "cfg.h"
#include <fstream>
#include <iostream>
#include <string>
#include <stdlib.h>

using namespace std;
using json = nlohmann::json;

int cfg_gen() {
    string homedir = getenv("HOME");
    ofstream cfg_file;
    cout << "Creating config file in " << homedir + CFG_FILE << endl;
    if (system(("mkdir -p " + homedir + "/.mcsrv").c_str()) != 0) {
        cout << "Error creating config folder" << endl;
        exit(EXIT_FAILURE);
    }
    cfg_file.open(homedir + CFG_FILE);
    cfg_file << "{\n";
    cfg_file << "    \"srv\": \"\",\n";
    cfg_file << "    \"folder\": \"\",\n";
    cfg_file << "    \"run\": \"\"\n";
    cfg_file << "}";
    cfg_file.close();
    return EXIT_SUCCESS;
}

json get_cfg() {
    string homedir = getenv("HOME");
    ifstream cfg_file;
    cfg_file.open(homedir + CFG_FILE);
    if (!cfg_file.is_open()) {
        cfg_gen();
        cfg_file.open(homedir + CFG_FILE);
    }
    json j;
    string s = "";
    string line;
    while (getline(cfg_file, line)) {
        s += line;
    }
    j = json::parse(s);
    cfg_file.close();
    return j;
}