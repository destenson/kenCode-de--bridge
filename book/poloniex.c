

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

const char* poloniex_url = "https://poloniex.com/";
// TODO: these are test values. They need to be replaced
const char* poloniex_apikey = "TFNUBJYV-84RXQBXI-3PONC8MW-3ATNRJ9B";
const char* poloniex_apisecret = "0ce4c944bce4afa24b9e68166699002b9f780ec4b0b23a6b30b5dc273dd28817bf31c15b4880c89d2de0669f0c7224925fc61ae5e98e59e23fd3e21751c5bf27";


/***
 * Build a URL compatible with bittrex
 * @param group public, account, or market
 * @param method the method to call
 * @param nonce the calculated nonce
 * @param results where to put the results
 * @returns the resultant string (that is also in the results parameter)
 */
char* poloniex_build_url(const char* method, char** results) {
	int len = strlen(poloniex_url) + strlen(method) + 1;
	*results = malloc(len);
	sprintf(*results, "%s%s", poloniex_url, method);
	return *results;
}

int poloniex_find_next_market_type(const char* json, jsmntok_t* tokens, size_t tokens_length, size_t start_pos, char** result) {
	// look for "bid"
	int pos = json_find_token(json, tokens, tokens_length, start_pos, "id");
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


struct Market* poloniex_parse_market(const char* json) {
	struct Market* head = NULL;
	struct Market* current = head;
	struct Market* last = head;
	jsmntok_t tokens[65535];

	// parse json
	int total_tokens = json_parse(json, tokens, 65535);
	if (total_tokens < 0)
		return NULL;
	int start_pos = 1;
	// loop through markets
	while (start_pos != 0) {
		current = market_new();
		if (current == NULL) {
			free(head);
			return NULL;
		}
		// market_currency
		char* full_ticker;
		start_pos = poloniex_find_next_market_type(json, tokens, total_tokens, start_pos, &full_ticker);
		if (start_pos == 0) {
			free(current);
			continue;
		}
		// split full_ticker into 2 parts
		char* pos = strchr(full_ticker, '_');
		if (pos == NULL) {
			market_free(current);
			continue;
		}
		current->base_currency = malloc(pos - full_ticker + 1);
		strncpy(current->base_currency, full_ticker, pos - full_ticker);
		current->base_currency[pos - full_ticker] = 0;
		current->market_currency = malloc(strlen(&pos[1]) + 1);
		strcpy(current->market_currency, &pos[1]);
		// market_name
		current->market_name = malloc(strlen(full_ticker) + 1);
		strcpy(current->market_name, full_ticker);
		free(full_ticker);
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

struct Market* poloniex_get_markets() {
	// btc
	char* url;
	poloniex_build_url("public?command=returnTicker", &url);
	char* json;
	int retVal = utils_https_get(url, &json);
	if (retVal != 0) {
		fprintf(stderr, "HTTP call to %s returned %d\n", url, retVal);
		free(url);
		return NULL;
	}
	free(url);
	struct Market* market = poloniex_parse_market(json);
	free(json);

	return market;
}

struct Book* poloniex_parse_book(const char* json) {
	struct Book* book = NULL;
	struct Book* current = book;
	struct Book* last = book;
	jsmntok_t tokens[65535];

	// parse json
	int tok_no = json_parse(json, tokens, 65535);
	if (tok_no < 0)
		return NULL;
	// find where is the "asks"
	int start_pos = 0;
	int sell_pos = json_find_token(json, tokens, tok_no, start_pos, "asks");
	if (sell_pos == 0)
		sell_pos = tok_no;
	// find where is the "bids"
	int bid_pos = json_find_token(json, tokens, tok_no, start_pos, "bids");
	if (bid_pos == 0)
		return NULL;
	start_pos = sell_pos + 3;
	// loop for the asks
	while (start_pos != 0 && start_pos < bid_pos) {
		double quantity = 0;
		double rate = 0;
		json_get_double(json, tokens[start_pos], &quantity);
		start_pos++;
		json_get_double(json, tokens[start_pos], &rate);
		start_pos += 2;
		if (start_pos < bid_pos) {
			current = book_new();
			current->ask_qty = quantity;
			current->ask_price = rate;
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
	// loop for the bids
	start_pos = bid_pos + 3;
	current = book;
	while (start_pos != 0 && start_pos < tok_no) {
		double quantity = 0;
		double rate = 0;
		json_get_double(json, tokens[start_pos], &quantity);
		start_pos++;
		if (start_pos >= tok_no) {
			start_pos = 0;
			continue;
		}
		json_get_double(json, tokens[start_pos], &rate);
		start_pos += 2;
		if (start_pos >= tok_no) {
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
		current->bid_qty = quantity;
		current->bid_price = rate;
		last = current;
		current = current->next;
	} // end of while loop
	return book;
}

struct Book* poloniex_get_books(const struct Market* market) {
	char getorderbook[strlen(market->market_name) + 60];
	sprintf(getorderbook, "public?command=returnOrderBook&currencyPair=%s&depth=10", market->market_name);
	char* url;
	poloniex_build_url(getorderbook, &url);
	char* results;
	utils_https_get(url, &results);
	free(url);
	struct Book* book = poloniex_parse_book(results);
	free(results);
	return book;
}

