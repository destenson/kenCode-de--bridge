#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "bridge/book.h"

int market_timeout = 180; // 3 minutes = 180s
/***
 * interface to the different vendors' books
 */

/**
 * helper method to set the name on the vendor struct
 */
void vendor_set_name(struct Vendor* vendor, const char* name) {
	vendor->Name = malloc(strlen(name) + 1);
	strcpy(vendor->Name, name);
}

struct Vendor* vendor_new() {
	struct Vendor* vendor = malloc(sizeof(struct Vendor));
	if (vendor == NULL)
		return NULL;
	//pthread_mutex_init(&vendor->market_mutex, NULL);
	vendor->Name = NULL;
	vendor->current_market = NULL;
	vendor->running = 0;
	vendor->IsInitialized = 0;
	return vendor;
}

void vendor_free(struct Vendor* vendor) {
	//shutdown thread
	vendor->running = 0;
#ifndef SINGLE_THREADED
	void* status;
	pthread_join(vendor->scheduler, &status);
#endif
	// free memory allocations
	free(vendor->Name);
	market_free(vendor->current_market);
	//pthread_mutex_destroy(&vendor->market_mutex);
	free(vendor);
}

void* market_update_loop(void* args) {
	struct Vendor* vendor = (struct Vendor*)args;
	struct Market* to_be_deleted = NULL;
	do {
		// update markets, keeping the old one around for a bit (market_timeout)
		if (to_be_deleted != NULL) {
			market_free(to_be_deleted);
		}
		to_be_deleted = vendor->current_market;
		vendor->current_market = vendor->markets_get();
		vendor->IsInitialized = 1;
		// pause in 2 loops, check every second, then exit loop after timeout
		int sTotal = 0;
		while(vendor->running && sTotal < market_timeout) {
			sleep(1);
			sTotal++;
		}
	} while (vendor->running);
#ifndef SINGLE_THREADED
	pthread_exit(NULL);
#endif
	return NULL;
}

/***
 * Get the Book struct for a specific vendor
 * @param vendor the name of the vendor
 * @returns the struct Book for that vendor, or NULL on error
 */
struct Vendor* vendor_get(const char* vendor_name) {
	struct Vendor* vendor = vendor_new();
	if (vendor == NULL)
		return NULL;

	vendor_set_name(vendor, vendor_name);

	if (strcmp(vendor_name, "mock") == 0) {
		vendor->books_get = mock_get_books;
		vendor->markets_get = mock_get_markets;
	}
	if (strcmp(vendor_name, "bittrex") == 0) {
		vendor->books_get = bittrex_get_books;
		vendor->markets_get = bittrex_get_markets;
	}
	if (strcmp(vendor_name, "btc38") == 0) {
		vendor->books_get = btc38_get_books;
		vendor->markets_get = btc38_get_markets;
	}
	if (strcmp(vendor_name, "poloniex")==0) {
		vendor->books_get = poloniex_get_books;
		vendor->markets_get = poloniex_get_markets;
	}
#ifndef SINGLE_THREADED
	vendor->running = 1;
	pthread_create(&(vendor->scheduler), NULL, market_update_loop, vendor);
#else
	market_update_loop(vendor);
#endif
	return vendor;
}

/**
 * Create a new struct VendorList
 * @returns a pointer to the newly allocated struct VendorList
 */
struct VendorList* vendor_list_new() {
	struct VendorList* list = malloc(sizeof(struct VendorList));
	if (list != NULL) {
		list->next = NULL;
	}
	return list;
}

/***
 * Free allocation of a struct VendorList
 * @param head the first in the list to be freed
 * @param free_vendor true(1) if you want to deallocate the vendor too
 */
void vendor_list_free(struct VendorList* head, int free_vendor) {
	struct VendorList* current = head;
	struct VendorList* next = NULL;
	while (current != NULL) {
		next = current->next;
		if (free_vendor)
			vendor_free(current->vendor);
		free(current);
		current = next;
	}
}

struct VendorList* vendors_get_all() {
	struct VendorList* vendors = vendor_list_new();
	vendors->vendor = vendor_get("btc38");
	vendors->next = vendor_list_new();
	vendors->next->vendor = vendor_get("bittrex");
	vendors->next->next = vendor_list_new();
	vendors->next->next->vendor = vendor_get("poloniex");
	return vendors;
}


struct VendorList* vendors_with_market(struct VendorList* head, const char* base_currency, const char* market_currency) {
	struct VendorList* new_list = NULL;
	struct VendorList* last_added = NULL;
	struct VendorList* current = head;
	while (current != NULL) {
		if (current->vendor->IsInitialized) { // ignore vendors that have yet to be initialized
			// try it the normal way
			const struct Market* mkt = market_get(current->vendor->current_market, base_currency, market_currency);
			if (mkt == NULL) // try it the opposite way
				mkt = market_get(current->vendor->current_market, market_currency, base_currency);
			if (mkt != NULL) {
				if (new_list == NULL) {
					new_list = vendor_list_new();
					last_added = new_list;
				} else {
					last_added->next = vendor_list_new();
					last_added = last_added->next;
				}
				last_added->vendor = current->vendor;
			}
		}
		current = current->next;
	}
	return new_list;
}


struct Book* book_new() {
	struct Book* book = (struct Book*)malloc(sizeof(struct Book));
	if (book != NULL) {
		book->ask_price = 0.0;
		book->ask_qty = 0.0;
		book->bid_price = 0.0;
		book->bid_qty = 0.0;
		book->next = NULL;
	}
	return book;
}

/**
 * Free memory resources used by linked list of Book
 * @param book the parent node of the book list
 */
void book_free(struct Book* book) {
	if (book != NULL) {
		struct Book* current = book;
		struct Book* next = book->next;
		while (current != NULL) {
			free(current);
			current = next;
			if (current != NULL)
				next = current->next;
		}
	}
}

struct Market* market_new() {
	struct Market* market = (struct Market*)malloc(sizeof(struct Market));
	if (market == NULL)
		return NULL;
	market->base_currency = NULL;
	market->market_currency = NULL;
	market->market_name = NULL;
	market->next = NULL;
	market->min_trade_size = 0.0;
	return market;
}
/**
 * Free memory resources used by linked list of Market
 * @param market the parent node of the market list
 */
void market_free(struct Market* market) {
	if (market != NULL) {
		struct Market* current = market;
		struct Market* next = market->next;
		while (current != NULL) {
			if (current->base_currency != NULL)
				free(current->base_currency);
			if (current->market_currency != NULL)
				free(current->market_currency);
			if (current->market_name != NULL)
				free(current->market_name);
			free(current);
			current = next;
			if (current != NULL)
				next = current->next;
		}
	}
}

const struct Market* market_get(const struct Market* head, const char* base_currency, const char* market_currency) {
	const struct Market* current = head;
	while (current != NULL) {
		if (strcmp(current->base_currency, base_currency) == 0 && strcmp(current->market_currency, market_currency) == 0)
			break;
		current = current->next;
	}
	return current;
}
