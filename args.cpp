#include "args.h"
#include "cmd.h"
#include <iostream>
#include <string>

using namespace std;

void read_args(int argc, char *argv[]) {
    if (argc == 2){

        string arg = argv[1];
        if (arg == "-h" || arg == "--help"){
            help();
        }
        else if (arg == "start"){
            start();
        }
        else if (arg == "stop"){
            stop();
        }
        else if (arg == "restart"){
            restart();
        }
        else if (arg == "status"){
            status();
        }
        else {
            error();
        }
    }
    else {
        error();
    }
}

void help() {
    cout << "Usage: " << endl;
    cout << "  -h, --help\t\tShow this help message and exit" << endl;
    cout << "  start\t\t\tStart the minecraft serveur" << endl;
    cout << "  stop\t\t\tStop the minecraft serveur" << endl;
    cout << "  restart\t\tRestart the minecraft serveur" << endl;
    cout << "  status\t\tShow the minecraft serveur status" << endl;
}

void error() {
    cout << "Error: Invalid arguments" << endl;
    cout << "Try 'mcsrv --help' for more information." << endl;
}

