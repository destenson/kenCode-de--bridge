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

