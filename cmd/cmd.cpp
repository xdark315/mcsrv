#include "cmd.h"
#include <iostream>

using namespace std;

int status() {
    int status = OFF;
    switch (status) {
        case ON:
            cout << "ON" << endl;
            return ON;
        case OFF:
            cout << "OFF" << endl;
            return OFF;
        default:
            cout << "ERROR" << endl;
            return ERROR;
    }
}

void start() {
    cout << "Start" << endl;
}

void stop() {
    cout << "Stop" << endl;
}

void restart() {
    cout << "Restart" << endl;
}

