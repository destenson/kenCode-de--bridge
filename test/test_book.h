#include <stdlib.h>
#include <unistd.h>

#include "../include/bridge/book.h"

int verify_book(struct Book* book) {
	int retVal = 1;
	// make sure there are bids and asks
	if (book->ask_price <= 0) {
		printf("No ask prices in book\n");
		retVal = 0;
	}
	if (book->ask_qty <= 0 ) {
		printf("No ask quantity in book\n");
		retVal = 0;
	}
	if (book->bid_price <= 0) {
		printf("No bid prices in book\n");
		retVal = 0;
	}
	if (book->bid_qty <= 0) {
		printf("No bid quantity in book\n");
		retVal = 0;
	}
	if (book->next == NULL) {
		printf("Only 1 level deep\n");
		retVal = 0;
	}
	if (book->next->ask_price <= 0) {
		printf("No ask prices in book\n");
		retVal = 0;
	}
	if (book->next->ask_qty <= 0 ) {
		printf("No ask quantity in book\n");
		retVal = 0;
	}
	if (book->next->bid_price <= 0) {
		printf("No bid prices in book\n");
		retVal = 0;
	}
	if (book->next->bid_qty <= 0) {
		printf("No bid quantity in book\n");
		retVal = 0;
	}
	return retVal;
}

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
		vendor_free(vendor);

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
		vendor_free(bittrex);
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

	if (verify_book(book) == 0)
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
		vendor_free(bittrex);
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

int test_book_all() {
	int retVal = 0;
	struct VendorList* vendorList = vendors_get_all();

	// give the other threads a chance to initialize
	struct VendorList* currVendorListNode = vendorList;
	while (currVendorListNode != NULL) {
		if (!currVendorListNode->vendor->IsInitialized)
			sleep(3);
		else
			currVendorListNode = currVendorListNode->next;
	}

	struct VendorList* vendorsWithMarket = vendors_with_market(vendorList, "BTC", "LTC");

	// should be n
	if (vendorsWithMarket != NULL && vendorsWithMarket->next != NULL && vendorsWithMarket->next->next == NULL) {
		retVal = 1;
	} else {
		printf("Incorrect number of vendors found.\n");
	}

	struct VendorList* current = vendorsWithMarket;
	while (current != NULL) {
		const struct Market* btc_ltc_market = market_get(current->vendor->current_market, "BTC", "LTC");
		struct Book* book = current->vendor->books_get(btc_ltc_market);
		if (verify_book(book) == 0) {
			printf("Unable to verify book for vendor ");
			printf("%s", current->vendor->Name);
			printf("\n");
		}
		book_free(book);
		current = current->next;
	}

	vendor_list_free(vendorsWithMarket, 0);
	vendor_list_free(vendorList, 1);

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
