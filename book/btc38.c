/***
 * All of the connectivity for https://www.bittrex.com
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "bridge/book.h"
#include "utils/https.h"
#include "utils/json.h"

const char* btc38_url = "http://api.btc38.com/v1/";
// TODO: these are test values. They need to be replaced
const char* btc38_apikey = "7133a888bd686cff84b2b6a934366d3f";
const char* btc38_apisecret = "70af5f27d79f5ab07822375ae5dbd5e1b50d651766a2d32bf109e1470bdf0e4c";


/***
 * Build a URL compatible with bittrex
 * @param group public, account, or market
 * @param method the method to call
 * @param nonce the calculated nonce
 * @param results where to put the results
 * @returns the resultant string (that is also in the results parameter)
 */
char* btc38_build_url(const char* method, char** results) {
	int len = strlen(btc38_url) + strlen(method) + 20;
	*results = malloc(len);
	sprintf(*results, "%s%s", btc38_url, method);
	return *results;
}

void btc38_all_upper(char* string) {
	for (int i = 0; i < strlen(string); i++) {
		string[i] = toupper(string[i]);
	}
	return;
}

int find_next_market_type(const char* json, jsmntok_t* tokens, size_t tokens_length, size_t start_pos, char** result) {
	// look for "bid"
	int pos = json_find_token(json, tokens, tokens_length, start_pos, "ticker");
	if (pos < 1)
		return 0;
	jsmntok_t curr_token = tokens[pos-2];
	if (curr_token.type != JSMN_STRING) {
		return 0;
	}
	// allocate memory
	int str_len = curr_token.end - curr_token.start;
	*result = malloc(str_len + 1);
	if (*result == NULL)
		return 0;
	strncpy(*result, &json[curr_token.start], str_len);
	(*result)[str_len] = 0;
	return pos;
}

struct Market* btc38_parse_market(const char* json, const char* base_currency) {
	struct Market* head = NULL;
	struct Market* current = head;
	struct Market* last = head;
	jsmntok_t tokens[65535];

	// parse json
	int total_tokens = json_parse(json, tokens, 65535);
	if (total_tokens < 0)
		return NULL;
	int base_currency_length = strlen(base_currency)+1;
	char base_currency_upper[base_currency_length];
	strcpy(base_currency_upper, base_currency);
	btc38_all_upper(base_currency_upper);
	int start_pos = 1;
	// loop through markets
	while (start_pos != 0) {
		current = market_new();
		if (current == NULL) {
			free(head);
			return NULL;
		}
		// market_currency
		start_pos = find_next_market_type(json, tokens, total_tokens, start_pos, &current->market_currency);
		if (start_pos == 0) {
			free(current);
			continue;
		}
		// base_currency
		current->base_currency = malloc(base_currency_length);
		strcpy(current->base_currency, base_currency_upper);
		// market_name
		current->market_name = malloc(base_currency_length + strlen(current->market_currency) + 13);
		sprintf(current->market_name, "?c=%s&mk_type=%s", base_currency, current->market_currency);
		// change market_currency to upper case
		btc38_all_upper(current->market_currency);
		// add it to the list
		if (head == NULL) {
			head = current;
		} else {
			last->next = current;
		}
		last = current;
		start_pos++;
	}
	return head;
}

struct Book* btc38_parse_book(const char* json) {
	struct Book* book = NULL;
	struct Book* current = book;
	struct Book* last = book;
	jsmntok_t tokens[65535];

	// parse json
	int tok_no = json_parse(json, tokens, 65535);
	if (tok_no < 0)
		return NULL;
	// find where is the "sell"
	int start_pos = 0;
	int sell_pos = json_find_token(json, tokens, tok_no, start_pos, "asks");
	if (sell_pos == 0)
		sell_pos = tok_no;
	// start building the book
	start_pos = json_find_token(json, tokens, tok_no, start_pos, "bids");
	start_pos += 3;
	// loop for the bid
	while (start_pos != 0 && start_pos < sell_pos) {
		double quantity = 0;
		double rate = 0;
		json_get_double(json, tokens[start_pos], &quantity);
		start_pos++;
		json_get_double(json, tokens[start_pos], &rate);
		start_pos += 2;
		if (start_pos < sell_pos) {
			current = book_new();
			current->bid_qty = quantity;
			current->bid_price = rate;
			if (book == NULL) {
				book = current;
				last = current;
			} else {
				last->next = current;
			}
			last = current;
		} else {
			start_pos = 0;
		}
	}
	// loop for the ask
	start_pos = sell_pos + 3;
	current = book;
	while (start_pos != 0) {
		double quantity = 0;
		double rate = 0;
		json_get_double(json, tokens[start_pos], &quantity);
		start_pos++;
		json_get_double(json, tokens[start_pos], &rate);
		start_pos += 2;
		if (start_pos > tok_no) {
			start_pos = 0;
			continue;
		}
		if (current == NULL) {
			// need to add new Book to list
			current = book_new();
			if (last == NULL) {
				// we don't have "book" yet
				book = current;
			} else {
				last->next = current;
			}
		}
		current->ask_qty = quantity;
		current->ask_price = rate;
		last = current;
		current = current->next;
	} // end of while loop
	return book;
}

struct Book* btc38_get_books(const struct Market* market) {
	char getorderbook[strlen(market->market_name) + 10];
	sprintf(getorderbook, "%s%s", "depth.php", market->market_name);
	char* url;
	btc38_build_url( getorderbook, &url);
	char* results;
	utils_https_get(url, &results);
	free(url);
	struct Book* book = btc38_parse_book(results);
	free(results);
	return book;
}

struct Market* btc38_get_markets() {
	char* url;
	btc38_build_url("ticker.php?c=all&mk_type=btc", &url);
	char* json;
	int retVal = utils_https_get(url, &json);
	if (retVal != 0) {
		fprintf(stderr, "HTTP call to %s returned %d\n", url, retVal);
		free(url);
		return NULL;
	}
	free(url);
	// yes this is crazy, but diagnosing a problem
	// start of btc38_parse_market(results, "btc");
	struct Market* head = NULL;
	struct Market* current = head;
	struct Market* last = head;
	const char* base_currency = "btc";
	jsmntok_t tokens[65535];

	// parse json
	int total_tokens = json_parse(json, tokens, 65535);
	if (total_tokens < 0)
		return NULL;
	int base_currency_length = strlen(base_currency)+1;
	char base_currency_upper[base_currency_length];
	strcpy(base_currency_upper, base_currency);
	btc38_all_upper(base_currency_upper);
	int start_pos = 1;
	// loop through markets
	while (start_pos != 0) {
		current = market_new();
		if (current == NULL) {
			free(head);
			return NULL;
		}
		// market_currency
		start_pos = find_next_market_type(json, tokens, total_tokens, start_pos, &current->market_currency);
		if (start_pos == 0) {
			free(current);
			continue;
		}
		// base_currency
		current->base_currency = malloc(base_currency_length);
		strcpy(current->base_currency, base_currency_upper);
		// market_name
		current->market_name = malloc(base_currency_length + strlen(current->market_currency) + 13);
		sprintf(current->market_name, "?c=%s&mk_type=%s", base_currency, current->market_currency);
		// change market_currency to upper case
		btc38_all_upper(current->market_currency);
		// add it to the list
		if (head == NULL) {
			head = current;
		} else {
			last->next = current;
		}
		last = current;
		start_pos++;
	}
	struct Market* market = head;
	// end of btc_38_parse_market(results, "btc");
	//struct Market* market = btc38_parse_market(results, "btc");
	free(json);
	return market;
}
