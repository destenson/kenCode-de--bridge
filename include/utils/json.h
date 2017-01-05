#include "jsmn.h"

int json_parse(const char* json, jsmntok_t* tokens, int max_length);

/**
 * Find the position of a key
 * @param data the string that contains the json
 * @param tokens the tokens of the parsed string
 * @param tok_length the number of tokens there are
 * @param tag what we're looking for
 * @returns the position of the requested token in the array, or -1
 */
int json_find_token(const char* data, const jsmntok_t* tokens, int tok_length, int start_from, const char* tag);

/**
 * Retrieves the value of a key / value pair from the JSON data
 * @param data the full JSON string
 * @param tokens the array of tokens
 * @param tok_length the number of tokens
 * @param search_from start search from this token onward
 * @param tag what to search for
 * @param result where to put the result. NOTE: allocates memory that must be freed
 * @returns true(1) on success
 */
int json_get_string_value(const char* data, const jsmntok_t* tokens, int tok_length, int search_from, const char* tag, char** result);

/**
 * Retrieves the value of a key / value pair from the JSON data
 * @param data the full JSON string
 * @param tokens the array of tokens
 * @param tok_length the number of tokens
 * @param search_from start search from this token onward
 * @param tag what to search for
 * @param result where to put the result
 * @returns true(1) on success
 */
int json_get_int_value(const char* data, const jsmntok_t* tokens, int tok_length, int search_from, const char* tag, int* result);

/**
 * Retrieves the value of a key / value pair from the JSON data
 * @param data the full JSON string
 * @param tokens the array of tokens
 * @param tok_length the number of tokens
 * @param search_from start search from this token onward
 * @param tag what to search for
 * @param result where to put the result
 * @returns true(1) on success
 */
int json_get_double_value(const char* data, const jsmntok_t* tokens, int tok_length, int search_from, const char* tag, double* result);
