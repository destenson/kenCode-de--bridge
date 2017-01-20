#include <stdlib.h>
#include <string.h>

#include "utils/jsmn.h"

int json_parse(const char* json, jsmntok_t* tokens, int max_length) {
	jsmn_parser parser;
	jsmn_init(&parser);
	return jsmn_parse(&parser, json, strlen(json), tokens, max_length);
}

/**
 * Find the position of a key
 * @param data the string that contains the json
 * @param tokens the tokens of the parsed string
 * @param tok_length the number of tokens there are
 * @param tag what we're looking for
 * @returns the position of the requested token in the array, or -1
 */
int json_find_token(const char* data, const jsmntok_t* tokens, int tok_length, int start_from, const char* tag) {
	for(int i = start_from; i < tok_length; i++) {
		jsmntok_t curr_token = tokens[i];
		if ( curr_token.type == JSMN_STRING) {
			// convert to string
			int str_len = curr_token.end - curr_token.start;
			char str[str_len + 1];
			strncpy(str, &data[curr_token.start], str_len );
			str[str_len] = 0;
			if (strcmp(str, tag) == 0)
				return i;
		}
	}
	return -1;
}

/**
 * Retrieves the value of a key / value pair from the JSON data
 * @param data the full JSON string
 * @param tokens the array of tokens
 * @param tok_length the number of tokens
 * @param search_from start search from this token onward
 * @param tag what to search for
 * @param result where to put the result. NOTE: allocates memory that must be freed
 * @returns the position where something was found
 */
int json_get_string_value(const char* data, const jsmntok_t* tokens, int tok_length, int search_from, const char* tag, char** result) {
	int pos = json_find_token(data, tokens, tok_length, search_from, tag);
	if (pos < 0)
		return 0;
	pos++;
	jsmntok_t curr_token = tokens[pos];
	if (curr_token.type == JSMN_PRIMITIVE) {
		// a null
		*result = NULL;
	}
	if (curr_token.type != JSMN_STRING)
		return 0;
	// allocate memory
	int str_len = curr_token.end - curr_token.start;
	*result = malloc(sizeof(char) * str_len + 1);
	if (*result == NULL)
		return 0;
	// copy in the string
	strncpy(*result, &data[curr_token.start], str_len);
	(*result)[str_len] = 0;
	return pos;
}

/**
 * Retrieves the value of a key / value pair from the JSON data
 * @param data the full JSON string
 * @param tokens the array of tokens
 * @param tok_length the number of tokens
 * @param search_from start search from this token onward
 * @param tag what to search for
 * @param result where to put the result
 * @returns the position where something was found
 */
int json_get_int_value(const char* data, const jsmntok_t* tokens, int tok_length, int search_from, const char* tag, int* result) {
	int pos = json_find_token(data, tokens, tok_length, search_from, tag);
	if (pos < 0)
		return 0;
	pos++;
	jsmntok_t curr_token = tokens[pos];
	if (curr_token.type != JSMN_PRIMITIVE)
		return 0;
	// allocate memory
	int str_len = curr_token.end - curr_token.start;
	char str[str_len + 1];
	// copy in the string
	strncpy(str, &data[curr_token.start], str_len);
	str[str_len] = 0;
	if (strcmp(str, "true") == 0)
		*result = 1;
	else if (strcmp(str, "false") == 0)
		*result = 0;
	else if (strcmp(str, "null") == 0) // what should we do here?
		*result = 0;
	else // its a real number
		*result = atoi(str);
	return pos;
}

/**
 * Retrieves the value of a key / value pair from the JSON data
 * @param data the full JSON string
 * @param tokens the array of tokens
 * @param tok_length the number of tokens
 * @param search_from start search from this token onward
 * @param tag what to search for
 * @param result where to put the result
 * @returns the position where something was found
 */
int json_get_double_value(const char* data, const jsmntok_t* tokens, int tok_length, int search_from, const char* tag, double* result) {
	int pos = json_find_token(data, tokens, tok_length, search_from, tag);
	if (pos < 0)
		return 0;
	pos++;
	jsmntok_t curr_token = tokens[pos];
	if (curr_token.type != JSMN_PRIMITIVE)
		return 0;
	// allocate memory
	int str_len = curr_token.end - curr_token.start;
	char str[str_len + 1];
	// copy in the string
	strncpy(str, &data[curr_token.start], str_len);
	str[str_len] = 0;
	if (strcmp(str, "true") == 0)
		*result = 1;
	else if (strcmp(str, "false") == 0)
		*result = 0;
	else if (strcmp(str, "null") == 0) // what should we do here?
		*result = 0;
	else // its a real number
		*result = atof(str);
	return pos;
}

/**
 * Retrieves the value of a double pointed to by token_no
 * @param data the full JSON string
 * @param curr_token the tokens that contains the position of the data
 * @param result where to put the result
 */
void json_get_double(const char* data, const jsmntok_t curr_token, double* result) {
	if (curr_token.type != JSMN_PRIMITIVE && curr_token.type != JSMN_STRING) // we may have to convert from string
		return;
	// allocate memory
	int str_len = curr_token.end - curr_token.start;
	char str[str_len + 1];
	str[0] = 0;
	// copy in the string
	strncpy(str, &data[curr_token.start], str_len);
	str[str_len] = 0;
	if (strcmp(str, "true") == 0)
		*result = 1;
	else if (strcmp(str, "false") == 0)
		*result = 0;
	else if (strcmp(str, "null") == 0) // what should we do here?
		*result = 0;
	else // its a real number
		*result = atof(str);
	return;
}

/**
 * Retrieves the value of a string pointed to by token_no
 * @param data the full JSON string
 * @param curr_token the tokens that contains the position of the data
 * @param result where to put the result
 */
void json_get_string(const char* data, const jsmntok_t curr_token, char** result) {
	if (curr_token.type != JSMN_STRING) // we may have to convert from string
		return;
	// allocate memory
	int str_len = curr_token.end - curr_token.start;
	*result = malloc(str_len + 1);
	memset(*result, 0, str_len + 1);
	// copy in the string
	strncpy(*result, &data[curr_token.start], str_len);
	return;
}
