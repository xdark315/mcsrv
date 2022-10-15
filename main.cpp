#include "args/args.h"
#include "cmd/cmd.h"
#include <iostream>
#include <string>
#include <fstream>

using namespace std;


int main(int argc, char *argv[]) {
    read_args(argc, argv);
    return EXIT_SUCCESS;
}