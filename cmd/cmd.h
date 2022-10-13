#ifndef SRV_CMD_H
#define SRV_CMD_H

#define ERROR -1
#define OFF false
#define ON true

// get srv status
bool get_status();

// start srv
void start();

// stop srv
void stop();

// restart srv
void restart();

#endif