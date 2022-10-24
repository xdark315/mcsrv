#ifndef CFG_GEN_H
#define CFG_GEN_H

#include "../json/json.hpp"

#define CFG_FILE "/.mcsrv/config.cfg"

using json = nlohmann::json;

// create config file and set default values in /etc/mcsrv/config.cfg
// return EXIT_SUCCESS if success
// return EXIT_FAILURE if error
int cfg_gen();

// get json from config file in /etc/mcsrv/config.cfg 
// if config file not exist, create it
json get_cfg();

#endif