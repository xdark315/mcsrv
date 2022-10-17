#include "cmd.h"
#include "../json/json.hpp"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
#include <cstdlib>

using namespace std;
using json = nlohmann::json;

// running command and get the result in a string
string exec(const char* cmd);

// get string from all line in txt file
string get_string_from_file(string path);

Cmd::Cmd() {
    string s = get_string_from_file("cfg/mcsrv.json");
    json j = json::parse(s);
    srv = j["srv"];
    folder = j["folder"];
    run = j["run"];
}

bool Cmd::get_status() {

    string s = exec("pgrep java");

    string buffer = "";
    for (size_t i = 0; i < s.length(); i++) {
        if (iswspace(s[i])) {
            string pwdx = exec(("pwdx " + buffer).c_str());
            string s = pwdx.substr(pwdx.find_last_of(" ") + 1);
            string path = s.substr (0,s.length()-1);
            if (path == folder) {
                return ON;
            }
            buffer = "";
        }
        else {
            buffer += s[i];
        }
    }
    return OFF;
}

void Cmd::start() {
    if (get_status()){
        cout << "Server is already started" << endl;
        return;
    }
    cout << "Starting server" << endl;
    if (system("screen -dmS mc") != 0 ) {
        cout << "Error starting screen" << endl;
    }
    string cd_cmd = "screen -S mc -X stuff 'cd " + folder +"\n'" ;
    cout << cd_cmd << endl;
    string run_cmd = "screen -S mc -X stuff './" + run + "\n' ";
    if (system(cd_cmd.c_str()) != 0) {
        cout << "Error cd server foler" << endl;
    }
    if (system(run_cmd.c_str()) != 0 ) {
        cout << "Error starting server" << endl;
    }
}

void Cmd::stop() {
    if (!get_status()){
        cout << "Server is already stopped" << endl;
        return;
    }
    cout << "Stoping server" << endl;
    if (system("screen -S mc -X stuff 'stop\n' ") !=0 ) {
        cout << "Error stoping server" << endl;
    }
    while (get_status()) {
        cout << "Waiting for server to stop" << endl;
        sleep(1);
    }
    if (system("screen -XS mc quit") !=0 ) {
        cout << "Error stoping screen" << endl;
    }
}

void Cmd::restart() {
    stop();
    start();
}

string exec(const char* cmd) {
    array<char, 128> buffer;
    string result;
    unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

string get_string_from_file(string path) {
    ifstream file(path);
    string str;
    string buffer;
    while (getline(file, buffer)) {
        str += buffer;
    }
    return str;
}