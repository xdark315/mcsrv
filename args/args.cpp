#include "args.h"
#include "../cmd/cmd.h"
#include <iostream>
#include <string>

using namespace std;

void read_args(int argc, char *argv[]) {
    Cmd cmd;
    if (argc == 2){

        string arg = argv[1];
        if (arg == "-h" || arg == "--help"){
            help();
        }
        else if (arg == "start"){
            cmd.start();
        }
        else if (arg == "stop"){
            cmd.stop();
        }
        else if (arg == "restart"){
            cmd.restart();
        }
        else if (arg == "status"){
            print_status(cmd.get_status());
        }
        else {
            error();
        }
    }
    else {
        error();
    }
}

void print_status(bool status) {
    if (status){
        cout << "ON" << endl;
    }
    else {
        cout << "OFF" << endl;
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

