#pragma once

/***
 * Logging...
 */

#define LOGLEVEL_DEBUG 1
#define LOGLEVEL_ERROR 2
#define CURRENT_LOGLEVEL 0

int loglevel(int log_level);

void logit(int log_level, char* message);

void logit_int(int log_level, char* message, int param);

void logit_string(int log_level, char* message, char* param);
