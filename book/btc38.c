/***
 * All of the connectivity for https://www.bittrex.com
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "mbedtls/md5.h"
#include "bridge/book.h"
#include "utils/https.h"
#include "utils/json.h"
#include "utils/logging.h"

const char* btc38_url = "http://api.btc38.com/v1/";
// TODO: these are test values. They need to be replaced
const char* btc38_userid = "179172";
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
	if (*results) {
		sprintf(*results, "%s%s", btc38_url, method);
	}
	return *results;
}

void btc38_all_upper(char* string) {
	int i;
	for (i = 0; i < strlen(string); i++) {
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
		if (current->base_currency) {
			strcpy(current->base_currency, base_currency_upper);
		}
		// market_name
		current->market_name = malloc(base_currency_length + strlen(current->market_currency) + 13);
		if (current->market_name) {
			sprintf(current->market_name, "?c=%s&mk_type=%s", base_currency, current->market_currency);
		}
		// change market_currency to upper case
		btc38_all_upper(current->market_currency);
		// add fee
		current->fee = 0.7;
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
	struct HttpConnection* connection = utils_https_new();
	utils_https_get(connection, url, &results);
	utils_https_free(connection);
	free(url);
	struct Book* book = btc38_parse_book(results);
	free(results);
	return book;
}

struct Market* btc38_get_markets() {
	// btc
	char* url;
	btc38_build_url("ticker.php?c=all&mk_type=btc", &url);
	char* json;
	struct HttpConnection* connection = utils_https_new();
	int retVal = utils_https_get(connection, url, &json);
	utils_https_free(connection);
	if (retVal != 0) {
		fprintf(stderr, "HTTP call to %s returned %d\n", url, retVal);
		free(url);
		return NULL;
	}
	free(url);
	struct Market* btc_market = btc38_parse_market(json, "btc");
	free(json);

	// cny
	btc38_build_url("ticker.php?c=all&mk_type=cny", &url);
	connection = utils_https_new();
	retVal = utils_https_get(connection, url, &json);
	utils_https_free(connection);
	if (retVal != 0) {
		fprintf(stderr, "HTTP call to %s returned %d\n", url, retVal);
		free(url);
		return btc_market;
	}
	free(url);
	struct Market* cny_market = btc38_parse_market(json, "cny");
	free(json);
	// now merge
	struct Market* market = btc_market;
	struct Market* last = market;
	while (last->next != NULL)
		last = last->next;
	last->next = cny_market;
	return market;
}

int btc38_limit_buy(const struct Market* currencyPair, double rate, double quantity) {
	return 0;
}

int btc38_limit_sell(const struct Market* currencyPair, double rate, double quantity) {
	return 0;
}

int btc38_market_buy(const struct Market* currencyPair, double quantity) {
	return 0;
}

int btc38_market_sell(const struct Market* currencyPair, double quantity) {
	return 0;
}

struct Balance* btc38_parse_balance(const char* currency, const char* json) {
	struct Balance* retVal = NULL;
	size_t max_tokens = strlen(json) / 4;
	jsmntok_t tokens[max_tokens];
	int success = 0;
	int token_position = 0;

	// parse json
	int num_tokens = json_parse(json, tokens, max_tokens);
	if (num_tokens < 0)
		goto exit;;

	retVal = balance_new();
	if (retVal == NULL)
		goto exit;

	// find the currency
	char currency_string[17];
	currency_string[0] = tolower(currency[0]);
	currency_string[1] = tolower(currency[1]);
	currency_string[2] = tolower(currency[2]);
	strcpy(&currency_string[3], "_balance");
	token_position = json_find_token(json, tokens, num_tokens, token_position, currency_string);
	if (token_position < 0)
		goto exit;

	// add the currency to the Balance object
	retVal->currency = (char*)malloc(strlen(currency) + 1);
	if (retVal->currency) {
		strcpy(retVal->currency, currency);
	}

	json_get_double(json, tokens[++token_position], &retVal->balance);

	strcpy(&currency_string[3], "_balance_lock");
	token_position = json_find_token(json, tokens, num_tokens, token_position, currency_string);
	if (token_position < 0)
		goto exit;
	json_get_double(json, tokens[++token_position], &retVal->pending);

	retVal->available = retVal->balance - retVal->pending;

	success = 1;
	exit:
	if (!success) {
		if (retVal != NULL) {
			free(retVal);
			retVal = NULL;
		}
	}
	return retVal;
}

struct Balance* btc38_balance(const char* currency) {
	// build url
	const char* template = "%sgetMyBalance.php";
	int url_len = strlen(btc38_url) + strlen(template);
	char url[url_len];
	sprintf(url, template, btc38_url);
	// parameters
	struct HttpConnection* connection = utils_https_new();
	utils_https_add_post_parameter(connection, "key", btc38_apikey);
	char* nonce = utils_https_get_nonce();
	utils_https_add_post_parameter(connection, "time", nonce);
	// compute md5
	unsigned char md5_input[strlen(btc38_apikey) + strlen(btc38_userid) + strlen(btc38_apisecret) + strlen(nonce) + 4];
	sprintf((char*)md5_input, "%s_%s_%s_%s", btc38_apikey, btc38_userid, btc38_apisecret, nonce);
	free(nonce);
	unsigned char md5[16];
	mbedtls_md5(md5_input, strlen((char*)md5_input), md5);
	size_t md5_encoded_length;
	unsigned char* md5_encoded = utils_https_bytes_to_hex_string(md5, 16, &md5_encoded_length);
	utils_https_add_post_parameter(connection, "md5", (char*)md5_encoded);
	free(md5_encoded);
	// finally do the call
	char* json;
	utils_https_post(connection, url, &json);
	utils_https_free(connection);
	struct Balance* retVal = btc38_parse_balance(currency, json);
	if (retVal == NULL) {
		fprintf(stderr, "Error retrieving balance for %s on btc38.com: %s\n", currency, json);
	}
	free(json);
	return retVal;
}

