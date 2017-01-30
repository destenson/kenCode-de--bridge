#include <stdio.h>
#include <string.h>

#include "utils/logging.h"

/***
 * Logging...
 */


int loglevel(int log_level) {
	return log_level > CURRENT_LOGLEVEL;
}

void logit(int log_level, char* message) {
	if (loglevel(log_level)) {
		fprintf(stderr, "%s\n", message);
	}
	return;
}

void logit_int(int log_level, char* message, int param) {
	if (loglevel(log_level)) {
		char msg[strlen(message) + 12];
		sprintf(msg, message, param);
		logit(log_level, msg);
	}
	return;
}

void logit_double(int log_level, char* message, double param) {
	if (loglevel(log_level)) {
		char msg[strlen(message) + 20];
		sprintf(msg, message, param);
		logit(log_level, msg);
	}
	return;
}

void logit_string(int log_level, char* message, char* param) {
	if (loglevel(log_level)) {
		char msg[strlen(message) + strlen(param) + 1];
		sprintf(msg, message, param);
		logit(log_level, msg);
	}
}
