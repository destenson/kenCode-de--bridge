#include <stdlib.h>

#include "../include/bridge/book.h"

int test_book_mock() {
	struct Book* mock = book_for_vendor("mock");

	if (mock->quote(SIDE_BIDS, "BTC", "USD", 2) > 0)
		return 0;

	free(mock);

	return 1;
}

int test_book_bittrex() {
	int ret = 0;
	struct Book* bittrex = book_for_vendor("bittrex");
	double result = bittrex->quote(SIDE_BIDS, "BTC", "LTC", 1);
	if (result < 0)
		goto exit;
	ret = 1;

	exit:
	free(bittrex);
	return ret;
}

// declared here for testing, defined in book/bittrex.c
char* bittrex_build_url(const char* group, const char* method, const char* nonce, char** results);

int test_book_bittrex_url() {
	int ret = 0;
	char* results;

	// test a basic url
	char* retVal = bittrex_build_url("public", "getmarkets", NULL, &results);
	if (strcmp(retVal, results) != 0) {
		goto exit;
	}
	if (strcmp(retVal, "https://bittrex.com/api/v1.1/public/getmarkets") != 0) {
		goto exit;
	}
	free(results);
	// test a more complicated url
	bittrex_build_url("market", "buylimit", "nonce", &results);
	if (strcmp(results, "https://bittrex.com/api/v1.1/market/buylimit?apikey=55bbbe56a47046ac858f26110b044b6d&nonce=nonce") != 0)
		goto exit;
	ret = 1;


	exit: // clean up
	free(results);

	return ret;
}
