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
// screen -dmS mc
// screen -S mc -X stuff '<cmd>\n'
// screen -XS mc quit

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
    cout << "Starting server" << endl;
    if (system("screen -dmS mc") != 0 ) {
        cout << "Error starting screen" << endl;
    }
    // test avec htop pour debug
    if (system("screen -S mc -X stuff 'htop\n' ") != 0 ) {
        cout << "Error starting server" << endl;
    }
}

void stop() {
    cout << "Stoping server" << endl;
    if (system("screen -S mc -X stuff 'stop\n' ") !=0 ) {
        cout << "Error stoping server" << endl;
    }
    if (system("screen -XS mc quit") !=0 ) {
        cout << "Error stoping screen" << endl;
    }
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