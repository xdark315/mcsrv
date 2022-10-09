#ifndef SRV_CMD_H
#define SRV_CMD_H

#define ERROR -1
#define OFF 0
#define ON 1

// get srv status
int status();

// start srv
void start();

// stop srv
void stop();

// restart srv
void restart();

#endif