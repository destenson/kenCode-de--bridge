#include <stdio.h>

#include "bridge/book.h"

int test_poloniex_account_balance() {
	int retVal = 0;
	const char* method = "test_poloniex_account_balance";

	struct Vendor* vendor = vendor_get("poloniex");
	struct Balance* balance = vendor->balance("BTC");
	if (balance == NULL) {
		fprintf(stderr, "%s: Unable to get balance\n", method);
		goto exit;
	}
	retVal = 1;
	exit:
	balance_free(balance);
	vendor_free(vendor);
	return retVal;
}

int test_book_poloniex() {
	int ret = 0;
	struct Vendor* vendor = NULL;
	struct Market* market = NULL;
	struct Book* book = NULL;
	const struct Market* btc_ltc = NULL;

	vendor = vendor_get("poloniex");
	if (vendor == NULL)
		goto exit;
	market = vendor->markets_get();
	if (market == NULL)
		goto exit;
	btc_ltc = market_get(market, "BTC", "LTC");
	if (btc_ltc == NULL)
		goto exit;

	book = vendor->books_get(btc_ltc);

	if (book == NULL)
		goto exit;

	if (verify_book(book) == 0)
		goto exit;

	ret = 1;

	exit:
	if (book != NULL)
		book_free(book);
	if (market != NULL)
		market_free(market);
	if (vendor != NULL)
		vendor_free(vendor);
	return ret;
}
