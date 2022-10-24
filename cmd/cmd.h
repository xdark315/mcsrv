#ifndef SRV_CMD_H
#define SRV_CMD_H

#include <string>

#define OFF false
#define ON true

using namespace std;

class Cmd {
    private:

        string srv;
        string folder;
        string run;

    public:
        // constructor
        Cmd();

        // get srv status
        bool get_status();

        // start srv
        void start();

        // stop srv
        void stop();

        // restart srv
        void restart();
    };


#endif