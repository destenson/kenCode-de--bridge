#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "bridge/book.h"
#include "utils/https.h"

const char* url = "https://bittrex.com/api/v1.1/";
const char* apikey = "55bbbe56a47046ac858f26110b044b6d";
const char* apisecret = "5d7773773d5c481f9ec1afa67227a7b4";

/***
 * Build a URL compatible with bittrex
 * @param group public, account, or market
 * @param method the method to call
 * @param nonce the calculated nonce
 * @param results where to put the results
 * @returns the resultant string (that is also in the results parameter)
 */
char* bittrex_build_url(const char* group, const char* method, const char* nonce, char** results) {
	int len = strlen(url) + strlen(group) + strlen(method) + strlen(apikey) + 20;
	if (nonce != NULL)
		len += strlen(nonce);
	*results = malloc(len);
	if (strcmp(group, "public") == 0) // public methods don't require apikey or nonce
		sprintf(*results, "%s%s/%s", url, group, method);
	else
		sprintf(*results, "%s%s/%s?apikey=%s&nonce=%s", url, group, method, apikey, nonce);
	return *results;
}

char* book_bittrex_get_book(enum Side side, const char* currency_from, const char* currency_to, char** results) {
	char* side_str = "buy";
	if (side == SIDE_ASKS)
		side_str  = "sell";
	char getorderbook[50];
	sprintf(getorderbook, "getorderbook?market=%s-%s&type=%s&depth=50", currency_from, currency_to, side_str);
	char* url;
	bittrex_build_url("public", getorderbook, NULL, &url);
	utils_https_get(url, results);
	free(url);
	return *results;
}

/**
 * given the book, find the price based on qty
 * @book the results from the query to the bittrex api
 * @qty the amount of the currency we want to exchange
 * @returns the price for the exchange, based solely on the book, will be a negative number on error
 */
double book_bittrex_get_price_from_book(const char* book, double qty) {
	// parse the json until we have the quantity we need
	return 0.0;
}

/***
 * Get a quote from bittrex for the appropriate currency pair and quantity
 * @param side the side of the book to examine
 * @param currency_from the currency that we desire to exchange
 * @param currency_to the currency that we desire to receive
 * @param qty the amount of the currency_from we wish to exchange
 * @returns the price in [currency_from] or a negative number if there was a problem
 */
double book_bittrex_quote(enum Side side, const char* currency_from, const char* currency_to, double qty) {
	// get book from bittrex
	char* book;
	book_bittrex_get_book(side, currency_from, currency_to, &book);
	// calculate the price based on the quantity
	double price = book_bittrex_get_price_from_book(book, qty);
	free(book);
	return price;
}
