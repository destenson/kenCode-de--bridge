#include <stdlib.h>

#include "../include/bridge/book.h"

int test_book_mock() {
	int retVal = 0;
	struct Vendor* vendor = NULL;
	struct Market* market = NULL;
	struct Book* book = NULL;

	vendor = vendor_get("mock");
	if (vendor == NULL)
		goto exit;
	market = vendor->markets_get();
	if (market == NULL)
		goto exit;
	book = vendor->books_get(market);
	if (book == NULL)
		goto exit;

	if (book->bid_price <= 0)
		goto exit;

	retVal = 1;

	exit:

	if (book != NULL)
		book_free(book);
	if (market != NULL)
		market_free(market);
	if (vendor != NULL)
		free(vendor);

	return retVal;
}
int test_market_bittrex() {
	int ret = 0;
	struct Vendor* bittrex = NULL;
	struct Market* market = NULL;

	bittrex = vendor_get("bittrex");
	if (bittrex == NULL)
		goto exit;
	market = bittrex->markets_get();
	if (market == NULL)
		goto exit;

	if (market->next == NULL)
		goto exit;

	ret = 1;

	exit:
	if (market != NULL)
		market_free(market);
	if (bittrex != NULL)
		free(bittrex);
	return ret;

}
int test_book_btc38() {
	int ret = 0;
	struct Vendor* vendor = NULL;
	struct Market* market = NULL;
	struct Book* book = NULL;
	const struct Market* btcltc = NULL;

	vendor = vendor_get("btc38");
	if (vendor == NULL)
		goto exit;
	market = vendor->markets_get();
	if (market == NULL)
		goto exit;
	btcltc = market_get(market, "BTC", "QRK");
	if (btcltc == NULL)
		goto exit;

	book = vendor->books_get(btcltc);

	if (book == NULL)
		goto exit;

	if (book->next == NULL)
		goto exit;

	ret = 1;

	exit:
	if (book != NULL)
		book_free(book);
	if (market != NULL)
		market_free(market);
	if (vendor != NULL)
		free(vendor);
	return ret;
}

int test_book_bittrex() {
	int ret = 0;
	struct Vendor* bittrex = NULL;
	struct Market* market = NULL;
	struct Book* book = NULL;
	const struct Market* btcltc = NULL;

	bittrex = vendor_get("bittrex");
	if (bittrex == NULL)
		goto exit;
	market = bittrex->markets_get();
	if (market == NULL)
		goto exit;
	btcltc = market_get(market, "BTC", "LTC");
	if (btcltc == NULL)
		goto exit;

	book = bittrex->books_get(btcltc);

	if (book == NULL)
		goto exit;

	if (book->next == NULL)
		goto exit;

	ret = 1;

	exit:
	if (book != NULL)
		book_free(book);
	if (market != NULL)
		market_free(market);
	if (bittrex != NULL)
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
