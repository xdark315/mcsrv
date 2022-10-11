#include "cmd.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
#include <cstdlib>

using namespace std;

string exec(const char* cmd);

bool status() {
    // pgrep java
    // pwdx 1234

    string s = exec("pgrep java");

    string buffer = "";
    for (size_t i = 0; i < s.length(); i++) {
        if (iswspace(s[i])) {
            string pwdx = exec(("pwdx " + buffer).c_str());
            string s = pwdx.substr(pwdx.find_last_of(" ") + 1);
            string path = s.substr (0,s.length()-1);
            if (path == "/home/antoine/atm7") {
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

void start() {
    cout << "Start" << endl;
}

void stop() {
    cout << "Stop" << endl;
}

void restart() {
    stop();
    start();
}

// running command and get the result in a string
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